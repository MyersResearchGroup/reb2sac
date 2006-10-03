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
#include "sac_phage_lambda_simulation_run_termination_decider.h"
#include "species_node.h"
#include "logical_species_node.h"
/*
#define SAC_PHAGE_DEBUG 1
*/
static BOOL _IsTerminationConditionMet( SAC_PHAGE_LAMBDA_SIMULATION_RUN_TERMINATION_DECIDER *decider, REACTION *reaction, double time );
static RET_VAL _Destroy( SAC_PHAGE_LAMBDA_SIMULATION_RUN_TERMINATION_DECIDER *decider );
static RET_VAL _Report( SAC_PHAGE_LAMBDA_SIMULATION_RUN_TERMINATION_DECIDER *decider, FILE *file );
static int _GetIndexOfCI( SPECIES **speciesArray, int size );
static int _GetIndexOfCro( SPECIES **speciesArray, int size );
static int _FindIndexOfSpeciesFromName( char *namePrefixOfSpeciesOfInterest, double threshold, SPECIES **speciesArray, int size );

DLLSCOPE SIMULATION_RUN_TERMINATION_DECIDER * STDCALL CreateSacPhageLambdaSimulationRunTerminationDecider( 
        BACK_END_PROCESSOR *backend, SPECIES **speciesArray, int size, 
        REACTION **reactionArray, int reactionSize, double timeLimit ) {
    char *valueString = NULL;
    COMPILER_RECORD_T *compRec = backend->record;
    REB2SAC_PROPERTIES *properties = NULL;    
    SAC_PHAGE_LAMBDA_SIMULATION_RUN_TERMINATION_DECIDER *decider = NULL;
    
    START_FUNCTION("CreateSacPhageLambdaSimulationRunTerminationDecider");

    if( ( decider = (SAC_PHAGE_LAMBDA_SIMULATION_RUN_TERMINATION_DECIDER*)MALLOC( sizeof(SAC_PHAGE_LAMBDA_SIMULATION_RUN_TERMINATION_DECIDER) ) ) == NULL ) {
        TRACE_0("failed to allocate memory for SAC_PHAGE_LAMBDA_SIMULATION_RUN_TERMINATION_DECIDER");
        return NULL;
    }
    decider->speciesArray = speciesArray;
    decider->size = size;
    decider->timeLimit = timeLimit;
    
    decider->IsTerminationConditionMet = 
        (BOOL(*)(SIMULATION_RUN_TERMINATION_DECIDER *, double))_IsTerminationConditionMet;        
    decider->Destroy = (RET_VAL(*)(SIMULATION_RUN_TERMINATION_DECIDER *))_Destroy;
    decider->Report = (RET_VAL(*)(SIMULATION_RUN_TERMINATION_DECIDER *, FILE *))_Report;
    
    decider->lysogenyNum = 0;
    decider->lysisNum = 0;    
    if( ( decider->indexCI = _GetIndexOfCI( speciesArray, size ) ) < 0 ) {
        TRACE_0("could not find the index of CI");
        return NULL;
    }
    if( ( decider->indexCro = _GetIndexOfCro( speciesArray, size ) ) < 0 ) {
        TRACE_0("could not find the index of Cro");
        return NULL;
    }
    
    properties = compRec->properties;
    if( ( valueString = properties->GetProperty( properties, SAC_PHAGE_LAMBDA_CI_THRESHOLD_KEY ) ) == NULL ) {
        decider->cIThreshold = DEFAULT_SAC_PHAGE_LAMBDA_CI_THRESHOLD_VALUE;        
    }
    else {
        if( IS_FAILED( StrToFloat( &(decider->cIThreshold), valueString ) ) ) {
            decider->cIThreshold = DEFAULT_SAC_PHAGE_LAMBDA_CI_THRESHOLD_VALUE;        
        }
    }
    
    if( ( valueString = properties->GetProperty( properties, SAC_PHAGE_LAMBDA_CRO_THRESHOLD_KEY ) ) == NULL ) {
        decider->croThreshold = DEFAULT_SAC_PHAGE_LAMBDA_CRO_THRESHOLD_VALUE;        
    }
    else {
        if( IS_FAILED( StrToFloat( &(decider->croThreshold), valueString ) ) ) {
            decider->croThreshold = DEFAULT_SAC_PHAGE_LAMBDA_CRO_THRESHOLD_VALUE;        
        }
    }
#ifdef SAC_PHAGE_DEBUG
    exit(1);
#endif    
    
    END_FUNCTION("CreateSacPhageLambdaSimulationRunTerminationDecider", SUCCESS );
    return (SIMULATION_RUN_TERMINATION_DECIDER*)decider;
}

