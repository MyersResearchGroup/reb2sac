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
#include "compartment_manager.h"


static COMPARTMENT_MANAGER manager;


static COMPARTMENT *_CreateCompartment( COMPARTMENT_MANAGER *manager, char *id );
static COMPARTMENT *_LookupCompartment( COMPARTMENT_MANAGER *manager,  char *id );
static RET_VAL _ResolveCompartmentLinks( COMPARTMENT_MANAGER *manager );
static LINKED_LIST *_CreateListOfCompartments( COMPARTMENT_MANAGER *manager );


COMPARTMENT_MANAGER *GetCompartmentManagerInstance( COMPILER_RECORD_T *record ) {
    
    START_FUNCTION("GetCompartmentManagerInstance");

    if( manager.record == NULL ) {
        manager.record = record;    
        if( ( manager.table = CreateHashTable( 128 ) ) == NULL ) {
            END_FUNCTION("GetCompartmentManagerInstance", FAILING );
            return NULL;
        }    
        manager.CreateCompartment = _CreateCompartment;
        manager.LookupCompartment = _LookupCompartment;
        manager.ResolveCompartmentLinks = _ResolveCompartmentLinks;
        manager.CreateListOfCompartments = _CreateListOfCompartments;
    }
        
    END_FUNCTION("GetCompartmentManagerInstance", SUCCESS );
    return &manager;
}


RET_VAL CloseCompartmentManager(  ) {
    RET_VAL ret = SUCCESS;
    LINKED_LIST *list = NULL;
    COMPARTMENT *compartment = NULL;
    
    START_FUNCTION("CloseCompartmentManager");
    
    if( manager.record == NULL ) {
        END_FUNCTION("CloseCompartmentManager", SUCCESS );
        return ret;
    }
    
    
    if( ( list = GenerateValueList( manager.table ) ) == NULL ) {
        END_FUNCTION("CloseCompartmentManager", FAILING );
        return FAILING;
    }
    
    while( ( compartment = (COMPARTMENT*)GetNextFromLinkedList( list ) ) != NULL ) {
        FreeString( &( compartment->id ) );
        FREE( compartment );
    }
    DeleteHashTable( (HASH_TABLE**)&(manager.table) );    
    
    manager.record = NULL;
    
        
    END_FUNCTION("CloseCompartmentManager", SUCCESS );
    return ret;
}

STRING *GetCompartmentID( COMPARTMENT *compartment ) {
    
    START_FUNCTION("GetCompartmentID");
    
    if( compartment == NULL ) {
        END_FUNCTION("GetCompartmentID", FAILING );
        return NULL;
    }
    
    END_FUNCTION("GetCompartmentID", SUCCESS );
    return compartment->id;
}


int GetSpatialDimensionsInCompartment( COMPARTMENT *compartment ) {
    START_FUNCTION("GetSpatialDimensionsInCompartment");
    
    if( compartment == NULL ) {
        END_FUNCTION("GetSpatialDimensionsInCompartment", FAILING );
        return -1;
    }
    
    END_FUNCTION("GetSpatialDimensionsInCompartment", SUCCESS );
    return compartment->spatialDimensions;
}


RET_VAL SetSpatialDimensionsInCompartment( COMPARTMENT *compartment, int spatialDimensions ) {
    START_FUNCTION("GetCompartmentID");
    
    if( compartment == NULL ) {
        END_FUNCTION("GetCompartmentID", FAILING );
        return FAILING;
    }
    
    compartment->spatialDimensions = spatialDimensions;
    END_FUNCTION("GetCompartmentID", SUCCESS );
    return SUCCESS;
}

double GetSizeInCompartment( COMPARTMENT *compartment ) {
    START_FUNCTION("GetSizeInCompartment");
    
    if( compartment == NULL ) {
        END_FUNCTION("GetSizeInCompartment", FAILING );
        return -1.0;
    }
    
    END_FUNCTION("GetSizeInCompartment", SUCCESS );
    return compartment->size;
}


