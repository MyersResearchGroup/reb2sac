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

static char * _GetDegradationStoichiometryAmplificationMethod3ID( ABSTRACTION_METHOD *method );
static RET_VAL _ApplyDegradationStoichiometryAmplificationMethod3( ABSTRACTION_METHOD *method, IR *ir );      
static BOOL _IsConditionSatisfied( ABSTRACTION_METHOD *method, IR *ir, REACTION *reaction );
static KINETIC_LAW *_CreateReactantReplacement( SPECIES *species, UINT32 amplifier, UINT32 stoichiometry ); 
static KINETIC_LAW *_CreateProductReplacement( SPECIES *species, UINT32 amplifier, UINT32 stoichiometry );
static UINT32 _FindAmplifier( ABSTRACTION_METHOD_MANAGER *manager );


ABSTRACTION_METHOD *DegradationStoichiometryAmplificationMethod3Constructor(  ABSTRACTION_METHOD_MANAGER *manager ) {
    static ABSTRACTION_METHOD method;
    UINT32 amplifier = 0;
    
    START_FUNCTION("DegradationStoichiometryAmplificationMethod3Constructor");

    if( method.manager == NULL ) {
        method.manager = manager;
        method.GetID = _GetDegradationStoichiometryAmplificationMethod3ID;
        method.Apply = _ApplyDegradationStoichiometryAmplificationMethod3;
    }
    
    amplifier = _FindAmplifier( manager );
    method._internal1 = (CADDR_T)amplifier;
    
    TRACE_0( "DegradationStoichiometryAmplificationMethod3Constructor invoked" );
    
    END_FUNCTION("DegradationStoichiometryAmplificationMethod3Constructor", SUCCESS );
    return &method;
}



static char * _GetDegradationStoichiometryAmplificationMethod3ID( ABSTRACTION_METHOD *method ) {
    START_FUNCTION("_GetDegradationStoichiometryAmplificationMethod3ID");
    
    END_FUNCTION("_GetDegradationStoichiometryAmplificationMethod3ID", SUCCESS );
    return "degradation-stoichiometry-amplifier3";
}



static RET_VAL _ApplyDegradationStoichiometryAmplificationMethod3( ABSTRACTION_METHOD *method, IR *ir ) {
    RET_VAL ret = SUCCESS;
    UINT32 amplifier = (UINT32)(method->_internal1);
    UINT32 stoichiometry = 0;
    SPECIES *species = NULL;
    KINETIC_LAW *kineticLaw = NULL;
    KINETIC_LAW *replacement = NULL;
    IR_EDGE *edge = NULL;
    LINKED_LIST *edges = NULL;
    REACTION *reaction = NULL;
    LINKED_LIST *reactionList = NULL;
    
    START_FUNCTION("_ApplyDegradationStoichiometryAmplificationMethod3");
     
    reactionList = ir->GetListOfReactionNodes( ir );        
    ResetCurrentElement( reactionList );    
    while( ( reaction = (REACTION*)GetNextFromLinkedList( reactionList ) ) != NULL ) {
        if( !_IsConditionSatisfied( method, ir, reaction ) ) {
            continue;
        }
        kineticLaw = GetKineticLawInReactionNode( reaction );
        edges = GetReactantEdges( reaction );
        ResetCurrentElement( edges );
        while( ( edge = GetNextEdge( edges ) ) != NULL ) {
            species = GetSpeciesInIREdge( edge );
            stoichiometry = GetStoichiometryInIREdge( edge );
            replacement = _CreateReactantReplacement( species, amplifier, stoichiometry );
            if( IS_FAILED( ( ret = ReplaceSpeciesWithKineticLawInKineticLaw( kineticLaw, species, replacement ) ) ) ) {
                END_FUNCTION("_ApplyDegradationStoichiometryAmplificationMethod3", ret );
                return ret;
            }        
#if 1            
            FreeKineticLaw( &replacement );
#endif            
#if 1            
            if( IS_FAILED( ( ret = SetStoichiometryInIREdge( edge, stoichiometry * amplifier ) ) ) ) {
                END_FUNCTION("_ApplyDegradationStoichiometryAmplificationMethod3", ret );
                return ret;
            }
#endif            
        }        
        edges = GetModifierEdges( reaction );
        ResetCurrentElement( edges );
        while( ( edge = GetNextEdge( edges ) ) != NULL ) {
            stoichiometry = GetStoichiometryInIREdge( edge );
            if( IS_FAILED( ( ret = SetStoichiometryInIREdge( edge, stoichiometry * amplifier ) ) ) ) {
                END_FUNCTION("_ApplyDegradationStoichiometryAmplificationMethod3", ret );
                return ret;
            }
        }        
    } 
            
    END_FUNCTION("_ApplyDegradationStoichiometryAmplificationMethod3", SUCCESS );
    return ret;
}      

static BOOL _IsConditionSatisfied( ABSTRACTION_METHOD *method, IR *ir, REACTION *reaction ) {
    LINKED_LIST *list = NULL;
    
    list = GetProductEdges( reaction );
    if( GetLinkedListSize( list ) > 0 ) {
        return FALSE;
    }
    list = GetReactantEdges( reaction );
    return ( GetLinkedListSize( list ) == 0 ? FALSE : TRUE );    
}



static UINT32 _FindAmplifier( ABSTRACTION_METHOD_MANAGER *manager ) {
    char *valueString = NULL;
    UINT32 amplifier = DEFAULT_REB2SAC_STOICHIOMETRY_AMPLIFIER;
    COMPILER_RECORD_T *record = NULL;
    REB2SAC_PROPERTIES *properties = NULL;
    
    record = manager->GetCompilerRecord( manager );
    properties = record->properties;
    
    if( ( valueString = properties->GetProperty( properties, REB2SAC_DEGRADATION_STOICHIOMETRY_AMPLIFIER_KEY ) ) != NULL ) {
        if( IS_FAILED( StrToUINT32( &amplifier, valueString ) ) ) {
            return DEFAULT_REB2SAC_DEGRADATION_STOICHIOMETRY_AMPLIFIER;         
        }
        return amplifier;
    }
    
    return DEFAULT_REB2SAC_DEGRADATION_STOICHIOMETRY_AMPLIFIER;
}

static KINETIC_LAW *_CreateReactantReplacement( SPECIES *species, UINT32 amplifier, UINT32 stoichiometry ) {
    KINETIC_LAW *replacement = NULL;
    
    replacement = CreateSpeciesKineticLaw( species );
#if 1    
    replacement = CreateOpKineticLaw( KINETIC_LAW_OP_MINUS, replacement, 
        CreateRealValueKineticLaw( (double)( stoichiometry * amplifier / 2 ) ) );
#endif        
    return replacement;              
}


static KINETIC_LAW *_CreateProductReplacement( SPECIES *species, UINT32 amplifier, UINT32 stoichiometry ) {
    KINETIC_LAW *replacement = NULL;
    
    replacement = CreateSpeciesKineticLaw( species );
#if 1    
    replacement = CreateOpKineticLaw( KINETIC_LAW_OP_MINUS, replacement, 
        CreateRealValueKineticLaw( (double)( stoichiometry * amplifier / 2 ) ) );
#endif        
    return replacement;              
}

 