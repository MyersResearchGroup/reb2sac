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

static char * _GetSimilarReactionCombiningMethodID( ABSTRACTION_METHOD *method );
static RET_VAL _ApplySimilarReactionCombiningMethod( ABSTRACTION_METHOD *method, IR *ir );      
 
static BOOL _AreReactionsStructurallyEqual( REACTION *r1, REACTION *r2 );
static BOOL _ContainsSameSpecies( LINKED_LIST *l1, LINKED_LIST *l2 );
static int _CompareSpeciesEdge( CADDR_T e1, CADDR_T e2 );
static KINETIC_LAW *_GenerateCombinedKineticLaw( KINETIC_LAW *kineticLaw1, KINETIC_LAW *kineticLaw2 );
static KINETIC_LAW *_GenerateCombinedDivisionKineticLaw( KINETIC_LAW *kineticLaw1, KINETIC_LAW *kineticLaw2 );
static KINETIC_LAW *_GenerateCombinedMultiplicationKineticLaw( KINETIC_LAW *kineticLaw1, KINETIC_LAW *kineticLaw2 );


ABSTRACTION_METHOD *SimilarReactionCombiningMethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager ) {
    static ABSTRACTION_METHOD method;
    
    START_FUNCTION("SimilarReactionCombiningMethodConstructor");

    if( method.manager == NULL ) {
        method.manager = manager;
        method.GetID = _GetSimilarReactionCombiningMethodID;
        method.Apply = _ApplySimilarReactionCombiningMethod;
    }
    
    TRACE_0( "SimilarReactionCombiningMethodConstructor invoked" );
    
    END_FUNCTION("SimilarReactionCombiningMethodConstructor", SUCCESS );
    return &method;
}



static char * _GetSimilarReactionCombiningMethodID( ABSTRACTION_METHOD *method ) {
    START_FUNCTION("_GetSimilarReactionCombiningMethodID");
    
    END_FUNCTION("_GetSimilarReactionCombiningMethodID", SUCCESS );
    return "similar-reaction-combiner";
}



static RET_VAL _ApplySimilarReactionCombiningMethod( ABSTRACTION_METHOD *method, IR *ir ) {
    RET_VAL ret = SUCCESS;
    char buf[4096];
    STRING *newName = NULL;
    KINETIC_LAW *kineticLaw1 = NULL;
    KINETIC_LAW *kineticLaw2 = NULL;
    KINETIC_LAW *newKineticLaw = NULL;
    REACTION *reaction1 = NULL;
    REACTION *reaction2 = NULL;
    LINKED_LIST *reactionList1 = NULL;
    LINKED_LIST *reactionList2 = NULL;
    
    START_FUNCTION("_ApplySimilarReactionCombiningMethod");

    
    reactionList1 = ir->GetListOfReactionNodes( ir );    
    reactionList2 = CloneLinkedList( reactionList1 );
    
    ResetCurrentElement( reactionList1 );    
    while( ( reaction1 = (REACTION*)GetNextFromLinkedList( reactionList1 ) ) != NULL ) {
        ResetCurrentElement( reactionList2 );
        while( ( reaction2 = (REACTION*)GetNextFromLinkedList( reactionList2 ) ) != NULL )  {
            if( reaction1 == reaction2 ) {
                continue;
            }
            if( _AreReactionsStructurallyEqual( reaction1, reaction2 ) ) {
                TRACE_2("combining reactions %s_%s", 
                    GetCharArrayOfString( GetReactionNodeName( reaction2 ) ), GetCharArrayOfString( GetReactionNodeName( reaction1 ) ) );                
                sprintf( buf, "%s_%s", 
                    GetCharArrayOfString( GetReactionNodeName( reaction2 ) ), GetCharArrayOfString( GetReactionNodeName( reaction1 ) ) );
                if( ( newName = CreateString( buf ) ) == NULL ) {
                    return ErrorReport( FAILING, "_ApplySimilarReactionCombiningMethod", 
                        "error creating new name for reactions %s, %s", 
                        GetCharArrayOfString( GetReactionNodeName( reaction2 ) ), GetCharArrayOfString( GetReactionNodeName( reaction1 ) ) );
                }
                if( IS_FAILED( ( ret = SetReactionNodeName( reaction2, newName ) ) ) ) {
                    END_FUNCTION("_ApplySimilarReactionCombiningMethod", ret );
                    return ret;
                }
                
                kineticLaw1 = GetKineticLawInReactionNode( reaction1 );
                kineticLaw2 = GetKineticLawInReactionNode( reaction2 );                
                if( ( newKineticLaw = _GenerateCombinedKineticLaw( kineticLaw1, kineticLaw2 ) ) == NULL ) {
                    return ErrorReport( FAILING, "_ApplySimilarReactionCombiningMethod", 
                        "error creating kinetic law for %s + %s", 
                        GetCharArrayOfString( ToStringKineticLaw( kineticLaw1 ) ), 
                        GetCharArrayOfString( ToStringKineticLaw( kineticLaw2 ) ) );
                } 
                if( IS_FAILED( ( ret = SetKineticLawInReactionNode( reaction2, newKineticLaw ) ) ) ) {
                    END_FUNCTION("_ApplySimilarReactionCombiningMethod", ret );
                    return ret;
                }
                if( IS_FAILED( ( RemoveElementFromLinkedList( reaction1, reactionList2 ) ) ) )  {
                    END_FUNCTION("_ApplySimilarReactionCombiningMethod", ret );
                    return ret;
                }
                if( IS_FAILED( ( ir->RemoveReaction( ir, reaction1 ) ) ) ) {
                    END_FUNCTION("_ApplySimilarReactionCombiningMethod", ret );
                    return ret;
                }
                FreeKineticLaw( &kineticLaw2 );
                break;
            }        
        }
    } 
    DeleteLinkedList( &reactionList2 );
            
    END_FUNCTION("_ApplySimilarReactionCombiningMethod", SUCCESS );
    return ret;
}      



