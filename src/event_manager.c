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
static RET_VAL _RemoveEvent( EVENT_MANAGER *manager, EVENT *event );
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
        manager.RemoveEvent = _RemoveEvent;
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
	FreeKineticLaw( &(eventDef->priority) );
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

KINETIC_LAW *GetPriorityInEvent( EVENT *eventDef ) {
    START_FUNCTION("GetPriorityInEvent");
            
    END_FUNCTION("GetPriorityInEvent", SUCCESS );
    return (eventDef == NULL ? NULL : eventDef->priority);
}

LINKED_LIST *GetEventAssignments( EVENT *eventDef ) {
    START_FUNCTION("GetEventAssignments");
            
    END_FUNCTION("GetEventAssignments", SUCCESS );
    return (eventDef == NULL ? NULL : eventDef->eventAssignments);
}

double GetNextEventTimeInEvent( EVENT *eventDef ) {
    double nextEventTime = -1;
    double *nextEventTimePtr;
    START_FUNCTION("GetNextEventTimeInEvent");

    ResetCurrentElement( eventDef->nextEventTime );
    while ( ( nextEventTimePtr = (double*)GetNextFromLinkedList( eventDef->nextEventTime ) ) != NULL ) {
      if ((nextEventTime == -1) || (nextEventTime > (*nextEventTimePtr))) { 
        nextEventTime = (*nextEventTimePtr);
      }
    } 

    END_FUNCTION("GetNextEventTimeInEvent", SUCCESS );
    return nextEventTime;
}

BYTE GetEventAssignmentVarType( EVENT_ASSIGNMENT *eventAssignDef ) {
    START_FUNCTION("GetEventAssignmentVarType");
            
    END_FUNCTION("GetEventAssignmentVarType", SUCCESS );
    return eventAssignDef->varType;
}

UINT32 GetEventAssignmentIndex( EVENT_ASSIGNMENT *eventAssignDef ) {
    START_FUNCTION("GetEventAssignmentIndex");
            
    END_FUNCTION("GetEventAssignmentIndex", SUCCESS );
    return eventAssignDef->index;
}

double GetEventAssignmentNextValue( EVENT_ASSIGNMENT *eventAssignDef ) {
    START_FUNCTION("GetEventAssignmentNextValue");
            
    END_FUNCTION("GetEventAssignmentNextValue", SUCCESS );
    return eventAssignDef->nextValue;
}

double GetEventAssignmentNextValueTime( EVENT_ASSIGNMENT *eventAssignDef, double nextTime ) {
    NEXT_VALUE_TIME *nextValueTimePtr = NULL;
    double nextValue = -1;

    START_FUNCTION("GetEventAssignmentNextValueTime");

    ResetCurrentElement( eventAssignDef->nextValueTime );
    while ( ( nextValueTimePtr = (NEXT_VALUE_TIME*)GetNextFromLinkedList( eventAssignDef->nextValueTime ) ) != NULL ){
      if (nextValueTimePtr->nextTime == nextTime) {
	nextValue = nextValueTimePtr->nextValue;
	RemoveElementFromLinkedList( (CADDR_T)nextValueTimePtr, eventAssignDef->nextValueTime );
	break;
      } 
    }

    END_FUNCTION("GetEventAssignmentNextValueTime", SUCCESS );
    return nextValue;
}

void SetNextEventTimeInEvent( EVENT *eventDef, double nextEventTime ) {
    double *nextEventTimePtr = NULL;
    double *nextEventTimePtrRm = NULL;

    START_FUNCTION("SetNextEventTimeInEvent");

    if (nextEventTime == -1) {
      ResetCurrentElement( eventDef->nextEventTime );
      while ( ( nextEventTimePtr = (double*)GetNextFromLinkedList( eventDef->nextEventTime ) ) != NULL ) {
	if ((nextEventTime == -1) || (nextEventTime > (*nextEventTimePtr))) { 
	  nextEventTime = (*nextEventTimePtr);
	  nextEventTimePtrRm = nextEventTimePtr;
	}
      } 
      if (nextEventTimePtrRm != NULL) {
	RemoveElementFromLinkedList( (CADDR_T)nextEventTimePtrRm, eventDef->nextEventTime );
      }
    } else {
      if( ( nextEventTimePtr = (double*)MALLOC( sizeof(double) ) ) == NULL ) {
        END_FUNCTION("_SetNextEventTimeInEvent", FAILING );
        return;
      }
      (*nextEventTimePtr) = nextEventTime;
      AddElementInLinkedList((CADDR_T)nextEventTimePtr,eventDef->nextEventTime);
    }

    END_FUNCTION("SetNextEventTimeInEvent", SUCCESS );
}

