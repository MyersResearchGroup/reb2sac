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
#include "logical_species_node.h"

static double _GetInitialConcentration( SPECIES *species );

static SPECIES *_Clone( SPECIES *species );    
static char *_GetType( );                                                                              
static RET_VAL _ReleaseResource( SPECIES *species );

LOGICAL_SPECIES *CreateLogicalSpeciesFromSpecies( SPECIES *species, STRING *newName, int order, int highestLevel, double criticalConcentration ) {
    double initialCon = 0.0;
    STRING *originalSpeciesName = NULL;
    LOGICAL_SPECIES *logicalSpecies = NULL;
    
    START_FUNCTION("CreateLogicalSpeciesFromSpecies");
    
    if( ( logicalSpecies = (LOGICAL_SPECIES*)MALLOC( sizeof( LOGICAL_SPECIES ) ) ) == NULL ) {
        END_FUNCTION("CreateLogicalSpeciesFromSpecies", FAILING );
        return NULL;
    }
       
    if( IS_FAILED( CopyIRNode( species, logicalSpecies ) ) ) {
        END_FUNCTION("CreateLogicalSpeciesFromSpecies", FAILING );
        return NULL;
    }
    
    memcpy( (CADDR_T)logicalSpecies + sizeof(IR_NODE), (CADDR_T)species + sizeof(IR_NODE), sizeof(SPECIES) - sizeof(IR_NODE) );    
    if( IS_FAILED( SetSpeciesNodeName( (SPECIES*)logicalSpecies, newName ) ) ) {
        TRACE_3("name of %ith logical species of %s %s could not be set", 
            order, GetCharArrayOfString( GetSpeciesNodeName( species ) ), GetCharArrayOfString( newName ) );
        END_FUNCTION("CreateLogicalSpeciesFromSpecies", FAILING );
        return NULL;
    }
   
    initialCon = _GetInitialConcentration( species );
    if( initialCon >= criticalConcentration ) {
        if( IS_FAILED( SetInitialConcentrationInSpeciesNode( (SPECIES*)logicalSpecies, 1.0 ) ) ) {
            TRACE_1("error to set the initial concentraion of %s to 1.0", GetCharArrayOfString( GetSpeciesNodeName( (SPECIES*)logicalSpecies ) ) );
        }        
    }
    else {
        if( IS_FAILED( SetInitialConcentrationInSpeciesNode( (SPECIES*)logicalSpecies, 0.0 ) ) ) {
            TRACE_1("error to set the initial concentraion of %s to 0.0", GetCharArrayOfString( GetSpeciesNodeName( (SPECIES*)logicalSpecies ) ) );
        }        
    }
    
    originalSpeciesName = GetSpeciesNodeName( species );
    
    logicalSpecies->Clone = _Clone;
    logicalSpecies->GetType = _GetType;
    logicalSpecies->ReleaseResource = _ReleaseResource;
    logicalSpecies->order = order;
    logicalSpecies->highestLevel = highestLevel;
    logicalSpecies->criticalConcentration = criticalConcentration;   
    logicalSpecies->originalSpeciesName = CloneString( originalSpeciesName );       
    
    END_FUNCTION("CreateLogicalSpeciesFromSpecies", SUCCESS );
    return logicalSpecies;
}

static double _GetInitialConcentration( SPECIES *species ) {
    return GetInitialConcentrationInSpeciesNode( species );
}


int GetOrderInLogicalSpecies( LOGICAL_SPECIES *species ) {
    START_FUNCTION("GetOrderInLogicalSpecies");
    END_FUNCTION("GetOrderInLogicalSpecies", SUCCESS );
    return species->order;
}

int GetHighestLevelInLogicalSpecies( LOGICAL_SPECIES *species ) {
    START_FUNCTION("GetHighestLevelInLogicalSpecies");
    END_FUNCTION("GetHighestLevelInLogicalSpecies", SUCCESS );
    return species->highestLevel;
}


double GetCriticalConcentrationInLogicalSpecies( LOGICAL_SPECIES *species ) {
    START_FUNCTION("GetCriticalConcentrationInLogicalSpecies");
    END_FUNCTION("GetCriticalConcentrationInLogicalSpecies", SUCCESS );
    return species->criticalConcentration;
}

STRING *GetOriginalSpeciesName(LOGICAL_SPECIES *species) {
    START_FUNCTION("GetOriginalSpeciesName");
    END_FUNCTION("GetOriginalSpeciesName", SUCCESS );
    return species->originalSpeciesName;
}


BOOL IsLogicalSpecies( SPECIES *species ) {
    char *type = NULL;
    
    START_FUNCTION("IsLogicalSpecies");
    
    if( species == NULL ) {
        END_FUNCTION("IsLogicalSpecies", SUCCESS );    
        return FALSE;
    }
    type = species->GetType( );
    
    if( strcmp( LOGICAL_SPECIES_TYPE_ID, type ) == 0 ) {
        END_FUNCTION("IsLogicalSpecies", SUCCESS );    
        return TRUE;
    }
    else {
        END_FUNCTION("IsLogicalSpecies", SUCCESS );    
        return FALSE;
    }
}


static SPECIES *_Clone( SPECIES *species ) {
    LOGICAL_SPECIES *clone = NULL;
    
    START_FUNCTION("_Clone");

    if( !IsLogicalSpecies( species ) ) {
        END_FUNCTION("_Clone", FAILING );
        return NULL;
    }
    
    if( ( clone = (LOGICAL_SPECIES*)MALLOC( sizeof( LOGICAL_SPECIES ) ) ) == NULL ) {
        END_FUNCTION("_Clone", FAILING );
        return NULL;
    }
    
    if( IS_FAILED( CopyIRNode( species, clone ) ) ) {
        END_FUNCTION("_Clone", FAILING );
        return NULL;
    }
    memcpy( (CADDR_T)clone + sizeof(IR_NODE), (CADDR_T)species + sizeof(IR_NODE), sizeof(LOGICAL_SPECIES) - sizeof(IR_NODE) );
    clone->originalSpeciesName = CloneString( ((LOGICAL_SPECIES*)species)->originalSpeciesName );   
           
    END_FUNCTION("_Clone", SUCCESS );    
    return (SPECIES*)clone;
}

static char * _GetType( ) {
    START_FUNCTION("_GetType");
    END_FUNCTION("_GetType", SUCCESS );    
    return LOGICAL_SPECIES_TYPE_ID;
}
                                                                          
static RET_VAL _ReleaseResource( SPECIES *species ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("_ReleaseResource");
    if( species == NULL ) {
        return ErrorReport( FAILING, "_ReleaseResource", "input species node is NULL" );
    }
    if( IS_FAILED( ( ret = ReleaseIRNodeResources( (IR_NODE*)species ) ) ) ) {
        END_FUNCTION("_ReleaseResource", ret );
        return ret;
    }
    
    FreeString( &(((LOGICAL_SPECIES*)species)->originalSpeciesName) ); 
        
    END_FUNCTION("_ReleaseResource", SUCCESS );
    return ret;
}


