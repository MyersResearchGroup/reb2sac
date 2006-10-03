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

#include "ctmc_analyzer.h"

static RET_VAL _ResetDt( CTMC_ANALYZER *analyzer, double dt );
static RET_VAL _Analyze( CTMC_ANALYZER *analyzer, double timeLimit );
static CTMC *_GetResult( CTMC_ANALYZER *analyzer );
static double _GetDt( CTMC_ANALYZER *analyzer );    

static double _CalculateOutRate( CTMC_STATE *state );
static double _CalculateDt( CTMC_ANALYZER *analyzer );
static RET_VAL _UpdateDistribution( int stateSize, CTMC_STATE *states );
  

CTMC_ANALYZER *CreateCTMCAnalyzer( CTMC *markovChain ) {
    CTMC_ANALYZER *analyzer = NULL;
    CTMC_STATE *states = NULL;
    CTMC_STATE *state = NULL;
    CTMC_ANALYSIS_REC *rec = NULL;
    int i = 0;
    int size = 0;
    double highestOutRate = 0.0;
    double outRate = 0.0;
    double dt = 0.0;
    
    if( ( analyzer = (CTMC_ANALYZER*)MALLOC( sizeof(CTMC_ANALYZER) ) ) == NULL ) {
        TRACE_0( "failed to create CTMC analyzer" );
        return NULL;
    }
    
    states = markovChain->states;
    size = markovChain->stateSize;    
    for( i = 0; i < size; i++ ) {
        /* assuming IEEE 754 double.  */
        if( ( rec = (CTMC_ANALYSIS_REC*)MALLOC( sizeof(CTMC_ANALYSIS_REC) ) ) == NULL ) {
            TRACE_0( "failed to create CTMC analysis record" );
        }
        rec->currentProb = 0.0;
        rec->newProb = 0.0;
        
        state = states + i;
        outRate = _CalculateOutRate( state );
        rec->outRate = outRate;
        if( outRate > highestOutRate ) {
            highestOutRate = outRate;
        }        
        state->analysisRecord = (CADDR_T)rec; 
    }
    rec = (CTMC_ANALYSIS_REC*)(markovChain->initialState->analysisRecord);
    rec->currentProb = 1.0;
            
    analyzer->markovChain = markovChain;
    analyzer->highestOutRate = highestOutRate;
    
    dt = _CalculateDt( analyzer );
    if( IS_FAILED( ( _ResetDt( analyzer, dt ) ) ) ) {
        TRACE_0( "failed to calculate dt for CTMC analysis" );    
    }
    analyzer->dt = dt;
    
    analyzer->ResetDt = _ResetDt;
    analyzer->GetDt = _GetDt;
    analyzer->GetResult = _GetResult;
    analyzer->Analyze = _Analyze;    
       
    return analyzer;
}

RET_VAL FreeCTMCAnalyzer( CTMC_ANALYZER **panalyzer ) {
    int i = 0;
    CTMC_ANALYZER *analyzer = *panalyzer;
    CTMC *markovChain = analyzer->markovChain;
    CTMC_STATE *states = markovChain->states;
    int size = markovChain->stateSize;
    CTMC_STATE *state = NULL;
    CTMC_ANALYSIS_REC *rec = NULL;

    for( i = 0; i < size; i++ ) {
        state = states + i;
        FREE( state->analysisRecord );        
    }
    
    FREE( analyzer );
        
    return SUCCESS;
}


