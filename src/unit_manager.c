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


static UNIT_DEFINITION * _CreateUnitDefinition( UNIT_MANAGER *manager, char *id, BOOL builtIn );
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
	_CreateUnitDefinition(&manager,"substance",TRUE);
	_CreateUnitDefinition(&manager,"volume",TRUE);
	_CreateUnitDefinition(&manager,"area",TRUE);
	_CreateUnitDefinition(&manager,"length",TRUE);
	_CreateUnitDefinition(&manager,"time",TRUE);
	_CreateUnitDefinition(&manager,"ampere",TRUE);
	_CreateUnitDefinition(&manager,"avogadro",TRUE);
	_CreateUnitDefinition(&manager,"becquerel",TRUE);
	_CreateUnitDefinition(&manager,"candela",TRUE);
	_CreateUnitDefinition(&manager,"celsius",TRUE);
	_CreateUnitDefinition(&manager,"coulomb",TRUE);
	_CreateUnitDefinition(&manager,"dimensionless",TRUE);
	_CreateUnitDefinition(&manager,"farad",TRUE);
	_CreateUnitDefinition(&manager,"gram",TRUE);
	_CreateUnitDefinition(&manager,"gray",TRUE);
	_CreateUnitDefinition(&manager,"henry",TRUE);
	_CreateUnitDefinition(&manager,"hertz",TRUE);
	_CreateUnitDefinition(&manager,"item",TRUE);
	_CreateUnitDefinition(&manager,"joule",TRUE);
	_CreateUnitDefinition(&manager,"katal",TRUE);
	_CreateUnitDefinition(&manager,"kelvin",TRUE);
	_CreateUnitDefinition(&manager,"kilogram",TRUE);
	_CreateUnitDefinition(&manager,"litre",TRUE);
	_CreateUnitDefinition(&manager,"lumen",TRUE);
	_CreateUnitDefinition(&manager,"lux",TRUE);
	_CreateUnitDefinition(&manager,"metre",TRUE);
	_CreateUnitDefinition(&manager,"mole",TRUE);
	_CreateUnitDefinition(&manager,"newton",TRUE);
	_CreateUnitDefinition(&manager,"ohm",TRUE);
	_CreateUnitDefinition(&manager,"pascal",TRUE);
	_CreateUnitDefinition(&manager,"radian",TRUE);
	_CreateUnitDefinition(&manager,"second",TRUE);
	_CreateUnitDefinition(&manager,"siemens",TRUE);
	_CreateUnitDefinition(&manager,"sievert",TRUE);
	_CreateUnitDefinition(&manager,"steradian",TRUE);
	_CreateUnitDefinition(&manager,"tesla",TRUE);
	_CreateUnitDefinition(&manager,"volt",TRUE);
	_CreateUnitDefinition(&manager,"watt",TRUE);
	_CreateUnitDefinition(&manager,"weber",TRUE);
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

BOOL IsBuiltInUnitDefinition( UNIT_DEFINITION *unitDef ) {
    START_FUNCTION("IsBuiltInUnitDefinition");
            
    END_FUNCTION("IsBuiltInUnitDefinition", SUCCESS );
    return unitDef->builtIn;
}


LINKED_LIST *GetUnitsInUnitDefinition( UNIT_DEFINITION *unitDef ) {
    START_FUNCTION("GetUnitsInUnitDefinition");
            
    END_FUNCTION("GetUnitsInUnitDefinition", SUCCESS );
    return (unitDef == NULL ? NULL : unitDef->units);
}


RET_VAL AddUnitInUnitDefinition( UNIT_DEFINITION *unitDef, char *kind, double exponent, int scale, double multiplier ) {
    RET_VAL ret = SUCCESS;
    UNIT *unit = NULL;    
    
    START_FUNCTION("AddUnitsInUnitDefinition");
    if( ( unit = (UNIT*)MALLOC( sizeof( UNIT ) ) ) == NULL ) {
        return ErrorReport( FAILING, "AddUnitsInUnitDefinition", "allocatation failed for unit %s", kind ); 
    }
    
    if( IS_FAILED( ( ret = AddElementInLinkedList( (CADDR_T)unit, unitDef->units ) ) ) ) {
        END_FUNCTION("AddUnitsInUnitDefinition", ret );
        return ret;
    }
    
    if( ( unit->kind = CreateString( kind ) ) == NULL ) {
        FREE( unit );
        return ErrorReport( FAILING, "AddUnitsInUnitDefinition", "allocatation failed for unit %s", kind ); 
    }  
    
    unit->exponent = exponent;
    unit->scale = scale;
    unit->multiplier = multiplier;
    
    END_FUNCTION("AddUnitsInUnitDefinition", SUCCESS );
    return ret;
}




static UNIT_DEFINITION * _CreateUnitDefinition( UNIT_MANAGER *manager, char *id, BOOL builtIn ) {
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
      return NULL;
    }
    unitDef->builtIn = builtIn;
    
    if( IS_FAILED( PutInHashTable( GetCharArrayOfString( unitDef->id ), GetStringLength( unitDef->id ), (CADDR_T)unitDef, manager->table ) ) ) {
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
    
    if( ( unitDef = (UNIT_DEFINITION*)GetValueFromHashTable( id, strlen(id), manager->table ) ) == NULL ) {
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

double GetExponentInUnit( UNIT *unit ) {
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
