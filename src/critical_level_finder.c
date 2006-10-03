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
#include "critical_level_finder.h"
#include "abstraction_method_properties.h"

static INTERMEDIATE_SPECIES_CRITICAL_LEVEL *_FindLevels( CRITICAL_LEVEL_FINDER *finder, SPECIES *species );        
static RET_VAL _FindLevelsFromCalculation( CRITICAL_LEVEL_FINDER *finder, SPECIES *species, LINKED_LIST *list );        
static RET_VAL _FindLevelsFromProperties( CRITICAL_LEVEL_FINDER *finder, SPECIES *species, LINKED_LIST *list );        
static RET_VAL _ReleaseResource( CRITICAL_LEVEL_FINDER *finder );        
static int _GetCriticalLevelsSpecificationType( REB2SAC_PROPERTIES *properties, SPECIES *species );
static double *_GenerateArrayFromList( LINKED_LIST *list );
static RET_VAL _SortLevels( double *levels, int size );
static INTERMEDIATE_SPECIES_CRITICAL_LEVEL *_GenerateIntermediateLevels( double *sortedLevels, int size );
static RET_VAL _AddLevel0( LINKED_LIST *list );
static int _Compare( double *e1, double *e2 );

CRITICAL_LEVEL_FINDER *CreateCriticalLevelFinder( REB2SAC_PROPERTIES *properties ) {
    CRITICAL_LEVEL_FINDER *finder = NULL;
    
    START_FUNCTION("CreateCriticalLevelFinder");
    
    if( ( finder = (CRITICAL_LEVEL_FINDER*)MALLOC( sizeof(CRITICAL_LEVEL_FINDER) ) ) == NULL ) {
        END_FUNCTION("CreateCriticalLevelFinder", FAILING );
        return NULL;
    }
    
    finder->properties = properties;
    finder->FindLevels = _FindLevels;
    finder->ReleaseResource = _ReleaseResource;    
        
    END_FUNCTION("CreateCriticalLevelFinder", SUCCESS );
    return finder;
}


RET_VAL FreeCriticalLevelFinder( CRITICAL_LEVEL_FINDER **pFinder ) {
    CRITICAL_LEVEL_FINDER *finder = *pFinder;
    FREE( finder );
    
    return SUCCESS;
}

static INTERMEDIATE_SPECIES_CRITICAL_LEVEL *_FindLevels( CRITICAL_LEVEL_FINDER *finder, SPECIES *species ) {
    int specType = 0;
    int size = 0;
    REB2SAC_PROPERTIES *properties = finder->properties;
    double *levelArray = NULL;
    LINKED_LIST *list = NULL;
    INTERMEDIATE_SPECIES_CRITICAL_LEVEL *intermediateLevels = NULL;
            
    if( ( list = CreateLinkedList() ) == NULL ) {
        return NULL;
    }
    
    if( IS_FAILED( ( _AddLevel0( list ) ) ) ) {
        return NULL;
    }    

    specType = _GetCriticalLevelsSpecificationType( properties, species );
    if( specType != REB2SAC_CRITICAL_CONCENTRATION_LEVEL_SPECIFICATION_TYPE_ONLY_CALCULATION ) {
        if( IS_FAILED( ( _FindLevelsFromProperties( finder, species, list ) ) ) ) {
            return NULL;
        }       
    }
    if( specType != REB2SAC_CRITICAL_CONCENTRATION_LEVEL_SPECIFICATION_TYPE_ONLY_PROPERTIES ) {
        if( IS_FAILED( ( _FindLevelsFromCalculation( finder, species, list ) ) ) ) {
            return NULL;
        }       
    }
    
    size = GetLinkedListSize( list );
    if( ( levelArray = _GenerateArrayFromList( list ) ) == NULL ) {
        return NULL;
    } 
    
    if( IS_FAILED( ( _SortLevels( levelArray, size ) ) ) ) {
        return NULL;
    }
    
    if( ( intermediateLevels = _GenerateIntermediateLevels( levelArray, size ) ) == NULL ) {
        return NULL;
    }
    
    return intermediateLevels;
}

static RET_VAL _ReleaseResource( CRITICAL_LEVEL_FINDER *finder ) {
    return SUCCESS;
}


static RET_VAL _FindLevelsFromCalculation( CRITICAL_LEVEL_FINDER *finder, SPECIES *species, LINKED_LIST *list ) {
    return SUCCESS;
}

static RET_VAL _FindLevelsFromProperties( CRITICAL_LEVEL_FINDER *finder, SPECIES *species, LINKED_LIST *list ) {
    RET_VAL ret = SUCCESS;
    int i = 0;
    int size = 0;
    char buf[2048];
    char *valueString = NULL;
    double *levels = NULL;
    double *pLevel = NULL;
    REB2SAC_PROPERTIES *properties = finder->properties;

    for( i = 1; ; i++ ) {
        sprintf( buf, "%s%s.%i", REB2SAC_CRITICAL_CONCENTRATION_LEVEL_KEY_PREFIX, 
                 GetCharArrayOfString( GetSpeciesNodeName( species ) ), i );
        if( ( valueString = properties->GetProperty( properties, buf ) ) == NULL ) {
            break;
        }
        if( ( pLevel = (double*)MALLOC( sizeof(double) ) ) == NULL ) {
            return ErrorReport( FAILING, "_FindLevelsFromProperties", "failed to allocated memory for a level" );
        }
        if( IS_FAILED( ( ret = StrToFloat( pLevel, valueString ) ) ) ) {
            return ErrorReport( FAILING, "_FindLevelsFromProperties", "%s is not a real num", valueString );
        }    
        
        if( IS_FAILED( ( ret = AddElementInLinkedList( (CADDR_T)pLevel, list ) ) ) ) {
            return ret;
        } 
    }
            
    return SUCCESS;
}





