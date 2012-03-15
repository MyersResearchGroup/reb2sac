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
#include <math.h>
#include <float.h>
#include "gsl/gsl_vector.h"
#include "gsl/gsl_multiroots.h"
#include "ssa_with_user_update.h"




static BOOL _IsModelConditionSatisfied( IR *ir );


static RET_VAL _InitializeRecord( SSA_WITH_USER_UPDATE_RECORD *rec, BACK_END_PROCESSOR *backend, IR *ir );
static RET_VAL _InitializeSimulation( SSA_WITH_USER_UPDATE_RECORD *rec, int runNum );
static RET_VAL _RunSimulation( SSA_WITH_USER_UPDATE_RECORD *rec );

static RET_VAL _CleanSimulation( SSA_WITH_USER_UPDATE_RECORD *rec );
static RET_VAL _CleanRecord( SSA_WITH_USER_UPDATE_RECORD *rec );

static RET_VAL _CalculateTotalPropensities( SSA_WITH_USER_UPDATE_RECORD *rec );
static RET_VAL _CalculatePropensities( SSA_WITH_USER_UPDATE_RECORD *rec );
static RET_VAL _CalculatePropensity( SSA_WITH_USER_UPDATE_RECORD *rec, REACTION *reaction );
static RET_VAL _FindNextReactionTime( SSA_WITH_USER_UPDATE_RECORD *rec );
static RET_VAL _FindNextReaction( SSA_WITH_USER_UPDATE_RECORD *rec );
static RET_VAL _Update( SSA_WITH_USER_UPDATE_RECORD *rec );
static RET_VAL _Print( SSA_WITH_USER_UPDATE_RECORD *rec );
static RET_VAL _PrintStatistics( SSA_WITH_USER_UPDATE_RECORD *rec, FILE *file);
static RET_VAL _UpdateNodeValues( SSA_WITH_USER_UPDATE_RECORD *rec );
static RET_VAL _UpdateSpeciesValues( SSA_WITH_USER_UPDATE_RECORD *rec );
static RET_VAL _UpdateReactionRateUpdateTime( SSA_WITH_USER_UPDATE_RECORD *rec );


static BOOL _IsUpdateByUserInputReady( SSA_WITH_USER_UPDATE_RECORD *rec );
static RET_VAL _UpdateByUserInput( SSA_WITH_USER_UPDATE_RECORD *rec );


static int _ComparePropensity( REACTION *a, REACTION *b );
static BOOL _IsTerminationConditionMet( SSA_WITH_USER_UPDATE_RECORD *rec );

static void fireEvent( EVENT *event, SSA_WITH_USER_UPDATE_RECORD *rec );


DLLSCOPE RET_VAL STDCALL DoSSAWithUserUpdateAnalysis( BACK_END_PROCESSOR *backend, IR *ir ) {
    RET_VAL ret = SUCCESS;
    UINT i = 0;
    UINT runs = 1;
    char *namePrefix = NULL;
    static SSA_WITH_USER_UPDATE_RECORD rec;
    UINT timeout = 0;

    START_FUNCTION("DoSSAWithUserUpdateAnalysis");
    
    if( !_IsModelConditionSatisfied( ir ) ) {
        return ErrorReport( FAILING, "DoSSAWithUserUpdateAnalysis", "Gillespie method cannot be applied to the model" );
    }
    
    if( IS_FAILED( ( ret = _InitializeRecord( &rec, backend, ir ) ) ) )  {
        return ErrorReport( ret, "DoSSAWithUserUpdateAnalysis", "initialization of the record failed" );
    }
    
    runs = rec.runs;    
    for( i = 1; i <= runs; i++ ) {
        SeedRandomNumberGenerators( rec.seed );
        rec.seed = GetNextUniformRandomNumber(0,RAND_MAX);
        timeout = 0;
	do {
	  SeedRandomNumberGenerators( rec.seed );
	  if( IS_FAILED( ( ret = _InitializeSimulation( &rec, i ) ) ) ) {
            return ErrorReport( ret, "DoGillespieMonteCarloAnalysis", "initialization of the %i-th simulation failed", i );
	  }
	  timeout++;
	} while ( (ret == CHANGE) && (timeout <= (rec.speciesSize + rec.compartmentsSize + rec.symbolsSize)) );
        if( IS_FAILED( ( ret = _RunSimulation( &rec ) ) ) ) {
            return ErrorReport( ret, "DoSSAWithUserUpdateAnalysis", "%i-th simulation failed at time %f", i, rec.time );
        }
        if( IS_FAILED( ( ret = _CleanSimulation( &rec ) ) ) ) {
            return ErrorReport( ret, "DoSSAWithUserUpdateAnalysis", "cleaning of the %i-th simulation failed", i );
        }         
	printf("Run = %d\n",i);
        fflush(stdout);
    }
    END_FUNCTION("DoSSAWithUserUpdateAnalysis", SUCCESS );
    return ret;            
}

DLLSCOPE RET_VAL STDCALL CloseSSAWithUserUpdateAnalyzer( BACK_END_PROCESSOR *backend ) {
    RET_VAL ret = SUCCESS;
    SSA_WITH_USER_UPDATE_RECORD *rec = (SSA_WITH_USER_UPDATE_RECORD *)(backend->_internal1);
        
    START_FUNCTION("CloseSSAWithUserUpdateAnalyzer");
    
    if( IS_FAILED( ( ret = _CleanRecord( rec ) ) ) )  {
        return ErrorReport( ret, "CloseSSAWithUserUpdateAnalyzer", "cleaning of the record failed" );
    }
        
    END_FUNCTION("CloseSSAWithUserUpdateAnalyzer",  SUCCESS );
    return ret;            
}


