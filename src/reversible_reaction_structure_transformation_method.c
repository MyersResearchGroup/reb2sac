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


typedef struct {
    REACTION *forward;
    REACTION *backward;
} REVERSIBLE_REACTION_PAIR;


static char * _GetReversibleReactionStructureTransformationMethodID( ABSTRACTION_METHOD *method );
static RET_VAL _ApplyReversibleReactionStructureTransformationMethod( ABSTRACTION_METHOD *method, IR *ir );      
static BOOL _IsConditionSatisfied( ABSTRACTION_METHOD *method, REVERSIBLE_REACTION_PAIR *pair );
static RET_VAL _DoTransformation( ABSTRACTION_METHOD *method, IR *ir, REVERSIBLE_REACTION_PAIR *pair );      
#if 0
static int _ComputeScore( REACTION *reaction );
#endif
static BOOL _AreListsOfSpeciesSame( LINKED_LIST *list1, LINKED_LIST *list2 ); 

ABSTRACTION_METHOD *ReversibleReactionStructureTransformationMethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager ) {
    static ABSTRACTION_METHOD method;
    
    START_FUNCTION("ReversibleReactionStructureTransformationMethodConstructor");

    if( method.manager == NULL ) {
        method.manager = manager;
        method.GetID = _GetReversibleReactionStructureTransformationMethodID;
        method.Apply = _ApplyReversibleReactionStructureTransformationMethod;
    }
    
    TRACE_0( "ReversibleReactionStructureTransformationMethodConstructor invoked" );
    
    END_FUNCTION("ReversibleReactionStructureTransformationMethodConstructor", SUCCESS );
    return &method;
}



static char * _GetReversibleReactionStructureTransformationMethodID( ABSTRACTION_METHOD *method ) {
    START_FUNCTION("_GetReversibleReactionStructureTransformationMethodID");
    
    END_FUNCTION("_GetReversibleReactionStructureTransformationMethodID", SUCCESS );
    return "reversible-reaction-structure-transformer";
}



static RET_VAL _ApplyReversibleReactionStructureTransformationMethod( ABSTRACTION_METHOD *method, IR *ir ) {
    RET_VAL ret = SUCCESS;
    REACTION *reaction = NULL;
    LINKED_LIST *reactionList = NULL;
    LINKED_LIST *cloneList = NULL;
    REVERSIBLE_REACTION_PAIR pair;
    
    START_FUNCTION("_ApplyReversibleReactionStructureTransformationMethod");

    reactionList = ir->GetListOfReactionNodes( ir );
    if( ( cloneList = CloneLinkedList( reactionList ) ) == NULL ) {
        return ErrorReport( FAILING, "_ApplyReversibleReactionStructureTransformationMethod", "could not create a clone of reaction list" );
    }
    
    ResetCurrentElement( reactionList );    
    while( ( pair.forward = (REACTION*)GetNextFromLinkedList( reactionList ) ) != NULL ) {
        ResetCurrentElement( cloneList );
        while( ( pair.backward = (REACTION*)GetNextFromLinkedList( cloneList ) ) != NULL ) {
            if( _IsConditionSatisfied( method, &pair ) ) {
                if( IS_FAILED( ( ret = _DoTransformation( method, ir, &pair ) ) ) ) {
                    END_FUNCTION("_ApplyReversibleReactionStructureTransformationMethod", ret );
                    return ret;
                }
                break;
            }
        }
    }
    DeleteLinkedList( &cloneList );        
    END_FUNCTION("_ApplyReversibleReactionStructureTransformationMethod", SUCCESS );
    return ret;
}      

