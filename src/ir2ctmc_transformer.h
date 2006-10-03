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
#if !defined(HAVE_IR2CTMC_TRANSFORMER)
#define HAVE_IR2CTMC_TRANSFORMER

#include "common.h"
#include "compiler_def.h"
#include "IR.h"
#include "markov_chain.h"
#include "species_critical_level.h"
#include "kinetic_law_evaluater.h"

BEGIN_C_NAMESPACE

typedef struct {
    double *pLevel;
} REB_SYSTEM_STATE_DATA;


struct _IR2CTMC_TRANSFORMER;
typedef struct _IR2CTMC_TRANSFORMER IR2CTMC_TRANSFORMER;

struct _IR2CTMC_TRANSFORMER {
    int reactionSize;
    REACTION **reactionArray;
    int speciesSize;
    SPECIES_CRITICAL_LEVEL **criticalLevelArray;
    int updatedSpeciesSize;
    SPECIES **updatedSpeciesArray;
    int *multiplyArray;
    int *currentArray;
    double *currentLevelArray;
    int stateSize;
    KINETIC_LAW_EVALUATER *evaluator;
    
    CTMC *(*Generate)( IR2CTMC_TRANSFORMER *transformer );
    int (*GetCriticalLevelArraySize)( IR2CTMC_TRANSFORMER *transformer );
    SPECIES_CRITICAL_LEVEL **(*GetCriticalLevelArray)( IR2CTMC_TRANSFORMER *transformer );
    RET_VAL (*ResetCurrentLevelArray)( IR2CTMC_TRANSFORMER *transformer );
    RET_VAL (*IncrementCurrentLevelArray)( IR2CTMC_TRANSFORMER *transformer );
    double *(*GetCurrentLevelArray)( IR2CTMC_TRANSFORMER *transformer );
    RET_VAL (*Print)( IR2CTMC_TRANSFORMER *transformer, FILE *file );
    RET_VAL (*ReleaseResource)( IR2CTMC_TRANSFORMER *transformer );
};



IR2CTMC_TRANSFORMER *CreateIR2CTMCTransformer( IR *ir, REB2SAC_PROPERTIES *properties );
RET_VAL FreeIR2CTMCTransformer( IR2CTMC_TRANSFORMER **pTransformer );
        
END_C_NAMESPACE
     
#endif













