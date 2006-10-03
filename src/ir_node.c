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


#include "ir_node.h"

static IR_EDGE * _CopyReactantEdge( IR_EDGE *edge, IR_NODE *from, IR_NODE *to );
static IR_EDGE * _CopyModifierEdge( IR_EDGE *edge, IR_NODE *from, IR_NODE *to );
static IR_EDGE * _CopyProductEdge( IR_EDGE *edge, IR_NODE *from, IR_NODE *to );
static LINKED_LIST * _CopyReactantEdgeList( LINKED_LIST *edges, IR_NODE *from, IR_NODE *to );
static LINKED_LIST * _CopyModifierEdgeList( LINKED_LIST *edges, IR_NODE *from, IR_NODE *to );
static LINKED_LIST * _CopyProductEdgeList( LINKED_LIST *edges, IR_NODE *from, IR_NODE *to );



RET_VAL InitIRNode(  IR_NODE *node, char *name ) {
    RET_VAL ret = SUCCESS;
    char buf[64];
    
    START_FUNCTION("InitIRNode");
    
    sprintf( buf, "N_%X", (int)node );
    if( ( node->id = CreateString( buf ) ) == NULL ) {
        END_FUNCTION("InitIRNode", FAILING );
        return FAILING;
    }
    
    if( ( node->name = CreateString( name ) ) == NULL ) {
        END_FUNCTION("InitIRNode", FAILING );
        return FAILING;
    }
    
    node->graphFlag = GRAPH_FLAG_WHITE;
    if( ( node->reactants = CreateLinkedList() ) == NULL ) {
        END_FUNCTION("InitIRNode", FAILING );
        return FAILING;    
    }
    if( ( node->modifiers = CreateLinkedList() ) == NULL ) {
        DeleteLinkedList( &(node->reactants) ); 
        END_FUNCTION("InitIRNode", FAILING );
        return FAILING;    
    }
    if( ( node->products = CreateLinkedList() ) == NULL ) {
        DeleteLinkedList( &(node->reactants) ); 
        DeleteLinkedList( &(node->modifiers) ); 
        END_FUNCTION("InitIRNode", FAILING );
        return FAILING;    
    }    
    
    END_FUNCTION("InitIRNode", SUCCESS );
    return ret;
}

RET_VAL CopyIRNode( IR_NODE *from, IR_NODE *to ) {
    RET_VAL ret = SUCCESS;
    char buf[64];
    LINKED_LIST *list = NULL;
    IR_NODE *target = NULL;
    
    START_FUNCTION("CopyIRNode");

    sprintf( buf, "N_%X", (int)to );
    if( ( to->id = CreateString( buf ) ) == NULL ) {
        END_FUNCTION("CopyIRNode", FAILING );
        return FAILING;
    }
            
    if( ( to->name = CloneString( from->name ) ) == NULL ) {
        END_FUNCTION("CopyIRNode", FAILING );
        return FAILING;
    }
    
    to->GetType = from->GetType;                                                                              
    to->Clone = from->Clone;    
    to->ReleaseResource = from->ReleaseResource;
    
    
    to->graphFlag = from->graphFlag;
    if( ( to->reactants = _CopyReactantEdgeList( from->reactants, from, to ) ) == NULL ) {
        END_FUNCTION("CopyIRNode", FAILING );
        return FAILING;    
    }
    if( ( to->modifiers = _CopyModifierEdgeList( from->modifiers, from, to ) ) == NULL ) {
        END_FUNCTION("CopyIRNode", FAILING );
        return FAILING;    
    }
    if( ( to->products = _CopyProductEdgeList( from->products, from, to ) ) == NULL ) {
        END_FUNCTION("CopyIRNode", FAILING );
        return FAILING;    
    }    
            
    END_FUNCTION("CopyIRNode", SUCCESS );
    return SUCCESS;
}



