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
#if defined(DEBUG)
#undef DEBUG
#endif

#include "abstraction_method_manager.h"
#include "IR.h"

static char * _GetDistributeMethodID( ABSTRACTION_METHOD *method );
static RET_VAL _ApplyDistributeMethod( ABSTRACTION_METHOD *method, IR *ir );      
static BOOL _IsConditionSatisfied( ABSTRACTION_METHOD *method, IR *ir, REACTION *reaction ); 
static RET_VAL _DoTransformation( ABSTRACTION_METHOD *method, IR *ir, REACTION *reaction );



ABSTRACTION_METHOD *DistributeMethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager ) {
    static ABSTRACTION_METHOD method;
    
    START_FUNCTION("DistributeMethodConstructor");

    if( method.manager == NULL ) {
        method.manager = manager;
        method.GetID = _GetDistributeMethodID;
        method.Apply = _ApplyDistributeMethod;
    }
    
    TRACE_0( "DistributeMethodConstructor invoked" );
    
    END_FUNCTION("DistributeMethodConstructor", SUCCESS );
    return &method;
}



static char * _GetDistributeMethodID( ABSTRACTION_METHOD *method ) {
    START_FUNCTION("_GetDistributeMethodID");
    
    END_FUNCTION("_GetDistributeMethodID", SUCCESS );
    return "distribute-transformer";
}



static RET_VAL _ApplyDistributeMethod( ABSTRACTION_METHOD *method, IR *ir ) {
    RET_VAL ret = SUCCESS;
    REACTION *reaction = NULL;
    LINKED_LIST *reactionList = NULL;
    
    START_FUNCTION("_ApplyDistributeMethod");

    reactionList = ir->GetListOfReactionNodes( ir );    
    ResetCurrentElement( reactionList );    
    while( ( reaction = (REACTION*)GetNextFromLinkedList( reactionList ) ) != NULL ) {
        if( _IsConditionSatisfied( method, ir, reaction ) ) {
            if( IS_FAILED( ( ret = _DoTransformation( method, ir, reaction ) ) ) ) {
                END_FUNCTION("_ApplyDistributeMethod", ret );
                return ret;
            }
        }            
    }
    
    END_FUNCTION("_ApplyDistributeMethod", SUCCESS );
    return ret;
}      


static BOOL _IsConditionSatisfied( ABSTRACTION_METHOD *method, IR *ir, REACTION *reaction ) {
    KINETIC_LAW *kineticLaw = NULL;
    KINETIC_LAW *kineticLawLeft = NULL;
    KINETIC_LAW *kineticLawRight = NULL;
    
    START_FUNCTION("_IsConditionSatisfied");

    kineticLaw = GetKineticLawInReactionNode( reaction );
    if( !IsOpKineticLaw( kineticLaw ) ) {
        END_FUNCTION("_IsConditionSatisfied", SUCCESS );
        return FALSE;
    }
    
    if( KINETIC_LAW_OP_TIMES != GetOpTypeFromKineticLaw( kineticLaw ) ) {
        END_FUNCTION("_IsConditionSatisfied", SUCCESS );
        return FALSE;
    }

    kineticLawLeft = GetOpLeftFromKineticLaw( kineticLaw );
    if( !IsCompartmentKineticLaw( kineticLawLeft ) ) {
        END_FUNCTION("_IsConditionSatisfied", SUCCESS );
        return FALSE;
    }

    kineticLawRight = GetOpRightFromKineticLaw( kineticLaw );
    if( !IsOpKineticLaw( kineticLawRight ) ) {
        END_FUNCTION("_IsConditionSatisfied", SUCCESS );
        return FALSE;
    }
    
    if( KINETIC_LAW_OP_MINUS != GetOpTypeFromKineticLaw( kineticLawRight ) ) {
        END_FUNCTION("_IsConditionSatisfied", SUCCESS );
        return FALSE;
    }

    END_FUNCTION("_IsConditionSatisfied", SUCCESS );
    return TRUE;
}

static RET_VAL _DoTransformation( ABSTRACTION_METHOD *method, IR *ir, REACTION *reaction ) {
    RET_VAL ret = SUCCESS;
    char buf[1024];
    KINETIC_LAW *kineticLaw = NULL;
    KINETIC_LAW *kineticLawLeft = NULL;
    KINETIC_LAW *kineticLawRight = NULL;
    KINETIC_LAW *constKineticLaw1 = NULL;
    KINETIC_LAW *constKineticLaw2 = NULL;
    KINETIC_LAW *forwardKineticLaw = NULL;
    KINETIC_LAW *backwardKineticLaw = NULL;
    
    START_FUNCTION("_DoTransformation");

    kineticLaw = GetKineticLawInReactionNode( reaction );
    if( ( constKineticLaw1 = CloneKineticLaw( GetOpLeftFromKineticLaw( kineticLaw ) ) ) == NULL ) {
        return ErrorReport( FAILING, "_DoTransformation", "failed to clone the forward kinetic law of %s", GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
    }
    if( ( constKineticLaw2 = CloneKineticLaw( GetOpLeftFromKineticLaw( kineticLaw ) ) ) == NULL ) {
        return ErrorReport( FAILING, "_DoTransformation", "failed to clone the forward kinetic law of %s", GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
    }
    kineticLawRight = GetOpRightFromKineticLaw( kineticLaw );
    if( ( forwardKineticLaw = CloneKineticLaw( GetOpLeftFromKineticLaw( kineticLawRight ) ) ) == NULL ) {
        return ErrorReport( FAILING, "_DoTransformation", "failed to clone the forward kinetic law of %s", GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
    }
    if( ( backwardKineticLaw = CloneKineticLaw( GetOpRightFromKineticLaw( kineticLawRight ) ) ) == NULL ) {
        return ErrorReport( FAILING, "_DoTransformation", "failed to clone the backward kinetic law of %s", GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
    }
    if( ( kineticLawLeft = CreateOpKineticLaw( KINETIC_LAW_OP_TIMES, constKineticLaw1, forwardKineticLaw ) ) == NULL ) {
      //FreeKineticLaw( &right );
      //FreeKineticLaw( &left );
      END_FUNCTION("_DoTransformation", FAILING );
      return NULL;
    }
    if( ( kineticLawRight = CreateOpKineticLaw( KINETIC_LAW_OP_TIMES, constKineticLaw2, backwardKineticLaw ) ) == NULL ) {
      //FreeKineticLaw( &right );
      //FreeKineticLaw( &left );
      END_FUNCTION("_DoTransformation", FAILING );
      return NULL;
    }
    if( ( kineticLaw = CreateOpKineticLaw( KINETIC_LAW_OP_MINUS, kineticLawLeft, kineticLawRight ) ) == NULL ) {
      //FreeKineticLaw( &right );
      //FreeKineticLaw( &left );
      END_FUNCTION("_DoTransformation", FAILING );
      return NULL;
    }

    if( IS_FAILED( ( ret = SetKineticLawInReactionNode( reaction, kineticLaw ) ) ) ) {
        END_FUNCTION("_DoTransformation", ret );
        return ret;
    }
        
    END_FUNCTION("_DoTransformation", SUCCESS );
    return ret;
}


