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

#include "kinetic_law_find_next_time.h"


static RET_VAL _SetSpeciesValue( KINETIC_LAW_FIND_NEXT_TIME *find_next_time, SPECIES *species, double value );
static RET_VAL _RemoveSpeciesValue( KINETIC_LAW_FIND_NEXT_TIME *find_next_time, SPECIES *species );
static RET_VAL _SetDefaultSpeciesValue( KINETIC_LAW_FIND_NEXT_TIME *find_next_time, double value ); 
static double _FindNextTime( KINETIC_LAW_FIND_NEXT_TIME *find_next_time, KINETIC_LAW *kineticLaw );       
static double _FindNextTimeWithCurrentAmounts( KINETIC_LAW_FIND_NEXT_TIME *find_next_time, KINETIC_LAW *kineticLaw );       
static double _FindNextTimeWithCurrentConcentrations( KINETIC_LAW_FIND_NEXT_TIME *find_next_time, KINETIC_LAW *kineticLaw );       
static double _FindNextTimeWithCurrentConcentrationsDeter( KINETIC_LAW_FIND_NEXT_TIME *find_next_time, KINETIC_LAW *kineticLaw );       

static double _GetValue( KINETIC_LAW_FIND_NEXT_TIME *find_next_time, SPECIES *species );       