RET_VAL SetEventAssignmentVarType( EVENT_ASSIGNMENT *eventAssignDef, BYTE varType ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("SetEventAssignmentVarType");
    eventAssignDef->varType = varType;

    END_FUNCTION("SetEventAssignmentVarType", SUCCESS );
    return ret;
}

RET_VAL SetEventAssignmentIndex( EVENT_ASSIGNMENT *eventAssignDef, UINT32 index ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("SetEventAssignmentIndex");

    eventAssignDef->index = index;

    END_FUNCTION("SetEventAssignmentIndex", SUCCESS );
    return ret;
}

RET_VAL SetEventAssignmentNextValue( EVENT_ASSIGNMENT *eventAssignDef, double nextValue ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("SetEventAssignmentNextValue");

    eventAssignDef->nextValue = nextValue;

    END_FUNCTION("SetEventAssignmentNextValue", SUCCESS );
    return ret;
}

RET_VAL SetEventAssignmentNextValueTime( EVENT_ASSIGNMENT *eventAssignDef, double nextValue, double nextTime ) {
    RET_VAL ret = SUCCESS;
    NEXT_VALUE_TIME *nextValueTime;

    START_FUNCTION("SetEventAssignmentNextValueTime");

    if( ( nextValueTime = CreateNextValueTime( nextValue, nextTime ) ) == NULL ) {
        END_FUNCTION("_SetEventAssignmentNextValueTime", FAILING );
        return FAILING;
    }
    if( IS_FAILED( AddElementInLinkedList( (CADDR_T)nextValueTime, eventAssignDef->nextValueTime ) ) ) {
        END_FUNCTION("_SetEventAssignmentNextValueTime", FAILING );
        return FAILING;
    }

    END_FUNCTION("SetEventAssignmentNextValueTime", SUCCESS );
    return ret;
}

BOOL GetUseValuesFromTriggerTime( EVENT *eventDef ) {
    START_FUNCTION("GetUseValuesFromTriggerTime");
            
    END_FUNCTION("GetUseValuesFromTriggerTime", SUCCESS );
    return eventDef->useValuesFromTriggerTime;
}

BOOL GetTriggerCanBeDisabled( EVENT *eventDef ) {
    START_FUNCTION("GetTriggerCanBeDisabled");
            
    END_FUNCTION("GetTriggerCanBeDisabled", SUCCESS );
    return eventDef->TriggerCanBeDisabled;
}

BOOL GetTriggerInitialValue( EVENT *eventDef ) {
    START_FUNCTION("GetTriggerInitialValue");
            
    END_FUNCTION("GetTriggerInitialValue", SUCCESS );
    return eventDef->TriggerInitialValue;
}

BOOL GetTriggerEnabledInEvent( EVENT *eventDef ) {
    START_FUNCTION("GetTriggerEnabledInEvent");
            
    END_FUNCTION("GetTriggerEnabledInEvent", SUCCESS );
    return eventDef->triggerEnabled;
}

void SetUseValuesFromTriggerTime( EVENT *eventDef, BOOL useValuesFromTriggerTime ) {
    START_FUNCTION("SetUseValuesFromTriggerTime");
  
    eventDef->useValuesFromTriggerTime = useValuesFromTriggerTime;
  
    END_FUNCTION("SetUseValuesFromTriggerTime", SUCCESS );
}

void SetTriggerCanBeDisabled( EVENT *eventDef, BOOL TriggerCanBeDisabled ) {
    START_FUNCTION("SetTriggerCanBeDisabled");
  
    eventDef->TriggerCanBeDisabled = TriggerCanBeDisabled;
  
    END_FUNCTION("SetTriggerCanBeDisabled", SUCCESS );
}

