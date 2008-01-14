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
#if !defined(HAVE_FUNCTION_MANAGER)
#define HAVE_FUNCTION_MANAGER

#include "common.h"
#include "linked_list.h"
#include "util.h"

#include "compiler_def.h"
#include "hash_table.h"
#include "kinetic_law.h"

BEGIN_C_NAMESPACE

typedef struct {
    STRING *id;
    LINKED_LIST *arguments;
    KINETIC_LAW *function;
} FUNCTION_DEFINITION;

struct _FUNCTION_MANAGER;
typedef struct _FUNCTION_MANAGER FUNCTION_MANAGER;

struct _FUNCTION_MANAGER {
    HASH_TABLE *table;   
    COMPILER_RECORD_T *record;
    
    FUNCTION_DEFINITION * (*CreateFunctionDefinition)( FUNCTION_MANAGER *manager, char *id );
    FUNCTION_DEFINITION * (*LookupFunctionDefinition)( FUNCTION_MANAGER *manager, char *id );
    LINKED_LIST *(*CreateListOfFunctionDefinitions)( FUNCTION_MANAGER *manager );                  
};

STRING *GetFunctionDefinitionID( FUNCTION_DEFINITION *functionDef );
LINKED_LIST *GetArgumentsInFunctionDefinition( FUNCTION_DEFINITION *functionDef );
RET_VAL AddArgumentInFunctionDefinition( FUNCTION_DEFINITION *functionDef, char *argument );
KINETIC_LAW *GetFunctionInFunctionDefinition( FUNCTION_DEFINITION *functionDef );
RET_VAL AddFunctionInFunctionDefinition( FUNCTION_DEFINITION *functionDef, KINETIC_LAW *function );

FUNCTION_MANAGER *GetFunctionManagerInstance( COMPILER_RECORD_T *record );
RET_VAL CloseFunctionManager( );

END_C_NAMESPACE

#endif
