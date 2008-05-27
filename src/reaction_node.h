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
#if !defined(HAVE_REACTION_NODE)
#define HAVE_REACTION_NODE

#include "common.h"

#include "ir_node.h"
#include "kinetic_law.h"

BEGIN_C_NAMESPACE


typedef struct {
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
    
    BOOL isReversible;    
    BOOL fast;
    KINETIC_LAW *kineticLaw;
    double rate;
    double rateUpdatedTime;
    KINETIC_LAW *waitingTime;
    double count;
} REACTION;

RET_VAL InitReactionNode( REACTION *reaction, char *name );


STRING *GetReactionNodeID( REACTION *reaction );
STRING *GetReactionNodeName( REACTION *reaction );
RET_VAL SetReactionNodeName( REACTION *reaction, STRING *name );

BOOL IsReactionReversibleInReactionNode( REACTION *reaction );
BOOL IsReactionFastInReactionNode( REACTION *reaction );
KINETIC_LAW *GetKineticLawInReactionNode( REACTION *reaction );

RET_VAL SetWaitingTimeInReactionNode( REACTION *reaction, KINETIC_LAW *waitingTime );
KINETIC_LAW *GetWaitingTimeInReactionNode( REACTION *reaction );


RET_VAL SetReactionReversibleInReactionNode( REACTION *reaction, BOOL isReversible );
RET_VAL SetReactionFastInReactionNode( REACTION *reaction, BOOL fast );
RET_VAL SetKineticLawInReactionNode( REACTION *reaction, KINETIC_LAW *kineticLaw );

#if 0
RET_VAL SetSymtabInReactionNode( REACTION *reaction, REB2SAC_SYMTAB *symtab );
REB2SAC_SYMTAB *GetSymtabInReactionNode( REACTION *reaction );

#endif


RET_VAL ReleaseResourcesInReactionNode( REACTION *reaction );

LINKED_LIST *GetReactantsInReactionNode( REACTION *reaction );
LINKED_LIST *GetModifiersInReactionNode( REACTION *reaction );
LINKED_LIST *GetProductsInReactionNode( REACTION *reaction );

double GetReactionRate(REACTION *reaction );
RET_VAL SetReactionRate(REACTION *reaction, double rate );

double GetReactionRateUpdatedTime(REACTION *reaction );
RET_VAL SetReactionRateUpdatedTime(REACTION *reaction, double time );

double GetReactionFireCount( REACTION *reaction );
RET_VAL IncrementReactionFireCount( REACTION *reaction );
RET_VAL ResetReactionFireCount( REACTION *reaction );

END_C_NAMESPACE

#endif
