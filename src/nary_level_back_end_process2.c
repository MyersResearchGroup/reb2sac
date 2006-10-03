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
#if 0
#if defined(DEBUG)
#undef DEBUG
#endif
#endif

#include "nary_level_back_end_process.h"


static RET_VAL _GenerateNaryLevels(  BACK_END_PROCESSOR *backend, IR *ir ); 
static RET_VAL _GenerateNaryLevelsForSpecies(  BACK_END_PROCESSOR *backend, SPECIES *species, FILE *file ); 
static RET_VAL _HandleLogicalSpecies( BACK_END_PROCESSOR *backend, FILE *file, LOGICAL_SPECIES *species ); 
static RET_VAL _HandleSpecies( BACK_END_PROCESSOR *backend, FILE *file, SPECIES *species ); 
static RET_VAL _PrintNoLevelFound( BACK_END_PROCESSOR *backend, FILE *file, char *speciesName ); 
static RET_VAL _PrintSpeciesProperties( BACK_END_PROCESSOR *backend, FILE *file, char *speciesName ); 
static RET_VAL _PrintOtherProperties( BACK_END_PROCESSOR *backend, FILE *file ); 

DLLSCOPE RET_VAL STDCALL ProcessNaryLevelBackend2( BACK_END_PROCESSOR *backend, IR *ir ) {
    RET_VAL ret = SUCCESS;

    START_FUNCTION("ProcessNaryLevelBackend2");    
    
    if( IS_FAILED( ( ret = _GenerateNaryLevels( backend, ir ) ) ) ) {
        return ret;
    }
            
    END_FUNCTION("ProcessNaryLevelBackend2", ret );    
    return ret;    
}



DLLSCOPE RET_VAL STDCALL CloseNaryLevelBackend2( BACK_END_PROCESSOR *backend ) {
    RET_VAL ret = SUCCESS;

    START_FUNCTION("CloseNaryLevelBackend2");    

    END_FUNCTION("CloseNaryLevelBackend2", ret );    
    return ret;    
}


static RET_VAL _GenerateNaryLevels(  BACK_END_PROCESSOR *backend, IR *ir ) {
    RET_VAL ret = SUCCESS;
    CRITICAL_CONCENTRATION_FINDER finder;
    FILE *file = NULL;
    NARY_ORDER_DECIDER *decider = NULL;
    ABSTRACTION_METHOD_MANAGER *manager = NULL;
    SPECIES *species = NULL;
    LINKED_LIST *speciesList = NULL;
    CRITICAL_CONCENTRATION_INFO info;
    
    if( ( file = fopen( REB2SAC_NARY_LEVEL_SPECIES_PROPERTIES_FILE_NAME, "w" ) ) == NULL ) {
        return FAILING;
    }    
    
    backend->_internal1 = NULL;
    speciesList = ir->GetListOfSpeciesNodes( ir );    
    ResetCurrentElement( speciesList );    
    while( ( species = (SPECIES*)GetNextFromLinkedList( speciesList ) ) ) {
        if( IS_FAILED( ( ret = _GenerateNaryLevelsForSpecies( backend, species, file ) ) ) ) {
            TRACE_1("error in finding levels for %s", GetCharArrayOfString( GetSpeciesNodeName( species ) ) );
            return ret;
        }   
    }
    
    if( IS_FAILED( ( ret = _PrintOtherProperties( backend, file ) ) ) ) {
        return ret;
    }   
    
    fclose( file );
                
    return ret;    
}

static RET_VAL _GenerateNaryLevelsForSpecies(  BACK_END_PROCESSOR *backend, SPECIES *species, FILE *file ) {
    RET_VAL ret = SUCCESS;
    
    if( IsLogicalSpecies( species ) ) {
        ret = _HandleLogicalSpecies( backend, file, (LOGICAL_SPECIES*)species );
    }
    else {
        ret = _HandleSpecies( backend, file, species );        
    }
    
    return SUCCESS;    
}

