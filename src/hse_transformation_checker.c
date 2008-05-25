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

#include "hse_transformation_checker.h"

static BOOL _IsSpeciesTransformableToLogicalStatement( BACK_END_PROCESSOR *backend, SPECIES *species );
static BOOL _IsReactionTransformableToLogicalStatement( BACK_END_PROCESSOR *backend, REACTION *reaction );


BOOL IsTransformableToHse( BACK_END_PROCESSOR *backend, IR *ir ) {
    SPECIES *species = NULL;
    REACTION *reaction = NULL;
    LINKED_LIST *list = NULL;
    
    START_FUNCTION("IsTransformableToHse");
    
    list = ir->GetListOfSpeciesNodes( ir );    
    ResetCurrentElement( list );    
    while( ( species = (SPECIES*)GetNextFromLinkedList( list ) ) != NULL ) {
        if( !_IsSpeciesTransformableToLogicalStatement( backend, species ) ) {
            TRACE_1("species %s cannot be used in logical statement", GetCharArrayOfString( GetSpeciesNodeName( species ) ) );
            printf("species %s cannot be used in logical statement" NEW_LINE, GetCharArrayOfString( GetSpeciesNodeName( species ) ) );
            END_FUNCTION("IsTransformableToHse", SUCCESS );
            return FALSE;
        }
    }
    
    list = ir->GetListOfReactionNodes( ir );    
    ResetCurrentElement( list );    
    while( ( reaction = (REACTION*)GetNextFromLinkedList( list ) ) != NULL ) {
        if( !_IsReactionTransformableToLogicalStatement( backend, reaction ) ) {
            TRACE_1("reaction %s cannot be transformed to logical statement", GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
            printf("reaction %s cannot be transformed to logical statement" NEW_LINE, GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
            END_FUNCTION("IsTransformableToHse", SUCCESS );
            return FALSE;
        }
    }
        
    TRACE_0("This IR is transformable to HSE");
    END_FUNCTION("IsTransformableToHse", SUCCESS );
    return TRUE;
}

static BOOL _IsSpeciesTransformableToLogicalStatement( BACK_END_PROCESSOR *backend, SPECIES *species ) {
    int num = 0;
    LINKED_LIST *reactionsAsReactant = NULL;
    LINKED_LIST *reactionsAsProduct = NULL;
    
    START_FUNCTION("_IsSpeciesTransformableToLogicalStatement");
        
    reactionsAsReactant = GetReactantEdges( (IR_NODE*)species );
    reactionsAsProduct = GetProductEdges( (IR_NODE*)species );
    
    num = GetLinkedListSize( reactionsAsReactant ) + GetLinkedListSize( reactionsAsProduct );
    if( num == 0 ) {
        END_FUNCTION("_IsSpeciesTransformableToLogicalStatement", SUCCESS );
        return FALSE;
    } 
    
    END_FUNCTION("_IsSpeciesTransformableToLogicalStatement", SUCCESS );
    return TRUE;
}


static BOOL _IsReactionTransformableToLogicalStatement( BACK_END_PROCESSOR *backend, REACTION *reaction ) {
    int num = 0;
    LINKED_LIST *reactants = NULL;
    LINKED_LIST *products = NULL;
    
    START_FUNCTION("_IsReactionTransformableToLogicalStatement");
    
    if( IsReactionReversibleInReactionNode( reaction ) ) {
        END_FUNCTION("_IsReactionTransformableToLogicalStatement", SUCCESS );
        return FALSE;
    }
    
    reactants = GetReactantEdges( (IR_NODE*)reaction );
    products = GetProductEdges( (IR_NODE*)reaction );
    
    num = GetLinkedListSize( reactants ) + GetLinkedListSize( products );
    if( num != 1 ) {
        END_FUNCTION("_IsReactionTransformableToLogicalStatement", SUCCESS );
        return FALSE;
    } 
    
    END_FUNCTION("_IsReactionTransformableToLogicalStatement", SUCCESS );
    return TRUE;
}