static BOOL _AreReactionsStructurallyEqual( REACTION *r1, REACTION *r2 ) {
    LINKED_LIST *l1 = NULL;
    LINKED_LIST *l2 = NULL;
    
    START_FUNCTION("_AreReactionsStructurallyEqual");
    
    l1 = GetReactantEdges( r1 );
    l2 = GetReactantEdges( r2 );
    if( !_ContainsSameSpecies( l1, l2 ) ) {
        END_FUNCTION("_AreReactionsStructurallyEqual", SUCCESS );
        return FALSE;
    }
    
    l1 = GetProductEdges( r1 );
    l2 = GetProductEdges( r2 );
    if( !_ContainsSameSpecies( l1, l2 ) ) {
        END_FUNCTION("_AreReactionsStructurallyEqual", SUCCESS );
        return FALSE;
    }

    l1 = GetModifierEdges( r1 );
    l2 = GetModifierEdges( r2 );
    if( !_ContainsSameSpecies( l1, l2 ) ) {
        END_FUNCTION("_AreReactionsStructurallyEqual", SUCCESS );
        return FALSE;
    }
            
    END_FUNCTION("_AreReactionsStructurallyEqual", SUCCESS );
    return TRUE;
}

static BOOL _ContainsSameSpecies( LINKED_LIST *l1, LINKED_LIST *l2 ) {
    IR_EDGE *target = NULL;
    
    START_FUNCTION("_ContainsSameSpecies");
    
    if( GetLinkedListSize( l1 ) != GetLinkedListSize( l2 ) ) {
        END_FUNCTION("_ContainsSameSpecies", SUCCESS );
        return FALSE;
    }
        
    ResetCurrentElement( l1 );
    while( ( target = GetNextEdge( l1 ) ) != NULL ) {
        if( FindElementInLinkedList( target, _CompareSpeciesEdge, l2 ) < 0 ) {
            END_FUNCTION("_ContainsSameSpecies", SUCCESS );
            return FALSE;
        }
    }
            
    END_FUNCTION("_ContainsSameSpecies", SUCCESS );
    return TRUE;
}


static int _CompareSpeciesEdge( CADDR_T e1, CADDR_T e2 ) {
    SPECIES *s1 = NULL;
    SPECIES *s2 = NULL;
    
    s1 = GetSpeciesInIREdge( (IR_EDGE*)e1 );
    s2 = GetSpeciesInIREdge( (IR_EDGE*)e2 );
            
    return (int)s1 - (int)s2;
}

