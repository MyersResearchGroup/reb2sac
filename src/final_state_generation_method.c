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
#include "abstraction_method_manager.h"
#include "IR.h"
#include "hash_table.h"
#include "symtab.h"
#include "strconv.h"
#include "logical_species_node.h"
#include "kinetic_law_evaluater.h"


typedef struct {
    int type;
    STRING *speciesName;
    LOGICAL_SPECIES *logicalSpecies;
} FINAL_STATE_GENARATOR_INTERNAL_ELEMENT;

 
typedef struct {
    LINKED_LIST *elements;
} FINAL_STATE_GENARATOR_INTERNAL;


static char * _GetFinalStateGenerationMethodID( ABSTRACTION_METHOD *method );
static RET_VAL _ApplyFinalStateGenerationMethod( ABSTRACTION_METHOD *method, IR *ir );      
static RET_VAL _DoTransformation( ABSTRACTION_METHOD *method, IR *ir, FINAL_STATE_GENARATOR_INTERNAL_ELEMENT *element );

static RET_VAL _InitFinalStateInternal( ABSTRACTION_METHOD *method, IR *ir, FINAL_STATE_GENARATOR_INTERNAL *internal );
static BOOL _FindLogicalSpeciesForInternalElement( FINAL_STATE_GENARATOR_INTERNAL_ELEMENT *element, IR *ir );
static RET_VAL _FreeFinalStateInternal( FINAL_STATE_GENARATOR_INTERNAL *internal );
static LINKED_LIST *_CreateReactionList( FINAL_STATE_GENARATOR_INTERNAL *internal );
static int _CompareLogicalSpecies( CADDR_T species1, CADDR_T species2 );

ABSTRACTION_METHOD *FinalStateGenerationMethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager ) {
    static ABSTRACTION_METHOD method;
    
    START_FUNCTION("FinalStateGenerationMethodConstructor");

    if( method.manager == NULL ) {
        method.manager = manager;
        method.GetID = _GetFinalStateGenerationMethodID;
        method.Apply = _ApplyFinalStateGenerationMethod;
    }
    
    TRACE_0( "FinalStateGenerationMethodConstructor invoked" );
    
    END_FUNCTION("FinalStateGenerationMethodConstructor", SUCCESS );
    return &method;
}



static char * _GetFinalStateGenerationMethodID( ABSTRACTION_METHOD *method ) {
    START_FUNCTION("_GetFinalStateGenerationMethodID");
    
    END_FUNCTION("_GetFinalStateGenerationMethodID", SUCCESS );
    return "final-state-generator";
}



static RET_VAL _ApplyFinalStateGenerationMethod( ABSTRACTION_METHOD *method, IR *ir ) {
    static BOOL firstTime = TRUE;
    RET_VAL ret = SUCCESS;
    FINAL_STATE_GENARATOR_INTERNAL_ELEMENT *element = NULL;
    LINKED_LIST *elements = NULL;
    FINAL_STATE_GENARATOR_INTERNAL internal;
    
    START_FUNCTION("_ApplyFinalStateGenerationMethod");

    if( !firstTime ) {
        END_FUNCTION("_ApplyFinalStateGenerationMethod", SUCCESS );
        return SUCCESS;
    }
    firstTime = FALSE;
    
    if( IS_FAILED( ( ret = _InitFinalStateInternal( method, ir, &internal ) ) ) ) {
        END_FUNCTION("_ApplyFinalStateGenerationMethod", ret );
        return ret;
    } 
    
    elements = internal.elements;
    ResetCurrentElement( elements );    
    while( ( element = (FINAL_STATE_GENARATOR_INTERNAL_ELEMENT*)GetNextFromLinkedList( elements ) ) != NULL ) {
        if( IS_FAILED( ( ret = _DoTransformation( method, ir, element ) ) ) ) {
            END_FUNCTION("_ApplyFinalStateGenerationMethod", ret );
            return ret;
        }
    }
    if( IS_FAILED( ( ret = _FreeFinalStateInternal( &internal ) ) ) ) {
        END_FUNCTION("_ApplyFinalStateGenerationMethod", ret );
        return ret;
    } 
            
    END_FUNCTION("_ApplyFinalStateGenerationMethod", SUCCESS );
    return ret;
}      

