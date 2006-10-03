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

#include "markov_chain.h"
static RET_VAL _CalculateTransitionProbs( CTMC_STATE *state );

static CTMC_STATE *_CreateStates( CTMC_GENERATOR *gen, int size );
static RET_VAL _SetInitialState( CTMC_GENERATOR *gen, int index );
static RET_VAL _SetStateData( CTMC_GENERATOR *gen, int index, CADDR_T data );
static RET_VAL _AddTransition( CTMC_GENERATOR *gen, int fromIndex, int toIndex, double rate );        
static CTMC *_Generate( CTMC_GENERATOR *gen );        
static RET_VAL _ReleaseResource( CTMC_GENERATOR *gen );
 
 
CTMC_GENERATOR *CreateCTMCGenerator() {
    CTMC_GENERATOR *gen = NULL;
    
    START_FUNCTION("CreateCTMCGenerator");
    if( ( gen = (CTMC_GENERATOR*)MALLOC( sizeof(CTMC_GENERATOR) ) ) == NULL ) {
        END_FUNCTION("CreateCTMCGenerator", FAILING );
        return NULL;
    }
    
    gen->CreateStates = _CreateStates;
    gen->SetInitialState = _SetInitialState;
    gen->SetStateData = _SetStateData;
    gen->AddTransition = _AddTransition;
    gen->Generate = _Generate;
    gen->ReleaseResource = _ReleaseResource;
    
    END_FUNCTION("CreateCTMCGenerator", SUCCESS);
    return gen;
}

RET_VAL FreeCTMCGenerator( CTMC_GENERATOR **pGen) {
    RET_VAL ret = SUCCESS;
    
    CTMC_GENERATOR *gen = *pGen;
    
    if( IS_FAILED( ( ret = gen->ReleaseResource( gen ) ) ) ) {
        return ret;
    }
    FREE( gen );
    return SUCCESS;
}


static CTMC_STATE *_CreateStates( CTMC_GENERATOR *gen, int size ) {
    int i = 0;
    CTMC_STATE *states = NULL;
    
    if( ( states = (CTMC_STATE*)MALLOC( size * sizeof(CTMC_STATE) ) ) == NULL ) {
        return NULL;
    }
    for( i = 0; i < size; i++ ) {
        if( ( states[i].transitions = CreateVector() ) == NULL ) {
            TRACE_0("could not create a transition vector");
            return NULL;
        }
    }
    gen->states = states;
    gen->stateSize = size;
    
    return states;    
}

static RET_VAL _SetInitialState( CTMC_GENERATOR *gen, int index ) {
    if( ( index < 0 ) || ( gen->stateSize <= index ) ) {
        return FAILING;
    } 
    
    gen->initialState = (gen->states) + index;

    return SUCCESS;
}

static RET_VAL _SetStateData( CTMC_GENERATOR *gen, int index, CADDR_T data ) {
    if( ( 0 < index ) || ( gen->stateSize >= index ) ) {
        return FAILING;
    } 
    
    (gen->states)[index].data = data;
    
    return SUCCESS;
}

static RET_VAL _AddTransition( CTMC_GENERATOR *gen, int fromIndex, int toIndex, double rate ) {
    RET_VAL ret = SUCCESS;
    CTMC_STATE *from = NULL;
    CTMC_STATE *to = NULL;
    CTMC_TRANSITION *tran = NULL;
    VECTOR *trans = NULL;
    
    from = (gen->states) + fromIndex;
    to = (gen->states) + toIndex;
    if( ( tran = (CTMC_TRANSITION*)MALLOC( sizeof(CTMC_TRANSITION) ) ) == NULL ) {
        TRACE_0("could not create a transition");
        return FAILING;
    }
    
    tran->rate = rate;
    tran->to = to;
    trans = from->transitions;
    if( IS_FAILED( ( ret = AddElementInVector( trans, (CADDR_T)tran ) ) ) ) {
        TRACE_0("could not add a transition");
        return FAILING;
    }
    
    return ret;
}

static CTMC *_Generate( CTMC_GENERATOR *gen ) {
    CTMC *ctmc = NULL;
    
    if( ( ctmc = (CTMC*)MALLOC( sizeof(CTMC) ) ) == NULL ) {
        return NULL;
    }
    ctmc->states = gen->states;
    ctmc->initialState = gen->initialState;
    ctmc->stateSize = gen->stateSize;
    
    return ctmc;
}

static RET_VAL _ReleaseResource( CTMC_GENERATOR *gen ) {
    return SUCCESS;
}