RET_VAL ReleaseIRNodeResources(  IR_NODE *node ) {
    RET_VAL ret = SUCCESS;
    LINKED_LIST *list = NULL;
    IR_EDGE *target = NULL;
    
    START_FUNCTION("ReleaseIRNodeResources");
        
    list = node->reactants;
    ResetCurrentElement( list );
    while( ( target = (IR_EDGE*)GetNextFromLinkedList( list ) ) != NULL ) {
        if( IS_FAILED( ( ret = FreeReactantEdge( &target ) ) ) ) {
            END_FUNCTION("ReleaseIRNodeResources", FAILING );
            return FAILING;    
        }
    }    
    if( IS_FAILED( ( ret = DeleteLinkedList( &list ) ) ) ) {
        END_FUNCTION("ReleaseIRNodeResources", FAILING );
        return FAILING;    
    }            
    
    list = node->modifiers;
    ResetCurrentElement( list );
    while( ( target = (IR_EDGE*)GetNextFromLinkedList( list ) ) != NULL ) {
        if( IS_FAILED( ( ret = FreeModifierEdge( &target ) ) ) ) {
            END_FUNCTION("ReleaseIRNodeResources", FAILING );
            return FAILING;    
        }            
    }    
    if( IS_FAILED( ( ret = DeleteLinkedList( &list ) ) ) ) {
        END_FUNCTION("ReleaseIRNodeResources", FAILING );
        return FAILING;    
    }            
    
    list = node->products;
    ResetCurrentElement( list );
    while( ( target = (IR_EDGE*)GetNextFromLinkedList( list ) ) != NULL ) {
        if( IS_FAILED( ( ret = FreeProductEdge( &target ) ) ) ) {
            END_FUNCTION("ReleaseIRNodeResources", FAILING );
            return FAILING;    
        }            
    }    
    if( IS_FAILED( ( ret = DeleteLinkedList( &list ) ) ) ) {
        END_FUNCTION("ReleaseIRNodeResources", FAILING );
        return FAILING;    
    }            

    FreeString( &(node->name) ); 
    FreeString( &(node->id) );   
    END_FUNCTION("ReleaseIRNodeResources", SUCCESS );
    return ret;
}

STRING *GetIRNodeID( IR_NODE *node ) {
    START_FUNCTION("GetIRNodeID");

    if( node == NULL ) {
        END_FUNCTION("GetIRNodeID", FAILING );
        return NULL;
    }
    
    END_FUNCTION("GetIRNodeID", SUCCESS );
#ifdef NAME_FOR_ID    
    return node->name;
#else 
    return node->id;
#endif
}


STRING *GetIRNodeName( IR_NODE *node ) {
    START_FUNCTION("GetIRNodeName");

    if( node == NULL ) {
        END_FUNCTION("GetIRNodeName", FAILING );
        return NULL;
    }
    
    END_FUNCTION("GetIRNodeName", SUCCESS );
    return node->name;
}

RET_VAL SetIRNodeName( IR_NODE *node, STRING *id ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("SetIRNodeName");

    if( node == NULL ) {
        return ErrorReport( FAILING, "SetIRNodeName", "input node is NULL");
    }
    FreeString( &(node->name) );
    node->name = id;
        
    END_FUNCTION("SetIRNodeName", SUCCESS );
    return ret;
}


LINKED_LIST *GetReactantEdges( IR_NODE *node ) {
    START_FUNCTION("GetReactantEdges");

    if( node == NULL ) {
        END_FUNCTION("GetReactantEdges", FAILING );
        return NULL;
    }
    
    END_FUNCTION("GetReactantEdges", SUCCESS );
    return node->reactants;
}

LINKED_LIST *GetModifierEdges( IR_NODE *node ) {
    START_FUNCTION("GetModifierEdges");

    if( node == NULL ) {
        END_FUNCTION("GetModifierEdges", FAILING );
        return NULL;
    }
    
    END_FUNCTION("GetModifierEdges", SUCCESS );
    return node->modifiers;
}

LINKED_LIST *GetProductEdges( IR_NODE *node ) {
    START_FUNCTION("GetProductEdges");

    if( node == NULL ) {
        END_FUNCTION("GetProductEdges", FAILING );
        return NULL;
    }
    
    END_FUNCTION("GetProductEdges", SUCCESS );
    return node->products;
}


CADDR_T GetTempFromIRNode( IR_NODE *node ) {
    START_FUNCTION("GetTempFromIRNode");

    if( node == NULL ) {
        END_FUNCTION("GetTempFromIRNode", FAILING );
        return NULL;
    }
    
    END_FUNCTION("GetTempFromIRNode", SUCCESS );
    return node->temp;
}



RET_VAL SetGraphFlagToWhiteInIRNode( IR_NODE *node ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("SetGraphFlagToWhiteInIRNode");

    if( node == NULL ) {
        return ErrorReport( FAILING, "SetGraphFlagToWhiteInIRNode", "input node is NULL");
    }
    node->graphFlag = GRAPH_FLAG_WHITE;
        
    END_FUNCTION("SetGraphFlagToWhiteInIRNode", SUCCESS );
    return ret;
}