static RET_VAL _DoTransformation( ABSTRACTION_METHOD *method, IR *ir, FINAL_STATE_GENARATOR_INTERNAL_ELEMENT *element ) {
    RET_VAL ret = SUCCESS;
    int type = 0;
    int reactantsNum = 0;
    int productsNum = 0;
    IR_EDGE *edge = NULL;
    LOGICAL_SPECIES *logicalSpecies = NULL;
    REACTION *reaction = NULL;
    LINKED_LIST *list = NULL;    
    LINKED_LIST *edges = NULL;
        
    START_FUNCTION("_DoTransformation");
    
    type = element->type;
    logicalSpecies = element->logicalSpecies;
    
    if( type == REB2SAC_TYPE_FINAL_STATE_HIGH ) {
        edges = GetReactantEdges( (IR_NODE*)logicalSpecies );
    }
    else if( type == REB2SAC_TYPE_FINAL_STATE_LOW ) {
        edges = GetProductEdges( (IR_NODE*)logicalSpecies );
    }
    else {
        return ErrorReport( FAILING, "_DoTransformation", "invalid final state type" );
    }
    
    ResetCurrentElement( edges );
    while( ( edge = GetNextEdge( edges ) ) != NULL ) {
        reaction = GetReactionInIREdge( edge );
        list = GetReactantsInReactionNode( reaction );
        reactantsNum = GetLinkedListSize( list );
        
        list = GetProductEdges( (IR_NODE*)reaction );
        productsNum = GetLinkedListSize( list );
        if( ( reactantsNum + productsNum ) != 1 ) {
            continue;
        }
        if( IS_FAILED( ( ret = ir->RemoveReaction( ir, reaction ) ) ) ) {
            END_FUNCTION("_DoTransformation", ret );
            return ret;
        }
    }
    
    END_FUNCTION("_DoTransformation", SUCCESS );
    return ret;
}



static RET_VAL _InitFinalStateInternal( ABSTRACTION_METHOD *method, IR *ir, FINAL_STATE_GENARATOR_INTERNAL *internal ) {
    RET_VAL ret = SUCCESS;
    int i = 0;
    char buf[REB2SAC_LOGICAL_SPECIES_NAME_SIZE];
    char speciesNameBuf[REB2SAC_LOGICAL_SPECIES_NAME_SIZE]; 
    char *valueString = NULL;
    FINAL_STATE_GENARATOR_INTERNAL_ELEMENT *element = NULL;    
    LINKED_LIST *elements = NULL;
    ABSTRACTION_METHOD_MANAGER *manager = NULL;
    COMPILER_RECORD_T *record = NULL;
    REB2SAC_PROPERTIES *properties = NULL;
    
    START_FUNCTION("_InitFinalStateInternal");
    
    manager = method->manager;
    record = manager->GetCompilerRecord( manager );
    properties = record->properties;
    
    if( ( elements = CreateLinkedList() ) == NULL ) {
        return ErrorReport( FAILING, "_InitFinalStateInternal", "could not create a list for internal elements" );
    }
    
    for( i = 1; ; i++ ) {
        sprintf( buf, "%s%i",  REB2SAC_FINAL_STATE_KEY_PREFIX, i );
        if( ( valueString = properties->GetProperty( properties, buf ) ) == NULL ) {
            break;
        }
        TRACE_2( "final state spec %i is %s", i, valueString );
        if( strncmp( valueString, "high.", 5 /* strlen("high.") */ ) == 0 ) {
            if( ( element = (FINAL_STATE_GENARATOR_INTERNAL_ELEMENT*)MALLOC( sizeof(FINAL_STATE_GENARATOR_INTERNAL_ELEMENT) ) ) == NULL ) {
                return ErrorReport( FAILING, "_InitFinalStateInternal", "could not create an element for %s", valueString );            
            } 
            element->type = REB2SAC_TYPE_FINAL_STATE_HIGH;
            strcpy( speciesNameBuf, valueString + 5 /* strlen("high.") */ );
            if( ( element->speciesName = CreateString( speciesNameBuf ) ) == NULL ) {
                return ErrorReport( FAILING, "_InitFinalStateInternal", "could not create a species name string", speciesNameBuf );            
            }
            if( !_FindLogicalSpeciesForInternalElement( element, ir ) ) {
                FreeString( &(element->speciesName) );
                FREE( element );
                continue;
            }
            if( IS_FAILED( ( ret = AddElementInLinkedList( (CADDR_T)element, elements ) ) ) ) {
                END_FUNCTION("_InitFinalStateInternal", ret );
                return ret;
            } 
        }
        else if( strncmp( valueString, "low.", 4 /* strlen("low.") */ ) == 0 ) {
            if( ( element = (FINAL_STATE_GENARATOR_INTERNAL_ELEMENT*)MALLOC( sizeof(FINAL_STATE_GENARATOR_INTERNAL_ELEMENT) ) ) == NULL ) {
                return ErrorReport( FAILING, "_InitFinalStateInternal", "could not create an element for %s", valueString );
            } 
            element->type = REB2SAC_TYPE_FINAL_STATE_LOW;
            strcpy( speciesNameBuf, valueString + 4 /* strlen("low.") */ );
            if( ( element->speciesName = CreateString( speciesNameBuf ) ) == NULL ) {
                return ErrorReport( FAILING, "_InitFinalStateInternal", "could not create a species name string", speciesNameBuf );            
            } 
            if( !_FindLogicalSpeciesForInternalElement( element, ir ) ) {
                FreeString( &(element->speciesName) );
                FREE( element );
                continue;
            }
            if( IS_FAILED( ( ret = AddElementInLinkedList( (CADDR_T)element, elements ) ) ) ) {
                END_FUNCTION("_InitFinalStateInternal", ret );
                return ret;
            } 
        }
        else {
            TRACE_1("%s is not a valid final state property", valueString );
            continue;
        }        
    }
    
    internal->elements = elements;
    END_FUNCTION("_InitFinalStateInternal", SUCCESS );
    return ret;
}

