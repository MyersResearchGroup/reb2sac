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
#include "constraint_simulation_run_termination_decider.h"

static BOOL _IsTerminationConditionMet( CONSTRAINT_SIMULATION_RUN_TERMINATION_DECIDER *decider, REACTION *reaction, double time );
static RET_VAL _Destroy( CONSTRAINT_SIMULATION_RUN_TERMINATION_DECIDER *decider );
static RET_VAL _Report( CONSTRAINT_SIMULATION_RUN_TERMINATION_DECIDER *decider, FILE *file );
static BOOL _EvaluateConstraints( CONSTRAINT_SIMULATION_RUN_TERMINATION_DECIDER *decider );

DLLSCOPE SIMULATION_RUN_TERMINATION_DECIDER * STDCALL CreateConstraintSimulationRunTerminationDecider( 
        BACK_END_PROCESSOR *backend, CONSTRAINT **constraintArray, int constraintsSize, 
        KINETIC_LAW_EVALUATER *evaluator, BOOL useConcentrations, double timeLimit ) {
    
    CONSTRAINT_SIMULATION_RUN_TERMINATION_DECIDER *decider = NULL;
    REB2SAC_PROPERTIES *properties = backend->record->properties;
    int i = 0;

    START_FUNCTION("CreateConstraintSimulationRunTerminationDecider");

    if( ( decider = (CONSTRAINT_SIMULATION_RUN_TERMINATION_DECIDER*)MALLOC( sizeof(CONSTRAINT_SIMULATION_RUN_TERMINATION_DECIDER) ) ) == NULL ) {
        TRACE_0("failed to allocate memory for CONSTRAINT_SIMULATION_RUN_TERMINATION_DECIDER");
        return NULL;
    }
    decider->timeLimit = timeLimit;
    
    decider->IsTerminationConditionMet = 
            (BOOL(*)(SIMULATION_RUN_TERMINATION_DECIDER *, REACTION *, double))_IsTerminationConditionMet;        
    decider->Destroy = (RET_VAL(*)(SIMULATION_RUN_TERMINATION_DECIDER *))_Destroy;
    decider->Report = (RET_VAL(*)(SIMULATION_RUN_TERMINATION_DECIDER *, FILE *))_Report;
    
    decider->constraintArray = constraintArray;
    decider->constraintsSize = constraintsSize;
    decider->countArray = (int*)MALLOC( decider->constraintsSize * sizeof(int) );
    for (i = 0; i < decider->constraintsSize; i++) {
      decider->countArray[i] = 0;
    }
    decider->evaluator = evaluator;
    decider->useConcentrations = useConcentrations;
    decider->timeLimitCount = 0;
    decider->totalCount = 0;
    
    END_FUNCTION("CreateConstraintSimulationRunTerminationDecider", SUCCESS );
    return (SIMULATION_RUN_TERMINATION_DECIDER*)decider;
}

static BOOL _IsTerminationConditionMet( CONSTRAINT_SIMULATION_RUN_TERMINATION_DECIDER *decider, REACTION *reaction, double time ) {

    if( time >= decider->timeLimit ) {
        (decider->timeLimitCount)++;
    }
    else if( !_EvaluateConstraints( decider ) ) {
        return FALSE;        
    }
            
    (decider->totalCount)++;        
    return TRUE;
}

static RET_VAL _Report( CONSTRAINT_SIMULATION_RUN_TERMINATION_DECIDER *decider, FILE *file ) {
    RET_VAL ret = SUCCESS;
    int i = 0;

    fprintf( file, "#total time-limit");
    
    for (i = 0; i < decider->constraintsSize; i++) {
      fprintf( file, " %s", GetCharArrayOfString( GetConstraintId( decider->constraintArray[i] ) ) );    
    }
    fprintf( file, NEW_LINE );
    
    fprintf( file, "%i %i", decider->totalCount, decider->timeLimitCount );
    for (i = 0; i < decider->constraintsSize; i++) {
      fprintf( file, " %i", decider->countArray[i] );    
    }
    fprintf( file, NEW_LINE );
        
    return ret;
}


static RET_VAL _Destroy( CONSTRAINT_SIMULATION_RUN_TERMINATION_DECIDER *decider ) {
    RET_VAL ret = SUCCESS;
    
    FREE(decider->countArray);
    
    return ret;
}
 

static BOOL _EvaluateConstraints( CONSTRAINT_SIMULATION_RUN_TERMINATION_DECIDER *decider ) {
    BOOL condition = FALSE;
    int i = 0;

    for (i = 0; i < decider->constraintsSize; i++) {
      if (decider->useConcentrations) {
	if (decider->evaluator->EvaluateWithCurrentConcentrations( decider->evaluator, (KINETIC_LAW*)GetMathInConstraint( decider->constraintArray[i] ) ) ) {
	  decider->countArray[i]++;
	  condition = TRUE;
	} 
      } else {
	if (decider->evaluator->EvaluateWithCurrentAmounts( decider->evaluator, (KINETIC_LAW*)GetMathInConstraint( decider->constraintArray[i] ) ) ) {
	  decider->countArray[i]++;
	  condition = TRUE;
	} 
      }
    }
        
    return condition;        
}

 
 
 
 