RET_VAL SetSizeInCompartment( COMPARTMENT *compartment, double size ) {
    START_FUNCTION("SetSizeInCompartment");
    
    if( compartment == NULL ) {
        END_FUNCTION("SetSizeInCompartment", FAILING );
        return FAILING;
    }
    
    compartment->size = size;
    compartment->currentSize = size;
    END_FUNCTION("SetSizeInCompartment", SUCCESS );
    return SUCCESS;
}


double GetCurrentSizeInCompartment( COMPARTMENT *compartment ) {
    START_FUNCTION("GetSizeInCompartment");
    
    if( compartment == NULL ) {
        END_FUNCTION("GetSizeInCompartment", FAILING );
        return -1.0;
    }
    
    END_FUNCTION("GetSizeInCompartment", SUCCESS );
    return compartment->currentSize;
}


RET_VAL SetCurrentSizeInCompartment( COMPARTMENT *compartment, double size ) {
    START_FUNCTION("SetSizeInCompartment");
    
    if( compartment == NULL ) {
        END_FUNCTION("SetSizeInCompartment", FAILING );
        return FAILING;
    }
    
    compartment->currentSize = size;
    END_FUNCTION("SetSizeInCompartment", SUCCESS );
    return SUCCESS;
}



UNIT_DEFINITION *GetUnitInCompartment( COMPARTMENT *compartment ) {
    START_FUNCTION("GetUnitInCompartment");
    
    if( compartment == NULL ) {
        END_FUNCTION("GetUnitInCompartment", FAILING );
        return NULL;
    }
    
    END_FUNCTION("GetUnitInCompartment", SUCCESS );
    return compartment->unit;
}


RET_VAL SetUnitInCompartment( COMPARTMENT *compartment, UNIT_DEFINITION *unit ) {
    START_FUNCTION("SetUnitInCompartment");
    
    if( compartment == NULL ) {
        END_FUNCTION("SetUnitInCompartment", FAILING );
        return FAILING;
    }
    
    compartment->unit = unit;
    END_FUNCTION("SetUnitInCompartment", SUCCESS );
    return SUCCESS;
}

struct KINETIC_LAW *GetInitialAssignmentInCompartment( COMPARTMENT *compartment ) {
    START_FUNCTION("GetInitialAssignmentInCompartment");
    
    if( compartment == NULL ) {
        END_FUNCTION("GetInitialAssignmentInCompartment", FAILING );
        return NULL;
    }
    
    END_FUNCTION("GetInitialAssignmentInCompartment", SUCCESS );
    return compartment->initialAssignment;
}


RET_VAL SetInitialAssignmentInCompartment( COMPARTMENT *compartment, struct KINETIC_LAW *law ) {
    START_FUNCTION("SetInitialAssignmentInCompartment");
    
    if( compartment == NULL ) {
        END_FUNCTION("SetInitialAssignmentInCompartment", FAILING );
        return FAILING;
    }
    
    compartment->initialAssignment = law;
    END_FUNCTION("SetInitialAssignmentInCompartment", SUCCESS );
    return SUCCESS;
}


STRING *GetOutsideInCompartment( COMPARTMENT *compartment ) {
    START_FUNCTION("GetOutsideInCompartment");
    
    if( compartment == NULL ) {
        END_FUNCTION("GetOutsideInCompartment", FAILING );
        return NULL;
    }
    
    END_FUNCTION("GetOutsideInCompartment", SUCCESS );
    return compartment->outside;
}


RET_VAL SetOutsideInCompartment( COMPARTMENT *compartment, char *id ) {
    STRING *outside = NULL;
    
    START_FUNCTION("SetOutsideInCompartment");
    
    if( compartment == NULL ) {
        END_FUNCTION("SetOutsideInCompartment", FAILING );
        return FAILING;
    }
    
    if( ( outside = CreateString( id ) ) == NULL ) {
        END_FUNCTION("SetOutsideInCompartment", FAILING );
        return FAILING;
    }
    
    if( compartment->outside != NULL ) {
        FreeString( &(compartment->outside) );
    }
        
    compartment->outside = outside;
    END_FUNCTION("SetOutsideInCompartment", SUCCESS );
    return SUCCESS;
}


