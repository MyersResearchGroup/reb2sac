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
#if !defined(HAVE_DEFAULT_SIMULATION_RUN_TERMINATION_DECIDER)
#define HAVE_DEFAULT_SIMULATION_RUN_TERMINATION_DECIDER

#include "simulation_run_termination_decider.h"

BEGIN_C_NAMESPACE

#define DEFAULT_SIMULATION_RUN_TERMINATION_CONDITION_TYPE_GE 0
#define DEFAULT_SIMULATION_RUN_TERMINATION_CONDITION_TYPE_GT 1
#define DEFAULT_SIMULATION_RUN_TERMINATION_CONDITION_TYPE_EQ 2
#define DEFAULT_SIMULATION_RUN_TERMINATION_CONDITION_TYPE_LE 3
#define DEFAULT_SIMULATION_RUN_TERMINATION_CONDITION_TYPE_LT 4

#define DEFAULT_SIMULATION_RUN_TERMINATION_CONDITION_TYPE_GE_STRING "ge"
#define DEFAULT_SIMULATION_RUN_TERMINATION_CONDITION_TYPE_GT_STRING "gt"
#define DEFAULT_SIMULATION_RUN_TERMINATION_CONDITION_TYPE_EQ_STRING "eq"
#define DEFAULT_SIMULATION_RUN_TERMINATION_CONDITION_TYPE_LE_STRING "le"
#define DEFAULT_SIMULATION_RUN_TERMINATION_CONDITION_TYPE_LT_STRING "lt"

#define SIMULATION_RUN_TERMINATION_CONDITION_IN_AMOUNT "amount"
#define SIMULATION_RUN_TERMINATION_CONDITION_IN_CONCENTRATION "concentration"

#define SIMULATION_RUN_TERMINATION_CONDITION_KEY_PREFIX "simulation.run.termination.condition."



typedef struct {
    int conditionType;
    int speciesIndex;
    double value;
    BOOL inAmount;
    int count;
    char *typeString;
} DEFAULT_SIMULATION_RUN_TERMINATION_CONDITION;

struct _DEFAULT_SIMULATION_RUN_TERMINATION_DECIDER;
typedef struct _DEFAULT_SIMULATION_RUN_TERMINATION_DECIDER DEFAULT_SIMULATION_RUN_TERMINATION_DECIDER;

struct _DEFAULT_SIMULATION_RUN_TERMINATION_DECIDER {
    SPECIES **speciesArray;
    int size;
    REACTION **reactionArray;
    int reactionSize;
    double timeLimit;
    BOOL (*IsTerminationConditionMet)( SIMULATION_RUN_TERMINATION_DECIDER *decider, REACTION *reaction, double time );
    RET_VAL (*Report)( SIMULATION_RUN_TERMINATION_DECIDER *decider, FILE *file );
    RET_VAL (*Destroy)( SIMULATION_RUN_TERMINATION_DECIDER *decider );
    
    int conditionSize;
    DEFAULT_SIMULATION_RUN_TERMINATION_CONDITION **conditions;
};



DLLSCOPE SIMULATION_RUN_TERMINATION_DECIDER * STDCALL 
    CreateDefaultSimulationRunTerminationDecider( 
        BACK_END_PROCESSOR *backend, SPECIES **speciesArray, int size, 
        REACTION **reactionArray, int reactionSize, double timeLimit );


END_C_NAMESPACE

#endif