void SetTriggerInitialValue( EVENT *eventDef, BOOL TriggerInitialValue ) {
    START_FUNCTION("SetTriggerInitialValue");
  
    eventDef->TriggerInitialValue = TriggerInitialValue;
  
    END_FUNCTION("SetTriggerInitialValue", SUCCESS );
}

void SetTriggerEnabledInEvent( EVENT *eventDef, BOOL triggerEnabled ) {
    START_FUNCTION("SetTriggerEnabledInEvent");
  
    eventDef->triggerEnabled = triggerEnabled;
  
    END_FUNCTION("SetTriggerEnabledInEvent", SUCCESS );
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

RET_VAL AddPriorityInEvent( EVENT *eventDef, KINETIC_LAW *priority ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("AddPriorityInEvent");

    eventDef->priority = priority;

    END_FUNCTION("AddPriorityInEvent", SUCCESS );
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

RET_VAL RemoveEventAssignmentFromEvent( EVENT *eventDef, EVENT_ASSIGNMENT *eventAssignment ) {
    RET_VAL ret = SUCCESS;

    START_FUNCTION("RemoveEventAssignmentToEvent");

    if( IS_FAILED( RemoveElementFromLinkedList( (CADDR_T)eventAssignment, eventDef->eventAssignments ) ) ) {
        END_FUNCTION("_RemoveEventAssignmentToEvent", FAILING );
        return FAILING;
    }

    END_FUNCTION("RemoveEventAssignmentToEvent", SUCCESS );
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
    eventAssignmentDef->nextValue = 0.0;
    if( ( eventAssignmentDef->nextValueTime = CreateLinkedList( ) ) == NULL ) {
      END_FUNCTION("_CreateEventAssignment", FAILING );
      return NULL;
    }    

    END_FUNCTION("_CreateAssignmentEvent", SUCCESS );
    return eventAssignmentDef;
}

NEXT_VALUE_TIME *CreateNextValueTime( double nextValue, double nextTime ) {
    NEXT_VALUE_TIME *nextValueTimeDef = NULL;
    
    START_FUNCTION("_CreateNextValueTime");

    if( ( nextValueTimeDef = (NEXT_VALUE_TIME*)MALLOC( sizeof(NEXT_VALUE_TIME) ) ) == NULL ) {
        END_FUNCTION("_CreateNextValueTime", FAILING );
        return NULL;
    }
    nextValueTimeDef->nextValue = nextValue;
    nextValueTimeDef->nextTime = nextTime;

    END_FUNCTION("_CreateNextValueTime", SUCCESS );
    return nextValueTimeDef;
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
    eventDef->priority = NULL;

    if( ( eventDef->eventAssignments = CreateLinkedList( ) ) == NULL ) {
      END_FUNCTION("_CreateEvent", FAILING );
      return NULL;
    }    

    if( ( eventDef->nextEventTime = CreateLinkedList( ) ) == NULL ) {
      END_FUNCTION("_CreateEvent", FAILING );
      return NULL;
    }    

    eventDef->triggerEnabled = FALSE;

    if( IS_FAILED( AddElementInLinkedList( (CADDR_T)eventDef, manager->events ) ) ) {
        FREE( eventDef );
        END_FUNCTION("_CreateEvent", FAILING );
        return NULL;
    } 
    
    END_FUNCTION("_CreateEvent", SUCCESS );
    return eventDef;
}

static RET_VAL _RemoveEvent( EVENT_MANAGER *manager, EVENT *eventDef ) {
  RET_VAL ret = SUCCESS;
    if( IS_FAILED( RemoveElementFromLinkedList( (CADDR_T)eventDef, manager->events ) ) ) {
      return FAILING;
    } 
    return ret;
}
    
static LINKED_LIST *_CreateListOfEvents( EVENT_MANAGER *manager ) {
    
    START_FUNCTION("_CreateListOfEvents");
    
    END_FUNCTION("_CreateListOfEvents", SUCCESS );
    return manager->events;
}                  
    
