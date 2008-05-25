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
#include "rule_manager.h"
#include "linked_list.h"

static RULE_MANAGER manager;


static RULE * _CreateRule( RULE_MANAGER *manager, BYTE type, char *var );
static RULE * _LookupRule( RULE_MANAGER *manager, char *var );
static LINKED_LIST *_CreateListOfRules( RULE_MANAGER *manager );                  


RULE_MANAGER *GetRuleManagerInstance( COMPILER_RECORD_T *record ) {
    
    START_FUNCTION("GetRuleManagerInstance");

    if( manager.record == NULL ) {
        manager.record = record;    
        if( ( manager.rules = CreateLinkedList( ) ) == NULL ) {
            END_FUNCTION("GetRuleManagerInstance", FAILING );
            return NULL;
        }    
        manager.CreateRule = _CreateRule;
        manager.CreateListOfRules = _CreateListOfRules;
    }
        
    END_FUNCTION("GetRuleManagerInstance", SUCCESS );
    return &manager;
}


RET_VAL CloseRuleManager(  ) {
    RET_VAL ret = SUCCESS;
    RULE *ruleDef = NULL;    
    
    START_FUNCTION("CloseRuleManager");
    
    if( manager.record == NULL ) {
        END_FUNCTION("CloseRuleManager", SUCCESS );
        return ret;
    }
     
    while( ( ruleDef = (RULE*)GetNextFromLinkedList( manager.rules ) ) != NULL ) {
        FreeString( &( ruleDef->var ) );
	FreeKineticLaw( &(ruleDef->math) );
        FREE( ruleDef );
    }
    DeleteLinkedList( &(manager.rules) );
    manager.record = NULL;       
    
    END_FUNCTION("CloseRuleManager", SUCCESS );
    return ret;
}

STRING *GetRuleVar( RULE *ruleDef ) {
    START_FUNCTION("GetRuleVar");
            
    END_FUNCTION("GetRuleVar", SUCCESS );
    return (ruleDef == NULL ? NULL : ruleDef->var);
}

BYTE GetRuleType( RULE *ruleDef ) {
    START_FUNCTION("GetRuleType");
            
    END_FUNCTION("GetRuleType", SUCCESS );
    return ruleDef->type;
}

KINETIC_LAW *GetMathInRule( RULE *ruleDef ) {
    START_FUNCTION("GetMathInRule");
            
    END_FUNCTION("GetMathInRule", SUCCESS );
    return (ruleDef == NULL ? NULL : ruleDef->math);
}

BYTE GetRuleVarType( RULE *ruleDef ) {
    START_FUNCTION("GetRuleVarType");
            
    END_FUNCTION("GetRuleVarType", SUCCESS );
    return ruleDef->varType;
}

UINT32 GetRuleIndex( RULE *ruleDef ) {
    START_FUNCTION("GetRuleIndex");
            
    END_FUNCTION("GetRuleIndex", SUCCESS );
    return ruleDef->index;
}

double GetRuleCurValue( RULE *ruleDef ) {
    START_FUNCTION("GetRuleCurValue");
            
    END_FUNCTION("GetRuleCurValue", SUCCESS );
    return ruleDef->curValue;
}

RET_VAL AddMathInRule( RULE *ruleDef, KINETIC_LAW *math ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("AddMathInRule");

    ruleDef->math = math;

    END_FUNCTION("AddMathInRule", SUCCESS );
    return ret;
}

RET_VAL SetRuleVarType( RULE *ruleDef, BYTE varType ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("SetRuleVarType");

    ruleDef->varType = varType;

    END_FUNCTION("SetRuleVarType", SUCCESS );
    return ret;
}

RET_VAL SetRuleIndex( RULE *ruleDef, UINT32 index ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("SetRuleIndex");

    ruleDef->index = index;

    END_FUNCTION("SetRuleIndex", SUCCESS );
    return ret;
}

RET_VAL SetRuleCurValue( RULE *ruleDef, double curValue ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("SetRuleCurValue");

    ruleDef->curValue = curValue;

    END_FUNCTION("SetRuleCurValue", SUCCESS );
    return ret;
}

static RULE * _CreateRule( RULE_MANAGER *manager, BYTE type, char *var ) {
    RULE *ruleDef = NULL;
    
    START_FUNCTION("_CreateRule");

    if( ( ruleDef = (RULE*)MALLOC( sizeof(RULE) ) ) == NULL ) {
        END_FUNCTION("_CreateRule", FAILING );
        return NULL;
    }
    ruleDef->type = type;
    if (var == NULL) {
      ruleDef->var = NULL;
    } else if( ( ruleDef->var = CreateString( var ) ) == NULL ) {
        FREE( ruleDef );
        END_FUNCTION("_CreateRule", FAILING );
        return NULL;
    }
        
    if( IS_FAILED( AddElementInLinkedList( (CADDR_T)ruleDef, manager->rules ) ) ) {
        FREE( ruleDef );
        END_FUNCTION("_CreateRule", FAILING );
        return NULL;
    } 
    
    END_FUNCTION("_CreateRule", SUCCESS );
    return ruleDef;
}
    
static LINKED_LIST *_CreateListOfRules( RULE_MANAGER *manager ) {
    
    START_FUNCTION("_CreateListOfRules");
    
    END_FUNCTION("_CreateListOfRules", SUCCESS );
    return manager->rules;
}                  
    
