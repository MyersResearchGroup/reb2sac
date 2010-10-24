/***************************************************************************
 *   Copyright (C) 2004 by Hiroyuki Kuwahara                               *
 *   kuwahara@cs.utah.edu                                                 *
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
#if !defined(HAVE_CONSTRAINT_SIMULATION_RUN_TERMINATION_DECIDER)
#define HAVE_CONSTRAINT_SIMULATION_RUN_TERMINATION_DECIDER

#include "simulation_run_termination_decider.h"
#include "kinetic_law.h"
#include "kinetic_law_evaluater.h"

BEGIN_C_NAMESPACE

#define CONSTRAINT_SIMULATION_RUN_TERMINATION_DECIDER_ID "constraint"



struct _CONSTRAINT_SIMULATION_RUN_TERMINATION_DECIDER;
typedef struct _CONSTRAINT_SIMULATION_RUN_TERMINATION_DECIDER CONSTRAINT_SIMULATION_RUN_TERMINATION_DECIDER;

struct _CONSTRAINT_SIMULATION_RUN_TERMINATION_DECIDER {
    SPECIES **speciesArray;
    int size;
    REACTION **reactionArray;
    int reactionSize;
    double timeLimit;
    BOOL (*IsTerminationConditionMet)( SIMULATION_RUN_TERMINATION_DECIDER *decider, REACTION *reaction, double time );
    RET_VAL (*Report)( SIMULATION_RUN_TERMINATION_DECIDER *decider, FILE *file );
    RET_VAL (*Destroy)( SIMULATION_RUN_TERMINATION_DECIDER *decider );
    
    CONSTRAINT **constraintArray;
    int constraintsSize;
    int *countArray;
    KINETIC_LAW_EVALUATER *evaluator;
    BOOL useConcentrations;
    int timeLimitCount;
    int totalCount;
    FILE *termTimeFile;
};

DLLSCOPE SIMULATION_RUN_TERMINATION_DECIDER * STDCALL CreateConstraintSimulationRunTerminationDecider( 
        BACK_END_PROCESSOR *backend, CONSTRAINT **constraintArray, int constraintsSize, 
        KINETIC_LAW_EVALUATER *evaluator, BOOL useConcentrations, double timeLimit );

END_C_NAMESPACE

#endif