STRING *GetTypeInCompartment( COMPARTMENT *compartment ) {
    START_FUNCTION("GetTypeInCompartment");
    
    if( compartment == NULL ) {
        END_FUNCTION("GetTypeInCompartment", FAILING );
        return NULL;
    }
    
    END_FUNCTION("GetTypeInCompartment", SUCCESS );
    return compartment->type;
}


RET_VAL SetTypeInCompartment( COMPARTMENT *compartment, char *type ) {
    STRING *typeStr = NULL;
    
    START_FUNCTION("SetTypeInCompartment");
    
    if( compartment == NULL ) {
        END_FUNCTION("SetOTypeInCompartment", FAILING );
        return FAILING;
    }
    
    if( ( typeStr = CreateString( type ) ) == NULL ) {
        END_FUNCTION("SetTypeInCompartment", FAILING );
        return FAILING;
    }
    
    if( compartment->type != NULL ) {
        FreeString( &(compartment->type) );
    }
        
    compartment->type = typeStr;
    END_FUNCTION("SetOutsideInCompartment", SUCCESS );
    return SUCCESS;
}



COMPARTMENT *GetOutsideCompartmentInCompartment( COMPARTMENT *compartment ) {
    START_FUNCTION("GetOutsideCompartmentInCompartment");
    
    if( compartment == NULL ) {
        END_FUNCTION("GetOutsideCompartmentInCompartment", FAILING );
        return NULL;
    }
    
    END_FUNCTION("GetOutsideCompartmentInCompartment", SUCCESS );
    return compartment->outsideCompartment;
}


BOOL IsCompartmentConstant( COMPARTMENT *compartment ) {
    START_FUNCTION("IsCompartmentConstant");
    
    if( compartment == NULL ) {
        END_FUNCTION("IsCompartmentConstant", FAILING );
        return FALSE;
    }
    
    END_FUNCTION("IsCompartmentConstant", SUCCESS );
    return compartment->constant;
}


RET_VAL SetCompartmentConstant( COMPARTMENT *compartment, BOOL constant ) {
    START_FUNCTION("SetCompartmentConstant");
    
    if( compartment == NULL ) {
        END_FUNCTION("SetCompartmentConstant", FAILING );
        return FAILING;
    }
    
    compartment->constant = constant;
    if (constant) {
      compartment->algebraic = FALSE;
    } 
    END_FUNCTION("SetCompartmentConstant", SUCCESS );
    return SUCCESS;
}

BOOL IsCompartmentAlgebraic( COMPARTMENT *compartment ) {
    START_FUNCTION("IsCompartmentAlgebraic");
    
    if( compartment == NULL ) {
        END_FUNCTION("IsCompartmentAlgebraic", FAILING );
        return FALSE;
    }
    
    END_FUNCTION("IsCompartmentAlgebraic", SUCCESS );
    return compartment->algebraic;
}


RET_VAL SetCompartmentAlgebraic( COMPARTMENT *compartment, BOOL algebraic ) {
    START_FUNCTION("SetCompartmentAlgebraic");
    
    if( compartment == NULL ) {
        END_FUNCTION("SetCompartmentAlgebraic", FAILING );
        return FAILING;
    }
    
    compartment->algebraic = algebraic;
    END_FUNCTION("SetCompartmentAlgebraic", SUCCESS );
    return SUCCESS;
}

BOOL PrintCompartment( COMPARTMENT *compartment ) {
    START_FUNCTION("PrintCompartment");
    
    if( compartment == NULL ) {
        END_FUNCTION("PrintCompartment", FAILING );
        return FALSE;
    }
    
    END_FUNCTION("PrintCompartment", SUCCESS );
    return compartment->print;
}


