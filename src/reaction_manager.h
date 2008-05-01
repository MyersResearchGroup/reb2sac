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
#if !defined(HAVE_REACTION_LAW_MANAGER)
#define HAVE_REACTION_LAW_MANAGER

#include "common.h"
#include "linked_list.h"
#include "hash_table.h"
#include "util.h"
#include "kinetic_law.h"

#include "unit_manager.h"

BEGIN_C_NAMESPACE

struct _REACTION_LAW;
typedef struct _REACTION_LAW  REACTION_LAW;

struct _REACTION_LAW {
    STRING *id;
    KINETIC_LAW *kineticLaw;
};


struct _REACTION_LAW_MANAGER;
typedef struct _REACTION_LAW_MANAGER  REACTION_LAW_MANAGER;

struct _REACTION_LAW_MANAGER {
    HASH_TABLE *table;
    COMPILER_RECORD_T *record;

    REACTION_LAW * (*CreateReactionLaw)( REACTION_LAW_MANAGER *manager, char *id );
    REACTION_LAW * (*LookupReactionLaw)( REACTION_LAW_MANAGER *manager,  char *id );
    LINKED_LIST *(*CreateListOfReactionLaws)( REACTION_LAW_MANAGER *manager );
};


REACTION_LAW_MANAGER *GetReactionLawManagerInstance( COMPILER_RECORD_T *record );
RET_VAL CloseReactionLawManager( );

STRING *GetReactionLawID( REACTION_LAW *reactionLaw );

KINETIC_LAW *GetKineticLawInReactionLaw( REACTION_LAW *reactionLaw );
RET_VAL SetKineticLawInReactionLaw( REACTION_LAW *reactionLaw, KINETIC_LAW *kineticLaw );

END_C_NAMESPACE

#endif