static RET_VAL _HandleLogicalSpecies( BACK_END_PROCESSOR *backend, FILE *file, LOGICAL_SPECIES *species ) {
    RET_VAL ret = SUCCESS;
    char *lastOriginalName = (char*)(backend->_internal1);
    char *originalName = NULL;
    int order = 0;
    double criticalCon = 0.0;
    
    originalName = GetCharArrayOfString( GetOriginalSpeciesName( species ) );
    order = GetOrderInLogicalSpecies( species );
    criticalCon = GetCriticalConcentrationInLogicalSpecies( species );
    
    if( ( lastOriginalName == NULL ) || ( strcmp( lastOriginalName, originalName ) != 0 ) ) {
        backend->_internal1 = originalName;
        fprintf( file, NEW_LINE NEW_LINE );
        if( IS_FAILED( ( ret = _PrintSpeciesProperties( backend, file, originalName ) ) ) ) {
            return ret;
        }
    } 
    
    fprintf( file, "%s%s.%i = %g" NEW_LINE, 
             REB2SAC_CRITICAL_CONCENTRATION_LEVEL_KEY_PREFIX, 
             originalName, 
             order,
             criticalCon );        
    
    return SUCCESS;
}

static RET_VAL _HandleSpecies( BACK_END_PROCESSOR *backend, FILE *file, SPECIES *species ) {
    RET_VAL ret = SUCCESS;
    char *originalName = NULL;
    
    originalName = GetCharArrayOfString( GetSpeciesNodeName( species ) );
    
    backend->_internal1 = originalName;
    fprintf( file, NEW_LINE NEW_LINE );
    if( IS_FAILED( ( ret = _PrintSpeciesProperties( backend, file, originalName ) ) ) ) {
        return ret;
    }
     
    if( IS_FAILED( ( ret = _PrintNoLevelFound( backend, file, originalName ) ) ) ) {
        return ret;
    }
    
    return SUCCESS;
}


static RET_VAL _PrintNoLevelFound( BACK_END_PROCESSOR *backend, FILE *file, char *speciesName ) {
    RET_VAL ret = SUCCESS;
    
    fprintf( file, "%s%s.1 = %s" NEW_LINE, 
            REB2SAC_CRITICAL_CONCENTRATION_LEVEL_KEY_PREFIX, 
            speciesName, 
            REB2SAC_NARY_LEVEL_SPECIES_UNKNOWN_VALUE );        
    return ret;
}



static RET_VAL _PrintSpeciesProperties( BACK_END_PROCESSOR *backend, FILE *file, char *speciesName ) {
    RET_VAL ret = SUCCESS;
    
    
    fprintf( file, "%s%s = %s" NEW_LINE,  
            REB2SAC_ABSOLUTE_INHIBITION_THRESHOLD_KEY_PREFIX, 
            speciesName,
            REB2SAC_NARY_LEVEL_SPECIES_UNKNOWN_VALUE );                 
            
    fprintf( file, "%s%s = %s" NEW_LINE NEW_LINE,  
            REB2SAC_CRITICAL_CONCENTRATION_LEVEL_SPECIFICATION_TYPE_KEY_PREFIX, 
            speciesName,
            REB2SAC_CRITICAL_CONCENTRATION_LEVEL_SPECIFICATION_TYPE_ONLY_PROPERTIES_STRING );                 

    return ret;
}


static RET_VAL _PrintOtherProperties( BACK_END_PROCESSOR *backend, FILE *file ) {
    RET_VAL ret = SUCCESS;
    
    fprintf( file, NEW_LINE NEW_LINE );
    fprintf( file, "%s = %s" NEW_LINE, 
            REB2SAC_ANALYSIS_STOP_ENABLED_KEY,
            REB2SAC_ANALYSIS_STOP_ENABLED_VALUE_TRUE 
    );
                    
    fprintf( file, "%s = %s" NEW_LINE, 
            REB2SAC_ANALYSIS_STOP_RATE_KEY,
            REB2SAC_NARY_LEVEL_SPECIES_UNKNOWN_VALUE 
    );
    
    fprintf( file, "%s1 = %s" NEW_LINE, 
            REB2SAC_FINAL_STATE_KEY_PREFIX,
            REB2SAC_NARY_LEVEL_SPECIES_UNKNOWN_VALUE 
    );
                        
    return ret;
}


