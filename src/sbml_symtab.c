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
#include "sbml_symtab.h"

static SBML_SYMTAB_MANAGER manager;

static RET_VAL _SetGlobal( SBML_SYMTAB_MANAGER *manager, ListOf_t *params );
static RET_VAL _SetLocal( SBML_SYMTAB_MANAGER *manager, ListOf_t *params );
static RET_VAL _SetLocalID( SBML_SYMTAB_MANAGER *manager, char *localID );
static BOOL _LookupValue( SBML_SYMTAB_MANAGER *manager, char *id, double *value );    
static BOOL _LookupGlobalValue( SBML_SYMTAB_MANAGER *manager, char *id, double *value );    
static BOOL _LookupLocalValue( SBML_SYMTAB_MANAGER *manager, char *id, double *value, char **unitsID );    
static BOOL _LookupLocalID( SBML_SYMTAB_MANAGER *manager, char **localID );    
static RET_VAL _PutParametersInGlobalSymtab( SBML_SYMTAB_MANAGER *manager, REB2SAC_SYMTAB *globalSymtab, UNIT_MANAGER *unitManager );    


SBML_SYMTAB_MANAGER *GetSymtabManagerInstance( COMPILER_RECORD_T *record ) {
    
    START_FUNCTION("GetSymtabManagerInstance");

    if( manager.record == NULL ) {
        manager.record = record;        
            
        manager.SetGlobal = _SetGlobal;
        manager.SetLocal = _SetLocal;
        manager.SetLocalID = _SetLocalID;
        manager.LookupValue = _LookupValue; 
        manager.LookupGlobalValue = _LookupGlobalValue;
        manager.LookupLocalValue = _LookupLocalValue;
        manager.LookupLocalID = _LookupLocalID;
        manager.PutParametersInGlobalSymtab = _PutParametersInGlobalSymtab;
    }
        
    END_FUNCTION("GetSymtabManagerInstance", SUCCESS);
    return &manager;
}

RET_VAL CloseSymtabManager() {
    RET_VAL ret = SUCCESS;
    LINKED_LIST *list = NULL;
    SBML_SYMTAB *symtab = NULL;
        
    START_FUNCTION("CloseSymtabManager");

    if( manager.record == NULL ) {
        END_FUNCTION("CloseSymtabManager", SUCCESS);
        return SUCCESS;
    }
    manager.record = NULL;                
    
    END_FUNCTION("CloseSymtabManager", SUCCESS);
    return SUCCESS;
}




static RET_VAL _SetGlobal( SBML_SYMTAB_MANAGER *manager, ListOf_t *params ) {
    START_FUNCTION("_SetGlobal");

    manager->global = params;

    END_FUNCTION("_SetGlobal", SUCCESS );
    return SUCCESS;
}


static RET_VAL _SetLocal( SBML_SYMTAB_MANAGER *manager, ListOf_t *params ) {
    START_FUNCTION("_SetLocal");

    manager->local = params;

    END_FUNCTION("_SetLocal", SUCCESS );
    return SUCCESS;
}

static RET_VAL _SetLocalID( SBML_SYMTAB_MANAGER *manager, char *localID ) {
    START_FUNCTION("_SetLocalID");

    manager->localID = localID;

    END_FUNCTION("_SetLocalID", SUCCESS );
    return SUCCESS;
}

static BOOL _LookupValue( SBML_SYMTAB_MANAGER *manager, char *id, double *value ) {
    UINT i = 0;
    UINT num = 0;
    Parameter_t *param = NULL;
    ListOf_t *localParams = NULL;
    ListOf_t *globalParams = NULL;
    
    START_FUNCTION("_LookupValue");

    TRACE_1("looking up %s", id);
    localParams = manager->local;
    if( localParams != NULL ) {
        /*first search local */
        num = ListOf_size( localParams );
        for( i = 0; i < num; i++ ) {
            param = (Parameter_t*)ListOf_get( localParams, i );
            if( strcmp( id, Parameter_getId( param ) ) == 0 ) {
                *value = Parameter_getValue( param );
                TRACE_1("found in local. value = %f", *value);
                END_FUNCTION("_LookupValue", SUCCESS );
                return TRUE;
            }
        }
    }

    globalParams = manager->global;
    if( globalParams != NULL ) {
        /*then search global */
        num = ListOf_size( globalParams );
        for( i = 0; i < num; i++ ) {
            param = (Parameter_t*)ListOf_get( globalParams, i );
            if( strcmp( id, Parameter_getId( param ) ) == 0 ) {
                *value = Parameter_getValue( param );
                TRACE_1("found in local. value = %f", *value);
                END_FUNCTION("_LookupValue", SUCCESS );
                return TRUE;
            }
        }
    }
    END_FUNCTION("_LookupValue", SUCCESS );
    return FALSE;
}    

