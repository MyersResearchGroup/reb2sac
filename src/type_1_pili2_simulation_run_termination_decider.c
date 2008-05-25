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
#include "type_1_pili2_simulation_run_termination_decider.h"

static BOOL _IsTerminationConditionMet( TYPE_1_PILI2_SIMULATION_RUN_TERMINATION_DECIDER *decider, REACTION *reaction, double time );
static RET_VAL _Destroy( TYPE_1_PILI2_SIMULATION_RUN_TERMINATION_DECIDER *decider );
static RET_VAL _Report( TYPE_1_PILI2_SIMULATION_RUN_TERMINATION_DECIDER *decider, FILE *file );
static int _GetIndexOfSwitch( SPECIES **speciesArray, int size );
static int _FindIndexOfSpeciesFromName( char *nameOfSpeciesOfInterest, SPECIES **speciesArray, int size );

static RET_VAL _RefreshCount( TYPE_1_PILI2_SIMULATION_RUN_TERMINATION_DECIDER *decider );    
static int _GetChangeCount( TYPE_1_PILI2_SIMULATION_RUN_TERMINATION_DECIDER *decider );    
static int _GetUnchangeCount( TYPE_1_PILI2_SIMULATION_RUN_TERMINATION_DECIDER *decider );    
static double _GetAverageFlippingTime( TYPE_1_PILI2_SIMULATION_RUN_TERMINATION_DECIDER *decider );    
static RET_VAL _CreateSwitchingReactionArray( TYPE_1_PILI2_SIMULATION_RUN_TERMINATION_DECIDER *decider, SPECIES *switchSpecies );
static RET_VAL _UpdateSwitchingCount( TYPE_1_PILI2_SIMULATION_RUN_TERMINATION_DECIDER *decider, REACTION *reaction );                                              

DLLSCOPE SIMULATION_RUN_TERMINATION_DECIDER * STDCALL CreateType1Pili2SimulationRunTerminationDecider( 
        BACK_END_PROCESSOR *backend, SPECIES **speciesArray, int size, 
        REACTION **reactionArray, int reactionSize, double timeLimit ) {
    TYPE_1_PILI2_SIMULATION_RUN_TERMINATION_DECIDER *decider = NULL;
    
    START_FUNCTION("CreateType1Pili2SimulationRunTerminationDecider");

    if( ( decider = (TYPE_1_PILI2_SIMULATION_RUN_TERMINATION_DECIDER*)MALLOC( sizeof(TYPE_1_PILI2_SIMULATION_RUN_TERMINATION_DECIDER) ) ) == NULL ) {
        TRACE_0("failed to allocate memory for TYPE_1_PILI2_SIMULATION_RUN_TERMINATION_DECIDER");
        return NULL;
    }
    decider->speciesArray = speciesArray;
    decider->size = size;
    decider->timeLimit = timeLimit;
    
    decider->IsTerminationConditionMet = 
      (BOOL(*)(SIMULATION_RUN_TERMINATION_DECIDER *, REACTION*, double))_IsTerminationConditionMet;        
    decider->Destroy = (RET_VAL(*)(SIMULATION_RUN_TERMINATION_DECIDER *))_Destroy;
    decider->Report = (RET_VAL(*)(SIMULATION_RUN_TERMINATION_DECIDER *, FILE *))_Report;
    decider->RefreshCount = _RefreshCount;
    decider->GetChangeCount = _GetChangeCount;
    decider->GetUnchangeCount = _GetUnchangeCount;
    decider->GetAverageFlippingTime = _GetAverageFlippingTime;
    
    decider->changeCount = 0;
    decider->unchangeCount = 0;
    decider->averageFlippingTime = 0.0;    
    if( ( decider->switchIndex = _GetIndexOfSwitch( speciesArray, size ) ) < 0 ) {
        TRACE_0("could not find the index of switch");
        return NULL;
    }
    if( IS_FAILED( ( _CreateSwitchingReactionArray( decider, speciesArray[decider->switchIndex] ) ) ) ) {
        TRACE_0("could not create swtiching reactions");
        return NULL;    
    }

    END_FUNCTION("CreateType1Pili2SimulationRunTerminationDecider", SUCCESS );
    return (SIMULATION_RUN_TERMINATION_DECIDER*)decider;
}



static BOOL _IsTerminationConditionMet( TYPE_1_PILI2_SIMULATION_RUN_TERMINATION_DECIDER *decider, REACTION *reaction, double time ) {
    double quantity = 0.0;
    double averageTime;
    int changeCount;
    SPECIES *species = NULL;
    SPECIES **speciesArray = decider->speciesArray;
            
    if( time >= decider->timeLimit ) {
        decider->unchangeCount++;
        return TRUE;
    }
    
    species = speciesArray[decider->switchIndex];    
    quantity = GetConcentrationInSpeciesNode( species );
    if( quantity >= 1.0 ) {
        changeCount = decider->changeCount + 1;
        averageTime = decider->averageFlippingTime;
        decider->averageFlippingTime = averageTime + ( ( time - averageTime ) / (double)changeCount );
        decider->changeCount = changeCount;
        _UpdateSwitchingCount( decider, reaction );
        return TRUE;
    }
    return FALSE;        
}

