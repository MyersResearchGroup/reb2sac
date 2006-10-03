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
#if defined(DEBUG)
#undef DEBUG
#endif

#include "abstraction_method_manager.h"
#include "IR.h"
#include "strconv.h"

static char * _GetStopFlagGenerationMethodID( ABSTRACTION_METHOD *method );
static RET_VAL _ApplyStopFlagGenerationMethod( ABSTRACTION_METHOD *method, IR *ir );      
static SPECIES *_CreateStopSpecies( ABSTRACTION_METHOD *method, IR *ir );
static RET_VAL _FindStopSpeciesName( IR *ir, char *buf, int size );
static RET_VAL _AddStopSpeciesInReaction( ABSTRACTION_METHOD *method, IR *ir, REACTION *reaction, SPECIES *stopSpecies );
static RET_VAL _SetStopReaction( ABSTRACTION_METHOD *method, IR *ir,  SPECIES *stopSpecies );

ABSTRACTION_METHOD *StopFlagGenerationMethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager ) {
    static ABSTRACTION_METHOD method;
    char *valueString = NULL;
    COMPILER_RECORD_T *record = NULL;
    REB2SAC_PROPERTIES *properties = NULL;
    
    START_FUNCTION("StopFlagGenerationMethodConstructor");

    if( method.manager == NULL ) {
        method.manager = manager;
        method.GetID = _GetStopFlagGenerationMethodID;
        method.Apply = _ApplyStopFlagGenerationMethod;
    }
    
    record = manager->GetCompilerRecord( manager );
    properties = record->properties;
    
    if( ( valueString = properties->GetProperty( properties, REB2SAC_ANALYSIS_STOP_ENABLED_KEY ) ) == NULL ) {
        valueString = DEFAULT_REB2SAC_ANALYSIS_STOP_ENABLED_VALUE;
    }
    
    if( strcmp( valueString, REB2SAC_ANALYSIS_STOP_ENABLED_VALUE_TRUE ) == 0 ) {
        method._internal1 = (CADDR_T)TRUE;
    }
    else {
        method._internal1 = (CADDR_T)FALSE;
    }
    
    method._internal2 = (CADDR_T)FALSE;
    
    TRACE_0( "StopFlagGenerationMethodConstructor invoked" );
    
    
    END_FUNCTION("StopFlagGenerationMethodConstructor", SUCCESS );
    return &method;
}



static char * _GetStopFlagGenerationMethodID( ABSTRACTION_METHOD *method ) {
    START_FUNCTION("_GetStopFlagGenerationMethodID");
    
    END_FUNCTION("_GetStopFlagGenerationMethodID", SUCCESS );
    return "stop-flag-generator";
}



static RET_VAL _ApplyStopFlagGenerationMethod( ABSTRACTION_METHOD *method, IR *ir ) {
    RET_VAL ret = SUCCESS;
    SPECIES *stopSpecies = NULL;
    REACTION *reaction = NULL;
    LINKED_LIST *reactionList = NULL;
    
    START_FUNCTION("_ApplyStopFlagGenerationMethod");

    
    
    if( !((BOOL)(method->_internal1)) ) {
        END_FUNCTION("_ApplyStopFlagGenerationMethod", SUCCESS );
        return ret;
    }
    
    if( (BOOL)(method->_internal2) ) {
        END_FUNCTION("_ApplyStopFlagGenerationMethod", SUCCESS );
        return ret;
    }
    
    method->_internal2 = (CADDR_T)TRUE;
   
    if( ( stopSpecies = _CreateStopSpecies( method, ir ) ) == NULL ) {
        END_FUNCTION("_ApplyStopFlagGenerationMethod", FAILING );
        return FAILING;
    }   
    
    reactionList = ir->GetListOfReactionNodes( ir );    
    ResetCurrentElement( reactionList );    
    while( ( reaction = (REACTION*)GetNextFromLinkedList( reactionList ) ) != NULL ) {
        if( IS_FAILED( ( ret = _AddStopSpeciesInReaction( method, ir, reaction, stopSpecies ) ) ) ) {
            END_FUNCTION("_ApplyStopFlagGenerationMethod", ret );
            return ret;
        }
    }
    
    if( IS_FAILED( ( ret = _SetStopReaction( method, ir, stopSpecies ) ) ) ) {
        END_FUNCTION("_ApplyStopFlagGenerationMethod", ret );
        return ret;
    }
    
    END_FUNCTION("_ApplyStopFlagGenerationMethod", SUCCESS );
    return ret;
}      


static SPECIES *_CreateStopSpecies( ABSTRACTION_METHOD *method, IR *ir ) {
    RET_VAL ret = SUCCESS;
    char buf[512];
    SPECIES *stopSpecies = NULL; 
    
    START_FUNCTION("_CreateStopSpecies");
    
    strcpy( buf, REB2SAC_STOP_SPECIES_NAME_PREFIX );     
    if( IS_FAILED( ( ret = _FindStopSpeciesName( ir, buf, sizeof( buf ) ) ) ) ) {
        END_FUNCTION("_CreateStopSpecies", ret );
        return NULL;
    }
    
    TRACE_1("the name of the stop species is", buf );
    
    if( ( stopSpecies = ir->CreateSpecies( ir, buf ) ) == NULL ) {
        TRACE_1( "could not create a species %s", buf );
        END_FUNCTION("_CreateStopSpecies", ret );
        return NULL;
    }
    
    if( IS_FAILED( ( ret = SetInitialConcentrationInSpeciesNode( stopSpecies, 0.0 ) ) ) ) {
        END_FUNCTION("_CreateStopSpecies", ret );
        return NULL;
    }

    END_FUNCTION("_CreateStopSpecies", SUCCESS );
    return stopSpecies;
}

