/***************************************************************************
 *   Copyright (C) 2004 by Hiroyuki Kuwahara                               *
 *   kuwahara@cs.utah.edu                                                 *
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
#if !defined(HAVE_DEPENDENCY_GRAPH)
#define HAVE_DEPENDENCY_GRAPH

#include "common.h"
#include "linked_list.h"
#include "reaction_node.h"

BEGIN_C_NAMESPACE

struct _NEXT_REACTION_DEPENDENCY_GRAPH_NODE;
typedef struct _NEXT_REACTION_DEPENDENCY_GRAPH_NODE NEXT_REACTION_DEPENDENCY_GRAPH_NODE;

struct _NEXT_REACTION_DEPENDENCY_GRAPH_NODE {
    REACTION *reaction;
    LINKED_LIST *affecteeList;        
};



struct _NEXT_REACTION_DEPENDENCY_GRAPH;
typedef struct _NEXT_REACTION_DEPENDENCY_GRAPH NEXT_REACTION_DEPENDENCY_GRAPH;


struct _NEXT_REACTION_DEPENDENCY_GRAPH {
    REACTION **reactionArray;
    UINT32 reactionsSize;
    
};


DLLSCOPE NEXT_REACTION_DEPENDENCY_GRAPH * STDCALL 
CreateNextReactionDependencyGraph( REACTION **reactionArray, UINT32 reactionsSize );

DLLSCOPE RET_VAL STDCALL FreeNextReactionDependencyGraph( NEXT_REACTION_DEPENDENCY_GRAPH **graph );





END_C_NAMESPACE

#endif
