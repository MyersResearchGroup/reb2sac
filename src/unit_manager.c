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
#include "unit_manager.h"
#include "hash_table.h"

static UNIT_MANAGER manager;


static UNIT_DEFINITION * _CreateUnitDefinition( UNIT_MANAGER *manager, char *id );
static UNIT_DEFINITION * _LookupUnitDefinition( UNIT_MANAGER *manager, char *id );
static LINKED_LIST *_CreateListOfUnitDefinitions( UNIT_MANAGER *manager );                  


UNIT_MANAGER *GetUnitManagerInstance( COMPILER_RECORD_T *record ) {
    
    START_FUNCTION("GetUnitManagerInstance");

    if( manager.record == NULL ) {
        manager.record = record;    
        if( ( manager.table = CreateHashTable( 128 ) ) == NULL ) {
            END_FUNCTION("GetUnitManagerInstance", FAILING );
            return NULL;
        }    
        manager.CreateUnitDefinition = _CreateUnitDefinition;
        manager.LookupUnitDefinition = _LookupUnitDefinition;
        manager.CreateListOfUnitDefinitions = _CreateListOfUnitDefinitions;
    }
        
    END_FUNCTION("GetUnitManagerInstance", SUCCESS );
    return &manager;
}


RET_VAL CloseUnitManager(  ) {
    RET_VAL ret = SUCCESS;
    LINKED_LIST *list = NULL;    
    LINKED_LIST *units = NULL;
    UNIT *unit = NULL;
    UNIT_DEFINITION *unitDef = NULL;    
    
    START_FUNCTION("CloseUnitManager");
    
    if( manager.record == NULL ) {
        END_FUNCTION("CloseUnitManager", SUCCESS );
        return ret;
    }
                
    if( ( list = GenerateValueList( manager.table ) ) == NULL ) {
        END_FUNCTION("CloseUnitManager", FAILING );
        return FAILING;
    }
    
    while( ( unitDef = (UNIT_DEFINITION*)GetNextFromLinkedList( list ) ) != NULL ) {
        FreeString( &( unitDef->id ) );
        if( ( units = unitDef->units ) != NULL ) {
            while( ( unit = (UNIT*)GetNextFromLinkedList( units ) ) != NULL ) {
                FreeString( &(unit->kind) );
                FREE( unit );
            }
            DeleteLinkedList( &units );
        }
        FREE( unitDef );
    }
    DeleteLinkedList( &list );
    DeleteHashTable( (HASH_TABLE**)&(manager.table) );        
    manager.record = NULL;       
    
    END_FUNCTION("CloseUnitManager", SUCCESS );
    return ret;
}



STRING *GetUnitDefinitionID( UNIT_DEFINITION *unitDef ) {
    START_FUNCTION("GetUnitDefinitionID");
            
    END_FUNCTION("GetUnitDefinitionID", SUCCESS );
    return (unitDef == NULL ? NULL : unitDef->id);
}


LINKED_LIST *GetUnitsInUnitDefinition( UNIT_DEFINITION *unitDef ) {
    START_FUNCTION("GetUnitsInUnitDefinition");
            
    END_FUNCTION("GetUnitsInUnitDefinition", SUCCESS );
    return (unitDef == NULL ? NULL : unitDef->units);
}


RET_VAL AddUnitInUnitDefinition( UNIT_DEFINITION *unitDef, char *kind, int exponent, int scale, double multiplier, double offset ) {
    RET_VAL ret = SUCCESS;
    UNIT *unit = NULL;    
    
    START_FUNCTION("GetUnitsInUnitDefinition");
            
    if( ( unit = (UNIT*)MALLOC( sizeof( UNIT ) ) ) == NULL ) {
        return ErrorReport( FAILING, "GetUnitsInUnitDefinition", "allocatation failed for unit %s", kind ); 
    }
    
    if( IS_FAILED( ( ret = AddElementInLinkedList( unit, unitDef->units ) ) ) ) {
        END_FUNCTION("GetUnitsInUnitDefinition", ret );
        return ret;
    }
    
    if( ( unit->kind = CreateString( kind ) ) == NULL ) {
        FREE( unit );
        return ErrorReport( FAILING, "GetUnitsInUnitDefinition", "allocatation failed for unit %s", kind ); 
    }  
    
    unit->exponent = exponent;
    unit->scale = scale;
    unit->multiplier = multiplier;
    unit->offset = offset;        
    
    END_FUNCTION("GetUnitsInUnitDefinition", SUCCESS );
    return ret;
}