RET_VAL SetGraphFlagToGreyInIRNode( IR_NODE *node ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("SetGraphFlagToGreyInIRNode");

    if( node == NULL ) {
        return ErrorReport( FAILING, "SetGraphFlagToGreyInIRNode", "input node is NULL");
    }
    node->graphFlag = GRAPH_FLAG_GREY;
        
    END_FUNCTION("SetGraphFlagToGreyInIRNode", SUCCESS );
    return ret;
}

RET_VAL SetGraphFlagToBlackInIRNode( IR_NODE *node ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("SetGraphFlagToBlackInIRNode");

    if( node == NULL ) {
        return ErrorReport( FAILING, "SetGraphFlagToBlackInIRNode", "input node is NULL");
    }
    node->graphFlag = GRAPH_FLAG_BLACK;
        
    END_FUNCTION("SetGraphFlagToBlackInIRNode", SUCCESS );
    return ret;
}

RET_VAL SetTempInIRNode( IR_NODE *node, CADDR_T temp ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("SetTempInIRNode");

    node->temp = temp;
        
    END_FUNCTION("SetTempInIRNode", SUCCESS );
    return ret;
}




BOOL IsGraphFlagWhiteInIRNode( IR_NODE *node ) {
    START_FUNCTION("IsGraphFlagWhiteInIRNode");

    if( node == NULL ) {
        END_FUNCTION("IsGraphFlagWhiteInIRNode", FAILING );
        return FALSE;
    }
        
    END_FUNCTION("IsGraphFlagWhiteInIRNode", SUCCESS );
    return ( (node->graphFlag == GRAPH_FLAG_WHITE) ? TRUE : FALSE );
}

BOOL IsGraphFlagGreyInIRNode( IR_NODE *node ) {
    START_FUNCTION("IsGraphFlagGreyInIRNode");

    if( node == NULL ) {
        END_FUNCTION("IsGraphFlagGreyInIRNode", FAILING );
        return FALSE;
    }
        
    END_FUNCTION("IsGraphFlagGreyInIRNode", SUCCESS );
    return ( (node->graphFlag == GRAPH_FLAG_WHITE) ? TRUE : FALSE );
}

BOOL IsGraphFlagBlackInIRNode( IR_NODE *node ) {
    START_FUNCTION("IsGraphFlagBlackInIRNode");

    if( node == NULL ) {
        END_FUNCTION("IsGraphFlagBlackInIRNode", FAILING );
        return FALSE;
    }
        
    END_FUNCTION("IsGraphFlagBlackInIRNode", SUCCESS );
    return ( (node->graphFlag == GRAPH_FLAG_BLACK) ? TRUE : FALSE );
}








/********************************************************************
edge 
********************************************************************/



DLLSCOPE IR_EDGE * STDCALL CreateReactantEdge( IR_NODE *reaction, IR_NODE *species, int stoichiometry  ) {
    RET_VAL ret = SUCCESS;
    IR_EDGE *edge = NULL;
    
    START_FUNCTION("CreateReactantEdge");
    
    if( ( edge = (IR_EDGE*)MALLOC( sizeof(IR_EDGE) ) ) == NULL ) {
        END_FUNCTION("CreateReactantEdge", FAILING );
        return NULL;
    }
    
    edge->species = species;
    edge->stoichiometry = stoichiometry;
    edge->reaction = reaction;;
    
    if( IS_FAILED( ( ret = AddElementInLinkedList( (CADDR_T)edge, reaction->reactants ) ) ) ) {
        FREE( edge );
        END_FUNCTION("CreateReactantEdge", ret );
        return NULL;    
    }
    if( IS_FAILED( ( ret = AddElementInLinkedList( (CADDR_T)edge, species->reactants ) ) ) ) {
        FREE( edge );
        END_FUNCTION("CreateReactantEdge", ret );
        return NULL;    
    }
    
    END_FUNCTION("CreateReactantEdge", SUCCESS );
    return edge;
}

