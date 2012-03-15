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
#include "type1pili_gillespie_ci.h"
#include "type_1_pili1_simulation_run_termination_decider.h"
#include "confidence_interval_stop_rule.h"


static BOOL _IsModelConditionSatisfied( IR *ir );


static RET_VAL _InitializeRecord( TYPE1PILI_GILLESPIE_CI_RECORD *rec, BACK_END_PROCESSOR *backend, IR *ir );
static RET_VAL _InitializeSimulation( TYPE1PILI_GILLESPIE_CI_RECORD *rec, int runNum );
static RET_VAL _RunSimulation( TYPE1PILI_GILLESPIE_CI_RECORD *rec );

static RET_VAL _CleanSimulation( TYPE1PILI_GILLESPIE_CI_RECORD *rec );
static RET_VAL _CleanRecord( TYPE1PILI_GILLESPIE_CI_RECORD *rec );

static RET_VAL _CalculateTotalPropensities( TYPE1PILI_GILLESPIE_CI_RECORD *rec );
static RET_VAL _CalculatePropensities( TYPE1PILI_GILLESPIE_CI_RECORD *rec );
static RET_VAL _CalculatePropensity( TYPE1PILI_GILLESPIE_CI_RECORD *rec, REACTION *reaction );
static RET_VAL _FindNextReactionTime( TYPE1PILI_GILLESPIE_CI_RECORD *rec );
static RET_VAL _FindNextReaction( TYPE1PILI_GILLESPIE_CI_RECORD *rec );
static RET_VAL _Update( TYPE1PILI_GILLESPIE_CI_RECORD *rec );
static RET_VAL _Print( TYPE1PILI_GILLESPIE_CI_RECORD *rec );
static RET_VAL _PrintStatistics( TYPE1PILI_GILLESPIE_CI_RECORD *rec, FILE *file);
static RET_VAL _UpdateNodeValues( TYPE1PILI_GILLESPIE_CI_RECORD *rec );
static RET_VAL _UpdateSpeciesValues( TYPE1PILI_GILLESPIE_CI_RECORD *rec );
static RET_VAL _UpdateReactionRateUpdateTime( TYPE1PILI_GILLESPIE_CI_RECORD *rec );

static int _ComparePropensity( REACTION *a, REACTION *b );
static BOOL _IsTerminationConditionMet( TYPE1PILI_GILLESPIE_CI_RECORD *rec );


static double _GetUniformRandom();