static UNIT_DEFINITION * _CreateUnitDefinition( UNIT_MANAGER *manager, char *id ) {
    UNIT_DEFINITION *unitDef = NULL;
    
    START_FUNCTION("_CreateUnitDefinition");

    if( ( unitDef = (UNIT_DEFINITION*)MALLOC( sizeof(UNIT_DEFINITION) ) ) == NULL ) {
        END_FUNCTION("_CreateUnitDefinition", FAILING );
        return NULL;
    }
    if( ( unitDef->id = CreateString( id ) ) == NULL ) {
        FREE( unitDef );
        END_FUNCTION("_CreateUnitDefinition", FAILING );
        return NULL;
    }
    
    if( ( unitDef->units = CreateLinkedList() ) == NULL ) {
        return ErrorReport( FAILING, "_CreateUnitDefinition", "could not create a unit list for %s", id );
    }
        
    if( IS_FAILED( PutInHashTable( GetCharArrayOfString( unitDef->id ), GetStringLength( unitDef->id ), unitDef, manager->table ) ) ) {
        FREE( unitDef );
        END_FUNCTION("_CreateUnitDefinition", FAILING );
        return NULL;
    } 
    
    END_FUNCTION("_CreateUnitDefinition", SUCCESS );
    return unitDef;
}

static UNIT_DEFINITION * _LookupUnitDefinition( UNIT_MANAGER *manager, char *id ) {
    UNIT_DEFINITION *unitDef = NULL;
        
    START_FUNCTION("_LookupUnitDefinition");
    
    if( ( unitDef = GetValueFromHashTable( id, strlen(id), manager->table ) ) == NULL ) {
        END_FUNCTION("_LookupUnitDefinition", FAILING );
        return NULL;
    }
    
    END_FUNCTION("_LookupUnitDefinition", SUCCESS );
    return unitDef;
}
    
static LINKED_LIST *_CreateListOfUnitDefinitions( UNIT_MANAGER *manager ) {
    LINKED_LIST *list = NULL;
    
    START_FUNCTION("_CreateListOfUnitDefinitions");

    if( ( list = GenerateValueList( manager->table ) ) == NULL ) {
        END_FUNCTION("_CreateListOfUnitDefinitions", FAILING );
        return NULL;
    }
    
    END_FUNCTION("_CreateListOfUnitDefinitions", SUCCESS );
    return list;
}                  
    

STRING *GetKindInUnit( UNIT *unit ) {
    START_FUNCTION("GetKindInUnit");
    
    if( unit == NULL ) {
        END_FUNCTION("GetKindInUnit", FAILING );
        return NULL;
    }
    
    END_FUNCTION("GetKindInUnit", SUCCESS );
    return unit->kind;
}

int GetExponentInUnit( UNIT *unit ) {
    START_FUNCTION("GetExponentInUnit");
    
    if( unit == NULL ) {
        END_FUNCTION("GetExponentInUnit", FAILING );
        return 1;
    }
    
    END_FUNCTION("GetExponentInUnit", SUCCESS );
    return unit->exponent;
}

int GetScaleInUnit( UNIT *unit )  {
    START_FUNCTION("GetScaleInUnit");
    
    if( unit == NULL ) {
        END_FUNCTION("GetScaleInUnit", FAILING );
        return 0;
    }
    
    END_FUNCTION("GetScaleInUnit", SUCCESS );
    return unit->scale;
}

double GetMultiplierInUnit( UNIT *unit ) {
    START_FUNCTION("GetMultiplierInUnit");
    
    if( unit == NULL ) {
        END_FUNCTION("GetMultiplierInUnit", FAILING );
        return 1.0;
    }
    
    END_FUNCTION("GetMultiplierInUnit", SUCCESS );
    return unit->multiplier;
}

double GetOffsetInUnit( UNIT *unit ) {
    START_FUNCTION("GetOffsetInUnit");
    
    if( unit == NULL ) {
        END_FUNCTION("GetOffsetInUnit", FAILING );
        return 0.0;
    }
    
    END_FUNCTION("GetOffsetInUnit", SUCCESS );
    return unit->offset;
}