static RET_VAL _ResetDt( CTMC_ANALYZER *analyzer, double dt ) {
    int i = 0;
    int j = 0;
    int size = 0; 
    int tranSize = 0; 
    double prob = 0.0;    
    CTMC *markovChain = analyzer->markovChain;
    VECTOR *transitions = NULL;
    CTMC_TRANSITION *transition = NULL;
    CTMC_STATE *states = markovChain->states;
    CTMC_STATE *state = NULL;
    CTMC_ANALYSIS_REC *rec = NULL;            
    
    
    size = markovChain->stateSize;
    for( i = 0; i < size; i++ ) {
        state = states + i;
        rec = (CTMC_ANALYSIS_REC*)(state->analysisRecord);
        prob = rec->outRate * dt;
        rec->noTranProb = ( prob > 1.0 ? 0.0 : ( 1.0 - prob ) );
        transitions = state->transitions;
        tranSize = GetVectorSize( transitions );
        for( j = 0; j < tranSize; j++ ) {
            transition = (CTMC_TRANSITION*)GetElementFromVector( transitions, j );
            prob = transition->rate * dt;
            transition->prob = ( prob > 1.0 ? 1.0 : prob ); 
        }
    }
    
    return SUCCESS;
}

static RET_VAL _Analyze( CTMC_ANALYZER *analyzer, double timeLimit ) {
    RET_VAL ret = SUCCESS;
    double time = 0.0;
    CTMC *markovChain = analyzer->markovChain;
    CTMC_STATE *states = markovChain->states;
    int stateSize = markovChain->stateSize; 
    double dt = analyzer->dt;
    
    while( time <= timeLimit ) {
        TRACE_1("at time %g", time );
        if( IS_FAILED( ( ret = _UpdateDistribution( stateSize, states ) ) ) ) {
            return ret;
        }
        time += dt;
    }
    
    return SUCCESS;
}

static CTMC *_GetResult( CTMC_ANALYZER *analyzer ) {
    return analyzer->markovChain;
}

static double _GetDt( CTMC_ANALYZER *analyzer ) {
    return analyzer->dt;
}


static double _CalculateOutRate( CTMC_STATE *state ) {
    double outRate = 0.0;
    VECTOR *transitions = state->transitions;
    CTMC_TRANSITION *transition = NULL;
    int size = 0; 
    int i = 0;
    
    size = GetVectorSize( transitions );
    for( i = 0; i < size; i++ ) {
        transition = (CTMC_TRANSITION*)GetElementFromVector( transitions, i );
        outRate += transition->rate;
    }
        
    return outRate;        
}

static double _CalculateDt( CTMC_ANALYZER *analyzer ) {
    double highestOutRate = analyzer->highestOutRate;
    double dt = CTMC_ANALYSIS_DT_RATE / highestOutRate;
    
    return dt; 
}


static RET_VAL _UpdateDistribution( int stateSize, CTMC_STATE *states ) {
    int i = 0;
    int j = 0;
    int transitionSize = 0; 
    double prob = 0.0;    
    double fromProb = 0.0;    
    CTMC_STATE *state = NULL;
    CTMC_STATE *to = NULL;
    CTMC_STATE *from = NULL;
    VECTOR *transitions = NULL;
    CTMC_TRANSITION *transition = NULL;
    CTMC_ANALYSIS_REC *rec = NULL;
    CTMC_ANALYSIS_REC *toRec = NULL;
    CTMC_ANALYSIS_REC *fromRec = NULL;
    
    for( i = 0; i < stateSize; i++ ) {
        state = states + i;
        rec = (CTMC_ANALYSIS_REC*)(state->analysisRecord);
        rec->newProb = rec->currentProb * rec->noTranProb;
    }
    for( i = 0; i < stateSize; i++ ) {
        from = states + i;
        fromRec = (CTMC_ANALYSIS_REC*)(from->analysisRecord);
        fromProb = fromRec->currentProb;
        transitions = from->transitions;
        transitionSize = GetVectorSize( transitions );
        for( j = 0; j < transitionSize; j++ ) {
            transition = (CTMC_TRANSITION*)GetElementFromVector( transitions, j );
            toRec = (CTMC_ANALYSIS_REC*)(transition->to->analysisRecord);            
            toRec->newProb += (transition->prob * fromProb);
        }                
    }
    
    for( i = 0; i < stateSize; i++ ) {
        state = states + i;
        rec = (CTMC_ANALYSIS_REC*)(state->analysisRecord);
        rec->currentProb = rec->newProb;
        TRACE_2("\tprobability of state %i is %g", i, rec->currentProb );
    }

    return SUCCESS;
}


