/***************************************************************************
 *   Copyright (C) 2004 by Hiroyuki Kuwahara                               *
 *   kuwahara@cs.utah.edu                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <float.h>
#include <math.h>
#include "gsl/gsl_vector.h"
#include "gsl/gsl_multiroots.h"
#include "euler_method.h"


static BOOL _IsModelConditionSatisfied( IR *ir );


static RET_VAL _InitializeRecord( EULER_SIMULATION_RECORD *rec, BACK_END_PROCESSOR *backend, IR *ir );
static RET_VAL _InitializeSimulation( EULER_SIMULATION_RECORD *rec, int runNum );
static RET_VAL _RunSimulation( EULER_SIMULATION_RECORD *rec );

static RET_VAL _CleanSimulation( EULER_SIMULATION_RECORD *rec );
static RET_VAL _CleanRecord( EULER_SIMULATION_RECORD *rec );

static RET_VAL _CalculateReactionRates( EULER_SIMULATION_RECORD *rec );
static RET_VAL _CalculateReactionRate( EULER_SIMULATION_RECORD *rec, REACTION *reaction );
static RET_VAL _Update( EULER_SIMULATION_RECORD *rec );
static RET_VAL _Print( EULER_SIMULATION_RECORD *rec );
static RET_VAL _UpdateNodeValues( EULER_SIMULATION_RECORD *rec );
static RET_VAL _UpdateSpeciesValues( EULER_SIMULATION_RECORD *rec );

static double fireEvents( EULER_SIMULATION_RECORD *rec, double time );
static void fireEvent( EVENT *event, EULER_SIMULATION_RECORD *rec );
static void ExecuteAssignments( EULER_SIMULATION_RECORD *rec );
static void SetEventAssignmentsNextValues( EVENT *event, EULER_SIMULATION_RECORD *rec );
static void SetEventAssignmentsNextValuesTime( EVENT *event, EULER_SIMULATION_RECORD *rec, double time );
static RET_VAL EvaluateAlgebraicRules( EULER_SIMULATION_RECORD *rec );
static RET_VAL ExecuteFastReactions( EULER_SIMULATION_RECORD *rec );

DLLSCOPE RET_VAL STDCALL DoEulerSimulation( BACK_END_PROCESSOR *backend, IR *ir ) {
    RET_VAL ret = SUCCESS;
    UINT i = 0;
    UINT runs = 1;
    char *namePrefix = NULL;
    static EULER_SIMULATION_RECORD rec;
    UINT timeout = 0;

    START_FUNCTION("DoEulerSimulation");

    if( !_IsModelConditionSatisfied( ir ) ) {
        return ErrorReport( FAILING, "DoEulerSimulation", "euler method cannot be applied to the model" );
    }

    if( IS_FAILED( ( ret = _InitializeRecord( &rec, backend, ir ) ) ) )  {
        return ErrorReport( ret, "DoEulerSimulation", "initialization of the record failed" );
    }
    runs = rec.runs;
    for( i = 1; i <= runs; i++ ) {
      timeout = 0;
      SeedRandomNumberGenerators( rec.seed );
      rec.seed = GetNextUniformRandomNumber(0,RAND_MAX);
      do {
	SeedRandomNumberGenerators( rec.seed );
	if( IS_FAILED( ( ret = _InitializeSimulation( &rec, i ) ) ) ) {
	  return ErrorReport( ret, "DoEulerSimulation", "initialization of the %i-th simulation failed", i );
	}
	timeout++;
      } while ( (ret == CHANGE) && (timeout <= (rec.speciesSize + rec.compartmentsSize + rec.symbolsSize)) );
      if (timeout > (rec.speciesSize + rec.compartmentsSize + rec.symbolsSize)) {
	return ErrorReport( ret, "DoEulerSimulation", "Cycle detected in initial and rule assignments" );
      }
      if( IS_FAILED( ( ret = _RunSimulation( &rec ) ) ) ) {
        return ErrorReport( ret, "DoEulerSimulation", "%i-th simulation failed at time %f", i, rec.time );
      }
      if( IS_FAILED( ( ret = _CleanSimulation( &rec ) ) ) ) {
        return ErrorReport( ret, "DoEulerSimulation", "cleaning of the %i-th simulation failed", i );
      }
      printf("Run = %d\n",i);
      fflush(stdout);
    }

    END_FUNCTION("DoEulerSimulation", SUCCESS );
    return ret;
}

DLLSCOPE RET_VAL STDCALL CloseEulerSimulation( BACK_END_PROCESSOR *backend ) {
    RET_VAL ret = SUCCESS;
    EULER_SIMULATION_RECORD *rec = (EULER_SIMULATION_RECORD*)(backend->_internal1);

    START_FUNCTION("CloseEulerSimulation");

    if( IS_FAILED( ( ret = _CleanRecord( rec ) ) ) )  {
        return ErrorReport( ret, "CloseEulerSimulation", "cleaning of the record failed" );
    }

    END_FUNCTION("CloseEulerSimulation",  SUCCESS );
    return ret;
}


static BOOL _IsModelConditionSatisfied( IR *ir ) {
#if 0
    REACTION *reaction = NULL;
    LINKED_LIST *reactions = NULL;

    reactions = ir->GetListOfReactionNodes( ir );
    while( ( reaction = (REACTION*)GetNextFromLinkedList( reactions ) ) != NULL ) {
        if( IsReactionReversibleInReactionNode( reaction ) ) {
            TRACE_0("the input model contains reversible reaction(s), and cannot be used for Euler method" );
            return FALSE;
        }
    }
#endif
    return TRUE;
}


static RET_VAL _InitializeRecord( EULER_SIMULATION_RECORD *rec, BACK_END_PROCESSOR *backend, IR *ir ) {
    RET_VAL ret = SUCCESS;
    UINT32 i = 0;
    UINT32 j = 0;
    double printInterval = 0.0;
    double minPrintInterval = 0.0;
    UINT32 size = 0;
    UINT32 algebraicVars = 0;
    char buf[512];
    char *valueString = NULL;
    SPECIES *species = NULL;
    SPECIES **speciesArray = NULL;
    REACTION *reaction = NULL;
    REACTION **reactions = NULL;
    COMPARTMENT *compartment = NULL;
    COMPARTMENT **compartmentArray = NULL;
    COMPARTMENT_MANAGER *compartmentManager;
    REB2SAC_SYMBOL *symbol = NULL;
    REB2SAC_SYMBOL **symbolArray = NULL;
    REB2SAC_SYMTAB *symTab;
    RULE *rule = NULL;
    RULE **ruleArray = NULL;
    RULE_MANAGER *ruleManager;
    CONSTRAINT *constraint = NULL;
    CONSTRAINT **constraintArray = NULL;
    CONSTRAINT_MANAGER *constraintManager;
    EVENT *event = NULL;
    EVENT **eventArray = NULL;
    EVENT_MANAGER *eventManager;
    EVENT_ASSIGNMENT *eventAssignment;
    COMPILER_RECORD_T *compRec = backend->record;
    LINKED_LIST *list = NULL;
    REB2SAC_PROPERTIES *properties = NULL;

#if GET_SEED_FROM_COMMAND_LINE
    PROPERTIES *options = NULL;
#endif

    list = ir->GetListOfReactionNodes( ir );
    rec->reactionsSize = GetLinkedListSize( list );
    rec->numberFastReactions = 0;
    if (rec->reactionsSize!=0) {
      if( ( reactions = (REACTION**)MALLOC( rec->reactionsSize * sizeof(REACTION*) ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not allocate memory for reaction array" );
      }
      i = 0;
      ResetCurrentElement( list );
      while( ( reaction = (REACTION*)GetNextFromLinkedList( list ) ) != NULL ) {
        reactions[i] = reaction;
        i++;
	if (IsReactionFastInReactionNode( reaction )) {
	  rec->numberFastReactions++;
	  /*
	  if (IsReactionReversibleInReactionNode( reaction )) {
	    rec->numberFastReactions++;
	  }
	  */
	}
      }
    }
    rec->reactionArray = reactions;

    if( rec->numberFastReactions > 1 ) {
        return ErrorReport( FAILING, "_InitializeRecord",
                            "Simulator supports only a single fast reaction" );
    }

    if( ( ruleManager = ir->GetRuleManager( ir ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not get the rule manager" );
    }
    list = ruleManager->CreateListOfRules( ruleManager );
    rec->rulesSize = GetLinkedListSize( list );
    if ( rec->rulesSize > 0 ) {
      if( ( ruleArray = (RULE**)MALLOC( rec->rulesSize * sizeof(RULE*) ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not allocate memory for rules array" );
      }
    }
    i = 0;
    rec->algebraicRulesSize = 0;
    ResetCurrentElement( list );
    while( ( rule = (RULE*)GetNextFromLinkedList( list ) ) != NULL ) {
      ruleArray[i] = rule;
      i++;
      if ( GetRuleType( rule ) == RULE_TYPE_ALGEBRAIC ) {
	rec->algebraicRulesSize++;
      }
    }
    rec->ruleArray = ruleArray;

    if( ( symTab = ir->GetGlobalSymtab( ir ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not get the symbol table" );
    }
    list = symTab->GenerateListOfSymbols( symTab );
    rec->symbolsSize = GetLinkedListSize( list );
    if ( rec->symbolsSize > 0 ) {
      if( ( symbolArray = (REB2SAC_SYMBOL**)MALLOC( rec->symbolsSize * sizeof(REB2SAC_SYMBOL*) ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not allocate memory for symbols array" );
      }
    }
    i = 0;
    ResetCurrentElement( list );
    while( ( symbol = (REB2SAC_SYMBOL*)GetNextFromLinkedList( list ) ) != NULL ) {
        symbolArray[i] = symbol;
	if (IsSymbolAlgebraic( symbol )) {
	  algebraicVars++;
	}
        i++;
    }
    rec->symbolArray = symbolArray;

    if( ( compartmentManager = ir->GetCompartmentManager( ir ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not get the compartment manager" );
    }
    list = compartmentManager->CreateListOfCompartments( compartmentManager );
    rec->compartmentsSize = GetLinkedListSize( list );
    if ( rec->compartmentsSize > 0 ) {
      if( ( compartmentArray = (COMPARTMENT**)MALLOC( rec->compartmentsSize * sizeof(RULE*) ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not allocate memory for compartment array" );
      }
    }
    i = 0;
    ResetCurrentElement( list );
    while( ( compartment = (COMPARTMENT*)GetNextFromLinkedList( list ) ) != NULL ) {
        compartmentArray[i] = compartment;
	if (IsCompartmentAlgebraic( compartment )) {
	  algebraicVars++;
	}
        i++;
    }
    rec->compartmentArray = compartmentArray;

    list = ir->GetListOfSpeciesNodes( ir );
    rec->speciesSize = GetLinkedListSize( list );
    if ( rec->speciesSize > 0 ) {
      if( ( speciesArray = (SPECIES**)MALLOC( rec->speciesSize * sizeof(SPECIES*) ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not allocate memory for species array" );
      }
    }
    i = 0;
    rec->numberFastSpecies = 0;
    ResetCurrentElement( list );
    while( ( species = (SPECIES*)GetNextFromLinkedList( list ) ) != NULL ) {
        speciesArray[i] = species;
	if (IsSpeciesNodeAlgebraic( species )) {
	  algebraicVars++;
	}
	if (IsSpeciesNodeFast( species )) {
	  rec->numberFastSpecies++;
	}
        i++;
    }
    if (rec->numberFastSpecies > 0) {
      if( ( rec->fastCons = (double*)MALLOC( rec->numberFastSpecies * sizeof(double) ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not allocate memory for fastCons array" );
      }
    }
    rec->speciesArray = speciesArray;
    if ( algebraicVars > rec->algebraicRulesSize ) {
      return ErrorReport( FAILING, "_InitializeRecord", "model underdetermined" );
    } else if ( algebraicVars < rec->algebraicRulesSize ) {
      return ErrorReport( FAILING, "_InitializeRecord", "model overdetermined" );
    } 

    for (i = 0; i < rec->rulesSize; i++) {
      if ( GetRuleType( rec->ruleArray[i] ) == RULE_TYPE_ASSIGNMENT ||
	   GetRuleType( rec->ruleArray[i] ) == RULE_TYPE_RATE_ASSIGNMENT ) {
	for (j = 0; j < rec->speciesSize; j++) {
	  if ( strcmp( GetCharArrayOfString(GetRuleVar( rec->ruleArray[i] )),
		       GetCharArrayOfString(GetSpeciesNodeID( rec->speciesArray[j] ) ) ) == 0 ) {
	    SetRuleVarType( ruleArray[i], SPECIES_RULE );
	    SetRuleIndex( ruleArray[i], j );
	    break;
	  }
	}
	for (j = 0; j < rec->compartmentsSize; j++) {
	  if ( strcmp( GetCharArrayOfString(GetRuleVar( rec->ruleArray[i] )),
		       GetCharArrayOfString(GetCompartmentID( rec->compartmentArray[j] ) ) ) == 0 ) {
	    SetRuleVarType( ruleArray[i], COMPARTMENT_RULE );
	    SetRuleIndex( ruleArray[i], j );
	    break;
	  }
	}
	for (j = 0; j < rec->symbolsSize; j++) {
	  if ( strcmp( GetCharArrayOfString(GetRuleVar( rec->ruleArray[i] )),
		       GetCharArrayOfString(GetSymbolID( rec->symbolArray[j] ) ) ) == 0 ) {
	    SetRuleVarType( ruleArray[i], PARAMETER_RULE );
	    SetRuleIndex( ruleArray[i], j );
	    break;
	  }
	}
      }
    }

    if( ( rec->evaluator = CreateKineticLawEvaluater() ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not create evaluator" );
    }

    if( ( rec->findNextTime = CreateKineticLawFind_Next_Time() ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not create find next time" );
    }

    properties = compRec->properties;
    if( ( valueString = properties->GetProperty( properties, ODE_SIMULATION_TIME_LIMIT ) ) == NULL ) {
        rec->timeLimit = DEFAULT_ODE_SIMULATION_TIME_LIMIT_VALUE;
    }
    else {
        if( IS_FAILED( ( ret = StrToFloat( &(rec->timeLimit), valueString ) ) ) ) {
            rec->timeLimit = DEFAULT_ODE_SIMULATION_TIME_LIMIT_VALUE;
        }
    }

    rec->minPrintInterval = -1.0;
    if ((valueString = properties->GetProperty(properties, ODE_SIMULATION_PRINT_INTERVAL)) == NULL) {
        if ((valueString = properties->GetProperty(properties, ODE_SIMULATION_NUMBER_STEPS)) == NULL) {
            if ((valueString = properties->GetProperty(properties, ODE_SIMULATION_MINIMUM_PRINT_INTERVAL))
                    == NULL) {
                rec->numberSteps = DEFAULT_ODE_SIMULATION_NUMBER_STEPS_VALUE;
            } else {
                if (IS_FAILED((ret = StrToFloat(&(minPrintInterval), valueString)))) {
                    rec->numberSteps = DEFAULT_ODE_SIMULATION_NUMBER_STEPS_VALUE;
                } else {
                    rec->minPrintInterval = minPrintInterval;
                }
            }
        } else {
            if (IS_FAILED((ret = StrToUINT32(&(rec->numberSteps), valueString)))) {
                rec->numberSteps = DEFAULT_ODE_SIMULATION_NUMBER_STEPS_VALUE;
            }
        }
    } else {
        if (IS_FAILED((ret = StrToFloat(&(printInterval), valueString)))) {
            rec->numberSteps = DEFAULT_ODE_SIMULATION_NUMBER_STEPS_VALUE;
        } else {
            rec->numberSteps = rec->timeLimit / printInterval;
        }
    }

#if GET_SEED_FROM_COMMAND_LINE
    options = compRec->options;
    if( ( valueString = options->GetProperty( options, "random.seed" ) ) == NULL ) {
        rec->seed = DEFAULT_MONTE_CARLO_SIMULATION_RANDOM_SEED_VALUE;
    }
    else {
      if( IS_FAILED( ( ret = StrToUINT32( (UINT32*)&(rec->seed), valueString ) ) ) ) {
            rec->seed = DEFAULT_MONTE_CARLO_SIMULATION_RANDOM_SEED_VALUE;
        }
        TRACE_1("seed from command line is %i", rec->seed );
    }
#else
    if( ( valueString = properties->GetProperty( properties, MONTE_CARLO_SIMULATION_RANDOM_SEED ) ) == NULL ) {
        rec->seed = DEFAULT_MONTE_CARLO_SIMULATION_RANDOM_SEED_VALUE;
    }
    else {
      if( IS_FAILED( ( ret = StrToUINT32( (UINT32*)&(rec->seed), valueString ) ) ) ) {
            rec->seed = DEFAULT_MONTE_CARLO_SIMULATION_RANDOM_SEED_VALUE;
        }
    }
#endif

    if( ( valueString = properties->GetProperty( properties, MONTE_CARLO_SIMULATION_START_INDEX ) ) == NULL ) {
        rec->startIndex = DEFAULT_MONTE_CARLO_SIMULATION_START_INDEX;
    }
    else {
      if( IS_FAILED( ( ret = StrToUINT32( (UINT32*)&(rec->startIndex), valueString ) ) ) ) {
	  rec->startIndex = DEFAULT_MONTE_CARLO_SIMULATION_START_INDEX;
        }
    }

    if( ( valueString = properties->GetProperty( properties, MONTE_CARLO_SIMULATION_RUNS ) ) == NULL ) {
        rec->runs = DEFAULT_MONTE_CARLO_SIMULATION_RUNS_VALUE;
    }
    else {
        if( IS_FAILED( ( ret = StrToUINT32( &(rec->runs), valueString ) ) ) ) {
            rec->runs = DEFAULT_MONTE_CARLO_SIMULATION_RUNS_VALUE;
        }
    }

    if( ( valueString = properties->GetProperty( properties, ODE_SIMULATION_TIME_STEP ) ) == NULL ) {
        rec->timeStep = DEFAULT_ODE_SIMULATION_TIME_STEP;
    }
    else {
        if( IS_FAILED( ( ret = StrToFloat( &(rec->timeStep), valueString ) ) ) ) {
            rec->timeStep = DEFAULT_ODE_SIMULATION_TIME_STEP;
        }
    }

    if( ( rec->outDir = properties->GetProperty( properties, ODE_SIMULATION_OUT_DIR ) ) == NULL ) {
        rec->outDir = DEFAULT_ODE_SIMULATION_OUT_DIR_VALUE;
    }

    if( ( rec->printer = CreateSimulationPrinter( backend, compartmentArray, rec->compartmentsSize,
						  speciesArray, rec->speciesSize,
						  symbolArray, rec->symbolsSize ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not create simulation printer" );
    }

    if( ( constraintManager = ir->GetConstraintManager( ir ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not get the constraint manager" );
    }
    list = constraintManager->CreateListOfConstraints( constraintManager );
    rec->constraintsSize = GetLinkedListSize( list );
    if ( rec->constraintsSize > 0 ) {
      if( ( constraintArray = (CONSTRAINT**)MALLOC( rec->constraintsSize * sizeof(CONSTRAINT*) ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not allocate memory for constraints array" );
      }
    }
    i = 0;
    ResetCurrentElement( list );
    while( ( constraint = (CONSTRAINT*)GetNextFromLinkedList( list ) ) != NULL ) {
        constraintArray[i] = constraint;
        i++;
    }
    rec->constraintArray = constraintArray;

    if( ( rec->decider =
        CreateSimulationRunTerminationDecider( backend, speciesArray, rec->speciesSize, reactions, rec->reactionsSize,
					       rec->constraintArray, rec->constraintsSize, rec->evaluator, TRUE, rec->timeLimit ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not create simulation printer" );
    }

    if( ( eventManager = ir->GetEventManager( ir ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not get the event manager" );
    }
    list = eventManager->CreateListOfEvents( eventManager );
    rec->eventsSize = GetLinkedListSize( list );
    if ( rec->eventsSize > 0 ) {
      if( ( eventArray = (EVENT**)MALLOC( rec->eventsSize * sizeof(EVENT*) ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not allocate memory for events array" );
      }
    }
    i = 0;
    ResetCurrentElement( list );
    while( ( event = (EVENT*)GetNextFromLinkedList( list ) ) != NULL ) {
        eventArray[i] = event;
        i++;
    }
    rec->eventArray = eventArray;

    for (i = 0; i < rec->eventsSize; i++) {
      list = GetEventAssignments( rec->eventArray[i] );
      ResetCurrentElement( list );
      while( ( eventAssignment = (EVENT_ASSIGNMENT*)GetNextFromLinkedList( list ) ) != NULL ) {
	for (j = 0; j < rec->speciesSize; j++) {
	  if ( strcmp( GetCharArrayOfString(eventAssignment->var),
		       GetCharArrayOfString(GetSpeciesNodeID( rec->speciesArray[j] ) ) ) == 0 ) {
	    SetEventAssignmentVarType( eventAssignment, SPECIES_EVENT_ASSIGNMENT );
	    SetEventAssignmentIndex( eventAssignment, j );
	    break;
	  }
	}
	for (j = 0; j < rec->compartmentsSize; j++) {
	  if ( strcmp( GetCharArrayOfString(eventAssignment->var),
		       GetCharArrayOfString(GetCompartmentID( rec->compartmentArray[j] ) ) ) == 0 ) {
	    SetEventAssignmentVarType( eventAssignment, COMPARTMENT_EVENT_ASSIGNMENT );
	    SetEventAssignmentIndex( eventAssignment, j );
	    break;
	  }
	}
	for (j = 0; j < rec->symbolsSize; j++) {
	  if ( strcmp( GetCharArrayOfString(eventAssignment->var),
		       GetCharArrayOfString(GetSymbolID( rec->symbolArray[j] ) ) ) == 0 ) {
	    SetEventAssignmentVarType( eventAssignment, PARAMETER_EVENT_ASSIGNMENT );
	    SetEventAssignmentIndex( eventAssignment, j );
	    break;
	  }
	}
      }
    }

    backend->_internal1 = (CADDR_T)rec;

    return ret;
}

static RET_VAL _InitializeSimulation( EULER_SIMULATION_RECORD *rec, int runNum ) {
    RET_VAL ret = SUCCESS;
    char filenameStem[512];
    double param = 0.0;
    double compSize = 0.0;
    double concentration = 0.0;
    UINT32 i = 0;
    UINT32 size = 0;
    SPECIES *species = NULL;
    SPECIES **speciesArray = rec->speciesArray;
    COMPARTMENT *compartment = NULL;
    COMPARTMENT **compartmentArray = rec->compartmentArray;
    REACTION *reaction = NULL;
    REACTION **reactionArray = rec->reactionArray;
    REB2SAC_SYMBOL *symbol = NULL;
    REB2SAC_SYMBOL **symbolArray = rec->symbolArray;
    KINETIC_LAW_EVALUATER *evaluator = rec->evaluator;
    SIMULATION_PRINTER *printer = rec->printer;
    KINETIC_LAW *law = NULL;
    BOOL change = FALSE;

    sprintf( filenameStem, "%s%crun-%i", rec->outDir, FILE_SEPARATOR, (runNum + rec->startIndex - 1) );
    if( IS_FAILED( (  ret = printer->PrintStart( printer, filenameStem ) ) ) ) {
        return ret;
    }
    if( IS_FAILED( (  ret = printer->PrintHeader( printer ) ) ) ) {
        return ret;
    }
    rec->time = 0.0;
    rec->nextPrintTime = 0.0;
    rec->currentStep = 0;
    size = rec->compartmentsSize;
    for( i = 0; i < size; i++ ) {
        compartment = compartmentArray[i];
	if ( (law = (KINETIC_LAW*)GetInitialAssignmentInCompartment( compartment )) == NULL ) {
	  compSize = GetSizeInCompartment( compartment );
	} else {
	  law = CloneKineticLaw( law );
	  SimplifyInitialAssignment(law);
	  if (law->valueType == KINETIC_LAW_VALUE_TYPE_REAL) {
	    compSize = GetRealValueFromKineticLaw(law);
	  } else if (law->valueType == KINETIC_LAW_VALUE_TYPE_INT) {
	    compSize = (double)GetIntValueFromKineticLaw(law);
	  }
	  if (GetSizeInCompartment( compartment ) != compSize) {
	    SetSizeInCompartment( compartment, compSize );
	    change = TRUE;
	  }
	  FreeKineticLaw( &(law) );
	}
        if( IS_FAILED( ( ret = SetCurrentSizeInCompartment( compartment, compSize ) ) ) ) {
            return ret;
        }
    }
    size = rec->speciesSize;
    for( i = 0; i < size; i++ ) {
        species = speciesArray[i];
	if ( (law = (KINETIC_LAW*)GetInitialAssignmentInSpeciesNode( species )) == NULL ) {
	  if( IsInitialQuantityInAmountInSpeciesNode( species ) ) {
	    compSize = GetCurrentSizeInCompartment( GetCompartmentInSpeciesNode( species ) );
	    concentration = GetInitialAmountInSpeciesNode( species ) / compSize;
	  }
	  else {
	    concentration = GetInitialConcentrationInSpeciesNode( species );
	  }
	} else {
	  law = CloneKineticLaw( law );
	  SimplifyInitialAssignment(law);
	  if (law->valueType == KINETIC_LAW_VALUE_TYPE_REAL) {
	    concentration = GetRealValueFromKineticLaw(law);
	  } else if (law->valueType == KINETIC_LAW_VALUE_TYPE_INT) {
	    concentration = (double)GetIntValueFromKineticLaw(law);
	  }
	  if( IsInitialQuantityInAmountInSpeciesNode( species ) ) {
	    if (GetInitialAmountInSpeciesNode( species ) != concentration) {
	      SetInitialAmountInSpeciesNode( species, concentration );
	      change = TRUE;
	    }
	  }
	  else {
	    if (GetInitialConcentrationInSpeciesNode( species ) != concentration) {
	      SetInitialConcentrationInSpeciesNode( species, concentration );
	      change = TRUE;
	    }
	  }
	  FreeKineticLaw( &(law) );
	}
        if( IS_FAILED( ( ret = SetConcentrationInSpeciesNode( species, concentration ) ) ) ) {
            return ret;
        }
    }
    size = rec->symbolsSize;
    for( i = 0; i < size; i++ ) {
        symbol = symbolArray[i];
	if ( (law = (KINETIC_LAW*)GetInitialAssignmentInSymbol( symbol )) == NULL ) {
	  param = GetRealValueInSymbol( symbol );
	} else {
	  law = CloneKineticLaw( law );
	  SimplifyInitialAssignment(law);
	  if (law->valueType == KINETIC_LAW_VALUE_TYPE_REAL) {
	    param = GetRealValueFromKineticLaw(law);
	  } else if (law->valueType == KINETIC_LAW_VALUE_TYPE_INT) {
	    param = (double)GetIntValueFromKineticLaw(law);
	  }
	  if( GetRealValueInSymbol( symbol ) != param ) {
	    SetRealValueInSymbol( symbol, param );
	    change = TRUE;
	  }
	  FreeKineticLaw( &(law) );
	}
	if( IS_FAILED( ( ret = SetCurrentRealValueInSymbol( symbol, param ) ) ) ) {
	  return ret;
	}
    }
    for (i = 0; i < rec->eventsSize; i++) {
      /* SetTriggerEnabledInEvent( rec->eventArray[i], FALSE ); */
      /* Use the line below to support true SBML semantics, i.e., nothing can be trigger at t=0 */
      if (rec->evaluator->EvaluateWithCurrentConcentrationsDeter( rec->evaluator,
							     (KINETIC_LAW*)GetTriggerInEvent( rec->eventArray[i] ) )) {
	if (GetTriggerInitialValue( rec->eventArray[i] )) {
    	  SetTriggerEnabledInEvent( rec->eventArray[i], TRUE );
	} else {
	  SetTriggerEnabledInEvent( rec->eventArray[i], FALSE );
	}
      } else {
    	  SetTriggerEnabledInEvent( rec->eventArray[i], FALSE );
      }
    }
    if (rec->algebraicRulesSize > 0) {
      EvaluateAlgebraicRules( rec );
    }
    if (rec->numberFastSpecies > 0) {
      ExecuteFastReactions( rec );
    }

    if (change)
      return CHANGE;
    return ret;
}

static RET_VAL _RunSimulation( EULER_SIMULATION_RECORD *rec ) {
    RET_VAL ret = SUCCESS;
    SIMULATION_PRINTER *printer = NULL;
    SIMULATION_RUN_TERMINATION_DECIDER *decider = NULL;
    double nextEventTime;

    printer = rec->printer;
    decider = rec->decider;
    nextEventTime = fireEvents( rec, rec->time );
    if (nextEventTime==-2.0) {
      return FAILING;
    }
    while( !(decider->IsTerminationConditionMet( decider, NULL, rec->time )) ) {
        if( IS_FAILED( ( ret = _CalculateReactionRates( rec ) ) ) ) {
            return ret;
        }
        if( IS_FAILED( ( ret = _Update( rec ) ) ) ) {
            return ret;
        }
	rec->time += rec->timeStep;
	nextEventTime = fireEvents( rec, rec->time );
	if (nextEventTime==-2.0) {
	  return FAILING;
	}
	if ((nextEventTime > 0) && (nextEventTime < rec->time)) {
	  rec->time = nextEventTime;
	}
    }
    if (rec->time > 0.0) printf("Time = %.2f\n",rec->time);
    if( IS_FAILED( ( ret = printer->PrintValues( printer, rec->time ) ) ) ) {
        return ret;
    }

    if( IS_FAILED( ( ret = printer->PrintEnd( printer ) ) ) ) {
        return ret;
    }

    if( IS_FAILED( ( ret = _CleanSimulation( rec ) ) ) ) {
        return ret;
    }

    return ret;
}

static RET_VAL _CleanSimulation( EULER_SIMULATION_RECORD *rec ) {
    RET_VAL ret = SUCCESS;
    return ret;
}

static RET_VAL _CleanRecord( EULER_SIMULATION_RECORD *rec ) {
    RET_VAL ret = SUCCESS;
    SIMULATION_PRINTER *printer = rec->printer;
    SIMULATION_RUN_TERMINATION_DECIDER *decider = rec->decider;

    if( rec->evaluator != NULL ) {
        FreeKineticLawEvaluater( &(rec->evaluator) );
    }
    if( rec->reactionArray != NULL ) {
        FREE( rec->reactionArray );
    }
    if( rec->speciesArray != NULL ) {
        FREE( rec->speciesArray );
    }

    printer->Destroy( printer );
    decider->Destroy( decider );

    return ret;
}


static RET_VAL _CalculateReactionRates( EULER_SIMULATION_RECORD *rec ) {
    RET_VAL ret = SUCCESS;
    int i = 0;
    int size = 0;
    REACTION **reactionArray = rec->reactionArray;

    size = rec->reactionsSize;
    for( i = 0; i < size; i++ ) {
        if( IS_FAILED( ( ret = _CalculateReactionRate( rec, reactionArray[i] ) ) ) ) {
            return ret;
        }
    }
    return ret;
}


static RET_VAL _CalculateReactionRate( EULER_SIMULATION_RECORD *rec, REACTION *reaction ) {
    RET_VAL ret = SUCCESS;
    double rate = 0.0;
    SPECIES *species = NULL;
    KINETIC_LAW *law = NULL;
    KINETIC_LAW_EVALUATER *evaluator = rec->evaluator;

    law = GetKineticLawInReactionNode( reaction );
    rate = evaluator->EvaluateWithCurrentConcentrationsDeter( evaluator, law );
    if( !( rate < DBL_MAX ) ) {
        rate = 0.0;
    }
    if( IS_FAILED( ( ret = SetReactionRate( reaction, rate ) ) ) ) {
        return ret;
    }
    return ret;
}

static RET_VAL _Update( EULER_SIMULATION_RECORD *rec ) {
    RET_VAL ret = SUCCESS;

    if( IS_FAILED( ( ret = _Print( rec ) ) ) ) {
        return ret;
    }
    if( IS_FAILED( ( ret = _UpdateNodeValues( rec ) ) ) ) {
        return ret;
    }

    return ret;
}

static RET_VAL _Print( EULER_SIMULATION_RECORD *rec ) {
    RET_VAL ret = SUCCESS;
    double numberSteps = rec->numberSteps;
    double nextPrintTime = rec->nextPrintTime;
    double time = rec->time;
    SIMULATION_PRINTER *printer = rec->printer;

    if (rec->minPrintInterval == -1.0) {
      while( nextPrintTime <= time ) {
	if (nextPrintTime > 0.0) {
	  printf("Time = %.2f\n",nextPrintTime);
	  fflush(stdout);
	}
	if( IS_FAILED( ( ret = printer->PrintValues( printer, nextPrintTime ) ) ) ) {
	  return ret;
	}
	rec->currentStep++;
	nextPrintTime = (rec->currentStep/numberSteps) * rec->timeLimit;
      }
    } else {
      if ( nextPrintTime <= time ) {
	if (nextPrintTime > 0.0) {
	  printf("Time = %.2f\n",nextPrintTime);
	  fflush(stdout);
	}
	if( IS_FAILED( ( ret = printer->PrintValues( printer, time ) ) ) ) {
	  return ret;
	}
	nextPrintTime = time + rec->minPrintInterval;
      }
    }
    rec->nextPrintTime = nextPrintTime;
    return ret;
}

static RET_VAL _UpdateNodeValues( EULER_SIMULATION_RECORD *rec ) {
    RET_VAL ret = SUCCESS;

    if( IS_FAILED( ( ret = _UpdateSpeciesValues( rec ) ) ) ) {
        return ret;
    }

    return ret;
}

static double fireEvents( EULER_SIMULATION_RECORD *rec, double time ) {
    int i;
    double nextEventTime;
    BOOL triggerEnabled;
    double deltaTime;
    BOOL eventFired = FALSE;
    double firstEventTime = -1.0;
    int eventToFire = -1;
    double prMax;
    double priority = 0.0;
    double randChoice = 0.0;

    do {
      eventFired = FALSE;
      eventToFire = -1;
      for (i = 0; i < rec->eventsSize; i++) {
	nextEventTime = GetNextEventTimeInEvent( rec->eventArray[i] );
	triggerEnabled = GetTriggerEnabledInEvent( rec->eventArray[i] );
	if (nextEventTime != -1.0) {
	  /* Disable event, if necessary */
	  if ((triggerEnabled) && (GetTriggerCanBeDisabled( rec->eventArray[i] ))) {
	    if (!rec->evaluator->EvaluateWithCurrentConcentrationsDeter( rec->evaluator,
									 (KINETIC_LAW*)GetTriggerInEvent( rec->eventArray[i] ) )) { 
	      nextEventTime = -1.0;
	      SetNextEventTimeInEvent( rec->eventArray[i], -1.0 );
	      SetTriggerEnabledInEvent( rec->eventArray[i], FALSE );
	      continue;
	    }
	  }
	  if (time >= nextEventTime) {
	    if (GetPriorityInEvent( rec->eventArray[i] )==NULL) {
	      priority = 0;
	    }
	    else {
	      priority = rec->evaluator->EvaluateWithCurrentConcentrations( rec->evaluator,
								     (KINETIC_LAW*)GetPriorityInEvent( rec->eventArray[i] ) );
	    }
	    if ((eventToFire==(-1)) || (priority > prMax)) {
	      eventToFire = i;
	      prMax = priority;
	    } else if (priority == prMax) {
	      randChoice=GetNextUniformRandomNumber(0,1);	   
	      if (randChoice > 0.5) {
		eventToFire = i;
	      }
	    }
	  } else {
	    /* Determine time to next event */
	    if ((firstEventTime == -1.0) || (nextEventTime < firstEventTime)) {
	      firstEventTime = nextEventTime;
	    }
	  }
	}
	/* Try to find time to next event trigger */
	nextEventTime = rec->time + 
	  rec->findNextTime->FindNextTimeWithCurrentAmounts( rec->findNextTime,
							     (KINETIC_LAW*)GetTriggerInEvent( rec->eventArray[i] ));
	if ((firstEventTime == -1.0) || (nextEventTime < firstEventTime)) {
	  firstEventTime = nextEventTime;
	}
	if (!triggerEnabled) {
	  /* Check if event has been triggered */
	  if (rec->evaluator->EvaluateWithCurrentConcentrationsDeter( rec->evaluator,
								 (KINETIC_LAW*)GetTriggerInEvent( rec->eventArray[i] ) )) {
	    SetTriggerEnabledInEvent( rec->eventArray[i], TRUE );
	    /* Calculate delay until the event fires */
	    if (GetDelayInEvent( rec->eventArray[i] )==NULL) {
	      deltaTime = 0;
	    }
	    else {
	      deltaTime = rec->evaluator->EvaluateWithCurrentConcentrations( rec->evaluator,
								      (KINETIC_LAW*)GetDelayInEvent( rec->eventArray[i] ) );
	    }
	    if (deltaTime == 0) eventFired = TRUE;
	    if (deltaTime >= 0) {
	      /* Set time for event to fire and get assignment values, if necessary */
	      SetNextEventTimeInEvent( rec->eventArray[i], time + deltaTime );
	      if (GetUseValuesFromTriggerTime( rec->eventArray[i] )) {
		SetEventAssignmentsNextValuesTime( rec->eventArray[i], rec, time + deltaTime ); 
	      }
	      if ((firstEventTime == -1.0) || (time + deltaTime < firstEventTime)) {
		firstEventTime = time + deltaTime;
	      }
	    } /* else if (deltaTime == 0) {
	      SetEventAssignmentsNextValues( rec->eventArray[i], rec ); 
	      fireEvent( rec->eventArray[i], rec );
	      eventFired = TRUE;
	      } */ else {
	      ErrorReport( FAILING, "_Update", "delay for event evaluates to a negative number" );
	      return -2;
	    }
	  }
	} else {
	  /* Set trigger enabled to false, if it has become disabled */
	  if (!rec->evaluator->EvaluateWithCurrentConcentrationsDeter( rec->evaluator,
							   (KINETIC_LAW*)GetTriggerInEvent( rec->eventArray[i] ) )) {
	    SetTriggerEnabledInEvent( rec->eventArray[i], FALSE );
	  } 
	}
      }
      /* Fire event */
      if (eventToFire >= 0) {
	if (!GetUseValuesFromTriggerTime( rec->eventArray[eventToFire] )) {
	  SetEventAssignmentsNextValues( rec->eventArray[eventToFire], rec ); 
	}
	fireEvent( rec->eventArray[eventToFire], rec );
	SetNextEventTimeInEvent( rec->eventArray[eventToFire], -1.0 );
	eventFired = TRUE;
	eventToFire = -1;
	firstEventTime = -1;

	/* When an event fires, update algebraic rules and fast reactions */
	ExecuteAssignments( rec );
	if (rec->algebraicRulesSize > 0) {
	  EvaluateAlgebraicRules( rec );
	}
	if (rec->numberFastSpecies > 0) {
	  ExecuteFastReactions( rec );
	}
      }
      /* Repeat as long as events are firing */
    } while (eventFired);
    /* Return the time for the next event firing or potential triggering */
    return firstEventTime;
}

static void SetEventAssignmentsNextValues( EVENT *event, EULER_SIMULATION_RECORD *rec ) {
  LINKED_LIST *list = NULL;
  EVENT_ASSIGNMENT *eventAssignment;
  double concentration = 0.0;
  UINT j;
  BYTE varType;

  list = GetEventAssignments( event );
  ResetCurrentElement( list );
  while( ( eventAssignment = (EVENT_ASSIGNMENT*)GetNextFromLinkedList( list ) ) != NULL ) {
    concentration = rec->evaluator->EvaluateWithCurrentConcentrations( rec->evaluator, eventAssignment->assignment );
    SetEventAssignmentNextValueTime( eventAssignment, concentration, rec->time );
  }
}

static void SetEventAssignmentsNextValuesTime( EVENT *event, EULER_SIMULATION_RECORD *rec, double time ) {
  LINKED_LIST *list = NULL;
  EVENT_ASSIGNMENT *eventAssignment;
  double amount = 0.0;
  UINT j;
  BYTE varType;

  list = GetEventAssignments( event );
  ResetCurrentElement( list );
  while( ( eventAssignment = (EVENT_ASSIGNMENT*)GetNextFromLinkedList( list ) ) != NULL ) {
    amount = rec->evaluator->EvaluateWithCurrentConcentrations( rec->evaluator, eventAssignment->assignment );
    SetEventAssignmentNextValueTime( eventAssignment, amount, time );
  }
}

static void fireEvent( EVENT *event, EULER_SIMULATION_RECORD *rec ) {
  LINKED_LIST *list = NULL;
  EVENT_ASSIGNMENT *eventAssignment;
  double concentration = 0.0;
  UINT j;
  BYTE varType;

  list = GetEventAssignments( event );
  ResetCurrentElement( list );
  while( ( eventAssignment = (EVENT_ASSIGNMENT*)GetNextFromLinkedList( list ) ) != NULL ) {
    //printf("Firing event %s\n",GetCharArrayOfString(eventAssignment->var));
    varType = GetEventAssignmentVarType( eventAssignment );
    j = GetEventAssignmentIndex( eventAssignment );
    //printf("varType = %d j = %d\n",varType,j);
    concentration = GetEventAssignmentNextValueTime( eventAssignment, rec->time );
    //printf("conc = %g\n",amount);
    if ( varType == SPECIES_EVENT_ASSIGNMENT ) {
      SetConcentrationInSpeciesNode( rec->speciesArray[j], concentration );
    } else if ( varType == COMPARTMENT_EVENT_ASSIGNMENT ) {
      SetCurrentSizeInCompartment( rec->compartmentArray[j], concentration );
    } else {
      SetCurrentRealValueInSymbol( rec->symbolArray[j], concentration );
    }
  }
}

/* Update values using assignments rules */
static void ExecuteAssignments( EULER_SIMULATION_RECORD *rec ) {
  UINT32 i = 0;
  UINT32 j = 0;
  double concentration = 0.0;
  BYTE varType;

  for (i = 0; i < rec->rulesSize; i++) {
    if ( GetRuleType( rec->ruleArray[i] ) == RULE_TYPE_ASSIGNMENT ) {
      concentration = rec->evaluator->EvaluateWithCurrentConcentrationsDeter( rec->evaluator,
									 (KINETIC_LAW*)GetMathInRule( rec->ruleArray[i] ) );
      varType = GetRuleVarType( rec->ruleArray[i] );
      j = GetRuleIndex( rec->ruleArray[i] );
      if ( varType == SPECIES_RULE ) {
	if (HasOnlySubstanceUnitsInSpeciesNode( rec->speciesArray[j] )) {
	  SetAmountInSpeciesNode( rec->speciesArray[j], concentration );
	} else {
	  SetConcentrationInSpeciesNode( rec->speciesArray[j], concentration );
	}
      } else if ( varType == COMPARTMENT_RULE ) {
	SetCurrentSizeInCompartment( rec->compartmentArray[j], concentration );
      } else {
	SetCurrentRealValueInSymbol( rec->symbolArray[j], concentration );
      }
    }
  }
}

int EulerAlgebraicRules(const gsl_vector * x, void *params, gsl_vector * f) {
  UINT32 i = 0;
  UINT32 j = 0;
  double amount = 0.0;
  EULER_SIMULATION_RECORD *rec = ((EULER_SIMULATION_RECORD*)params);
  SPECIES *species = NULL;
  COMPARTMENT *compartment = NULL;
  REB2SAC_SYMBOL *symbol = NULL;
  
  for( i = 0; i < rec->speciesSize; i++ ) {
    species = rec->speciesArray[i];
    if (IsSpeciesNodeAlgebraic( species )) {
      amount = gsl_vector_get (x, j);
      j++; 
      if (HasOnlySubstanceUnitsInSpeciesNode( species )) {
	SetAmountInSpeciesNode( species, amount );
      } else {
	SetConcentrationInSpeciesNode( species, amount );
      }
    }
  }
  for( i = 0; i < rec->compartmentsSize; i++ ) {
    compartment = rec->compartmentArray[i];
    if (IsCompartmentAlgebraic( compartment )) {
      amount = gsl_vector_get (x, j);
      j++;
      SetCurrentSizeInCompartment( compartment, amount );
    }
  }
  for( i = 0; i < rec->symbolsSize; i++ ) {
    symbol = rec->symbolArray[i];
    if (IsSymbolAlgebraic( symbol )) {
      amount = gsl_vector_get (x, j);
      j++;
      SetCurrentRealValueInSymbol( symbol, amount );
    }
  }
  j = 0;
  for (i = 0; i < rec->rulesSize; i++) {
    if ( GetRuleType( rec->ruleArray[i] ) == RULE_TYPE_ALGEBRAIC ) {
      amount = rec->evaluator->EvaluateWithCurrentConcentrationsDeter( rec->evaluator,
							   (KINETIC_LAW*)GetMathInRule( rec->ruleArray[i] ) );
      gsl_vector_set (f, j, amount);
      j++;
    } 
  }
     
  return GSL_SUCCESS;
}

int EULER_print_state (size_t iter,gsl_multiroot_fsolver * s,int n) {
  UINT32 i = 0;

  printf("x = [ ");
  for (i = 0; i < n; i++) {
    printf(" %g ",gsl_vector_get (s->x, i));
  }
  printf("] f = [ ");
  for (i = 0; i < n; i++) {
    printf(" %g ",gsl_vector_get (s->f, i));
  }
  printf("]\n");
}

static RET_VAL EvaluateAlgebraicRules( EULER_SIMULATION_RECORD *rec ) {
  const gsl_multiroot_fsolver_type *T;
  gsl_multiroot_fsolver *s;
  SPECIES *species = NULL;
  COMPARTMENT *compartment = NULL;
  REB2SAC_SYMBOL *symbol = NULL;
     
  int status;
  size_t i, j, iter = 0;
  double amount;

  const size_t n = rec->algebraicRulesSize;

  gsl_multiroot_function f = {&EulerAlgebraicRules, n, rec};
  gsl_vector *x = gsl_vector_alloc (n);
     
  j = 0;
  for( i = 0; i < rec->speciesSize; i++ ) {
    species = rec->speciesArray[i];
    if (IsSpeciesNodeAlgebraic( species )) {
      if (HasOnlySubstanceUnitsInSpeciesNode( species )) {
	amount = GetAmountInSpeciesNode( species );
      } else {
	amount = GetConcentrationInSpeciesNode( species );
      }
      gsl_vector_set (x, j, amount);
      j++;
    } 
  }
  for( i = 0; i < rec->compartmentsSize; i++ ) {
    compartment = rec->compartmentArray[i];
    if (IsCompartmentAlgebraic( compartment )) {
      amount = GetCurrentSizeInCompartment( compartment );
      gsl_vector_set (x, j, amount);
      j++;
    }
  }
  for( i = 0; i < rec->symbolsSize; i++ ) {
    symbol = rec->symbolArray[i];
    if (IsSymbolAlgebraic( symbol )) {
      amount = GetCurrentRealValueInSymbol( symbol );
      gsl_vector_set (x, j, amount);
      j++;
    }
  }
     
  T = gsl_multiroot_fsolver_hybrids;
  s = gsl_multiroot_fsolver_alloc (T, n);
  gsl_multiroot_fsolver_set (s, &f, x);
     
  //Euler_print_state (iter, s, n);
     
  do {
      iter++;
      status = gsl_multiroot_fsolver_iterate (s);
      
      //Euler_print_state (iter, s, n);
      
      if (status)   /* check if solver is stuck */
	break;
      
      status = gsl_multiroot_test_residual (s->f, 1e-7);
  } while (status == GSL_CONTINUE && iter < 1000);
     
  //printf ("status = %s\n", gsl_strerror (status));
     
  gsl_multiroot_fsolver_free (s);
  gsl_vector_free (x);
  return 0;
}

int EulerfastReactions(const gsl_vector * x, void *params, gsl_vector * f) {
  RET_VAL ret = SUCCESS;
  UINT32 i = 0;
  UINT32 j;
  double amount = 0.0;
  EULER_SIMULATION_RECORD *rec = ((EULER_SIMULATION_RECORD*)params);
  SPECIES *species = NULL;
  REACTION *reaction = NULL;
  LINKED_LIST *Redges = NULL;
  LINKED_LIST *Pedges = NULL;
  IR_EDGE *edge = NULL;
  double stoichiometry = 0.0;
  BOOL boundary = FALSE;
  REB2SAC_SYMBOL *speciesRef = NULL;
  REB2SAC_SYMBOL *convFactor = NULL;

  j = 0;
  for( i = 0; i < rec->speciesSize; i++ ) {
    species = rec->speciesArray[i];
    if (IsSpeciesNodeFast( species )) {
      amount = gsl_vector_get (x, j);
      j++; 
      if (HasOnlySubstanceUnitsInSpeciesNode( species )) {
	SetAmountInSpeciesNode( species, amount );
      } else {
	SetConcentrationInSpeciesNode( species, amount );
      }
    }
  }
  if( IS_FAILED( ( ret = _CalculateReactionRates( rec ) ) ) ) {
    return GSL_FAILURE;
  }
  j = 0;
  for( i = 0; i < rec->reactionsSize; i++ ) {
    reaction = rec->reactionArray[i];
    if (IsReactionFastInReactionNode( reaction )) {
      amount = 0.0;
      Redges = GetReactantsInReactionNode( reaction );
      Pedges = GetProductsInReactionNode( reaction );
      ResetCurrentElement( Redges );
      if ( ( edge = GetNextEdge( Redges ) ) != NULL ) {
	speciesRef = GetSpeciesRefInIREdge( edge );
	if (speciesRef) {
	  stoichiometry = GetCurrentRealValueInSymbol( speciesRef );
	} else {
	  stoichiometry = GetStoichiometryInIREdge( edge );
	}
	species = GetSpeciesInIREdge( edge );
	if (( convFactor = GetConversionFactorInSpeciesNode( species ) )!=NULL) {
	  stoichiometry *= GetCurrentRealValueInSymbol( convFactor );
	}
	if (HasOnlySubstanceUnitsInSpeciesNode( species )) {
	  if (HasBoundaryConditionInSpeciesNode( species )) {
	    amount = GetAmountInSpeciesNode( species );
	    boundary = TRUE;
	  } else {
	    amount = GetAmountInSpeciesNode( species ) / stoichiometry;
	    boundary = FALSE;
	  }
	} else {
	  if (HasBoundaryConditionInSpeciesNode( species )) {
	    amount = GetConcentrationInSpeciesNode( species );
	    boundary = TRUE;
	  } else {
	    amount = GetConcentrationInSpeciesNode( species ) / stoichiometry;
	    boundary = FALSE;
	  }
	}
	ResetCurrentElement( Pedges );
	while( ( edge = GetNextEdge( Pedges ) ) != NULL ) {
	  speciesRef = GetSpeciesRefInIREdge( edge );
	  if (speciesRef) {
	    stoichiometry = GetCurrentRealValueInSymbol( speciesRef );
	  } else {
	    stoichiometry = GetStoichiometryInIREdge( edge );
	  }
	  species = GetSpeciesInIREdge( edge );
	  if (( convFactor = GetConversionFactorInSpeciesNode( species ) )!=NULL) {
	    stoichiometry *= GetCurrentRealValueInSymbol( convFactor );
	  }
	  if (HasOnlySubstanceUnitsInSpeciesNode( species )) {
	    if (boundary) {
	      if (HasBoundaryConditionInSpeciesNode( species )) {
		gsl_vector_set (f, j, amount + GetAmountInSpeciesNode( species ) - rec->fastCons[j]);
	      } else {
		gsl_vector_set (f, j, amount - rec->fastCons[j]);
	      }
	    } else {
	      if (HasBoundaryConditionInSpeciesNode( species )) {
		gsl_vector_set (f, j, GetAmountInSpeciesNode( species ) - rec->fastCons[j]);
	      } else {
		gsl_vector_set (f, j, amount + GetAmountInSpeciesNode( species ) / stoichiometry - rec->fastCons[j]);
	      }
	    }
	    j++;
	  } else {
	    if (boundary) {
	      if (HasBoundaryConditionInSpeciesNode( species )) {
		gsl_vector_set (f, j, amount + GetConcentrationInSpeciesNode( species ) - rec->fastCons[j]);
	      } else {
		gsl_vector_set (f, j, amount - rec->fastCons[j]);
	      }
	    } else {
	      if (HasBoundaryConditionInSpeciesNode( species )) {
		gsl_vector_set (f, j, GetConcentrationInSpeciesNode( species ) - rec->fastCons[j]);
	      } else {
		gsl_vector_set (f, j, amount + GetConcentrationInSpeciesNode( species ) / stoichiometry - rec->fastCons[j]);
	      }
	    }
	    j++;
	  }
	}
      }
      ResetCurrentElement( Pedges );
      if ( ( edge = GetNextEdge( Pedges ) ) != NULL ) {
	speciesRef = GetSpeciesRefInIREdge( edge );
	if (speciesRef) {
	  stoichiometry = GetCurrentRealValueInSymbol( speciesRef );
	} else {
	  stoichiometry = GetStoichiometryInIREdge( edge );
	}
	species = GetSpeciesInIREdge( edge );
	if (( convFactor = GetConversionFactorInSpeciesNode( species ) )!=NULL) {
	  stoichiometry *= GetCurrentRealValueInSymbol( convFactor );
	}
	if (HasOnlySubstanceUnitsInSpeciesNode( species )) {
	  if (HasBoundaryConditionInSpeciesNode( species )) {
	    amount = GetAmountInSpeciesNode( species );
	    boundary = TRUE;
	  } else {
	    amount = GetAmountInSpeciesNode( species ) / stoichiometry;
	    boundary = FALSE;
	  }
	} else {
	  if (HasBoundaryConditionInSpeciesNode( species )) {
	    amount = GetConcentrationInSpeciesNode( species );
	    boundary = TRUE;
	  } else {
	    amount = GetConcentrationInSpeciesNode( species ) / stoichiometry;
	    boundary = FALSE;
	  }
	}
	while( ( edge = GetNextEdge( Redges ) ) != NULL ) {
	  speciesRef = GetSpeciesRefInIREdge( edge );
	  if (speciesRef) {
	    stoichiometry = GetCurrentRealValueInSymbol( speciesRef );
	  } else {
	    stoichiometry = GetStoichiometryInIREdge( edge );
	  }
	  species = GetSpeciesInIREdge( edge );
	  if (( convFactor = GetConversionFactorInSpeciesNode( species ) )!=NULL) {
	    stoichiometry *= GetCurrentRealValueInSymbol( convFactor );
	  }
	  if (HasOnlySubstanceUnitsInSpeciesNode( species )) {
	    if (boundary) {
	      if (HasBoundaryConditionInSpeciesNode( species )) {
		gsl_vector_set (f, j, amount + GetAmountInSpeciesNode( species ) - rec->fastCons[j]);
	      } else {
		gsl_vector_set (f, j, amount - rec->fastCons[j]);
	      }
	    } else {
	      if (HasBoundaryConditionInSpeciesNode( species )) {
		gsl_vector_set (f, j, GetAmountInSpeciesNode( species ) - rec->fastCons[j]);
	      } else {
		gsl_vector_set (f, j, amount + GetAmountInSpeciesNode( species ) / stoichiometry - rec->fastCons[j]);
	      }
	    }
	    j++;
	  } else {
	    if (boundary) {
	      if (HasBoundaryConditionInSpeciesNode( species )) {
		gsl_vector_set (f, j, amount + GetConcentrationInSpeciesNode( species ) - rec->fastCons[j]);
	      } else {
		gsl_vector_set (f, j, amount - rec->fastCons[j]);
	      }
	    } else {
	      if (HasBoundaryConditionInSpeciesNode( species )) {
		gsl_vector_set (f, j, GetConcentrationInSpeciesNode( species ) - rec->fastCons[j]);
	      } else {
		gsl_vector_set (f, j, amount + GetConcentrationInSpeciesNode( species ) / stoichiometry - rec->fastCons[j]);
	      }
	    }
	    j++;
	  }
	}
      }
      amount = GetReactionRate( reaction );
      gsl_vector_set (f, j, amount);
      j++;
      break;
    }
  } 
       
  return GSL_SUCCESS;
}

static RET_VAL ExecuteFastReactions( EULER_SIMULATION_RECORD *rec ) {
  const gsl_multiroot_fsolver_type *T;
  gsl_multiroot_fsolver *s;
  SPECIES *species = NULL;
  REACTION *reaction = NULL;
  LINKED_LIST *Redges = NULL;
  LINKED_LIST *Pedges = NULL;
  IR_EDGE *edge = NULL;
  double stoichiometry = 0.0;
  int status;
  size_t i, j, iter = 0;
  double amount;
  const size_t n = rec->numberFastSpecies;
  BOOL boundary = FALSE;
  REB2SAC_SYMBOL *speciesRef = NULL;
  REB2SAC_SYMBOL *convFactor = NULL;

  j=0;
  for( i = 0; i < rec->reactionsSize; i++ ) {
    reaction = rec->reactionArray[i];
    if (IsReactionFastInReactionNode( reaction )) {
      amount = 0.0;
      Redges = GetReactantsInReactionNode( reaction );
      Pedges = GetProductsInReactionNode( reaction );
      ResetCurrentElement( Redges );
      if ( ( edge = GetNextEdge( Redges ) ) != NULL ) {
	speciesRef = GetSpeciesRefInIREdge( edge );
	if (speciesRef) {
	  stoichiometry = GetCurrentRealValueInSymbol( speciesRef );
	} else {
	  stoichiometry = GetStoichiometryInIREdge( edge );
	}
	species = GetSpeciesInIREdge( edge );
	if (( convFactor = GetConversionFactorInSpeciesNode( species ) )!=NULL) {
	  stoichiometry *= GetCurrentRealValueInSymbol( convFactor );
	}
	if (HasOnlySubstanceUnitsInSpeciesNode( species )) {
	  if (HasBoundaryConditionInSpeciesNode( species )) {
	    amount = GetAmountInSpeciesNode( species );
	    boundary = TRUE;
	  } else {
	    amount = GetAmountInSpeciesNode( species ) / stoichiometry;
	    boundary = FALSE;
	  }
	} else {
	  if (HasBoundaryConditionInSpeciesNode( species )) {
	    amount = GetConcentrationInSpeciesNode( species );
	    boundary = TRUE;
	  } else {
	    amount = GetConcentrationInSpeciesNode( species ) / stoichiometry;
	    boundary = FALSE;
	  }
	}
	ResetCurrentElement( Pedges );
	while( ( edge = GetNextEdge( Pedges ) ) != NULL ) {
	  speciesRef = GetSpeciesRefInIREdge( edge );
	  if (speciesRef) {
	    stoichiometry = GetCurrentRealValueInSymbol( speciesRef );
	  } else {
	    stoichiometry = GetStoichiometryInIREdge( edge );
	  }
	  species = GetSpeciesInIREdge( edge );
	  if (( convFactor = GetConversionFactorInSpeciesNode( species ) )!=NULL) {
	    stoichiometry *= GetCurrentRealValueInSymbol( convFactor );
	  }
	  if (HasOnlySubstanceUnitsInSpeciesNode( species )) {
	    if (boundary) {
	      rec->fastCons[j] = amount;
	      if (HasBoundaryConditionInSpeciesNode( species )) {
		rec->fastCons[j] += GetAmountInSpeciesNode( species );
	      }
	    } else {
	      if (HasBoundaryConditionInSpeciesNode( species )) {
		rec->fastCons[j] = GetAmountInSpeciesNode( species );
	      } else {
		rec->fastCons[j] = amount + GetAmountInSpeciesNode( species ) / stoichiometry;
	      }
	    }
	    j++;
	  } else {
	    if (boundary) {
	      rec->fastCons[j] = amount;
	      if (HasBoundaryConditionInSpeciesNode( species )) {
		rec->fastCons[j] += GetConcentrationInSpeciesNode( species );
	      }
	    } else {
	      if (HasBoundaryConditionInSpeciesNode( species )) {
		rec->fastCons[j] = GetConcentrationInSpeciesNode( species );
	      } else {
		rec->fastCons[j] = amount + GetConcentrationInSpeciesNode( species ) / stoichiometry;
	      }
	    }
	    j++;
	  }
	}
      }
      ResetCurrentElement( Pedges );
      if ( ( edge = GetNextEdge( Pedges ) ) != NULL ) {
	speciesRef = GetSpeciesRefInIREdge( edge );
	if (speciesRef) {
	  stoichiometry = GetCurrentRealValueInSymbol( speciesRef );
	} else {
	  stoichiometry = GetStoichiometryInIREdge( edge );
	}
	species = GetSpeciesInIREdge( edge );
	if (( convFactor = GetConversionFactorInSpeciesNode( species ) )!=NULL) {
	  stoichiometry *= GetCurrentRealValueInSymbol( convFactor );
	}
	if (HasOnlySubstanceUnitsInSpeciesNode( species )) {
	  if (HasBoundaryConditionInSpeciesNode( species )) {
	    amount = GetAmountInSpeciesNode( species );
	    boundary = TRUE;
	  } else {
	    amount = GetAmountInSpeciesNode( species ) / stoichiometry;
	    boundary = FALSE;
	  }
	} else {
	  if (HasBoundaryConditionInSpeciesNode( species )) {
	    amount = GetConcentrationInSpeciesNode( species );
	    boundary = TRUE;
	  } else {
	    amount = GetConcentrationInSpeciesNode( species ) / stoichiometry;
	    boundary = FALSE;
	  }
	}
	while( ( edge = GetNextEdge( Redges ) ) != NULL ) {
	  speciesRef = GetSpeciesRefInIREdge( edge );
	  if (speciesRef) {
	    stoichiometry = GetCurrentRealValueInSymbol( speciesRef );
	  } else {
	    stoichiometry = GetStoichiometryInIREdge( edge );
	  }
	  species = GetSpeciesInIREdge( edge );
	  if (( convFactor = GetConversionFactorInSpeciesNode( species ) )!=NULL) {
	    stoichiometry *= GetCurrentRealValueInSymbol( convFactor );
	  }
	  if (HasOnlySubstanceUnitsInSpeciesNode( species )) {
	    if (boundary) {
	      rec->fastCons[j] = amount;
	      if (HasBoundaryConditionInSpeciesNode( species )) {
		rec->fastCons[j] += GetAmountInSpeciesNode( species );
	      }
	    } else {
	      if (HasBoundaryConditionInSpeciesNode( species )) {
		rec->fastCons[j] = GetAmountInSpeciesNode( species );
	      } else {
		rec->fastCons[j] = amount + GetAmountInSpeciesNode( species ) / stoichiometry;
	      }
	    }
	    j++;
	  } else {
	    if (boundary) {
	      rec->fastCons[j] = amount;
	      if (HasBoundaryConditionInSpeciesNode( species )) {
		rec->fastCons[j] += GetConcentrationInSpeciesNode( species );
	      }
	    } else {
	      if (HasBoundaryConditionInSpeciesNode( species )) {
		rec->fastCons[j] = GetConcentrationInSpeciesNode( species );
	      } else {
		rec->fastCons[j] = amount + GetConcentrationInSpeciesNode( species ) / stoichiometry;
	      }
	    }
	    j++;
	  }
	}
      }
      break;
    }
  } 

  gsl_multiroot_function f = {&EulerfastReactions, n, rec};

  gsl_vector *x = gsl_vector_alloc (n);
  
  j = 0;
  for( i = 0; i < rec->speciesSize; i++ ) {
    species = rec->speciesArray[i];
    if (!HasBoundaryConditionInSpeciesNode( species ) && IsSpeciesNodeFast( species )) {
      if (HasOnlySubstanceUnitsInSpeciesNode( species )) {
	amount = GetAmountInSpeciesNode( species );
      } else {
	amount = GetConcentrationInSpeciesNode( species );
      }
      gsl_vector_set (x, j, amount);
      j++;
    } 
  }
     
  T = gsl_multiroot_fsolver_hybrids;
  s = gsl_multiroot_fsolver_alloc (T, n);
  gsl_multiroot_fsolver_set (s, &f, x);

  //printf("time = %g\n",rec->time);
  //Euler_print_state (iter, s, n);
     
  do {
      iter++;
      status = gsl_multiroot_fsolver_iterate (s);
      
      //Euler_print_state (iter, s, n);
      
      if (status)   /* check if solver is stuck */
	break;
      
      status = gsl_multiroot_test_residual (s->f, 1e-7);
  } while (status == GSL_CONTINUE && iter < 1000);
     
  //printf ("status = %s\n", gsl_strerror (status));

  j = 0;
  for( i = 0; i < rec->speciesSize; i++ ) {
    species = rec->speciesArray[i];
    if (!HasBoundaryConditionInSpeciesNode( species ) && IsSpeciesNodeFast( species )) {
      //if (IsSpeciesNodeFast( species )) {
      amount = gsl_vector_get (s->x, j);
      j++; 
      if (HasOnlySubstanceUnitsInSpeciesNode( species )) {
	SetAmountInSpeciesNode( species, amount );
      } else {
	SetConcentrationInSpeciesNode( species, amount );
      }
    }
  }
     
  gsl_multiroot_fsolver_free (s);
  gsl_vector_free (x);
  return 0;
}

static RET_VAL _UpdateSpeciesValues( EULER_SIMULATION_RECORD *rec ) {
    RET_VAL ret = SUCCESS;
    UINT32 i = 0;
    UINT32 j = 0;
    UINT32 speciesSize = rec->speciesSize;
    double stoichiometry = 0.0;
    double concentration = 0.0;
    double size = 0.0;
    double change = 0.0;
    double deltaTime = rec->timeStep;
    double rate = 0.0;
    IR_EDGE *edge = NULL;
    REACTION *reaction = NULL;
    SPECIES *species = NULL;
    SPECIES **speciesArray = rec->speciesArray;
    LINKED_LIST *edges = NULL;
    KINETIC_LAW_EVALUATER *evaluator = rec->evaluator;
    double nextEventTime;
    BOOL triggerEnabled;
    BYTE varType;
    REB2SAC_SYMBOL *speciesRef = NULL;
    REB2SAC_SYMBOL *convFactor = NULL;

    for (i = 0; i < rec->rulesSize; i++) {
      if ( GetRuleType( rec->ruleArray[i] ) == RULE_TYPE_RATE_ASSIGNMENT ) {
	change = rec->evaluator->EvaluateWithCurrentConcentrationsDeter( rec->evaluator,
								    (KINETIC_LAW*)GetMathInRule( rec->ruleArray[i] ) );
	varType = GetRuleVarType( rec->ruleArray[i] );
	j = GetRuleIndex( rec->ruleArray[i] );
	if ( varType == SPECIES_RULE ) {
	  SetRateInSpeciesNode( rec->speciesArray[j], change );
	  concentration = GetConcentrationInSpeciesNode( rec->speciesArray[j] );
	  concentration += (change * deltaTime);
	  SetRuleCurValue( rec->ruleArray[i], concentration );
	} else if ( varType == COMPARTMENT_RULE ) {
	  SetCurrentRateInCompartment( rec->compartmentArray[j], change );
	  concentration = GetCurrentSizeInCompartment( rec->compartmentArray[j] );
	  concentration += (change * deltaTime);
	  SetRuleCurValue( rec->ruleArray[i], concentration );
	} else {
	  SetCurrentRateInSymbol( rec->symbolArray[j], change );
	  concentration = GetCurrentRealValueInSymbol( rec->symbolArray[j] );
	  concentration += (change * deltaTime);
	  SetRuleCurValue( rec->ruleArray[i], concentration );
	}
      }
    }
    for (i = 0; i < rec->rulesSize; i++) {
      if ( GetRuleType( rec->ruleArray[i] ) == RULE_TYPE_RATE_ASSIGNMENT ) {
	concentration = GetRuleCurValue( rec->ruleArray[i] );
	varType = GetRuleVarType( rec->ruleArray[i] );
	j = GetRuleIndex( rec->ruleArray[i] );
	if ( varType == SPECIES_RULE ) {
	  SetConcentrationInSpeciesNode( rec->speciesArray[j], concentration );
	} else if ( varType == COMPARTMENT_RULE ) {
	  SetCurrentSizeInCompartment( rec->compartmentArray[j], concentration );
	} else {
	  SetCurrentRealValueInSymbol( rec->symbolArray[j], concentration );
	}
      }
    }

    for( i = 0; i < speciesSize; i++ ) {
        species = speciesArray[i];
	if (HasBoundaryConditionInSpeciesNode(species)) continue;
        change = 0.0;

        concentration = GetConcentrationInSpeciesNode( species );
        TRACE_2( "%s changes from %g", GetCharArrayOfString( GetSpeciesNodeName( species ) ),
            concentration );
        edges = GetReactantEdges( (IR_NODE*)species );
        ResetCurrentElement( edges );
        while( ( edge = GetNextEdge( edges ) ) != NULL ) {
	  speciesRef = GetSpeciesRefInIREdge( edge );
	  if (speciesRef) {
	    stoichiometry = GetCurrentRealValueInSymbol( speciesRef );
	  } else {
	    stoichiometry = GetStoichiometryInIREdge( edge );
	  }
	  reaction = GetReactionInIREdge( edge );
	  if (IsReactionFastInReactionNode( reaction )) continue;
	  rate = GetReactionRate( reaction );
	  change -= (stoichiometry * rate);
	  TRACE_2( "\tchanges from %s is %g", GetCharArrayOfString( GetReactionNodeName( reaction ) ),
		   -(stoichiometry * rate));
        }
        edges = GetProductEdges( (IR_NODE*)species );
        ResetCurrentElement( edges );
        while( ( edge = GetNextEdge( edges ) ) != NULL ) {
	  speciesRef = GetSpeciesRefInIREdge( edge );
	  if (speciesRef) {
	    stoichiometry = GetCurrentRealValueInSymbol( speciesRef );
	  } else {
	    stoichiometry = GetStoichiometryInIREdge( edge );
	  }
	  reaction = GetReactionInIREdge( edge );
	  if (IsReactionFastInReactionNode( reaction )) continue;
	  rate = GetReactionRate( reaction );
	  change += (stoichiometry * rate);
	  TRACE_2( "\tchanges from %s is %g", GetCharArrayOfString( GetReactionNodeName( reaction ) ),
		   (stoichiometry * rate));
        }
	if (( convFactor = GetConversionFactorInSpeciesNode( species ) )!=NULL) {
	  change *= GetCurrentRealValueInSymbol( convFactor );
	}
	size = GetCurrentSizeInCompartment( GetCompartmentInSpeciesNode( species ) );
        concentration += (change * deltaTime) / size;
        if( concentration < 0.0 ) {
            concentration = 0.0;
        }
        TRACE_2( "%s changes to %g", GetCharArrayOfString( GetSpeciesNodeName( species ) ),
            concentration );
        SetConcentrationInSpeciesNode( species, concentration );
    }


    for (j = 0; j < rec->symbolsSize; j++) {
      if ((strcmp(GetCharArrayOfString( GetSymbolID(rec->symbolArray[j]) ),"t")==0) ||
	  (strcmp(GetCharArrayOfString( GetSymbolID(rec->symbolArray[j]) ),"time")==0)) {
	SetCurrentRealValueInSymbol( rec->symbolArray[j], rec->time );
      }
    }

    ExecuteAssignments( rec );
    if (rec->algebraicRulesSize > 0) {
      EvaluateAlgebraicRules( rec );
    }
    if (rec->numberFastSpecies > 0) {
      ExecuteFastReactions( rec );
    }

    return ret;
}
