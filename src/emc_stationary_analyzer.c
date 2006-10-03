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

#include "emc_stationary_analyzer.h"

static RET_VAL _Analyze( EMC_STATIONARY_ANALYZER *analyzer, int maxIteration );
static EMC *_GetResult( EMC_STATIONARY_ANALYZER *analyzer );
static RET_VAL _SetTolerance( EMC_STATIONARY_ANALYZER *analyzer, double tolerance );
static double _GetTolerance( EMC_STATIONARY_ANALYZER *analyzer );
static RET_VAL _ComputeNewDistribution( int stateSize, EMC_STATE *states );
static BOOL _UpdateDistribution( int jumpCount, int stateSize, EMC_STATE *states, double tolerance ); 
static EMC *_ConvertCTMC2EMC( CTMC *markovChain );
static RET_VAL _CalculateTransitionProbs( CTMC_STATE *state );



EMC_STATIONARY_ANALYZER *CreateEMCStationaryAnalyzer( CTMC *ctmc ) {
    EMC *markovChain = NULL;
    EMC_STATIONARY_ANALYZER *analyzer = NULL;
    EMC_STATE *states = NULL;
    EMC_STATIONARY_ANALYSIS_REC *rec = NULL;
    int i = 0;
    int size = 0;
    
    if( ( markovChain = _ConvertCTMC2EMC( ctmc ) ) == NULL ) {
        TRACE_0( "failed to convert CTMC to EMC" );
        return NULL;
    }
     
    if( ( analyzer = (EMC_STATIONARY_ANALYZER*)MALLOC( sizeof(EMC_STATIONARY_ANALYZER) ) ) == NULL ) {
        TRACE_0( "failed to create EMC analyzer" );
        return NULL;
    }
    
    states = markovChain->states;
    size = markovChain->stateSize;    
    for( i = 0; i < size; i++ ) {
        /* assuming IEEE 754 double.  */
        if( ( rec = (EMC_STATIONARY_ANALYSIS_REC*)CALLOC( 1, sizeof(EMC_STATIONARY_ANALYSIS_REC) ) ) == NULL ) {
            TRACE_0( "failed to create EMC analysis record" );
            return NULL;
        }
        
        states[i].analysisRecord = (CADDR_T)rec; 
    }
    rec = (EMC_STATIONARY_ANALYSIS_REC*)(markovChain->initialState->analysisRecord);
    rec->currentProb = 1.0;
            
    analyzer->markovChain = markovChain;
    analyzer->tolerance = DEFAULT_EMC_STATIONARY_ANALYSIS_TOLERANCE;   
    analyzer->GetResult = _GetResult;
    analyzer->Analyze = _Analyze;    
    analyzer->SetTolerance = _SetTolerance;
    analyzer->GetTolerance = _GetTolerance;
    return analyzer;
}

RET_VAL FreeEMCStationaryAnalyzer( EMC_STATIONARY_ANALYZER **panalyzer ) {
    int i = 0;
    EMC_STATIONARY_ANALYZER *analyzer = *panalyzer;
    EMC *markovChain = analyzer->markovChain;
    EMC_STATE *states = markovChain->states;
    int size = markovChain->stateSize;
    EMC_STATIONARY_ANALYSIS_REC *rec = NULL;

    for( i = 0; i < size; i++ ) {
        FREE( states[i].analysisRecord );
    }
    
    FREE( analyzer );
        
    return SUCCESS;
}

static RET_VAL _SetTolerance( EMC_STATIONARY_ANALYZER *analyzer, double tolerance ) {
    analyzer->tolerance = tolerance;
    return SUCCESS;
}

static double _GetTolerance( EMC_STATIONARY_ANALYZER *analyzer ) {
    return analyzer->tolerance;
}




