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
#if !defined(HAVE_ABS_PHAGE_LAMBDA2_SIMULATION_RUN_TERMINATION_DECIDER)
#define HAVE_ABS_PHAGE_LAMBDA2_SIMULATION_RUN_TERMINATION_DECIDER

#include "simulation_run_termination_decider.h"

BEGIN_C_NAMESPACE

#define ABS_PHAGE_LAMBDA2_SIMULATION_RUN_TERMINATION_DECIDER_ID "abs.phage.lambda.2"

#define ABS_PHAGE_LAMBDA2_CI_NAME "cI"
#define ABS_PHAGE_LAMBDA2_CRO_NAME "Cro"

#define ABS_PHAGE_LAMBDA2_CI_THRESHOLD_KEY "abs.phage.lambda.cI.threshold"
#define ABS_PHAGE_LAMBDA2_CRO_THRESHOLD_KEY "abs.phage.lambda.cro.threshold"

#define DEFAULT_ABS_PHAGE_LAMBDA2_CI_THRESHOLD_VALUE 328.0
#define DEFAULT_ABS_PHAGE_LAMBDA2_CRO_THRESHOLD_VALUE 133.0


struct _ABS_PHAGE_LAMBDA2_SIMULATION_RUN_TERMINATION_DECIDER;
typedef struct _ABS_PHAGE_LAMBDA2_SIMULATION_RUN_TERMINATION_DECIDER ABS_PHAGE_LAMBDA2_SIMULATION_RUN_TERMINATION_DECIDER;

struct _ABS_PHAGE_LAMBDA2_SIMULATION_RUN_TERMINATION_DECIDER {
    SPECIES **speciesArray;
    int size;
    REACTION **reactionArray;
    int reactionSize;
    double timeLimit;
    BOOL (*IsTerminationConditionMet)( SIMULATION_RUN_TERMINATION_DECIDER *decider, REACTION *reaction, double time );
    RET_VAL (*Report)( SIMULATION_RUN_TERMINATION_DECIDER *decider, FILE *file );
    RET_VAL (*Destroy)( SIMULATION_RUN_TERMINATION_DECIDER *decider );    
    
    int indexCI;
    int indexCro;
    double cIThreshold;
    double croThreshold;
    int case3;
    int case4;
    int case5;
    int case6;
    int case7;
    int case8;
};

/**
        3) Number of cases where Cro total is greater than cI total at end of 2100
        seconds.
      
        4) Number of cases where Cro total is greater than cI total at end of
        2100 seconds and Cro total is above 133 molecules.

        5) Number of cases where Cro total is greater than cI total at end of
        2100 seconds and Cro total is above 133 molecules and cI total is less
        than 328 molecules.

        6) Number of cases where cI total is greater than Cro total at end of 2100
        seconds.

        7) Number of cases where cI total is greater than Cro total at end of 2100
        seconds and cI total is greater than 328 molecules.

        8) Number of cases where cI total is greater than Cro total at end of 2100
        seconds and cI total is greater than 328 molecules and Cro is less than
        133 molecules.
*/


DLLSCOPE SIMULATION_RUN_TERMINATION_DECIDER * STDCALL 
    CreateAbsPhageLambda2SimulationRunTerminationDecider( 
        BACK_END_PROCESSOR *backend, SPECIES **speciesArray, int size, 
        REACTION **reactionArray, int reactionSize, double timeLimit );


END_C_NAMESPACE

#endif

