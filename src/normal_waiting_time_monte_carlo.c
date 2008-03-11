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
#include "normal_waiting_time_monte_carlo.h"




static BOOL _IsModelConditionSatisfied( IR *ir );


static RET_VAL _InitializeRecord( NORMAL_WAITING_TIME_MONTE_CARLO_RECORD *rec, BACK_END_PROCESSOR *backend, IR *ir );
static RET_VAL _InitializeSimulation( NORMAL_WAITING_TIME_MONTE_CARLO_RECORD *rec, int runNum );
static RET_VAL _RunSimulation( NORMAL_WAITING_TIME_MONTE_CARLO_RECORD *rec );

static RET_VAL _CleanSimulation( NORMAL_WAITING_TIME_MONTE_CARLO_RECORD *rec );
static RET_VAL _CleanRecord( NORMAL_WAITING_TIME_MONTE_CARLO_RECORD *rec );

static RET_VAL _CalculateTotalPropensities( NORMAL_WAITING_TIME_MONTE_CARLO_RECORD *rec );
static RET_VAL _CalculatePropensities( NORMAL_WAITING_TIME_MONTE_CARLO_RECORD *rec );
static RET_VAL _CalculatePropensity( NORMAL_WAITING_TIME_MONTE_CARLO_RECORD *rec, REACTION *reaction );
static RET_VAL _FindNextReactionTime( NORMAL_WAITING_TIME_MONTE_CARLO_RECORD *rec );
static RET_VAL _FindNextReaction( NORMAL_WAITING_TIME_MONTE_CARLO_RECORD *rec );
static RET_VAL _Update( NORMAL_WAITING_TIME_MONTE_CARLO_RECORD *rec );
static RET_VAL _Print( NORMAL_WAITING_TIME_MONTE_CARLO_RECORD *rec );
static RET_VAL _UpdateNodeValues( NORMAL_WAITING_TIME_MONTE_CARLO_RECORD *rec );
static RET_VAL _UpdateSpeciesValues( NORMAL_WAITING_TIME_MONTE_CARLO_RECORD *rec );
static RET_VAL _UpdateReactionRateUpdateTime( NORMAL_WAITING_TIME_MONTE_CARLO_RECORD *rec );

static int _ComparePropensity( REACTION *a, REACTION *b );
static BOOL _IsTerminationConditionMet( NORMAL_WAITING_TIME_MONTE_CARLO_RECORD *rec );

static void fireEvent( EVENT *event, NORMAL_WAITING_TIME_MONTE_CARLO_RECORD *rec );

static double _GetUniformRandom();



DLLSCOPE RET_VAL STDCALL DoNormalWaitingTimeMonteCarloAnalysis( BACK_END_PROCESSOR *backend, IR *ir ) {
    RET_VAL ret = SUCCESS;
    UINT i = 0;
    UINT runs = 1;
    char *namePrefix = NULL;
    static NORMAL_WAITING_TIME_MONTE_CARLO_RECORD rec;
    
    START_FUNCTION("DoNormalWaitingTimeMonteCarloAnalysis");
    
    if( !_IsModelConditionSatisfied( ir ) ) {
        return ErrorReport( FAILING, "DoNormalWaitingTimeMonteCarloAnalysis", "NormalWaitingTime method cannot be applied to the model" );
    }
    
    if( IS_FAILED( ( ret = _InitializeRecord( &rec, backend, ir ) ) ) )  {
        return ErrorReport( ret, "DoNormalWaitingTimeMonteCarloAnalysis", "initialization of the record failed" );
    }
    
    runs = rec.runs;    
    for( i = 1; i <= runs; i++ ) {
        if( IS_FAILED( ( ret = _InitializeSimulation( &rec, i ) ) ) ) {
            return ErrorReport( ret, "DoNormalWaitingTimeMonteCarloAnalysis", "initialization of the %i-th simulation failed", i );
        }
        if( IS_FAILED( ( ret = _RunSimulation( &rec ) ) ) ) {
            return ErrorReport( ret, "DoNormalWaitingTimeMonteCarloAnalysis", "%i-th simulation failed at time %f", i, rec.time );
        }
        if( IS_FAILED( ( ret = _CleanSimulation( &rec ) ) ) ) {
            return ErrorReport( ret, "DoNormalWaitingTimeMonteCarloAnalysis", "cleaning of the %i-th simulation failed", i );
        }         
      printf("Run = %d\n",i);
      fflush(stdout);
    }
    END_FUNCTION("DoNormalWaitingTimeMonteCarloAnalysis", SUCCESS );
    return ret;            
}

