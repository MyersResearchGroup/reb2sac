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

#include "reaction_node.h"

static IR_NODE *_Clone( IR_NODE *reaction );    
static char * _GetType(  );                                                                              
static RET_VAL _ReleaseResource( IR_NODE *reaction );


RET_VAL InitReactionNode( REACTION *reaction, char *name ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("InitReactionNode");

    if( IS_FAILED( InitIRNode( (IR_NODE*)reaction, name ) ) ) {
        END_FUNCTION("InitReactionNode", FAILING );
        return FAILING;
    }

    reaction->Clone = _Clone;
    reaction->GetType = _GetType;
    reaction->ReleaseResource = _ReleaseResource;
    reaction->count = 0;
    reaction->compartment = NULL;
    
    END_FUNCTION("InitReactionNode", SUCCESS );
    return ret;    
}



STRING *GetReactionNodeID( REACTION *reaction ) {
    START_FUNCTION("GetReactionNodeID");
    if( reaction == NULL ) {
        END_FUNCTION("GetReactionNodeID", FAILING );
        return NULL;
    }
    END_FUNCTION("GetReactionNodeID", SUCCESS );

#ifdef NAME_FOR_ID
    return reaction->name;
#else 
    return reaction->id;
#endif
}


STRING *GetReactionNodeName( REACTION *reaction ) {
    START_FUNCTION("GetReactionNodeName");
    if( reaction == NULL ) {
        END_FUNCTION("GetReactionNodeName", FAILING );
        return NULL;
    }
    END_FUNCTION("GetReactionNodeName", SUCCESS );
    return reaction->name;
}

RET_VAL SetReactionNodeName( REACTION *reaction, STRING *name ) {
   RET_VAL ret = SUCCESS;
    
    START_FUNCTION("SetReactionNodeName");
    if( reaction == NULL ) {
        return ErrorReport( FAILING, "SetReactionNodeName", "input reaction node is NULL" );
    }
    FreeString( &(reaction->name) );
    reaction->name = name;
    END_FUNCTION("SetReactionNodeName", SUCCESS );
    return ret;
}

STRING *GetReactionNodeCompartment( REACTION *reaction ) {
    START_FUNCTION("GetReactionNodeCompartment");
    if( reaction == NULL ) {
        END_FUNCTION("GetReactionNodeCompartment", FAILING );
        return NULL;
    }
    END_FUNCTION("GetReactionNodeCompartment", SUCCESS );
    return reaction->compartment;
}

RET_VAL SetReactionNodeCompartment( REACTION *reaction, char *compartment ) {
   RET_VAL ret = SUCCESS;
   STRING *compartmentStr = NULL;
    
    START_FUNCTION("SetReactionNodeCompartment");
    if( reaction == NULL ) {
        return ErrorReport( FAILING, "SetReactionNodeCompartment", "input reaction node is NULL" );
    }
    FreeString( &(reaction->compartment) );
    if( ( compartmentStr = CreateString( compartment ) ) == NULL ) {
        END_FUNCTION("SetReactionNodeCompartment", FAILING );
        return FAILING;
    }
    reaction->compartment = compartmentStr;
    END_FUNCTION("SetReactionNodeCompartment", SUCCESS );
    return ret;
}


BOOL IsReactionReversibleInReactionNode( REACTION *reaction ) {
    START_FUNCTION("IsReactionReversibleInReactionNode");
    if( reaction == NULL ) {
        END_FUNCTION("IsReactionReversibleInReactionNode", FAILING );
        return FALSE;
    }
    END_FUNCTION("IsReactionReversibleInReactionNode", SUCCESS );
    return reaction->isReversible;
}

BOOL IsReactionFastInReactionNode( REACTION *reaction ) {
    START_FUNCTION("IsReactionFastInReactionNode");
    if( reaction == NULL ) {
        END_FUNCTION("IsReactionFastInReactionNode", FAILING );
        return FALSE;
    }
    END_FUNCTION("IsReactionFastInReactionNode", SUCCESS );
    return reaction->fast;
}


KINETIC_LAW *GetKineticLawInReactionNode( REACTION *reaction ) {
    START_FUNCTION("GetKineticLawInReactionNode");
    if( reaction == NULL ) {
        END_FUNCTION("GetKineticLawInReactionNode", FAILING );
        return NULL;
    }
    END_FUNCTION("GetKineticLawInReactionNode", SUCCESS );
    return reaction->kineticLaw;
}