static KINETIC_LAW *_GenerateCombinedKineticLaw( KINETIC_LAW *kineticLaw1, KINETIC_LAW *kineticLaw2 ) {
    BYTE opType1;
    BYTE opType2;
    KINETIC_LAW *result = NULL;
    
    if( IsOpKineticLaw( kineticLaw1 ) && IsOpKineticLaw( kineticLaw2 ) ) {
        opType1 = GetOpTypeFromKineticLaw( kineticLaw1 );
        opType2 = GetOpTypeFromKineticLaw( kineticLaw2 );
        if( opType1 == opType2 ) {
            switch( opType1 ) {
                case KINETIC_LAW_OP_TIMES:
                    result = _GenerateCombinedMultiplicationKineticLaw( kineticLaw1, kineticLaw2 );
                    return result;
                
                case KINETIC_LAW_OP_DIVIDE:
                    result = _GenerateCombinedDivisionKineticLaw( kineticLaw1, kineticLaw2 );
                    return result;
                
            }
        }
    }
    result = CreateOpKineticLaw( KINETIC_LAW_OP_PLUS, CloneKineticLaw( kineticLaw1 ), CloneKineticLaw( kineticLaw2 ) );    
    return result;
}

static KINETIC_LAW *_GenerateCombinedDivisionKineticLaw( KINETIC_LAW *kineticLaw1, KINETIC_LAW *kineticLaw2 ) {
    KINETIC_LAW *left1 = NULL;
    KINETIC_LAW *right1 = NULL;
    KINETIC_LAW *left2 = NULL;
    KINETIC_LAW *right2 = NULL;
    KINETIC_LAW *result = NULL;

    left1 = GetOpLeftFromKineticLaw( kineticLaw1 );
    right1 = GetOpRightFromKineticLaw( kineticLaw1 );
    left2 = GetOpLeftFromKineticLaw( kineticLaw2 );
    right2 = GetOpRightFromKineticLaw( kineticLaw2 );
    
    if( AreKineticLawsStructurallyEqual( right1, right2 ) ) {
        result = _GenerateCombinedKineticLaw( left1, left2 );
        result = CreateOpKineticLaw( KINETIC_LAW_OP_DIVIDE, result, CloneKineticLaw( right1 ) );
    }
    else {
        result = CreateOpKineticLaw( KINETIC_LAW_OP_PLUS, CloneKineticLaw( kineticLaw1 ), CloneKineticLaw( kineticLaw2 ) );
    }
    return result;
}

static KINETIC_LAW *_GenerateCombinedMultiplicationKineticLaw( KINETIC_LAW *kineticLaw1, KINETIC_LAW *kineticLaw2 ) {
    KINETIC_LAW *left1 = NULL;
    KINETIC_LAW *right1 = NULL;
    KINETIC_LAW *left2 = NULL;
    KINETIC_LAW *right2 = NULL;
    KINETIC_LAW *result = NULL;

    left1 = GetOpLeftFromKineticLaw( kineticLaw1 );
    right1 = GetOpRightFromKineticLaw( kineticLaw1 );
    left2 = GetOpLeftFromKineticLaw( kineticLaw2 );
    right2 = GetOpRightFromKineticLaw( kineticLaw2 );
    
    if( AreKineticLawsStructurallyEqual( left1, left2 ) ) {
        result = CreateOpKineticLaw( KINETIC_LAW_OP_PLUS, CloneKineticLaw( right1 ), CloneKineticLaw( right2 ) );
        result = CreateOpKineticLaw( KINETIC_LAW_OP_TIMES, CloneKineticLaw( left1 ), result );
    }
    else if( AreKineticLawsStructurallyEqual( right1, right2 ) ) {
        result = CreateOpKineticLaw( KINETIC_LAW_OP_PLUS, CloneKineticLaw( left1 ), CloneKineticLaw( left2 ) );
        result = CreateOpKineticLaw( KINETIC_LAW_OP_TIMES, result, CloneKineticLaw( right1 ) );
    }
    else {
        result = CreateOpKineticLaw( KINETIC_LAW_OP_PLUS, CloneKineticLaw( kineticLaw1 ), CloneKineticLaw( kineticLaw2 ) );
    }
    return result;
}


