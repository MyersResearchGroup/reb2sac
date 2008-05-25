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

#include "ctmc_transformation_checker.h"


static BOOL _IsSpeciesTransformable( SPECIES *species );
static BOOL _IsReactionTransformable( REACTION *reaction );


BOOL IsTransformableToCTMC( IR *ir ) {
    SPECIES *species = NULL;
    REACTION *reaction = NULL;
    LINKED_LIST *list = NULL;
    
    START_FUNCTION("IsTransformableToCTMC");
    
    list = ir->GetListOfSpeciesNodes( ir );    
    ResetCurrentElement( list );    
    while( ( species = (SPECIES*)GetNextFromLinkedList( list ) ) != NULL ) {
        if( !_IsSpeciesTransformable( species ) ) {
            TRACE_1("species %s cannot be used in CTMC", GetCharArrayOfString( GetSpeciesNodeName( species ) ) );
            fprintf( stderr, "species %s cannot be used in CTMC" NEW_LINE, 
                     GetCharArrayOfString( GetSpeciesNodeName( species ) ) );
            END_FUNCTION("IsTransformableToCTMC", SUCCESS );
            return FALSE;
        }
    }
    
    list = ir->GetListOfReactionNodes( ir );    
    ResetCurrentElement( list );    
    while( ( reaction = (REACTION*)GetNextFromLinkedList( list ) ) != NULL ) {
        if( !_IsReactionTransformable( reaction ) ) {
            TRACE_1("reaction %s cannot be transformed to CTMC", GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
            fprintf( stderr, "reaction %s cannot be transformed to CTMC" NEW_LINE, 
                     GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
            END_FUNCTION("IsTransformableToCTMC", SUCCESS );
            return FALSE;
        }
    }
        
    TRACE_0("This IR is transformable to CTMC");
    END_FUNCTION("IsTransformableToCTMC", SUCCESS );
    return TRUE;
}

static BOOL _IsSpeciesTransformable( SPECIES *species ) {
    int num = 0;
    LINKED_LIST *reactionsAsReactant = NULL;
    LINKED_LIST *reactionsAsProduct = NULL;
    
    START_FUNCTION("_IsSpeciesTransformable");
        
    reactionsAsReactant = GetReactantEdges( (IR_NODE*)species );
    reactionsAsProduct = GetProductEdges( (IR_NODE*)species );
    
    num = GetLinkedListSize( reactionsAsReactant ) + GetLinkedListSize( reactionsAsProduct );
    if( num == 0 ) {
        END_FUNCTION("_IsSpeciesTransformable", SUCCESS );
        return FALSE;
    } 
    
    END_FUNCTION("_IsSpeciesTransformable", SUCCESS );
    return TRUE;
}


static BOOL _IsReactionTransformable( REACTION *reaction ) {
    int num = 0;
    LINKED_LIST *reactants = NULL;
    LINKED_LIST *products = NULL;
    
    START_FUNCTION("_IsReactionTransformable");
    
    if( IsReactionReversibleInReactionNode( reaction ) ) {
        END_FUNCTION("_IsReactionTransformable", SUCCESS );
        return FALSE;
    }
    
    reactants = GetReactantEdges( (IR_NODE*)reaction );
    products = GetProductEdges( (IR_NODE*)reaction );
    
    num = GetLinkedListSize( reactants ) + GetLinkedListSize( products );
    if( num != 1 ) {
        END_FUNCTION("_IsReactionTransformable", SUCCESS );
        return FALSE;
    } 
    
    END_FUNCTION("_IsReactionTransformable", SUCCESS );
    return TRUE;
}
