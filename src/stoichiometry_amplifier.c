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

static char * _GetStoichiometryAmplificationMethodID( ABSTRACTION_METHOD *method );
static RET_VAL _ApplyStoichiometryAmplificationMethod( ABSTRACTION_METHOD *method, IR *ir );      
static KINETIC_LAW *_CreateReactantReplacement( SPECIES *species, UINT32 amplifier, double stoichiometry ); 
static KINETIC_LAW *_CreateProductReplacement( SPECIES *species, UINT32 amplifier, double stoichiometry ); 
static KINETIC_LAW *_CreateNormalization( UINT32 amplifier ); 
static UINT32 _FindAmplifier( ABSTRACTION_METHOD_MANAGER *manager );


ABSTRACTION_METHOD *StoichiometryAmplificationMethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager ) {
    static ABSTRACTION_METHOD method;
    UINT32 amplifier = 0;
    
    START_FUNCTION("StoichiometryAmplificationMethodConstructor");

    if( method.manager == NULL ) {
        method.manager = manager;
        method.GetID = _GetStoichiometryAmplificationMethodID;
        method.Apply = _ApplyStoichiometryAmplificationMethod;
    }
    
    amplifier = _FindAmplifier( manager );
    method._internal1 = (CADDR_T)amplifier;
    
    TRACE_0( "StoichiometryAmplificationMethodConstructor invoked" );
    
    END_FUNCTION("StoichiometryAmplificationMethodConstructor", SUCCESS );
    return &method;
}



static char * _GetStoichiometryAmplificationMethodID( ABSTRACTION_METHOD *method ) {
    START_FUNCTION("_GetStoichiometryAmplificationMethodID");
    
    END_FUNCTION("_GetStoichiometryAmplificationMethodID", SUCCESS );
    return "stoichiometry-amplifier";
}



static RET_VAL _ApplyStoichiometryAmplificationMethod( ABSTRACTION_METHOD *method, IR *ir ) {
    RET_VAL ret = SUCCESS;
    UINT32 amplifier = (UINT32)(method->_internal1);
    double stoichiometry = 0;
    SPECIES *species = NULL;
    KINETIC_LAW *normalizationTerm = NULL;
    KINETIC_LAW *kineticLaw = NULL;
    KINETIC_LAW *replacement = NULL;
    IR_EDGE *edge = NULL;
    LINKED_LIST *edges = NULL;
    REACTION *reaction = NULL;
    LINKED_LIST *reactionList = NULL;
    
    START_FUNCTION("_ApplyStoichiometryAmplificationMethod");
     
    reactionList = ir->GetListOfReactionNodes( ir );        
    ResetCurrentElement( reactionList );    
    while( ( reaction = (REACTION*)GetNextFromLinkedList( reactionList ) ) != NULL ) {
        kineticLaw = GetKineticLawInReactionNode( reaction );
        edges = GetReactantEdges( (IR_NODE*)reaction );
        ResetCurrentElement( edges );
        while( ( edge = GetNextEdge( edges ) ) != NULL ) {
            species = GetSpeciesInIREdge( edge );
            stoichiometry = GetStoichiometryInIREdge( edge );
            replacement = _CreateReactantReplacement( species, amplifier, stoichiometry );
            if( IS_FAILED( ( ret = ReplaceSpeciesWithKineticLawInKineticLaw( kineticLaw, species, replacement ) ) ) ) {
                END_FUNCTION("_ApplyStoichiometryAmplificationMethod", ret );
                return ret;
            }        
            FreeKineticLaw( &replacement );
            if( IS_FAILED( ( ret = SetStoichiometryInIREdge( edge, stoichiometry * amplifier ) ) ) ) {
                END_FUNCTION("_ApplyStoichiometryAmplificationMethod", ret );
                return ret;
            }
        }        
        edges = GetProductEdges( (IR_NODE*)reaction );
        ResetCurrentElement( edges );
        while( ( edge = GetNextEdge( edges ) ) != NULL ) {
            species = GetSpeciesInIREdge( edge );
            stoichiometry = GetStoichiometryInIREdge( edge );
            replacement = _CreateProductReplacement( species, amplifier, stoichiometry );
            if( IS_FAILED( ( ret = ReplaceSpeciesWithKineticLawInKineticLaw( kineticLaw, species, replacement ) ) ) ) {
                END_FUNCTION("_ApplyStoichiometryAmplificationMethod", ret );
                return ret;
            }        
            FreeKineticLaw( &replacement );
            if( IS_FAILED( ( ret = SetStoichiometryInIREdge( edge, stoichiometry * amplifier ) ) ) ) {
                END_FUNCTION("_ApplyStoichiometryAmplificationMethod", ret );
                return ret;
            }
        }        
        edges = GetModifierEdges( (IR_NODE*)reaction );
        ResetCurrentElement( edges );
        while( ( edge = GetNextEdge( edges ) ) != NULL ) {
            stoichiometry = GetStoichiometryInIREdge( edge );
            if( IS_FAILED( ( ret = SetStoichiometryInIREdge( edge, stoichiometry * amplifier ) ) ) ) {
                END_FUNCTION("_ApplyStoichiometryAmplificationMethod", ret );
                return ret;
            }
        }        
        
        if( ( normalizationTerm = _CreateNormalization( amplifier ) ) == NULL ) {
            return ErrorReport( FAILING, "_ApplyStoichiometryAmplificationMethod", "could not create normailization term" );
        }
        if( ( kineticLaw = CreateOpKineticLaw( KINETIC_LAW_OP_TIMES, normalizationTerm, kineticLaw ) ) == NULL ) {
            return ErrorReport( FAILING, "_ApplyStoichiometryAmplificationMethod", "could not create a new kinetic law with normailization term" );
        }   
        SetKineticLawInReactionNode( reaction, kineticLaw ); 
    
    } 
            
    END_FUNCTION("_ApplyStoichiometryAmplificationMethod", SUCCESS );
    return ret;
}      


