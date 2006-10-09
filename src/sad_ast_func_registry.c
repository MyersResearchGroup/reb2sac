/***************************************************************************
 *   Copyright (C) 2006 by Hiroyuki Kuwahara   *
 *   kuwahara@cs.utah.edu   *
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
#include "sad_ast_func_registry.h"
#include <math.h>

static SAD_AST_FUNC_REGISTRY _registry;

static double _Log( double *argv[] );
static double _Pow( double *argv[] );
static double _Exp( double *argv[] );


static SAD_AST_FUNC_REGISTRY_ENTRY *_GetEntry( SAD_AST_FUNC_REGISTRY *registry, char *id );
static RET_VAL _AddEntry( SAD_AST_FUNC_REGISTRY *registry, char *id, int argc, double (*func)( double *[] ) );    
static RET_VAL _AddPredefinedEntries( SAD_AST_FUNC_REGISTRY *registry );    


SAD_AST_FUNC_REGISTRY *GetSadAstFuncRegistryInstance() {
    SAD_AST_FUNC_REGISTRY *registry = &_registry;
    
    START_FUNCTION("GetSadAstFuncRegistryInstance");
    if( registry->table == NULL ) {
        if( ( registry->table = CreateHashTable( 256 ) ) == NULL ) {
            END_FUNCTION("GetSadAstFuncRegistryInstance", FAILING );
            return NULL;
        }
        registry->AddEntry = _AddEntry;
        registry->GetEntry = _GetEntry;  
                
        if( IS_FAILED( _AddPredefinedEntries( registry ) ) ) {
            END_FUNCTION("GetSadAstFuncRegistryInstance", FAILING );
            return NULL;
        }
    }

    END_FUNCTION("GetSadAstFuncRegistryInstance", SUCCESS );
    return registry;
}


static RET_VAL _AddPredefinedEntries( SAD_AST_FUNC_REGISTRY *registry ) {
    RET_VAL ret = SUCCESS;
    
    if( IS_FAILED( ( ret = registry->AddEntry( registry, "log", 1, _Log ) ) ) ) {
        return ret;
    }
    if( IS_FAILED( ( ret = registry->AddEntry( registry, "exp", 1, _Exp ) ) ) ) {
        return ret;
    }
    if( IS_FAILED( ( ret = registry->AddEntry( registry, "pow", 2, _Pow ) ) ) ) {
        return ret;
    }
    
    return SUCCESS;
}


RET_VAL FreeSadAstFuncRegistryInstance() {
    LINKED_LIST *list = NULL;
    SAD_AST_FUNC_REGISTRY_ENTRY *entry = NULL;
    
    START_FUNCTION("FreeSadAstFuncRegistryInstance");
    if( _registry.table == NULL ) {
        END_FUNCTION("FreeSadAstFuncRegistryInstance", SUCCESS );
        return SUCCESS;
    }
    if( ( list = GenerateValueList( _registry.table ) ) == NULL ) {
        _registry.table = NULL;
        END_FUNCTION("FreeSadAstFuncRegistryInstance", SUCCESS );
        return SUCCESS;
    }
    
    ResetCurrentElement( list );
    while( (entry = (SAD_AST_FUNC_REGISTRY_ENTRY*)GetNextFromLinkedList( list ) ) != NULL ) {
        FREE( entry );
    }
    DeleteLinkedList( &list );
    DeleteHashTable( &(_registry.table) );
    
    END_FUNCTION("FreeSadAstFuncRegistryInstance", SUCCESS );

    return SUCCESS;
}


static SAD_AST_FUNC_REGISTRY_ENTRY *_GetEntry( SAD_AST_FUNC_REGISTRY *registry, char *id ) {
    SAD_AST_FUNC_REGISTRY_ENTRY *entry = NULL;
    
    entry = (SAD_AST_FUNC_REGISTRY_ENTRY*)GetValueFromHashTable( id, strlen(id), registry->table );
    return entry;    
}

static RET_VAL _AddEntry( SAD_AST_FUNC_REGISTRY *registry, char *id, int argc, double (*func)( double *[] ) ) {
    RET_VAL ret = SUCCESS;
    
    SAD_AST_FUNC_REGISTRY_ENTRY *entry = NULL;

    entry = (SAD_AST_FUNC_REGISTRY_ENTRY*)MALLOC( sizeof(SAD_AST_FUNC_REGISTRY_ENTRY) ); 
    if( entry == NULL ) {
        return FAILING;
    }
    
    entry->funcID = id;
    entry->argc = argc;
    entry->func = func;
    
    if( IS_FAILED( ( ret = PutInHashTable( id, strlen(id), (CADDR_T)entry, registry->table ) ) ) ) {
        return ret;
    }
    
    return SUCCESS;
}  



static double _Log( double *argv[] ) {
    double x = *(argv[0]);
    
    return log(x);
}

static double _Pow( double *argv[] ) {
    double x = *(argv[0]);
    double y = *(argv[1]);
    
    return pow( x, y );
}

static double _Exp( double *argv[] ) {
    double x = *(argv[0]);
    
    return exp( x );        
}
