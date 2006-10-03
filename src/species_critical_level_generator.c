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
#include "species_critical_level_generator.h"

static SPECIES_CRITICAL_LEVEL *_Generate( SPECIES_CRITICAL_LEVEL_GENERATOR *gen, SPECIES *species );
static int _FindInitialLevelIndex( INTERMEDIATE_SPECIES_CRITICAL_LEVEL *intermediateLevels, double initialLevel );
static double _GetInitialLevel( SPECIES *species );
static BOOL _GenerateDegradationFlag( SPECIES *species );
static BOOL _GenerateProductionFlag( SPECIES *species );

SPECIES_CRITICAL_LEVEL_GENERATOR *CreateSpeciesCriticalLevelGenerator( REB2SAC_PROPERTIES *properties ) {
    SPECIES_CRITICAL_LEVEL_GENERATOR *gen = NULL;    
    CRITICAL_LEVEL_FINDER *finder = NULL;
    CRITICAL_LEVEL_ORDER_DECIDER *decider = NULL;
    
    START_FUNCTION("CreateSpeciesCriticalLevelGenerator");

    if( ( gen = (SPECIES_CRITICAL_LEVEL_GENERATOR*)MALLOC( sizeof(SPECIES_CRITICAL_LEVEL_GENERATOR) ) ) == NULL ) {
        TRACE_0( "failed to create a critical level generator");
        END_FUNCTION("CreateSpeciesCriticalLevelGenerator", FAILING );
        return NULL;
    }
    
    if( ( finder = CreateCriticalLevelFinder( properties ) ) == NULL ) {
        END_FUNCTION("CreateSpeciesCriticalLevelGenerator", FAILING );
        return NULL;
    }
    if( ( decider = CreateCriticalLevelOrderDecider( properties ) ) == NULL ) {
        END_FUNCTION("CreateSpeciesCriticalLevelGenerator", FAILING );
        return NULL;
    }  
          
    gen->properties = properties;
    gen->finder = finder;
    gen->decider = decider;
    gen->Generate = _Generate;
    
    END_FUNCTION("CreateSpeciesCriticalLevelGenerator", SUCCESS );
    return gen;
}


RET_VAL FreeSpeciesCriticalLevelGenerator( SPECIES_CRITICAL_LEVEL_GENERATOR **pGen ) {
    RET_VAL ret = SUCCESS;
    SPECIES_CRITICAL_LEVEL_GENERATOR *gen = *pGen;
    
    START_FUNCTION("FreeSpeciesCriticalLevel");
    
    FreeCriticalLevelFinder( &(gen->finder) );
    FreeCriticalLevelOrderDecider( &(gen->decider) ); 
    
    FREE( gen );
    END_FUNCTION("CreateSpeciesCriticalLevelGenerator", SUCCESS );
    
    return ret;
}



static SPECIES_CRITICAL_LEVEL *_Generate( SPECIES_CRITICAL_LEVEL_GENERATOR *gen, SPECIES *species ) {
    BOOL degradationFlag = FALSE;
    BOOL productionFlag = FALSE;
    int offset = 0;
    int i = 0;
    int index = 0;
    int size = 1;
    int initialLevelIndex = 0;
    double initialLevel = 0.0;        
    double *elementArray = NULL;
    double *intermediateLevelArray = NULL;
    CRITICAL_LEVEL_FINDER *finder = gen->finder;
    CRITICAL_LEVEL_ORDER_DECIDER *decider = gen->decider;    
    INTERMEDIATE_SPECIES_CRITICAL_LEVEL *intermediateLevels1 = NULL;
    INTERMEDIATE_SPECIES_CRITICAL_LEVEL *intermediateLevels2 = NULL;
    SPECIES_CRITICAL_LEVEL *criticalLevels = NULL;
     
    if( ( intermediateLevels1 = finder->FindLevels( finder, species ) ) == NULL ) {
        return NULL;
    }
    if( ( intermediateLevels2 = decider->DecideLevels( decider, species, intermediateLevels1 ) ) == NULL ) {
        return NULL;
    }        
    FreeIntermediateSpeciesCriticalLevel( &intermediateLevels1 );
     
    initialLevel = _GetInitialLevel( species );
    initialLevelIndex = _FindInitialLevelIndex( intermediateLevels2, initialLevel );
    
    degradationFlag = _GenerateDegradationFlag( species );
    productionFlag = _GenerateProductionFlag( species );
    
    if( degradationFlag && productionFlag ) {
        size = intermediateLevels2->size;
        offset = 0;
    }
    else if( degradationFlag ) {
        size = initialLevelIndex + 1;
        offset = 0;
    }
    else if( productionFlag ) {
        size = intermediateLevels2->size - initialLevelIndex;
        offset = initialLevelIndex;    
    }
    else {
        size = 1;
        offset = initialLevelIndex;
    }
    
    elementArray = (double*)MALLOC( size * sizeof(double) );
    if( elementArray == NULL ) {
        TRACE_0("could not allocate memory for the array of the critical level elements" );
        return NULL;
    }    
    
    intermediateLevelArray = intermediateLevels2->levels;
    for( i = offset; i < size; i++ ) {
        index = i - offset;
        elementArray[index] = intermediateLevelArray[i];
    }
    
    FreeIntermediateSpeciesCriticalLevel( &intermediateLevels2 );
    
    criticalLevels = (SPECIES_CRITICAL_LEVEL*)MALLOC( sizeof(SPECIES_CRITICAL_LEVEL) );
    if( criticalLevels == NULL ) {
        TRACE_0("could not allocate memory for critical level" );
        return NULL;
    }
    criticalLevels->levels = elementArray;
    criticalLevels->size = size;
    criticalLevels->initialLevelIndex = initialLevelIndex;
    criticalLevels->species = species;
    
    return criticalLevels;
}

static int _FindInitialLevelIndex( INTERMEDIATE_SPECIES_CRITICAL_LEVEL *intermediateLevels, double initialLevel ) {
    int i = 0;
    double *levels = intermediateLevels->levels;
    int size = intermediateLevels->size;
        
    for( i = size - 1; i > 0; i-- ) {
        if( levels[i] <= initialLevel ) {
            return i;
        } 
    }
    
    return 0;        
}

static double _GetInitialLevel( SPECIES *species ) {
    return GetInitialAmountInSpeciesNode( species );
}

static BOOL _GenerateDegradationFlag( SPECIES *species ) {
    LINKED_LIST *list = NULL;
    
    list = GetReactantEdges( (IR_NODE*)species );    
    return ( ( GetLinkedListSize( list ) > 0 ) ? TRUE : FALSE );   
}

static BOOL _GenerateProductionFlag( SPECIES *species ) {
    LINKED_LIST *list = NULL;
    
    list = GetProductEdges( (IR_NODE*)species );    
    return ( ( GetLinkedListSize( list ) > 0 ) ? TRUE : FALSE );   
}