#if 0
REB2SAC_SYMTAB *GetSymtabInReactionNode( REACTION *reaction ) {
    START_FUNCTION("GetSymtabInReactionNode");
    if( reaction == NULL ) {
        END_FUNCTION("GetSymtabInReactionNode", FAILING );
        return NULL;
    }
    END_FUNCTION("GetSymtabInReactionNode", SUCCESS );
    return reaction->symtab;
}
#endif



RET_VAL SetReactionReversibleInReactionNode( REACTION *reaction, BOOL isReversible ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("SetReactionReversibleInReactionNode");
    if( reaction == NULL ) {
        return ErrorReport( FAILING, "SetReactionReversibleInReactionNode", "input reaction node is NULL" );
    }
    reaction->isReversible = isReversible;
    END_FUNCTION("SetReactionReversibleInReactionNode", SUCCESS );
    return ret;
}

RET_VAL SetReactionFastInReactionNode( REACTION *reaction, BOOL fast ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("SetReactionFastInReactionNode");
    if( reaction == NULL ) {
        return ErrorReport( FAILING, "SetReactionFastInReactionNode", "input reaction node is NULL" );
    }
    reaction->fast = fast;
    END_FUNCTION("SetReactionFastInReactionNode", SUCCESS );
    return ret;
}

RET_VAL SetKineticLawInReactionNode( REACTION *reaction, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("SetKineticLawInReactionNode");
    if( reaction == NULL ) {
        return ErrorReport( FAILING, "SetKineticLawInReactionNode", "input reaction node is NULL" );
    }
    reaction->kineticLaw = kineticLaw;
    END_FUNCTION("SetKineticLawInReactionNode", SUCCESS );
    return ret;
}

RET_VAL SetReactionRate(REACTION *reaction, double rate ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("SetReactionRate");
    if( reaction == NULL ) {
        return ErrorReport( FAILING, "SetReactionRate", "input reaction node is NULL" );
    }
    reaction->rate = rate;
    END_FUNCTION("SetReactionRate", SUCCESS );
    return ret;
}

RET_VAL SetReactionRateUpdatedTime(REACTION *reaction, double time ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("SetReactionRateUpdatedTime");
    if( reaction == NULL ) {
        return ErrorReport( FAILING, "SetReactionRateUpdatedTime", "input reaction node is NULL" );
    }
    reaction->rateUpdatedTime = time;
    END_FUNCTION("SetReactionRateUpdatedTime", SUCCESS );
    return ret;
}


RET_VAL SetWaitingTimeInReactionNode( REACTION *reaction, KINETIC_LAW *waitingTime ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("SetWaitingTimeInReactionNode");
    if( reaction == NULL ) {
        return ErrorReport( FAILING, "SetWaitingTimeInReactionNode", "input reaction node is NULL" );
    }
    reaction->waitingTime = waitingTime;
    END_FUNCTION("SetWaitingTimeInReactionNode", SUCCESS );
    return ret;
}

KINETIC_LAW *GetWaitingTimeInReactionNode( REACTION *reaction ) {
    START_FUNCTION("GetWaitingTimeInReactionNode");
    if( reaction == NULL ) {
        END_FUNCTION("GetWaitingTimeInReactionNode", FAILING );
        return NULL;
    }
    END_FUNCTION("GetWaitingTimeInReactionNode", SUCCESS );
    return reaction->waitingTime;
}


#if 0
RET_VAL SetSymtabInReactionNode( REACTION *reaction, REB2SAC_SYMTAB *symtab ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("SetSymtabInReactionNode");
    if( reaction == NULL ) {
        return ErrorReport( FAILING, "SetSymtabInReactionNode", "input reaction node is NULL" );
    }
    reaction->symtab = symtab;
    END_FUNCTION("SetSymtabInReactionNode", SUCCESS );
    return ret;
}
#endif

RET_VAL ReleaseResourcesInReactionNode( REACTION *reaction ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("ReleaseResourcesInReactionNode");
    if( reaction == NULL ) {
        return ErrorReport( FAILING, "ReleaseResourcesInReactionNode", "input reaction node is NULL" );
    }
    if( IS_FAILED( ( ret = ReleaseIRNodeResources( (IR_NODE*)reaction ) ) ) ) {
        END_FUNCTION("ReleaseResourcesInSpeciesNode", ret );
        return ret;
    }
    FreeKineticLaw( &(reaction->kineticLaw) );
    END_FUNCTION("ReleaseResourcesInReactionNode", SUCCESS );
    return ret;
}


LINKED_LIST *GetReactantsInReactionNode( REACTION *reaction ) {
    START_FUNCTION("GetReactantsInReactionNode");
    if( reaction == NULL ) {
        END_FUNCTION("GetReactantsInReactionNode", FAILING );
        return NULL;
    }
    END_FUNCTION("GetReactantsInReactionNode", SUCCESS );
    return reaction->reactants;
}