DLLSCOPE RET_VAL STDCALL CloseNormalWaitingTimeMonteCarloAnalyzer( BACK_END_PROCESSOR *backend ) {
    RET_VAL ret = SUCCESS;
    NORMAL_WAITING_TIME_MONTE_CARLO_RECORD *rec = (NORMAL_WAITING_TIME_MONTE_CARLO_RECORD *)(backend->_internal1);
        
    START_FUNCTION("CloseNormalWaitingTimeMonteCarloAnalyzer");
    
    if( IS_FAILED( ( ret = _CleanRecord( rec ) ) ) )  {
        return ErrorReport( ret, "CloseNormalWaitingTimeMonteCarloAnalyzer", "cleaning of the record failed" );
    }
        
    END_FUNCTION("CloseNormalWaitingTimeMonteCarloAnalyzer",  SUCCESS );
    return ret;            
}


static BOOL _IsModelConditionSatisfied( IR *ir ) {
    REACTION *reaction = NULL;
    LINKED_LIST *reactions = NULL;
    
    reactions = ir->GetListOfReactionNodes( ir );
    while( ( reaction = (REACTION*)GetNextFromLinkedList( reactions ) ) != NULL ) {
        if( IsReactionReversibleInReactionNode( reaction ) ) {
            TRACE_0("the input model contains reversible reaction(s), and cannot be used for NormalWaitingTime method" );            
            return FALSE;
        }
    }
    return TRUE;
}


static RET_VAL _InitializeRecord( NORMAL_WAITING_TIME_MONTE_CARLO_RECORD *rec, BACK_END_PROCESSOR *backend, IR *ir ) {
    RET_VAL ret = SUCCESS;
    UINT32 i = 0;
    UINT32 size = 0;
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
    REB2SAC_SYMBOL *symbol = NULL;
    REB2SAC_SYMBOL **symbolArray = NULL;
    REB2SAC_SYMTAB *symTab;
    COMPARTMENT *compartment = NULL;
    COMPARTMENT **compartmentArray = NULL;
    COMPARTMENT_MANAGER *compartmentManager;
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
    
    if( ( valueString = properties->GetProperty( properties, MONTE_CARLO_SIMULATION_START_INDEX ) ) == NULL ) {
        rec->startIndex = DEFAULT_MONTE_CARLO_SIMULATION_START_INDEX;
    }
    else {
      if( IS_FAILED( ( ret = StrToUINT32( (UINT32*)&(rec->startIndex), valueString ) ) ) ) {
            rec->startIndex = DEFAULT_MONTE_CARLO_SIMULATION_START_INDEX;
        }
    }    

    
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
    
    if( ( rec->printer = CreateSimulationPrinter( backend, speciesArray, rec->speciesSize ) ) == NULL ) {
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
					       rec->constraintArray, rec->constraintsSize, rec->evaluator, FALSE, rec->timeLimit ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not create simulation printer" );
    }
        
    if( ( rec->randomGen = CreateRandomNumberGenerator() ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not create random number generator" );
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
    
    backend->_internal1 = (CADDR_T)rec;
    
    return ret;            
}

static RET_VAL _InitializeSimulation( NORMAL_WAITING_TIME_MONTE_CARLO_RECORD *rec, int runNum ) {
    RET_VAL ret = SUCCESS;
    char filenameStem[512];
    double amount = 0;
    double param = 0;
    UINT32 i = 0;
    UINT32 size = 0;
    SPECIES *species = NULL;
    SPECIES **speciesArray = rec->speciesArray;
    REACTION *reaction = NULL;
    REACTION **reactionArray = rec->reactionArray;
    KINETIC_LAW_EVALUATER *evaluator = rec->evaluator;
    SIMULATION_PRINTER *printer = rec->printer;
    double compSize = 0.0;
    COMPARTMENT *compartment = NULL;
    COMPARTMENT **compartmentArray = rec->compartmentArray;
    REB2SAC_SYMBOL *symbol = NULL;
    REB2SAC_SYMBOL **symbolArray = rec->symbolArray;
    
    srand( rec->seed );
    rec->seed = rand();
    srand( rec->seed );
    
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
        if( IsInitialQuantityInAmountInSpeciesNode( species ) ) {
            amount = GetInitialAmountInSpeciesNode( species );
        }
        else {
            /* need to multiply concentration by the volume.  But not doing it yet */
            amount = GetInitialConcentrationInSpeciesNode( species ); 
        }
        if( IS_FAILED( ( ret = SetAmountInSpeciesNode( species, amount ) ) ) ) {
            return ret;            
        }
    }
            
    size = rec->reactionsSize;
    for( i = 0; i < size; i++ ) {
        reaction = reactionArray[i];
        if( IS_FAILED( ( ret = SetReactionRateUpdatedTime( reaction, 0.0 ) ) ) ) {
            return ret;
        }
        if( IS_FAILED( ( ret = ResetReactionFireCount( reaction ) ) ) ) {
            return ret;
        }        
    }
    size = rec->compartmentsSize;        
    for( i = 0; i < size; i++ ) {
        compartment = compartmentArray[i];
	compSize = GetSizeInCompartment( compartment );
        if( IS_FAILED( ( ret = SetCurrentSizeInCompartment( compartment, compSize ) ) ) ) {
            return ret;            
        }
    }
    size = rec->symbolsSize;        
    for( i = 0; i < size; i++ ) {
        symbol = symbolArray[i];
	param = GetRealValueInSymbol( symbol );
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
    
    return ret;            
}

static RET_VAL _RunSimulation( NORMAL_WAITING_TIME_MONTE_CARLO_RECORD *rec ) {
    RET_VAL ret = SUCCESS;
    int i = 0;
    int j = 0;
    REACTION *reaction = NULL;
    double timeLimit = rec->timeLimit;
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
            rec->time += rec->printInterval; 
            if( IS_FAILED( ( ret = _Print( rec ) ) ) ) {
                return ret;            
            }
            reaction = NULL;
        }
        else { 
            if( IS_FAILED( ( ret = _FindNextReactionTime( rec ) ) ) ) {
                return ret;
            }
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
	      if (rec->time < timeLimit) {
		if( IS_FAILED( ( ret = _FindNextReaction( rec ) ) ) ) {
		  return ret;
		}
		reaction = rec->nextReaction;
		if( IS_FAILED( ( ret = _Update( rec ) ) ) ) {
		  return ret;
		}
	      } else {
		if( IS_FAILED( ( ret = _Print( rec ) ) ) ) {
		  return ret;            
		}
	      }
	    }
        }
    }
