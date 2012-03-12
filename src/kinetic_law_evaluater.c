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

#include "kinetic_law_evaluater.h"


static RET_VAL _SetSpeciesValue( KINETIC_LAW_EVALUATER *evaluater, SPECIES *species, double value );
static RET_VAL _RemoveSpeciesValue( KINETIC_LAW_EVALUATER *evaluater, SPECIES *species );
static RET_VAL _SetDefaultSpeciesValue( KINETIC_LAW_EVALUATER *evaluater, double value ); 
static double _Evaluate( KINETIC_LAW_EVALUATER *evaluater, KINETIC_LAW *kineticLaw );       
static double _EvaluateWithCurrentAmounts( KINETIC_LAW_EVALUATER *evaluater, KINETIC_LAW *kineticLaw );       
static double _EvaluateWithCurrentAmountsDeter( KINETIC_LAW_EVALUATER *evaluater, KINETIC_LAW *kineticLaw );       
static double _EvaluateWithCurrentConcentrations( KINETIC_LAW_EVALUATER *evaluater, KINETIC_LAW *kineticLaw );       
static double _EvaluateWithCurrentConcentrationsDeter( KINETIC_LAW_EVALUATER *evaluater, KINETIC_LAW *kineticLaw );       

static double _GetValue( KINETIC_LAW_EVALUATER *evaluater, SPECIES *species );       