static BOOL _IsConditionSatisfied( ABSTRACTION_METHOD *method, REVERSIBLE_REACTION_PAIR *pair ) {
    int score1 = 0;
    int score2 = 0;
    REACTION *reaction1 = NULL;
    REACTION *reaction2 = NULL;
    LINKED_LIST *reactionList = NULL;    
    LINKED_LIST *reactants1 = NULL;
    LINKED_LIST *reactants2 = NULL;
    LINKED_LIST *modifiers1 = NULL;
    LINKED_LIST *modifiers2 = NULL;
    LINKED_LIST *products1 = NULL;
    LINKED_LIST *products2 = NULL;
    
    START_FUNCTION("_IsConditionSatisfied");

    reaction1 = pair->forward;
    reaction2 = pair->backward;  
        
    if( reaction1 == reaction2 ) {
        END_FUNCTION("_IsConditionSatisfied", SUCCESS );
        return FALSE;
    }
    
    if( IsReactionReversibleInReactionNode( reaction1 ) ) {
        END_FUNCTION("_IsConditionSatisfied", SUCCESS );
        return FALSE;
    }
    if( IsReactionReversibleInReactionNode( reaction2 ) ) {
        END_FUNCTION("_IsConditionSatisfied", SUCCESS );
        return FALSE;
    }
    
    
    reactants1 = GetReactantEdges( reaction1 );
    if( GetLinkedListSize( reactants1 ) == 0 ) {
        END_FUNCTION("_IsConditionSatisfied", SUCCESS );
        return FALSE;
    }
    reactants2 = GetReactantEdges( reaction2 );
    if( GetLinkedListSize( reactants2 ) == 0 ) {
        END_FUNCTION("_IsConditionSatisfied", SUCCESS );
        return FALSE;
    }
    
    products1 = GetProductEdges( reaction1 );
    if( GetLinkedListSize( products1 ) == 0 ) {
        END_FUNCTION("_IsConditionSatisfied", SUCCESS );
        return FALSE;
    }
    products2 = GetProductEdges( reaction2 );
    if( GetLinkedListSize( products2 ) == 0 ) {
        END_FUNCTION("_IsConditionSatisfied", SUCCESS );
        return FALSE;
    }
    
    modifiers1 = GetModifierEdges( reaction1 );
    modifiers2 = GetModifierEdges( reaction2 );
    
    if( !_AreListsOfSpeciesSame( reactants1, products2 ) ) {
        END_FUNCTION("_IsConditionSatisfied", SUCCESS );
        return FALSE;
    }
    
    if( !_AreListsOfSpeciesSame( reactants2, products1 ) ) {
        END_FUNCTION("_IsConditionSatisfied", SUCCESS );
        return FALSE;
    }
    
    if( !_AreListsOfSpeciesSame( modifiers1, modifiers2 ) ) {
        END_FUNCTION("_IsConditionSatisfied", SUCCESS );
        return FALSE;
    }
    
    TRACE_2( "reaction %s and reaction %s are complimentary set", 
        GetCharArrayOfString( GetReactionNodeName( reaction1 ) ), GetCharArrayOfString( GetReactionNodeName( reaction2 ) ) );

    /*        
    score1 = _ComputeScore( reaction1 );
    score2 = _ComputeScore( reaction2 );
    
    if( score1 < score2 ) {
        TRACE_2( "%s is the forward reaction and %s is the backward reaction", 
            GetCharArrayOfString( GetReactionNodeName( reaction1 ) ), GetCharArrayOfString( GetReactionNodeName( reaction2 ) ) );
        
        END_FUNCTION("_IsConditionSatisfied", SUCCESS );
        return TRUE;
    }
    else if( score1 > score2 ) {
        TRACE_2( "%s is the forward reaction and %s is the backward reaction", 
            GetCharArrayOfString( GetReactionNodeName( reaction2 ) ), GetCharArrayOfString( GetReactionNodeName( reaction1 ) ) );
        
        pair->forward = reaction2;
        pair->backward = reaction1;
            
        END_FUNCTION("_IsConditionSatisfied", SUCCESS );
        return TRUE;
    }
    */

    if( GetLinkedListSize( reactants1 ) >= GetLinkedListSize( reactants2 ) ) {
        TRACE_2( "%s is the forward reaction and %s is the backward reaction", 
            GetCharArrayOfString( GetReactionNodeName( reaction1 ) ), GetCharArrayOfString( GetReactionNodeName( reaction2 ) ) );
        
        END_FUNCTION("_IsConditionSatisfied", SUCCESS );
        return TRUE;
    }
    else {
        TRACE_2( "%s is the forward reaction and %s is the backward reaction", 
            GetCharArrayOfString( GetReactionNodeName( reaction2 ) ), GetCharArrayOfString( GetReactionNodeName( reaction1 ) ) );
        
        pair->forward = reaction2;
        pair->backward = reaction1;
            
        END_FUNCTION("_IsConditionSatisfied", SUCCESS );
        return TRUE;
    }
}