static RET_VAL _Report( TYPE_1_PILI2_SIMULATION_RUN_TERMINATION_DECIDER *decider, FILE *file ) {
    RET_VAL ret = SUCCESS;
    int changeCount = decider->changeCount;
    int unchangeCount = decider->unchangeCount;
    int totalNum = changeCount + unchangeCount;
    double averageFlippingTime = decider->averageFlippingTime;
    int i = 0;
    int size = decider->switchingReactionSize;
    REACTION *reaction = NULL;
    REACTION **reactions = decider->switchingReactions;
    int count = 0;
    int *counts = decider->switchingEventCounts;

    
    fprintf( file, "# fliiping-rate average-flipping-time flipping-count total-count");
    for( i = 0; i < size; i++ ) {
        reaction = reactions[i];
        fprintf( file, " %s", GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
    }
    fprintf( file, NEW_LINE );
    
    fprintf( file, "%g %g %i %i",
                     (double)changeCount / (double)totalNum, 
                     averageFlippingTime, 
                     changeCount,
                     totalNum );
    for( i = 0; i < size; i++ ) {
        count = counts[i];
        fprintf( file, " %i", count );
    }
    fprintf( file, NEW_LINE );
                
    return ret;
}


static RET_VAL _Destroy( TYPE_1_PILI2_SIMULATION_RUN_TERMINATION_DECIDER *decider ) {
    RET_VAL ret = SUCCESS;
    
    FREE( decider->switchingEventCounts );
    FREE( decider->switchingReactions );
    FREE( decider );
    
    return ret;
}

static int _GetIndexOfSwitch( SPECIES **speciesArray, int size ) {
    int index = 0;
    
    index = _FindIndexOfSpeciesFromName( "switch", speciesArray, size );
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


static RET_VAL _RefreshCount( TYPE_1_PILI2_SIMULATION_RUN_TERMINATION_DECIDER *decider ) {
    decider->changeCount = 0;
    decider->unchangeCount = 0;
    decider->averageFlippingTime = 0.0;
    return SUCCESS;
}

static int _GetChangeCount( TYPE_1_PILI2_SIMULATION_RUN_TERMINATION_DECIDER *decider ) {
    return decider->changeCount;
}

static int _GetUnchangeCount( TYPE_1_PILI2_SIMULATION_RUN_TERMINATION_DECIDER *decider ) {
    return decider->unchangeCount;
}
    
static double _GetAverageFlippingTime( TYPE_1_PILI2_SIMULATION_RUN_TERMINATION_DECIDER *decider ) {
    return decider->averageFlippingTime;
}

static RET_VAL _CreateSwitchingReactionArray( TYPE_1_PILI2_SIMULATION_RUN_TERMINATION_DECIDER *decider, SPECIES *switchSpecies ) {
    int i = 0;
    int size = 0;
    IR_EDGE *edge = NULL;
    REACTION *reaction = NULL;
    REACTION **reactions = NULL;
    int *counts = NULL;
    LINKED_LIST *edges = NULL;
    
    edges = GetProductEdges( (IR_NODE*)switchSpecies );
    size = GetLinkedListSize( edges );
    if( ( reactions = (REACTION**)MALLOC( size * sizeof(REACTION*) ) ) == NULL ) {
        return ErrorReport( FAILING, "_CreateSwitchingReactionArray", "could not create an array for switching reactions" );
    }
    if( ( counts = (int*)CALLOC( size, sizeof(int) ) ) == NULL ) {
        return ErrorReport( FAILING, "_CreateSwitchingReactionArray", "could not create an array for switching counts" );
    }
    
    ResetCurrentElement( edges );
    for( i = 0; i < size; i++ ) {
        edge = GetNextEdge( edges );
        reaction = GetReactionInIREdge( edge );
        reactions[i] = reaction;
    }
    
    decider->switchingReactions = reactions;
    decider->switchingEventCounts = counts;
    decider->switchingReactionSize = size;
    
    return SUCCESS;
}

static RET_VAL _UpdateSwitchingCount( TYPE_1_PILI2_SIMULATION_RUN_TERMINATION_DECIDER *decider, REACTION *reaction ) {
    int i = 0;
    int size = decider->switchingReactionSize;
    REACTION *switchingReaction = NULL;
    REACTION **switchingReactions = decider->switchingReactions;
    int *counts = decider->switchingEventCounts;
    
    for( ; i < size; i++ ) {
        switchingReaction = switchingReactions[i];
        if( switchingReaction == reaction ) {
            (counts[i])++; 
        }
    }
    return SUCCESS;
}
