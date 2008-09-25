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

static char * _GetReversibleToIrreversibleMethodID( ABSTRACTION_METHOD *method );
static RET_VAL _ApplyReversibleToIrreversibleMethod( ABSTRACTION_METHOD *method, IR *ir );      
static BOOL _IsConditionSatisfied( ABSTRACTION_METHOD *method, IR *ir, REACTION *reaction ); 
static RET_VAL _DoTransformation( ABSTRACTION_METHOD *method, IR *ir, REACTION *reaction );



ABSTRACTION_METHOD *ReversibleToIrreversibleMethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager ) {
    static ABSTRACTION_METHOD method;
    
    START_FUNCTION("ReversibleToIrreversibleMethodConstructor");

    if( method.manager == NULL ) {
        method.manager = manager;
        method.GetID = _GetReversibleToIrreversibleMethodID;
        method.Apply = _ApplyReversibleToIrreversibleMethod;
    }
    
    TRACE_0( "ReversibleToIrreversibleMethodConstructor invoked" );
    
    END_FUNCTION("ReversibleToIrreversibleMethodConstructor", SUCCESS );
    return &method;
}



static char * _GetReversibleToIrreversibleMethodID( ABSTRACTION_METHOD *method ) {
    START_FUNCTION("_GetReversibleToIrreversibleMethodID");
    
    END_FUNCTION("_GetReversibleToIrreversibleMethodID", SUCCESS );
    return "reversible-to-irreversible-transformer";
}



static RET_VAL _ApplyReversibleToIrreversibleMethod( ABSTRACTION_METHOD *method, IR *ir ) {
    RET_VAL ret = SUCCESS;
    REACTION *reaction = NULL;
    LINKED_LIST *reactionList = NULL;
    
    START_FUNCTION("_ApplyReversibleToIrreversibleMethod");

    reactionList = ir->GetListOfReactionNodes( ir );    
    ResetCurrentElement( reactionList );    
    while( ( reaction = (REACTION*)GetNextFromLinkedList( reactionList ) ) != NULL ) {
        if( _IsConditionSatisfied( method, ir, reaction ) ) {
            if( IS_FAILED( ( ret = _DoTransformation( method, ir, reaction ) ) ) ) {
                END_FUNCTION("_ApplyReversibleToIrreversibleMethod", ret );
                return ret;
            }
        }            
    }
    
    END_FUNCTION("_ApplyReversibleToIrreversibleMethod", SUCCESS );
    return ret;
}      


static BOOL _IsConditionSatisfied( ABSTRACTION_METHOD *method, IR *ir, REACTION *reaction ) {
    KINETIC_LAW *kineticLaw = NULL;
    
    START_FUNCTION("_IsConditionSatisfied");

    if( !IsReactionReversibleInReactionNode( reaction ) ) {
        END_FUNCTION("_IsConditionSatisfied", SUCCESS );
        return FALSE;
    }
    
    kineticLaw = GetKineticLawInReactionNode( reaction );
    if( !IsOpKineticLaw( kineticLaw ) ) {
        END_FUNCTION("_IsConditionSatisfied", SUCCESS );
        return FALSE;
    }
    
    if( KINETIC_LAW_OP_MINUS != GetOpTypeFromKineticLaw( kineticLaw ) ) {
        END_FUNCTION("_IsConditionSatisfied", SUCCESS );
        return FALSE;
    }
        
    END_FUNCTION("_IsConditionSatisfied", SUCCESS );
    return TRUE;
}

