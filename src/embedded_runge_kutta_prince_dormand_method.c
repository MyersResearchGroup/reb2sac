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
#include "gsl/gsl_errno.h"
#include "gsl/gsl_matrix.h"
#include "gsl/gsl_odeiv.h"
#include "gsl/gsl_vector.h"
#include "gsl/gsl_multiroots.h"
#include "embedded_runge_kutta_prince_dormand_method.h"


static BOOL _IsModelConditionSatisfied( IR *ir );


static RET_VAL _InitializeRecord( EMBEDDED_RUNGE_KUTTA_PRINCE_DORMAND_SIMULATION_RECORD *rec, BACK_END_PROCESSOR *backend, IR *ir );
static RET_VAL _InitializeSimulation( EMBEDDED_RUNGE_KUTTA_PRINCE_DORMAND_SIMULATION_RECORD *rec, int runNum );
static RET_VAL _RunSimulation( EMBEDDED_RUNGE_KUTTA_PRINCE_DORMAND_SIMULATION_RECORD *rec );

static RET_VAL _CleanSimulation( EMBEDDED_RUNGE_KUTTA_PRINCE_DORMAND_SIMULATION_RECORD *rec );
static RET_VAL _CleanRecord( EMBEDDED_RUNGE_KUTTA_PRINCE_DORMAND_SIMULATION_RECORD *rec );

static RET_VAL _CalculateReactionRates( EMBEDDED_RUNGE_KUTTA_PRINCE_DORMAND_SIMULATION_RECORD *rec );
static RET_VAL _CalculateReactionRate( EMBEDDED_RUNGE_KUTTA_PRINCE_DORMAND_SIMULATION_RECORD *rec, REACTION *reaction );
static int _Update( double t, const double y[], double f[], EMBEDDED_RUNGE_KUTTA_PRINCE_DORMAND_SIMULATION_RECORD *rec );
static RET_VAL _Print( EMBEDDED_RUNGE_KUTTA_PRINCE_DORMAND_SIMULATION_RECORD *rec, double time );
static RET_VAL _PrintStatistics( EMBEDDED_RUNGE_KUTTA_PRINCE_DORMAND_SIMULATION_RECORD *rec, FILE *file);

static double fireEvents( EMBEDDED_RUNGE_KUTTA_PRINCE_DORMAND_SIMULATION_RECORD *rec, double time );
static void fireEvent( EVENT *event, EMBEDDED_RUNGE_KUTTA_PRINCE_DORMAND_SIMULATION_RECORD *rec );
static void ExecuteAssignments( EMBEDDED_RUNGE_KUTTA_PRINCE_DORMAND_SIMULATION_RECORD *rec );
static void SetEventAssignmentsNextValues( EVENT *event, EMBEDDED_RUNGE_KUTTA_PRINCE_DORMAND_SIMULATION_RECORD *rec );
static void SetEventAssignmentsNextValuesTime( EVENT *event, EMBEDDED_RUNGE_KUTTA_PRINCE_DORMAND_SIMULATION_RECORD *rec, double time );

DLLSCOPE RET_VAL STDCALL DoEmbeddedRungeKuttaPrinceDormandSimulation( BACK_END_PROCESSOR *backend, IR *ir ) {
    RET_VAL ret = SUCCESS;
    UINT i = 0;
    UINT runs = 1;
    char *namePrefix = NULL;
    static EMBEDDED_RUNGE_KUTTA_PRINCE_DORMAND_SIMULATION_RECORD rec;
    UINT timeout = 0;

    START_FUNCTION("DoEmbeddedRungeKuttaPrinceDormandSimulation");

    if( !_IsModelConditionSatisfied( ir ) ) {
        return ErrorReport( FAILING, "DoEmbeddedRungeKuttaPrinceDormandSimulation",
                            "Embedded Runge-Kutta-Prince-Dormand method cannot be applied to the model" );
    }

    if( IS_FAILED( ( ret = _InitializeRecord( &rec, backend, ir ) ) ) )  {
        return ErrorReport( ret, "DoEmbeddedRungeKuttaPrinceDormandSimulation", "initialization of the record failed" );
    }
    runs = rec.runs;
    for( i = 1; i <= runs; i++ ) {
      timeout = 0;
      SeedRandomNumberGenerators( rec.seed );
      rec.seed = GetNextUniformRandomNumber(0,RAND_MAX);
      do {
	SeedRandomNumberGenerators( rec.seed );
	if( IS_FAILED( ( ret = _InitializeSimulation( &rec, i ) ) ) ) {
	  return ErrorReport( ret, "DoEmbeddedRungeKuttaPrinceDormandSimulation", "initialization of the %i-th simulation failed", i );
	}
	timeout++;
      } while ( (ret == CHANGE) && (timeout <= (rec.speciesSize + rec.compartmentsSize + rec.symbolsSize)) );
      if (timeout > (rec.speciesSize + rec.compartmentsSize + rec.symbolsSize)) {
	return ErrorReport( ret, "DoEmbeddedRungeKuttaPrinceDormandSimulation", "Cycle detected in initial and rule assignments" );
      }
      if( IS_FAILED( ( ret = _RunSimulation( &rec ) ) ) ) {
        return ErrorReport( ret, "DoEmbeddedRungeKuttaPrinceDormandSimulation", "%i-th simulation failed at time %f", i, rec.time );
      }
      if( IS_FAILED( ( ret = _CleanSimulation( &rec ) ) ) ) {
        return ErrorReport( ret, "DoEmbeddedRungeKuttaPrinceDormandSimulation", "cleaning of the %i-th simulation failed", i );
      }
      printf("Run = %d\n",i);
      fflush(stdout);
    }

    END_FUNCTION("DoEmbeddedRungeKuttaPrinceDormandSimulation", SUCCESS );
    return ret;
}