/*    
    if( IS_FAILED( ( ret = printer->PrintValues( printer, rec->timeLimit ) ) ) ) {
        return ret;
    }
*/
    if( rec->time >= timeLimit ) {
        rec->time = timeLimit;
    } 
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

static RET_VAL _CleanSimulation( NORMAL_WAITING_TIME_MONTE_CARLO_RECORD *rec ) {
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

static RET_VAL _CleanRecord( NORMAL_WAITING_TIME_MONTE_CARLO_RECORD *rec ) {
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

static BOOL _IsTerminationConditionMet( NORMAL_WAITING_TIME_MONTE_CARLO_RECORD *rec ) {
    return FALSE;
}



static RET_VAL _CalculateTotalPropensities( NORMAL_WAITING_TIME_MONTE_CARLO_RECORD *rec ) {
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

static RET_VAL _CalculatePropensities( NORMAL_WAITING_TIME_MONTE_CARLO_RECORD *rec ) {
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
            if( IS_FAILED( ( ret = _CalculatePropensity( rec, reactionArray[i] ) ) ) ) {
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


static RET_VAL _CalculatePropensity( NORMAL_WAITING_TIME_MONTE_CARLO_RECORD *rec, REACTION *reaction ) {
    RET_VAL ret = SUCCESS;
    long stoichiometry = 0;
    long amount = 0;    
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
        stoichiometry = (long)GetStoichiometryInIREdge( edge );
        species = GetSpeciesInIREdge( edge );
        amount = (long)GetAmountInSpeciesNode( species );
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
        stoichiometry = (long)GetStoichiometryInIREdge( edge );
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


static RET_VAL _FindNextReactionTime( NORMAL_WAITING_TIME_MONTE_CARLO_RECORD *rec ) {
    RET_VAL ret = SUCCESS;
    double random = 0.0;
    double t = 0.0;
    double average = 0.0;
    RANDOM_NUMBER_GENERATOR *randomGen = rec->randomGen;
    
    average = 1.0 / rec->totalPropensities; 
    t = randomGen->GetNextNormal( average, sqrt( average ) );
    rec->time += t;
    rec->t = t;
    if( rec->time > rec->timeLimit ) {
        rec->time = rec->timeLimit;
    }
    TRACE_1( "time to next reaction is %f", t );
    return ret;            
}

static RET_VAL _FindNextReaction( NORMAL_WAITING_TIME_MONTE_CARLO_RECORD *rec ) {
    RET_VAL ret = SUCCESS;
    UINT32 i = 0;
    UINT32 size = rec->reactionsSize - 1;
    double random = 0.0;
    double threshold = 0.0;
    double sum = 0.0;
    REACTION *reaction = NULL;
    REACTION **reactionArray = rec->reactionArray;
    
    random = _GetUniformRandom();
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

static RET_VAL _Update( NORMAL_WAITING_TIME_MONTE_CARLO_RECORD *rec ) {
    RET_VAL ret = SUCCESS;

    if( IS_FAILED( ( ret = _Print( rec ) ) ) ) {
        return ret;            
    }
    if( IS_FAILED( ( ret = _UpdateNodeValues( rec ) ) ) ) {
        return ret;            
    }         
    
    return ret;            
}



static RET_VAL _Print( NORMAL_WAITING_TIME_MONTE_CARLO_RECORD *rec ) {
    RET_VAL ret = SUCCESS;
    double nextPrintTime = rec->nextPrintTime;
    double time = rec->time;
    double printInterval = rec->printInterval;
    REACTION *reaction = NULL;
    SIMULATION_PRINTER *printer = rec->printer;

    while( nextPrintTime < time ) {
        if( IS_FAILED( ( ret = printer->PrintValues( printer, nextPrintTime ) ) ) ) {
            return ret;
        }
        nextPrintTime += printInterval; 
    }     
    rec->nextPrintTime = nextPrintTime;
    return ret;            
}

static RET_VAL _UpdateNodeValues( NORMAL_WAITING_TIME_MONTE_CARLO_RECORD *rec ) {
    RET_VAL ret = SUCCESS;
    
    if( IS_FAILED( ( ret = _UpdateSpeciesValues( rec ) ) ) ) {
        return ret;            
    }
    if( IS_FAILED( ( ret = _UpdateReactionRateUpdateTime( rec ) ) ) ) {
        return ret;            
    }
    
    return ret;            
}


static void fireEvent( EVENT *event, NORMAL_WAITING_TIME_MONTE_CARLO_RECORD *rec ) {
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

static RET_VAL _UpdateSpeciesValues( NORMAL_WAITING_TIME_MONTE_CARLO_RECORD *rec ) {
    RET_VAL ret = SUCCESS;
    long stoichiometry = 0;
    double amount = 0;    
    SPECIES *species = NULL;
    IR_EDGE *edge = NULL;
    REACTION *reaction = rec->nextReaction;
    LINKED_LIST *edges = NULL;
    KINETIC_LAW_EVALUATER *evaluator = rec->evaluator;
    double change = 0;    
    UINT i = 0;
    UINT j = 0;
    double deltaTime;
    BOOL triggerEnabled;

    for (i = 0; i < rec->rulesSize; i++) {
      if ( GetRuleType( rec->ruleArray[i] ) == RULE_TYPE_RATE ) {
	for (j = 0; j < rec->speciesSize; j++) {
	  if ( strcmp( GetCharArrayOfString(GetRuleVar( rec->ruleArray[i] )),
		       GetCharArrayOfString(GetSpeciesNodeID( rec->speciesArray[j] ) ) ) == 0 ) {
	    /* Not technically correct as only calculated when a reaction fires */
	    change = rec->evaluator->EvaluateWithCurrentAmounts( rec->evaluator, 
								 (KINETIC_LAW*)GetMathInRule( rec->ruleArray[i] ) );
	    amount = GetAmountInSpeciesNode( rec->speciesArray[j] );
	    amount += (change * rec->t);
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
	    amount += (change * rec->t);
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
	    amount += (change * rec->t);
	    SetCurrentRealValueInSymbol( rec->symbolArray[j], amount );
	    break;
	  }
	}
      }
    }
    edges = GetReactantEdges( (IR_NODE*)reaction );
    ResetCurrentElement( edges );
    while( ( edge = GetNextEdge( edges ) ) != NULL ) {
        stoichiometry = (long)GetStoichiometryInIREdge( edge );
        species = GetSpeciesInIREdge( edge );
	if (HasBoundaryConditionInSpeciesNode(species)) continue;
        amount = GetAmountInSpeciesNode( species ) - (double)stoichiometry;
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
        stoichiometry = (long)GetStoichiometryInIREdge( edge );
        species = GetSpeciesInIREdge( edge );
	if (HasBoundaryConditionInSpeciesNode(species)) continue;
        amount = GetAmountInSpeciesNode( species ) + (double)stoichiometry;
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
	}
      }
    }
    for (j = 0; j < rec->symbolsSize; j++) {
      if (strcmp(GetCharArrayOfString( GetSymbolID(rec->symbolArray[j]) ),"t")==0) {
	SetCurrentRealValueInSymbol( rec->symbolArray[j], rec->time );
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
	  } else {
	    fireEvent( rec->eventArray[i], rec );
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

static RET_VAL _UpdateReactionRateUpdateTime( NORMAL_WAITING_TIME_MONTE_CARLO_RECORD *rec ) {
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



static double _GetUniformRandom() {
    int value = 0; 
    double uniformRandom = 0.0;
    value = rand();
    uniformRandom = (value + 0.999999) / ( RAND_MAX + 1.0 );
    return uniformRandom;
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

 
 
