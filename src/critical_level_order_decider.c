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
#include "critical_level_order_decider.h"
#include "abstraction_method_properties.h"


static INTERMEDIATE_SPECIES_CRITICAL_LEVEL *_DecideLevels( 
        CRITICAL_LEVEL_ORDER_DECIDER *decider, 
        SPECIES *species, INTERMEDIATE_SPECIES_CRITICAL_LEVEL *intermediateLevels );        

static INTERMEDIATE_SPECIES_CRITICAL_LEVEL *_DecideDistinctLevels( 
        CRITICAL_LEVEL_ORDER_DECIDER *decider, 
        SPECIES *species, INTERMEDIATE_SPECIES_CRITICAL_LEVEL *intermediateLevels );        

static RET_VAL _ReleaseResource( CRITICAL_LEVEL_ORDER_DECIDER *decider );        


CRITICAL_LEVEL_ORDER_DECIDER *CreateCriticalLevelOrderDecider( REB2SAC_PROPERTIES *properties ) {
    CRITICAL_LEVEL_ORDER_DECIDER *decider = NULL;
    
    if( ( decider = (CRITICAL_LEVEL_ORDER_DECIDER*)MALLOC( sizeof(CRITICAL_LEVEL_ORDER_DECIDER) ) ) == NULL ) {
        return NULL;
    }
    
    decider->properties = properties;
    decider->DecideLevels = _DecideLevels;
    decider->ReleaseResource = _ReleaseResource;
    
    return decider;
}


RET_VAL FreeCriticalLevelOrderDecider( CRITICAL_LEVEL_ORDER_DECIDER **pOrderDecider ) {
    CRITICAL_LEVEL_ORDER_DECIDER *decider = *pOrderDecider;
    
    FREE( decider );
    
    return SUCCESS;
}


static INTERMEDIATE_SPECIES_CRITICAL_LEVEL *_DecideLevels( 
        CRITICAL_LEVEL_ORDER_DECIDER *decider, 
        SPECIES *species, 
        INTERMEDIATE_SPECIES_CRITICAL_LEVEL *intermediateLevels ) 
{
    REB2SAC_PROPERTIES *properties = decider->properties;
    char *deciderName = NULL;
    char buf[2048];
    
    if( ( deciderName = properties->GetProperty( properties, REB2SAC_NARY_ORDER_DECIDER_KEY ) ) == NULL ) {
        deciderName = REB2SAC_DEFAULT_NARY_ORDER_DECIDER;
    }

    switch( deciderName[0] ) {
        case 'd':
            if( strcmp( deciderName, "distinct" ) == 0 ) {
                return _DecideDistinctLevels( decider, species, intermediateLevels );  
            }
            else {
                return NULL;
            }
            break;

        default:
        return NULL;                            
    }
    
    
    return NULL;
}

static RET_VAL _ReleaseResource( CRITICAL_LEVEL_ORDER_DECIDER *decider ) {
    return SUCCESS;
}


static INTERMEDIATE_SPECIES_CRITICAL_LEVEL *_DecideDistinctLevels( 
        CRITICAL_LEVEL_ORDER_DECIDER *decider, 
        SPECIES *species, 
        INTERMEDIATE_SPECIES_CRITICAL_LEVEL *intermediateLevels )
{
    INTERMEDIATE_SPECIES_CRITICAL_LEVEL *newLevels = NULL;
    double *newArray = NULL;
    double *array = intermediateLevels->levels;
    int i = 0;
    int size = 0;
    
    newLevels = (INTERMEDIATE_SPECIES_CRITICAL_LEVEL*)MALLOC( sizeof(INTERMEDIATE_SPECIES_CRITICAL_LEVEL) );
    if( newLevels == NULL ) {
        return NULL;
    }
    
    size = intermediateLevels->size;
    if( ( newArray = (double*)MALLOC( size * sizeof(double) ) ) == NULL ) {
        return NULL;
    }
    
    for( ; i < size; i++ ) {
        newArray[i] = array[i];
    }
    
    newLevels->levels = newArray;
    newLevels->size = size;
    
    return newLevels;
}        