DLLSCOPE RET_VAL STDCALL CloseEmbeddedRungeKuttaPrinceDormandSimulation( BACK_END_PROCESSOR *backend ) {
    RET_VAL ret = SUCCESS;
    EMBEDDED_RUNGE_KUTTA_PRINCE_DORMAND_SIMULATION_RECORD *rec = (EMBEDDED_RUNGE_KUTTA_PRINCE_DORMAND_SIMULATION_RECORD*)(backend->_internal1);

    START_FUNCTION("CloseEmbeddedRungeKuttaPrinceDormandSimulation");

    if( IS_FAILED( ( ret = _CleanRecord( rec ) ) ) )  {
        return ErrorReport( ret, "CloseEmbeddedRungeKuttaPrinceDormandSimulation", "cleaning of the record failed" );
    }

    END_FUNCTION("CloseEmbeddedRungeKuttaPrinceDormandSimulation",  SUCCESS );
    return ret;
}


static BOOL _IsModelConditionSatisfied( IR *ir ) {
    return TRUE;
}


static RET_VAL _InitializeRecord( EMBEDDED_RUNGE_KUTTA_PRINCE_DORMAND_SIMULATION_RECORD *rec, BACK_END_PROCESSOR *backend, IR *ir ) {
    RET_VAL ret = SUCCESS;
    UINT32 i = 0;
    UINT32 j = 0;
    double printInterval = 0.0;
    char buf[512];
    char *valueString = NULL;
    SPECIES *species = NULL;
    SPECIES **speciesArray = NULL;
    REACTION *reaction = NULL;
    REACTION **reactions = NULL;
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
    COMPARTMENT *compartment = NULL;
    COMPARTMENT **compartmentArray = NULL;
    COMPARTMENT_MANAGER *compartmentManager;
    REB2SAC_SYMBOL *symbol = NULL;
    REB2SAC_SYMBOL **symbolArray = NULL;
    REB2SAC_SYMTAB *symTab;
    COMPILER_RECORD_T *compRec = backend->record;
    LINKED_LIST *list = NULL;
    REB2SAC_PROPERTIES *properties = NULL;

#if GET_SEED_FROM_COMMAND_LINE
    PROPERTIES *options = NULL;
#endif

    list = ir->GetListOfReactionNodes( ir );
    rec->reactionsSize = GetLinkedListSize( list );
    if (rec->reactionsSize!=0) {
      if( ( reactions = (REACTION**)MALLOC( rec->reactionsSize * sizeof(REACTION*) ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not allocate memory for reaction array" );
      }
      i = 0;
      ResetCurrentElement( list );
      while( ( reaction = (REACTION*)GetNextFromLinkedList( list ) ) != NULL ) {
        reactions[i] = reaction;
        i++;
      }
    }
    rec->reactionArray = reactions;

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
    ResetCurrentElement( list );
    while( ( rule = (RULE*)GetNextFromLinkedList( list ) ) != NULL ) {
        ruleArray[i] = rule;
        i++;
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
    ResetCurrentElement( list );
    while( ( species = (SPECIES*)GetNextFromLinkedList( list ) ) != NULL ) {
        speciesArray[i] = species;
        i++;
    }
    rec->speciesArray = speciesArray;

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

    if( ( rec->concentrations = (double*)MALLOC( (rec->symbolsSize + rec->compartmentsSize + rec->speciesSize) * sizeof( double ) ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not allocate memory for concentrations" );
    }

    if( ( rec->evaluator = CreateKineticLawEvaluater() ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not create evaluator" );
    }

    if( ( rec->findNextTime = CreateKineticLawFind_Next_Time() ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not create find next time" );
    }

    properties = compRec->properties;
    if( ( valueString = properties->GetProperty( properties, ODE_SIMULATION_ABSOLUTE_ERROR ) ) == NULL ) {
      rec->absoluteError = DEFAULT_ODE_SIMULATION_ABSOLUTE_ERROR;
    }
    else {
      if( IS_FAILED( ( ret = StrToFloat( &(rec->absoluteError), valueString ) ) ) ) {
	rec->absoluteError = DEFAULT_ODE_SIMULATION_ABSOLUTE_ERROR;
      }
    }

    if( ( valueString = properties->GetProperty( properties, ODE_SIMULATION_TIME_STEP ) ) == NULL ) {
        rec->timeStep = DEFAULT_ODE_SIMULATION_TIME_STEP;
    }
    else {
      if (strcmp(valueString,"inf")==0) {
	rec->timeStep = DBL_MAX;
      } else if( IS_FAILED( ( ret = StrToFloat( &(rec->timeStep), valueString ) ) ) ) {
	rec->timeStep = DEFAULT_ODE_SIMULATION_TIME_STEP;
      }
    }

    if( ( valueString = properties->GetProperty( properties, ODE_SIMULATION_TIME_LIMIT ) ) == NULL ) {
        rec->timeLimit = DEFAULT_ODE_SIMULATION_TIME_LIMIT_VALUE;
    }
    else {
        if( IS_FAILED( ( ret = StrToFloat( &(rec->timeLimit), valueString ) ) ) ) {
            rec->timeLimit = DEFAULT_ODE_SIMULATION_TIME_LIMIT_VALUE;
        }
    }

    if( ( valueString = properties->GetProperty( properties, ODE_SIMULATION_PRINT_INTERVAL ) ) == NULL ) {
      if( ( valueString = properties->GetProperty( properties, ODE_SIMULATION_NUMBER_STEPS ) ) == NULL ) {
        rec->numberSteps = DEFAULT_ODE_SIMULATION_NUMBER_STEPS_VALUE;
      } else {
        if( IS_FAILED( ( ret = StrToUINT32( &(rec->numberSteps), valueString ) ) ) ) {
            rec->numberSteps = DEFAULT_ODE_SIMULATION_NUMBER_STEPS_VALUE;
        } 
      }
    }
    else {
        if( IS_FAILED( ( ret = StrToFloat( &(printInterval), valueString ) ) ) ) {
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
        if( IS_FAILED( ( ret = StrToUINT32( &(rec->seed), valueString ) ) ) ) {
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

static RET_VAL _InitializeSimulation( EMBEDDED_RUNGE_KUTTA_PRINCE_DORMAND_SIMULATION_RECORD *rec, int runNum ) {
    RET_VAL ret = SUCCESS;
    char filenameStem[512];
    double concentration = 0.0;
    double oldSize = 0.0;
    double compSize = 0.0;
    double param = 0.0;
    double *concentrations = rec->concentrations;
    UINT32 i = 0;
    UINT32 size = 0;
    SPECIES *species = NULL;
    SPECIES **speciesArray = rec->speciesArray;
    COMPARTMENT *compartment = NULL;
    COMPARTMENT **compartmentArray = rec->compartmentArray;
    REB2SAC_SYMBOL *symbol = NULL;
    REB2SAC_SYMBOL **symbolArray = rec->symbolArray;
    REACTION *reaction = NULL;
    REACTION **reactionArray = rec->reactionArray;
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
	    oldSize = GetSizeInCompartment( compartment );
	    SetSizeInCompartment( compartment, compSize );
	    for (i = 0; i <  rec->speciesSize; i++) {
	      species = speciesArray[i];
	      if (GetCompartmentInSpeciesNode( species ) == compartment) {
		if( !IsInitialQuantityInAmountInSpeciesNode( species ) ) {
		  concentration = GetInitialAmountInSpeciesNode( species );
		  SetInitialAmountInSpeciesNode( species, concentration / oldSize * compSize );
		}
	      }
	    }
	    change = TRUE;
	  }
	  FreeKineticLaw( &(law) );
	}
        if( IS_FAILED( ( ret = SetCurrentSizeInCompartment( compartment, compSize ) ) ) ) {
            return ret;
        }
	concentrations[rec->speciesSize + i] = compSize;
    }
    size = rec->speciesSize;
    for( i = 0; i < size; i++ ) {
        species = speciesArray[i];
	if ( (law = (KINETIC_LAW*)GetInitialAssignmentInSpeciesNode( species )) == NULL ) {
	  if( IsInitialQuantityInAmountInSpeciesNode( species ) ) {
	    concentration = GetInitialAmountInSpeciesNode( species );
	    SetAmountInSpeciesNode( species, concentration );
	  }
	  else {
	    concentration = GetInitialConcentrationInSpeciesNode( species );
	    SetConcentrationInSpeciesNode( species, concentration );
	    concentration = GetAmountInSpeciesNode( species );
	  }
	} else {
	  law = CloneKineticLaw( law );
	  SimplifyInitialAssignment(law);
	  if (law->valueType == KINETIC_LAW_VALUE_TYPE_REAL) {
	    concentration = GetRealValueFromKineticLaw(law);
	  } else if (law->valueType == KINETIC_LAW_VALUE_TYPE_INT) {
	    concentration = (double)GetIntValueFromKineticLaw(law);
	  }
	  if( HasOnlySubstanceUnitsInSpeciesNode( species ) ) {
	    if (GetAmountInSpeciesNode( species ) != concentration) {
	      SetAmountInSpeciesNode( species, concentration );
	      change = TRUE;
	    }
	    SetInitialAmountInSpeciesNode( species, concentration );
	  }
	  else {
	    compSize = GetCurrentSizeInCompartment( GetCompartmentInSpeciesNode( species ) ); 
	    if (compSize == -1.0) {
	      compSize = 1.0;
	    }
	    if (GetAmountInSpeciesNode( species ) != concentration * compSize) {
	      SetAmountInSpeciesNode( species, concentration * compSize );
	      change = TRUE;
	    }
	    SetInitialConcentrationInSpeciesNode( species, concentration );
	    concentration = concentration * compSize;
	  }
	  FreeKineticLaw( &(law) );
	}
/*         if( IS_FAILED( ( ret = SetConcentrationInSpeciesNode( species, concentration ) ) ) ) { */
/*             return ret; */
/*         } */
        concentrations[i] = concentration;
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
	concentrations[rec->compartmentsSize + rec->speciesSize + i] = param;
    }
    for (i = 0; i < rec->eventsSize; i++) {
      /* SetTriggerEnabledInEvent( rec->eventArray[i], FALSE ); */
      /* Use the line below to support true SBML semantics, i.e., nothing can be trigger at t=0 */
      if (rec->evaluator->EvaluateWithCurrentAmounts( rec->evaluator,
							     (KINETIC_LAW*)GetTriggerInEvent( rec->eventArray[i] ) )) {
    	  SetTriggerEnabledInEvent( rec->eventArray[i], TRUE );
      } else {
    	  SetTriggerEnabledInEvent( rec->eventArray[i], FALSE );
      }
    }

    if (change)
      return CHANGE;
    return ret;
}

static RET_VAL _RunSimulation( EMBEDDED_RUNGE_KUTTA_PRINCE_DORMAND_SIMULATION_RECORD *rec ) {
    RET_VAL ret = SUCCESS;
    int status = GSL_SUCCESS;
    double h = EMBEDDED_RUNGE_KUTTA_PRINCE_DORMAND_H;
    double *y = rec->concentrations;
    double nextPrintTime = rec->time;
    double time = rec->time;
    double timeStep = rec->timeStep;
    double nextEventTime;
    double maxTime;
    int curStep = 0;
    double numberSteps = rec->numberSteps;
    SIMULATION_PRINTER *printer = NULL;
    SIMULATION_RUN_TERMINATION_DECIDER *decider = NULL;
    const gsl_odeiv_step_type *stepType = gsl_odeiv_step_rk8pd;
    gsl_odeiv_step *step = NULL;
    gsl_odeiv_control *control = NULL;
    gsl_odeiv_evolve *evolve = NULL;
    int size = rec->speciesSize + rec->compartmentsSize + rec->symbolsSize;
    gsl_odeiv_system system =
    {
        (int(*)(double , const double [], double [], void*))_Update,
        NULL,
        size,
        rec
    };

    step = gsl_odeiv_step_alloc( stepType, size );
    control = gsl_odeiv_control_y_new( rec->absoluteError, EMBEDDED_RUNGE_KUTTA_PRINCE_DORMAND_LOCAL_ERROR );
    evolve = gsl_odeiv_evolve_alloc( size );

    printer = rec->printer;
    decider = rec->decider;
    while( !(decider->IsTerminationConditionMet( decider, NULL, time )) ) {
      if( IS_FAILED( ( ret = _Print( rec, time ) ) ) ) {
	return ret;
      }
      curStep++;
      nextPrintTime = (curStep/numberSteps) * rec->timeLimit;
      while( time < nextPrintTime ) {
	if ((timeStep == DBL_MAX) || (maxTime + timeStep > nextPrintTime)) {
	  maxTime = nextPrintTime;
	} else {
	  maxTime = maxTime + timeStep;
	}
	nextEventTime = fireEvents( rec, time );
	if (nextEventTime==-2.0) {
	  return FAILING;
	}
	if ((nextEventTime != -1) && (nextEventTime < maxTime)) {
	  maxTime = nextEventTime;
	}
	status = gsl_odeiv_evolve_apply( evolve, control, step,
					 &system, &time, maxTime,
					 &h, y );
	if( status != GSL_SUCCESS ) {
	  return FAILING;
	}
      }
      if (time > 0.0) printf("Time = %g\n",time);
      fflush(stdout);
    }
    if( IS_FAILED( ( ret = _Print( rec, time ) ) ) ) {
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

static RET_VAL _CleanSimulation( EMBEDDED_RUNGE_KUTTA_PRINCE_DORMAND_SIMULATION_RECORD *rec ) {
    RET_VAL ret = SUCCESS;
    return ret;
}

static RET_VAL _CleanRecord( EMBEDDED_RUNGE_KUTTA_PRINCE_DORMAND_SIMULATION_RECORD *rec ) {
    RET_VAL ret = SUCCESS;
    char filename[512];
    FILE *file = NULL;
    SIMULATION_PRINTER *printer = rec->printer;
    SIMULATION_RUN_TERMINATION_DECIDER *decider = rec->decider;

    sprintf( filename, "%s%cstatistics.txt", rec->outDir, FILE_SEPARATOR );
    if( ( file = fopen( filename, "w" ) ) == NULL ) {
        return ErrorReport( FAILING, "_CleanRecord", "could not create a statistics file" );
    }
    if( IS_FAILED( ( ret = _PrintStatistics( rec, file ) ) ) ) {
        return ret;
    }
    fclose( file );

    if( rec->evaluator != NULL ) {
        FreeKineticLawEvaluater( &(rec->evaluator) );
    }
    if( rec->reactionArray != NULL ) {
        FREE( rec->reactionArray );
    }
    if( rec->speciesArray != NULL ) {
        FREE( rec->speciesArray );
    }
    if( rec->concentrations != NULL ) {
        FREE( rec->concentrations );
    }

    printer->Destroy( printer );
    decider->Destroy( decider );

    return ret;
}

static RET_VAL _PrintStatistics(EMBEDDED_RUNGE_KUTTA_PRINCE_DORMAND_SIMULATION_RECORD *rec, FILE *file) {
	RET_VAL ret = SUCCESS;
	double stoichiometry = 0;
	UINT32 i = 0;
	UINT32 j = 0;
	UINT32 reactionsSize = rec->reactionsSize;
	UINT32 speciesSize = rec->speciesSize;
	REACTION *reaction = NULL;
	REACTION **reactionArray = rec->reactionArray;
	SPECIES *species = NULL;
	SPECIES **speciesArray = rec->speciesArray;
	REB2SAC_SYMBOL *speciesRef = NULL;
	REB2SAC_SYMBOL *convFactor = NULL;
	IR_EDGE *edge = NULL;
	LINKED_LIST *edges = NULL;
	gsl_matrix *delta_matrix = gsl_matrix_alloc(speciesSize, reactionsSize);
	gsl_matrix *reactant_matrix = gsl_matrix_alloc(speciesSize, reactionsSize);

	for (i = 0; i < reactionsSize; i++) {
		for (j = 0; j < speciesSize; j++) {
			gsl_matrix_set(delta_matrix, j, i, 0);
			gsl_matrix_set(reactant_matrix, j, i, 0);
		}
	}

	fprintf( file, "Reaction Rate Array:" NEW_LINE);

	for (i = 0; i < reactionsSize; i++) {
		reaction = reactionArray[i];
		fprintf( file, "%f ", GetReactionRate(reaction));
		edges = GetReactantEdges((IR_NODE*) reaction);
		ResetCurrentElement(edges);
		while ((edge = GetNextEdge(edges)) != NULL) {
			speciesRef = GetSpeciesRefInIREdge(edge);
			if (speciesRef) {
				stoichiometry = GetCurrentRealValueInSymbol(speciesRef);
			} else {
				stoichiometry = GetStoichiometryInIREdge(edge);
			}
			species = GetSpeciesInIREdge(edge);
			if ((convFactor = GetConversionFactorInSpeciesNode(species)) != NULL) {
				stoichiometry *= GetCurrentRealValueInSymbol(convFactor);
			}
			if (HasBoundaryConditionInSpeciesNode(species))
				continue;
			for (j = 0; j < speciesSize; j++) {
				if (species == speciesArray[j]) {
					gsl_matrix_set(delta_matrix, j, i, gsl_matrix_get(delta_matrix, j, i) - stoichiometry);
					gsl_matrix_set(reactant_matrix, j, i, gsl_matrix_get(reactant_matrix, j, i)+stoichiometry);
				}
			}
		}
		edges = GetProductEdges((IR_NODE*) reaction);
		ResetCurrentElement(edges);
		while ((edge = GetNextEdge(edges)) != NULL) {
			speciesRef = GetSpeciesRefInIREdge(edge);
			if (speciesRef) {
				stoichiometry = GetCurrentRealValueInSymbol(speciesRef);
			} else {
				stoichiometry = GetStoichiometryInIREdge(edge);
			}
			species = GetSpeciesInIREdge(edge);
			if ((convFactor = GetConversionFactorInSpeciesNode(species)) != NULL) {
				stoichiometry *= GetCurrentRealValueInSymbol(convFactor);
			}
			if (HasBoundaryConditionInSpeciesNode(species))
				continue;
			for (j = 0; j < speciesSize; j++) {
				if (species == speciesArray[j]) {
					gsl_matrix_set(delta_matrix, j, i, gsl_matrix_get(delta_matrix, j, i) + stoichiometry);
				}
			}
		}
	}
	fprintf( file, NEW_LINE);
	fprintf( file, NEW_LINE);

	fprintf( file, "Reactant Matrix:" NEW_LINE);
	for (i = 0; i < reactionsSize; i++) {
		for (j = 0; j < speciesSize; j++) {
			fprintf( file, "%f ", gsl_matrix_get(reactant_matrix, j, i));
		}
		fprintf( file, NEW_LINE);
	}

	fprintf( file, NEW_LINE);

	fprintf( file, "Delta Matrix:" NEW_LINE);
	for (i = 0; i < reactionsSize; i++) {
		for (j = 0; j < speciesSize; j++) {
			fprintf(file, "%f ", gsl_matrix_get(delta_matrix, j, i));
		}
		fprintf(file, NEW_LINE);
	}

	return ret;
}

static RET_VAL _Print( EMBEDDED_RUNGE_KUTTA_PRINCE_DORMAND_SIMULATION_RECORD *rec, double time ) {
    RET_VAL ret = SUCCESS;
    SIMULATION_PRINTER *printer = rec->printer;

    if( IS_FAILED( ( ret = printer->PrintValues( printer, time ) ) ) ) {
        return ret;
    }
    return ret;
}



static RET_VAL _CalculateReactionRates( EMBEDDED_RUNGE_KUTTA_PRINCE_DORMAND_SIMULATION_RECORD *rec ) {
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


static RET_VAL _CalculateReactionRate( EMBEDDED_RUNGE_KUTTA_PRINCE_DORMAND_SIMULATION_RECORD *rec, REACTION *reaction ) {
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

static double fireEvents( EMBEDDED_RUNGE_KUTTA_PRINCE_DORMAND_SIMULATION_RECORD *rec, double time ) {
    int i;
    double nextEventTime;
    BOOL triggerEnabled;
    double deltaTime;
    BOOL eventFired = FALSE;
    double firstEventTime = -1.0;

    rec->time = time;
    do {
      eventFired = FALSE;
      for (i = 0; i < rec->eventsSize; i++) {
	nextEventTime = GetNextEventTimeInEvent( rec->eventArray[i] );
	triggerEnabled = GetTriggerEnabledInEvent( rec->eventArray[i] );
	if (nextEventTime != -1.0) {
	  if  (time >= nextEventTime) {
	    if (!GetUseValuesFromTriggerTime( rec->eventArray[i] )) {
	      SetEventAssignmentsNextValues( rec->eventArray[i], rec ); 
	    }
	    fireEvent( rec->eventArray[i], rec );
	    SetNextEventTimeInEvent( rec->eventArray[i], -1.0 );
	    eventFired = TRUE;
	  } else {
	    if ((firstEventTime == -1.0) || (nextEventTime < firstEventTime)) {
	      firstEventTime = nextEventTime;
	    }
	  }
	}
	nextEventTime = time + 
	  rec->findNextTime->FindNextTimeWithCurrentAmounts( rec->findNextTime,
							     (KINETIC_LAW*)GetTriggerInEvent( rec->eventArray[i] ));
	if ((firstEventTime == -1.0) || (nextEventTime < firstEventTime)) {
	  firstEventTime = nextEventTime;
	}
	if (!triggerEnabled) {
	  if (rec->evaluator->EvaluateWithCurrentConcentrationsDeter( rec->evaluator,
								 (KINETIC_LAW*)GetTriggerInEvent( rec->eventArray[i] ) )) {
	    SetTriggerEnabledInEvent( rec->eventArray[i], TRUE );
	    if (GetDelayInEvent( rec->eventArray[i] )==NULL) {
	      deltaTime = 0;
	    }
	    else {
	      deltaTime = rec->evaluator->EvaluateWithCurrentConcentrationsDeter( rec->evaluator,
									     (KINETIC_LAW*)GetDelayInEvent( rec->eventArray[i] ) );
	    }
	    if (deltaTime > 0) {
	      SetNextEventTimeInEvent( rec->eventArray[i], time + deltaTime );
	      if (GetUseValuesFromTriggerTime( rec->eventArray[i] )) {
		SetEventAssignmentsNextValuesTime( rec->eventArray[i], rec, time + deltaTime ); 
	      }
	      if ((firstEventTime == -1.0) || (time + deltaTime < firstEventTime)) {
		firstEventTime = time + deltaTime;
	      }
	    } else if (deltaTime == 0) {
	      SetEventAssignmentsNextValues( rec->eventArray[i], rec ); 
	      fireEvent( rec->eventArray[i], rec );
	      eventFired = TRUE;
	    } else {
	      ErrorReport( FAILING, "_Update", "delay for event evaluates to a negative number" );
	      return -2;
	    }
	  }
	} else {
	  if (!rec->evaluator->EvaluateWithCurrentConcentrationsDeter( rec->evaluator,
								  (KINETIC_LAW*)GetTriggerInEvent( rec->eventArray[i] ) )) {
	    SetTriggerEnabledInEvent( rec->eventArray[i], FALSE );
	  }
	}
      }
      if (eventFired) {
	ExecuteAssignments( rec );
      }
    } while (eventFired);
    return firstEventTime;
}

static void SetEventAssignmentsNextValues( EVENT *event, EMBEDDED_RUNGE_KUTTA_PRINCE_DORMAND_SIMULATION_RECORD *rec ) {
  LINKED_LIST *list = NULL;
  EVENT_ASSIGNMENT *eventAssignment;
  double concentration = 0.0;
  UINT j;
  BYTE varType;

  list = GetEventAssignments( event );
  ResetCurrentElement( list );
  while( ( eventAssignment = (EVENT_ASSIGNMENT*)GetNextFromLinkedList( list ) ) != NULL ) {
    concentration = rec->evaluator->EvaluateWithCurrentConcentrations( rec->evaluator, eventAssignment->assignment );
    SetEventAssignmentNextValue( eventAssignment, concentration );
  }
}

static void SetEventAssignmentsNextValuesTime( EVENT *event, EMBEDDED_RUNGE_KUTTA_PRINCE_DORMAND_SIMULATION_RECORD *rec, double time ) {
  LINKED_LIST *list = NULL;
  EVENT_ASSIGNMENT *eventAssignment;
  double concentration = 0.0;
  UINT j;
  BYTE varType;

  list = GetEventAssignments( event );
  ResetCurrentElement( list );
  while( ( eventAssignment = (EVENT_ASSIGNMENT*)GetNextFromLinkedList( list ) ) != NULL ) {
    concentration = rec->evaluator->EvaluateWithCurrentConcentrations( rec->evaluator, eventAssignment->assignment );
    SetEventAssignmentNextValueTime( eventAssignment, concentration, time );
  }
}

static void fireEvent( EVENT *event, EMBEDDED_RUNGE_KUTTA_PRINCE_DORMAND_SIMULATION_RECORD *rec ) {
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
    if (!GetUseValuesFromTriggerTime( event )) {
      concentration = GetEventAssignmentNextValue( eventAssignment );
    } else {
      concentration = GetEventAssignmentNextValueTime( eventAssignment, rec->time );
    }
    //printf("conc = %g\n",amount);
    if ( varType == SPECIES_EVENT_ASSIGNMENT ) {
	if (HasOnlySubstanceUnitsInSpeciesNode( rec->speciesArray[j] )) {
	  SetAmountInSpeciesNode( rec->speciesArray[j], concentration );
	  rec->concentrations[j] = concentration;
	} else {
	  SetConcentrationInSpeciesNode( rec->speciesArray[j], concentration );
	  rec->concentrations[j] = GetAmountInSpeciesNode(rec->speciesArray[j]);
	}
    } else if ( varType == COMPARTMENT_EVENT_ASSIGNMENT ) {
      SetCurrentSizeInCompartment( rec->compartmentArray[j], concentration );
      rec->concentrations[rec->speciesSize + j] = concentration;
    } else {
      SetCurrentRealValueInSymbol( rec->symbolArray[j], concentration );
      rec->concentrations[rec->speciesSize + rec->compartmentsSize + j] = concentration;
    }
  }
}

/* Update values using assignments rules */
static void ExecuteAssignments( EMBEDDED_RUNGE_KUTTA_PRINCE_DORMAND_SIMULATION_RECORD *rec ) {
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
	  rec->concentrations[j] = concentration;
	} else {
	  SetConcentrationInSpeciesNode( rec->speciesArray[j], concentration );
	  rec->concentrations[j] = GetAmountInSpeciesNode(rec->speciesArray[j]);
	}
      } else if ( varType == COMPARTMENT_RULE ) {
	SetCurrentSizeInCompartment( rec->compartmentArray[j], concentration );
	rec->concentrations[rec->speciesSize + j] = concentration;
      } else {
	SetCurrentRealValueInSymbol( rec->symbolArray[j], concentration );
	rec->concentrations[rec->speciesSize + rec->compartmentsSize + j] = concentration;
      }
    }
  }
}

static int _Update( double t, const double y[], double f[], EMBEDDED_RUNGE_KUTTA_PRINCE_DORMAND_SIMULATION_RECORD *rec ) {
    RET_VAL ret = SUCCESS;
    UINT32 i = 0;
    UINT32 j = 0;
    UINT32 speciesSize = rec->speciesSize;
    UINT32 compartmentsSize = rec->compartmentsSize;
    double stoichiometry = 0.0;
    double concentration = 0.0;
    double size = 0.0;
    double deltaTime = 0.0;
    double rate = 0.0;
    IR_EDGE *edge = NULL;
    REACTION *reaction = NULL;
    COMPARTMENT *compartment = NULL;
    COMPARTMENT **compartmentsArray = rec->compartmentArray;
    UINT32 symbolsSize = rec->symbolsSize;
    REB2SAC_SYMBOL *symbol = NULL;
    REB2SAC_SYMBOL **symbolArray = rec->symbolArray;
    SPECIES *species = NULL;
    SPECIES **speciesArray = rec->speciesArray;
    LINKED_LIST *edges = NULL;
    KINETIC_LAW_EVALUATER *evaluator = rec->evaluator;
    double nextEventTime;
    BOOL triggerEnabled;
    BYTE varType;

    /* Update values from y[] */
    for( i = 0; i < speciesSize; i++ ) {
        species = speciesArray[i];
	if (f[i] != 0.0) {
	  if( IS_FAILED( ( ret = SetAmountInSpeciesNode( species, y[i] ) ) ) ) {
	    return GSL_FAILURE;
	  }
	}
	f[i] = 0.0;
    }
    for( i = 0; i < compartmentsSize; i++ ) {
        compartment = compartmentsArray[i];
	if (f[speciesSize + i] != 0.0) {
	  if( IS_FAILED( ( ret = SetCurrentSizeInCompartment( compartment, y[speciesSize + i] ) ) ) ) {
            return GSL_FAILURE;
	  }
	}
	f[speciesSize + i] = 0.0;
    }
    for( i = 0; i < symbolsSize; i++ ) {
        symbol = symbolArray[i];
	if (f[speciesSize + compartmentsSize + i] != 0.0) {
	  if( IS_FAILED( ( ret = SetCurrentRealValueInSymbol( symbol, y[speciesSize + compartmentsSize + i] ) ) ) ) {
            return GSL_FAILURE;
	  }
	}
	f[speciesSize + compartmentsSize + i] = 0.0;
	if ((strcmp(GetCharArrayOfString( GetSymbolID(symbol) ),"time")==0) ||
	    (strcmp(GetCharArrayOfString( GetSymbolID(symbol) ),"t")==0)) {
	  f[speciesSize + compartmentsSize + i] = 1.0;
	}
    }

    ExecuteAssignments( rec );

    /* Update rates using rate rules */
    for (i = 0; i < rec->rulesSize; i++) {
      if (GetRuleType( rec->ruleArray[i] ) == RULE_TYPE_RATE_ASSIGNMENT ) {
	rate = rec->evaluator->EvaluateWithCurrentConcentrationsDeter( rec->evaluator,
								  (KINETIC_LAW*)GetMathInRule( rec->ruleArray[i] ) );
	varType = GetRuleVarType( rec->ruleArray[i] );
	j = GetRuleIndex( rec->ruleArray[i] );
	if ( varType == SPECIES_RULE ) {
	  if (HasOnlySubstanceUnitsInSpeciesNode( speciesArray[j] )) {
	    f[j] = rate;
	  } else {
	    size = GetCurrentSizeInCompartment( GetCompartmentInSpeciesNode( speciesArray[j] ) );
	    f[j] = rate * size;
	  }
	} else if ( varType == COMPARTMENT_RULE ) {
	  f[speciesSize + j] = rate;
	} else {
	  f[speciesSize + compartmentsSize + j] = rate;
	}
      }
    }

    /* Update rates using kinetic laws from the reactions */
    if( IS_FAILED( ( ret = _CalculateReactionRates( rec ) ) ) ) {
        return GSL_FAILURE;
    }
    for( i = 0; i < speciesSize; i++ ) {
        species = speciesArray[i];
	if (HasBoundaryConditionInSpeciesNode(species)) continue;
	size = GetCurrentSizeInCompartment( GetCompartmentInSpeciesNode( species ) );
        TRACE_2( "%s changes from %g", GetCharArrayOfString( GetSpeciesNodeName( species ) ),
            concentration );
        edges = GetReactantEdges( (IR_NODE*)species );
        ResetCurrentElement( edges );
        while( ( edge = GetNextEdge( edges ) ) != NULL ) {
            stoichiometry = GetStoichiometryInIREdge( edge );
            reaction = GetReactionInIREdge( edge );
            rate = GetReactionRate( reaction );
	    f[i] -= (stoichiometry * rate);
            TRACE_2( "\tchanges from %s is %g", GetCharArrayOfString( GetReactionNodeName( reaction ) ),
               -(stoichiometry * rate));
        }
        edges = GetProductEdges( (IR_NODE*)species );
        ResetCurrentElement( edges );
        while( ( edge = GetNextEdge( edges ) ) != NULL ) {
            stoichiometry = GetStoichiometryInIREdge( edge );
            reaction = GetReactionInIREdge( edge );
            rate = GetReactionRate( reaction );
	    f[i] += (stoichiometry * rate);
            TRACE_2( "\tchanges from %s is %g", GetCharArrayOfString( GetReactionNodeName( reaction ) ),
               (stoichiometry * rate));
        }
        TRACE_2( "change of %s is %g\n", GetCharArrayOfString( GetSpeciesNodeName( species ) ), f[i] );
    }
    return GSL_SUCCESS;
}