static RET_VAL _DoTransformation( ABSTRACTION_METHOD *method, IR *ir, REVERSIBLE_REACTION_PAIR *pair ) {
    RET_VAL ret = SUCCESS;
    char buf[256];
    STRING *newID = NULL;
    SPECIES *species = NULL;
    REACTION *forward = NULL;
    REACTION *backward = NULL;
    REACTION *newReaction = NULL;
    KINETIC_LAW *newKineticLaw = NULL;
    KINETIC_LAW *left = NULL;
    KINETIC_LAW *right = NULL;
    LINKED_LIST *list = NULL;
    
    START_FUNCTION("_DoTransformation");
    
    forward = pair->forward;
    backward = pair->backward;
    
    sprintf( buf, "%s_and_%s", 
        GetCharArrayOfString( GetReactionNodeName( forward ) ), GetCharArrayOfString( GetReactionNodeName( backward ) ) );
    
    if( ( newID = CreateString( buf ) ) == NULL ) {
        return ErrorReport( FAILING, "_DoTransformation", "could not create a reaction %s", buf );
    }
    if( IS_FAILED( ( ret = SetReactionNodeName( forward, newID ) ) ) ) {
        END_FUNCTION("_DoTransformation", ret );
        return ret;
    }
    
    left = GetKineticLawInReactionNode( forward );
    right = GetKineticLawInReactionNode( backward );
    
    if( ( newKineticLaw = CreateOpKineticLaw( KINETIC_LAW_OP_MINUS, CloneKineticLaw( left ), CloneKineticLaw( right ) ) ) == NULL ) {
        return ErrorReport( FAILING, "_DoTransformation", "failed to create a kinetic law for %s", buf );                        
    }
    
    
    if( IS_FAILED( ( ret = SetKineticLawInReactionNode( forward, newKineticLaw ) ) ) ) {
        END_FUNCTION("_DoTransformation", ret );
        return ret;
    }
    
    if( IS_FAILED( ( ret = SetReactionReversibleInReactionNode( forward, TRUE ) ) ) ) {
        END_FUNCTION("_DoTransformation", ret );
        return ret;
    }
    
    FreeKineticLaw( &left );
    if( IS_FAILED( ( ret = ir->RemoveReaction( ir, backward ) ) ) ) {
        END_FUNCTION("_DoTransformation", ret );
        return ret;
    }
    
    
    END_FUNCTION("_DoTransformation", SUCCESS );
    return ret;
}      

#if 0
static int _ComputeScore( REACTION *reaction ) {
    int score = 0;
    SPECIES *reactant = NULL;
    REACTION *target = NULL;
    LINKED_LIST *reactants = NULL;    
    LINKED_LIST *reactions = NULL;    
    
    START_FUNCTION("_DoTransformation");

    reactants = GetReactantsInReactionNode( reaction );
    ResetCurrentElement( reactants );
    while( ( reactant = (SPECIES*)GetNextFromLinkedList( reactants ) ) != NULL ) {
        reactions = GetReactionsAsReactantInSpeciesNode( reactant );
        while( ( target = (REACTION*)GetNextFromLinkedList( reactions ) ) != NULL ) {
            if( !IsReactionReversibleInReactionNode( target ) ) {
                score++;
            }
        }
        
        reactions = GetReactionsAsProductInSpeciesNode( reactant );
        while( ( target = (REACTION*)GetNextFromLinkedList( reactions ) ) != NULL ) {
            if( !IsReactionReversibleInReactionNode( target ) ) {
                score--;
            }
        }
    }
    TRACE_2("the score of %s is %i", GetCharArrayOfString( GetReactionNodeName( reaction ) ), score );
                
    END_FUNCTION("_DoTransformation", SUCCESS );
    return score;
}
#endif

static BOOL _AreListsOfSpeciesSame( LINKED_LIST *list1, LINKED_LIST *list2 ) {
    IR_EDGE *e1 = NULL;
    IR_EDGE *e2 = NULL;
    SPECIES *species1 = NULL;
    SPECIES *species2 = NULL;
    BOOL foundSame = FALSE;
    
    START_FUNCTION("_AreListsOfSpeciesSame");

    if( list1 == list2 ) {
        END_FUNCTION("_AreListsOfSpeciesSame", SUCCESS );
        return TRUE;
    }
    
    if( GetLinkedListSize( list1 ) != GetLinkedListSize( list2 ) ) {
        END_FUNCTION("_AreListsOfSpeciesSame", SUCCESS );
        return FALSE;
    }

    ResetCurrentElement( list1 );        
    while( ( e1 = GetNextEdge( list1 ) ) != NULL ) {
        species1 = GetSpeciesInIREdge( e1 );
        ResetCurrentElement( list2 );
        foundSame = FALSE;
        while( ( e2 = GetNextEdge( list2 ) ) != NULL ) {
            species2 = GetSpeciesInIREdge( e2 );            
            if( species1 == species2 ) {
                foundSame = TRUE;
                break;
            }
        }
        if ( !foundSame ) {
            END_FUNCTION("_AreListsOfSpeciesSame", SUCCESS );
            return FALSE;
        }
    }
        
    END_FUNCTION("_AreListsOfSpeciesSame", SUCCESS );
    return TRUE;
}