DLLSCOPE IR_EDGE * STDCALL CreateModifierEdge( IR_NODE *reaction, IR_NODE *species, int stoichiometry  ) {
    RET_VAL ret = SUCCESS;
    IR_EDGE *edge = NULL;
    
    START_FUNCTION("CreateModifierEdge");
    
    if( ( edge = (IR_EDGE*)MALLOC( sizeof(IR_EDGE) ) ) == NULL ) {
        END_FUNCTION("CreateModifierEdge", FAILING );
        return NULL;
    }
    
    edge->species = species;
    edge->stoichiometry = stoichiometry;
    edge->reaction = reaction;;
    
    if( IS_FAILED( ( ret = AddElementInLinkedList( (CADDR_T)edge, reaction->modifiers ) ) ) ) {
        FREE( edge );
        END_FUNCTION("CreateModifierEdge", ret );
        return NULL;    
    }
    if( IS_FAILED( ( ret = AddElementInLinkedList( (CADDR_T)edge, species->modifiers ) ) ) ) {
        FREE( edge );
        END_FUNCTION("CreateModifierEdge", ret );
        return NULL;    
    }
    
    END_FUNCTION("CreateModifierEdge", SUCCESS );
    return edge;
}

DLLSCOPE IR_EDGE * STDCALL CreateProductEdge( IR_NODE *reaction, IR_NODE *species, int stoichiometry  ) {
    RET_VAL ret = SUCCESS;
    IR_EDGE *edge = NULL;
    
    START_FUNCTION("CreateProductEdge");
    
    if( ( edge = (IR_EDGE*)MALLOC( sizeof(IR_EDGE) ) ) == NULL ) {
        END_FUNCTION("CreateProductEdge", FAILING );
        return NULL;
    }
    
    edge->species = species;
    edge->stoichiometry = stoichiometry;
    edge->reaction = reaction;;
    
    if( IS_FAILED( ( ret = AddElementInLinkedList( (CADDR_T)edge, reaction->products ) ) ) ) {
        FREE( edge );
        END_FUNCTION("CreateProductEdge", ret );
        return NULL;    
    }
    if( IS_FAILED( ( ret = AddElementInLinkedList( (CADDR_T)edge, species->products ) ) ) ) {
        FREE( edge );
        END_FUNCTION("CreateProductEdge", ret );
        return NULL;    
    }
    
    END_FUNCTION("CreateProductEdge", SUCCESS );
    return edge;
}


DLLSCOPE RET_VAL STDCALL FreeReactantEdge( IR_EDGE **edge ) {
    RET_VAL ret =SUCCESS;
    IR_NODE *reaction = NULL;
    IR_NODE *species = NULL;
    IR_EDGE *temp = NULL;    
    
    START_FUNCTION("FreeReactantEdge");
    
    temp = *edge;    
    if( temp == NULL ) {
        END_FUNCTION("FreeReactantEdge", SUCCESS );
        return ret;
    }
    
    reaction = temp->reaction;
    species = temp->species;
    
    if( IS_FAILED( ( ret =  RemoveElementFromLinkedList( (CADDR_T)temp, reaction->reactants ) ) ) ) {
        END_FUNCTION("FreeReactantEdge", ret );
        return ret;    
    }
    if( IS_FAILED( ( ret = RemoveElementFromLinkedList( (CADDR_T)temp, species->reactants ) ) ) ) {
        END_FUNCTION("FreeReactantEdge", ret );
        return ret;    
    }

    FREE( *edge );    
    
    END_FUNCTION("FreeReactantEdge", SUCCESS );
    return ret;
}

DLLSCOPE RET_VAL STDCALL FreeModifierEdge( IR_EDGE **edge ) {
    RET_VAL ret =SUCCESS;
    IR_NODE *reaction = NULL;
    IR_NODE *species = NULL;
    IR_EDGE *temp = NULL;    
    
    START_FUNCTION("FreeModifierEdge");
    
    temp = *edge;    
    if( temp == NULL ) {
        END_FUNCTION("FreeModifierEdge", SUCCESS );
        return ret;
    }
    
    reaction = temp->reaction;
    species = temp->species;
    
    if( IS_FAILED( ( ret =  RemoveElementFromLinkedList( (CADDR_T)temp, reaction->modifiers ) ) ) ) {
        END_FUNCTION("FreeModifierEdge", ret );
        return ret;    
    }
    if( IS_FAILED( ( ret = RemoveElementFromLinkedList( (CADDR_T)temp, species->modifiers ) ) ) ) {
        END_FUNCTION("FreeModifierEdge", ret );
        return ret;    
    }

    FREE( *edge );    
    
    END_FUNCTION("FreeModifierEdge", SUCCESS );
    return ret;
}

