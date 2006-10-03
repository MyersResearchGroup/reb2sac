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
#if !defined(HAVE_IR_NODE)
#define HAVE_IR_NODE


#include "common.h"
#include "linked_list.h"
#include "util.h"

BEGIN_C_NAMESPACE


typedef BYTE GRAPH_FLAG;
#define GRAPH_FLAG_WHITE (GRAPH_FLAG)0
#define GRAPH_FLAG_GREY (GRAPH_FLAG)1
#define GRAPH_FLAG_BLACK (GRAPH_FLAG)2

struct _IR_NODE;
typedef struct _IR_NODE IR_NODE;

struct _IR_NODE {
    STRING *id;
    STRING *name;
    GRAPH_FLAG graphFlag;
    CADDR_T temp;
    LINKED_LIST *reactants;
    LINKED_LIST *modifiers;
    LINKED_LIST *products;
    
    char * (*GetType)( );                                                                              
    IR_NODE *(*Clone)( IR_NODE *node );    
    RET_VAL (*ReleaseResource)( IR_NODE *node );
};

RET_VAL InitIRNode(  IR_NODE *node, char *name );

RET_VAL CopyIRNode( IR_NODE *from, IR_NODE *to );

RET_VAL ReleaseIRNodeResources(  IR_NODE *node );

STRING *GetIRNodeID( IR_NODE *node );

STRING *GetIRNodeName( IR_NODE *node );
RET_VAL SetIRNodeName( IR_NODE *node, STRING *name );
LINKED_LIST *GetReactantEdges( IR_NODE *node );
LINKED_LIST *GetModifierEdges( IR_NODE *node );
LINKED_LIST *GetProductEdges( IR_NODE *node );

RET_VAL SetGraphFlagToWhiteInIRNode( IR_NODE *node );
RET_VAL SetGraphFlagToGreyInIRNode( IR_NODE *node );
RET_VAL SetGraphFlagToBlackInIRNode( IR_NODE *node );

BOOL IsGraphFlagWhiteInIRNode( IR_NODE *node );
BOOL IsGraphFlagGreyInIRNode( IR_NODE *node );
BOOL IsGraphFlagBlackInIRNode( IR_NODE *node );

RET_VAL SetTempInIRNode( IR_NODE *node, CADDR_T temp );
CADDR_T GetTempFromIRNode( IR_NODE *node );


typedef struct {
    IR_NODE *species;
    int stoichiometry;
    IR_NODE *reaction;    
} IR_EDGE;


DLLSCOPE IR_EDGE * STDCALL CreateReactantEdge( IR_NODE *reaction, IR_NODE *species, int stoichiometry  );
DLLSCOPE IR_EDGE * STDCALL CreateModifierEdge( IR_NODE *reaction, IR_NODE *species, int stoichiometry  );
DLLSCOPE IR_EDGE * STDCALL CreateProductEdge( IR_NODE *reaction, IR_NODE *species, int stoichiometry  );
DLLSCOPE RET_VAL STDCALL FreeReactantEdge( IR_EDGE **edge );
DLLSCOPE RET_VAL STDCALL FreeModifierEdge( IR_EDGE **edge );
DLLSCOPE RET_VAL STDCALL FreeProductEdge( IR_EDGE **edge );
DLLSCOPE IR_EDGE * STDCALL GetNextEdge( LINKED_LIST *list );
DLLSCOPE IR_EDGE * STDCALL GetHeadEdge( LINKED_LIST *list );
DLLSCOPE IR_EDGE * STDCALL GetTailEdge( LINKED_LIST *list );



END_C_NAMESPACE

#endif