DLLSCOPE RET_VAL STDCALL DoGillespiesForType1PiliWithCI( BACK_END_PROCESSOR *backend, IR *ir ) {
    static TYPE1PILI_GILLESPIE_CI_RECORD rec;
    RET_VAL ret = SUCCESS;
    UINT i = 0;
    UINT runs = 1;
    char *namePrefix = NULL;
    char *valueString = NULL;
    int oldCount = 0;
    int totalCount = 0;
    double confidenceLevel = 0.95;
    double tolerance = 0.1;
    char filename[1024];
    FILE *report = NULL;
    TYPE_1_PILI1_SIMULATION_RUN_TERMINATION_DECIDER *decider = NULL;
    CONFIDENCE_INTERVAL_STOP_RULE *stopRule = NULL;
    COMPILER_RECORD_T *compRec = backend->record;
    REB2SAC_PROPERTIES *properties = NULL;
    
    START_FUNCTION("DoGillespiesForType1PiliWithCI");
    
    if( !_IsModelConditionSatisfied( ir ) ) {
        return ErrorReport( FAILING, "DoGillespiesForType1PiliWithCI", "Gillespie method cannot be applied to the model" );
    }
    
    if( IS_FAILED( ( ret = _InitializeRecord( &rec, backend, ir ) ) ) )  {
        return ErrorReport( ret, "DoGillespiesForType1PiliWithCI", "initialization of the record failed" );
    }
    
    runs = rec.runs;    
    decider = (TYPE_1_PILI1_SIMULATION_RUN_TERMINATION_DECIDER*)(rec.decider);
    
    properties = compRec->properties;
    if( ( valueString = properties->GetProperty( properties, MONTE_CARLO_CI_CONFIDENCE_LEVEL ) ) == NULL ) {
        confidenceLevel = DEFAULT_MONTE_CARLO_CI_CONFIDENCE_LEVEL_VALUE;
    }
    else {
        if( IS_FAILED( ( ret = StrToFloat( &confidenceLevel, valueString ) ) ) ) {
            confidenceLevel = DEFAULT_MONTE_CARLO_CI_CONFIDENCE_LEVEL_VALUE;
        }
    }    
    if( ( valueString = properties->GetProperty( properties, MONTE_CARLO_CI_ERROR_TOLERANCE ) ) == NULL ) {
        tolerance = DEFAULT_MONTE_CARLO_CI_ERROR_TOLERANCE_VALUE;
    }
    else {
        if( IS_FAILED( ( ret = StrToFloat( &tolerance, valueString ) ) ) ) {
            tolerance = DEFAULT_MONTE_CARLO_CI_ERROR_TOLERANCE_VALUE;
        }
    }        
    
    if( ( stopRule = CreateConfidenceIntervalStopRule( confidenceLevel, tolerance, runs ) ) == NULL ) {
        return ErrorReport( FAILING, "DoGillespiesForType1PiliWithCI", "failed to create confidence interval stop rule" );
    } 
    
    sprintf( filename, "%s%cci-report.txt", rec.outDir, FILE_SEPARATOR );        
    if( ( report = fopen( filename, "w" ) ) == NULL ) {
        return ErrorReport( FAILING, "DoGillespiesForType1PiliWithCI", "could not create a report file" );
    }
    do {
        oldCount = totalCount;
        for( i = 1; i <= runs; i++ ) {
            if( IS_FAILED( ( ret = _InitializeSimulation( &rec, i ) ) ) ) {
                return ErrorReport( ret, "DoGillespiesForType1PiliWithCI", "initialization of the %i-th simulation failed", i );
            }
            if( IS_FAILED( ( ret = _RunSimulation( &rec ) ) ) ) {
                return ErrorReport( ret, "DoGillespiesForType1PiliWithCI", "%i-th simulation failed at time %f", i, rec.time );
            }
            if( IS_FAILED( ( ret = _CleanSimulation( &rec ) ) ) ) {
                return ErrorReport( ret, "DoGillespiesForType1PiliWithCI", "cleaning of the %i-th simulation failed", i );
            }         
	    printf("Run = %d\n",i);
	    fflush(stdout);
        }
        totalCount = decider->GetChangeCount( decider );
        
        
        TRACE_1( "new sample is %g" NEW_LINE, (double)( totalCount - oldCount ) );
        
        if( IS_FAILED( ( ret = stopRule->AddNewSample( stopRule, (double)( totalCount - oldCount ) ) ) ) ) {
            return ErrorReport( ret, "DoGillespiesForType1PiliWithCI", "failed to add new sample" );
        }
        if( IS_FAILED( ( ret = stopRule->Report( stopRule, report ) ) ) ) {
            return ErrorReport( ret, "DoGillespiesForType1PiliWithCI", "failed to generate report" );
        }
        
    } while( !(stopRule->IsConditionMet( stopRule )) );
    /*    
    if( IS_FAILED( ( ret = stopRule->Report( stopRule, report ) ) ) ) {
        return ErrorReport( ret, "DoGillespiesForType1PiliWithCI", "failed to generate report" );
    }
    */
    fclose( report );
    FreeConfidenceIntervalStopRule( &stopRule );
    END_FUNCTION("DoGillespiesForType1PiliWithCI", SUCCESS );
    return ret;            
}

