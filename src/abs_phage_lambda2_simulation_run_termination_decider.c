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
#include "abs_phage_lambda2_simulation_run_termination_decider.h"

static BOOL _IsTerminationConditionMet( ABS_PHAGE_LAMBDA2_SIMULATION_RUN_TERMINATION_DECIDER *decider, double time );
static RET_VAL _Destroy( ABS_PHAGE_LAMBDA2_SIMULATION_RUN_TERMINATION_DECIDER *decider );
static RET_VAL _Report( ABS_PHAGE_LAMBDA2_SIMULATION_RUN_TERMINATION_DECIDER *decider, FILE *file );
static int _GetIndexOfCI( SPECIES **speciesArray, int size );
static int _GetIndexOfCro( SPECIES **speciesArray, int size );
static int _FindIndexOfSpeciesFromName( char *nameOfSpeciesOfInterest, SPECIES **speciesArray, int size );

DLLSCOPE SIMULATION_RUN_TERMINATION_DECIDER * STDCALL CreateAbsPhageLambda2SimulationRunTerminationDecider( 
        BACK_END_PROCESSOR *backend, SPECIES **speciesArray, int size, 
        REACTION **reactionArray, int reactionSize, double timeLimit ) {
    char *valueString = NULL;
    COMPILER_RECORD_T *compRec = backend->record;
    REB2SAC_PROPERTIES *properties = NULL;    
    ABS_PHAGE_LAMBDA2_SIMULATION_RUN_TERMINATION_DECIDER *decider = NULL;
    
    START_FUNCTION("CreateAbsPhageLambdaSimulationRunTerminationDecider");

    if( ( decider = (ABS_PHAGE_LAMBDA2_SIMULATION_RUN_TERMINATION_DECIDER*)MALLOC( sizeof(ABS_PHAGE_LAMBDA2_SIMULATION_RUN_TERMINATION_DECIDER) ) ) == NULL ) {
        TRACE_0("failed to allocate memory for ABS_PHAGE_LAMBDA2_SIMULATION_RUN_TERMINATION_DECIDER");
        return NULL;
    }
    decider->speciesArray = speciesArray;
    decider->size = size;
    decider->timeLimit = timeLimit;
    
    decider->IsTerminationConditionMet = 
        (BOOL(*)(SIMULATION_RUN_TERMINATION_DECIDER *, double))_IsTerminationConditionMet;        
    decider->Destroy = (RET_VAL(*)(SIMULATION_RUN_TERMINATION_DECIDER *))_Destroy;
    decider->Report = (RET_VAL(*)(SIMULATION_RUN_TERMINATION_DECIDER *, FILE *))_Report;
    
    decider->case3 = 0;
    decider->case4 = 0;
    decider->case5 = 0;
    decider->case6 = 0;
    decider->case7 = 0;
    decider->case8 = 0;
    
    if( ( decider->indexCI = _GetIndexOfCI( speciesArray, size ) ) < 0 ) {
        TRACE_0("could not find the index of CI");
        return NULL;
    }
    if( ( decider->indexCro = _GetIndexOfCro( speciesArray, size ) ) < 0 ) {
        TRACE_0("could not find the index of Cro");
        return NULL;
    }
    
    properties = compRec->properties;
    if( ( valueString = properties->GetProperty( properties, ABS_PHAGE_LAMBDA2_CI_THRESHOLD_KEY ) ) == NULL ) {
        decider->cIThreshold = DEFAULT_ABS_PHAGE_LAMBDA2_CI_THRESHOLD_VALUE;        
    }
    else {
        if( IS_FAILED( StrToFloat( &(decider->cIThreshold), valueString ) ) ) {
            decider->cIThreshold = DEFAULT_ABS_PHAGE_LAMBDA2_CI_THRESHOLD_VALUE;        
        }
    }
    
    if( ( valueString = properties->GetProperty( properties, ABS_PHAGE_LAMBDA2_CRO_THRESHOLD_KEY ) ) == NULL ) {
        decider->croThreshold = DEFAULT_ABS_PHAGE_LAMBDA2_CRO_THRESHOLD_VALUE;        
    }
    else {
        if( IS_FAILED( StrToFloat( &(decider->croThreshold), valueString ) ) ) {
            decider->croThreshold = DEFAULT_ABS_PHAGE_LAMBDA2_CRO_THRESHOLD_VALUE;        
        }
    }
    
    END_FUNCTION("CreateAbsPhageLambdaSimulationRunTerminationDecider", SUCCESS );
    return (SIMULATION_RUN_TERMINATION_DECIDER*)decider;
}

static BOOL _IsTerminationConditionMet( ABS_PHAGE_LAMBDA2_SIMULATION_RUN_TERMINATION_DECIDER *decider, double time ) {
    SPECIES *species = NULL;
    SPECIES **speciesArray = decider->speciesArray;
    int indexCI = decider->indexCI;
    int indexCro = decider->indexCro;
    double cIThreshold = decider->cIThreshold;
    double croThreshold = decider->croThreshold;    
    double cICon = 0.0;
    double croCon = 0.0;
            
    if( time >= decider->timeLimit ) {
        species = speciesArray[indexCI];    
        cICon = GetConcentrationInSpeciesNode( species );
        species = speciesArray[indexCro];
        croCon = GetConcentrationInSpeciesNode( species );
        
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
        if( croCon > cICon ) {
            decider->case3++;
            if( croCon > croThreshold ) {
                decider->case4++;
                if( cICon < cIThreshold ) {
                    decider->case5++;
                }
            }
        }
        else if( cICon > croCon ) {
            decider->case6++;
            if( cICon > cIThreshold ) {
                decider->case7++;
                if( croCon < croThreshold ) {
                    decider->case8++;
                }
            }
        }
        
        return TRUE;
    }
    else {
        return FALSE;       
    } 
}

static RET_VAL _Report( ABS_PHAGE_LAMBDA2_SIMULATION_RUN_TERMINATION_DECIDER *decider, FILE *file ) {
    RET_VAL ret = SUCCESS;
    
    fprintf( file, "# case3 case4 case5 case6 case7 case8" NEW_LINE );
    fprintf( file, "%i %i %i %i %i %i" NEW_LINE,
                     decider->case3,
                     decider->case4,
                     decider->case5,
                     decider->case6,
                     decider->case7,
                     decider->case8 );
                
    return ret;
}


static RET_VAL _Destroy( ABS_PHAGE_LAMBDA2_SIMULATION_RUN_TERMINATION_DECIDER *decider ) {
    RET_VAL ret = SUCCESS;
    
    FREE( decider );
    
    return ret;
}

static int _GetIndexOfCI( SPECIES **speciesArray, int size ) {
    int index = 0;
    
    index = _FindIndexOfSpeciesFromName( ABS_PHAGE_LAMBDA2_CI_NAME, speciesArray, size );
    return index;
}

static int _GetIndexOfCro( SPECIES **speciesArray, int size ) {
    int index = 0;
    
    index = _FindIndexOfSpeciesFromName( ABS_PHAGE_LAMBDA2_CRO_NAME, speciesArray, size );
    return index;
}


static int _FindIndexOfSpeciesFromName( char *nameOfSpeciesOfInterest, SPECIES **speciesArray, int size ) {
    int i = 0;
    SPECIES *species = NULL;
    char *nameString = NULL;
    
    for( i = 0; i < size; i++ ) {
        species = speciesArray[i];
        nameString = GetCharArrayOfString( GetSpeciesNodeName( species ) );
        if( strcmp( nameString, nameOfSpeciesOfInterest ) == 0 ) {
            return i;
        }            
    }
        
    return -1;
}


