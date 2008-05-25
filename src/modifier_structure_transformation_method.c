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



static char * _GetModifierStructureTransformationMethodID( ABSTRACTION_METHOD *method );
static RET_VAL _ApplyModifierStructureTransformationMethod( ABSTRACTION_METHOD *method, IR *ir );      


ABSTRACTION_METHOD *ModifierStructureTransformationMethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager ) {
    static ABSTRACTION_METHOD method;
    
    START_FUNCTION("ModifierStructureTransformationMethodConstructor");

    if( method.manager == NULL ) {
        method.manager = manager;
        method.GetID = _GetModifierStructureTransformationMethodID;
        method.Apply = _ApplyModifierStructureTransformationMethod;
    }
    
    TRACE_0( "ModifierStructureTransformationMethodConstructor invoked" );
    
    END_FUNCTION("ModifierStructureTransformationMethodConstructor", SUCCESS );
    return &method;
}



static char * _GetModifierStructureTransformationMethodID( ABSTRACTION_METHOD *method ) {
    START_FUNCTION("_GetModifierStructureTransformationMethodID");
    
    END_FUNCTION("_GetModifierStructureTransformationMethodID", SUCCESS );
    return "modifier-structure-transformer";
}



static RET_VAL _ApplyModifierStructureTransformationMethod( ABSTRACTION_METHOD *method, IR *ir ) {
    RET_VAL ret = SUCCESS;
    int stoichiometry = 0;
    SPECIES *reactant = NULL;
    SPECIES *product = NULL;
    REACTION *reaction = NULL;
    IR_EDGE *reactantEdge = NULL;
    IR_EDGE *productEdge = NULL;
    LINKED_LIST *reactionList = NULL;
    LINKED_LIST *reactantEdges = NULL;
    LINKED_LIST *productEdges = NULL;
    
    START_FUNCTION("_ApplyModifierStructureTransformationMethod");

    reactionList = ir->GetListOfReactionNodes( ir );
    
    ResetCurrentElement( reactionList );    
    while( ( reaction = (REACTION*)GetNextFromLinkedList( reactionList ) ) != NULL ) {
        reactantEdges = GetReactantEdges( (IR_NODE*)reaction );
        productEdges = GetProductEdges( (IR_NODE*)reaction );
        
        if( ( GetLinkedListSize( reactantEdges ) == 0 ) || ( GetLinkedListSize( productEdges ) == 0 ) ) {
            continue;
        }
        
        ResetCurrentElement( reactantEdges );
        while( ( reactantEdge = GetNextEdge( reactantEdges ) ) != NULL ) {
            reactant = GetSpeciesInIREdge( reactantEdge );
            stoichiometry = GetStoichiometryInIREdge( reactantEdge );
            ResetCurrentElement( productEdges );
            while( ( productEdge = GetNextEdge( productEdges ) ) != NULL ) {
                product = GetSpeciesInIREdge( productEdge );
                if( ( reactant == product ) && (  stoichiometry == GetStoichiometryInIREdge( productEdge ) ) ) {
                    TRACE_2("setting species %s in reaction %s as modifier", 
                        GetCharArrayOfString( GetSpeciesNodeName( reactant ) ), GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
                    if( IS_FAILED( ( ret = ir->RemoveReactantEdge( ir, &reactantEdge ) ) ) ) {
                        END_FUNCTION("_ApplyModifierStructureTransformationMethod", ret );
                        return ret;
                    }
                    if( IS_FAILED( ( ret = ir->RemoveProductEdge( ir, &productEdge ) ) ) ) {
                        END_FUNCTION("_ApplyModifierStructureTransformationMethod", ret );
                        return ret;
                    }
                    if( IS_FAILED( ( ret = ir->AddModifierEdge( ir, reaction, reactant, stoichiometry ) ) ) ) {
                        END_FUNCTION("_ApplyModifierStructureTransformationMethod", ret );
                        return ret;
                    }
                    break;
                }
            }
        }
    }
            
    END_FUNCTION("_ApplyModifierStructureTransformationMethod", SUCCESS );
    return ret;
}      