static RET_VAL _VisitPWToFindNextTime( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitOpToFindNextTime( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitUnaryOpToFindNextTime( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitIntToFindNextTime( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitRealToFindNextTime( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSpeciesToFindNextTime( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitCompartmentToFindNextTime( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSymbolToFindNextTime( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitFunctionSymbolToFindNextTime( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitOpToFindNextTimeDeter( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitUnaryOpToFindNextTimeDeter( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );

static RET_VAL _VisitCompartmentToFindNextTimeWithCurrentSize( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSpeciesToFindNextTimeWithCurrentAmounts( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSpeciesToFindNextTimeWithCurrentConcentrations( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );


KINETIC_LAW_FIND_NEXT_TIME *CreateKineticLawFind_Next_Time() {
    KINETIC_LAW_FIND_NEXT_TIME *find_next_time = NULL;
    
    START_FUNCTION("FreeKineticLawFind_Next_Time");
    
    if( ( find_next_time = (KINETIC_LAW_FIND_NEXT_TIME*)MALLOC( sizeof(KINETIC_LAW_FIND_NEXT_TIME) ) ) == NULL ) {
        END_FUNCTION("FreeKineticLawFind_Next_Time", FAILING );        
        return NULL;
    }
    
    if( ( find_next_time->table = CreateHashTable( 32 ) ) == NULL ) {
        END_FUNCTION("FreeKineticLawFind_Next_Time", FAILING );        
        return NULL;
    }
    
    find_next_time->SetSpeciesValue = _SetSpeciesValue;
    find_next_time->RemoveSpeciesValue = _RemoveSpeciesValue;
    find_next_time->SetDefaultSpeciesValue = _SetDefaultSpeciesValue;
    find_next_time->FindNextTime =_FindNextTime;
    find_next_time->FindNextTimeWithCurrentAmounts = _FindNextTimeWithCurrentAmounts;
    find_next_time->FindNextTimeWithCurrentConcentrations = _FindNextTimeWithCurrentConcentrations;    
    find_next_time->FindNextTimeWithCurrentConcentrationsDeter = _FindNextTimeWithCurrentConcentrationsDeter;    
    
    END_FUNCTION("FreeKineticLawFind_Next_Time", SUCCESS );        
    return find_next_time;
}

RET_VAL FreeKineticLawFind_Next_Time( KINETIC_LAW_FIND_NEXT_TIME **find_next_time ) {
    RET_VAL ret = SUCCESS;
    KINETIC_LAW_FIND_NEXT_TIME *target = NULL;

    START_FUNCTION("FreeKineticLawFind_Next_Time");
    
    target = *find_next_time;    
    if( target == NULL ) {
        END_FUNCTION("FreeKineticLawFind_Next_Time", SUCCESS );        
        return ret;
    }
    
    DeleteHashTable( &(target->table) );
    FREE( *find_next_time );
        
    END_FUNCTION("FreeKineticLawFind_Next_Time", SUCCESS );        
    return ret;
}


static RET_VAL _SetSpeciesValue( KINETIC_LAW_FIND_NEXT_TIME *find_next_time, SPECIES *species, double value ) {
    RET_VAL ret = SUCCESS;
    KINETIC_LAW_FIND_NEXT_TIME_ELEMENT *element = NULL;
    HASH_TABLE *table = NULL;
    
    START_FUNCTION("_SetSpeciesValue");

    table = find_next_time->table;    
    if( ( element = (KINETIC_LAW_FIND_NEXT_TIME_ELEMENT*)GetValueFromHashTable( (CADDR_T)species, sizeof( species ), table ) ) != NULL ) {
        element->value = value;
        END_FUNCTION("_SetSpeciesValue", SUCCESS );        
        return ret;
    } 
    
    if( ( element = (KINETIC_LAW_FIND_NEXT_TIME_ELEMENT*)MALLOC( sizeof(KINETIC_LAW_FIND_NEXT_TIME_ELEMENT) ) ) == NULL ) {
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

static RET_VAL _RemoveSpeciesValue( KINETIC_LAW_FIND_NEXT_TIME *find_next_time, SPECIES *species ) {
    RET_VAL ret = SUCCESS;
    KINETIC_LAW_FIND_NEXT_TIME_ELEMENT *element = NULL;
    HASH_TABLE *table = NULL;

    START_FUNCTION("_RemoveSpeciesValue");
    
    table = find_next_time->table;    
    if( ( element = (KINETIC_LAW_FIND_NEXT_TIME_ELEMENT*)GetValueFromHashTable( (CADDR_T)species, sizeof( species ), table ) ) != NULL ) {
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

static RET_VAL _SetDefaultSpeciesValue( KINETIC_LAW_FIND_NEXT_TIME *find_next_time, double value ) {
    RET_VAL ret = SUCCESS;

    START_FUNCTION("_SetDefaultSpeciesValue");

    find_next_time->defaultValue = value;
        
    END_FUNCTION("_SetDefaultSpeciesValue", SUCCESS );
        
    return ret;
}

static double _FindNextTime( KINETIC_LAW_FIND_NEXT_TIME *find_next_time, KINETIC_LAW *kineticLaw ) {
    static KINETIC_LAW_VISITOR visitor;
    RET_VAL ret = SUCCESS;
    double result = 0.0;
    
    START_FUNCTION("_FindNextTime");
    
    KINETIC_LAW *massActionRatio = NULL;
    
    START_FUNCTION("_FindNextTime");
    
    if( visitor.VisitOp == NULL ) {
        visitor.VisitPW = _VisitPWToFindNextTime;
        visitor.VisitOp = _VisitOpToFindNextTime;
        visitor.VisitUnaryOp = _VisitUnaryOpToFindNextTime;
        visitor.VisitInt = _VisitIntToFindNextTime;
        visitor.VisitReal = _VisitRealToFindNextTime;
        visitor.VisitSpecies = _VisitSpeciesToFindNextTime;
        visitor.VisitCompartment = _VisitCompartmentToFindNextTime;
        visitor.VisitSymbol = _VisitSymbolToFindNextTime;
        visitor.VisitFunctionSymbol = _VisitFunctionSymbolToFindNextTime;
    }
    
    visitor._internal1 = (CADDR_T)find_next_time;
    visitor._internal2 = (CADDR_T)(&result);
    
    if( IS_FAILED( kineticLaw->Accept( kineticLaw, &visitor ) ) ) {
        END_FUNCTION("_FindNextTime", FAILING );
        return -1.0;
    } 
        
    END_FUNCTION("_FindNextTime", SUCCESS );        
    return result;
}     


static double _FindNextTimeWithCurrentAmounts( KINETIC_LAW_FIND_NEXT_TIME *find_next_time, KINETIC_LAW *kineticLaw ) {
    static KINETIC_LAW_VISITOR visitor;
    RET_VAL ret = SUCCESS;
    double result = 0.0;
    
    START_FUNCTION("_FindNextTime");
    
    KINETIC_LAW *massActionRatio = NULL;
    
    START_FUNCTION("_FindNextTime");
    
    if( visitor.VisitOp == NULL ) {
        visitor.VisitPW = _VisitPWToFindNextTime;
        visitor.VisitOp = _VisitOpToFindNextTime;
        visitor.VisitUnaryOp = _VisitUnaryOpToFindNextTime;
        visitor.VisitInt = _VisitIntToFindNextTime;
        visitor.VisitReal = _VisitRealToFindNextTime;
        visitor.VisitSpecies = _VisitSpeciesToFindNextTimeWithCurrentAmounts;
        visitor.VisitCompartment = _VisitCompartmentToFindNextTimeWithCurrentSize;
        visitor.VisitSymbol = _VisitSymbolToFindNextTime;
	visitor.VisitFunctionSymbol = _VisitFunctionSymbolToFindNextTime;
    }
    
    visitor._internal1 = (CADDR_T)find_next_time;
    visitor._internal2 = (CADDR_T)(&result);
    
    if( IS_FAILED( kineticLaw->Accept( kineticLaw, &visitor ) ) ) {
        END_FUNCTION("_FindNextTime", FAILING );
        return -1.0;
    } 
        
    END_FUNCTION("_FindNextTime", SUCCESS );        
    return result;
}     


static double _FindNextTimeWithCurrentConcentrations( KINETIC_LAW_FIND_NEXT_TIME *find_next_time, KINETIC_LAW *kineticLaw ) {
    static KINETIC_LAW_VISITOR visitor;
    RET_VAL ret = SUCCESS;
    double result = 0.0;
    
    START_FUNCTION("_FindNextTime");
    
    KINETIC_LAW *massActionRatio = NULL;
    
    START_FUNCTION("_FindNextTime");
    
    if( visitor.VisitOp == NULL ) {
        visitor.VisitPW = _VisitPWToFindNextTime;
        visitor.VisitOp = _VisitOpToFindNextTime;
        visitor.VisitUnaryOp = _VisitUnaryOpToFindNextTime;
        visitor.VisitInt = _VisitIntToFindNextTime;
        visitor.VisitReal = _VisitRealToFindNextTime;
        visitor.VisitSpecies = _VisitSpeciesToFindNextTimeWithCurrentConcentrations;
        visitor.VisitCompartment = _VisitCompartmentToFindNextTimeWithCurrentSize;
        visitor.VisitSymbol = _VisitSymbolToFindNextTime;
	visitor.VisitFunctionSymbol = _VisitFunctionSymbolToFindNextTime;
    }
    
    visitor._internal1 = (CADDR_T)find_next_time;
    visitor._internal2 = (CADDR_T)(&result);

    if( IS_FAILED( kineticLaw->Accept( kineticLaw, &visitor ) ) ) {
        END_FUNCTION("_FindNextTime", FAILING );
        return -1.0;
    } 
        
    END_FUNCTION("_FindNextTime", SUCCESS );        
    return result;
}     


static double _FindNextTimeWithCurrentConcentrationsDeter( KINETIC_LAW_FIND_NEXT_TIME *find_next_time, KINETIC_LAW *kineticLaw ) {
    static KINETIC_LAW_VISITOR visitor;
    RET_VAL ret = SUCCESS;
    double result = 0.0;
    
    START_FUNCTION("_FindNextTime");
    
    KINETIC_LAW *massActionRatio = NULL;
    
    START_FUNCTION("_FindNextTime");
    
    if( visitor.VisitOp == NULL ) {
        visitor.VisitPW = _VisitPWToFindNextTime;
        visitor.VisitOp = _VisitOpToFindNextTimeDeter;
        visitor.VisitUnaryOp = _VisitUnaryOpToFindNextTimeDeter;
        visitor.VisitInt = _VisitIntToFindNextTime;
        visitor.VisitReal = _VisitRealToFindNextTime;
        visitor.VisitSpecies = _VisitSpeciesToFindNextTimeWithCurrentConcentrations;
        visitor.VisitCompartment = _VisitCompartmentToFindNextTimeWithCurrentSize;
        visitor.VisitSymbol = _VisitSymbolToFindNextTime;
	visitor.VisitFunctionSymbol = _VisitFunctionSymbolToFindNextTime;
    }
    
    visitor._internal1 = (CADDR_T)find_next_time;
    visitor._internal2 = (CADDR_T)(&result);

    if( IS_FAILED( kineticLaw->Accept( kineticLaw, &visitor ) ) ) {
        END_FUNCTION("_FindNextTime", FAILING );
        return -1.0;
    } 
        
    END_FUNCTION("_FindNextTime", SUCCESS );        
    return result;
}     


static RET_VAL _VisitPWToFindNextTime( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    BYTE opType = 0x00;
    double childValue = 0.0;
    double *result = NULL;
    KINETIC_LAW *child = NULL;
    LINKED_LIST *children = NULL;
    UINT num = 0;
    UINT i = 0;
    
    START_FUNCTION("_VisitPWToFindNextTime");

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
	  END_FUNCTION("_VisitPWToFindNextTime", ret );
	  return ret;
	}
	if (childValue) {
	  child = (KINETIC_LAW*)GetElementByIndex( i-1,children );
	  visitor->_internal2 = (CADDR_T)(&childValue);
	  if( IS_FAILED( ( ret = child->Accept( child, visitor ) ) ) ) {
	    END_FUNCTION("_VisitPWToFindNextTime", ret );
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
	  END_FUNCTION("_VisitPWToFindNextTime", ret );
	  return ret;
	}
	*result = childValue;
	break;
      }
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
    default:
      END_FUNCTION("_VisitUnaryOpToFindNextTime", E_WRONGDATA );
      return E_WRONGDATA;
    }

    visitor->_internal2 = (CADDR_T)result;
    
    END_FUNCTION("_VisitOpToFindNextTime", SUCCESS );
    return ret;
}

static RET_VAL _VisitOpToFindNextTime( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    BYTE opType = 0x00;
    double leftValue = 0.0;
    double rightValue = 0.0;
    double *result = NULL;
    KINETIC_LAW *left = NULL;
    KINETIC_LAW *right = NULL;
    
    START_FUNCTION("_VisitOpToFindNextTime");
    
    result = (double*)(visitor->_internal2);
    
    left = GetOpLeftFromKineticLaw( kineticLaw );
    visitor->_internal2 = (CADDR_T)(&leftValue);
    if( IS_FAILED( ( ret = left->Accept( left, visitor ) ) ) ) {
        END_FUNCTION("_VisitOpToFindNextTime", ret );
        return ret;
    }
    
    right = GetOpRightFromKineticLaw( kineticLaw );       
    visitor->_internal2 = (CADDR_T)(&rightValue);
    if( IS_FAILED( ( ret = right->Accept( right, visitor ) ) ) ) {
        END_FUNCTION("_VisitOpToFindNextTime", ret );
        return ret;
    }            
    
    opType = GetOpTypeFromKineticLaw( kineticLaw );
    switch( opType ) {
        case KINETIC_LAW_OP_PLUS:
            *result = leftValue + rightValue;
	    /*
	    if ( left->valueType == KINETIC_LAW_VALUE_TYPE_SYMBOL ) {
	      if (strcmp(GetCharArrayOfString( GetSymbolID( GetSymbolFromKineticLaw( left ) ) ), "t") == 0) {
		*result = (leftValue - rightValue);
	      }
	    } else if ( right->valueType == KINETIC_LAW_VALUE_TYPE_SYMBOL ) {
	      if (strcmp(GetCharArrayOfString( GetSymbolID( GetSymbolFromKineticLaw( right ) ) ), "t") == 0) {
		*result = (rightValue - leftValue);
	      }
	      } */
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
        
        case KINETIC_LAW_OP_DELAY:
	  *result = leftValue;
        break;        

        case KINETIC_LAW_OP_ROOT:
	  *result = pow(rightValue,(1./leftValue));
        break;        
	/*
        case KINETIC_LAW_OP_XOR:
	  if (leftValue < rightValue) {
            *result = leftValue;
	  } else {
            *result = rightValue;
	  }
        break;

        case KINETIC_LAW_OP_AND:
	  if (leftValue < rightValue) {
            *result = leftValue;
	  } else {
            *result = rightValue;
	  }
        break;

        case KINETIC_LAW_OP_OR:
	  if (leftValue < rightValue) {
            *result = leftValue;
	  } else {
            *result = rightValue;
	  }
        break;
	*/
        case KINETIC_LAW_OP_EQ:
	  *result = DBL_MAX;
	  if (leftValue < rightValue) {
	    if ( left->valueType == KINETIC_LAW_VALUE_TYPE_SYMBOL ) {
	      if ((strcmp(GetCharArrayOfString( GetSymbolID( GetSymbolFromKineticLaw( left ) ) ), "t") == 0) ||
		  (strcmp(GetCharArrayOfString( GetSymbolID( GetSymbolFromKineticLaw( left ) ) ), "time") == 0)) {
		*result = (rightValue - leftValue);
	      }
	    } 
	  }
        break;

        case KINETIC_LAW_OP_NEQ:
	  *result = DBL_MAX;
	  if (leftValue < rightValue) {
	    if ( left->valueType == KINETIC_LAW_VALUE_TYPE_SYMBOL ) {
	      if ((strcmp(GetCharArrayOfString( GetSymbolID( GetSymbolFromKineticLaw( left ) ) ), "t") == 0) ||
		  (strcmp(GetCharArrayOfString( GetSymbolID( GetSymbolFromKineticLaw( left ) ) ), "time") == 0)) {
		*result = (rightValue - leftValue);
	      }
	    } 
	  }
        break;

        case KINETIC_LAW_OP_GEQ:
	  *result = DBL_MAX;
	  if (leftValue < rightValue) {
	    if ( left->valueType == KINETIC_LAW_VALUE_TYPE_SYMBOL ) {
	      if ((strcmp(GetCharArrayOfString( GetSymbolID( GetSymbolFromKineticLaw( left ) ) ), "t") == 0) ||
		  (strcmp(GetCharArrayOfString( GetSymbolID( GetSymbolFromKineticLaw( left ) ) ), "time") == 0)) {
		*result = (rightValue - leftValue);
	      }
	    } 
	  }
        break;

        case KINETIC_LAW_OP_GT:
	  *result = DBL_MAX;
	  if (leftValue <= rightValue) {
	    if ( left->valueType == KINETIC_LAW_VALUE_TYPE_SYMBOL ) {
	      if ((strcmp(GetCharArrayOfString( GetSymbolID( GetSymbolFromKineticLaw( left ) ) ), "t") == 0) ||
		  (strcmp(GetCharArrayOfString( GetSymbolID( GetSymbolFromKineticLaw( left ) ) ), "time") == 0)) {
		*result = (rightValue - leftValue) + 0.0000001;
	      }
	    } 
	  }
        break;

        case KINETIC_LAW_OP_LEQ:
	  *result = DBL_MAX;
	  if (leftValue <= rightValue) {
	    if ( left->valueType == KINETIC_LAW_VALUE_TYPE_SYMBOL ) {
	      if ((strcmp(GetCharArrayOfString( GetSymbolID( GetSymbolFromKineticLaw( left ) ) ), "t") == 0) ||
		  (strcmp(GetCharArrayOfString( GetSymbolID( GetSymbolFromKineticLaw( left ) ) ), "time") == 0)) {
		*result = (rightValue - leftValue) + 0.0000001;
	      }
	    } 
	  }
        break;

        case KINETIC_LAW_OP_LT:
	  *result = DBL_MAX;
	  if (leftValue < rightValue) {
	    if ( left->valueType == KINETIC_LAW_VALUE_TYPE_SYMBOL ) {
	      if ((strcmp(GetCharArrayOfString( GetSymbolID( GetSymbolFromKineticLaw( left ) ) ), "t") == 0) ||
		  (strcmp(GetCharArrayOfString( GetSymbolID( GetSymbolFromKineticLaw( left ) ) ), "time") == 0)) {
		*result = (rightValue - leftValue);
	      }
	    } 
	  }
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
        
        default:
            END_FUNCTION("_VisitOpToFindNextTime", E_WRONGDATA );
        return E_WRONGDATA;
    }
    
    visitor->_internal2 = (CADDR_T)result;
    
    END_FUNCTION("_VisitOpToFindNextTime", SUCCESS );
    return ret;
}

static RET_VAL _VisitUnaryOpToFindNextTime( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    BYTE opType = 0x00;
    double childValue = 0.0;
    double *result = NULL;
    KINETIC_LAW *child = NULL;
    UINT i = 0;
    START_FUNCTION("_VisitUnaryOpToFindNextTime");
    
    result = (double*)(visitor->_internal2);
    
    child = GetUnaryOpChildFromKineticLaw( kineticLaw );
    visitor->_internal2 = (CADDR_T)(&childValue);
    if( IS_FAILED( ( ret = child->Accept( child, visitor ) ) ) ) {
        END_FUNCTION("_VisitUnaryOpToFindNextTime", ret );
        return ret;
    }
    
    opType = GetUnaryOpTypeFromKineticLaw( kineticLaw );     
    switch( opType ) {
        case KINETIC_LAW_UNARY_OP_NOT:
	  *result = childValue;
        break;
        case KINETIC_LAW_UNARY_OP_ABS:
	  *result = fabs(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_COT:
	  *result = cos(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_COTH:
	  *result = cosh(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_CSC:
	  *result = cos(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_CSCH:
	  *result = cosh(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_SEC:
	  *result = cos(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_SECH:
	  *result = cosh(childValue);
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
	  *result = atan( 1. / SQRT( (childValue-1.)*(childValue+1.) )); 
        break;
        case KINETIC_LAW_UNARY_OP_ARCCSCH:
	  *result = log((1.+SQRT((1+SQR(childValue)))) /childValue);
        break;
        case KINETIC_LAW_UNARY_OP_ARCSEC:
	  *result = atan( SQRT(( childValue-1.)*( childValue+1.)) );
        break;
        case KINETIC_LAW_UNARY_OP_ARCSECH:
	  *result = log((1.+pow((1-SQR(childValue)),0.5))/childValue);
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
        case KINETIC_LAW_UNARY_OP_LOG:
	  *result = log10(childValue);
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
        
        default:
            END_FUNCTION("_VisitUnaryOpToFindNextTime", E_WRONGDATA );
        return E_WRONGDATA;
    }
    
    visitor->_internal2 = (CADDR_T)result;

    END_FUNCTION("_VisitUnaryOpToFindNextTime", SUCCESS );
    return ret;
}

static RET_VAL _VisitOpToFindNextTimeDeter( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    BYTE opType = 0x00;
    double leftValue = 0.0;
    double rightValue = 0.0;
    double *result = NULL;
    KINETIC_LAW *left = NULL;
    KINETIC_LAW *right = NULL;
    
    START_FUNCTION("_VisitOpToFindNextTime");
    
    result = (double*)(visitor->_internal2);
    
    left = GetOpLeftFromKineticLaw( kineticLaw );
    visitor->_internal2 = (CADDR_T)(&leftValue);
    if( IS_FAILED( ( ret = left->Accept( left, visitor ) ) ) ) {
        END_FUNCTION("_VisitOpToFindNextTime", ret );
        return ret;
    }
    
    right = GetOpRightFromKineticLaw( kineticLaw );       
    visitor->_internal2 = (CADDR_T)(&rightValue);
    if( IS_FAILED( ( ret = right->Accept( right, visitor ) ) ) ) {
        END_FUNCTION("_VisitOpToFindNextTime", ret );
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
        
        case KINETIC_LAW_OP_DELAY:
	  *result = leftValue;
        break;        
        
        case KINETIC_LAW_OP_ROOT:
	  *result = pow(rightValue,(1./leftValue));
        break;        

        case KINETIC_LAW_OP_XOR:
	  if (leftValue < rightValue) {
            *result = leftValue;
	  } else {
            *result = rightValue;
	  }
        break;

        case KINETIC_LAW_OP_AND:
	  if (leftValue < rightValue) {
            *result = leftValue;
	  } else {
            *result = rightValue;
	  }
        break;

        case KINETIC_LAW_OP_EQ:
	  *result = DBL_MAX;
	  if (leftValue < rightValue) {
	    if ( left->valueType == KINETIC_LAW_VALUE_TYPE_SYMBOL ) {
	      if ((strcmp(GetCharArrayOfString( GetSymbolID( GetSymbolFromKineticLaw( left ) ) ), "t") == 0) ||
		  (strcmp(GetCharArrayOfString( GetSymbolID( GetSymbolFromKineticLaw( left ) ) ), "time") == 0)) {
		*result = (rightValue - leftValue);
	      }
	    } 
	  }
        break;

        case KINETIC_LAW_OP_NEQ:
	  *result = DBL_MAX;
	  if (leftValue < rightValue) {
	    if ( left->valueType == KINETIC_LAW_VALUE_TYPE_SYMBOL ) {
	      if ((strcmp(GetCharArrayOfString( GetSymbolID( GetSymbolFromKineticLaw( left ) ) ), "t") == 0) ||
		  (strcmp(GetCharArrayOfString( GetSymbolID( GetSymbolFromKineticLaw( left ) ) ), "time") == 0)) {
		*result = (rightValue - leftValue);
	      }
	    } 
	  }
        break;

        case KINETIC_LAW_OP_GEQ:
	  *result = DBL_MAX;
	  if (leftValue < rightValue) {
	    if ( left->valueType == KINETIC_LAW_VALUE_TYPE_SYMBOL ) {
	      if ((strcmp(GetCharArrayOfString( GetSymbolID( GetSymbolFromKineticLaw( left ) ) ), "t") == 0) ||
		  (strcmp(GetCharArrayOfString( GetSymbolID( GetSymbolFromKineticLaw( left ) ) ), "time") == 0)) {
		*result = (rightValue - leftValue);
	      }
	    } 
	  }
        break;

        case KINETIC_LAW_OP_GT:
	  *result = DBL_MAX;
	  if (leftValue <= rightValue) {
	    if ( left->valueType == KINETIC_LAW_VALUE_TYPE_SYMBOL ) {
	      if ((strcmp(GetCharArrayOfString( GetSymbolID( GetSymbolFromKineticLaw( left ) ) ), "t") == 0) ||
		  (strcmp(GetCharArrayOfString( GetSymbolID( GetSymbolFromKineticLaw( left ) ) ), "time") == 0)) {
		*result = (rightValue - leftValue) + 0.0000001;
	      }
	    } 
	  }
        break;

        case KINETIC_LAW_OP_LEQ:
	  *result = DBL_MAX;
	  if (leftValue <= rightValue) {
	    if ( left->valueType == KINETIC_LAW_VALUE_TYPE_SYMBOL ) {
	      if ((strcmp(GetCharArrayOfString( GetSymbolID( GetSymbolFromKineticLaw( left ) ) ), "t") == 0) ||
		  (strcmp(GetCharArrayOfString( GetSymbolID( GetSymbolFromKineticLaw( left ) ) ), "time") == 0)) {
		*result = (rightValue - leftValue) + 0.0000001;
	      }
	    } 
	  }
        break;

        case KINETIC_LAW_OP_LT:
	  *result = DBL_MAX;
	  if (leftValue < rightValue) {
	    if ( left->valueType == KINETIC_LAW_VALUE_TYPE_SYMBOL ) {
	      if ((strcmp(GetCharArrayOfString( GetSymbolID( GetSymbolFromKineticLaw( left ) ) ), "t") == 0) ||
		  (strcmp(GetCharArrayOfString( GetSymbolID( GetSymbolFromKineticLaw( left ) ) ), "time") == 0)) {
		*result = (rightValue - leftValue);
	      }
	    } 
	  }
        break;

        case KINETIC_LAW_OP_OR:
	  if (leftValue < rightValue) {
            *result = leftValue;
	  } else {
            *result = rightValue;
	  }
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
        
        default:
            END_FUNCTION("_VisitOpToFindNextTime", E_WRONGDATA );
        return E_WRONGDATA;
    }
    
    visitor->_internal2 = (CADDR_T)result;
    
    END_FUNCTION("_VisitOpToFindNextTime", SUCCESS );
    return ret;
}

static RET_VAL _VisitUnaryOpToFindNextTimeDeter( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    BYTE opType = 0x00;
    double childValue = 0.0;
    double *result = NULL;
    KINETIC_LAW *child = NULL;
    UINT i = 0;
    START_FUNCTION("_VisitUnaryOpToFindNextTime");
    
    result = (double*)(visitor->_internal2);
    
    child = GetUnaryOpChildFromKineticLaw( kineticLaw );
    visitor->_internal2 = (CADDR_T)(&childValue);
    if( IS_FAILED( ( ret = child->Accept( child, visitor ) ) ) ) {
        END_FUNCTION("_VisitUnaryOpToFindNextTime", ret );
        return ret;
    }
    
    opType = GetUnaryOpTypeFromKineticLaw( kineticLaw );     
    switch( opType ) {
        case KINETIC_LAW_UNARY_OP_NEG:
	  *result = (-1)*childValue;
	break;
        case KINETIC_LAW_UNARY_OP_NOT:
	  *result = childValue;
	break;
        case KINETIC_LAW_UNARY_OP_ABS:
	  *result = fabs(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_COT:
	  *result = cos(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_COTH:
	  *result = cosh(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_CSC:
	  *result = cos(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_CSCH:
	  *result = cosh(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_SEC:
	  *result = cos(childValue);
        break;
        case KINETIC_LAW_UNARY_OP_SECH:
	  *result = cosh(childValue);
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
	  *result = atan( 1. / SQRT( (childValue-1.)*(childValue+1.) )); 
        break;
        case KINETIC_LAW_UNARY_OP_ARCCSCH:
	  *result = log((1.+SQRT((1+SQR(childValue)))) /childValue);
        break;
        case KINETIC_LAW_UNARY_OP_ARCSEC:
	  *result = atan( SQRT(( childValue-1.)*( childValue+1.)) );
        break;
        case KINETIC_LAW_UNARY_OP_ARCSECH:
	  *result = log((1.+pow((1-SQR(childValue)),0.5))/childValue);
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
        case KINETIC_LAW_UNARY_OP_LOG:
	  *result = log10(childValue);
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
	  *result = childValue;
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
        
        default:
            END_FUNCTION("_VisitUnaryOpToFindNextTime", E_WRONGDATA );
        return E_WRONGDATA;
    }
    
    visitor->_internal2 = (CADDR_T)result;

    END_FUNCTION("_VisitUnaryOpToFindNextTime", SUCCESS );
    return ret;
}

static RET_VAL _VisitIntToFindNextTime( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    double *result = NULL;
    double value = 0.0;
    START_FUNCTION("_VisitIntToFindNextTime");
        
    result = (double*)(visitor->_internal2);
    value = (double)GetIntValueFromKineticLaw( kineticLaw );
    *result = value; 
    
    END_FUNCTION("_VisitIntToFindNextTime", SUCCESS );
    return SUCCESS;
}

static RET_VAL _VisitRealToFindNextTime( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    double *result = NULL;
    double value = 0.0;
    
    START_FUNCTION("_VisitRealToFindNextTime");
    
    result = (double*)(visitor->_internal2);
    value = GetRealValueFromKineticLaw( kineticLaw );
    *result = value; 
    
    END_FUNCTION("_VisitRealToFindNextTime", SUCCESS );
    return SUCCESS;
}

static RET_VAL _VisitSymbolToFindNextTime( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    double *result = NULL;
    double value = 0.0;
    REB2SAC_SYMBOL *sym = NULL;
    
    START_FUNCTION("_VisitSymbolToFindNextTime");
    
    sym = GetSymbolFromKineticLaw( kineticLaw );
    /*
    if( !IsSymbolConstant( sym ) ) {
        return ErrorReport( FAILING, "_VisitSymbolToFindNextTime", "symbol %s is not a constant real", GetCharArrayOfString( GetSymbolID( sym ) ) );
    }
    */
    if( !IsRealValueSymbol( sym ) ) {
        return ErrorReport( FAILING, "_VisitSymbolToFindNextTime", "symbol %s is not a constant real", GetCharArrayOfString( GetSymbolID( sym ) ) );
    } 
    
    result = (double*)(visitor->_internal2);
    value = GetCurrentRealValueInSymbol( sym );

    *result = value; 

    END_FUNCTION("_VisitSymbolToFindNextTime", SUCCESS );
    return SUCCESS;
}

static RET_VAL _VisitFunctionSymbolToFindNextTime( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    
    START_FUNCTION("_VisitFunctionSymbolToFindNextTime");
    
    END_FUNCTION("_VisitFunctionSymbolToFindNextTime", SUCCESS );
    return SUCCESS;
}

static RET_VAL _VisitSpeciesToFindNextTime( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    double *result = NULL;
    double value = 0.0;
    SPECIES *species = NULL;    
    KINETIC_LAW_FIND_NEXT_TIME *find_next_time = NULL;
    
    START_FUNCTION("_VisitSpeciesToFindNextTime");
    
    find_next_time = (KINETIC_LAW_FIND_NEXT_TIME*)(visitor->_internal1);
    result = (double*)(visitor->_internal2);
    species = GetSpeciesFromKineticLaw( kineticLaw );
    
    value = _GetValue( find_next_time, species );
    *result = value; 
            
    END_FUNCTION("_VisitSpeciesToFindNextTime", SUCCESS );
    return SUCCESS;
}

static RET_VAL _VisitCompartmentToFindNextTime( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    double *result = NULL;
    double value = 0.0;
    COMPARTMENT *compartment = NULL;    
    KINETIC_LAW_FIND_NEXT_TIME *find_next_time = NULL;
    
    START_FUNCTION("_VisitCompartmentToFindNextTime");
    
    find_next_time = (KINETIC_LAW_FIND_NEXT_TIME*)(visitor->_internal1);
    result = (double*)(visitor->_internal2);
    compartment = GetCompartmentFromKineticLaw( kineticLaw );
    
    value = GetCurrentSizeInCompartment( compartment );
    *result = value; 
            
    END_FUNCTION("_VisitCompartmentToFindNextTime", SUCCESS );
    return SUCCESS;
}


static double _GetValue( KINETIC_LAW_FIND_NEXT_TIME *find_next_time, SPECIES *species ) {
    double result = 0.0;
    KINETIC_LAW_FIND_NEXT_TIME_ELEMENT *element = NULL;
    HASH_TABLE *table = NULL;
    
    START_FUNCTION("_GetValue");

    table = find_next_time->table;    
    if( ( element = (KINETIC_LAW_FIND_NEXT_TIME_ELEMENT*)GetValueFromHashTable( (CADDR_T)species, sizeof( species ), table ) ) != NULL ) {
        result = element->value;
    }
    else {
        result = find_next_time->defaultValue;;
    } 
    TRACE_2("The value for %s is %g", GetCharArrayOfString( GetSpeciesNodeName( species ) ), result );
    
    END_FUNCTION("_GetValue", SUCCESS );        
    return result;
}       

static RET_VAL _VisitCompartmentToFindNextTimeWithCurrentSize( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    double *result = NULL;
    double value = 0.0;
    COMPARTMENT *compartment = NULL;    
    KINETIC_LAW_FIND_NEXT_TIME *find_next_time = NULL;
    
    find_next_time = (KINETIC_LAW_FIND_NEXT_TIME*)(visitor->_internal1);
    result = (double*)(visitor->_internal2);
    compartment = GetCompartmentFromKineticLaw( kineticLaw );
    
    value = GetCurrentSizeInCompartment( compartment );
    *result = value; 
    
    return SUCCESS;
}

static RET_VAL _VisitSpeciesToFindNextTimeWithCurrentAmounts( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    double *result = NULL;
    double value = 0.0;
    SPECIES *species = NULL;    
    KINETIC_LAW_FIND_NEXT_TIME *find_next_time = NULL;
    
    find_next_time = (KINETIC_LAW_FIND_NEXT_TIME*)(visitor->_internal1);
    result = (double*)(visitor->_internal2);
    species = GetSpeciesFromKineticLaw( kineticLaw );
    
    value = GetAmountInSpeciesNode( species );
    *result = value; 
    
    return SUCCESS;
}


static RET_VAL _VisitSpeciesToFindNextTimeWithCurrentConcentrations( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    double *result = NULL;
    double value = 0.0;
    SPECIES *species = NULL;    
    KINETIC_LAW_FIND_NEXT_TIME *find_next_time = NULL;
    
    find_next_time = (KINETIC_LAW_FIND_NEXT_TIME*)(visitor->_internal1);
    result = (double*)(visitor->_internal2);
    species = GetSpeciesFromKineticLaw( kineticLaw );
    
    value = GetConcentrationInSpeciesNode( species );
    //printf("Looking up %s found %g\n",
    //	   GetCharArrayOfString(GetSpeciesNodeID( species )),value);
    *result = value; 
    
    return SUCCESS;
}