static RET_VAL _DoTransformation( ABSTRACTION_METHOD *method, IR *ir, REACTION *reaction ) {
    RET_VAL ret = SUCCESS;
    char buf[1024];
    double stoichiometry = 0.0;
    STRING *newName = NULL;
    STRING *originalName = NULL;
    REACTION *backwardReaction = NULL;
    KINETIC_LAW *kineticLaw = NULL;
    KINETIC_LAW *forwardKineticLaw = NULL;
    KINETIC_LAW *backwardKineticLaw = NULL;
    IR_EDGE *edge = NULL;
    SPECIES *species = NULL;
    LINKED_LIST *list = NULL;
    
    START_FUNCTION("_DoTransformation");

    kineticLaw = GetKineticLawInReactionNode( reaction );
    if( ( forwardKineticLaw = CloneKineticLaw( GetOpLeftFromKineticLaw( kineticLaw ) ) ) == NULL ) {
        return ErrorReport( FAILING, "_DoTransformation", "failed to clone the forward kinetic law of %s", GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
    }
    if( ( backwardKineticLaw = CloneKineticLaw( GetOpRightFromKineticLaw( kineticLaw ) ) ) == NULL ) {
        return ErrorReport( FAILING, "_DoTransformation", "failed to clone the backward kinetic law of %s", GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
    }
    originalName = GetReactionNodeName( reaction );
    sprintf( buf, "%s_b", GetCharArrayOfString( originalName ) );
    if( ( backwardReaction = ir->CreateReaction( ir, buf ) ) == NULL ) {
        return ErrorReport( FAILING, "_DoTransformation", "could not create a backward reaction for %s", GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
    }
    if( IS_FAILED( ( ret = SetKineticLawInReactionNode( backwardReaction, backwardKineticLaw ) ) ) ) {
        END_FUNCTION("_DoTransformation", ret );
        return ret;
    }
    if( IS_FAILED( ( ret = SetReactionReversibleInReactionNode( backwardReaction, FALSE ) ) ) ) {
        END_FUNCTION("_DoTransformation", ret );
        return ret;
    }
    
    list = GetProductEdges( (IR_NODE*)reaction );
    ResetCurrentElement( list );
    while( ( edge = GetNextEdge( list ) ) != NULL ) {
        species = GetSpeciesInIREdge( edge );
        stoichiometry = GetStoichiometryInIREdge( edge ); 
        if( IS_FAILED( ( ret = ir->AddReactantEdge(  ir, backwardReaction, species, stoichiometry ) ) ) ) {
            END_FUNCTION("_DoTransformation", ret );
            return ret;
        } 
    } 
    
    list = GetReactantEdges( (IR_NODE*)reaction );
    ResetCurrentElement( list );
    while( ( edge = GetNextEdge( list ) ) != NULL ) {
        species = GetSpeciesInIREdge( edge );
        stoichiometry = GetStoichiometryInIREdge( edge ); 
        if( IS_FAILED( ( ret = ir->AddProductEdge( ir, backwardReaction, species, stoichiometry ) ) ) ) {
            END_FUNCTION("_DoTransformation", ret );
            return ret;
        } 
    } 
       
    list = GetModifierEdges( (IR_NODE*)reaction );
    ResetCurrentElement( list );
    while( ( edge = GetNextEdge( list ) ) != NULL ) {
        species = GetSpeciesInIREdge( edge );
        stoichiometry = GetStoichiometryInIREdge( edge ); 
        if( IS_FAILED( ( ret = ir->AddModifierEdge( ir, backwardReaction, species, stoichiometry ) ) ) ) {
            END_FUNCTION("_DoTransformation", ret );
            return ret;
        } 
    } 

    
    sprintf( buf, "%s_f", GetCharArrayOfString( originalName ) );    
    if( ( newName = CreateString( buf ) ) == NULL ) {
        return ErrorReport( FAILING, "_DoTransformation", "could not create a forward reaction name for %s", GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
    }
    if( IS_FAILED( ( ret = SetReactionNodeName( reaction, newName ) ) ) ) {
        END_FUNCTION("_DoTransformation", ret );
        return ret;
    }
    
    if( IS_FAILED( ( ret = SetKineticLawInReactionNode( reaction, forwardKineticLaw ) ) ) ) {
        END_FUNCTION("_DoTransformation", ret );
        return ret;
    }
    FreeKineticLaw( &kineticLaw );
    
    if( IS_FAILED( ( ret = SetReactionReversibleInReactionNode( reaction, FALSE ) ) ) ) {
        END_FUNCTION("_DoTransformation", ret );
        return ret;
    }        
        
    END_FUNCTION("_DoTransformation", SUCCESS );
    return ret;
}


