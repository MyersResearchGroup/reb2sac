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

#define TESTING_NARY_LEVEL_BACKEND 1

static RET_VAL _GenerateSBMLForAbstractedModel(  BACK_END_PROCESSOR *backend, IR *ir ); 
static RET_VAL _GenerateNaryLevels(  BACK_END_PROCESSOR *backend, IR *ir ); 
static RET_VAL _GenerateNaryLevelsForSpecies(  BACK_END_PROCESSOR *backend, SPECIES *species, CRITICAL_CONCENTRATION_INFO *info ); 
static RET_VAL _PrintNoLevelFound( BACK_END_PROCESSOR *backend, FILE *file, SPECIES *species ); 
static RET_VAL _CleanCriticalConInfo( CRITICAL_CONCENTRATION_INFO *info );
static RET_VAL _PrintSpeciesProperties( BACK_END_PROCESSOR *backend, FILE *file, SPECIES *species ); 
static RET_VAL _PrintOtherProperties( BACK_END_PROCESSOR *backend, FILE *file ); 

DLLSCOPE RET_VAL STDCALL ProcessNaryLevelBackend( BACK_END_PROCESSOR *backend, IR *ir ) {
    RET_VAL ret = SUCCESS;

    START_FUNCTION("ProcessNaryLevelBackend");    
    
    if( IS_FAILED( ( ret = _GenerateNaryLevels( backend, ir ) ) ) ) {
        return ret;
    }
    /*            
    if( IS_FAILED( ( ret = _GenerateSBMLForAbstractedModel( backend, ir ) ) ) ) {
        return ret;
    }
    */
    END_FUNCTION("ProcessNaryLevelBackend", ret );    
    return ret;    
}



DLLSCOPE RET_VAL STDCALL CloseNaryLevelBackend( BACK_END_PROCESSOR *backend ) {
    RET_VAL ret = SUCCESS;

    START_FUNCTION("CloseNaryLevelBackend");    

    END_FUNCTION("CloseNaryLevelBackend", ret );    
    return ret;    
}