static RET_VAL _Analyze( EMC_STATIONARY_ANALYZER *analyzer, int maxIteration ) {
    RET_VAL ret = SUCCESS;
    double time = 0.0;
    double tolerance = analyzer->tolerance;
    EMC *markovChain = analyzer->markovChain;
    EMC_STATE *states = markovChain->states;
    int stateSize = markovChain->stateSize; 
    int i = 1;
    BOOL done = FALSE;
    
    while( ( !done ) || ( i <= maxIteration ) ) {        
        if( IS_FAILED( ( ret = _ComputeNewDistribution( stateSize, states ) ) ) ) {
            return ret;
        }
        done = _UpdateDistribution( i, stateSize, states, tolerance ); 
        i++;
    }
}

static EMC *_GetResult( EMC_STATIONARY_ANALYZER *analyzer ) {
    return analyzer->markovChain;
}


static RET_VAL _ComputeNewDistribution( int stateSize, EMC_STATE *states ) {
    int i = 0;
    int j = 0;
    int transitionSize = 0; 
    double prob = 0.0;    
    double fromProb = 0.0;    
    EMC_STATE *to = NULL;
    EMC_STATE *from = NULL;
    VECTOR *transitions = NULL;
    EMC_TRANSITION *transition = NULL;
    EMC_STATIONARY_ANALYSIS_REC *rec = NULL;
    EMC_STATIONARY_ANALYSIS_REC *toRec = NULL;
    EMC_STATIONARY_ANALYSIS_REC *fromRec = NULL;
    
    for( i = 0; i < stateSize; i++ ) {
        rec = (EMC_STATIONARY_ANALYSIS_REC*)(states[i].analysisRecord);
        rec->newProb = 0.0;
    }
    for( i = 0; i < stateSize; i++ ) {
        from = states + i;
        fromRec = (EMC_STATIONARY_ANALYSIS_REC*)(from->analysisRecord);
        fromProb = rec->currentProb;
        transitions = from->transitions;
        transitionSize = GetVectorSize( transitions );
        for( j = 0; j < transitionSize; j++ ) {
            transition = (EMC_TRANSITION*)GetElementFromVector( transitions, j );
            toRec = (EMC_STATIONARY_ANALYSIS_REC*)(transition->to->analysisRecord);            
            toRec->newProb += (transition->prob * fromProb);
        }                
    }
    
    return SUCCESS;
}



static BOOL _UpdateDistribution( int jumpCount, int stateSize, EMC_STATE *states, double tolerance ) {
    int i = 0;
    BOOL canDone = TRUE;
    EMC_STATIONARY_ANALYSIS_REC *rec = NULL;

    /* check the end condition only for the even jump count */
    if( ( jumpCount & 0x01 ) == 0x00 ) {
        for( i = 0; i < stateSize; i++ ) {
            rec = (EMC_STATIONARY_ANALYSIS_REC*)(states[i].analysisRecord);
            if( canDone ) {
                if( fabs( rec->currentProb - rec->newProb ) >= tolerance ) {
                    canDone = FALSE;
                }
            }
            rec->currentProb = rec->newProb;
        }
        return canDone;
    }
    else {
        for( i = 0; i < stateSize; i++ ) {
            rec = (EMC_STATIONARY_ANALYSIS_REC*)(states[i].analysisRecord);
            rec->currentProb = rec->newProb;
        }
        return FALSE;
    }
}



static EMC *_ConvertCTMC2EMC( CTMC *markovChain ) {
    CTMC_STATE *states = NULL;
    CTMC_STATE *state = NULL;
    int i = 0;
    int size = 0;
    
    states = markovChain->states;
    size = markovChain->stateSize;    
    for( i = 0; i < size; i++ ) {
        state = states + i;
        if( IS_FAILED( ( _CalculateTransitionProbs( state ) ) ) ) {
            TRACE_0( "failed to calculate the transition probabilities" );
            return NULL;
        }
    }
    return (EMC*)markovChain;
}


static RET_VAL _CalculateTransitionProbs( CTMC_STATE *state ) {
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
        
    for( i = 0; i < size; i++ ) {
        transition = (CTMC_TRANSITION*)GetElementFromVector( transitions, i );
        transition->prob = transition->rate / outRate;
    }
    
    return SUCCESS;    
}