static BOOL _LookupGlobalValue( SBML_SYMTAB_MANAGER *manager, char *id, double *value ) {
    UINT i = 0;
    UINT num = 0;
    Parameter_t *param = NULL;
    ListOf_t *globalParams = NULL;
    
    START_FUNCTION("_LookupGlobalValue");

    TRACE_1("looking up %s", id);
    globalParams = manager->global;
    if( globalParams != NULL ) {
        /*then search global */
        num = ListOf_size( globalParams );
        for( i = 0; i < num; i++ ) {
            param = (Parameter_t*)ListOf_get( globalParams, i );
            if( strcmp( id, Parameter_getId( param ) ) == 0 ) {
                *value = Parameter_getValue( param );
                TRACE_1("found in global. value = %f", *value);
                END_FUNCTION("_LookupGlobalValue", SUCCESS );
                return TRUE;
            }
        }
    }
    END_FUNCTION("_LookupGlobalValue", SUCCESS );
    return FALSE;
}

static BOOL _LookupLocalValue( SBML_SYMTAB_MANAGER *manager, char *id, double *value, char **unitsID ) {
    UINT i = 0;
    UINT num = 0;
    Parameter_t *param = NULL;
    ListOf_t *localParams = NULL;
    
    START_FUNCTION("_LookupLocalValue");

    TRACE_1("looking up %s", id);
    localParams = manager->local;
    if( localParams != NULL ) {
        /*first search local */
        num = ListOf_size( localParams );
        for( i = 0; i < num; i++ ) {
            param = (Parameter_t*)ListOf_get( localParams, i );
            if( strcmp( id, Parameter_getId( param ) ) == 0 ) {
                *value = Parameter_getValue( param );
                *unitsID = (char*)Parameter_getUnits( param );
                TRACE_1("found in local. value = %f", *value);
                END_FUNCTION("_LookupLocalValue", SUCCESS );
                return TRUE;
            }
        }
    }
    END_FUNCTION("_LookupLocalValue", SUCCESS );
    return FALSE;
}

static BOOL _LookupLocalID( SBML_SYMTAB_MANAGER *manager, char **localID ) {
  
  *localID = manager->localID;
  return TRUE;
}

static RET_VAL _PutParametersInGlobalSymtab( SBML_SYMTAB_MANAGER *manager, REB2SAC_SYMTAB *globalSymtab, UNIT_MANAGER *unitManager ) {
    RET_VAL ret = SUCCESS;
    char *id = NULL;
    double value = 0.0;
    UINT i = 0;
    UINT num = 0;
    Parameter_t *param = NULL;
    ListOf_t *globalParams = NULL;
    BOOL constant = TRUE;
    char *units;
    UNIT_DEFINITION *unitDef;
    REB2SAC_SYMBOL *paramS;

    START_FUNCTION("_PutParametersInGlobalSymtab");

    globalParams = manager->global;
    if( globalParams != NULL ) {
      num = ListOf_size( globalParams );
      for( i = 0; i < num; i++ ) {
	param = (Parameter_t*)ListOf_get( globalParams, i );
	id = (char*)Parameter_getId( param );
	value = Parameter_getValue( param );
	constant = TRUE;
	if (!Parameter_getConstant( param )) {
	  constant = FALSE;
	} 
	if( ( ( paramS = globalSymtab->AddRealValueSymbol( globalSymtab, id, value, constant ) ) ) == NULL ) {
	  return ErrorReport( FAILING, "_PutParametersInGlobalSymtab", 
			      "failed to put parameter %s in global symtab", id );
	}     
	if( (units = (char*)Parameter_getUnits( param )) != NULL ) {
	  if( ( unitDef = unitManager->LookupUnitDefinition( unitManager, units ) ) == NULL ) {
            return ErrorReport( FAILING, "_PutParametersInGlobalSymtab",  "unit def %s is not declared", units );
	  }             
	  if( IS_FAILED( ( ret = SetUnitsInSymbol( paramS, unitDef  ) ) ) ) {
            END_FUNCTION("_PutParametersInGlobalSymtab", ret );
            return ret;
	  }
	}
      }
    }
    END_FUNCTION("_PutParametersInGlobalSymtab", SUCCESS );
    return SUCCESS;
}