RET_VAL SetPrintCompartment( COMPARTMENT *compartment, BOOL print ) {
    START_FUNCTION("SetPrintCompartment");
    
    if( compartment == NULL ) {
        END_FUNCTION("SetPrintCompartment", FAILING );
        return FAILING;
    }
    
    compartment->print = print;
    END_FUNCTION("SetPrintCompartment", SUCCESS );
    return SUCCESS;
}


static COMPARTMENT *_CreateCompartment( COMPARTMENT_MANAGER *manager, char *id ) {
    COMPARTMENT *compartment = NULL;
    
    START_FUNCTION("_CreateCompartment");

    if( ( compartment = (COMPARTMENT*)CALLOC( 1, sizeof(COMPARTMENT) ) ) == NULL ) {
        END_FUNCTION("_CreateCompartment", FAILING );
        return NULL;
    }
    if( ( compartment->id = CreateString( id ) ) == NULL ) {
        FREE( compartment );
        END_FUNCTION("_CreateUnitDefinition", FAILING );
        return NULL;
    }
    compartment->initialAssignment = NULL;
    compartment->outside = NULL;
    compartment->type = NULL;
    compartment->algebraic = FALSE;
    if( IS_FAILED( PutInHashTable( GetCharArrayOfString( compartment->id ), GetStringLength( compartment->id ), (CADDR_T)compartment, manager->table ) ) ) {
        FREE( compartment );
        END_FUNCTION("_CreateCompartment", FAILING );
        return NULL;
    } 
    
    END_FUNCTION("_CreateCompartment", SUCCESS );
    return compartment;
}

static COMPARTMENT *_LookupCompartment( COMPARTMENT_MANAGER *manager,  char *id ) {
    COMPARTMENT *compartment = NULL;
    
    START_FUNCTION("_LookupCompartment");
    
    if( ( compartment = (COMPARTMENT*)GetValueFromHashTable( id, strlen(id), manager->table ) ) == NULL ) {
        END_FUNCTION("_LookupCompartment", FAILING );
        return NULL;
    }
    
    END_FUNCTION("_LookupCompartment", SUCCESS );
    return compartment;
}

static RET_VAL _ResolveCompartmentLinks( COMPARTMENT_MANAGER *manager ) {
    RET_VAL ret = SUCCESS;
    char *outside = NULL;
    COMPARTMENT *compartment = NULL;
    COMPARTMENT *outsideCompartment = NULL;
    LINKED_LIST *list = NULL;
    
    START_FUNCTION("_ResolveCompartmentLinks");
    
    if( ( list = GenerateValueList( manager->table ) ) == NULL ) {
        return ErrorReport( FAILING, "_ResolveCompartmentLinks", "internal error -- could not create a compartment list from table" );        
    }
    
    while( ( compartment = (COMPARTMENT*)GetNextFromLinkedList( list ) ) != NULL ) {
        if( compartment->outside != NULL ) {
            outside = GetCharArrayOfString( compartment->outside );
            outsideCompartment = (COMPARTMENT*)GetValueFromHashTable( outside, strlen(outside), manager->table );
            if( outsideCompartment == NULL ) {
                return ErrorReport( FAILING, "_ResolveCompartmentLinks", "compartment %s not defined", outside );        
            }
            compartment->outsideCompartment = outsideCompartment;
        }        
    }
    
    END_FUNCTION("_ResolveCompartmentLinks", SUCCESS );
    return ret;
}

static LINKED_LIST *_CreateListOfCompartments( COMPARTMENT_MANAGER *manager ) {
    LINKED_LIST *list = NULL;
    
    START_FUNCTION("_CreateListOfCompartments");

    if( ( list = GenerateValueList( manager->table ) ) == NULL ) {
        END_FUNCTION("_CreateListOfCompartments", FAILING );
        return NULL;
    }
    
    END_FUNCTION("_CreateListOfCompartments", SUCCESS );
    return list;
}
    

