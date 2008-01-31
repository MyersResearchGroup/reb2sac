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
#if !defined(HAVE_EVENT_MANAGER)
#define HAVE_EVENT_MANAGER

#include "common.h"
#include "linked_list.h"
#include "util.h"

#include "compiler_def.h"
#include "kinetic_law.h"

BEGIN_C_NAMESPACE

typedef struct {
    STRING *id;
    KINETIC_LAW *trigger;
    KINETIC_LAW *delay;
    LINKED_LIST *eventAssignments;
    double nextEventTime;
    BOOL triggerEnabled;
} EVENT;

typedef struct {
    STRING *var;
    KINETIC_LAW *assignment;
} EVENT_ASSIGNMENT;

struct _EVENT_MANAGER;
typedef struct _EVENT_MANAGER EVENT_MANAGER;

struct _EVENT_MANAGER {
    LINKED_LIST *events;   
    COMPILER_RECORD_T *record;
    
    EVENT * (*CreateEvent)( EVENT_MANAGER *manager, char *id );
    LINKED_LIST *(*CreateListOfEvents)( EVENT_MANAGER *manager );                  
};

STRING *GetEventId( EVENT *eventDef );
KINETIC_LAW *GetTriggerInEvent( EVENT *eventDef );
KINETIC_LAW *GetDelayInEvent( EVENT *eventDef );
LINKED_LIST *GetEventAssignments( EVENT *eventDef );
double GetNextEventTimeInEvent( EVENT *eventDef );
BOOL GetTriggerEnabledInEvent( EVENT *eventDef );
RET_VAL AddTriggerInEvent( EVENT *eventDef, KINETIC_LAW *trigger );
RET_VAL AddDelayInEvent( EVENT *eventDef, KINETIC_LAW *delay );
RET_VAL AddEventAssignmentToEvent( EVENT *eventDef, char *var, KINETIC_LAW *assignment );
EVENT_ASSIGNMENT *CreateEventAssignment( char *var, KINETIC_LAW *assignment );
void SetNextEventTimeInEvent( EVENT *eventDef, double nextEventTime );
void SetTriggerEnabledInEvent( EVENT *eventDef, BOOL triggerEnabled );

EVENT_MANAGER *GetEventManagerInstance( COMPILER_RECORD_T *record );
RET_VAL CloseEventManager( );

END_C_NAMESPACE

#endif