DLLSCOPE RET_VAL STDCALL FreeProductEdge( IR_EDGE **edge ) {
    RET_VAL ret =SUCCESS;
    IR_NODE *reaction = NULL;
    IR_NODE *species = NULL;
    IR_EDGE *temp = NULL;    
    
    START_FUNCTION("FreeProductEdge");
    
    temp = *edge;    
    if( temp == NULL ) {
        END_FUNCTION("FreeProductEdge", SUCCESS );
        return ret;
    }
    
    reaction = temp->reaction;
    species = temp->species;
    
    if( IS_FAILED( ( ret =  RemoveElementFromLinkedList( (CADDR_T)temp, reaction->products ) ) ) ) {
        END_FUNCTION("FreeProductEdge", ret );
        return ret;    
    }
    if( IS_FAILED( ( ret = RemoveElementFromLinkedList( (CADDR_T)temp, species->products ) ) ) ) {
        END_FUNCTION("FreeProductEdge", ret );
        return ret;    
    }

    FREE( *edge );    
    
    END_FUNCTION("FreeProductEdge", SUCCESS );
    return ret;
}


static IR_EDGE * _CopyReactantEdge( IR_EDGE *edge, IR_NODE *from, IR_NODE *to ) {
    IR_EDGE *clone = NULL;
    IR_NODE *species = NULL;
    IR_NODE *reaction = NULL;
    
    if( edge == NULL ) {
        TRACE_0("the input edge is NULL" );
        return NULL;    
    }
    
    if( ( clone = (IR_EDGE*)MALLOC( sizeof( IR_EDGE ) ) ) == NULL ) {
        TRACE_0("could not create a copy of edge" );
        return NULL;    
    }
    
    clone->stoichiometry = edge->stoichiometry;
    
    species = edge->species;
    reaction = edge->reaction;
    if( species == from ) {
        if( IS_FAILED( ( AddElementInLinkedList( (CADDR_T)clone, reaction->reactants ) ) ) ) {
            TRACE_0("could not create a copy of edge" );
            return NULL;    
        }
        clone->species = to;
        clone->reaction = reaction;
    }
    else {
        if( IS_FAILED( ( AddElementInLinkedList( (CADDR_T)clone, species->reactants ) ) ) ) {
            TRACE_0("could not create a copy of edge" );
            return NULL;    
        }
        clone->species = species;
        clone->reaction = to; 
    }    
    
    return clone;
}

static IR_EDGE * _CopyModifierEdge( IR_EDGE *edge, IR_NODE *from, IR_NODE *to ) {
    IR_EDGE *clone = NULL;
    IR_NODE *species = NULL;
    IR_NODE *reaction = NULL;
    
    if( edge == NULL ) {
        TRACE_0("the input edge is NULL" );
        return NULL;    
    }
    
    if( ( clone = (IR_EDGE*)MALLOC( sizeof( IR_EDGE ) ) ) == NULL ) {
        TRACE_0("could not create a copy of edge" );
        return NULL;    
    }
    
    clone->stoichiometry = edge->stoichiometry;
    
    species = edge->species;
    reaction = edge->reaction;
    if( species == from ) {
        if( IS_FAILED( ( AddElementInLinkedList( (CADDR_T)clone, reaction->modifiers ) ) ) ) {
            TRACE_0("could not create a copy of edge" );
            return NULL;    
        }
        clone->species = to;
        clone->reaction = reaction;
    }
    else {
        if( IS_FAILED( ( AddElementInLinkedList( (CADDR_T)clone, species->modifiers ) ) ) ) {
            TRACE_0("could not create a copy of edge" );
            return NULL;    
        }
        clone->species = species;
        clone->reaction = to; 
    }    
    
    return clone;
}

static IR_EDGE * _CopyProductEdge( IR_EDGE *edge, IR_NODE *from, IR_NODE *to ) {
    IR_EDGE *clone = NULL;
    IR_NODE *species = NULL;
    IR_NODE *reaction = NULL;
    
    if( edge == NULL ) {
        TRACE_0("the input edge is NULL" );
        return NULL;    
    }
    
    if( ( clone = (IR_EDGE*)MALLOC( sizeof( IR_EDGE ) ) ) == NULL ) {
        TRACE_0("could not create a copy of edge" );
        return NULL;    
    }
    
    clone->stoichiometry = edge->stoichiometry;
    
    species = edge->species;
    reaction = edge->reaction;
    if( species == from ) {
        if( IS_FAILED( ( AddElementInLinkedList( (CADDR_T)clone, reaction->products ) ) ) ) {
            TRACE_0("could not create a copy of edge" );
            return NULL;    
        }
        clone->species = to;
        clone->reaction = reaction;
    }
    else {
        if( IS_FAILED( ( AddElementInLinkedList( (CADDR_T)clone, species->products ) ) ) ) {
            TRACE_0("could not create a copy of edge" );
            return NULL;    
        }
        clone->species = species;
        clone->reaction = to; 
    }    
    
    return clone;
}

