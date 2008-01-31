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
#include "event_manager.h"
#include "linked_list.h"

static EVENT_MANAGER manager;


static EVENT * _CreateEvent( EVENT_MANAGER *manager, char *id );
static LINKED_LIST *_CreateListOfEvents( EVENT_MANAGER *manager );                  


EVENT_MANAGER *GetEventManagerInstance( COMPILER_RECORD_T *record ) {
    
    START_FUNCTION("GetEventManagerInstance");

    if( manager.record == NULL ) {
        manager.record = record;    
        if( ( manager.events = CreateLinkedList( ) ) == NULL ) {
            END_FUNCTION("GetEventManagerInstance", FAILING );
            return NULL;
        }    
        manager.CreateEvent = _CreateEvent;
        manager.CreateListOfEvents = _CreateListOfEvents;
    }
        
    END_FUNCTION("GetEventManagerInstance", SUCCESS );
    return &manager;
}

RET_VAL FreeEventAssignments( LINKED_LIST **eventAssignments ) {
    RET_VAL ret = SUCCESS;
    EVENT_ASSIGNMENT *eventAssignmentDef = NULL;    
    
    START_FUNCTION("FreeEventAssignments");
     
    while( ( eventAssignmentDef = (EVENT_ASSIGNMENT*)GetNextFromLinkedList( *eventAssignments ) ) != NULL ) {
        FreeString( &( eventAssignmentDef->var ) );
	FreeKineticLaw( &(eventAssignmentDef->assignment) );
        FREE( eventAssignmentDef );
    }
    DeleteLinkedList( eventAssignments );
    manager.record = NULL;       
    
    END_FUNCTION("FreeEventAssignments", SUCCESS );
    return ret;
}

RET_VAL CloseEventManager(  ) {
    RET_VAL ret = SUCCESS;
    EVENT *eventDef = NULL;    
    
    START_FUNCTION("CloseEventManager");
    
    if( manager.record == NULL ) {
        END_FUNCTION("CloseEventManager", SUCCESS );
        return ret;
    }
     
    while( ( eventDef = (EVENT*)GetNextFromLinkedList( manager.events ) ) != NULL ) {
        if (eventDef->id) FreeString( &( eventDef->id ) );
	FreeKineticLaw( &(eventDef->trigger) );
	FreeKineticLaw( &(eventDef->delay) );
        FreeEventAssignments( &( eventDef->eventAssignments ) );
        FREE( eventDef );
    }
    DeleteLinkedList( &(manager.events) );
    manager.record = NULL;       
    
    END_FUNCTION("CloseEventManager", SUCCESS );
    return ret;
}

STRING *GetEventId( EVENT *eventDef ) {
    START_FUNCTION("GetEventId");
            
    END_FUNCTION("GetEventId", SUCCESS );
    return (eventDef == NULL ? NULL : eventDef->id);
}

KINETIC_LAW *GetTriggerInEvent( EVENT *eventDef ) {
    START_FUNCTION("GetTriggerInEvent");
            
    END_FUNCTION("GetTriggerInEvent", SUCCESS );
    return (eventDef == NULL ? NULL : eventDef->trigger);
}

KINETIC_LAW *GetDelayInEvent( EVENT *eventDef ) {
    START_FUNCTION("GetDelayInEvent");
            
    END_FUNCTION("GetDelayInEvent", SUCCESS );
    return (eventDef == NULL ? NULL : eventDef->delay);
}

LINKED_LIST *GetEventAssignments( EVENT *eventDef ) {
    START_FUNCTION("GetEventAssignments");
            
    END_FUNCTION("GetEventAssignments", SUCCESS );
    return (eventDef == NULL ? NULL : eventDef->eventAssignments);
}

double GetNextEventTimeInEvent( EVENT *eventDef ) {
    START_FUNCTION("GetNextEventTimeInEvent");
            
    END_FUNCTION("GetNextEventTimeInEvent", SUCCESS );
    return eventDef->nextEventTime;
}

void SetNextEventTimeInEvent( EVENT *eventDef, double nextEventTime ) {
    START_FUNCTION("SetNextEventTimeInEvent");
  
    eventDef->nextEventTime = nextEventTime;
  
    END_FUNCTION("SetNextEventTimeInEvent", SUCCESS );
}

BOOL GetTriggerEnabledInEvent( EVENT *eventDef ) {
    START_FUNCTION("GetTriggerEnabledInEvent");
            
    END_FUNCTION("GetTriggerEnabledInEvent", SUCCESS );
    return eventDef->triggerEnabled;
}