static RET_VAL _GenerateSBMLForAbstractedModel(  BACK_END_PROCESSOR *backend, IR *ir ) {
    RET_VAL ret = SUCCESS;
    char *filename;
    FILE *file = NULL;

    if( ( filename = backend->outputFilename ) == NULL ) {
        filename = REB2SAC_DEFAULT_NARY_LEVEL_OUTPUT_NAME;
    }
    if( ( file = fopen( filename, "w" ) ) == NULL ) {
        return FAILING;
    }
    if( IS_FAILED( ( ret = ir->GenerateSBML( ir, file ) ) ) ) {
        return ret;
    }
    fclose( file );
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
    if( ( manager = GetAbstractionMethodManagerInstance() ) == NULL ) {
        return FAILING;
    }
    if( ( decider = GetNaryOrderDeciderInstance( manager ) ) == NULL ) {
        return FAILING;
    }        
    if( IS_FAILED( InitCriticalConcentrationFinder( &finder, manager->GetCompilerRecord( manager ) ) ) ) {
        END_FUNCTION("NaryOrderUnaryTransformationMethodConstructor", FAILING );
        return FAILING;
    }
    backend->_internal1 = (CADDR_T)(file);
    backend->_internal2 = (CADDR_T)(&finder);
    backend->_internal3 = (CADDR_T)(decider);
             
    speciesList = ir->GetListOfSpeciesNodes( ir );    
    ResetCurrentElement( speciesList );    
    while( ( species = (SPECIES*)GetNextFromLinkedList( speciesList ) ) ) {
        if( IS_FAILED( ( ret = _GenerateNaryLevelsForSpecies( backend, species, &info ) ) ) ) {
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

static RET_VAL _GenerateNaryLevelsForSpecies(  BACK_END_PROCESSOR *backend, SPECIES *species, CRITICAL_CONCENTRATION_INFO *info ) {
    RET_VAL ret = SUCCESS;
    int i = 0;
    int len = 0;
    FILE *file = NULL;
    double criticalConcentration = 0.0;
    double *concentrationListElement = NULL;
    double *criticalCons = NULL;
    IR_EDGE *edge = NULL;
    REACTION *reaction = NULL;
    KINETIC_LAW *kineticLaw = NULL;
    LINKED_LIST *edges = NULL;
    LINKED_LIST *list = NULL;
    LINKED_LIST *concentrationList = NULL;
    HASH_TABLE *table = NULL;
    NARY_ORDER_DECIDER *decider = NULL;
    CRITICAL_CONCENTRATION_FINDER *finder = NULL;
    CRITICAL_CONCENTRATION_ELEMENT *elements = NULL;    

    file = (FILE*)(backend->_internal1);
    finder = (CRITICAL_CONCENTRATION_FINDER*)(backend->_internal2);
    decider = (NARY_ORDER_DECIDER*)(backend->_internal3);

    
    /*
    first, print bunch of properties for this species
    */
    if( IS_FAILED( ( ret = _PrintSpeciesProperties( backend, file, species ) ) ) ) {
        return ret;
    }
    
    /*
     * species S must be either produced or consumed 
     */  
    
    edges = GetReactantEdges( species );
    if( GetLinkedListSize( edges ) == 0 ) {
        edges = GetProductEdges( species );
        if( GetLinkedListSize( edges ) == 0 ) {
            ret = _PrintNoLevelFound( backend, file, species );            
            return ret;
        }
    }
    
    edges = GetReactantEdges( species );
    ResetCurrentElement( edges );
    while( ( edge = GetNextEdge( edges ) ) != NULL ) {
        /*
        * for each r in { reactions which use S as a reactant }
        *  S is the only reactant in r
        */  
        reaction = GetReactionInIREdge( edge );
        list = GetReactantEdges( reaction );
        if( GetLinkedListSize( list ) > 1 ) {
            ret = _PrintNoLevelFound( backend, file, species );            
            return ret;
        }
        /*
        * for each r in { reactions which use S as a reactant }
        *  there is no product in r
        */  
        list = GetProductEdges( reaction );
        if( GetLinkedListSize( list ) > 0 ) {
            ret = _PrintNoLevelFound( backend, file, species );            
            return ret;
        }    
    }        
    
    edges = GetProductEdges( species );
    ResetCurrentElement( edges );
    while( ( edge = GetNextEdge( edges ) ) != NULL ) {
        /*
        * for each r in { reactions which use S as a product }
        *  there is no reactant in r
        */  
        reaction = GetReactionInIREdge( edge );
        list = GetReactantEdges( reaction );
        if( GetLinkedListSize( list ) > 0 ) {
            ret = _PrintNoLevelFound( backend, file, species );            
            return ret;
        }
        /*
        * for each r in { reactions which use S as a product }
        *  S is the only product in r
        */  
        list = GetProductEdges( reaction );
        if( GetLinkedListSize( list ) > 1 ) {
            ret = _PrintNoLevelFound( backend, file, species );            
            return ret;
        }    
    }
    
    if( ( concentrationList = CreateLinkedList() ) == NULL ) {
        TRACE_1( "failed to create critical concentration list %s", GetCharArrayOfString( GetSpeciesNodeName( species ) ) ); 
        return FAILING;
    } 
    
    list = GetModifierEdges( species );
    ResetCurrentElement( list );
    while( ( edge = GetNextEdge( list ) ) != NULL ) {
        reaction = GetReactionInIREdge( edge );    
        if( IS_FAILED( ( ret = finder->AddListOfCriticalConcentrationLevels( finder, reaction, species, concentrationList ) ) ) ) {
            TRACE_1( "failed to add critical concentration for %s", GetCharArrayOfString( GetSpeciesNodeName( species ) ) ); 
            return FAILING;
        }
    }
        
    len = GetLinkedListSize( concentrationList );
    if( len == 0 ) {
        ret = _PrintNoLevelFound( backend, file, species );            
        return ret;
    } 
    
    if( ( criticalCons = (double*)MALLOC( sizeof(double) * len ) ) == NULL ) {
        TRACE_1( "failed to create critical concentrations for %s", GetCharArrayOfString( GetSpeciesNodeName( species ) ) ); 
        return FAILING;
    }

    ResetCurrentElement( concentrationList );
    i = 0;
    while( ( concentrationListElement = (double*)GetNextFromLinkedList( concentrationList ) ) != NULL ) {
        criticalCons[i] = *concentrationListElement;
        FREE( concentrationListElement );
        i++;
    }    
    DeleteLinkedList( &concentrationList );

    info->species = species;        

    /*
    * 
    * default decider, a.k.a distinct decider, does not use the first argument 
    */    
    if( IS_FAILED( ( ret = decider->Decide( NULL, info, criticalCons, len ) ) ) ) {
        FREE( criticalCons );
        return ret;
    } 
            
    FREE( criticalCons );
    len = info->len;
    elements = info->elements;
    
    for( i = 0; i < len; i++ ) {
        fprintf( file, "%s%s.%i = %g" NEW_LINE, 
                REB2SAC_CRITICAL_CONCENTRATION_LEVEL_KEY_PREFIX, 
                GetCharArrayOfString( GetSpeciesNodeName( species ) ), 
                (i+1),
                elements[i].concentration );        
    }
    fprintf( file, NEW_LINE ); 
 
    if( IS_FAILED( ( ret = _CleanCriticalConInfo( info ) ) ) ) {
        return FAILING;
    }
       
    return SUCCESS;    
}

static RET_VAL _PrintNoLevelFound( BACK_END_PROCESSOR *backend, FILE *file, SPECIES *species ) {
    RET_VAL ret = SUCCESS;
    
    fprintf( file, "%s%s.1 = %s" NEW_LINE NEW_LINE, 
            REB2SAC_CRITICAL_CONCENTRATION_LEVEL_KEY_PREFIX, 
            GetCharArrayOfString( GetSpeciesNodeName( species ) ), 
            REB2SAC_NARY_LEVEL_SPECIES_UNKNOWN_VALUE );        
    return ret;
}


static RET_VAL _CleanCriticalConInfo( CRITICAL_CONCENTRATION_INFO *info ) {
    RET_VAL ret = SUCCESS;
    int i = 0;
    int len = 0;
    CRITICAL_CONCENTRATION_ELEMENT *elements = NULL;
    
    START_FUNCTION("_CleanCriticalConInfo");
    
    info->len = 0;
    elements = info->elements;
    FREE( info->elements );
    
    END_FUNCTION("_CleanCriticalConInfo", SUCCESS );
    return ret;
}

static RET_VAL _PrintSpeciesProperties( BACK_END_PROCESSOR *backend, FILE *file, SPECIES *species ) {
    RET_VAL ret = SUCCESS;
    
    
    fprintf( file, "%s%s = %s" NEW_LINE,  
            REB2SAC_ABSOLUTE_INHIBITION_THRESHOLD_KEY_PREFIX, 
            GetCharArrayOfString( GetSpeciesNodeName( species ) ),
            REB2SAC_NARY_LEVEL_SPECIES_UNKNOWN_VALUE );                 
            
    fprintf( file, "%s%s = %s" NEW_LINE,  
            REB2SAC_CRITICAL_CONCENTRATION_LEVEL_SPECIFICATION_TYPE_KEY_PREFIX, 
            GetCharArrayOfString( GetSpeciesNodeName( species ) ),
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