static UINT32 _FindAmplifier( ABSTRACTION_METHOD_MANAGER *manager ) {
    char *valueString = NULL;
    UINT32 amplifier = DEFAULT_REB2SAC_STOICHIOMETRY_AMPLIFIER;
    COMPILER_RECORD_T *record = NULL;
    REB2SAC_PROPERTIES *properties = NULL;
    
    record = manager->GetCompilerRecord( manager );
    properties = record->properties;
    
    if( ( valueString = properties->GetProperty( properties, REB2SAC_STOICHIOMETRY_AMPLIFIER_KEY ) ) != NULL ) {
        if( IS_FAILED( StrToUINT32( &amplifier, valueString ) ) ) {
            return DEFAULT_REB2SAC_STOICHIOMETRY_AMPLIFIER;         
        }
        return amplifier;
    }
    
    return DEFAULT_REB2SAC_STOICHIOMETRY_AMPLIFIER;
}

static KINETIC_LAW *_CreateReactantReplacement( SPECIES *species, UINT32 amplifier, double stoichiometry ) {
    KINETIC_LAW *replacement = NULL;
    
    replacement = CreateSpeciesKineticLaw( species );
#if 1    
    replacement = CreateOpKineticLaw( KINETIC_LAW_OP_MINUS, replacement, 
        CreateRealValueKineticLaw( ( stoichiometry * amplifier / 2 ) ) );
#endif        
    return replacement;              
}

static KINETIC_LAW *_CreateProductReplacement( SPECIES *species, UINT32 amplifier, double stoichiometry ) {
    KINETIC_LAW *replacement = NULL;
    
    replacement = CreateSpeciesKineticLaw( species );
#if 1    
    replacement = CreateOpKineticLaw( KINETIC_LAW_OP_PLUS, replacement, 
        CreateRealValueKineticLaw( ( stoichiometry * amplifier / 2 ) ) );
#endif        
    return replacement;                      
}


static KINETIC_LAW *_CreateNormalization( UINT32 amplifier ) {
    KINETIC_LAW *normalization = NULL;
    
    normalization = CreateRealValueKineticLaw( 1.0 / (amplifier) );
    return normalization;                      
}
