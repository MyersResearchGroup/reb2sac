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
#if !defined(HAVE_SBML_SYMTAB)
#define HAVE_SBML_SYMTAB

#include "common.h"
#include "compiler_def.h"
#include "hash_table.h"
#include "symtab.h"

#include "sbml/ListOf.h"
#include "sbml/Parameter.h"

BEGIN_C_NAMESPACE


struct _SBML_SYMTAB;
typedef struct _SBML_SYMTAB SBML_SYMTAB;

struct _SBML_SYMTAB {
    ListOf_t *params;
};




struct _SBML_SYMTAB_MANAGER;

typedef struct _SBML_SYMTAB_MANAGER SBML_SYMTAB_MANAGER;

/*
struct _SBML_SYMTAB_MANAGER {    
    SBML_SYMTAB *global;
    SBML_SYMTAB **local;
    COMPILER_RECORD_T *record;
    
    RET_VAL (*SetGlobal)( SBML_SYMTAB_MANAGER *manager, ListOf_t *params );
    RET_VAL (*SetLocal)(  SBML_SYMTAB_MANAGER *manager, ListOf_t *params );
    BOOL (*LookupValue)( SBML_SYMTAB_MANAGER *manager, char *id, double *value );    
};
*/
struct _SBML_SYMTAB_MANAGER {    
    ListOf_t *global;
    ListOf_t *local;
    COMPILER_RECORD_T *record;
    
    RET_VAL (*SetGlobal)( SBML_SYMTAB_MANAGER *manager, ListOf_t *params );
    RET_VAL (*SetLocal)(  SBML_SYMTAB_MANAGER *manager, ListOf_t *params );
    BOOL (*LookupValue)( SBML_SYMTAB_MANAGER *manager, char *id, double *value );    
    BOOL (*LookupGlobalValue)( SBML_SYMTAB_MANAGER *manager, char *id, double *value );    
    BOOL (*LookupLocalValue)( SBML_SYMTAB_MANAGER *manager, char *id, double *value );
    RET_VAL (*PutParametersInGlobalSymtab)( SBML_SYMTAB_MANAGER *manager, REB2SAC_SYMTAB *globalSymtab );    
    RET_VAL (*UpdateParametersInGlobalSymtab)( SBML_SYMTAB_MANAGER *manager, REB2SAC_SYMTAB *globalSymtab );    
};

SBML_SYMTAB_MANAGER *GetSymtabManagerInstance( COMPILER_RECORD_T *record );
RET_VAL CloseSymtabManager();


END_C_NAMESPACE

#endif
