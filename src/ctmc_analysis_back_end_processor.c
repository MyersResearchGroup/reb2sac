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
#include "ctmc_analysis_back_end_processor.h"
#include "markov_chain.h"
#include "ctmc_analyzer.h"
#include "ir2ctmc_transformer.h"
#include "strconv.h"
#include "markov_chain_analysis_properties.h"
#include "markov_analysis_result_reporter.h"

static double _FindTimeLimit( REB2SAC_PROPERTIES *properties );
static double _GetStateProb( CTMC_STATE *state );
static RET_VAL _ReportResults( FILE *file, CTMC *ctmc, IR2CTMC_TRANSFORMER *transformer );
static RET_VAL _PrintState( FILE *file, int state, int size, double *currentLevelArray, double prob );

RET_VAL ProcessCTMCAnalysisBackend( BACK_END_PROCESSOR *backend, IR *ir ) {
    RET_VAL ret = SUCCESS;
    char *filename = NULL;
    double timeLimit = 0.0;
    FILE *file = NULL;
    COMPILER_RECORD_T *record = backend->record;
    REB2SAC_PROPERTIES *properties = record->properties;
    CTMC *ctmc = NULL;
    CTMC_ANALYZER *analyzer = NULL;
    IR2CTMC_TRANSFORMER *transformer = NULL;
    MARKOV_ANALYSIS_RESULT_REPORTER *reporter = NULL;
    
    START_FUNCTION("ProcessCTMCAnalysisBackend");
    
    if( ( filename = backend->outputFilename ) == NULL ) {
        filename = DEFAULT_CTMC_ANALYSIS_OUTPUT_NAME;
    }
    if( ( file = fopen( filename, "w" ) ) == NULL ) {
        return ErrorReport( FAILING, "ProcessCTMCAnalysisBackend", "transient probability analysis file open error" ); 
    }
    
    if( ( transformer = CreateIR2CTMCTransformer( ir, properties ) ) == NULL ) {
        return ErrorReport( FAILING, "ProcessCTMCAnalysisBackend", "failed to transform IR to CTMC" );
    }
    
    /* start testing the transformer    
    ret = transformer->Print( transformer, file );
    fclose( file );
    return ret;
    end testing the transformer*/    
    
    if( ( ctmc = transformer->Generate( transformer ) ) == NULL ) {
        return ErrorReport( FAILING, "ProcessCTMCAnalysisBackend", "failed to generate CTMC" );
    } 
        
    timeLimit = _FindTimeLimit( properties );
    
    if( ( analyzer = CreateCTMCAnalyzer( ctmc ) ) == NULL ) {
        return ErrorReport( FAILING, "ProcessCTMCAnalysisBackend", "failed to create CTMC analyzer" );
    }
    
    /*
    if( IS_FAILED( ( ret = _ReportResults( file, ctmc, transformer ) ) ) ) {
        END_FUNCTION("ProcessCTMCAnalysisBackend", ret );
        return ret;
    }     
    return ret;
    */
    
    if( IS_FAILED( ( ret = analyzer->Analyze( analyzer, timeLimit ) ) ) ) {
        END_FUNCTION("ProcessCTMCAnalysisBackend", ret );
        return ret;
    }
    
    ctmc = analyzer->GetResult( analyzer );
    
    if( ( reporter = CreateMarkovAnalysisResultReporter( properties ) ) == NULL ) {
        return ErrorReport( FAILING, "ProcessCTMCAnalysisBackend", "could not create a markov analysis reporter" );
    } 
    if( IS_FAILED( ( ret = reporter->Report( reporter, file, (MARKOV_CHAIN*)ctmc, (CADDR_T)transformer ) ) ) ) {
        END_FUNCTION("ProcessCTMCAnalysisBackend", ret );
        return ret;
    }
    /*    
    if( IS_FAILED( ( ret = _ReportResults( file, ctmc, transformer ) ) ) ) {
        END_FUNCTION("ProcessCTMCAnalysisBackend", ret );
        return ret;
    } 
    */
    END_FUNCTION("ProcessCTMCAnalysisBackend", ret );
    
    return ret;
}


RET_VAL CloseCTMCAnalysisBackend( BACK_END_PROCESSOR *backend ) {
    RET_VAL ret = SUCCESS;
    
    return ret;
}


static double _FindTimeLimit( REB2SAC_PROPERTIES *properties ) {
    char *valueString = NULL;
    double timeLimit = 0.0;
    
    if( ( valueString = properties->GetProperty( properties, MARKOV_CHAIN_ANALYSIS_TIME_LIMIT_KEY ) ) == NULL ) {
        timeLimit = DEFAULT_MARKOV_CHAIN_ANALYSIS_TIME_LIMIT;
    }
    else {
        if( IS_FAILED( ( StrToFloat( &timeLimit, valueString ) ) ) ) {
            timeLimit = DEFAULT_MARKOV_CHAIN_ANALYSIS_TIME_LIMIT;
        }
    }
    
    return timeLimit;    
}

static double _GetStateProb( CTMC_STATE *state ) {
    CTMC_ANALYSIS_REC *rec = (CTMC_ANALYSIS_REC*)(state->analysisRecord);
    return rec->currentProb;    
}


static RET_VAL _ReportResults( FILE *file, CTMC *ctmc, IR2CTMC_TRANSFORMER *transformer ) {
    RET_VAL ret = SUCCESS;
    int i = 0;
    int stateSize = ctmc->stateSize;
    int criticalLevelArraySize = 0;
    double prob = 0.0;
    double *currentLevelArray = NULL;
    SPECIES *species = NULL;
    SPECIES_CRITICAL_LEVEL *criticalLevel = NULL;
    SPECIES_CRITICAL_LEVEL **criticalLevelArray = NULL;
    CTMC_STATE *state = NULL;
    CTMC_STATE *states = ctmc->states;
    
    criticalLevelArraySize = transformer->GetCriticalLevelArraySize( transformer );
    criticalLevelArray = transformer->GetCriticalLevelArray( transformer );
    
    fprintf( file, "# state-id" );
    for( i = 0; i < criticalLevelArraySize; i++ ) {
        criticalLevel = criticalLevelArray[i];
        species = criticalLevel->species;
        fprintf( file, " %s", GetCharArrayOfString( GetSpeciesNodeName( species ) ) );        
    }
    fprintf( file, " probability" NEW_LINE );
             
    currentLevelArray = transformer->GetCurrentLevelArray( transformer );    
    if( IS_FAILED( ( ret = transformer->ResetCurrentLevelArray( transformer ) ) ) ) {
        return ret;
    }
    state = states + 0;
    prob = _GetStateProb( states );
    if( IS_FAILED( ( ret = _PrintState( file, 0, criticalLevelArraySize, currentLevelArray, prob ) ) ) ) {
        return ret;
    } 
    for( i = 1; i < stateSize; i++ ){        
        transformer->IncrementCurrentLevelArray( transformer );
        state = states + i;
        prob = _GetStateProb( state );
        if( IS_FAILED( ( ret = _PrintState( file, i, criticalLevelArraySize, currentLevelArray, prob ) ) ) ) {
            return ret;
        } 
    }
        
    return ret;
}

static RET_VAL _PrintState( FILE *file, int state, int size, double *currentLevelArray, double prob ) {
    RET_VAL ret = SUCCESS;
    int i = 0;
    
    fprintf( file, "%i", state );
    for( ; i < size; i++ ) {
        fprintf( file, " %g", currentLevelArray[i] );        
    }
    fprintf( file, " %g" NEW_LINE, prob );
    
    return ret;
}


