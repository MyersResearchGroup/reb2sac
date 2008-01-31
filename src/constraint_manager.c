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
#include "constraint_manager.h"
#include "linked_list.h"

static CONSTRAINT_MANAGER manager;


static CONSTRAINT * _CreateConstraint( CONSTRAINT_MANAGER *manager, char *id );
static LINKED_LIST *_CreateListOfConstraints( CONSTRAINT_MANAGER *manager );                  


CONSTRAINT_MANAGER *GetConstraintManagerInstance( COMPILER_RECORD_T *record ) {
    
    START_FUNCTION("GetConstraintManagerInstance");

    if( manager.record == NULL ) {
        manager.record = record;    
        if( ( manager.constraints = CreateLinkedList( ) ) == NULL ) {
            END_FUNCTION("GetConstraintManagerInstance", FAILING );
            return NULL;
        }    
        manager.CreateConstraint = _CreateConstraint;
        manager.CreateListOfConstraints = _CreateListOfConstraints;
    }
        
    END_FUNCTION("GetConstraintManagerInstance", SUCCESS );
    return &manager;
}


RET_VAL CloseConstraintManager(  ) {
    RET_VAL ret = SUCCESS;
    CONSTRAINT *constraintDef = NULL;    
    
    START_FUNCTION("CloseConstraintManager");
    
    if( manager.record == NULL ) {
        END_FUNCTION("CloseConstraintManager", SUCCESS );
        return ret;
    }
     
    while( ( constraintDef = (CONSTRAINT*)GetNextFromLinkedList( manager.constraints ) ) != NULL ) {
        FreeString( &( constraintDef->id ) );
	FreeKineticLaw( &(constraintDef->math) );
        FreeString( &( constraintDef->message ) );
        FREE( constraintDef );
    }
    DeleteLinkedList( &(manager.constraints) );
    manager.record = NULL;       
    
    END_FUNCTION("CloseConstraintManager", SUCCESS );
    return ret;
}

STRING *GetConstraintId( CONSTRAINT *constraintDef ) {
    START_FUNCTION("GetConstraintId");
            
    END_FUNCTION("GetConstraintId", SUCCESS );
    return (constraintDef == NULL ? NULL : constraintDef->id);
}

KINETIC_LAW *GetMathInConstraint( CONSTRAINT *constraintDef ) {
    START_FUNCTION("GetMathInConstraint");
            
    END_FUNCTION("GetMathInConstraint", SUCCESS );
    return (constraintDef == NULL ? NULL : constraintDef->math);
}

STRING *GetConstraintMessage( CONSTRAINT *constraintDef ) {
    START_FUNCTION("GetConstraintMessage");
            
    END_FUNCTION("GetConstraintMessage", SUCCESS );
    return (constraintDef == NULL ? NULL : constraintDef->message);
}

RET_VAL AddMathInConstraint( CONSTRAINT *constraintDef, KINETIC_LAW *math ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("AddMathInConstraint");

    constraintDef->math = math;

    END_FUNCTION("AddMathInConstraint", SUCCESS );
    return ret;
}

RET_VAL AddMessageInConstraint( CONSTRAINT *constraintDef, char *message ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("AddMessageInConstraint");

    if( ( constraintDef->message = CreateString( message ) ) == NULL ) {
        END_FUNCTION("_AddMessageInConstraint", FAILING );
        return FAILING;
    }

    END_FUNCTION("AddMathInConstraint", SUCCESS );
    return ret;
}

static CONSTRAINT * _CreateConstraint( CONSTRAINT_MANAGER *manager, char *id ) {
    CONSTRAINT *constraintDef = NULL;
    
    START_FUNCTION("_CreateConstraint");

    if( ( constraintDef = (CONSTRAINT*)MALLOC( sizeof(CONSTRAINT) ) ) == NULL ) {
        END_FUNCTION("_CreateConstraint", FAILING );
        return NULL;
    }
    if (id == NULL) {
      constraintDef->id = NULL;
    } else if( ( constraintDef->id = CreateString( id ) ) == NULL ) {
        FREE( constraintDef );
        END_FUNCTION("_CreateConstraint", FAILING );
        return NULL;
    }
    constraintDef->math = NULL;
    constraintDef->message = NULL;

    if( IS_FAILED( AddElementInLinkedList( constraintDef, manager->constraints ) ) ) {
        FREE( constraintDef );
        END_FUNCTION("_CreateConstraint", FAILING );
        return NULL;
    } 
    
    END_FUNCTION("_CreateConstraint", SUCCESS );
    return constraintDef;
}
    
static LINKED_LIST *_CreateListOfConstraints( CONSTRAINT_MANAGER *manager ) {
    
    START_FUNCTION("_CreateListOfConstraints");
    
    END_FUNCTION("_CreateListOfConstraints", SUCCESS );
    return manager->constraints;
}                  
    