DLLSCOPE RET_VAL STDCALL CloseGillespiesForType1PiliWithCI( BACK_END_PROCESSOR *backend ) {
    RET_VAL ret = SUCCESS;
    TYPE1PILI_GILLESPIE_CI_RECORD *rec = (TYPE1PILI_GILLESPIE_CI_RECORD *)(backend->_internal1);
        
    START_FUNCTION("CloseGillespiesForType1PiliWithCI");
    
    if( IS_FAILED( ( ret = _CleanRecord( rec ) ) ) )  {
        return ErrorReport( ret, "CloseGillespiesForType1PiliWithCI", "cleaning of the record failed" );
    }
        
    END_FUNCTION("CloseGillespiesForType1PiliWithCI",  SUCCESS );
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


static RET_VAL _InitializeRecord( TYPE1PILI_GILLESPIE_CI_RECORD *rec, BACK_END_PROCESSOR *backend, IR *ir ) {
    RET_VAL ret = SUCCESS;
    UINT32 i = 0;
    UINT32 size = 0;
    char buf[512];
    char *valueString = NULL;
    SPECIES *species = NULL;
    SPECIES **speciesArray = NULL;
    REACTION *reaction = NULL;
    REACTION **reactions = NULL;
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
    
    if( ( rec->printer = CreateSimulationPrinter( backend, NULL, 0,
						  speciesArray, rec->speciesSize,
						  NULL, 0 ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not create simulation printer" );
    }                
    
    if( ( rec->decider = 
          CreateType1Pili1SimulationRunTerminationDecider( 
            backend, 
            speciesArray, 
            rec->speciesSize,
            reactions,
            rec->reactionsSize, 
            rec->timeLimit ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not create type 1 pili simulation terminator" );
    }
        
    backend->_internal1 = (CADDR_T)rec;
    
    return ret;            
}

static RET_VAL _InitializeSimulation( TYPE1PILI_GILLESPIE_CI_RECORD *rec, int runNum ) {
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
    char filename[512];
    FILE *file = NULL;
    
    SeedRandomNumberGenerators( rec->seed );
    rec->seed = GetNextUniformRandomNumber(0,RAND_MAX);
    SeedRandomNumberGenerators( rec->seed );
    
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

static RET_VAL _RunSimulation( TYPE1PILI_GILLESPIE_CI_RECORD *rec ) {
    RET_VAL ret = SUCCESS;
    int i = 0;
    REACTION *reaction = NULL;
    SIMULATION_PRINTER *printer = NULL;
    SIMULATION_RUN_TERMINATION_DECIDER *decider = NULL;
    
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
            if( IS_FAILED( ( ret = _Print( rec ) ) ) ) {
                return ret;            
            }
            reaction = NULL;
        }
        else { 
            if( IS_FAILED( ( ret = _FindNextReactionTime( rec ) ) ) ) {
                return ret;
            }
            if( IS_FAILED( ( ret = _FindNextReaction( rec ) ) ) ) {
                return ret;
            }
            reaction = rec->nextReaction;
            if( IS_FAILED( ( ret = _Update( rec ) ) ) ) {
                return ret;
            }
        }
    }
/*    
    if( IS_FAILED( ( ret = printer->PrintValues( printer, rec->timeLimit ) ) ) ) {
        return ret;
    }
*/
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

static RET_VAL _CleanSimulation( TYPE1PILI_GILLESPIE_CI_RECORD *rec ) {
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

static RET_VAL _CleanRecord( TYPE1PILI_GILLESPIE_CI_RECORD *rec ) {
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

static RET_VAL _PrintStatistics(TYPE1PILI_GILLESPIE_CI_RECORD *rec, FILE *file) {
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

static BOOL _IsTerminationConditionMet( TYPE1PILI_GILLESPIE_CI_RECORD *rec ) {
    return FALSE;
}



static RET_VAL _CalculateTotalPropensities( TYPE1PILI_GILLESPIE_CI_RECORD *rec ) {
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

static RET_VAL _CalculatePropensities( TYPE1PILI_GILLESPIE_CI_RECORD *rec ) {
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


static RET_VAL _CalculatePropensity( TYPE1PILI_GILLESPIE_CI_RECORD *rec, REACTION *reaction ) {
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


static RET_VAL _FindNextReactionTime( TYPE1PILI_GILLESPIE_CI_RECORD *rec ) {
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

static RET_VAL _FindNextReaction( TYPE1PILI_GILLESPIE_CI_RECORD *rec ) {
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

static RET_VAL _Update( TYPE1PILI_GILLESPIE_CI_RECORD *rec ) {
    RET_VAL ret = SUCCESS;

    if( IS_FAILED( ( ret = _Print( rec ) ) ) ) {
        return ret;            
    }
    if( IS_FAILED( ( ret = _UpdateNodeValues( rec ) ) ) ) {
        return ret;            
    }         
    
    return ret;            
}



static RET_VAL _Print( TYPE1PILI_GILLESPIE_CI_RECORD *rec ) {
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

static RET_VAL _UpdateNodeValues( TYPE1PILI_GILLESPIE_CI_RECORD *rec ) {
    RET_VAL ret = SUCCESS;
    
    if( IS_FAILED( ( ret = _UpdateSpeciesValues( rec ) ) ) ) {
        return ret;            
    }
    if( IS_FAILED( ( ret = _UpdateReactionRateUpdateTime( rec ) ) ) ) {
        return ret;            
    }
    
    return ret;            
}


static RET_VAL _UpdateSpeciesValues( TYPE1PILI_GILLESPIE_CI_RECORD *rec ) {
    RET_VAL ret = SUCCESS;
    double stoichiometry = 0;
    double amount = 0;    
    SPECIES *species = NULL;
    IR_EDGE *edge = NULL;
    REACTION *reaction = rec->nextReaction;
    LINKED_LIST *edges = NULL;
    KINETIC_LAW_EVALUATER *evaluator = rec->evaluator;

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
    return ret;            
}

static RET_VAL _UpdateReactionRateUpdateTime( TYPE1PILI_GILLESPIE_CI_RECORD *rec ) {
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

 
 
