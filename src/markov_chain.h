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
#if !defined(HAVE_MARKOV_CHAIN)
#define HAVE_MARKOV_CHAIN


#include "common.h"
#include "vector.h"

BEGIN_C_NAMESPACE

struct _MC_STATE;
typedef struct _MC_STATE MC_STATE;

struct _MC_TRANSITION;
typedef struct _MC_TRANSITION MC_TRANSITION;

struct _MC_STATE {    
    CADDR_T data;
    CADDR_T analysisRecord;        
    VECTOR *transitions;
};

struct _MC_TRANSITION {
    MC_STATE *to;
    double prob;
    double rate;
};


typedef struct {
    MC_STATE *initialState;
    MC_STATE *states;
    int stateSize;    
} MARKOV_CHAIN;


struct _CTMC_STATE;
typedef struct _CTMC_STATE CTMC_STATE;

struct _CTMC_TRANSITION;
typedef struct _CTMC_TRANSITION CTMC_TRANSITION;

struct _CTMC_STATE {    
    CADDR_T data;
    CADDR_T analysisRecord;        
    VECTOR *transitions;
};

struct _CTMC_TRANSITION {
    CTMC_STATE *to;
    double prob;
    double rate;
};

typedef struct {
    CTMC_STATE *initialState;
    CTMC_STATE *states;
    int stateSize;    
} CTMC;


struct _EMC_STATE;
typedef struct _EMC_STATE EMC_STATE;

struct _EMC_TRANSITION;
typedef struct _EMC_TRANSITION EMC_TRANSITION;

struct _EMC_STATE {    
    CADDR_T data;
    CADDR_T analysisRecord;        
    VECTOR *transitions;
};

struct _EMC_TRANSITION {
    EMC_STATE *to;
    double prob;
    double rate;
};

typedef struct {
    EMC_STATE *initialState;
    EMC_STATE *states;
    int stateSize;    
} EMC;


struct _DTMC_STATE;
typedef struct _DTMC_STATE DTMC_STATE;

struct _DTMC_TRANSITION;
typedef struct _DTMC_TRANSITION DTMC_TRANSITION;

struct _DTMC_STATE {    
    CADDR_T data;
    CADDR_T analysisRecord;        
    VECTOR *transitions;
};

struct _DTMC_TRANSITION {
    DTMC_STATE *to;
    double prob;
};

typedef struct {
    DTMC_STATE *initialState;
    DTMC_STATE *states;
    int stateSize;    
} DTMC;


struct _CTMC_GENERATOR;
typedef struct _CTMC_GENERATOR CTMC_GENERATOR;

struct _CTMC_GENERATOR {
    CTMC_STATE *initialState;
    CTMC_STATE *states;
    int stateSize;    
    CTMC_STATE *(*CreateStates)( CTMC_GENERATOR *gen, int size );
    RET_VAL (*SetInitialState)( CTMC_GENERATOR *gen, int index );
    RET_VAL (*SetStateData)( CTMC_GENERATOR *gen, int index, CADDR_T data );
    RET_VAL (*AddTransition)( CTMC_GENERATOR *gen, int fromIndex, int toIndex, double rate );        
    CTMC *(*Generate)( CTMC_GENERATOR *gen );        
    RET_VAL (*ReleaseResource)( CTMC_GENERATOR *gen );
};

CTMC_GENERATOR *CreateCTMCGenerator();
RET_VAL FreeCTMCGenerator( CTMC_GENERATOR **pGen);


END_C_NAMESPACE

#endif
