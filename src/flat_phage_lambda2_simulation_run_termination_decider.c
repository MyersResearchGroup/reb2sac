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
#include "flat_phage_lambda2_simulation_run_termination_decider.h"

#define TESTING_FLAT_PHAGE_LAMBDA_SIMULATION_RUN_TERMINATION_DECIDER2 0

static BOOL _IsTerminationConditionMet( FLAT_PHAGE_LAMBDA2_SIMULATION_RUN_TERMINATION_DECIDER *decider, REACTION *reaction, double time );
static RET_VAL _Destroy( FLAT_PHAGE_LAMBDA2_SIMULATION_RUN_TERMINATION_DECIDER *decider );
static RET_VAL _Report( FLAT_PHAGE_LAMBDA2_SIMULATION_RUN_TERMINATION_DECIDER *decider, FILE *file );
    
static FLAT_PHAGE_LAMBDA2_SPECIES_OF_INTEREST_ARRAY *
_CreateLysogenySpeciesArray( SPECIES **speciesArray, int size );
static FLAT_PHAGE_LAMBDA2_SPECIES_OF_INTEREST_ARRAY *
_CreateLysisSpeciesArray( SPECIES **speciesArray, int size );
static FLAT_PHAGE_LAMBDA2_SPECIES_OF_INTEREST_ARRAY *
_CreateSpeciesArrayOfInterest( char *nameOfSpeciesOfInterest, SPECIES **speciesArray, int size );
static SPECIES *
_FindSpeciesFromName( char *nameOfSpeciesOfInterest, SPECIES **speciesArray, int size );

static RET_VAL _FreeSpeciesOfInterestArray( FLAT_PHAGE_LAMBDA2_SPECIES_OF_INTEREST_ARRAY **array );

