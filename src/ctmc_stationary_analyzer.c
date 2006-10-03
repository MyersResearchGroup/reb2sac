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

#include "ctmc_stationary_analyzer.h"
#include "emc_stationary_analyzer.h"

static RET_VAL _Analyze( CTMC_STATIONARY_ANALYZER *analyzer );
static CTMC *_GetResult( CTMC_STATIONARY_ANALYZER *analyzer );
static RET_VAL _SetIterationMax( CTMC_STATIONARY_ANALYZER *analyzer, int iterationMax );
static int _GetIterationMax( CTMC_STATIONARY_ANALYZER *analyzer );
static CTMC *_ConvertEMCResultToCTMCResult( EMC *emc );

CTMC_STATIONARY_ANALYZER *CreateCTMCStationaryAnalyzer( CTMC *markovChain ) {
    CTMC_STATIONARY_ANALYZER *analyzer = NULL;
    CTMC_STATE **states = NULL;
    CTMC_STATE *state = NULL;
    CTMC_STATIONARY_ANALYSIS_REC *rec = NULL;
    int i = 0;
    int size = 0;
    double highestOutRate = 0.0;
    double outRate = 0.0;
    double dt = 0.0;
    
    if( ( analyzer = (CTMC_STATIONARY_ANALYZER*)MALLOC( sizeof(CTMC_STATIONARY_ANALYZER) ) ) == NULL ) {
        TRACE_0( "failed to create CTMC analyzer" );
        return NULL;
    }
    
    analyzer->markovChain = markovChain;
    analyzer->iterationMax = DEFAULT_CTMC_ITERATION_MAX;
    analyzer->GetResult = _GetResult;
    analyzer->Analyze = _Analyze;    
       
    return analyzer;
}

RET_VAL FreeCTMCStationaryAnalyzer( CTMC_STATIONARY_ANALYZER **panalyzer ) {
    CTMC_STATIONARY_ANALYZER *analyzer;
    FREE( analyzer );
        
    return SUCCESS;
}



static RET_VAL _Analyze( CTMC_STATIONARY_ANALYZER *analyzer ) {
    RET_VAL ret = SUCCESS;
    CTMC *ctmc = analyzer->markovChain;
    EMC *emc = NULL;
    EMC_STATIONARY_ANALYZER *emcAnalyzer = NULL;
    
    
    if( ( emcAnalyzer = CreateEMCStationaryAnalyzer( ctmc ) ) == NULL ) {
        return FAILING;
    }
    if( IS_FAILED( ( ret = emcAnalyzer->Analyze( emcAnalyzer, analyzer->iterationMax ) ) ) ) {
        return ret;    
    }
    emc = emcAnalyzer->GetResult( emcAnalyzer );
    analyzer->markovChain = _ConvertEMCResultToCTMCResult( emc ); 
    
    return SUCCESS;
}

static CTMC *_GetResult( CTMC_STATIONARY_ANALYZER *analyzer ) {
    return analyzer->markovChain;
}

static RET_VAL _SetIterationMax( CTMC_STATIONARY_ANALYZER *analyzer, int iterationMax ) {
    analyzer->iterationMax = iterationMax;
}

static int _GetIterationMax( CTMC_STATIONARY_ANALYZER *analyzer ) {
    return analyzer->iterationMax;
}



static double _CalculateOutRate( CTMC_STATE *state ) {
    int i = 0;
    double outRate = 0.0;
    VECTOR *transitions = state->transitions;
    CTMC_TRANSITION *transition = NULL;
    int size = 0; 
    
    size = GetVectorSize( transitions );
    for( i = 0; i < size; i++ ) {
        transition = (CTMC_TRANSITION*)GetElementFromVector( transitions, i );
        outRate += transition->rate;
    }
 
    return outRate;        
}

static CTMC *_ConvertEMCResultToCTMCResult( EMC *emc ) {
    EMC_STATIONARY_ANALYSIS_REC *rec = NULL;
    EMC_STATE *states = emc->states;
    EMC_STATE *state = NULL;
    int i = 0;
    int stateSize = emc->stateSize;
    VECTOR *transitions = NULL;
    EMC_TRANSITION *transition = NULL;
    int j = 0;
    int tranSize = 0;
    double outRate = 0.0;
    double currentProb = 0.0;
    double norm1 = 0.0;
                
    for( i = 0; i < stateSize; i++ ) {
        transitions = states[i].transitions;
        outRate = 0.0;
        tranSize = GetVectorSize( transitions );
        for( j = 0; j < tranSize; j++ ) {
            transition = (EMC_TRANSITION*)GetElementFromVector( transitions, j );
            outRate += transition->rate;
        }
        rec = (EMC_STATIONARY_ANALYSIS_REC*)(state->analysisRecord);
        currentProb = rec->currentProb / outRate;
        norm1 += currentProb;
        rec->currentProb = currentProb;
    }
    for( i = 0; i < stateSize; i++ ) {
        rec = (EMC_STATIONARY_ANALYSIS_REC*)(states[i].analysisRecord);
        rec->currentProb = rec->currentProb / norm1;
    }
    
    return (CTMC*)emc;    
}




