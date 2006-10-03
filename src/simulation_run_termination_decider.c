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
#include "simulation_run_termination_decider.h"
#include "default_simulation_run_termination_decider.h"
#include "flat_phage_lambda_simulation_run_termination_decider.h"
#include "flat_phage_lambda2_simulation_run_termination_decider.h"
#include "abs_phage_lambda_simulation_run_termination_decider.h"
#include "abs_phage_lambda2_simulation_run_termination_decider.h"
#include "sac_phage_lambda_simulation_run_termination_decider.h"
#include "type_1_pili1_simulation_run_termination_decider.h"
#include "type_1_pili2_simulation_run_termination_decider.h" 
 
 
DLLSCOPE SIMULATION_RUN_TERMINATION_DECIDER * STDCALL CreateSimulationRunTerminationDecider( BACK_END_PROCESSOR *backend, SPECIES **speciesArray, int size, REACTION **reactionArray, int reactionSize, double timeLimit ) {
    int i = 0;
    SIMULATION_RUN_TERMINATION_DECIDER *decider = NULL;
    char *valueString = NULL;
    COMPILER_RECORD_T *compRec = backend->record;
    REB2SAC_PROPERTIES *properties = NULL;
    
    START_FUNCTION("CreateSimulationRunTerminationDecider");
    
    properties = compRec->properties;
    if( ( valueString = properties->GetProperty( properties, SIMULATION_RUN_TERMINATION_DECIDER_KEY ) ) == NULL ) {
        if( ( decider = CreateDefaultSimulationRunTerminationDecider( 
              backend, speciesArray, size, reactionArray, reactionSize, timeLimit ) ) == NULL ) {
            END_FUNCTION("CreateSimulationRunTerminationDecider", FAILING );
            return NULL;
        }
        END_FUNCTION("CreateSimulationRunTerminationDecider", SUCCESS );
        return decider;
    }

    switch( valueString[0] ) {
        case 'a':
            if( strcmp( valueString, ABS_PHAGE_LAMBDA_SIMULATION_RUN_TERMINATION_DECIDER_ID ) == 0 ) {
                if( ( decider = CreateAbsPhageLambdaSimulationRunTerminationDecider( 
                      backend, speciesArray, size, reactionArray, reactionSize, timeLimit ) ) == NULL ) {
                    END_FUNCTION("CreateSimulationRunTerminationDecider", FAILING );
                    return NULL;
                }
            }        
            else if( strcmp( valueString, ABS_PHAGE_LAMBDA2_SIMULATION_RUN_TERMINATION_DECIDER_ID ) == 0 ) {
                if( ( decider = CreateAbsPhageLambda2SimulationRunTerminationDecider( 
                      backend, speciesArray, size, reactionArray, reactionSize, timeLimit ) ) == NULL ) {
                    END_FUNCTION("CreateSimulationRunTerminationDecider", FAILING );
                    return NULL;
                      }
            }        
            break;
        
        case 'f':
            if( strcmp( valueString, FLAT_PHAGE_LAMBDA_SIMULATION_RUN_TERMINATION_DECIDER_ID ) == 0 ) {
                if( ( decider = CreateFlatPhageLambdaSimulationRunTerminationDecider( 
                      backend, speciesArray, size, reactionArray, reactionSize, timeLimit ) ) == NULL ) {
                    END_FUNCTION("CreateSimulationRunTerminationDecider", FAILING );
                    return NULL;
                }
            }        
            else if( strcmp( valueString, FLAT_PHAGE_LAMBDA2_SIMULATION_RUN_TERMINATION_DECIDER_ID ) == 0 ) {
                if( ( decider = CreateFlatPhageLambda2SimulationRunTerminationDecider( 
                      backend, speciesArray, size, reactionArray, reactionSize, timeLimit ) ) == NULL ) {
                    END_FUNCTION("CreateSimulationRunTerminationDecider", FAILING );
                    return NULL;
                }
            }
        break;
        
        case 's':
            if( strcmp( valueString, SAC_PHAGE_LAMBDA_SIMULATION_RUN_TERMINATION_DECIDER_ID ) == 0 ) {
                if( ( decider = CreateSacPhageLambdaSimulationRunTerminationDecider( 
                      backend, speciesArray, size, reactionArray, reactionSize, timeLimit ) ) == NULL ) {
                    END_FUNCTION("CreateSimulationRunTerminationDecider", FAILING );
                    return NULL;
                }
            }        
        break;
        
        case 't':
            if( strcmp( valueString, TYPE_1_PILI1_SIMULATION_RUN_TERMINATION_DECIDER_ID ) == 0 ) {
                if( ( decider = CreateType1Pili1SimulationRunTerminationDecider( 
                      backend, speciesArray, size, reactionArray, reactionSize, timeLimit ) ) == NULL ) {
                    END_FUNCTION("CreateSimulationRunTerminationDecider", FAILING );
                    return NULL;
                }
            }        
            else if( strcmp( valueString, TYPE_1_PILI2_SIMULATION_RUN_TERMINATION_DECIDER_ID ) == 0 ) {
                if( ( decider = CreateType1Pili2SimulationRunTerminationDecider( 
                      backend, speciesArray, size, reactionArray, reactionSize, timeLimit ) ) == NULL ) {
                    END_FUNCTION("CreateSimulationRunTerminationDecider", FAILING );
                    return NULL;
                }
            }        
        break;

        default:
            if( ( decider = CreateDefaultSimulationRunTerminationDecider( 
                  backend, speciesArray, size, reactionArray, reactionSize, timeLimit ) ) == NULL ) {
                END_FUNCTION("CreateSimulationRunTerminationDecider", FAILING );
                return NULL;
            }
    }
    END_FUNCTION("CreateSimulationRunTerminationDecider", SUCCESS );
    return decider;
}

DLLSCOPE RET_VAL STDCALL DestroySimulationRunTerminationDecider( SIMULATION_RUN_TERMINATION_DECIDER *decider ) {
    RET_VAL ret = SUCCESS;
    
    ret = decider->Destroy( decider );
    
    return ret;
}
 