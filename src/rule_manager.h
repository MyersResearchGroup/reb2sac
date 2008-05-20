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
#if !defined(HAVE_RULE_MANAGER)
#define HAVE_RULE_MANAGER

#include "common.h"
#include "linked_list.h"
#include "util.h"

#include "compiler_def.h"
#include "hash_table.h"
#include "kinetic_law.h"

BEGIN_C_NAMESPACE

#define RULE_TYPE_ALGEBRAIC ((BYTE)1)
#define RULE_TYPE_ASSIGNMENT ((BYTE)2)
#define RULE_TYPE_RATE ((BYTE)3)   

#define SPECIES_RULE ((BYTE)1)
#define COMPARTMENT_RULE ((BYTE)2)
#define PARAMETER_RULE ((BYTE)3)   

typedef struct {
    BYTE type;
    STRING *var;
    KINETIC_LAW *math;
    BYTE varType;
    UINT32 index;
    double curValue;
} RULE;

struct _RULE_MANAGER;
typedef struct _RULE_MANAGER RULE_MANAGER;

struct _RULE_MANAGER {
    LINKED_LIST *rules;   
    COMPILER_RECORD_T *record;
    
    RULE * (*CreateRule)( RULE_MANAGER *manager, BYTE type, char *var );
    LINKED_LIST *(*CreateListOfRules)( RULE_MANAGER *manager );                  
};

BYTE GetRuleType( RULE *ruleDef );
STRING *GetRuleVar( RULE *ruleDef );
KINETIC_LAW *GetMathInRule( RULE *ruleDef );
BYTE GetRuleVarType( RULE *ruleDef );
UINT32 GetRuleIndex( RULE *ruleDef );
double GetRuleCurValue( RULE *ruleDef );
RET_VAL AddMathInRule( RULE *ruleDef, KINETIC_LAW *math );
RET_VAL SetRuleVarType( RULE *ruleDef, BYTE varType );
RET_VAL SetRuleIndex( RULE *ruleDef, UINT32 index );
RET_VAL SetRuleCurValue( RULE *ruleDef, double curValue );

RULE_MANAGER *GetRuleManagerInstance( COMPILER_RECORD_T *record );
RET_VAL CloseRuleManager( );

END_C_NAMESPACE

#endif
