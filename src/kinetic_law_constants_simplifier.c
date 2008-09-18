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
#include "species_node.h"
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
static RET_VAL _VisitPWToSimplifyKineticLaw( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitOpToSimplifyKineticLaw( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitUnaryOpToSimplifyKineticLaw( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitIntToSimplifyKineticLaw( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitRealToSimplifyKineticLaw( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSpeciesToSimplifyKineticLaw( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitCompartmentToSimplifyKineticLaw( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSymbolToSimplifyKineticLaw( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitFunctionSymbolToSimplifyKineticLaw( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitPWToSimplifyInitial( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitOpToSimplifyInitial( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitUnaryOpToSimplifyInitial( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );

static RET_VAL _VisitIntToSimplify( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitRealToSimplify( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSpeciesToSimplify( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitCompartmentToSimplify( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSymbolToSimplify( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );

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

RET_VAL SimplifyInitialAssignment( KINETIC_LAW *kineticLaw ) {
    static KINETIC_LAW_VISITOR visitor;
    RET_VAL ret = SUCCESS;    
#ifdef DEBUG
    STRING *kineticLawString = NULL;
#endif        

    START_FUNCTION("_SimplifyKineticLaw");

    if( visitor.VisitOp == NULL ) {
        visitor.VisitPW = _VisitPWToSimplifyInitial;
        visitor.VisitOp = _VisitOpToSimplifyInitial;
        visitor.VisitUnaryOp = _VisitUnaryOpToSimplifyInitial;
        visitor.VisitInt = _VisitIntToSimplify;
        visitor.VisitReal = _VisitRealToSimplify;
        visitor.VisitSpecies = _VisitSpeciesToSimplify;
        visitor.VisitCompartment = _VisitCompartmentToSimplify;
        visitor.VisitSymbol = _VisitSymbolToSimplify;
    }    

    if( IS_FAILED( ( ret = kineticLaw->Accept( kineticLaw, &visitor ) ) ) ) {    
        END_FUNCTION("_SimplifyKineticLaw", ret );
        return ret;
    }     
#ifdef DEBUG
    kineticLawString = ToStringKineticLaw( kineticLaw );
    printf("initial assignment is: %s" NEW_LINE, GetCharArrayOfString( kineticLawString ) );
    FreeString( &kineticLawString );
#endif        
    END_FUNCTION("_SimplifyKineticLaw", SUCCESS );
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
        visitor.VisitPW = _VisitPWToSimplifyKineticLaw;
        visitor.VisitOp = _VisitOpToSimplifyKineticLaw;
        visitor.VisitUnaryOp = _VisitUnaryOpToSimplifyKineticLaw;
        visitor.VisitInt = _VisitIntToSimplifyKineticLaw;
        visitor.VisitReal = _VisitRealToSimplifyKineticLaw;
        visitor.VisitSpecies = _VisitSpeciesToSimplifyKineticLaw;
        visitor.VisitCompartment = _VisitCompartmentToSimplifyKineticLaw;
        visitor.VisitSymbol = _VisitSymbolToSimplifyKineticLaw;
        visitor.VisitFunctionSymbol = _VisitFunctionSymbolToSimplifyKineticLaw;
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

static RET_VAL _VisitPWToSimplifyInitial( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    double result = 0.0;
    double childValue = 0.0;
    KINETIC_LAW *child = NULL;
    REB2SAC_SYMBOL *sym = NULL;    
    SPECIES *species = NULL;
    COMPARTMENT *compartment = NULL;
    LINKED_LIST *children;
    UINT num = 0;
    UINT i = 0;

    START_FUNCTION("_VisitPWToSimplifyKineticLaw");
    
    children = GetPWChildrenFromKineticLaw( kineticLaw );
    num = GetLinkedListSize( children );
    for ( i = 1; i < num; i+=2 ) {
      child = (KINETIC_LAW*)GetElementByIndex( i,children );
      if( IS_FAILED( ( ret = child->Accept( child, visitor ) ) ) ) {
        END_FUNCTION("_VisitPWToSimplifyInitial", ret );
        return ret;
      }
      if( IsRealValueKineticLaw( child ) ) {
        childValue = GetRealValueFromKineticLaw( child );
      }
      else if( IsSymbolKineticLaw( child ) ) {
#if 1
        sym = GetSymbolFromKineticLaw( child );
        if( !IsRealValueSymbol( sym ) ) {
            END_FUNCTION("_VisitOpToSimplifyKineticLaw", SUCCESS );
            return ret;
        }
        childValue = GetRealValueInSymbol( sym );
#endif
      }
      else if( IsIntValueKineticLaw( child ) ) {
        childValue = (double)GetIntValueFromKineticLaw( child );
      }
      else if( IsSpeciesKineticLaw( child ) ) {
	species = GetSpeciesFromKineticLaw( child );
	if (IsInitialQuantityInAmountInSpeciesNode( species )) {
	  childValue = GetInitialAmountInSpeciesNode( species );
	} else {
	  childValue = GetInitialConcentrationInSpeciesNode( species );
	}
      }
      else if( IsCompartmentKineticLaw( child ) ) {
	compartment = GetCompartmentFromKineticLaw( child );
	childValue = GetSizeInCompartment( compartment );
      }
      if (childValue) {
	child = (KINETIC_LAW*)GetElementByIndex( i-1,children );
	if( IS_FAILED( ( ret = child->Accept( child, visitor ) ) ) ) {
	  END_FUNCTION("_VisitPWToSimplifyInitial", ret );
	  return ret;
	}
	if( IsRealValueKineticLaw( child ) ) {
	  childValue = GetRealValueFromKineticLaw( child );
	}
	else if( IsSymbolKineticLaw( child ) ) {
#if 1
	  sym = GetSymbolFromKineticLaw( child );
	  if( !IsRealValueSymbol( sym ) ) {
            END_FUNCTION("_VisitOpToSimplifyKineticLaw", SUCCESS );
            return ret;
	  }
	  childValue = GetRealValueInSymbol( sym );
#endif
	}
	else if( IsIntValueKineticLaw( child ) ) {
	  childValue = (double)GetIntValueFromKineticLaw( child );
	}
	else if( IsSpeciesKineticLaw( child ) ) {
	  species = GetSpeciesFromKineticLaw( child );
	  if (IsInitialQuantityInAmountInSpeciesNode( species )) {
	    childValue = GetInitialAmountInSpeciesNode( species );
	  } else {
	    childValue = GetInitialConcentrationInSpeciesNode( species );
	  }
	}
	else if( IsCompartmentKineticLaw( child ) ) {
	  compartment = GetCompartmentFromKineticLaw( child );
	  childValue = GetSizeInCompartment( compartment );
	}
	result = childValue;
	break;
      }
    }
    if ( i == num ) {
      child = (KINETIC_LAW*)GetElementByIndex( i-1,children );
      if( IS_FAILED( ( ret = child->Accept( child, visitor ) ) ) ) {
	END_FUNCTION("_VisitPWToSimplifyInitial", ret );
	return ret;
      }
      if( IsRealValueKineticLaw( child ) ) {
	childValue = GetRealValueFromKineticLaw( child );
      }
      else if( IsSymbolKineticLaw( child ) ) {
#if 1
	sym = GetSymbolFromKineticLaw( child );
	if( !IsRealValueSymbol( sym ) ) {
	  END_FUNCTION("_VisitOpToSimplifyKineticLaw", SUCCESS );
	  return ret;
	}
	childValue = GetRealValueInSymbol( sym );
#endif
      }
      else if( IsIntValueKineticLaw( child ) ) {
	childValue = (double)GetIntValueFromKineticLaw( child );
      }
      else if( IsSpeciesKineticLaw( child ) ) {
	species = GetSpeciesFromKineticLaw( child );
	if (IsInitialQuantityInAmountInSpeciesNode( species )) {
	  childValue = GetInitialAmountInSpeciesNode( species );
	} else {
	  childValue = GetInitialConcentrationInSpeciesNode( species );
	}
      }
      else if( IsCompartmentKineticLaw( child ) ) {
	compartment = GetCompartmentFromKineticLaw( child );
	childValue = GetSizeInCompartment( compartment );
      }
      result = childValue;
    }
    if( IS_FAILED( ( ret = SetRealValueKineticLaw( kineticLaw, result ) ) ) ) {
        END_FUNCTION("_VisitPWToSimplifyInitial", ret );
        return ret;
    } 
            
    END_FUNCTION("_VisitPWToSimplifyInitial", SUCCESS );
    return ret;
}

static RET_VAL _VisitOpToSimplifyInitial( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    double result = 0.0;
    double leftValue = 0.0;
    double rightValue = 0.0;
    KINETIC_LAW *left = NULL;    
    KINETIC_LAW *right = NULL;
    REB2SAC_SYMBOL *sym = NULL;    
    SPECIES *species = NULL;
    COMPARTMENT *compartment = NULL;
 
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
    else if( IsSpeciesKineticLaw( left ) ) {
      species = GetSpeciesFromKineticLaw( left );
      if (IsInitialQuantityInAmountInSpeciesNode( species )) {
	leftValue = GetInitialAmountInSpeciesNode( species );
      } else {
	leftValue = GetInitialConcentrationInSpeciesNode( species );
      }
    }
    else if( IsCompartmentKineticLaw( left ) ) {
      compartment = GetCompartmentFromKineticLaw( left );
      leftValue = GetSizeInCompartment( compartment );
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
    else if( IsSpeciesKineticLaw( right ) ) {
      species = GetSpeciesFromKineticLaw( right );
      if (IsInitialQuantityInAmountInSpeciesNode( species )) {
	rightValue = GetInitialAmountInSpeciesNode( species );
      } else {
	rightValue = GetInitialConcentrationInSpeciesNode( species );
      }
    }
    else if( IsCompartmentKineticLaw( right ) ) {
      compartment = GetCompartmentFromKineticLaw( right );
      rightValue = GetSizeInCompartment( compartment );
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
        
        case KINETIC_LAW_OP_ROOT:
	  result = pow(rightValue,(1./leftValue));
        break;        

        case KINETIC_LAW_OP_XOR:
	  result = (!leftValue && rightValue)||(leftValue && !rightValue);
        break;        

        case KINETIC_LAW_OP_AND:
            result = leftValue && rightValue;
        break;        

        case KINETIC_LAW_OP_OR:
            result = leftValue || rightValue;
        break;        

        case KINETIC_LAW_OP_EQ:
	  result = (leftValue == rightValue);
        break;

        case KINETIC_LAW_OP_NEQ:
	  result = (leftValue != rightValue);
        break;

        case KINETIC_LAW_OP_GEQ:
	  result = (leftValue >= rightValue);
        break;

        case KINETIC_LAW_OP_GT:
	  result = (leftValue > rightValue);
        break;

        case KINETIC_LAW_OP_LEQ:
	  result = (leftValue <= rightValue);
        break;

        case KINETIC_LAW_OP_LT:
	  result = (leftValue < rightValue);
        break;

        case KINETIC_LAW_OP_UNIFORM:
	  result = GetNextUniformRandomNumber(leftValue,rightValue);
	break;

        case KINETIC_LAW_OP_NORMAL:
	  result = GetNextNormalRandomNumber(leftValue,rightValue);
	break;

        case KINETIC_LAW_OP_BINOMIAL:
	  result = GetNextBinomialRandomNumber(leftValue,(unsigned int)rightValue);
	break;

        case KINETIC_LAW_OP_GAMMA:
	  result = GetNextGammaRandomNumber(leftValue,rightValue);
	break;

        case KINETIC_LAW_OP_LOGNORMAL:
	  result = GetNextLogNormalRandomNumber(leftValue,rightValue);
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

static RET_VAL _VisitUnaryOpToSimplifyInitial( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    double result = 0.0;
    double childValue = 0.0;
    KINETIC_LAW *child = NULL;
    REB2SAC_SYMBOL *sym = NULL;    
    SPECIES *species = NULL;
    COMPARTMENT *compartment = NULL;
    UINT i = 0;

    START_FUNCTION("_VisitUnaryOpToSimplifyKineticLaw");
    
    child = GetUnaryOpChildFromKineticLaw( kineticLaw );
    if( IS_FAILED( ( ret = child->Accept( child, visitor ) ) ) ) {
        END_FUNCTION("_VisitUnaryOpToSimplifyKineticLaw", ret );
        return ret;
    }
    
    if( IsRealValueKineticLaw( child ) ) {
        childValue = GetRealValueFromKineticLaw( child );
    }
    else if( IsSymbolKineticLaw( child ) ) {
#if 1
        sym = GetSymbolFromKineticLaw( child );
        if( !IsRealValueSymbol( sym ) ) {
            END_FUNCTION("_VisitOpToSimplifyKineticLaw", SUCCESS );
            return ret;
        }
        childValue = GetRealValueInSymbol( sym );
#endif
    }
    else if( IsIntValueKineticLaw( child ) ) {
        childValue = (double)GetIntValueFromKineticLaw( child );
    }
    else if( IsSpeciesKineticLaw( child ) ) {
      species = GetSpeciesFromKineticLaw( child );
      if (IsInitialQuantityInAmountInSpeciesNode( species )) {
	childValue = GetInitialAmountInSpeciesNode( species );
      } else {
	childValue = GetInitialConcentrationInSpeciesNode( species );
      }
    }
    else if( IsCompartmentKineticLaw( child ) ) {
      compartment = GetCompartmentFromKineticLaw( child );
      childValue = GetSizeInCompartment( compartment );
    }
        
    switch( GetUnaryOpTypeFromKineticLaw( kineticLaw ) ) {
        case KINETIC_LAW_UNARY_OP_NEG:
	  result = (-1)*childValue;
        break;
        case KINETIC_LAW_UNARY_OP_NOT:
	  result = !childValue;
        break;
        case KINETIC_LAW_UNARY_OP_ABS:
	  result = labs(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_COT:
	  result = (1./tan(childValue));
        break;
        case KINETIC_LAW_UNARY_OP_COTH:
	  result = cosh(childValue)/sinh(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_CSC:
	  result = (1./sin(childValue));
        break;
        case KINETIC_LAW_UNARY_OP_CSCH:
	  result = (1./cosh(childValue));
        break;
        case KINETIC_LAW_UNARY_OP_SEC:
	  result = (1./cos(childValue));
        break;
        case KINETIC_LAW_UNARY_OP_SECH:
	  result = (1./sinh(childValue));
        break;
        case KINETIC_LAW_UNARY_OP_COS:
	  result = cos(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_COSH:
	  result = cosh(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_SIN:
	  result = sin(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_SINH:
	  result = sinh(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_TAN:
	  result = tan(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_TANH:
	  result = tanh(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_ARCCOT:
	  result = atan(1./(childValue));
        break;
        case KINETIC_LAW_UNARY_OP_ARCCOTH:
	  result = ((1./2.)*log((childValue+1.)/(childValue-1.)) );
        break;
        case KINETIC_LAW_UNARY_OP_ARCCSC:
	  result = atan( 1. / SQRT( (childValue-1.)*(childValue+1.) )); 
        break;
        case KINETIC_LAW_UNARY_OP_ARCCSCH:
	  result = log((1.+SQRT((1+SQR(childValue)))) /childValue);
        break;
        case KINETIC_LAW_UNARY_OP_ARCSEC:
	  result = atan( SQRT(( childValue-1.)*( childValue+1.)) );
        break;
        case KINETIC_LAW_UNARY_OP_ARCSECH:
	  result = log((1.+pow((1-SQR(childValue)),0.5))/childValue);
        break;
        case KINETIC_LAW_UNARY_OP_ARCCOS:
	  result = acos(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_ARCCOSH:
	  result = acosh(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_ARCSIN:
	  result = asin(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_ARCSINH:
	  result = asinh(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_ARCTAN:
	  result = atan(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_ARCTANH:
	  result = atanh(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_CEILING:
	  result = ceil(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_EXP:
	  result = exp(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_FACTORIAL:
	  i = floor(childValue);
	  for(result=1;i>1;--i)
	    result *= i;
        break;
        case KINETIC_LAW_UNARY_OP_FLOOR:
	  result = floor(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_LN:
	  result = log(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_LOG:
	  result = log10(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_EXPRAND:
	  result = GetNextExponentialRandomNumber(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_POISSON:
	  result = GetNextPoissonRandomNumber(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_CHISQ:
	  result = GetNextChiSquaredRandomNumber(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_LAPLACE:
	  result = GetNextLaplaceRandomNumber(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_CAUCHY:
	  result = GetNextCauchyRandomNumber(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_RAYLEIGH:
	  result = GetNextRayleighRandomNumber(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_BERNOULLI:
	  result = GetNextBernoulliRandomNumber(childValue);
        break;
        
        default:
        return ErrorReport( FAILING, "_VisitUnaryOpToSimplifyKineticLaw", "invalid operator type" );        
    }
    
    if( IS_FAILED( ( ret = SetRealValueKineticLaw( kineticLaw, result ) ) ) ) {
        END_FUNCTION("_VisitUnaryOpToSimplifyKineticLaw", ret );
        return ret;
    } 
            
    END_FUNCTION("_VisitUnaryOpToSimplifyKineticLaw", SUCCESS );
    return ret;
}

static RET_VAL _VisitPWToSimplifyKineticLaw( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    double result = 0.0;
    double childValue = 0.0;
    KINETIC_LAW *child = NULL;
    REB2SAC_SYMBOL *sym = NULL;    
    SPECIES *species = NULL;
    COMPARTMENT *compartment = NULL;
    LINKED_LIST *children;
    UINT num = 0;
    UINT i = 0;

    START_FUNCTION("_VisitPWToSimplifyKineticLaw");
    
    children = GetPWChildrenFromKineticLaw( kineticLaw );
    num = GetLinkedListSize( children );
    for ( i = 0; i < num; i++ ) {
      child = (KINETIC_LAW*)GetElementByIndex( i,children );
      if( !IsConstantValueKineticLaw( child ) ) {
        END_FUNCTION("_VisitOpToSimplifyKineticLaw", SUCCESS );
        return ret;
      }
    }
    for ( i = 1; i < num; i+=2 ) {
      child = (KINETIC_LAW*)GetElementByIndex( i,children );
      if( IS_FAILED( ( ret = child->Accept( child, visitor ) ) ) ) {
        END_FUNCTION("_VisitPWToSimplifyInitial", ret );
        return ret;
      }
      if( IsRealValueKineticLaw( child ) ) {
        childValue = GetRealValueFromKineticLaw( child );
      }
      else if( IsSymbolKineticLaw( child ) ) {
#if 1
        sym = GetSymbolFromKineticLaw( child );
        if( !IsRealValueSymbol( sym ) ) {
            END_FUNCTION("_VisitOpToSimplifyKineticLaw", SUCCESS );
            return ret;
        }
        childValue = GetRealValueInSymbol( sym );
#endif
      }
      else if( IsIntValueKineticLaw( child ) ) {
        childValue = (double)GetIntValueFromKineticLaw( child );
      }
      else if( IsSpeciesKineticLaw( child ) ) {
	species = GetSpeciesFromKineticLaw( child );
	if (IsInitialQuantityInAmountInSpeciesNode( species )) {
	  childValue = GetInitialAmountInSpeciesNode( species );
	} else {
	  childValue = GetInitialConcentrationInSpeciesNode( species );
	}
      }
      else if( IsCompartmentKineticLaw( child ) ) {
	compartment = GetCompartmentFromKineticLaw( child );
	childValue = GetSizeInCompartment( compartment );
      }
      if (childValue) {
	child = (KINETIC_LAW*)GetElementByIndex( i-1,children );
	if( IS_FAILED( ( ret = child->Accept( child, visitor ) ) ) ) {
	  END_FUNCTION("_VisitPWToSimplifyInitial", ret );
	  return ret;
	}
	if( IsRealValueKineticLaw( child ) ) {
	  childValue = GetRealValueFromKineticLaw( child );
	}
	else if( IsSymbolKineticLaw( child ) ) {
#if 1
	  sym = GetSymbolFromKineticLaw( child );
	  if( !IsRealValueSymbol( sym ) ) {
            END_FUNCTION("_VisitOpToSimplifyKineticLaw", SUCCESS );
            return ret;
	  }
	  childValue = GetRealValueInSymbol( sym );
#endif
	}
	else if( IsIntValueKineticLaw( child ) ) {
	  childValue = (double)GetIntValueFromKineticLaw( child );
	}
	else if( IsSpeciesKineticLaw( child ) ) {
	  species = GetSpeciesFromKineticLaw( child );
	  if (IsInitialQuantityInAmountInSpeciesNode( species )) {
	    childValue = GetInitialAmountInSpeciesNode( species );
	  } else {
	    childValue = GetInitialConcentrationInSpeciesNode( species );
	  }
	}
	else if( IsCompartmentKineticLaw( child ) ) {
	  compartment = GetCompartmentFromKineticLaw( child );
	  childValue = GetSizeInCompartment( compartment );
	}
	result = childValue;
	break;
      }
    }
    if ( i == num ) {
      child = (KINETIC_LAW*)GetElementByIndex( i-1,children );
      if( IS_FAILED( ( ret = child->Accept( child, visitor ) ) ) ) {
	END_FUNCTION("_VisitPWToSimplifyInitial", ret );
	return ret;
      }
      if( IsRealValueKineticLaw( child ) ) {
	childValue = GetRealValueFromKineticLaw( child );
      }
      else if( IsSymbolKineticLaw( child ) ) {
#if 1
	sym = GetSymbolFromKineticLaw( child );
	if( !IsRealValueSymbol( sym ) ) {
	  END_FUNCTION("_VisitOpToSimplifyKineticLaw", SUCCESS );
	  return ret;
	}
	childValue = GetRealValueInSymbol( sym );
#endif
      }
      else if( IsIntValueKineticLaw( child ) ) {
	childValue = (double)GetIntValueFromKineticLaw( child );
      }
      else if( IsSpeciesKineticLaw( child ) ) {
	species = GetSpeciesFromKineticLaw( child );
	if (IsInitialQuantityInAmountInSpeciesNode( species )) {
	  childValue = GetInitialAmountInSpeciesNode( species );
	} else {
	  childValue = GetInitialConcentrationInSpeciesNode( species );
	}
      }
      else if( IsCompartmentKineticLaw( child ) ) {
	compartment = GetCompartmentFromKineticLaw( child );
	childValue = GetSizeInCompartment( compartment );
      }
      result = childValue;
    }
    if( IS_FAILED( ( ret = SetRealValueKineticLaw( kineticLaw, result ) ) ) ) {
        END_FUNCTION("_VisitPWToSimplifyInitial", ret );
        return ret;
    } 
            
    END_FUNCTION("_VisitPWToSimplifyInitial", SUCCESS );
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
        
        case KINETIC_LAW_OP_ROOT:
	  result = pow(rightValue,(1./leftValue));
        break;        

        case KINETIC_LAW_OP_XOR:
	  result = (!leftValue && rightValue)||(leftValue && !rightValue);
        break;        

        case KINETIC_LAW_OP_AND:
            result = leftValue && rightValue;
        break;        

        case KINETIC_LAW_OP_OR:
            result = leftValue || rightValue;
        break;        

        case KINETIC_LAW_OP_EQ:
	  result = (leftValue == rightValue);
        break;

        case KINETIC_LAW_OP_NEQ:
	  result = (leftValue != rightValue);
        break;

        case KINETIC_LAW_OP_GEQ:
	  result = (leftValue >= rightValue);
        break;

        case KINETIC_LAW_OP_GT:
	  result = (leftValue > rightValue);
        break;

        case KINETIC_LAW_OP_LEQ:
	  result = (leftValue <= rightValue);
        break;

        case KINETIC_LAW_OP_LT:
	  result = (leftValue < rightValue);
        break;

        case KINETIC_LAW_OP_UNIFORM:
	  return ret;
	break;

        case KINETIC_LAW_OP_NORMAL:
	  return ret;
	break;

        case KINETIC_LAW_OP_BINOMIAL:
	  return ret;
	break;

        case KINETIC_LAW_OP_GAMMA:
	  return ret;
	break;

        case KINETIC_LAW_OP_LOGNORMAL:
	  return ret;
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

static RET_VAL _VisitUnaryOpToSimplifyKineticLaw( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    double result = 0.0;
    double childValue = 0.0;
    KINETIC_LAW *child = NULL;
    REB2SAC_SYMBOL *sym = NULL;    
    UINT i = 0;

    START_FUNCTION("_VisitUnaryOpToSimplifyKineticLaw");
    
    child = GetUnaryOpChildFromKineticLaw( kineticLaw );
    if( IS_FAILED( ( ret = child->Accept( child, visitor ) ) ) ) {
        END_FUNCTION("_VisitUnaryOpToSimplifyKineticLaw", ret );
        return ret;
    }
    
    if( !IsConstantValueKineticLaw( child ) ) {
        END_FUNCTION("_VisitUnaryOpToSimplifyKineticLaw", SUCCESS );
        return ret;
    }
    
    if( IsRealValueKineticLaw( child ) ) {
        childValue = GetRealValueFromKineticLaw( child );
    }
    else if( IsSymbolKineticLaw( child ) ) {
#if 1
        sym = GetSymbolFromKineticLaw( child );
        if( !IsRealValueSymbol( sym ) ) {
            END_FUNCTION("_VisitOpToSimplifyKineticLaw", SUCCESS );
            return ret;
        }
        childValue = GetRealValueInSymbol( sym );
#endif
    }
    else if( IsIntValueKineticLaw( child ) ) {
        childValue = (double)GetIntValueFromKineticLaw( child );
    }
    
    switch( GetUnaryOpTypeFromKineticLaw( kineticLaw ) ) {
        case KINETIC_LAW_UNARY_OP_NEG:
	  result = (-1)*childValue;
        break;
        case KINETIC_LAW_UNARY_OP_NOT:
	  result = !childValue;
        break;
        case KINETIC_LAW_UNARY_OP_ABS:
	  result = labs(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_COT:
	  result = (1./tan(childValue));
        break;
        case KINETIC_LAW_UNARY_OP_COTH:
	  result = cosh(childValue)/sinh(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_CSC:
	  result = (1./sin(childValue));
        break;
        case KINETIC_LAW_UNARY_OP_CSCH:
	  result = (1./cosh(childValue));
        break;
        case KINETIC_LAW_UNARY_OP_SEC:
	  result = (1./cos(childValue));
        break;
        case KINETIC_LAW_UNARY_OP_SECH:
	  result = (1./sinh(childValue));
        break;
        case KINETIC_LAW_UNARY_OP_COS:
	  result = cos(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_COSH:
	  result = cosh(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_SIN:
	  result = sin(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_SINH:
	  result = sinh(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_TAN:
	  result = tan(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_TANH:
	  result = tanh(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_ARCCOT:
	  result = atan(1./(childValue));
        break;
        case KINETIC_LAW_UNARY_OP_ARCCOTH:
	  result = ((1./2.)*log((childValue+1.)/(childValue-1.)) );
        break;
        case KINETIC_LAW_UNARY_OP_ARCCSC:
	  result = atan( 1. / SQRT( (childValue-1.)*(childValue+1.) )); 
        break;
        case KINETIC_LAW_UNARY_OP_ARCCSCH:
	  result = log((1.+SQRT((1+SQR(childValue)))) /childValue);
        break;
        case KINETIC_LAW_UNARY_OP_ARCSEC:
	  result = atan( SQRT(( childValue-1.)*( childValue+1.)) );
        break;
        case KINETIC_LAW_UNARY_OP_ARCSECH:
	  result = log((1.+pow((1-SQR(childValue)),0.5))/childValue);
        break;
        case KINETIC_LAW_UNARY_OP_ARCCOS:
	  result = acos(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_ARCCOSH:
	  result = acosh(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_ARCSIN:
	  result = asin(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_ARCSINH:
	  result = asinh(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_ARCTAN:
	  result = atan(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_ARCTANH:
	  result = atanh(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_CEILING:
	  result = ceil(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_EXP:
	  result = exp(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_FACTORIAL:
	  i = floor(childValue);
	  for(result=1;i>1;--i)
	    result *= i;
        break;
        case KINETIC_LAW_UNARY_OP_FLOOR:
	  result = floor(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_LN:
	  result = log(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_LOG:
	  result = log10(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_EXPRAND:
	  return ret;
        break;
        case KINETIC_LAW_UNARY_OP_POISSON:
	  return ret;
        break;
        case KINETIC_LAW_UNARY_OP_CHISQ:
	  return ret;
        break;
        case KINETIC_LAW_UNARY_OP_LAPLACE:
	  return ret;
        break;
        case KINETIC_LAW_UNARY_OP_CAUCHY:
	  return ret;
        break;
        case KINETIC_LAW_UNARY_OP_RAYLEIGH:
	  return ret;
        break;
        case KINETIC_LAW_UNARY_OP_BERNOULLI:
	  return ret;
        break;
        
        default:
        return ErrorReport( FAILING, "_VisitUnaryOpToSimplifyKineticLaw", "invalid operator type" );        
    }
    
    if( IS_FAILED( ( ret = SetRealValueKineticLaw( kineticLaw, result ) ) ) ) {
        END_FUNCTION("_VisitUnaryOpToSimplifyKineticLaw", ret );
        return ret;
    } 
            
    END_FUNCTION("_VisitUnaryOpToSimplifyKineticLaw", SUCCESS );
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

static RET_VAL _VisitCompartmentToSimplifyKineticLaw( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;    
    START_FUNCTION("_VisitCompartmentToSimplifyKineticLaw");
    END_FUNCTION("_VisitCompartmentToSimplifyKineticLaw", SUCCESS );
    return ret;
}

static RET_VAL _VisitSymbolToSimplifyKineticLaw( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;    
    START_FUNCTION("_VisitSymbolToSimplifyKineticLaw");
    END_FUNCTION("_VisitSymbolToSimplifyKineticLaw", SUCCESS );
    return ret;
}

static RET_VAL _VisitFunctionSymbolToSimplifyKineticLaw( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;    
    START_FUNCTION("_VisitFunctionSymbolToSimplifyKineticLaw");
    END_FUNCTION("_VisitFunctionSymbolToSimplifyKineticLaw", SUCCESS );
    return ret;
}

static RET_VAL _VisitIntToSimplify( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;        
    double result;

    START_FUNCTION("_VisitIntToSimplify");

    result = (double)GetIntValueFromKineticLaw( kineticLaw );
    
    if( IS_FAILED( ( ret = SetRealValueKineticLaw( kineticLaw, result ) ) ) ) {
        END_FUNCTION("_VisitIntToSimplify", ret );
        return ret;
    } 
    END_FUNCTION("_VisitIntToSimplify", SUCCESS );
    return ret;
}

static RET_VAL _VisitRealToSimplify( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    double result;

    START_FUNCTION("_VisitRealToSimplify");

    result = (double)GetRealValueFromKineticLaw( kineticLaw );
    
    if( IS_FAILED( ( ret = SetRealValueKineticLaw( kineticLaw, result ) ) ) ) {
        END_FUNCTION("_VisitRealToSimplify", ret );
        return ret;
    } 
    END_FUNCTION("_VisitRealToSimplify", SUCCESS );
    return ret;
}

static RET_VAL _VisitSpeciesToSimplify( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;    
    double result;
    SPECIES *species;

    START_FUNCTION("_VisitSpeciesToSimplify");

    species = GetSpeciesFromKineticLaw( kineticLaw );
    if (IsInitialQuantityInAmountInSpeciesNode( species )) {
      result = GetInitialAmountInSpeciesNode( species );
    } else {
      result = GetInitialConcentrationInSpeciesNode( species );
    }
    if( IS_FAILED( ( ret = SetRealValueKineticLaw( kineticLaw, result ) ) ) ) {
        END_FUNCTION("_VisitSpeciesToSimplify", ret );
        return ret;
    } 
    END_FUNCTION("_VisitSpeciesToSimplify", SUCCESS );
    return ret;
}

static RET_VAL _VisitCompartmentToSimplify( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;    
    double result;
    COMPARTMENT *compartment;

    START_FUNCTION("_VisitCompartmentToSimplify");

    compartment = GetCompartmentFromKineticLaw( kineticLaw );
    result = GetSizeInCompartment( compartment );
    if( IS_FAILED( ( ret = SetRealValueKineticLaw( kineticLaw, result ) ) ) ) {
        END_FUNCTION("_VisitCompartmentToSimplify", ret );
        return ret;
    } 
    END_FUNCTION("_VisitCompartmentToSimplify", SUCCESS );
    return ret;
}

static RET_VAL _VisitSymbolToSimplify( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;    
    double result;
    REB2SAC_SYMBOL *sym = NULL;    

    START_FUNCTION("_VisitSymbolToSimplify");

    sym = GetSymbolFromKineticLaw( kineticLaw );
    if( !IsRealValueSymbol( sym ) ) {
      END_FUNCTION("_VisitSymbolToSimplify", SUCCESS );
      return ret;
    }
    result = GetRealValueInSymbol( sym );
    if( IS_FAILED( ( ret = SetRealValueKineticLaw( kineticLaw, result ) ) ) ) {
        END_FUNCTION("_VisitSymbolToSimplify", ret );
        return ret;
    } 
    END_FUNCTION("_VisitSymbolToSimplify", SUCCESS );
    return ret;
}

