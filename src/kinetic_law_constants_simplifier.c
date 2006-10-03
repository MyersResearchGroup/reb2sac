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
#include "abstraction_method_manager.h"
#include "reaction_node.h"
#include "kinetic_law.h"
#include <math.h>

static char * _GetKineticLawConstantsSimplifierMethodID( ABSTRACTION_METHOD *method );
static RET_VAL _ApplyKineticLawConstantsSimplifierMethod( ABSTRACTION_METHOD *method, IR *ir );      

static RET_VAL _SimplifyKineticLaw( ABSTRACTION_METHOD *method, IR *ir, REACTION *reaction );
/*
static BOOL _TransformKineticLaw( KINETIC_LAW *kineticLaw, double *result );
static BOOL _TransformOpKineticLaw( KINETIC_LAW *kineticLaw, double *result  );
*/
static RET_VAL _VisitOpToSimplifyKineticLaw( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitIntToSimplifyKineticLaw( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitRealToSimplifyKineticLaw( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSpeciesToSimplifyKineticLaw( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSymbolToSimplifyKineticLaw( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );


ABSTRACTION_METHOD *KineticLawConstantsSimplifierMethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager ) {
    static ABSTRACTION_METHOD method;
    
    START_FUNCTION("KineticLawConstantsSimplifierMethodConstructor");

    if( method.manager == NULL ) {
        method.manager = manager;
        method.GetID = _GetKineticLawConstantsSimplifierMethodID;
        method.Apply = _ApplyKineticLawConstantsSimplifierMethod;       
    }
    
    TRACE_0( "KineticLawConstantsSimplifierMethodConstructor invoked" );
    
    END_FUNCTION("KineticLawConstantsSimplifierMethodConstructor", SUCCESS );
    return &method;
}



static char * _GetKineticLawConstantsSimplifierMethodID( ABSTRACTION_METHOD *method ) {
    START_FUNCTION("_GetKineticLawConstantsSimplifierMethodID");
    
    END_FUNCTION("_GetKineticLawConstantsSimplifierMethodID", SUCCESS );
    return "kinetic-law-constants-simplifier";
}



static RET_VAL _ApplyKineticLawConstantsSimplifierMethod( ABSTRACTION_METHOD *method, IR *ir ) {
    RET_VAL ret = SUCCESS;
    REACTION *reaction = NULL;
    LINKED_LIST *reactionList = NULL;
    
    START_FUNCTION("_ApplyKineticLawConstantsSimplifierMethod");
    
    reactionList = ir->GetListOfReactionNodes( ir );
    ResetCurrentElement( reactionList );    
    while( ( reaction = (REACTION*)GetNextFromLinkedList( reactionList ) ) ) {
        if( IS_FAILED( ( ret = _SimplifyKineticLaw( method, ir, reaction ) ) ) ) {
            END_FUNCTION("_ApplyKineticLawConstantsSimplifierMethod", ret );
            return ret;
        }
    }
         
    END_FUNCTION("_ApplyKineticLawConstantsSimplifierMethod", SUCCESS );
    return ret;
}      

static RET_VAL _SimplifyKineticLaw( ABSTRACTION_METHOD *method, IR *ir, REACTION *reaction ) {
    static KINETIC_LAW_VISITOR visitor;
    RET_VAL ret = SUCCESS;    
    KINETIC_LAW *kineticLaw = NULL;
#ifdef DEBUG
    STRING *kineticLawString = NULL;
#endif        

    START_FUNCTION("_SimplifyKineticLaw");

    if( visitor.VisitOp == NULL ) {
        visitor.VisitOp = _VisitOpToSimplifyKineticLaw;
        visitor.VisitInt = _VisitIntToSimplifyKineticLaw;
        visitor.VisitReal = _VisitRealToSimplifyKineticLaw;
        visitor.VisitSpecies = _VisitSpeciesToSimplifyKineticLaw;
        visitor.VisitSymbol = _VisitSymbolToSimplifyKineticLaw;
    }    

    kineticLaw = GetKineticLawInReactionNode( reaction );
    
    if( IS_FAILED( ( ret = kineticLaw->Accept( kineticLaw, &visitor ) ) ) ) {    
        END_FUNCTION("_SimplifyKineticLaw", ret );
        return ret;
    }     
#ifdef DEBUG
    kineticLawString = ToStringKineticLaw( kineticLaw );
    printf("kinetic law of %s is: %s" NEW_LINE, GetCharArrayOfString( GetReactionNodeName( reaction ) ), GetCharArrayOfString( kineticLawString ) );
    FreeString( &kineticLawString );
#endif        
    END_FUNCTION("_SimplifyKineticLaw", SUCCESS );
    return ret;
}

static RET_VAL _VisitOpToSimplifyKineticLaw( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    double result = 0.0;
    double leftValue = 0.0;
    double rightValue = 0.0;
    KINETIC_LAW *left = NULL;    
    KINETIC_LAW *right = NULL;
    REB2SAC_SYMBOL *sym = NULL;    
        
    START_FUNCTION("_VisitOpToSimplifyKineticLaw");
    
    left = GetOpLeftFromKineticLaw( kineticLaw );
    if( IS_FAILED( ( ret = left->Accept( left, visitor ) ) ) ) {
        END_FUNCTION("_VisitIntToSimplifyKineticLaw", ret );
        return ret;
    }
    
    right = GetOpRightFromKineticLaw( kineticLaw );
    if( IS_FAILED( ( ret = right->Accept( right, visitor ) ) ) ) {
        END_FUNCTION("_VisitIntToSimplifyKineticLaw", ret );
        return ret;
    }
    
    
    if( !IsConstantValueKineticLaw( left ) ) {
        END_FUNCTION("_VisitOpToSimplifyKineticLaw", SUCCESS );
        return ret;
    }
    if( !IsConstantValueKineticLaw( right ) ) {
        END_FUNCTION("_VisitOpToSimplifyKineticLaw", SUCCESS );
        return ret;
    }
    
    if( IsRealValueKineticLaw( left ) ) {
        leftValue = GetRealValueFromKineticLaw( left );
    }
    else if( IsSymbolKineticLaw( left ) ) {
#if 1
        sym = GetSymbolFromKineticLaw( left );
        if( !IsRealValueSymbol( sym ) ) {
            END_FUNCTION("_VisitOpToSimplifyKineticLaw", SUCCESS );
            return ret;
        }
        leftValue = GetRealValueInSymbol( sym );
#endif
    }
    else if( IsIntValueKineticLaw( left ) ) {
        leftValue = (double)GetIntValueFromKineticLaw( left );
    }
    
    if( IsRealValueKineticLaw( right ) ) {
        rightValue = GetRealValueFromKineticLaw( right );
    }
    else if( IsSymbolKineticLaw( right ) ) {
#if 1
        sym = GetSymbolFromKineticLaw( right );
        if( !IsRealValueSymbol( sym ) ) {
            END_FUNCTION("_VisitOpToSimplifyKineticLaw", SUCCESS );
            return ret;
        }
        rightValue = GetRealValueInSymbol( sym );
#endif
    }
    else if( IsIntValueKineticLaw( right ) ) {
        rightValue = (double)GetIntValueFromKineticLaw( right );
    }
        
    switch( GetOpTypeFromKineticLaw( kineticLaw ) ) {
        case KINETIC_LAW_OP_PLUS:
            result = leftValue + rightValue;
        break;
                
        case KINETIC_LAW_OP_MINUS:
            result = leftValue - rightValue;
        break;
        
        case KINETIC_LAW_OP_TIMES:
            result = leftValue * rightValue;
        break;
        
        case KINETIC_LAW_OP_DIVIDE:
            result = leftValue / rightValue;
        break;
        
        case KINETIC_LAW_OP_POW:
            result = pow( leftValue, rightValue );
        break;        
        
        default:
        return ErrorReport( FAILING, "_VisitOpToSimplifyKineticLaw", "invalid operator type" );        
    }
    
    if( IS_FAILED( ( ret = SetRealValueKineticLaw( kineticLaw, result ) ) ) ) {
        END_FUNCTION("_VisitOpToSimplifyKineticLaw", ret );
        return ret;
    } 
            
    END_FUNCTION("_VisitOpToSimplifyKineticLaw", SUCCESS );
    return ret;
}

static RET_VAL _VisitIntToSimplifyKineticLaw( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;        
    END_FUNCTION("_VisitIntToSimplifyKineticLaw", SUCCESS );
    return ret;
}

static RET_VAL _VisitRealToSimplifyKineticLaw( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;    
    START_FUNCTION("_VisitRealToSimplifyKineticLaw");
    END_FUNCTION("_VisitRealToSimplifyKineticLaw", SUCCESS );
    return ret;
}

static RET_VAL _VisitSpeciesToSimplifyKineticLaw( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;    
    START_FUNCTION("_VisitSpeciesToSimplifyKineticLaw");
    END_FUNCTION("_VisitSpeciesToSimplifyKineticLaw", SUCCESS );
    return ret;
}

static RET_VAL _VisitSymbolToSimplifyKineticLaw( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;    
    START_FUNCTION("_VisitSymbolToSimplifyKineticLaw");
    END_FUNCTION("_VisitSymbolToSimplifyKineticLaw", SUCCESS );
    return ret;
}