static RET_VAL _VisitPWToEvaluate( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitOpToEvaluate( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitUnaryOpToEvaluate( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitIntToEvaluate( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitRealToEvaluate( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSpeciesToEvaluate( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitCompartmentToEvaluate( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSymbolToEvaluate( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitFunctionSymbolToEvaluate( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitOpToEvaluateDeter( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitUnaryOpToEvaluateDeter( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );

static RET_VAL _VisitCompartmentToEvaluateWithCurrentSize( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSpeciesToEvaluateWithCurrentAmounts( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSpeciesToEvaluateWithCurrentConcentrations( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );


KINETIC_LAW_EVALUATER *CreateKineticLawEvaluater() {
    KINETIC_LAW_EVALUATER *evaluater = NULL;
    
    START_FUNCTION("FreeKineticLawEvaluater");
    
    if( ( evaluater = (KINETIC_LAW_EVALUATER*)MALLOC( sizeof(KINETIC_LAW_EVALUATER) ) ) == NULL ) {
        END_FUNCTION("FreeKineticLawEvaluater", FAILING );        
        return NULL;
    }
    
    if( ( evaluater->table = CreateHashTable( 32 ) ) == NULL ) {
        END_FUNCTION("FreeKineticLawEvaluater", FAILING );        
        return NULL;
    }
    
    evaluater->SetSpeciesValue = _SetSpeciesValue;
    evaluater->RemoveSpeciesValue = _RemoveSpeciesValue;
    evaluater->SetDefaultSpeciesValue = _SetDefaultSpeciesValue;
    evaluater->Evaluate =_Evaluate;
    evaluater->EvaluateWithCurrentAmounts = _EvaluateWithCurrentAmounts;
    evaluater->EvaluateWithCurrentAmountsDeter = _EvaluateWithCurrentAmountsDeter;
    evaluater->EvaluateWithCurrentConcentrations = _EvaluateWithCurrentConcentrations;    
    evaluater->EvaluateWithCurrentConcentrationsDeter = _EvaluateWithCurrentConcentrationsDeter;    
    
    END_FUNCTION("FreeKineticLawEvaluater", SUCCESS );        
    return evaluater;
}

RET_VAL FreeKineticLawEvaluater( KINETIC_LAW_EVALUATER **evaluater ) {
    RET_VAL ret = SUCCESS;
    KINETIC_LAW_EVALUATER *target = NULL;

    START_FUNCTION("FreeKineticLawEvaluater");
    
    target = *evaluater;    
    if( target == NULL ) {
        END_FUNCTION("FreeKineticLawEvaluater", SUCCESS );        
        return ret;
    }
    
    DeleteHashTable( &(target->table) );
    FREE( *evaluater );
        
    END_FUNCTION("FreeKineticLawEvaluater", SUCCESS );        
    return ret;
}


static RET_VAL _SetSpeciesValue( KINETIC_LAW_EVALUATER *evaluater, SPECIES *species, double value ) {
    RET_VAL ret = SUCCESS;
    KINETIC_LAW_EVALUATION_ELEMENT *element = NULL;
    HASH_TABLE *table = NULL;
    
    START_FUNCTION("_SetSpeciesValue");

    table = evaluater->table;    
    if( ( element = (KINETIC_LAW_EVALUATION_ELEMENT*)GetValueFromHashTable( (CADDR_T)species, sizeof( species ), table ) ) != NULL ) {
        element->value = value;
        END_FUNCTION("_SetSpeciesValue", SUCCESS );        
        return ret;
    } 
    
    if( ( element = (KINETIC_LAW_EVALUATION_ELEMENT*)MALLOC( sizeof(KINETIC_LAW_EVALUATION_ELEMENT) ) ) == NULL ) {
        return ErrorReport( FAILING, "", "could not create an evaluation element: species %s value %g", GetCharArrayOfString( GetSpeciesNodeName( species ) ), value );
    } 
    
    element->species = species;
    element->value = value;
    
    if( IS_FAILED( ( ret = PutInHashTable( (CADDR_T)species, sizeof(species), (CADDR_T)element, table ) ) ) ) {
        END_FUNCTION("_SetSpeciesValue", ret );        
        return ret;
    } 
                
    END_FUNCTION("_SetSpeciesValue", SUCCESS );        
    return ret;
}

static RET_VAL _RemoveSpeciesValue( KINETIC_LAW_EVALUATER *evaluater, SPECIES *species ) {
    RET_VAL ret = SUCCESS;
    KINETIC_LAW_EVALUATION_ELEMENT *element = NULL;
    HASH_TABLE *table = NULL;

    START_FUNCTION("_RemoveSpeciesValue");
    
    table = evaluater->table;    
    if( ( element = (KINETIC_LAW_EVALUATION_ELEMENT*)GetValueFromHashTable( (CADDR_T)species, sizeof( species ), table ) ) != NULL ) {
      if( IS_FAILED( ( ret = RemoveFromHashTable( (CADDR_T)species, sizeof( species ), table ) ) ) ) {
            END_FUNCTION("_SetSpeciesValue", ret );        
            return ret;
        } 
        FREE( element );
        END_FUNCTION("_SetSpeciesValue", SUCCESS );        
        return ret;
    } 
    
    END_FUNCTION("_RemoveSpeciesValue", SUCCESS );        
    return ret;
}

static RET_VAL _SetDefaultSpeciesValue( KINETIC_LAW_EVALUATER *evaluater, double value ) {
    RET_VAL ret = SUCCESS;

    START_FUNCTION("_SetDefaultSpeciesValue");

    evaluater->defaultValue = value;
        
    END_FUNCTION("_SetDefaultSpeciesValue", SUCCESS );
        
    return ret;
}

static double _Evaluate( KINETIC_LAW_EVALUATER *evaluater, KINETIC_LAW *kineticLaw ) {
    static KINETIC_LAW_VISITOR visitor;
    RET_VAL ret = SUCCESS;
    double result = 0.0;
    
    START_FUNCTION("_Evaluate");
    
    KINETIC_LAW *massActionRatio = NULL;
    
    START_FUNCTION("_Evaluate");
    
    if( visitor.VisitOp == NULL ) {
        visitor.VisitPW = _VisitPWToEvaluate;
        visitor.VisitOp = _VisitOpToEvaluate;
        visitor.VisitUnaryOp = _VisitUnaryOpToEvaluate;
        visitor.VisitInt = _VisitIntToEvaluate;
        visitor.VisitReal = _VisitRealToEvaluate;
        visitor.VisitSpecies = _VisitSpeciesToEvaluate;
        visitor.VisitCompartment = _VisitCompartmentToEvaluate;
        visitor.VisitSymbol = _VisitSymbolToEvaluate;
        visitor.VisitFunctionSymbol = _VisitFunctionSymbolToEvaluate;
    }
    
    visitor._internal1 = (CADDR_T)evaluater;
    visitor._internal2 = (CADDR_T)(&result);
    
    if( IS_FAILED( kineticLaw->Accept( kineticLaw, &visitor ) ) ) {
        END_FUNCTION("_Evaluate", FAILING );
        return -1.0;
    } 
        
    END_FUNCTION("_Evaluate", SUCCESS );        
    return result;
}     


static double _EvaluateWithCurrentAmounts( KINETIC_LAW_EVALUATER *evaluater, KINETIC_LAW *kineticLaw ) {
    static KINETIC_LAW_VISITOR visitor;
    RET_VAL ret = SUCCESS;
    double result = 0.0;
    
    START_FUNCTION("_Evaluate");
    
    KINETIC_LAW *massActionRatio = NULL;
    
    START_FUNCTION("_Evaluate");
    
    if( visitor.VisitOp == NULL ) {
        visitor.VisitPW = _VisitPWToEvaluate;
        visitor.VisitOp = _VisitOpToEvaluate;
        visitor.VisitUnaryOp = _VisitUnaryOpToEvaluate;
        visitor.VisitInt = _VisitIntToEvaluate;
        visitor.VisitReal = _VisitRealToEvaluate;
        visitor.VisitSpecies = _VisitSpeciesToEvaluateWithCurrentAmounts;
        visitor.VisitCompartment = _VisitCompartmentToEvaluateWithCurrentSize;
        visitor.VisitSymbol = _VisitSymbolToEvaluate;
	visitor.VisitFunctionSymbol = _VisitFunctionSymbolToEvaluate;
    }
    
    visitor._internal1 = (CADDR_T)evaluater;
    visitor._internal2 = (CADDR_T)(&result);
    
    if( IS_FAILED( kineticLaw->Accept( kineticLaw, &visitor ) ) ) {
        END_FUNCTION("_Evaluate", FAILING );
        return -1.0;
    } 
        
    END_FUNCTION("_Evaluate", SUCCESS );        
    return result;
}     


static double _EvaluateWithCurrentAmountsDeter( KINETIC_LAW_EVALUATER *evaluater, KINETIC_LAW *kineticLaw ) {
    static KINETIC_LAW_VISITOR visitor;
    RET_VAL ret = SUCCESS;
    double result = 0.0;
    
    START_FUNCTION("_Evaluate");
    
    KINETIC_LAW *massActionRatio = NULL;
    
    START_FUNCTION("_Evaluate");
    
    if( visitor.VisitOp == NULL ) {
        visitor.VisitPW = _VisitPWToEvaluate;
        visitor.VisitOp = _VisitOpToEvaluateDeter;
        visitor.VisitUnaryOp = _VisitUnaryOpToEvaluateDeter;
        visitor.VisitInt = _VisitIntToEvaluate;
        visitor.VisitReal = _VisitRealToEvaluate;
        visitor.VisitSpecies = _VisitSpeciesToEvaluateWithCurrentAmounts;
        visitor.VisitCompartment = _VisitCompartmentToEvaluateWithCurrentSize;
        visitor.VisitSymbol = _VisitSymbolToEvaluate;
	visitor.VisitFunctionSymbol = _VisitFunctionSymbolToEvaluate;
    }
    
    visitor._internal1 = (CADDR_T)evaluater;
    visitor._internal2 = (CADDR_T)(&result);
    
    if( IS_FAILED( kineticLaw->Accept( kineticLaw, &visitor ) ) ) {
        END_FUNCTION("_Evaluate", FAILING );
        return -1.0;
    } 
        
    END_FUNCTION("_Evaluate", SUCCESS );        
    return result;
}     


static double _EvaluateWithCurrentConcentrations( KINETIC_LAW_EVALUATER *evaluater, KINETIC_LAW *kineticLaw ) {
    static KINETIC_LAW_VISITOR visitor;
    RET_VAL ret = SUCCESS;
    double result = 0.0;
    
    START_FUNCTION("_Evaluate");
    
    KINETIC_LAW *massActionRatio = NULL;
    
    START_FUNCTION("_Evaluate");
    
    if( visitor.VisitOp == NULL ) {
        visitor.VisitPW = _VisitPWToEvaluate;
        visitor.VisitOp = _VisitOpToEvaluate;
        visitor.VisitUnaryOp = _VisitUnaryOpToEvaluate;
        visitor.VisitInt = _VisitIntToEvaluate;
        visitor.VisitReal = _VisitRealToEvaluate;
        visitor.VisitSpecies = _VisitSpeciesToEvaluateWithCurrentConcentrations;
        visitor.VisitCompartment = _VisitCompartmentToEvaluateWithCurrentSize;
        visitor.VisitSymbol = _VisitSymbolToEvaluate;
	visitor.VisitFunctionSymbol = _VisitFunctionSymbolToEvaluate;
    }
    
    visitor._internal1 = (CADDR_T)evaluater;
    visitor._internal2 = (CADDR_T)(&result);
    
    if( IS_FAILED( kineticLaw->Accept( kineticLaw, &visitor ) ) ) {
        END_FUNCTION("_Evaluate", FAILING );
        return -1.0;
    } 
        
    END_FUNCTION("_Evaluate", SUCCESS );        
    return result;
}     


static double _EvaluateWithCurrentConcentrationsDeter( KINETIC_LAW_EVALUATER *evaluater, KINETIC_LAW *kineticLaw ) {
    static KINETIC_LAW_VISITOR visitor;
    RET_VAL ret = SUCCESS;
    double result = 0.0;
    
    START_FUNCTION("_Evaluate");
    
    KINETIC_LAW *massActionRatio = NULL;
    
    START_FUNCTION("_Evaluate");
    
    if( visitor.VisitOp == NULL ) {
        visitor.VisitPW = _VisitPWToEvaluate;
        visitor.VisitOp = _VisitOpToEvaluateDeter;
        visitor.VisitUnaryOp = _VisitUnaryOpToEvaluateDeter;
        visitor.VisitInt = _VisitIntToEvaluate;
        visitor.VisitReal = _VisitRealToEvaluate;
        visitor.VisitSpecies = _VisitSpeciesToEvaluateWithCurrentConcentrations;
        visitor.VisitCompartment = _VisitCompartmentToEvaluateWithCurrentSize;
        visitor.VisitSymbol = _VisitSymbolToEvaluate;
	visitor.VisitFunctionSymbol = _VisitFunctionSymbolToEvaluate;
    }
    
    visitor._internal1 = (CADDR_T)evaluater;
    visitor._internal2 = (CADDR_T)(&result);
    
    if( IS_FAILED( kineticLaw->Accept( kineticLaw, &visitor ) ) ) {
        END_FUNCTION("_Evaluate", FAILING );
        return -1.0;
    } 
        
    END_FUNCTION("_Evaluate", SUCCESS );        
    return result;
}     


static RET_VAL _VisitPWToEvaluate( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    BYTE opType = 0x00;
    double childValue = 0.0;
    double *result = NULL;
    KINETIC_LAW *child = NULL;
    LINKED_LIST *children = NULL;
    UINT num = 0;
    UINT i = 0;
    
    START_FUNCTION("_VisitPWToEvaluate");

    result = (double*)(visitor->_internal2);
    
    children = GetPWChildrenFromKineticLaw( kineticLaw );
    num = GetLinkedListSize( children );
    opType = GetPWTypeFromKineticLaw( kineticLaw );
    switch( opType ) {
    case KINETIC_LAW_OP_PW:
      for ( i = 1; i < num; i+=2 ) {
	child = (KINETIC_LAW*)GetElementByIndex( i,children );
	visitor->_internal2 = (CADDR_T)(&childValue);
	if( IS_FAILED( ( ret = child->Accept( child, visitor ) ) ) ) {
	  END_FUNCTION("_VisitPWToEvaluate", ret );
	  return ret;
	}
	if (childValue) {
	  child = (KINETIC_LAW*)GetElementByIndex( i-1,children );
	  visitor->_internal2 = (CADDR_T)(&childValue);
	  if( IS_FAILED( ( ret = child->Accept( child, visitor ) ) ) ) {
	    END_FUNCTION("_VisitPWToEvaluate", ret );
	    return ret;
	  }
	  *result = childValue;
	  break;
	}
      }
      if (i==num) {
	child = (KINETIC_LAW*)GetElementByIndex( i-1,children );
	visitor->_internal2 = (CADDR_T)(&childValue);
	if( IS_FAILED( ( ret = child->Accept( child, visitor ) ) ) ) {
	  END_FUNCTION("_VisitPWToEvaluate", ret );
	  return ret;
	}
	*result = childValue;
      }
      break;
    case KINETIC_LAW_OP_XOR:
      *result = 0;
      for ( i = 0; i < num; i++ ) {
	child = (KINETIC_LAW*)GetElementByIndex( i,children );
	visitor->_internal2 = (CADDR_T)(&childValue);
	if( IS_FAILED( ( ret = child->Accept( child, visitor ) ) ) ) {
	  END_FUNCTION("_VisitPWToEvaluate", ret );
	  return ret;
	}
	*result = (!(*result) && childValue)||((*result) && !childValue);
      }
      break;
    case KINETIC_LAW_OP_OR:
      *result = 0;
      for ( i = 0; i < num; i++ ) {
	child = (KINETIC_LAW*)GetElementByIndex( i,children );
	visitor->_internal2 = (CADDR_T)(&childValue);
	if( IS_FAILED( ( ret = child->Accept( child, visitor ) ) ) ) {
	  END_FUNCTION("_VisitPWToEvaluate", ret );
	  return ret;
	}
	*result = *result || childValue;
      }
      break;
    case KINETIC_LAW_OP_AND:
      *result = 1;
      for ( i = 0; i < num; i++ ) {
	child = (KINETIC_LAW*)GetElementByIndex( i,children );
	visitor->_internal2 = (CADDR_T)(&childValue);
	if( IS_FAILED( ( ret = child->Accept( child, visitor ) ) ) ) {
	  END_FUNCTION("_VisitPWToEvaluate", ret );
	  return ret;
	}
	*result = *result && childValue;
      }
      break;
    case KINETIC_LAW_OP_PLUS:
      *result = 0;
      for ( i = 0; i < num; i++ ) {
	child = (KINETIC_LAW*)GetElementByIndex( i,children );
	visitor->_internal2 = (CADDR_T)(&childValue);
	if( IS_FAILED( ( ret = child->Accept( child, visitor ) ) ) ) {
	  END_FUNCTION("_VisitPWToEvaluate", ret );
	  return ret;
	}
	*result = *result + childValue;
      }
      break;
    case KINETIC_LAW_OP_TIMES:
      *result = 1;
      for ( i = 0; i < num; i++ ) {
	child = (KINETIC_LAW*)GetElementByIndex( i,children );
	visitor->_internal2 = (CADDR_T)(&childValue);
	if( IS_FAILED( ( ret = child->Accept( child, visitor ) ) ) ) {
	  END_FUNCTION("_VisitPWToEvaluate", ret );
	  return ret;
	}
	*result = *result * childValue;
      }
      break;
    default:
      END_FUNCTION("_VisitUnaryOpToFindNextTime", E_WRONGDATA );
      return E_WRONGDATA;
    }
    
    visitor->_internal2 = (CADDR_T)result;
    
    END_FUNCTION("_VisitOpToEvaluate", SUCCESS );
    return ret;
}

static RET_VAL _VisitOpToEvaluate( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    BYTE opType = 0x00;
    double leftValue = 0.0;
    double rightValue = 0.0;
    double *result = NULL;
    KINETIC_LAW *left = NULL;
    KINETIC_LAW *right = NULL;
    
    START_FUNCTION("_VisitOpToEvaluate");
    
    result = (double*)(visitor->_internal2);
    
    left = GetOpLeftFromKineticLaw( kineticLaw );
    visitor->_internal2 = (CADDR_T)(&leftValue);
    if( IS_FAILED( ( ret = left->Accept( left, visitor ) ) ) ) {
        END_FUNCTION("_VisitOpToEvaluate", ret );
        return ret;
    }
    
    right = GetOpRightFromKineticLaw( kineticLaw );       
    visitor->_internal2 = (CADDR_T)(&rightValue);
    if( IS_FAILED( ( ret = right->Accept( right, visitor ) ) ) ) {
        END_FUNCTION("_VisitOpToEvaluate", ret );
        return ret;
    }            
    
    opType = GetOpTypeFromKineticLaw( kineticLaw );     
    switch( opType ) {
        case KINETIC_LAW_OP_PLUS:
            *result = leftValue + rightValue;
        break;

        case KINETIC_LAW_OP_MINUS:
            *result = leftValue - rightValue;
        break;

        case KINETIC_LAW_OP_TIMES:
            *result = leftValue * rightValue;
        break;

        case KINETIC_LAW_OP_DIVIDE:
            *result = leftValue / rightValue;
        break;
        
        case KINETIC_LAW_OP_POW:
            *result = pow( leftValue, rightValue );
        break;
        
        case KINETIC_LAW_OP_LOG:
	  *result = log( rightValue ) / log( leftValue);
        break;

        case KINETIC_LAW_OP_DELAY:
	  *result = leftValue;
        break;        
        
        case KINETIC_LAW_OP_ROOT:
	  *result = pow(rightValue,(1./leftValue));
        break;        

	/*
        case KINETIC_LAW_OP_XOR:
	  *result = (!leftValue && rightValue)||(leftValue && !rightValue);
        break;

        case KINETIC_LAW_OP_OR:
            *result = leftValue || rightValue;
        break;

        case KINETIC_LAW_OP_AND:
            *result = leftValue && rightValue;
        break;
	*/
        case KINETIC_LAW_OP_EQ:
	  *result = (leftValue == rightValue);
        break;

        case KINETIC_LAW_OP_NEQ:
	  *result = (leftValue != rightValue);
        break;

        case KINETIC_LAW_OP_GEQ:
	  *result = (leftValue >= rightValue);
	  /*
	  if (*result == 0) {
	    if ( left->valueType == KINETIC_LAW_VALUE_TYPE_SYMBOL ) {
	      if (strcmp(GetCharArrayOfString( GetSymbolID( GetSymbolFromKineticLaw( left ) ) ), "t") == 0) {
		*result = (rightValue - leftValue);
		if (*result == 1.0) { 
		  *result = 1.0000001;
		}
	      }
	    }
	  }
	  */
        break;

        case KINETIC_LAW_OP_GT:
	  *result = (leftValue > rightValue);
        break;

        case KINETIC_LAW_OP_LEQ:
	  *result = (leftValue <= rightValue);
        break;

        case KINETIC_LAW_OP_LT:
	  *result = (leftValue < rightValue);
        break;

        case KINETIC_LAW_OP_UNIFORM:
	    *result = GetNextUniformRandomNumber(leftValue,rightValue);
        break;

        case KINETIC_LAW_OP_NORMAL:
	    *result = GetNextNormalRandomNumber(leftValue,rightValue);
        break;

        case KINETIC_LAW_OP_BINOMIAL:
	  *result = GetNextBinomialRandomNumber(leftValue,(unsigned int)rightValue);
        break;

        case KINETIC_LAW_OP_GAMMA:
	    *result = GetNextGammaRandomNumber(leftValue,rightValue);
        break;

        case KINETIC_LAW_OP_LOGNORMAL:
	    *result = GetNextLogNormalRandomNumber(leftValue,rightValue);
        break;

        case KINETIC_LAW_OP_BITWISE_AND:
	  *result = ((int)rint(leftValue) & (int)rint(rightValue));
        break;

        case KINETIC_LAW_OP_BITWISE_OR:
	  *result = ((int)rint(leftValue) | (int)rint(rightValue));
        break;

        case KINETIC_LAW_OP_BITWISE_XOR:
	  *result = ((int)rint(leftValue) ^ (int)rint(rightValue));
        break;

        case KINETIC_LAW_OP_MOD:
	  *result = ((int)rint(leftValue) % (int)rint(rightValue));
        break;

        case KINETIC_LAW_OP_BIT:
	  *result = ((int)rint(leftValue) >> (int)rint(rightValue)) & 1;
        break;
        
        default:
            END_FUNCTION("_VisitOpToEvaluate", E_WRONGDATA );
        return E_WRONGDATA;
    }
    
    visitor->_internal2 = (CADDR_T)result;
    
    END_FUNCTION("_VisitOpToEvaluate", SUCCESS );
    return ret;
}

static RET_VAL _VisitUnaryOpToEvaluate( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    BYTE opType = 0x00;
    double childValue = 0.0;
    double *result = NULL;
    KINETIC_LAW *child = NULL;
    UINT i = 0;
    START_FUNCTION("_VisitUnaryOpToEvaluate");
    
    result = (double*)(visitor->_internal2);
    
    child = GetUnaryOpChildFromKineticLaw( kineticLaw );
    visitor->_internal2 = (CADDR_T)(&childValue);
    if( IS_FAILED( ( ret = child->Accept( child, visitor ) ) ) ) {
        END_FUNCTION("_VisitUnaryOpToEvaluate", ret );
        return ret;
    }
    
    opType = GetUnaryOpTypeFromKineticLaw( kineticLaw );     
    switch( opType ) {
        case KINETIC_LAW_UNARY_OP_NEG:
	  *result = (-1)*childValue;
        break;
        case KINETIC_LAW_UNARY_OP_NOT:
	  *result = !childValue;
        break;
        case KINETIC_LAW_UNARY_OP_ABS:
	  *result = fabs(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_COT:
	  *result = (1./tan(childValue));
        break;
        case KINETIC_LAW_UNARY_OP_COTH:
	  *result = cosh(childValue)/sinh(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_CSC:
	  *result = (1./sin(childValue));
        break;
        case KINETIC_LAW_UNARY_OP_CSCH:
	  *result = (1./cosh(childValue));
        break;
        case KINETIC_LAW_UNARY_OP_SEC:
	  *result = (1./cos(childValue));
        break;
        case KINETIC_LAW_UNARY_OP_SECH:
	  *result = (1./sinh(childValue));
        break;
        case KINETIC_LAW_UNARY_OP_COS:
	  *result = cos(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_COSH:
	  *result = cosh(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_SIN:
	  *result = sin(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_SINH:
	  *result = sinh(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_TAN:
	  *result = tan(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_TANH:
	  *result = tanh(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_ARCCOT:
	  *result = atan(1./(childValue));
        break;
        case KINETIC_LAW_UNARY_OP_ARCCOTH:
	  *result = ((1./2.)*log((childValue+1.)/(childValue-1.)) );
        break;
        case KINETIC_LAW_UNARY_OP_ARCCSC:
	  *result = asin( 1. / childValue ); //atan( 1. / SQRT( (childValue-1.)*(childValue+1.) )); 
        break;
        case KINETIC_LAW_UNARY_OP_ARCCSCH:
	  *result = log(1./childValue + SQRT(1./SQR(childValue)+1)); 
	  //*result = log((1.+SQRT((1+SQR(childValue)))) /childValue);
        break;
        case KINETIC_LAW_UNARY_OP_ARCSEC:
	  *result = acos( 1. / childValue ); //atan( SQRT(( childValue-1.)*( childValue+1.)) );
        break;
        case KINETIC_LAW_UNARY_OP_ARCSECH:
	  *result = log(1./childValue + SQRT(1./childValue + 1) * SQRT(1./childValue -1));
	  //*result = log((1.+pow((1-SQR(childValue)),0.5))/childValue);
        break;
        case KINETIC_LAW_UNARY_OP_ARCCOS:
	  *result = acos(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_ARCCOSH:
	  *result = acosh(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_ARCSIN:
	  *result = asin(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_ARCSINH:
	  *result = asinh(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_ARCTAN:
	  *result = atan(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_ARCTANH:
	  *result = atanh(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_CEILING:
	  *result = ceil(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_EXP:
	  *result = exp(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_FACTORIAL:
	  i = floor(childValue);
	  for((*result)=1;i>1;--i)
	    *result *= i;
        break;
        case KINETIC_LAW_UNARY_OP_FLOOR:
	  *result = floor(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_LN:
	  *result = log(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_EXPRAND:
	  *result = GetNextExponentialRandomNumber(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_POISSON:
	  *result = GetNextPoissonRandomNumber(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_CHISQ:
	  *result = GetNextChiSquaredRandomNumber(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_LAPLACE:
	  *result = GetNextLaplaceRandomNumber(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_CAUCHY:
	  *result = GetNextCauchyRandomNumber(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_RAYLEIGH:
	  *result = GetNextRayleighRandomNumber(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_BERNOULLI:
	  *result = GetNextBernoulliRandomNumber(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_RATE:
	  if (IsSymbolKineticLaw( child )) {
	    *result = GetCurrentRateInSymbol( GetSymbolFromKineticLaw( child ) );
	  } else if (IsSpeciesKineticLaw( child )) {
	    *result = GetRateInSpeciesNode( GetSpeciesFromKineticLaw( child ) );
	  } else if (IsCompartmentKineticLaw( child )) {
	    *result = GetCurrentRateInCompartment( GetCompartmentFromKineticLaw( child ) );
	  } else {
            END_FUNCTION("_VisitUnaryOpToEvaluate", E_WRONGDATA );
	  }
        break;
        case KINETIC_LAW_UNARY_OP_BITWISE_NOT:
	  *result = ~((int)rint(childValue));
	break;
        case KINETIC_LAW_UNARY_OP_INT:
	  *result = childValue;
        break;
        
        default:
            END_FUNCTION("_VisitUnaryOpToEvaluate", E_WRONGDATA );
        return E_WRONGDATA;
    }
    
    visitor->_internal2 = (CADDR_T)result;

    END_FUNCTION("_VisitUnaryOpToEvaluate", SUCCESS );
    return ret;
}

static RET_VAL _VisitOpToEvaluateDeter( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    BYTE opType = 0x00;
    double leftValue = 0.0;
    double rightValue = 0.0;
    double *result = NULL;
    KINETIC_LAW *left = NULL;
    KINETIC_LAW *right = NULL;
    
    START_FUNCTION("_VisitOpToEvaluate");
    
    result = (double*)(visitor->_internal2);
    
    left = GetOpLeftFromKineticLaw( kineticLaw );
    visitor->_internal2 = (CADDR_T)(&leftValue);
    if( IS_FAILED( ( ret = left->Accept( left, visitor ) ) ) ) {
        END_FUNCTION("_VisitOpToEvaluate", ret );
        return ret;
    }
    
    right = GetOpRightFromKineticLaw( kineticLaw );       
    visitor->_internal2 = (CADDR_T)(&rightValue);
    if( IS_FAILED( ( ret = right->Accept( right, visitor ) ) ) ) {
        END_FUNCTION("_VisitOpToEvaluate", ret );
        return ret;
    }            
    
    opType = GetOpTypeFromKineticLaw( kineticLaw );     
    switch( opType ) {
        case KINETIC_LAW_OP_PLUS:
            *result = leftValue + rightValue;
        break;
        
        case KINETIC_LAW_OP_TIMES:
            *result = leftValue * rightValue;
        break;

        case KINETIC_LAW_OP_MINUS:
            *result = leftValue - rightValue;
        break;
        
        case KINETIC_LAW_OP_DIVIDE:
            *result = leftValue / rightValue;
        break;
        
        case KINETIC_LAW_OP_POW:
            *result = pow( leftValue, rightValue );
        break;
        
        case KINETIC_LAW_OP_LOG:
	  *result = log( rightValue ) / log( leftValue);
        break;
        
        case KINETIC_LAW_OP_DELAY:
	  *result = leftValue;
        break;        

        case KINETIC_LAW_OP_ROOT:
	  *result = pow(rightValue,(1./leftValue));
        break;        

	/*
        case KINETIC_LAW_OP_XOR:
	  *result = (!leftValue && rightValue)||(leftValue && !rightValue);
        break;

        case KINETIC_LAW_OP_AND:
            *result = leftValue && rightValue;
        break;

        case KINETIC_LAW_OP_OR:
            *result = leftValue || rightValue;
        break;
	*/

        case KINETIC_LAW_OP_EQ:
	  *result = (leftValue == rightValue);
        break;

        case KINETIC_LAW_OP_NEQ:
	  *result = (leftValue != rightValue);
        break;

        case KINETIC_LAW_OP_GEQ:
	  *result = (leftValue >= rightValue);
        break;

        case KINETIC_LAW_OP_GT:
	  *result = (leftValue > rightValue);
        break;

        case KINETIC_LAW_OP_LEQ:
	  *result = (leftValue <= rightValue);
        break;

        case KINETIC_LAW_OP_LT:
	  *result = (leftValue < rightValue);
        break;

        case KINETIC_LAW_OP_UNIFORM:
	    *result = (leftValue + rightValue)/2;
        break;

        case KINETIC_LAW_OP_NORMAL:
	    *result = leftValue;
        break;

        case KINETIC_LAW_OP_BINOMIAL:
	  *result = (leftValue * rightValue);
        break;

        case KINETIC_LAW_OP_GAMMA:
	    *result = (leftValue * rightValue);
        break;

        case KINETIC_LAW_OP_LOGNORMAL:
	    *result = exp(leftValue + (rightValue * rightValue)/2);
        break;

        case KINETIC_LAW_OP_BITWISE_AND:
	  *result = ((int)rint(leftValue) & (int)rint(rightValue));
        break;

        case KINETIC_LAW_OP_BITWISE_OR:
	  *result = ((int)rint(leftValue) | (int)rint(rightValue));
        break;

        case KINETIC_LAW_OP_BITWISE_XOR:
	  *result = ((int)rint(leftValue) ^ (int)rint(rightValue));
        break;

        case KINETIC_LAW_OP_MOD:
	  *result = ((int)rint(leftValue) % (int)rint(rightValue));
        break;

        case KINETIC_LAW_OP_BIT:
	  *result = ((int)rint(leftValue) >> (int)rint(rightValue)) & 1;
        break;
        
        default:
            END_FUNCTION("_VisitOpToEvaluate", E_WRONGDATA );
        return E_WRONGDATA;
    }
    
    visitor->_internal2 = (CADDR_T)result;
    
    END_FUNCTION("_VisitOpToEvaluate", SUCCESS );
    return ret;
}

static RET_VAL _VisitUnaryOpToEvaluateDeter( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    BYTE opType = 0x00;
    double childValue = 0.0;
    double *result = NULL;
    KINETIC_LAW *child = NULL;
    UINT i = 0;
    START_FUNCTION("_VisitUnaryOpToEvaluate");
    
    result = (double*)(visitor->_internal2);
    
    child = GetUnaryOpChildFromKineticLaw( kineticLaw );
    visitor->_internal2 = (CADDR_T)(&childValue);
    if( IS_FAILED( ( ret = child->Accept( child, visitor ) ) ) ) {
        END_FUNCTION("_VisitUnaryOpToEvaluate", ret );
        return ret;
    }
    
    opType = GetUnaryOpTypeFromKineticLaw( kineticLaw );     
    switch( opType ) {
        case KINETIC_LAW_UNARY_OP_NEG:
	  *result = (-1)*childValue;
        break;
        case KINETIC_LAW_UNARY_OP_NOT:
	  *result = !childValue;
        break;
        case KINETIC_LAW_UNARY_OP_ABS:
	  *result = fabs(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_COT:
	  *result = (1./tan(childValue));
        break;
        case KINETIC_LAW_UNARY_OP_COTH:
	  *result = cosh(childValue)/sinh(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_CSC:
	  *result = (1./sin(childValue));
        break;
        case KINETIC_LAW_UNARY_OP_CSCH:
	  *result = (1./cosh(childValue));
        break;
        case KINETIC_LAW_UNARY_OP_SEC:
	  *result = (1./cos(childValue));
        break;
        case KINETIC_LAW_UNARY_OP_SECH:
	  *result = (1./sinh(childValue));
        break;
        case KINETIC_LAW_UNARY_OP_COS:
	  *result = cos(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_COSH:
	  *result = cosh(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_SIN:
	  *result = sin(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_SINH:
	  *result = sinh(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_TAN:
	  *result = tan(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_TANH:
	  *result = tanh(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_ARCCOT:
	  *result = atan(1./(childValue));
        break;
        case KINETIC_LAW_UNARY_OP_ARCCOTH:
	  *result = ((1./2.)*log((childValue+1.)/(childValue-1.)) );
        break;
        case KINETIC_LAW_UNARY_OP_ARCCSC:
	  *result = asin( 1. / childValue ); //atan( 1. / SQRT( (childValue-1.)*(childValue+1.) )); 
        break;
        case KINETIC_LAW_UNARY_OP_ARCCSCH:
	  *result = log(1./childValue + SQRT(1./SQR(childValue)+1)); 
	  //*result = log((1.+SQRT((1+SQR(childValue)))) /childValue);
        break;
        case KINETIC_LAW_UNARY_OP_ARCSEC:
	  *result = acos( 1. / childValue ); //atan( SQRT(( childValue-1.)*( childValue+1.)) );
        break;
        case KINETIC_LAW_UNARY_OP_ARCSECH:
	  *result = log(1./childValue + SQRT(1./childValue + 1) * SQRT(1./childValue -1));
	  //*result = log((1.+pow((1-SQR(childValue)),0.5))/childValue);
        break;
        case KINETIC_LAW_UNARY_OP_ARCCOS:
	  *result = acos(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_ARCCOSH:
	  *result = acosh(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_ARCSIN:
	  *result = asin(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_ARCSINH:
	  *result = asinh(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_ARCTAN:
	  *result = atan(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_ARCTANH:
	  *result = atanh(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_CEILING:
	  *result = ceil(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_EXP:
	  *result = exp(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_FACTORIAL:
	  i = floor(childValue);
	  for((*result)=1;i>1;--i)
	    *result *= i;
        break;
        case KINETIC_LAW_UNARY_OP_FLOOR:
	  *result = floor(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_LN:
	  *result = log(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_EXPRAND:
	  *result = 1 / childValue;
        break;
        case KINETIC_LAW_UNARY_OP_POISSON:
	  *result = childValue;
        break;
        case KINETIC_LAW_UNARY_OP_CHISQ:
	  *result = childValue;
        break;
        case KINETIC_LAW_UNARY_OP_LAPLACE:
	  *result = 0;
        break;
        case KINETIC_LAW_UNARY_OP_CAUCHY:
	  *result = childValue;
        break;
        case KINETIC_LAW_UNARY_OP_RAYLEIGH:
	  *result = childValue;
        break;
        case KINETIC_LAW_UNARY_OP_BERNOULLI:
	  *result = childValue;
        break;
        case KINETIC_LAW_UNARY_OP_RATE:
	  if (IsSymbolKineticLaw( child )) {
	    *result = GetCurrentRateInSymbol( GetSymbolFromKineticLaw( child ) );
	  } else if (IsSpeciesKineticLaw( child )) {
	    *result = GetRateInSpeciesNode( GetSpeciesFromKineticLaw( child ) );
	  } else if (IsCompartmentKineticLaw( child )) {
	    *result = GetCurrentRateInCompartment( GetCompartmentFromKineticLaw( child ) );
	  } else {
            END_FUNCTION("_VisitUnaryOpToEvaluate", E_WRONGDATA );
	  }
        break;
        case KINETIC_LAW_UNARY_OP_BITWISE_NOT:
	  *result = ~((int)rint(childValue));
	break;
        case KINETIC_LAW_UNARY_OP_INT:
	  *result = childValue;
        break;

        default:
            END_FUNCTION("_VisitUnaryOpToEvaluate", E_WRONGDATA );
        return E_WRONGDATA;
    }
    
    visitor->_internal2 = (CADDR_T)result;

    END_FUNCTION("_VisitUnaryOpToEvaluate", SUCCESS );
    return ret;
}

static RET_VAL _VisitIntToEvaluate( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    double *result = NULL;
    double value = 0.0;
    START_FUNCTION("_VisitIntToEvaluate");
        
    result = (double*)(visitor->_internal2);
    value = (double)GetIntValueFromKineticLaw( kineticLaw );
    *result = value; 
    
    END_FUNCTION("_VisitIntToEvaluate", SUCCESS );
    return SUCCESS;
}

static RET_VAL _VisitRealToEvaluate( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    double *result = NULL;
    double value = 0.0;
    
    START_FUNCTION("_VisitRealToEvaluate");
    
    result = (double*)(visitor->_internal2);
    value = GetRealValueFromKineticLaw( kineticLaw );
    *result = value; 
    
    END_FUNCTION("_VisitRealToEvaluate", SUCCESS );
    return SUCCESS;
}

static RET_VAL _VisitSymbolToEvaluate( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    double *result = NULL;
    double value = 0.0;
    REB2SAC_SYMBOL *sym = NULL;
    
    START_FUNCTION("_VisitSymbolToEvaluate");
    
    sym = GetSymbolFromKineticLaw( kineticLaw );
    /*
    if( !IsSymbolConstant( sym ) ) {
        return ErrorReport( FAILING, "_VisitSymbolToEvaluate", "symbol %s is not a constant real", GetCharArrayOfString( GetSymbolID( sym ) ) );
    }
    */
    if( !IsRealValueSymbol( sym ) ) {
        return ErrorReport( FAILING, "_VisitSymbolToEvaluate", "symbol %s is not a constant real", GetCharArrayOfString( GetSymbolID( sym ) ) );
    } 
    
    result = (double*)(visitor->_internal2);
    value = GetCurrentRealValueInSymbol( sym );
    
    *result = value; 

    END_FUNCTION("_VisitSymbolToEvaluate", SUCCESS );
    return SUCCESS;
}

static RET_VAL _VisitFunctionSymbolToEvaluate( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    
    START_FUNCTION("_VisitFunctionSymbolToEvaluate");
    
    END_FUNCTION("_VisitFunctionSymbolToEvaluate", SUCCESS );
    return SUCCESS;
}

static RET_VAL _VisitSpeciesToEvaluate( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    double *result = NULL;
    double value = 0.0;
    SPECIES *species = NULL;    
    KINETIC_LAW_EVALUATER *evaluater = NULL;
    
    START_FUNCTION("_VisitSpeciesToEvaluate");
    
    evaluater = (KINETIC_LAW_EVALUATER*)(visitor->_internal1);
    result = (double*)(visitor->_internal2);
    species = GetSpeciesFromKineticLaw( kineticLaw );
    
    value = _GetValue( evaluater, species );
    *result = value; 
            
    END_FUNCTION("_VisitSpeciesToEvaluate", SUCCESS );
    return SUCCESS;
}

static RET_VAL _VisitCompartmentToEvaluate( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    double *result = NULL;
    double value = 0.0;
    COMPARTMENT *compartment = NULL;    
    KINETIC_LAW_EVALUATER *evaluater = NULL;
    
    START_FUNCTION("_VisitCompartmentToEvaluate");
    
    evaluater = (KINETIC_LAW_EVALUATER*)(visitor->_internal1);
    result = (double*)(visitor->_internal2);
    compartment = GetCompartmentFromKineticLaw( kineticLaw );
    
    value = GetCurrentSizeInCompartment( compartment );
    *result = value; 
            
    END_FUNCTION("_VisitCompartmentToEvaluate", SUCCESS );
    return SUCCESS;
}


static double _GetValue( KINETIC_LAW_EVALUATER *evaluater, SPECIES *species ) {
    double result = 0.0;
    KINETIC_LAW_EVALUATION_ELEMENT *element = NULL;
    HASH_TABLE *table = NULL;
    
    START_FUNCTION("_GetValue");

    table = evaluater->table;    
    if( ( element = (KINETIC_LAW_EVALUATION_ELEMENT*)GetValueFromHashTable( (CADDR_T)species, sizeof( species ), table ) ) != NULL ) {
        result = element->value;
    }
    else {
        result = evaluater->defaultValue;;
    } 
    TRACE_2("The value for %s is %g", GetCharArrayOfString( GetSpeciesNodeName( species ) ), result );
    
    END_FUNCTION("_GetValue", SUCCESS );        
    return result;
}       

static RET_VAL _VisitCompartmentToEvaluateWithCurrentSize( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    double *result = NULL;
    double value = 0.0;
    COMPARTMENT *compartment = NULL;    
    KINETIC_LAW_EVALUATER *evaluater = NULL;
    
    evaluater = (KINETIC_LAW_EVALUATER*)(visitor->_internal1);
    result = (double*)(visitor->_internal2);
    compartment = GetCompartmentFromKineticLaw( kineticLaw );
    
    value = GetCurrentSizeInCompartment( compartment );
    *result = value; 
    
    return SUCCESS;
}

static RET_VAL _VisitSpeciesToEvaluateWithCurrentAmounts( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    double *result = NULL;
    double value = 0.0;
    SPECIES *species = NULL;    
    KINETIC_LAW_EVALUATER *evaluater = NULL;
    
    evaluater = (KINETIC_LAW_EVALUATER*)(visitor->_internal1);
    result = (double*)(visitor->_internal2);
    species = GetSpeciesFromKineticLaw( kineticLaw );
    
    //if( IsInitialQuantityInAmountInSpeciesNode( species ) ) {
    if( HasOnlySubstanceUnitsInSpeciesNode( species ) ) {
      value = GetAmountInSpeciesNode( species );
    } else {
      value = GetConcentrationInSpeciesNode( species );
    }
    *result = value; 
    
    return SUCCESS;
}


static RET_VAL _VisitSpeciesToEvaluateWithCurrentConcentrations( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    double *result = NULL;
    double value = 0.0;
    SPECIES *species = NULL;    
    KINETIC_LAW_EVALUATER *evaluater = NULL;
    
    evaluater = (KINETIC_LAW_EVALUATER*)(visitor->_internal1);
    result = (double*)(visitor->_internal2);
    species = GetSpeciesFromKineticLaw( kineticLaw );
    
    //if( IsInitialQuantityInAmountInSpeciesNode( species ) ) {
    if( HasOnlySubstanceUnitsInSpeciesNode( species ) ) {
      value = GetAmountInSpeciesNode( species );
    } else {
      value = GetConcentrationInSpeciesNode( species );
    }
    //printf("Looking up %s found %g\n",
    //	   GetCharArrayOfString(GetSpeciesNodeID( species )),value);
    *result = value; 
    
    return SUCCESS;
}


