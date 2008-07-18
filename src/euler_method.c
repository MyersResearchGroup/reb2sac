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
    UINT32 size = 0;
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
	   GetRuleType( rec->ruleArray[i] ) == RULE_TYPE_RATE ) {
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
    
    properties = compRec->properties;
    if( ( valueString = properties->GetProperty( properties, ODE_SIMULATION_TIME_LIMIT ) ) == NULL ) {
        rec->timeLimit = DEFAULT_ODE_SIMULATION_TIME_LIMIT_VALUE;
    }
    else {
        if( IS_FAILED( ( ret = StrToFloat( &(rec->timeLimit), valueString ) ) ) ) {
            rec->timeLimit = DEFAULT_ODE_SIMULATION_TIME_LIMIT_VALUE;
        }
    }    
    
    if( ( valueString = properties->GetProperty( properties, ODE_SIMULATION_PRINT_INTERVAL ) ) == NULL ) {
        rec->printInterval = DEFAULT_ODE_SIMULATION_PRINT_INTERVAL_VALUE;
    }
    else {
        if( IS_FAILED( ( ret = StrToFloat( &(rec->printInterval), valueString ) ) ) ) {
            rec->printInterval = DEFAULT_ODE_SIMULATION_PRINT_INTERVAL_VALUE;
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
    size = rec->speciesSize;
    for( i = 0; i < size; i++ ) {
        species = speciesArray[i];
	if ( (law = (KINETIC_LAW*)GetInitialAssignmentInSpeciesNode( species )) == NULL ) {
	  if( IsInitialQuantityInAmountInSpeciesNode( species ) ) {
	    concentration = GetInitialAmountInSpeciesNode( species );
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
      SetTriggerEnabledInEvent( rec->eventArray[i], FALSE );
      /* Use the line below to support true SBML semantics, i.e., nothing can be trigger at t=0 */
      /* 
      if (rec->evaluator->EvaluateWithCurrentAmounts( rec->evaluator, 
							     (KINETIC_LAW*)GetTriggerInEvent( rec->eventArray[i] ) )) {
	SetTriggerEnabledInEvent( rec->eventArray[i], TRUE );
      } else {
	SetTriggerEnabledInEvent( rec->eventArray[i], FALSE );
      } 
      */
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
    while( !(decider->IsTerminationConditionMet( decider, NULL, rec->time )) ) {
	nextEventTime = fireEvents( rec, rec->time );
	if (nextEventTime==-2.0) {
	  return FAILING;
	}
        if( IS_FAILED( ( ret = _CalculateReactionRates( rec ) ) ) ) {
            return ret;
        }
        if( IS_FAILED( ( ret = _Update( rec ) ) ) ) {
            return ret;
        }
	rec->time += rec->timeStep;
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
    rate = evaluator->EvaluateWithCurrentConcentrations( evaluator, law );
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
    double nextPrintTime = rec->nextPrintTime;
    double time = rec->time;
    double printInterval = rec->printInterval;
    SIMULATION_PRINTER *printer = rec->printer;

    while( nextPrintTime <= time ) {
      if (nextPrintTime > 0.0) printf("Time = %.2f\n",nextPrintTime);
      fflush(stdout);
        if( IS_FAILED( ( ret = printer->PrintValues( printer, nextPrintTime ) ) ) ) {
            return ret;
        }
        nextPrintTime += printInterval; 
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

    do {
      eventFired = FALSE;
      for (i = 0; i < rec->eventsSize; i++) {
	nextEventTime = GetNextEventTimeInEvent( rec->eventArray[i] );
	triggerEnabled = GetTriggerEnabledInEvent( rec->eventArray[i] );
	if (nextEventTime != -1.0) {
	  if (time >= nextEventTime) {
	    fireEvent( rec->eventArray[i], rec );
	    SetNextEventTimeInEvent( rec->eventArray[i], -1.0 );
	    eventFired = TRUE;
	  } else {
	    if ((firstEventTime == -1.0) || (nextEventTime < firstEventTime)) {
	      firstEventTime = nextEventTime;
	    }
	  }
	}
	if (!triggerEnabled) {
	  if (rec->evaluator->EvaluateWithCurrentAmounts( rec->evaluator, 
								 (KINETIC_LAW*)GetTriggerInEvent( rec->eventArray[i] ) )) {
	    SetTriggerEnabledInEvent( rec->eventArray[i], TRUE );
	    if (GetDelayInEvent( rec->eventArray[i] )==NULL) {
	      deltaTime = 0; 
	    } 
	    else {
	      deltaTime = rec->evaluator->EvaluateWithCurrentAmounts( rec->evaluator, 
								      (KINETIC_LAW*)GetDelayInEvent( rec->eventArray[i] ) );
	    }
	    if (deltaTime > 0) { 
	      SetNextEventTimeInEvent( rec->eventArray[i], time + deltaTime );
	      if ((firstEventTime == -1.0) || (time + deltaTime < firstEventTime)) {
		firstEventTime = time + deltaTime;
	      }
	    } else if (deltaTime == 0) {
	      fireEvent( rec->eventArray[i], rec );
	      eventFired = TRUE;
	    } else {
	      ErrorReport( FAILING, "_Update", "delay for event evaluates to a negative number" );
	      return -2;
	    }
	  }
	} else {
	  if (!rec->evaluator->EvaluateWithCurrentAmounts( rec->evaluator, 
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
    if ( varType == SPECIES_EVENT_ASSIGNMENT ) {
      concentration = rec->evaluator->EvaluateWithCurrentConcentrations( rec->evaluator, eventAssignment->assignment );
      //printf("conc = %g\n",concentration);
      SetConcentrationInSpeciesNode( rec->speciesArray[j], concentration );
    } else if ( varType == COMPARTMENT_EVENT_ASSIGNMENT ) {
      concentration = rec->evaluator->EvaluateWithCurrentConcentrations( rec->evaluator, eventAssignment->assignment );  
      //printf("conc = %g\n",concentration);
      SetCurrentSizeInCompartment( rec->compartmentArray[j], concentration );
    } else {
      concentration = rec->evaluator->EvaluateWithCurrentConcentrations( rec->evaluator, eventAssignment->assignment );   
      //printf("conc = %g\n",concentration);
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
      concentration = rec->evaluator->EvaluateWithCurrentConcentrations( rec->evaluator, 
									 (KINETIC_LAW*)GetMathInRule( rec->ruleArray[i] ) );
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
}

static RET_VAL _UpdateSpeciesValues( EULER_SIMULATION_RECORD *rec ) {
    RET_VAL ret = SUCCESS;
    UINT32 i = 0;
    UINT32 j = 0;
    UINT32 speciesSize = rec->speciesSize;
    long stoichiometry = 0;
    double concentration = 0.0;    
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

    for (i = 0; i < rec->rulesSize; i++) {
      if ( GetRuleType( rec->ruleArray[i] ) == RULE_TYPE_RATE ) {
	change = rec->evaluator->EvaluateWithCurrentConcentrations( rec->evaluator, 
								    (KINETIC_LAW*)GetMathInRule( rec->ruleArray[i] ) );
	varType = GetRuleVarType( rec->ruleArray[i] );
	j = GetRuleIndex( rec->ruleArray[i] );
	if ( varType == SPECIES_RULE ) {
	  concentration = GetConcentrationInSpeciesNode( rec->speciesArray[j] );
	  concentration += (change * deltaTime);
	  SetRuleCurValue( rec->ruleArray[i], concentration );
	} else if ( varType == COMPARTMENT_RULE ) {
	  concentration = GetCurrentSizeInCompartment( rec->compartmentArray[j] );
	  concentration += (change * deltaTime);
	  SetRuleCurValue( rec->ruleArray[i], concentration );
	} else {
	  concentration = GetCurrentRealValueInSymbol( rec->symbolArray[j] );
	  concentration += (change * deltaTime);
	  SetRuleCurValue( rec->ruleArray[i], concentration );
	}
      }
    }
    for (i = 0; i < rec->rulesSize; i++) {
      if ( GetRuleType( rec->ruleArray[i] ) == RULE_TYPE_RATE ) {
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
            stoichiometry = (long)GetStoichiometryInIREdge( edge );
            reaction = GetReactionInIREdge( edge );
            rate = GetReactionRate( reaction );
            change -= ((double)stoichiometry * rate);
            TRACE_2( "\tchanges from %s is %g", GetCharArrayOfString( GetReactionNodeName( reaction ) ), 
               -((double)stoichiometry * rate));
        }                
        edges = GetProductEdges( (IR_NODE*)species );
        ResetCurrentElement( edges );
        while( ( edge = GetNextEdge( edges ) ) != NULL ) {
            stoichiometry = (long)GetStoichiometryInIREdge( edge );
            reaction = GetReactionInIREdge( edge );
            rate = GetReactionRate( reaction );
            change += ((double)stoichiometry * rate);
            TRACE_2( "\tchanges from %s is %g", GetCharArrayOfString( GetReactionNodeName( reaction ) ), 
               ((double)stoichiometry * rate));
        }
        
        concentration += (change * deltaTime);
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

    return ret;            
}