static BOOL _IsModelConditionSatisfied( IR *ir ) {
    REACTION *reaction = NULL;
    LINKED_LIST *reactions = NULL;
    
    reactions = ir->GetListOfReactionNodes( ir );
    while( ( reaction = (REACTION*)GetNextFromLinkedList( reactions ) ) != NULL ) {
        if( IsReactionReversibleInReactionNode( reaction ) ) {
            TRACE_0("the input model contains reversible reaction(s), and cannot be used for Gillespie method" );            
            return FALSE;
        }
    }
    return TRUE;
}


static RET_VAL _InitializeRecord( SSA_WITH_USER_UPDATE_RECORD *rec, BACK_END_PROCESSOR *backend, IR *ir ) {
    RET_VAL ret = SUCCESS;
    UINT32 i = 0;
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
    RULE *rule = NULL;
    RULE **ruleArray = NULL;
    RULE_MANAGER *ruleManager;
    CONSTRAINT *constraint = NULL;
    CONSTRAINT **constraintArray = NULL;
    CONSTRAINT_MANAGER *constraintManager;
    REB2SAC_SYMBOL *symbol = NULL;
    REB2SAC_SYMBOL **symbolArray = NULL;
    REB2SAC_SYMTAB *symTab;
    EVENT *event = NULL;
    EVENT **eventArray = NULL;
    EVENT_MANAGER *eventManager;
    COMPILER_RECORD_T *compRec = backend->record;
    LINKED_LIST *list = NULL;
    REB2SAC_PROPERTIES *properties = NULL;

#if GET_SEED_FROM_COMMAND_LINE
    PROPERTIES *options = NULL;
#endif    
            
    list = ir->GetListOfReactionNodes( ir );
    rec->reactionsSize = GetLinkedListSize( list );
    if (rec->reactionsSize==0) {
        return ErrorReport( FAILING, "_InitializeRecord", "no reactions in the model" );
    }
    if( ( reactions = (REACTION**)MALLOC( rec->reactionsSize * sizeof(REACTION*) ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not allocate memory for reaction array" );
    }
    i = 0;
    ResetCurrentElement( list );
    while( ( reaction = (REACTION*)GetNextFromLinkedList( list ) ) != NULL ) {
        reactions[i] = reaction;
        i++;        
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
    if( ( speciesArray = (SPECIES**)MALLOC( rec->speciesSize * sizeof(SPECIES*) ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not allocate memory for species array" );
    }
    i = 0;
    ResetCurrentElement( list );
    while( ( species = (SPECIES*)GetNextFromLinkedList( list ) ) != NULL ) {
        speciesArray[i] = species;
        i++;        
    }
    rec->speciesArray = speciesArray;    
    
    if( ( rec->evaluator = CreateKineticLawEvaluater() ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not create evaluator" );
    }                
    
    properties = compRec->properties;
    if( ( valueString = properties->GetProperty( properties, MONTE_CARLO_SIMULATION_TIME_LIMIT ) ) == NULL ) {
        rec->timeLimit = DEFAULT_MONTE_CARLO_SIMULATION_TIME_LIMIT_VALUE;
    }
    else {
        if( IS_FAILED( ( ret = StrToFloat( &(rec->timeLimit), valueString ) ) ) ) {
            rec->timeLimit = DEFAULT_MONTE_CARLO_SIMULATION_TIME_LIMIT_VALUE;
        }
    }    
    
    if( ( valueString = properties->GetProperty( properties, MONTE_CARLO_SIMULATION_PRINT_INTERVAL ) ) == NULL ) {
        rec->printInterval = DEFAULT_MONTE_CARLO_SIMULATION_PRINT_INTERVAL_VALUE;
    }
    else {
        if( IS_FAILED( ( ret = StrToFloat( &(rec->printInterval), valueString ) ) ) ) {
            rec->printInterval = DEFAULT_MONTE_CARLO_SIMULATION_PRINT_INTERVAL_VALUE;
        }
    }    

#if GET_SEED_FROM_COMMAND_LINE
    options = compRec->options;
    if( ( valueString = options->GetProperty( options, "random.seed" ) ) == NULL ) {
        rec->seed = DEFAULT_MONTE_CARLO_SIMULATION_RANDOM_SEED_VALUE;
    } 
    else {        
        if( IS_FAILED( ( ret = StrToUINT32( &(rec->seed), valueString ) ) ) ) {
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
    
    if( ( valueString = properties->GetProperty( properties, MONTE_CARLO_SIMULATION_RUNS ) ) == NULL ) {
        rec->runs = DEFAULT_MONTE_CARLO_SIMULATION_RUNS_VALUE;
    }
    else {
        if( IS_FAILED( ( ret = StrToUINT32( &(rec->runs), valueString ) ) ) ) {
            rec->runs = DEFAULT_MONTE_CARLO_SIMULATION_RUNS_VALUE;
        }
    }    
    
    if( ( rec->outDir = properties->GetProperty( properties, MONTE_CARLO_SIMULATION_OUT_DIR ) ) == NULL ) {
        rec->outDir = DEFAULT_MONTE_CARLO_SIMULATION_OUT_DIR_VALUE;
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
    
    if( ( rec->decider = 
        CreateSimulationRunTerminationDecider( backend, speciesArray, rec->speciesSize, reactions, rec->reactionsSize, 
					       rec->constraintArray, rec->constraintsSize, rec->evaluator, FALSE, rec->timeLimit ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not create simulation printer" );
    }
    
    if( ( rec->timeSeriesSpeciesLevelUpdater = 
          CreateTimeSeriesSpeciesLevelUpdater( speciesArray, rec->speciesSize, properties ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not create time series species level updater" );        
    } 
        
    backend->_internal1 = (CADDR_T)rec;
    
    return ret;            
}

static RET_VAL _InitializeSimulation( SSA_WITH_USER_UPDATE_RECORD *rec, int runNum ) {
    RET_VAL ret = SUCCESS;
    char filenameStem[512];
    double amount = 0;
    UINT32 i = 0;
    UINT32 size = 0;
    SPECIES *species = NULL;
    SPECIES **speciesArray = rec->speciesArray;
    REACTION *reaction = NULL;
    REACTION **reactionArray = rec->reactionArray;
    KINETIC_LAW_EVALUATER *evaluator = rec->evaluator;
    SIMULATION_PRINTER *printer = rec->printer;
    TIME_SERIES_SPECIES_LEVEL_UPDATER *timeSeriesSpeciesLevelUpdater = rec->timeSeriesSpeciesLevelUpdater;
    double compSize = 0.0;
    double param = 0.0;
    COMPARTMENT *compartment = NULL;
    COMPARTMENT **compartmentArray = rec->compartmentArray;
    REB2SAC_SYMBOL *symbol = NULL;
    REB2SAC_SYMBOL **symbolArray = rec->symbolArray;
    KINETIC_LAW *law = NULL;
    BOOL change = FALSE;
    char filename[512];
    FILE *file = NULL;
    
    sprintf( filenameStem, "%s%crun-%i", rec->outDir, FILE_SEPARATOR, runNum );        
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
	    amount = GetInitialAmountInSpeciesNode( species );
	  }
	  else {
	    amount = GetInitialAmountInSpeciesNode( species ); 
	  }
	} else {
	  law = CloneKineticLaw( law );
	  SimplifyInitialAssignment(law);
	  if (law->valueType == KINETIC_LAW_VALUE_TYPE_REAL) {
	    amount = GetRealValueFromKineticLaw(law); 
	  } else if (law->valueType == KINETIC_LAW_VALUE_TYPE_INT) {
	    amount = (double)GetIntValueFromKineticLaw(law); 
	  } 
	  if( IsInitialQuantityInAmountInSpeciesNode( species ) ) {
	    if (GetInitialAmountInSpeciesNode( species ) != amount) {
	      SetInitialAmountInSpeciesNode( species, amount );
	      change = TRUE;
	    }
	  }
	  else {
	    if (GetInitialAmountInSpeciesNode( species ) != amount) {
	      SetInitialAmountInSpeciesNode( species, amount ); 
	      change = TRUE;
	    }
	  }
	  FreeKineticLaw( &(law) );
	}
        if( IS_FAILED( ( ret = SetAmountInSpeciesNode( species, amount ) ) ) ) {
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
      if (rec->evaluator->EvaluateWithCurrentAmounts( rec->evaluator, 
							     (KINETIC_LAW*)GetTriggerInEvent( rec->eventArray[i] ) )) {
	SetTriggerEnabledInEvent( rec->eventArray[i], TRUE );
      } else {
	SetTriggerEnabledInEvent( rec->eventArray[i], FALSE );
      }
    }

    if (change)
      return CHANGE;
    if( IS_FAILED( ( ret = timeSeriesSpeciesLevelUpdater->Initialize( timeSeriesSpeciesLevelUpdater ) ) ) ) {
        return ret;
    }
    rec->nextTimeSeriesSpeciesLevelUpdateTime = 
            timeSeriesSpeciesLevelUpdater->GetNextUpdateTime( timeSeriesSpeciesLevelUpdater );

    sprintf( filename, "%s%cstatistics.txt", rec->outDir, FILE_SEPARATOR );
    if( ( file = fopen( filename, "w" ) ) == NULL ) {
    	return ErrorReport( FAILING, "_InitializeSimulation", "could not create a statistics file" );
    }
    if( IS_FAILED( ( ret = _PrintStatistics( rec, file ) ) ) ) {
    	return ret;
    }
    fclose( file );
        
    return ret;            
}

static RET_VAL _RunSimulation( SSA_WITH_USER_UPDATE_RECORD *rec ) {
    RET_VAL ret = SUCCESS;
    int i = 0;
    int j = 0;
    REACTION *reaction = NULL;
    SIMULATION_PRINTER *printer = NULL;
    SIMULATION_RUN_TERMINATION_DECIDER *decider = NULL;
    int nextEvent = 0;
    double nextEventTime = 0;

    printer = rec->printer;
    decider = rec->decider;
    while( !(decider->IsTerminationConditionMet( decider, reaction, rec->time )) ) {
        i++;
        if( IS_FAILED( ( ret = _CalculatePropensities( rec ) ) ) ) {
            return ret;
        }
        if( IS_FAILED( ( ret = _CalculateTotalPropensities( rec ) ) ) ) {
            return ret;
        }
        if( IS_REAL_EQUAL( rec->totalPropensities, 0.0 ) ) {                        
            TRACE_1( "the total propensity is 0 at iteration %i", i );
            rec->time = rec->timeLimit;
            reaction = NULL;
            if( _IsUpdateByUserInputReady( rec ) ) {
                if( IS_FAILED( ( ret = _UpdateByUserInput( rec ) ) ) ) {
                    return ret;
                }
            } 
            else {
                if( IS_FAILED( ( ret = _Print( rec ) ) ) ) {
                    return ret;
                }            
                break;
            }
        }
        else { 
            if( IS_FAILED( ( ret = _FindNextReactionTime( rec ) ) ) ) {
                return ret;
            }
            if( _IsUpdateByUserInputReady( rec ) ) {
                if( IS_FAILED( ( ret = _UpdateByUserInput( rec ) ) ) ) {
                    return ret;
                }
                reaction = NULL;
            } 
            else {
	      nextEvent = -1;
	      for (j = 0; j < rec->eventsSize; j++) {
		nextEventTime = GetNextEventTimeInEvent( rec->eventArray[j] );
		if ( nextEventTime > 0 && nextEventTime < rec->time ) {
		  nextEvent = j;
		  rec->time = GetNextEventTimeInEvent( rec->eventArray[j] );
		}
	      }
	      if ( nextEvent >= 0) {
		fireEvent( rec->eventArray[nextEvent], rec );
		SetNextEventTimeInEvent( rec->eventArray[nextEvent], -1.0 );
	      } else {
		if ( rec->time < rec->timeLimit ) {
		  if( IS_FAILED( ( ret = _FindNextReaction( rec ) ) ) ) {
                    return ret;
		  }
		  if( IS_FAILED( ( ret = _Update( rec ) ) ) ) {
                    return ret;
		  }
		  reaction = rec->nextReaction;
		} else {
		  if( IS_FAILED( ( ret = _Print( rec ) ) ) ) {
		  return ret;            
		  }
		}
	      }
	    }
        }
    }
    if( rec->time >= rec->timeLimit ) {
        rec->time = rec->timeLimit;
    } 
    if( IS_FAILED( ( ret = printer->PrintValues( printer, rec->time ) ) ) ) {
        return ret;
    }
    /*
    if( IS_FAILED( ( ret = printer->PrintValues( printer, rec->time ) ) ) ) {
        return ret;
    }
    */
    if( IS_FAILED( ( ret = printer->PrintEnd( printer ) ) ) ) {
        return ret;
    }
    
    if( IS_FAILED( ( ret = _CleanSimulation( rec ) ) ) ) {
        return ret;
    }
        
    return ret;            
}

static RET_VAL _CleanSimulation( SSA_WITH_USER_UPDATE_RECORD *rec ) {
    RET_VAL ret = SUCCESS;
    /* something goes wrong if I put fclose here */

#if 0 
    if( 0 != fclose( rec->out ) ) {
        TRACE_0("file close error" );
        
        rec->out = NULL;
    }  
#endif
    return ret;            
}

static RET_VAL _CleanRecord( SSA_WITH_USER_UPDATE_RECORD *rec ) {
    RET_VAL ret = SUCCESS;
    char filename[512];
    FILE *file = NULL;
    SIMULATION_PRINTER *printer = rec->printer;
    SIMULATION_RUN_TERMINATION_DECIDER *decider = rec->decider;
    
    sprintf( filename, "%s%csim-rep.txt", rec->outDir, FILE_SEPARATOR );        
    if( ( file = fopen( filename, "w" ) ) == NULL ) {
        return ErrorReport( FAILING, "_CleanRecord", "could not create a report file" );
    }
    if( IS_FAILED( ( ret = decider->Report( decider, file ) ) ) ) {
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
    
    printer->Destroy( printer );
    decider->Destroy( decider );
    
    return ret;            
}

static RET_VAL _PrintStatistics(SSA_WITH_USER_UPDATE_RECORD *rec, FILE *file) {
	RET_VAL ret = SUCCESS;
	double stoichiometry = 0;
	UINT32 i = 0;
	UINT32 j = 0;
	UINT32 reactionsSize = rec->reactionsSize;
	UINT32 speciesSize = rec->speciesSize;
	UINT32 symbolsSize = rec->symbolsSize;
	REACTION *reaction = NULL;
	REACTION **reactionArray = rec->reactionArray;
	SPECIES *species = NULL;
	SPECIES **speciesArray = rec->speciesArray;
	REB2SAC_SYMBOL *symbol = NULL;
	REB2SAC_SYMBOL **symbolArray = rec->symbolArray;
	REB2SAC_SYMBOL *speciesRef = NULL;
	REB2SAC_SYMBOL *convFactor = NULL;
	IR_EDGE *edge = NULL;
	LINKED_LIST *edges = NULL;

	if ((speciesSize <= 0) || (reactionsSize <= 0)) return ret;

	fprintf( file, "Parameter Values:" NEW_LINE);

	for (i = 0; i < symbolsSize; i++) {
		symbol = symbolArray[i];
		if (IsRealValueSymbol(symbol)) {
			fprintf( file, "%s = %f" NEW_LINE, *GetSymbolID(symbol), GetRealValueInSymbol(symbol));
		}
	}
	fprintf( file, NEW_LINE);

	fprintf( file, "Initial State Vector:" NEW_LINE);

	for (i = 0; i < speciesSize; i++) {
		species = speciesArray[i];
		SetAmountInSpeciesNode(species, GetInitialAmountInSpeciesNode(species));
		fprintf( file, "%s = %f" NEW_LINE, *GetSpeciesNodeID(species), GetInitialAmountInSpeciesNode(species));
	}
	fprintf( file, NEW_LINE);

	gsl_matrix *delta_matrix = gsl_matrix_alloc(speciesSize, reactionsSize);
	gsl_matrix *reactant_matrix = gsl_matrix_alloc(speciesSize, reactionsSize);

	for (i = 0; i < reactionsSize; i++) {
		for (j = 0; j < speciesSize; j++) {
			gsl_matrix_set(delta_matrix, j, i, 0);
			gsl_matrix_set(reactant_matrix, j, i, 0);
		}
	}

	if (IS_FAILED((ret = _UpdateAllReactionRateUpdateTimes(rec, rec->time)))) {
		return ret;
	}
	if (IS_FAILED((ret = _CalculatePropensities(rec)))) {
		return ret;
	}

	fprintf( file, "Initial Reaction Rate Array:" NEW_LINE);

	for (i = 0; i < reactionsSize; i++) {
		reaction = reactionArray[i];
		fprintf( file, "%f" NEW_LINE, GetReactionRate(reaction));
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

	fprintf( file, "Reaction Rate Equation Array:" NEW_LINE);

	for (i = 0; i < reactionsSize; i++) {
		reaction = reactionArray[i];
		fprintf( file, "%s" NEW_LINE, *ToStringKineticLaw(GetKineticLawInReactionNode(reaction)));
	}
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

static BOOL _IsTerminationConditionMet( SSA_WITH_USER_UPDATE_RECORD *rec ) {
    return FALSE;
}



static RET_VAL _CalculateTotalPropensities( SSA_WITH_USER_UPDATE_RECORD *rec ) {
    RET_VAL ret = SUCCESS;
    UINT32 i = 0;
    UINT32 size = rec->reactionsSize;
    double total = 0.0;
    REACTION **reactionArray = rec->reactionArray;    
    
    total = GetReactionRate( reactionArray[0] );
    for( i = 1; i < size; i++ ) {
        total += GetReactionRate( reactionArray[i] );     
    }         
    rec->totalPropensities = total;        
    TRACE_1( "the total propensity is %f", total );
    return ret;            
}

static RET_VAL _CalculatePropensities( SSA_WITH_USER_UPDATE_RECORD *rec ) {
    RET_VAL ret = SUCCESS;
    int i = 0;
    int size = 0;
    double time = rec->time;
    double updatedTime = 0.0;
    REACTION *reaction = NULL;
    REACTION **reactionArray = rec->reactionArray;    
    
    size = rec->reactionsSize;    
    for( i = 0; i < size; i++ ) {
        reaction = reactionArray[i];
        updatedTime = GetReactionRateUpdatedTime( reaction );
        if( IS_REAL_EQUAL( updatedTime, time ) ) {
            if( IS_FAILED( ( ret = _CalculatePropensity( rec, reaction ) ) ) ) {
                return ret;        
            }
        }
    }
#if 0    
    /* qsort is not good way to sort an array which is nearly sorted. */
    qsort( rec->reactionArray, size, sizeof(REACTION*), (int(*)(const void *, const void *))_ComparePropensity );
#endif

#ifdef DEBUG
    for( i = 0; i < size; i++ ) {
        printf( "(%s, %f), ", GetCharArrayOfString( GetReactionNodeName( rec->reactionArray[i] ) ), GetReactionRate( rec->reactionArray[i] ) );
    }
    printf( NEW_LINE );
#endif                   
    return ret;          
}


static RET_VAL _CalculatePropensity( SSA_WITH_USER_UPDATE_RECORD *rec, REACTION *reaction ) {
    RET_VAL ret = SUCCESS;
    double stoichiometry = 0;
    double amount = 0;    
    double propensity = 0.0;
    double time = rec->time;
    SPECIES *species = NULL;
    IR_EDGE *edge = NULL;
    LINKED_LIST *edges = NULL;
    KINETIC_LAW *law = NULL;
    KINETIC_LAW_EVALUATER *evaluator = rec->evaluator;
        
    edges = GetReactantEdges( (IR_NODE*)reaction );
    ResetCurrentElement( edges );
    while( ( edge = GetNextEdge( edges ) ) != NULL ) {
        stoichiometry = GetStoichiometryInIREdge( edge );
        species = GetSpeciesInIREdge( edge );
        amount = GetAmountInSpeciesNode( species );
        if( amount < stoichiometry ) {
            if( IS_FAILED( ( ret = SetReactionRate( reaction, 0.0 ) ) ) ) {
                return ret;         
            }
#ifdef DEBUG
            printf( "(%s, %f)" NEW_LINE, GetCharArrayOfString( GetReactionNodeName( reaction ) ), 
                GetReactionRate( reaction ) );
#endif                   
            return SUCCESS;         
        }                
    }    
#if 0    
    edges = GetModifierEdges( (IR_NODE*)reaction );
    ResetCurrentElement( edges );
    while( ( edge = GetNextEdge( edges ) ) != NULL ) {
        stoichiometry = GetStoichiometryInIREdge( edge );
        species = GetSpeciesInIREdge( edge );        
        amount = GetAmountInSpeciesNode( species );
        if( amount < stoichiometry ) {
            if( IS_FAILED( ( ret = SetReactionRate( reaction, 0.0 ) ) ) ) {
                return ret;         
            }
            return SUCCESS;         
        }                
    }    
#endif
    
    evaluator = rec->evaluator;    
    law = GetKineticLawInReactionNode( reaction );
    propensity = evaluator->EvaluateWithCurrentAmounts( evaluator, law );
    if( propensity <= 0.0 ) {
        if( IS_FAILED( ( ret = SetReactionRate( reaction, 0.0 ) ) ) ) {
            return ret;         
        }
    }
    /* in case nan */    
    else if( !( propensity < DBL_MAX ) ) {
        if( IS_FAILED( ( ret = SetReactionRate( reaction, 0.0 ) ) ) ) {
            return ret;         
        }
    }
    else {
        if( IS_FAILED( ( ret = SetReactionRate( reaction, propensity ) ) ) ) {
            return ret;         
        }
    }
#ifdef DEBUG
    printf( "(%s, %f)" NEW_LINE, GetCharArrayOfString( GetReactionNodeName( reaction ) ), 
        GetReactionRate( reaction ) );
#endif                   
    return ret;         
}


static RET_VAL _FindNextReactionTime( SSA_WITH_USER_UPDATE_RECORD *rec ) {
    RET_VAL ret = SUCCESS;
    double random = 0.0;
    double t = 0.0;
    
    random = GetNextUnitUniformRandomNumber();
    t = log( 1.0 / random ) / rec->totalPropensities;
    rec->time += t;  
    if( rec->time > rec->timeLimit ) {
        rec->time = rec->timeLimit;
    }
    TRACE_1( "time to next reaction is %f", t );
    return ret;            
}

static RET_VAL _FindNextReaction( SSA_WITH_USER_UPDATE_RECORD *rec ) {
    RET_VAL ret = SUCCESS;
    UINT32 i = 0;
    UINT32 size = rec->reactionsSize - 1;
    double random = 0.0;
    double threshold = 0.0;
    double sum = 0.0;
    REACTION *reaction = NULL;
    REACTION **reactionArray = rec->reactionArray;
    
    random = GetNextUnitUniformRandomNumber();
    threshold = random * rec->totalPropensities;
    
    TRACE_1( "next reaction threshold is %f", threshold );
    
    for( i = 0; i < size; i++ ) {
        sum += GetReactionRate( reactionArray[i] );                
        if( sum >= threshold ) {
            break;
        }
    }
    
    rec->nextReaction = reactionArray[i];    
    TRACE_1( "next reaction is %s", GetCharArrayOfString( GetReactionNodeName( rec->nextReaction ) ) );
    return ret;            
}

static RET_VAL _Update( SSA_WITH_USER_UPDATE_RECORD *rec ) {
    RET_VAL ret = SUCCESS;

    if( IS_FAILED( ( ret = _Print( rec ) ) ) ) {
        return ret;            
    }
    if( IS_FAILED( ( ret = _UpdateNodeValues( rec ) ) ) ) {
        return ret;            
    }         
    
    return ret;            
}



static RET_VAL _Print( SSA_WITH_USER_UPDATE_RECORD *rec ) {
    RET_VAL ret = SUCCESS;
    double nextPrintTime = rec->nextPrintTime;
    double time = rec->time;
    double printInterval = rec->printInterval;
    REACTION *reaction = NULL;
    SIMULATION_PRINTER *printer = rec->printer;

    while(( nextPrintTime < time ) && ( nextPrintTime < rec->timeLimit )){
        if( IS_FAILED( ( ret = printer->PrintValues( printer, nextPrintTime ) ) ) ) {
            return ret;
        }
        nextPrintTime += printInterval; 
    }     
    rec->nextPrintTime = nextPrintTime;
    return ret;            
}

static RET_VAL _UpdateNodeValues( SSA_WITH_USER_UPDATE_RECORD *rec ) {
    RET_VAL ret = SUCCESS;
    
    if( IS_FAILED( ( ret = _UpdateSpeciesValues( rec ) ) ) ) {
        return ret;            
    }
    if( IS_FAILED( ( ret = _UpdateReactionRateUpdateTime( rec ) ) ) ) {
        return ret;            
    }
    
    return ret;            
}


static void fireEvent( EVENT *event, SSA_WITH_USER_UPDATE_RECORD *rec ) {
  LINKED_LIST *list = NULL;
  EVENT_ASSIGNMENT *eventAssignment;
  double amount = 0.0;    
  UINT j;

  list = GetEventAssignments( event );
  ResetCurrentElement( list );
  while( ( eventAssignment = (EVENT_ASSIGNMENT*)GetNextFromLinkedList( list ) ) != NULL ) {
    //printf("Firing event %s\n",GetCharArrayOfString(eventAssignment->var));
    for (j = 0; j < rec->speciesSize; j++) {
      if ( strcmp( GetCharArrayOfString(eventAssignment->var),
		   GetCharArrayOfString(GetSpeciesNodeID( rec->speciesArray[j] ) ) ) == 0 ) {
	amount = rec->evaluator->EvaluateWithCurrentAmounts( rec->evaluator, eventAssignment->assignment );
	//printf("conc = %g\n",amount);
	SetAmountInSpeciesNode( rec->speciesArray[j], amount );
	break;
      } 
    }
    for (j = 0; j < rec->compartmentsSize; j++) {
      if ( strcmp( GetCharArrayOfString(eventAssignment->var),
		   GetCharArrayOfString(GetCompartmentID( rec->compartmentArray[j] ) ) ) == 0 ) {
	amount = rec->evaluator->EvaluateWithCurrentAmounts( rec->evaluator, eventAssignment->assignment );  
	//printf("conc = %g\n",amount);
	SetCurrentSizeInCompartment( rec->compartmentArray[j], amount );
	break;
      }
    }
    for (j = 0; j < rec->symbolsSize; j++) {
      if ( strcmp( GetCharArrayOfString(eventAssignment->var),
		   GetCharArrayOfString(GetSymbolID( rec->symbolArray[j] ) ) ) == 0 ) {
	amount = rec->evaluator->EvaluateWithCurrentAmounts( rec->evaluator, eventAssignment->assignment );   
	//printf("conc = %g\n",amount);
	SetCurrentRealValueInSymbol( rec->symbolArray[j], amount );
	break;
      } 
    }
  }
}


static RET_VAL _UpdateSpeciesValues( SSA_WITH_USER_UPDATE_RECORD *rec ) {
    RET_VAL ret = SUCCESS;
    double stoichiometry = 0;
    double amount = 0;    
    double change = 0;    
    SPECIES *species = NULL;
    IR_EDGE *edge = NULL;
    REACTION *reaction = rec->nextReaction;
    LINKED_LIST *edges = NULL;
    KINETIC_LAW_EVALUATER *evaluator = rec->evaluator;
    UINT i = 0;
    UINT j = 0;
    double deltaTime;
    BOOL triggerEnabled;

    for (i = 0; i < rec->rulesSize; i++) {
      if ( GetRuleType( rec->ruleArray[i] ) == RULE_TYPE_RATE_ASSIGNMENT ) {
	for (j = 0; j < rec->speciesSize; j++) {
	  if ( strcmp( GetCharArrayOfString(GetRuleVar( rec->ruleArray[i] )),
		       GetCharArrayOfString(GetSpeciesNodeID( rec->speciesArray[j] ) ) ) == 0 ) {
	    /* Not technically correct as only calculated when a reaction fires */
	    change = rec->evaluator->EvaluateWithCurrentAmounts( rec->evaluator, 
								 (KINETIC_LAW*)GetMathInRule( rec->ruleArray[i] ) );
	    amount = GetAmountInSpeciesNode( rec->speciesArray[j] );
	    amount += (change * rec->time);
	    SetAmountInSpeciesNode( rec->speciesArray[j], amount );
	    break;
	  }
	}
	for (j = 0; j < rec->compartmentsSize; j++) {
	  if ( strcmp( GetCharArrayOfString(GetRuleVar( rec->ruleArray[i] )),
		       GetCharArrayOfString(GetCompartmentID( rec->compartmentArray[j] ) ) ) == 0 ) {
	    /* Not technically correct as only calculated when a reaction fires */
	    change = rec->evaluator->EvaluateWithCurrentAmounts( rec->evaluator, 
								 (KINETIC_LAW*)GetMathInRule( rec->ruleArray[i] ) );
	    amount = GetCurrentSizeInCompartment( rec->compartmentArray[j] );
	    amount += (change * rec->time);
	    SetCurrentSizeInCompartment( rec->compartmentArray[j], amount );
	    break;
	  }
	}
	for (j = 0; j < rec->symbolsSize; j++) {
	  if ( strcmp( GetCharArrayOfString(GetRuleVar( rec->ruleArray[i] )),
		       GetCharArrayOfString(GetSymbolID( rec->symbolArray[j] ) ) ) == 0 ) {
	    /* Not technically correct as only calculated when a reaction fires */
	    change = rec->evaluator->EvaluateWithCurrentAmounts( rec->evaluator, 
								 (KINETIC_LAW*)GetMathInRule( rec->ruleArray[i] ) );
	    amount = GetCurrentRealValueInSymbol( rec->symbolArray[j] );
	    amount += (change * rec->time);
	    SetCurrentRealValueInSymbol( rec->symbolArray[j], amount );
	    break;
	  }
	}
      }
    }

    edges = GetReactantEdges( (IR_NODE*)reaction );
    ResetCurrentElement( edges );
    while( ( edge = GetNextEdge( edges ) ) != NULL ) {
        stoichiometry = GetStoichiometryInIREdge( edge );
        species = GetSpeciesInIREdge( edge );
        amount = GetAmountInSpeciesNode( species ) - stoichiometry;
        TRACE_3( "the amount of %s decreases from %g to %g", 
            GetCharArrayOfString( GetSpeciesNodeName( species ) ), 
            GetAmountInSpeciesNode( species ),
            amount );
        SetAmountInSpeciesNode( species, amount );
        if( IS_FAILED( ( ret = evaluator->SetSpeciesValue( evaluator, species, amount ) ) ) ) {
            return ret;
        }        
    }    
        
    edges = GetProductEdges( (IR_NODE*)reaction );
    ResetCurrentElement( edges );
    while( ( edge = GetNextEdge( edges ) ) != NULL ) {
        stoichiometry = GetStoichiometryInIREdge( edge );
        species = GetSpeciesInIREdge( edge );
        amount = GetAmountInSpeciesNode( species ) + stoichiometry;
        TRACE_3( "the amount of %s increases from %g to %g", 
            GetCharArrayOfString( GetSpeciesNodeName( species ) ), 
            GetAmountInSpeciesNode( species ),
            amount );
        SetAmountInSpeciesNode( species, amount );
    }    

    for (i = 0; i < rec->rulesSize; i++) {
      if ( GetRuleType( rec->ruleArray[i] ) == RULE_TYPE_ASSIGNMENT ) {
	for (j = 0; j < rec->speciesSize; j++) {
	  if ( strcmp( GetCharArrayOfString(GetRuleVar( rec->ruleArray[i] )),
		       GetCharArrayOfString(GetSpeciesNodeID( rec->speciesArray[j] ) ) ) == 0 ) {
	    amount = rec->evaluator->EvaluateWithCurrentAmounts( rec->evaluator, 
								 (KINETIC_LAW*)GetMathInRule( rec->ruleArray[i] ) );
	    SetAmountInSpeciesNode( rec->speciesArray[j], amount );
	    break;
	  } 
	}
	for (j = 0; j < rec->compartmentsSize; j++) {
	  if ( strcmp( GetCharArrayOfString(GetRuleVar( rec->ruleArray[i] )),
		       GetCharArrayOfString(GetCompartmentID( rec->compartmentArray[j] ) ) ) == 0 ) {
	    amount = rec->evaluator->EvaluateWithCurrentAmounts( rec->evaluator, 
								 (KINETIC_LAW*)GetMathInRule( rec->ruleArray[i] ) );
	    SetCurrentSizeInCompartment( rec->compartmentArray[j], amount );
	    break;
	  } 
	}
	for (j = 0; j < rec->symbolsSize; j++) {
	  if ( strcmp( GetCharArrayOfString(GetRuleVar( rec->ruleArray[i] )),
		       GetCharArrayOfString(GetSymbolID( rec->symbolArray[j] ) ) ) == 0 ) {
	    amount = rec->evaluator->EvaluateWithCurrentAmounts( rec->evaluator, 
								 (KINETIC_LAW*)GetMathInRule( rec->ruleArray[i] ) );
	    SetCurrentRealValueInSymbol( rec->symbolArray[j], amount );
	    break;
	  } 
	  if ((strcmp(GetCharArrayOfString( GetSymbolID(rec->symbolArray[j]) ),"t")==0) ||
	      (strcmp(GetCharArrayOfString( GetSymbolID(rec->symbolArray[j]) ),"time")==0)) {
	    SetCurrentRealValueInSymbol( rec->symbolArray[j], rec->time );
	  }
	}
      }
    }
    for (i = 0; i < rec->eventsSize; i++) {
      triggerEnabled = GetTriggerEnabledInEvent( rec->eventArray[i] );
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
	    SetNextEventTimeInEvent( rec->eventArray[i], rec->time + deltaTime );
	  } else if (deltaTime == 0) {
	    fireEvent( rec->eventArray[i], rec );
	  } else {
	    return ErrorReport( FAILING, "_Update", "delay for event evaluates to a negative number" );
	  }
	}
      } else {
	if (!rec->evaluator->EvaluateWithCurrentAmounts( rec->evaluator, 
							 (KINETIC_LAW*)GetTriggerInEvent( rec->eventArray[i] ) )) {
	  SetTriggerEnabledInEvent( rec->eventArray[i], FALSE );
	}
      }
    }

    return ret;            
}

static RET_VAL _UpdateReactionRateUpdateTime( SSA_WITH_USER_UPDATE_RECORD *rec ) {
    RET_VAL ret = SUCCESS;
    double time = rec->time;
    double rate = 0.0;
    SPECIES *species = NULL;
    IR_EDGE *edge = NULL;
    IR_EDGE *updateEdge = NULL;
    REACTION *reaction = NULL;
    LINKED_LIST *edges = NULL;
    LINKED_LIST *updateEdges = NULL;

    edges = GetReactantEdges( (IR_NODE*)rec->nextReaction );
    ResetCurrentElement( edges );
    while( ( edge = GetNextEdge( edges ) ) != NULL ) {
        species = GetSpeciesInIREdge( edge );        
        
        updateEdges = GetReactantEdges( (IR_NODE*)species );
        ResetCurrentElement( updateEdges );
        while( ( updateEdge = GetNextEdge( updateEdges ) ) != NULL ) {
            reaction = GetReactionInIREdge( updateEdge );
            if( IS_FAILED( ( ret = SetReactionRateUpdatedTime( reaction, time ) ) ) ) {
                return ret;                
            }
        }                
    
        updateEdges = GetModifierEdges( (IR_NODE*)species );
        ResetCurrentElement( updateEdges );
        while( ( updateEdge = GetNextEdge( updateEdges ) ) != NULL ) {
            reaction = GetReactionInIREdge( updateEdge );
            if( IS_FAILED( ( ret = SetReactionRateUpdatedTime( reaction, time ) ) ) ) {
                return ret;                
            }
        }                
    
        updateEdges = GetProductEdges( (IR_NODE*)species );
        ResetCurrentElement( updateEdges );
        while( ( updateEdge = GetNextEdge( updateEdges ) ) != NULL ) {
            reaction = GetReactionInIREdge( updateEdge );
            if( IS_FAILED( ( ret = SetReactionRateUpdatedTime( reaction, time ) ) ) ) {
                return ret;                
            }
        }                
    }    
        
    edges = GetProductEdges( (IR_NODE*)rec->nextReaction );
    ResetCurrentElement( edges );
    while( ( edge = GetNextEdge( edges ) ) != NULL ) {
        species = GetSpeciesInIREdge( edge );        
        
        updateEdges = GetReactantEdges( (IR_NODE*)species );
        ResetCurrentElement( updateEdges );
        while( ( updateEdge = GetNextEdge( updateEdges ) ) != NULL ) {
            reaction = GetReactionInIREdge( updateEdge );
            if( IS_FAILED( ( ret = SetReactionRateUpdatedTime( reaction, time ) ) ) ) {
                return ret;                
            }
        }                
    
        updateEdges = GetModifierEdges( (IR_NODE*)species );
        ResetCurrentElement( updateEdges );
        while( ( updateEdge = GetNextEdge( updateEdges ) ) != NULL ) {
            reaction = GetReactionInIREdge( updateEdge );
            if( IS_FAILED( ( ret = SetReactionRateUpdatedTime( reaction, time ) ) ) ) {
                return ret;                
            }
        }                
    
        updateEdges = GetProductEdges( (IR_NODE*)species );
        ResetCurrentElement( updateEdges );
        while( ( updateEdge = GetNextEdge( updateEdges ) ) != NULL ) {
            reaction = GetReactionInIREdge( updateEdge );
            if( IS_FAILED( ( ret = SetReactionRateUpdatedTime( reaction, time ) ) ) ) {
                return ret;                
            }
        }                
    }    
    return ret;            
}

static int _ComparePropensity( REACTION *a, REACTION *b ) {
    double d1 = 0.0;
    double d2 = 0.0;
    
    d1 = GetReactionRate( a );
    d2 = GetReactionRate( b );
    
    if( IS_REAL_EQUAL( d1, d2 ) ) {
        return 0;
    }
    return ( d1 < d2 ) ? -1 : 1;
}

 
 
 
/***************************************************************/ 
 
static BOOL _IsUpdateByUserInputReady( SSA_WITH_USER_UPDATE_RECORD *rec ) {
    return ( rec->time < rec->nextTimeSeriesSpeciesLevelUpdateTime ? FALSE : TRUE );
}
  
static RET_VAL _UpdateByUserInput( SSA_WITH_USER_UPDATE_RECORD *rec ) {
    RET_VAL ret = SUCCESS;
    TIME_SERIES_SPECIES_LEVEL_UPDATER *timeSeriesSpeciesLevelUpdater = rec->timeSeriesSpeciesLevelUpdater; 
    double updateTime = rec->nextTimeSeriesSpeciesLevelUpdateTime;    
    double nextTime = updateTime;
                
    rec->time = nextTime;    
    while( IS_REAL_EQUAL( updateTime, nextTime ) ) {
        if( IS_FAILED( ( ret = _Print( rec ) ) ) ) {
            return ret;            
        }
        if( IS_FAILED( ( ret = timeSeriesSpeciesLevelUpdater->Update( timeSeriesSpeciesLevelUpdater, nextTime ) ) ) ) {
            return ret;            
        }         
        updateTime = timeSeriesSpeciesLevelUpdater->GetNextUpdateTime( timeSeriesSpeciesLevelUpdater );
    }
    
    rec->nextTimeSeriesSpeciesLevelUpdateTime = updateTime;
    
    return ret;            
}

 
 
 
 
