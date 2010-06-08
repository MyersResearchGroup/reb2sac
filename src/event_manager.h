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

#define SPECIES_EVENT_ASSIGNMENT ((BYTE)1)
#define COMPARTMENT_EVENT_ASSIGNMENT ((BYTE)2)
#define PARAMETER_EVENT_ASSIGNMENT ((BYTE)3)   

typedef struct {
    STRING *id;
    KINETIC_LAW *trigger;
    KINETIC_LAW *delay;
    KINETIC_LAW *priority;
    LINKED_LIST *eventAssignments;
    LINKED_LIST *nextEventTime;
    BOOL useValuesFromTriggerTime;
    BOOL TriggerCanBeDisabled;
    BOOL triggerEnabled;
} EVENT;

typedef struct {
    STRING *var;
    KINETIC_LAW *assignment;
    double nextValue;
    LINKED_LIST *nextValueTime;
    BYTE varType;
    UINT32 index;
} EVENT_ASSIGNMENT;

typedef struct {
  double nextValue;
  double nextTime;
} NEXT_VALUE_TIME;

struct _EVENT_MANAGER;
typedef struct _EVENT_MANAGER EVENT_MANAGER;

struct _EVENT_MANAGER {
    LINKED_LIST *events;   
    COMPILER_RECORD_T *record;
    
    EVENT * (*CreateEvent)( EVENT_MANAGER *manager, char *id );
    RET_VAL (*RemoveEvent)( EVENT_MANAGER *manager, EVENT *eventDef );
    LINKED_LIST *(*CreateListOfEvents)( EVENT_MANAGER *manager );                  
};

STRING *GetEventId( EVENT *eventDef );
KINETIC_LAW *GetTriggerInEvent( EVENT *eventDef );
KINETIC_LAW *GetDelayInEvent( EVENT *eventDef );
KINETIC_LAW *GetPriorityInEvent( EVENT *eventDef );
LINKED_LIST *GetEventAssignments( EVENT *eventDef );
double GetNextEventTimeInEvent( EVENT *eventDef );
BOOL GetUseValuesFromTriggerTime( EVENT *eventDef );
BOOL GetTriggerCanBeDisabled( EVENT *eventDef );
BOOL GetTriggerEnabledInEvent( EVENT *eventDef );
RET_VAL AddTriggerInEvent( EVENT *eventDef, KINETIC_LAW *trigger );
RET_VAL AddDelayInEvent( EVENT *eventDef, KINETIC_LAW *delay );
RET_VAL AddPriorityInEvent( EVENT *eventDef, KINETIC_LAW *priority );
RET_VAL AddEventAssignmentToEvent( EVENT *eventDef, char *var, KINETIC_LAW *assignment );
RET_VAL RemoveEventAssignmentFromEvent( EVENT *eventDef, EVENT_ASSIGNMENT *eventAssignDef );
EVENT_ASSIGNMENT *CreateEventAssignment( char *var, KINETIC_LAW *assignment );
NEXT_VALUE_TIME *CreateNextValueTime( double nextValue, double nextTime );
void SetNextEventTimeInEvent( EVENT *eventDef, double nextEventTime );
void SetUseValuesFromTriggerTime( EVENT *eventDef, BOOL useValuesFromTriggerTime );
void SetTriggerCanBeDisabled( EVENT *eventDef, BOOL disableTrigger );
void SetTriggerEnabledInEvent( EVENT *eventDef, BOOL triggerEnabled );

BYTE GetEventAssignmentVarType( EVENT_ASSIGNMENT *eventAssignDef );
UINT32 GetEventAssignmentIndex( EVENT_ASSIGNMENT *eventAssignDef );
double GetEventAssignmentNextValue( EVENT_ASSIGNMENT *eventAssignDef );
double GetEventAssignmentNextValueTime( EVENT_ASSIGNMENT *eventAssignDef, double time );
RET_VAL SetEventAssignmentVarType( EVENT_ASSIGNMENT *eventAssignDef, BYTE varType );
RET_VAL SetEventAssignmentIndex( EVENT_ASSIGNMENT *eventAssignDef, UINT32 index );
RET_VAL SetEventAssignmentNextValue( EVENT_ASSIGNMENT *eventAssignDef, double nextValue );
RET_VAL SetEventAssignmentNextValueTime( EVENT_ASSIGNMENT *eventAssignDef, double nextValue, double nextTime );

EVENT_MANAGER *GetEventManagerInstance( COMPILER_RECORD_T *record );
RET_VAL CloseEventManager( );

END_C_NAMESPACE

#endif