DLLSCOPE SIMULATION_RUN_TERMINATION_DECIDER * STDCALL CreateFlatPhageLambda2SimulationRunTerminationDecider(        
        BACK_END_PROCESSOR *backend, SPECIES **speciesArray, int size, 
        REACTION **reactionArray, int reactionSize, double timeLimit ) {
    char *valueString = NULL;
    COMPILER_RECORD_T *compRec = backend->record;
    REB2SAC_PROPERTIES *properties = NULL;    
    FLAT_PHAGE_LAMBDA2_SIMULATION_RUN_TERMINATION_DECIDER *decider = NULL;
    
    START_FUNCTION("CreateFlatPhageLambda2SimulationRunTerminationDecider");

    if( ( decider = (FLAT_PHAGE_LAMBDA2_SIMULATION_RUN_TERMINATION_DECIDER*)MALLOC( sizeof(FLAT_PHAGE_LAMBDA2_SIMULATION_RUN_TERMINATION_DECIDER) ) ) == NULL ) {
        TRACE_0("failed to allocate memory for FLAT_PHAGE_LAMBDA2_SIMULATION_RUN_TERMINATION_DECIDER");
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
    decider->lysogenySpeciesArray = _CreateLysogenySpeciesArray( speciesArray, size );
    decider->lysisSpeciesArray = _CreateLysisSpeciesArray( speciesArray, size );
    properties = compRec->properties;
    if( ( valueString = properties->GetProperty( properties, FLAT_PHAGE_LAMBDA2_CI_TOTAL_THRESHOLD_KEY ) ) == NULL ) {
        decider->cIThreshold = DEFAULT_FLAT_PHAGE_LAMBDA2_CI_TOTAL_THRESHOLD_VALUE;        
    }
    else {
        if( IS_FAILED( StrToFloat( &(decider->cIThreshold), valueString ) ) ) {
            decider->cIThreshold = DEFAULT_FLAT_PHAGE_LAMBDA2_CI_TOTAL_THRESHOLD_VALUE;        
        }
    }
    
    if( ( valueString = properties->GetProperty( properties, FLAT_PHAGE_LAMBDA2_CRO_TOTAL_THRESHOLD_KEY ) ) == NULL ) {
        decider->croThreshold = DEFAULT_FLAT_PHAGE_LAMBDA2_CRO_TOTAL_THRESHOLD_VALUE;        
    }
    else {
        if( IS_FAILED( StrToFloat( &(decider->croThreshold), valueString ) ) ) {
            decider->croThreshold = DEFAULT_FLAT_PHAGE_LAMBDA2_CRO_TOTAL_THRESHOLD_VALUE;        
        }
    }

#if TESTING_FLAT_PHAGE_LAMBDA_SIMULATION_RUN_TERMINATION_DECIDER2
    exit(0);
#endif
       
    END_FUNCTION("CreateFlatPhageLambda2SimulationRunTerminationDecider", SUCCESS );
    return (SIMULATION_RUN_TERMINATION_DECIDER*)decider;
}

static BOOL _IsTerminationConditionMet( FLAT_PHAGE_LAMBDA2_SIMULATION_RUN_TERMINATION_DECIDER *decider, REACTION *reaction, double time ) {
    int i = 0;
    int size = 0;
    int multiplier = 0;
    double quantity = 0.0;
    SPECIES *species = NULL;
    FLAT_PHAGE_LAMBDA2_SPECIES_OF_INTEREST_ELEMENT *element = NULL;
    FLAT_PHAGE_LAMBDA2_SPECIES_OF_INTEREST_ELEMENT **elements = NULL;
    FLAT_PHAGE_LAMBDA2_SPECIES_OF_INTEREST_ARRAY *lysogenySpeciesArray = decider->lysogenySpeciesArray;
    FLAT_PHAGE_LAMBDA2_SPECIES_OF_INTEREST_ARRAY *lysisSpeciesArray = decider->lysisSpeciesArray;
    double cIThreshold = decider->cIThreshold;
    double croThreshold = decider->croThreshold;    
            
    if( time >= decider->timeLimit ) {
        return TRUE;
    }
    
    size = lysogenySpeciesArray->size;
    elements = lysogenySpeciesArray->elements;
    quantity = 0.0;
    for( i = 0; i < size; i++ ) {
        element = elements[i];
        species = element->species;
        multiplier = element->number;
        quantity += (GetConcentrationInSpeciesNode(species) * multiplier);
    }    
    if( quantity >= cIThreshold ) {
        decider->lysogenyNum++;
        return TRUE;
    }
    
    size = lysisSpeciesArray->size;
    elements = lysisSpeciesArray->elements;
    quantity = 0.0;
    for( i = 0; i < size; i++ ) {
        element = elements[i];
        species = element->species;
        multiplier = element->number;
        quantity += (GetConcentrationInSpeciesNode(species) * multiplier);
    }    
    if( quantity >= croThreshold ) {
        decider->lysisNum++;
        return TRUE;
    }
        
    return FALSE;        
}

static RET_VAL _Report( FLAT_PHAGE_LAMBDA2_SIMULATION_RUN_TERMINATION_DECIDER *decider, FILE *file ) {
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


static RET_VAL _Destroy( FLAT_PHAGE_LAMBDA2_SIMULATION_RUN_TERMINATION_DECIDER *decider ) {
    RET_VAL ret = SUCCESS;
    
    if( IS_FAILED( ( ret = _FreeSpeciesOfInterestArray( &(decider->lysisSpeciesArray) ) ) ) ) {
        return ret;
    }
    if( IS_FAILED( ( ret = _FreeSpeciesOfInterestArray( &(decider->lysogenySpeciesArray) ) ) ) ) {
        return ret;
    }
    
    FREE( decider );
    
    return ret;
}


static FLAT_PHAGE_LAMBDA2_SPECIES_OF_INTEREST_ARRAY *
_CreateLysogenySpeciesArray( SPECIES **speciesArray, int size ) {
    FLAT_PHAGE_LAMBDA2_SPECIES_OF_INTEREST_ARRAY *array = NULL;
    array = _CreateSpeciesArrayOfInterest( 
                FLAT_PHAGE_LAMBDA2_NAME_OF_SPECIES_OF_INTEREST_FOR_LYSOGENY,
                speciesArray,
                size );

#if TESTING_FLAT_PHAGE_LAMBDA_SIMULATION_RUN_TERMINATION_DECIDER2
    printf( "for " FLAT_PHAGE_LAMBDA2_NAME_OF_SPECIES_OF_INTEREST_FOR_LYSOGENY NEW_LINE );
    for( array->size--; array->size >= 0; array->size-- ) {
        printf( "{%s, %i}" NEW_LINE, 
            GetCharArrayOfString( GetSpeciesNodeName( (array->elements[array->size])->species ) ),
            (array->elements[array->size])->number ); 
    }
#endif
    
    return array;
}

static FLAT_PHAGE_LAMBDA2_SPECIES_OF_INTEREST_ARRAY *
_CreateLysisSpeciesArray( SPECIES **speciesArray, int size ) {
    FLAT_PHAGE_LAMBDA2_SPECIES_OF_INTEREST_ARRAY *array = NULL;
    array = _CreateSpeciesArrayOfInterest( 
                FLAT_PHAGE_LAMBDA2_NAME_OF_SPECIES_OF_INTEREST_FOR_LYSIS,
                speciesArray,
                size );

#if TESTING_FLAT_PHAGE_LAMBDA_SIMULATION_RUN_TERMINATION_DECIDER2
    printf( "for " FLAT_PHAGE_LAMBDA2_NAME_OF_SPECIES_OF_INTEREST_FOR_LYSIS NEW_LINE );
    for( array->size--; array->size >= 0; array->size-- ) {
        printf( "{%s, %i}" NEW_LINE, 
            GetCharArrayOfString( GetSpeciesNodeName( (array->elements[array->size])->species ) ),
            (array->elements[array->size])->number ); 
    }
#endif
    
    return array;
}

static FLAT_PHAGE_LAMBDA2_SPECIES_OF_INTEREST_ARRAY *
_CreateSpeciesArrayOfInterest( char *nameOfSpeciesOfInterest, SPECIES **speciesArray, int size ) {
    int i = 0;
    int listSize = 0;
    int stoichiometry = 0;
    int multiplier = 0;
    SPECIES *species = NULL;
    SPECIES *relevantSpecies = NULL;
    REACTION *reaction = NULL;
    IR_EDGE *relevantEdge = NULL;
    IR_EDGE *edge = NULL;
    LINKED_LIST *relevantEdges = NULL;
    LINKED_LIST *list = NULL;
    LINKED_LIST *edges = NULL;
    SPECIES *speciesOfInterest = NULL;
    FLAT_PHAGE_LAMBDA2_SPECIES_OF_INTEREST_ELEMENT *element = NULL;
    FLAT_PHAGE_LAMBDA2_SPECIES_OF_INTEREST_ELEMENT **elements = NULL;
    FLAT_PHAGE_LAMBDA2_SPECIES_OF_INTEREST_ARRAY *array = NULL;
    
    if( ( speciesOfInterest = _FindSpeciesFromName( nameOfSpeciesOfInterest, speciesArray, size ) ) == NULL ) {
        TRACE_1( "species %s is not found", nameOfSpeciesOfInterest );
        return NULL;
    }
    
    if( ( list = CreateLinkedList() ) == NULL ) {
        TRACE_1( "temp list for species %s could not be created", nameOfSpeciesOfInterest );
        return NULL;    
    }

    if( ( element = (FLAT_PHAGE_LAMBDA2_SPECIES_OF_INTEREST_ELEMENT*)MALLOC( sizeof(FLAT_PHAGE_LAMBDA2_SPECIES_OF_INTEREST_ELEMENT) ) ) == NULL ) {
        TRACE_2( "element %s for species %s could not be created", nameOfSpeciesOfInterest, nameOfSpeciesOfInterest );
        return NULL;                
    }
    element->species = speciesOfInterest;
    element->number = 2; 
    if( IS_FAILED( AddElementInLinkedList( (CADDR_T)element, list ) ) ) {
        TRACE_2( "element %s for species %s could not be added", nameOfSpeciesOfInterest, nameOfSpeciesOfInterest );
        return NULL;                
    }
            
    relevantEdges = GetReactantEdges( (IR_NODE*)speciesOfInterest );
    ResetCurrentElement( relevantEdges );
    while( ( relevantEdge = GetNextEdge( relevantEdges ) ) != NULL ) {
        reaction = GetReactionInIREdge( relevantEdge );
        multiplier = GetStoichiometryInIREdge( relevantEdge );
        edges = GetProductEdges( (IR_NODE*)reaction );
        if( GetLinkedListSize( edges ) != 1 ) {
            continue;
        }
        edge = GetHeadEdge( edges );
        stoichiometry = GetStoichiometryInIREdge( edge );
        relevantSpecies = GetSpeciesInIREdge( edge );
        if( ( element = (FLAT_PHAGE_LAMBDA2_SPECIES_OF_INTEREST_ELEMENT*)MALLOC( sizeof(FLAT_PHAGE_LAMBDA2_SPECIES_OF_INTEREST_ELEMENT) ) ) == NULL ) {
            TRACE_2( "element %s for species %s could not be created", GetCharArrayOfString( GetSpeciesNodeName( relevantSpecies ) ), nameOfSpeciesOfInterest );
            return NULL;                
        }
        element->species = relevantSpecies;
        if( stoichiometry != 1 ) {
            element->number =  multiplier; 
        }
        else {
            element->number =  2 * multiplier; 
        }
        if( IS_FAILED( AddElementInLinkedList( (CADDR_T)element, list ) ) ) {
            TRACE_2( "element %s for species %s could not be added", GetCharArrayOfString( GetSpeciesNodeName( relevantSpecies ) ), nameOfSpeciesOfInterest );
            return NULL;                
        }
    }
    
    listSize = GetLinkedListSize( list );    
    if( ( elements = (FLAT_PHAGE_LAMBDA2_SPECIES_OF_INTEREST_ELEMENT**)MALLOC( 
                sizeof(FLAT_PHAGE_LAMBDA2_SPECIES_OF_INTEREST_ELEMENT*) * listSize ) ) == NULL ) {
        TRACE_1( "elements space for species %s could not be created", nameOfSpeciesOfInterest );
        return NULL;
    }
    ResetCurrentElement( list );    
    while( ( element = (FLAT_PHAGE_LAMBDA2_SPECIES_OF_INTEREST_ELEMENT*)GetNextFromLinkedList( list ) ) != NULL ) {
        elements[i] = element;
        i++;
    }
    if( ( array = (FLAT_PHAGE_LAMBDA2_SPECIES_OF_INTEREST_ARRAY*)MALLOC( sizeof(FLAT_PHAGE_LAMBDA2_SPECIES_OF_INTEREST_ARRAY) ) ) == NULL ) {
        TRACE_1( "array space for species %s could not be created", nameOfSpeciesOfInterest );
        return NULL;
    }
    
    array->elements = elements;
    array->size = listSize;
        
    return array;
}

static SPECIES *
_FindSpeciesFromName( char *nameOfSpeciesOfInterest, SPECIES **speciesArray, int size ) {
    int i = 0;
    SPECIES *species = NULL;
    char *nameString = NULL;
    
    for( i = 0; i < size; i++ ) {
        species = speciesArray[i];
        nameString = GetCharArrayOfString( GetSpeciesNodeName( species ) );
        if( strcmp( nameString, nameOfSpeciesOfInterest ) == 0 ) {
            return species;
        }            
    }
        
    return NULL;
}

static RET_VAL _FreeSpeciesOfInterestArray( FLAT_PHAGE_LAMBDA2_SPECIES_OF_INTEREST_ARRAY **array ) {
    RET_VAL ret = SUCCESS;
    int i = 0;
    int size = 0;
    FLAT_PHAGE_LAMBDA2_SPECIES_OF_INTEREST_ELEMENT *element = NULL;
    FLAT_PHAGE_LAMBDA2_SPECIES_OF_INTEREST_ELEMENT **elements;
    FLAT_PHAGE_LAMBDA2_SPECIES_OF_INTEREST_ARRAY *temp = *array;
    
    size = temp->size;
    elements = temp->elements;
    
    for( i = 0; i < size; i++ ) {
        element = elements[i];
        FREE( element );        
    }
    FREE( elements );
    FREE( temp );
    *array = NULL;
    
    return ret;
}