static BOOL _FindLogicalSpeciesForInternalElement( FINAL_STATE_GENARATOR_INTERNAL_ELEMENT *element, IR *ir ) {
    int type = 0;
    int highestLevel = 0;
    int order = 0;
    STRING *originalSpeciesName = NULL;
    SPECIES *species = NULL;
    LOGICAL_SPECIES *logicalSpecies = NULL;
    LINKED_LIST *speciesList = NULL;
    
    START_FUNCTION("_FindLogicalSpeciesForInternalElement");
    
    type = element->type;
    
    speciesList = ir->GetListOfSpeciesNodes( ir );    
    ResetCurrentElement( speciesList );    
    while( ( species = (SPECIES*)GetNextFromLinkedList( speciesList ) ) != NULL ) {
        if( !IsLogicalSpecies( species ) ) {
            continue;
        }
        logicalSpecies = (LOGICAL_SPECIES*)species;
        originalSpeciesName = GetOriginalSpeciesName( logicalSpecies );
        if( CompareString( originalSpeciesName, element->speciesName ) != 0 ) {
            continue;
        }
        if( type == REB2SAC_TYPE_FINAL_STATE_HIGH ) {
            highestLevel = GetHighestLevelInLogicalSpecies( logicalSpecies );
            order = GetOrderInLogicalSpecies( logicalSpecies );
            if( highestLevel == order ) {
                TRACE_1("setting %s as a high final state species", GetCharArrayOfString( GetSpeciesNodeName( logicalSpecies ) ) );
                element->logicalSpecies = logicalSpecies;
                END_FUNCTION("_FindLogicalSpeciesForInternalElement", SUCCESS );
                return TRUE;
            }  
        }
        else if( type == REB2SAC_TYPE_FINAL_STATE_LOW ) {
            order = GetOrderInLogicalSpecies( logicalSpecies );
            if( order == 1 ) {
                TRACE_1("setting %s as a low final state species", GetCharArrayOfString( GetSpeciesNodeName( logicalSpecies ) ) );
                element->logicalSpecies = logicalSpecies;
                END_FUNCTION("_FindLogicalSpeciesForInternalElement", SUCCESS );
                return TRUE;
            }  
        }
        else {
        }
    }    
    
    END_FUNCTION("_FindLogicalSpeciesForInternalElement", SUCCESS );
    return FALSE;
}

static RET_VAL _FreeFinalStateInternal( FINAL_STATE_GENARATOR_INTERNAL *internal ) {
    RET_VAL ret = SUCCESS;
    FINAL_STATE_GENARATOR_INTERNAL_ELEMENT *element = NULL;
    LINKED_LIST *elements = NULL;
    
    START_FUNCTION("_FreeFinalStateInternal");

    elements = internal->elements;
    ResetCurrentElement( elements );
    while( ( element = (FINAL_STATE_GENARATOR_INTERNAL_ELEMENT*)GetNextFromLinkedList( elements ) ) != NULL ) {
        FreeString( &(element->speciesName) );
        FREE( element );
    }        
    DeleteLinkedList( &(internal->elements) );
    
    END_FUNCTION("_FreeFinalStateInternal", SUCCESS );
    return ret;
}

static int _CompareLogicalSpecies( CADDR_T species1, CADDR_T species2 ) {
  return (species1 - species2);
}
