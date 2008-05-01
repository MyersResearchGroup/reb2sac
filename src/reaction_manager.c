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
#include "reaction_manager.h"


static REACTION_LAW_MANAGER manager;


static REACTION_LAW *_CreateReactionLaw( REACTION_LAW_MANAGER *manager, char *id );
static REACTION_LAW *_LookupReactionLaw( REACTION_LAW_MANAGER *manager,  char *id );
static LINKED_LIST *_CreateListOfReactionLaws( REACTION_LAW_MANAGER *manager );


REACTION_LAW_MANAGER *GetReactionLawManagerInstance( COMPILER_RECORD_T *record ) {
    
    START_FUNCTION("GetReactionLawManagerInstance");

    if( manager.record == NULL ) {
        manager.record = record;    
        if( ( manager.table = CreateHashTable( 128 ) ) == NULL ) {
            END_FUNCTION("GetReactionLawManagerInstance", FAILING );
            return NULL;
        }    
        manager.CreateReactionLaw = _CreateReactionLaw;
        manager.LookupReactionLaw = _LookupReactionLaw;
        manager.CreateListOfReactionLaws = _CreateListOfReactionLaws;
    }
        
    END_FUNCTION("GetReactionLawManagerInstance", SUCCESS );
    return &manager;
}


RET_VAL CloseReactionLawManager(  ) {
    RET_VAL ret = SUCCESS;
    LINKED_LIST *list = NULL;
    REACTION_LAW *reactionLaw = NULL;
    
    START_FUNCTION("CloseReactionLawManager");
    
    if( manager.record == NULL ) {
        END_FUNCTION("CloseReactionLawManager", SUCCESS );
        return ret;
    }
    
    
    if( ( list = GenerateValueList( manager.table ) ) == NULL ) {
        END_FUNCTION("CloseReactionLawManager", FAILING );
        return FAILING;
    }
    
    while( ( reactionLaw = (REACTION_LAW*)GetNextFromLinkedList( list ) ) != NULL ) {
        FreeString( &( reactionLaw->id ) );
        FREE( reactionLaw );
    }
    DeleteHashTable( (HASH_TABLE**)&(manager.table) );    
    
    manager.record = NULL;
    
        
    END_FUNCTION("CloseReactionLawManager", SUCCESS );
    return ret;
}

STRING *GetReactionLawID( REACTION_LAW *reactionLaw ) {
    
    START_FUNCTION("GetReactionLawID");
    
    if( reactionLaw == NULL ) {
        END_FUNCTION("GetReactionLawID", FAILING );
        return NULL;
    }
    
    END_FUNCTION("GetReactionLawID", SUCCESS );
    return reactionLaw->id;
}

KINETIC_LAW *GetKineticLawInReactionLaw( REACTION_LAW *reactionLaw ) {
    START_FUNCTION("GetKineticLawInReactionLaw");
    
    if( reactionLaw == NULL ) {
        END_FUNCTION("GetKineticLawInReactionLaw", FAILING );
        return NULL;
    }
    
    END_FUNCTION("GetKineticLawInReactionLaw", SUCCESS );
    return reactionLaw->kineticLaw;
}


RET_VAL SetKineticLawInReactionLaw( REACTION_LAW *reactionLaw, KINETIC_LAW *kineticLaw ) {
    START_FUNCTION("SetKineticLawInReactionLaw");
    
    if( reactionLaw == NULL ) {
        END_FUNCTION("SetKineticLawInReactionLaw", FAILING );
        return FAILING;
    }
    
    reactionLaw->kineticLaw = kineticLaw;
    END_FUNCTION("SetKineticLawInReactionLaw", SUCCESS );
    return SUCCESS;
}

static REACTION_LAW *_CreateReactionLaw( REACTION_LAW_MANAGER *manager, char *id ) {
    REACTION_LAW *reactionLaw = NULL;
    
    START_FUNCTION("_CreateReactionLaw");

    if( ( reactionLaw = (REACTION_LAW*)CALLOC( 1, sizeof(REACTION_LAW) ) ) == NULL ) {
        END_FUNCTION("_CreateReactionLaw", FAILING );
        return NULL;
    }
    if( ( reactionLaw->id = CreateString( id ) ) == NULL ) {
        FREE( reactionLaw );
        END_FUNCTION("_CreateReactionLaw", FAILING );
        return NULL;
    }
    if( IS_FAILED( PutInHashTable( GetCharArrayOfString( reactionLaw->id ), GetStringLength( reactionLaw->id ), reactionLaw, manager->table ) ) ) {
        FREE( reactionLaw );
        END_FUNCTION("_CreateReactionLaw", FAILING );
        return NULL;
    } 
    
    END_FUNCTION("_CreateReactionLaw", SUCCESS );
    return reactionLaw;
}

static REACTION_LAW *_LookupReactionLaw( REACTION_LAW_MANAGER *manager,  char *id ) {
    REACTION_LAW *reactionLaw = NULL;
    
    START_FUNCTION("_LookupReactionLaw");
    
    if( ( reactionLaw = (REACTION_LAW*)GetValueFromHashTable( id, strlen(id), manager->table ) ) == NULL ) {
        END_FUNCTION("_LookupReactionLaw", FAILING );
        return NULL;
    }
    
    END_FUNCTION("_LookupReactionLaw", SUCCESS );
    return reactionLaw;
}

static LINKED_LIST *_CreateListOfReactionLaws( REACTION_LAW_MANAGER *manager ) {
    LINKED_LIST *list = NULL;
    
    START_FUNCTION("_CreateListOfReactionLaw");

    if( ( list = GenerateValueList( manager->table ) ) == NULL ) {
        END_FUNCTION("_CreateListOfReactionLaw", FAILING );
        return NULL;
    }
    
    END_FUNCTION("_CreateListOfReactionLaw", SUCCESS );
    return list;
}
    

