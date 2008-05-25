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

static char * _GetSingleReactantProductReactionEliminationMethodID( ABSTRACTION_METHOD *method );
static RET_VAL _ApplySingleReactantProductReactionEliminationMethod( ABSTRACTION_METHOD *method, IR *ir );      
static BOOL _IsConditionSatisfied( ABSTRACTION_METHOD *method, IR *ir, REACTION *reaction ); 
static RET_VAL _DoTransformation( ABSTRACTION_METHOD *method, IR *ir, REACTION *reaction );

static BOOL _FindSpecies( KINETIC_LAW *kineticLaw, SPECIES *species );
static RET_VAL _VisitOpToFindSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitIntToFindSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitRealToFindSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSpeciesToFindSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSymbolToFindSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );




ABSTRACTION_METHOD *SingleReactantProductReactionEliminationMethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager ) {
    static ABSTRACTION_METHOD method;
    
    START_FUNCTION("SingleReactantProductReactionEliminationMethodConstructor");

    if( method.manager == NULL ) {
        method.manager = manager;
        method.GetID = _GetSingleReactantProductReactionEliminationMethodID;
        method.Apply = _ApplySingleReactantProductReactionEliminationMethod;
    }
    
    TRACE_0( "SingleReactantProductReactionEliminationMethodConstructor invoked" );
    
    END_FUNCTION("SingleReactantProductReactionEliminationMethodConstructor", SUCCESS );
    return &method;
}



static char * _GetSingleReactantProductReactionEliminationMethodID( ABSTRACTION_METHOD *method ) {
    START_FUNCTION("_GetSingleReactantProductReactionEliminationMethodID");
    
    END_FUNCTION("_GetSingleReactantProductReactionEliminationMethodID", SUCCESS );
    return "single-reactant-product-reaction-eliminator";
}



static RET_VAL _ApplySingleReactantProductReactionEliminationMethod( ABSTRACTION_METHOD *method, IR *ir ) {
    RET_VAL ret = SUCCESS;
    REACTION *reaction = NULL;
    LINKED_LIST *reactionList = NULL;
    
    START_FUNCTION("_ApplySingleReactantProductReactionEliminationMethod");

    reactionList = ir->GetListOfReactionNodes( ir );    
    ResetCurrentElement( reactionList );    
    while( ( reaction = (REACTION*)GetNextFromLinkedList( reactionList ) ) != NULL ) {
        if( _IsConditionSatisfied( method, ir, reaction ) ) {
            if( IS_FAILED( ( ret = _DoTransformation( method, ir, reaction ) ) ) ) {
                END_FUNCTION("_ApplySingleReactantProductReactionEliminationMethod", ret );
                return ret;
            }
        }            
    }
    
    END_FUNCTION("_ApplySingleReactantProductReactionEliminationMethod", SUCCESS );
    return ret;
}      


static BOOL _IsConditionSatisfied( ABSTRACTION_METHOD *method, IR *ir, REACTION *reaction ) {
    LINKED_LIST *list = NULL;
    
    START_FUNCTION("_IsConditionSatisfied");

    if( IsReactionReversibleInReactionNode( reaction ) ) {
        END_FUNCTION("_IsConditionSatisfied", SUCCESS );
        return FALSE;
    }
    
    list = GetReactantEdges( (IR_NODE*)reaction );
    if( GetLinkedListSize( list ) != 1 ) {
        END_FUNCTION("_IsConditionSatisfied", SUCCESS );
        return FALSE;
    }
    list = GetProductEdges( (IR_NODE*)reaction );
    if( GetLinkedListSize( list ) != 1 ) {
        END_FUNCTION("_IsConditionSatisfied", SUCCESS );
        return FALSE;
    }
    
    END_FUNCTION("_IsConditionSatisfied", SUCCESS );
    return TRUE;
}