static LINKED_LIST * _CopyReactantEdgeList( LINKED_LIST *edges, IR_NODE *from, IR_NODE *to ) {
    LINKED_LIST *clone = NULL;
    IR_EDGE *edge = NULL;
    IR_EDGE *copy = NULL;
    CADDR_T savedCurrent = NULL;
    
    if( edges == NULL ) {
        TRACE_0("the input edge list is NULL" );
        return NULL;    
    }
    if( ( clone = CreateLinkedList() ) == NULL ) {
        TRACE_0("could not create a copy of edge list" );
        return NULL;    
    }
    

    savedCurrent = GetCurrentFromLinkedList( edges );    

    ResetCurrentElement( edges );
    while( ( edge = GetNextEdge( edges ) ) != NULL ) {
        if( IS_FAILED( AddElementInLinkedList( (CADDR_T)(_CopyReactantEdge( edge, from, to )), clone ) ) ) {
            TRACE_0("could not create a copy of edge list" );
            return NULL;    
        }                
    }
    

    SetAsCurrentInLinkedList( savedCurrent, edges );

    return clone;    
}

static LINKED_LIST * _CopyModifierEdgeList( LINKED_LIST *edges, IR_NODE *from, IR_NODE *to ) {
    LINKED_LIST *clone = NULL;
    IR_EDGE *edge = NULL;
    CADDR_T savedCurrent = NULL;
        
    if( edges == NULL ) {
        TRACE_0("the input edge list is NULL" );
        return NULL;    
    }
    if( ( clone = CreateLinkedList() ) == NULL ) {
        TRACE_0("could not create a copy of edge list" );
        return NULL;    
    }
    
    savedCurrent = GetCurrentFromLinkedList( edges );    

    ResetCurrentElement( edges );
    while( ( edge = (IR_EDGE*)GetNextFromLinkedList( edges ) ) != NULL ) {
        if( IS_FAILED( AddElementInLinkedList( (CADDR_T)(_CopyModifierEdge( edge, from, to )), clone ) ) ) {
            TRACE_0("could not create a copy of edge list" );
            return NULL;    
        }                
    }

    SetAsCurrentInLinkedList( savedCurrent, edges );
    return clone;    
}

static LINKED_LIST * _CopyProductEdgeList( LINKED_LIST *edges, IR_NODE *from, IR_NODE *to ) {
    LINKED_LIST *clone = NULL;
    IR_EDGE *edge = NULL;
    CADDR_T savedCurrent = NULL;
        
    if( edges == NULL ) {
        TRACE_0("the input edge list is NULL" );
        return NULL;    
    }
    if( ( clone = CreateLinkedList() ) == NULL ) {
        TRACE_0("could not create a copy of edge list" );
        return NULL;    
    }
    

    savedCurrent = GetCurrentFromLinkedList( edges );    
    ResetCurrentElement( edges );
    while( ( edge = (IR_EDGE*)GetNextFromLinkedList( edges ) ) != NULL ) {
        if( IS_FAILED( AddElementInLinkedList( (CADDR_T)(_CopyProductEdge( edge, from, to )), clone ) ) ) {
            TRACE_0("could not create a copy of edge list" );
            return NULL;    
        }                
    }

    SetAsCurrentInLinkedList( savedCurrent, edges );
    return clone;    
}

DLLSCOPE IR_EDGE * STDCALL GetNextEdge( LINKED_LIST *list ) {
    IR_EDGE *edge = NULL;
    
    START_FUNCTION("GetNextEdge");
    edge = (IR_EDGE*)GetNextFromLinkedList( list );
    END_FUNCTION("GetNextEdge", SUCCESS );
    return edge;
}

DLLSCOPE IR_EDGE * STDCALL GetHeadEdge( LINKED_LIST *list ) {
    IR_EDGE *edge = NULL;
    
    START_FUNCTION("GetHeadEdge");
    edge = (IR_EDGE*)GetHeadFromLinkedList( list );
    END_FUNCTION("GetHeadEdge", SUCCESS );
    return edge;
}

DLLSCOPE IR_EDGE * STDCALL GetTailEdge( LINKED_LIST *list ) {
    IR_EDGE *edge = NULL;
    
    START_FUNCTION("GetTailEdge");
    edge = (IR_EDGE*)GetTailFromLinkedList( list );
    END_FUNCTION("GetTailEdge", SUCCESS );
    return edge;
}




