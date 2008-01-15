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
#include "function_manager.h"
#include "hash_table.h"

static FUNCTION_MANAGER manager;


static FUNCTION_DEFINITION * _CreateFunctionDefinition( FUNCTION_MANAGER *manager, char *id );
static FUNCTION_DEFINITION * _LookupFunctionDefinition( FUNCTION_MANAGER *manager, char *id );
static LINKED_LIST *_CreateListOfFunctionDefinitions( FUNCTION_MANAGER *manager );                  


FUNCTION_MANAGER *GetFunctionManagerInstance( COMPILER_RECORD_T *record ) {
    
    START_FUNCTION("GetFunctionManagerInstance");

    if( manager.record == NULL ) {
        manager.record = record;    
        if( ( manager.table = CreateHashTable( 128 ) ) == NULL ) {
            END_FUNCTION("GetFunctionManagerInstance", FAILING );
            return NULL;
        }    
        manager.CreateFunctionDefinition = _CreateFunctionDefinition;
        manager.LookupFunctionDefinition = _LookupFunctionDefinition;
        manager.CreateListOfFunctionDefinitions = _CreateListOfFunctionDefinitions;
    }
        
    END_FUNCTION("GetFunctionManagerInstance", SUCCESS );
    return &manager;
}


RET_VAL CloseFunctionManager(  ) {
    RET_VAL ret = SUCCESS;
    LINKED_LIST *list = NULL;    
    LINKED_LIST *arguments = NULL;
    char *argument = NULL;
    FUNCTION_DEFINITION *functionDef = NULL;    
    
    START_FUNCTION("CloseFunctionManager");
    
    if( manager.record == NULL ) {
        END_FUNCTION("CloseFunctionManager", SUCCESS );
        return ret;
    }
                
    if( ( list = GenerateValueList( manager.table ) ) == NULL ) {
        END_FUNCTION("CloseFunctionManager", FAILING );
        return FAILING;
    }
    
    while( ( functionDef = (FUNCTION_DEFINITION*)GetNextFromLinkedList( list ) ) != NULL ) {
        FreeString( &( functionDef->id ) );
        if( ( arguments = functionDef->arguments ) != NULL ) {
            while( ( argument = (char*)GetNextFromLinkedList( arguments ) ) != NULL ) {
                FREE( argument );
            }
            DeleteLinkedList( &arguments );
        }
	FreeKineticLaw( &(functionDef->function) );
        FREE( functionDef );
    }
    DeleteLinkedList( &list );
    DeleteHashTable( (HASH_TABLE**)&(manager.table) );        
    manager.record = NULL;       
    
    END_FUNCTION("CloseFunctionManager", SUCCESS );
    return ret;
}



STRING *GetFunctionDefinitionID( FUNCTION_DEFINITION *functionDef ) {
    START_FUNCTION("GetFunctionDefinitionID");
            
    END_FUNCTION("GetFunctionDefinitionID", SUCCESS );
    return (functionDef == NULL ? NULL : functionDef->id);
}


LINKED_LIST *GetArgumentsInFunctionDefinition( FUNCTION_DEFINITION *functionDef ) {
    START_FUNCTION("GetArgumentsInFunctionDefinition");
            
    END_FUNCTION("GetArgumentsInFunctionDefinition", SUCCESS );
    return (functionDef == NULL ? NULL : functionDef->arguments);
}


RET_VAL AddArgumentInFunctionDefinition( FUNCTION_DEFINITION *functionDef, char *argument ) {
    RET_VAL ret = SUCCESS;
    char *arg = NULL;    
    
    START_FUNCTION("AddArgumentInFunctionDefinition");
            
    if( ( arg = (char*)MALLOC( sizeof( char* ) ) ) == NULL ) {
        return ErrorReport( FAILING, "AddArgumentInFunctionDefinition", "allocatation failed for argument %s", argument ); 
    }
    
    if( IS_FAILED( ( ret = AddElementInLinkedList( argument, functionDef->arguments ) ) ) ) {
        END_FUNCTION("GetFunctionsInFunctionDefinition", ret );
        return ret;
    }
    
//     if( ( function->argument = CreateString( argument ) ) == NULL ) {
//         FREE( function );
//         return ErrorReport( FAILING, "AddArgumentInFunctionDefinition", "allocatation failed for argument %s", argument ); 
//     }  
    
    END_FUNCTION("AddArgumentInFunctionDefinition", SUCCESS );
    return ret;
}


KINETIC_LAW *GetFunctionInFunctionDefinition( FUNCTION_DEFINITION *functionDef ) {
    START_FUNCTION("GetFunctionInFunctionDefinition");
            
    END_FUNCTION("GetFunctionInFunctionDefinition", SUCCESS );
    return (functionDef == NULL ? NULL : functionDef->function);
}


RET_VAL AddFunctionInFunctionDefinition( FUNCTION_DEFINITION *functionDef, KINETIC_LAW *function ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("AddFunctionInFunctionDefinition");

    functionDef->function = function;

    END_FUNCTION("AddFunctionInFunctionDefinition", SUCCESS );
    return ret;
}


static FUNCTION_DEFINITION * _CreateFunctionDefinition( FUNCTION_MANAGER *manager, char *id ) {
    FUNCTION_DEFINITION *functionDef = NULL;
    
    START_FUNCTION("_CreateFunctionDefinition");

    if( ( functionDef = (FUNCTION_DEFINITION*)MALLOC( sizeof(FUNCTION_DEFINITION) ) ) == NULL ) {
        END_FUNCTION("_CreateFunctionDefinition", FAILING );
        return NULL;
    }
    if( ( functionDef->id = CreateString( id ) ) == NULL ) {
        FREE( functionDef );
        END_FUNCTION("_CreateFunctionDefinition", FAILING );
        return NULL;
    }
    
    if( ( functionDef->arguments = CreateLinkedList() ) == NULL ) {
        return ErrorReport( FAILING, "_CreateFunctionDefinition", "could not create a function list for %s", id );
    }
        
    if( IS_FAILED( PutInHashTable( GetCharArrayOfString( functionDef->id ), GetStringLength( functionDef->id ), functionDef, manager->table ) ) ) {
        FREE( functionDef );
        END_FUNCTION("_CreateFunctionDefinition", FAILING );
        return NULL;
    } 
    
    END_FUNCTION("_CreateFunctionDefinition", SUCCESS );
    return functionDef;
}

static FUNCTION_DEFINITION * _LookupFunctionDefinition( FUNCTION_MANAGER *manager, char *id ) {
    FUNCTION_DEFINITION *functionDef = NULL;
        
    START_FUNCTION("_LookupFunctionDefinition");
    
    if( ( functionDef = GetValueFromHashTable( id, strlen(id), manager->table ) ) == NULL ) {
        END_FUNCTION("_LookupFunctionDefinition", FAILING );
        return NULL;
    }
    
    END_FUNCTION("_LookupFunctionDefinition", SUCCESS );
    return functionDef;
}
    
static LINKED_LIST *_CreateListOfFunctionDefinitions( FUNCTION_MANAGER *manager ) {
    LINKED_LIST *list = NULL;
    
    START_FUNCTION("_CreateListOfFunctionDefinitions");

    if( ( list = GenerateValueList( manager->table ) ) == NULL ) {
        END_FUNCTION("_CreateListOfFunctionDefinitions", FAILING );
        return NULL;
    }
    
    END_FUNCTION("_CreateListOfFunctionDefinitions", SUCCESS );
    return list;
}                  
    