static BOOL _IsTerminationConditionMet( SAC_PHAGE_LAMBDA_SIMULATION_RUN_TERMINATION_DECIDER *decider, REACTION *reaction, double time ) {
    double quantity = 0.0;
    SPECIES *species = NULL;
    SPECIES **speciesArray = decider->speciesArray;
    int indexCI = decider->indexCI;
    int indexCro = decider->indexCro;
            
    if( time >= decider->timeLimit ) {
        return TRUE;
    }
    
    species = speciesArray[indexCI];    
    quantity = GetConcentrationInSpeciesNode( species );
    if( quantity > 0.0 ) {
        decider->lysogenyNum++;
        return TRUE;
    }
    
    species = speciesArray[indexCro];
    quantity = GetConcentrationInSpeciesNode( species );
    if( quantity > 0.0 ) {
        decider->lysisNum++;
        return TRUE;
    }
    return FALSE;        
}

static RET_VAL _Report( SAC_PHAGE_LAMBDA_SIMULATION_RUN_TERMINATION_DECIDER *decider, FILE *file ) {
    RET_VAL ret = SUCCESS;
    int totalNum = 0;
    int lysogenyNum = decider->lysogenyNum;
    int lysisNum = decider->lysisNum;     
    
    totalNum = lysogenyNum + lysisNum;
    fprintf( file, "# lysogeny-probability lysis-probability lysogeny-count lysis-count total-count" NEW_LINE );
    fprintf( file, "%g %g %i %i %i" NEW_LINE,
                     (double)lysogenyNum / (double)totalNum, 
                     (double)lysisNum / (double)totalNum, 
                     lysogenyNum,
                     lysisNum,
                     totalNum );
                
    return ret;
}


static RET_VAL _Destroy( SAC_PHAGE_LAMBDA_SIMULATION_RUN_TERMINATION_DECIDER *decider ) {
    RET_VAL ret = SUCCESS;
    
    FREE( decider );
    
    return ret;
}

static int _GetIndexOfCI( SPECIES **speciesArray, int size ) {
    int index = 0;
    
    index = _FindIndexOfSpeciesFromName( SAC_PHAGE_LAMBDA_CI_NAME, DEFAULT_SAC_PHAGE_LAMBDA_CI_THRESHOLD_VALUE, speciesArray, size );
#ifdef SAC_PHAGE_DEBUG
    if( index < 0 ) {
        printf("cannot find a target logical species for %s" NEW_LINE, SAC_PHAGE_LAMBDA_CI_NAME );
    }
    else {
        printf( "target logical species: %s" NEW_LINE, GetCharArrayOfString( GetSpeciesNodeName( speciesArray[index] ) ) ); 
    }
#endif
    return index;
}

static int _GetIndexOfCro( SPECIES **speciesArray, int size ) {
    int index = 0;
    
    index = _FindIndexOfSpeciesFromName( SAC_PHAGE_LAMBDA_CRO_NAME, DEFAULT_SAC_PHAGE_LAMBDA_CRO_THRESHOLD_VALUE, speciesArray, size );
#ifdef SAC_PHAGE_DEBUG
    if( index < 0 ) {
        printf("cannot find a target logical species for %s" NEW_LINE, SAC_PHAGE_LAMBDA_CRO_NAME );
    }
    else {
        printf( "target logical species: %s" NEW_LINE, GetCharArrayOfString( GetSpeciesNodeName( speciesArray[index] ) ) ); 
    }
#endif
    return index;
}


static int _FindIndexOfSpeciesFromName( char *namePrefixOfSpeciesOfInterest, double threshold, SPECIES **speciesArray, int size ) {
    int i = 0;
    int bestOne = -1;
    int prefixLen = 0;
    SPECIES *species = NULL;
    LOGICAL_SPECIES *logicalSpecies = NULL;    
    char *nameString = NULL;
    double bestCon = 0.0;
    double con = 0.0;
    
    prefixLen = strlen( namePrefixOfSpeciesOfInterest );
    
    for( i = 0; i < size; i++ ) {
        species = speciesArray[i];
        if( !IsLogicalSpecies( species ) ) {
            continue;                
        }
        
        nameString = GetCharArrayOfString( GetSpeciesNodeName( species ) );        
        if( strncmp( nameString, namePrefixOfSpeciesOfInterest, prefixLen ) == 0 ) {
            logicalSpecies = (LOGICAL_SPECIES*)species;
            con = GetCriticalConcentrationInLogicalSpecies( logicalSpecies );
            if( con <= threshold ) {
                if( con > bestCon ) {
                    bestCon = con;
                    bestOne = i;
                }
            }                 
        }            
    }
        
    return bestOne;
}