static RET_VAL _FindStopSpeciesName( IR *ir, char *buf, int size ) {
    RET_VAL ret = SUCCESS;
    int i = 0;
    char *name = NULL;
    SPECIES *species = NULL;
    LINKED_LIST *list = NULL;
    
    START_FUNCTION("_FindStopSpeciesName");
    
    if( strlen( buf ) >= size ) {
        return ErrorReport( FAILING, "_FindStopSpeciesName", "failed to create a name for the stop species" );
    }
    
    list = ir->GetListOfSpeciesNodes( ir );
    ResetCurrentElement( list );
    while( ( species = (SPECIES*)GetNextFromLinkedList( list ) ) != NULL ) {
        name = GetCharArrayOfString( GetSpeciesNodeName( species ) );
        if( strcmp( buf, name ) == 0 ) {
            strcat( buf, "_" );
            ret = _FindStopSpeciesName( ir, buf, size );
            END_FUNCTION("_ApplyStopFlagGenerationMethod", ret );
            return ret;
        }
    }
    
    END_FUNCTION("_ApplyStopFlagGenerationMethod", SUCCESS );
    return ret;
}



static RET_VAL _AddStopSpeciesInReaction( ABSTRACTION_METHOD *method, IR *ir, REACTION *reaction, SPECIES *stopSpecies ) {
    RET_VAL ret = SUCCESS;
    KINETIC_LAW *kineticLaw = NULL;
    KINETIC_LAW *stopKineticLaw = NULL;
    
    START_FUNCTION("_AddStopSpeciesInReaction");
        
    if( ( stopKineticLaw = CreateOpKineticLaw( KINETIC_LAW_OP_MINUS, CreateRealValueKineticLaw( 1.0 ), CreateSpeciesKineticLaw( stopSpecies ) ) ) == NULL ) {
        return ErrorReport( FAILING, "_AddStopSpeciesInReaction", "could not create stop kinetic law for %s", GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
    }
    kineticLaw = GetKineticLawInReactionNode( reaction );
    if( ( kineticLaw = CreateOpKineticLaw( KINETIC_LAW_OP_TIMES, stopKineticLaw, kineticLaw ) ) == NULL ) {
        return ErrorReport( FAILING, "_AddStopSpeciesInReaction", "could not put stop kinetic law in %s", GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
    }
    
    if( IS_FAILED( ( ret = ir->AddModifierEdge( ir, reaction, stopSpecies, 1 ) ) ) ) {
        END_FUNCTION("_AddStopSpeciesInReaction", ret );
        return ret;
    }
    
    
    if( IS_FAILED( ( ret = SetKineticLawInReactionNode( reaction, kineticLaw ) ) ) ) {
        END_FUNCTION("_AddStopSpeciesInReaction", ret );
        return ret;
    }
    
    END_FUNCTION("_AddStopSpeciesInReaction", SUCCESS );
    return ret;
}

static RET_VAL _SetStopReaction( ABSTRACTION_METHOD *method, IR *ir,  SPECIES *stopSpecies ) {
    RET_VAL ret = SUCCESS;
    KINETIC_LAW *kineticLaw = NULL;
    REACTION *reaction = NULL;
    char *valueString = NULL;
    double rate = 0.0;
    ABSTRACTION_METHOD_MANAGER *manager = NULL;
    COMPILER_RECORD_T *record = NULL;
    REB2SAC_PROPERTIES *properties = NULL;
    
    START_FUNCTION("_SetStopReaction");
    
    manager = method->manager;
    record = manager->GetCompilerRecord( manager );
    properties = record->properties;
    
    if( ( valueString = properties->GetProperty( properties, REB2SAC_ANALYSIS_STOP_RATE_KEY ) ) != NULL ) {
        if( IS_FAILED( ( ret = StrToFloat( &rate, valueString ) ) ) ) {
            END_FUNCTION("_SetStopReaction", ret );
            return ret;
        }
    }
    else {
        rate = DEFAULT_REB2SAC_ANALYSIS_STOP_RATE;
    }
    
    if( ( reaction = ir->CreateReaction( ir, REB2SAC_STOP_REACTION_NAME ) ) == NULL ) {
        return ErrorReport( FAILING, "_SetStopReaction", "could not create a reaction %s", REB2SAC_STOP_REACTION_NAME );
    }
    if( IS_FAILED( ( ret = SetReactionReversibleInReactionNode( reaction, FALSE ) ) ) ) {
        END_FUNCTION("_AddStopSpeciesInReaction", ret );
        return ret;
    }
    if( ( kineticLaw = CreateRealValueKineticLaw( rate ) ) == NULL ) {
        return ErrorReport( FAILING, "_SetStopReaction", "could not create a kinetic law %g for reaction %s", rate, REB2SAC_STOP_REACTION_NAME );
    }
    if( IS_FAILED( ( ret = SetKineticLawInReactionNode( reaction, kineticLaw ) ) ) ) {
        END_FUNCTION("_AddStopSpeciesInReaction", ret );
        return ret;
    }    
    if( IS_FAILED( ( ret = ir->AddProductEdge( ir, reaction, stopSpecies, 1 ) ) ) ) {
        END_FUNCTION("_AddStopSpeciesInReaction", ret );
        return ret;
    }        
    
    END_FUNCTION("_SetStopReaction", SUCCESS );
    return ret;
}