LINKED_LIST *GetModifiersInReactionNode( REACTION *reaction ) {
    START_FUNCTION("GetModifiersInReactionNode");
    if( reaction == NULL ) {
        END_FUNCTION("GetModifiersInReactionNode", FAILING );
        return NULL;
    }
    END_FUNCTION("GetModifiersInReactionNode", SUCCESS );
    return reaction->modifiers;
}

LINKED_LIST *GetProductsInReactionNode( REACTION *reaction ) {
    START_FUNCTION("GetProductsInReactionNode");
    if( reaction == NULL ) {
        END_FUNCTION("GetProductsInReactionNode", FAILING );
        return NULL;
    }
    END_FUNCTION("GetProductsInReactionNode", SUCCESS );
    return reaction->products;
}

double GetReactionRate(REACTION *reaction ) {
    START_FUNCTION("GetReactionRate");
    if( reaction == NULL ) {
        END_FUNCTION("GetReactionRate", FAILING );
        return 1.0 / 0.0;
    }
    END_FUNCTION("GetReactionRate", SUCCESS );
    return reaction->rate;
}

double GetReactionRateUpdatedTime(REACTION *reaction ) {
    START_FUNCTION("GetReactionRateUpdatedTime");
    if( reaction == NULL ) {
        END_FUNCTION("GetReactionRateUpdatedTime", FAILING );
        return 1.0 / 0.0;
    }
    END_FUNCTION("GetReactionRateUpdatedTime", SUCCESS );
    return reaction->rateUpdatedTime;
}


double GetReactionFireCount( REACTION *reaction ) {
    START_FUNCTION("GetReactionFireCount");
    if( reaction == NULL ) {
        END_FUNCTION("GetReactionFireCount", FAILING );
        return 0.0;
    }
    END_FUNCTION("GetReactionFireCount", SUCCESS );
    return reaction->count;
}


RET_VAL IncrementReactionFireCount( REACTION *reaction ) {
    RET_VAL ret = SUCCESS;
    START_FUNCTION("IncrementReactionFireCount");
    if( reaction == NULL ) {
        return ErrorReport( FAILING, "IncrementReactionFireCount", "input reaction node is NULL" );
    }
    reaction->count = reaction->count + 1;
    END_FUNCTION("IncrementReactionFireCount", SUCCESS );
    return ret;
}

RET_VAL ResetReactionFireCount( REACTION *reaction ) {
    RET_VAL ret = SUCCESS;
    START_FUNCTION("ResetReactionFireCount");
    if( reaction == NULL ) {
        return ErrorReport( FAILING, "ResetReactionFireCount", "input reaction node is NULL" );
    }
    reaction->count = 0;
    END_FUNCTION("ResetReactionFireCount", SUCCESS );
    return ret;
}



static char * _GetType( ) {
    START_FUNCTION("_GetType");
    END_FUNCTION("_GetType", SUCCESS );    
    return "reaction";
}


static IR_NODE* _Clone( IR_NODE *reaction ) {
    REACTION *clone = NULL;
    
    START_FUNCTION("_Clone");

    if( ( clone = (REACTION*)MALLOC(  sizeof( REACTION ) ) ) == NULL ) {
        END_FUNCTION("_Clone", FAILING );
        return NULL;
    }
    
    if( IS_FAILED( CopyIRNode( (IR_NODE*)reaction, (IR_NODE*)clone ) ) ) {
        END_FUNCTION("_Clone", FAILING );
        return NULL;
    }
    
    clone->isReversible = ((REACTION*)reaction)->isReversible;
    clone->fast = ((REACTION*)reaction)->fast;
    if( ( clone->kineticLaw = CloneKineticLaw( ((REACTION*)reaction)->kineticLaw ) ) == NULL ) {
        END_FUNCTION("_Clone", FAILING );
        return NULL;
    }
    
    END_FUNCTION("_Clone", SUCCESS );    
    return (IR_NODE*)clone;
}


static RET_VAL _ReleaseResource( IR_NODE *reaction ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("_ReleaseResource");
    if( reaction == NULL ) {
        return ErrorReport( FAILING, "_ReleaseResource", "input reaction node is NULL" );
    }
    if( IS_FAILED( ( ret = ReleaseIRNodeResources( (IR_NODE*)reaction ) ) ) ) {
        END_FUNCTION("_ReleaseResource", ret );
        return ret;
    }
    FreeKineticLaw( &(((REACTION*)reaction)->kineticLaw) );
    END_FUNCTION("_ReleaseResource", SUCCESS );
    return ret;
}


