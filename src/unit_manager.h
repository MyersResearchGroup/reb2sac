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
#if !defined(HAVE_UNIT_MANAGER)
#define HAVE_UNIT_MANAGER

#include "common.h"
#include "linked_list.h"
#include "util.h"

#include "compiler_def.h"
#include "hash_table.h"

BEGIN_C_NAMESPACE

typedef struct {
    STRING *kind;
    double exponent;
    int scale;
    double multiplier;
} UNIT;

typedef struct {
    STRING *id;
    BOOL builtIn;
    LINKED_LIST *units;
} UNIT_DEFINITION;


struct _UNIT_MANAGER;
typedef struct _UNIT_MANAGER UNIT_MANAGER;

struct _UNIT_MANAGER {
    HASH_TABLE *table;   
    COMPILER_RECORD_T *record;
    
    UNIT_DEFINITION * (*CreateUnitDefinition)( UNIT_MANAGER *manager, char *id, BOOL builtIn );
    UNIT_DEFINITION * (*LookupUnitDefinition)( UNIT_MANAGER *manager, char *id );
    LINKED_LIST *(*CreateListOfUnitDefinitions)( UNIT_MANAGER *manager );                  
};



STRING *GetUnitDefinitionID( UNIT_DEFINITION *unitDef );
LINKED_LIST *GetUnitsInUnitDefinition( UNIT_DEFINITION *unitDef );
RET_VAL AddUnitInUnitDefinition( UNIT_DEFINITION *unitDef, char *kind, double exponent, int scale, double multiplier );
BOOL IsBuiltInUnitDefinition( UNIT_DEFINITION *unitDef );

STRING *GetKindInUnit( UNIT *unit );
double GetExponentInUnit( UNIT *unit );
int GetScaleInUnit( UNIT *unit );
double GetMultiplierInUnit( UNIT *unit );

UNIT_MANAGER *GetUnitManagerInstance( COMPILER_RECORD_T *record );
RET_VAL CloseUnitManager( );

END_C_NAMESPACE

#endif