static RET_VAL _DoTransformation( ABSTRACTION_METHOD *method, IR *ir, REACTION *reaction ) {
    RET_VAL ret = SUCCESS;
    char buf[1024];
    int stoichiometry = 0;
    STRING *newName = NULL;
    STRING *originalName = NULL;
    KINETIC_LAW *kineticLaw = NULL;
    SPECIES *reactant = NULL;
    SPECIES *product = NULL;
    REACTION *newReaction = NULL;
    REACTION *degradation = NULL;
    IR_EDGE *edge = NULL;
    LINKED_LIST *edges = NULL;
    LINKED_LIST *list = NULL;
    
    START_FUNCTION("_DoTransformation");
    
    kineticLaw = GetKineticLawInReactionNode( reaction );    
    originalName = GetReactionNodeName( reaction );
    
    if( ( degradation = ir->CloneReaction( ir, reaction ) ) == NULL ) {
        return ErrorReport( FAILING, "_DoTransformation", "failed to clone reaction %s", GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
    } 
    sprintf( buf, "%s_r", GetCharArrayOfString( originalName ) );
    if( ( newName = CreateString( buf ) ) == NULL ) {
        return ErrorReport( FAILING, "_DoTransformation", "failed to create string %s", buf );
    }
    if( IS_FAILED( ( ret = SetReactionNodeName( degradation, newName ) ) ) ) {
        END_FUNCTION("_DoTransformation", ret );
        return ret;
    }
    
    edges = GetProductEdges( (IR_NODE*)degradation );
    edge = GetHeadEdge( edges );
    product = GetSpeciesInIREdge( edge );
    stoichiometry = GetStoichiometryInIREdge( edge );
    if( _FindSpecies( kineticLaw, product ) ) {
        if( IS_FAILED( ( ret = ir->AddModifierEdge( ir, degradation, product, stoichiometry ) ) ) ) {
            END_FUNCTION("_DoTransformation", ret );
            return ret;
        }
    }
    if( IS_FAILED( ( ret = ir->RemoveProductEdge( ir, &edge ) ) ) ) {
        END_FUNCTION("_DoTransformation", ret );
        return ret;
    }
        
    
    sprintf( buf, "%s_p", GetCharArrayOfString( originalName ) );
    if( ( newName = CreateString( buf ) ) == NULL ) {
        return ErrorReport( FAILING, "_DoTransformation", "failed to create string %s", buf );
    }
    if( IS_FAILED( ( ret = SetReactionNodeName( reaction, newName ) ) ) ) {
        END_FUNCTION("_DoTransformation", ret );
        return ret;
    }
    
    edges = GetReactantEdges( (IR_NODE*)reaction );
    edge = GetHeadEdge( edges );
    reactant = GetSpeciesInIREdge( edge );
    stoichiometry = GetStoichiometryInIREdge( edge );
    if( _FindSpecies( kineticLaw, reactant ) ) {
        if( IS_FAILED( ( ret = ir->AddModifierEdge( ir, reaction, reactant, stoichiometry ) ) ) ) {
            END_FUNCTION("_DoTransformation", ret );
            return ret;
        }
    }
    if( IS_FAILED( ( ret = ir->RemoveReactantEdge( ir, &edge ) ) ) ) {
        END_FUNCTION("_DoTransformation", ret );
        return ret;
    }
                
    END_FUNCTION("_DoTransformation", SUCCESS );
    return ret;
}




static BOOL _FindSpecies( KINETIC_LAW *kineticLaw, SPECIES *species ) {
    static KINETIC_LAW_VISITOR visitor;
    BOOL flag = FALSE;
    
    START_FUNCTION("_FindSpecies");
    
    if( visitor.VisitOp == NULL ) {
        visitor.VisitOp = _VisitOpToFindSpecies;
        visitor.VisitInt = _VisitIntToFindSpecies;
        visitor.VisitReal = _VisitRealToFindSpecies;
        visitor.VisitSpecies = _VisitSpeciesToFindSpecies;
        visitor.VisitSymbol = _VisitSymbolToFindSpecies;
    }
    
    visitor._internal1 = (CADDR_T)species;
    visitor._internal2 = (CADDR_T)(&flag);
    
    if( IS_FAILED( kineticLaw->AcceptPostOrder( kineticLaw, &visitor ) ) ) {
        END_FUNCTION("_FindSpecies", SUCCESS );
        return FALSE;
    } 
    
    END_FUNCTION("_FindSpecies", SUCCESS );
    return flag;
}

static RET_VAL _VisitOpToFindSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    START_FUNCTION("_VisitOpToFindSpecies");
    END_FUNCTION("_VisitOpToFindSpecies", SUCCESS );
    return SUCCESS;
}

static RET_VAL _VisitIntToFindSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    START_FUNCTION("_VisitIntToFindSpecies");
    END_FUNCTION("_VisitIntToFindSpecies", SUCCESS );
    return SUCCESS;
}

static RET_VAL _VisitRealToFindSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    START_FUNCTION("_VisitRealToFindSpecies");
    END_FUNCTION("_VisitRealToFindSpecies", SUCCESS );
    return SUCCESS;
}


static RET_VAL _VisitSymbolToFindSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    START_FUNCTION("_VisitSymbolToFindSpecies");
    END_FUNCTION("_VisitSymbolToFindSpecies", SUCCESS );
    return SUCCESS;
}


static RET_VAL _VisitSpeciesToFindSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    BOOL *flag = NULL;
    SPECIES *species = NULL;
        
    START_FUNCTION("_VisitSpeciesToFindSpecies");
    
    species = (SPECIES*)(visitor->_internal1);
    flag = (BOOL*)(visitor->_internal2);
        
    if( !(*flag) ) {
        *flag = ( species == GetSpeciesFromKineticLaw( kineticLaw ) ? TRUE : FALSE );     
    }    
    END_FUNCTION("_VisitSpeciesToFindSpecies", SUCCESS );
    return SUCCESS;
}