void SetTriggerEnabledInEvent( EVENT *eventDef, BOOL triggerEnabled ) {
    START_FUNCTION("SetTriggerEnabledInEvent");
  
    eventDef->triggerEnabled = triggerEnabled;
  
    END_FUNCTION("GetTriggerEnabledInEvent", SUCCESS );
}

RET_VAL AddTriggerInEvent( EVENT *eventDef, KINETIC_LAW *trigger ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("AddTriggerInEvent");

    eventDef->trigger = trigger;

    END_FUNCTION("AddTriggerInEvent", SUCCESS );
    return ret;
}

RET_VAL AddDelayInEvent( EVENT *eventDef, KINETIC_LAW *delay ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("AddDelayInEvent");

    eventDef->delay = delay;

    END_FUNCTION("AddDelayInEvent", SUCCESS );
    return ret;
}

RET_VAL AddEventAssignmentToEvent( EVENT *eventDef, char *var, KINETIC_LAW *assignment ) {
    RET_VAL ret = SUCCESS;
    EVENT_ASSIGNMENT *eventAssignment;

    START_FUNCTION("AddEventAssignmentToEvent");

    if( ( eventAssignment = CreateEventAssignment( var, assignment ) ) == NULL ) {
        END_FUNCTION("_AddEventAssignmentToEvent", FAILING );
        return FAILING;
    }
    if( IS_FAILED( AddElementInLinkedList( (CADDR_T)eventAssignment, eventDef->eventAssignments ) ) ) {
        END_FUNCTION("_AddEventAssignmentToEvent", FAILING );
        return FAILING;
    }

    END_FUNCTION("AddEventAssignmentToEvent", SUCCESS );
    return ret;
}

EVENT_ASSIGNMENT *CreateEventAssignment( char *var, KINETIC_LAW *assignment ) {
    EVENT_ASSIGNMENT *eventAssignmentDef = NULL;
    
    START_FUNCTION("_CreateEventAssignment");

    if( ( eventAssignmentDef = (EVENT_ASSIGNMENT*)MALLOC( sizeof(EVENT_ASSIGNMENT) ) ) == NULL ) {
        END_FUNCTION("_CreateEventAssignement", FAILING );
        return NULL;
    }
    if (var == NULL) {
      eventAssignmentDef->var = NULL;
    } else if( ( eventAssignmentDef->var = CreateString( var ) ) == NULL ) {
        FREE( eventAssignmentDef );
        END_FUNCTION("_CreateEventAssignment", FAILING );
        return NULL;
    }
    eventAssignmentDef->assignment = assignment;
    
    END_FUNCTION("_CreateAssignmentEvent", SUCCESS );
    return eventAssignmentDef;
}

static EVENT * _CreateEvent( EVENT_MANAGER *manager, char *id ) {
    EVENT *eventDef = NULL;
    
    START_FUNCTION("_CreateEvent");

    if( ( eventDef = (EVENT*)MALLOC( sizeof(EVENT) ) ) == NULL ) {
        END_FUNCTION("_CreateEvent", FAILING );
        return NULL;
    }
    if (id == NULL) {
      eventDef->id = NULL;
    } else if( ( eventDef->id = CreateString( id ) ) == NULL ) {
        FREE( eventDef );
        END_FUNCTION("_CreateEvent", FAILING );
        return NULL;
    }
    eventDef->trigger = NULL;
    eventDef->delay = NULL;
    if( ( eventDef->eventAssignments = CreateLinkedList( ) ) == NULL ) {
      END_FUNCTION("_CreateEvent", FAILING );
      return NULL;
    }    
    eventDef->nextEventTime = -1.0;
    eventDef->triggerEnabled = FALSE;

    if( IS_FAILED( AddElementInLinkedList( (CADDR_T)eventDef, manager->events ) ) ) {
        FREE( eventDef );
        END_FUNCTION("_CreateEvent", FAILING );
        return NULL;
    } 
    
    END_FUNCTION("_CreateEvent", SUCCESS );
    return eventDef;
}
    
static LINKED_LIST *_CreateListOfEvents( EVENT_MANAGER *manager ) {
    
    START_FUNCTION("_CreateListOfEvents");
    
    END_FUNCTION("_CreateListOfEvents", SUCCESS );
    return manager->events;
}                  
    