static int _GetCriticalLevelsSpecificationType( REB2SAC_PROPERTIES *properties, SPECIES *species ) {
    int type = REB2SAC_CRITICAL_CONCENTRATION_LEVEL_SPECIFICATION_TYPE_DEFAULT;
    char buf[2048];
    char *valueString = NULL;
    
    sprintf( buf, "%s%s", 
             REB2SAC_CRITICAL_CONCENTRATION_LEVEL_SPECIFICATION_TYPE_KEY_PREFIX, 
             GetCharArrayOfString( GetSpeciesNodeName( species ) ) );
    if( ( valueString = properties->GetProperty( properties, buf ) ) == NULL ) {
        return type;
    }
    if( strcmp( valueString, REB2SAC_CRITICAL_CONCENTRATION_LEVEL_SPECIFICATION_TYPE_ONLY_PROPERTIES_STRING ) == 0 ) {
        type = REB2SAC_CRITICAL_CONCENTRATION_LEVEL_SPECIFICATION_TYPE_ONLY_PROPERTIES;
        return type;
    }
    else if( strcmp( valueString, REB2SAC_CRITICAL_CONCENTRATION_LEVEL_SPECIFICATION_TYPE_ONLY_CALCULATION_STRING ) == 0 ) {
        type = REB2SAC_CRITICAL_CONCENTRATION_LEVEL_SPECIFICATION_TYPE_ONLY_CALCULATION;
        return type;
    }
    
    return type;
}


static double *_GenerateArrayFromList( LINKED_LIST *list ) {    
    int i = 0;
    int size = 0;
    double *pLevel = NULL;
    double *array = NULL;

    size = GetLinkedListSize( list );
    if( ( array = (double*)MALLOC( size * sizeof(double) ) ) == NULL ) {
        return NULL;
    }
    
    ResetCurrentElement( list );
    for( ; i < size; i++ ){
        pLevel = (double*)GetNextFromLinkedList( list );
        array[i] = *pLevel;
        FREE( pLevel );
    }
    
    return array;
}


static RET_VAL _SortLevels( double *levels, int size ) {
    
    qsort( levels, size, sizeof(double), (int(*)(const void *, const void *))_Compare );
    
    return SUCCESS;
}

static INTERMEDIATE_SPECIES_CRITICAL_LEVEL *_GenerateIntermediateLevels( double *sortedLevels, int size ) {
    RET_VAL ret = SUCCESS;
    int i = 0;
    int j = 0;
    int newSize = 0;
    double *newLevels = NULL;
    SPECIES *species = NULL;
    INTERMEDIATE_SPECIES_CRITICAL_LEVEL *intermediateLevels = NULL;
    
    newSize = size;
    for( i = 1; i < size; i++ ) {
        if( IS_REAL_EQUAL( sortedLevels[i -1], sortedLevels[i] ) ) {
            newSize--;
        } 
    }
    
    intermediateLevels = (INTERMEDIATE_SPECIES_CRITICAL_LEVEL*)MALLOC( sizeof(INTERMEDIATE_SPECIES_CRITICAL_LEVEL) );
    if( intermediateLevels == NULL ) {
        return NULL; 
    }
    newLevels = (double*)MALLOC( newSize * sizeof(double) );
    if( newLevels == NULL ) {
        return NULL; 
    }
    
    newLevels[0] = sortedLevels[0];
    for( i = 1, j = 1; i < size; i++ ) {
        if( !IS_REAL_EQUAL( sortedLevels[i -1], sortedLevels[i] ) ) {
            newLevels[j] = sortedLevels[i];
            j++;
        } 
    }

    intermediateLevels->size = newSize;
    intermediateLevels->levels = newLevels;

    return intermediateLevels;
}


static int _Compare( double *e1, double *e2 ) {
    double c1 = 0.0;
    double c2 = 0.0;
    
    c1 = *e1;
    c2 = *e2;
    
    if( c1 < c2 ) {
        return -1;
    }
    else if( c1 > c2 ) {
        return 1;
    }
    else {
        return 0;
    }
}


static RET_VAL _AddLevel0( LINKED_LIST *list ) {
    RET_VAL ret = SUCCESS;
    double *pLevel0 = NULL;
     
    if( ( pLevel0 = (double*)MALLOC( sizeof(double) ) ) == NULL ) {
        return FAILING;
    }
    *pLevel0 = DEFAULT_LOWEST_CRITICAL_LEVEL;
    
    if( IS_FAILED( ( ret = AddElementInLinkedList( (CADDR_T)pLevel0, list ) ) ) ) {
        return ret;
    } 
    
    return SUCCESS;
}


