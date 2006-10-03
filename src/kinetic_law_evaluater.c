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
static double _EvaluateWithCurrentConcentrations( KINETIC_LAW_EVALUATER *evaluater, KINETIC_LAW *kineticLaw );       

static double _GetValue( KINETIC_LAW_EVALUATER *evaluater, SPECIES *species );       

static RET_VAL _VisitOpToEvaluate( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitIntToEvaluate( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitRealToEvaluate( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSpeciesToEvaluate( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSymbolToEvaluate( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );

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
    evaluater->EvaluateWithCurrentConcentrations = _EvaluateWithCurrentConcentrations;    
    
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
    if( ( element = (KINETIC_LAW_EVALUATION_ELEMENT*)GetValueFromHashTable( species, sizeof( species ), table ) ) != NULL ) {
        element->value = value;
        END_FUNCTION("_SetSpeciesValue", SUCCESS );        
        return ret;
    } 
    
    if( ( element = (KINETIC_LAW_EVALUATION_ELEMENT*)MALLOC( sizeof(KINETIC_LAW_EVALUATION_ELEMENT) ) ) == NULL ) {
        return ErrorReport( FAILING, "", "could not create an evaluation element: species %s value %g", GetCharArrayOfString( GetSpeciesNodeName( species ) ), value );
    } 
    
    element->species = species;
    element->value = value;
    
    if( IS_FAILED( ( ret = PutInHashTable( species, sizeof(species), element, table ) ) ) ) {
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
    if( ( element = (KINETIC_LAW_EVALUATION_ELEMENT*)GetValueFromHashTable( species, sizeof( species ), table ) ) != NULL ) {
        if( IS_FAILED( ( ret = RemoveFromHashTable( species, sizeof( species ), table ) ) ) ) {
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
        visitor.VisitOp = _VisitOpToEvaluate;
        visitor.VisitInt = _VisitIntToEvaluate;
        visitor.VisitReal = _VisitRealToEvaluate;
        visitor.VisitSpecies = _VisitSpeciesToEvaluate;
        visitor.VisitSymbol = _VisitSymbolToEvaluate;
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
        visitor.VisitOp = _VisitOpToEvaluate;
        visitor.VisitInt = _VisitIntToEvaluate;
        visitor.VisitReal = _VisitRealToEvaluate;
        visitor.VisitSpecies = _VisitSpeciesToEvaluateWithCurrentAmounts;
        visitor.VisitSymbol = _VisitSymbolToEvaluate;
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
        visitor.VisitOp = _VisitOpToEvaluate;
        visitor.VisitInt = _VisitIntToEvaluate;
        visitor.VisitReal = _VisitRealToEvaluate;
        visitor.VisitSpecies = _VisitSpeciesToEvaluateWithCurrentConcentrations;
        visitor.VisitSymbol = _VisitSymbolToEvaluate;
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
        
        default:
            END_FUNCTION("_VisitOpToEvaluate", E_WRONGDATA );
        return E_WRONGDATA;
    }
    
    visitor->_internal2 = (CADDR_T)result;
    
    END_FUNCTION("_VisitOpToEvaluate", SUCCESS );
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
    if( !IsSymbolConstant( sym ) ) {
        return ErrorReport( FAILING, "_VisitSymbolToEvaluate", "symbol %s is not a constant real", GetCharArrayOfString( GetSymbolID( sym ) ) );
    }
    if( !IsRealValueSymbol( sym ) ) {
        return ErrorReport( FAILING, "_VisitSymbolToEvaluate", "symbol %s is not a constant real", GetCharArrayOfString( GetSymbolID( sym ) ) );
    } 
    
    result = (double*)(visitor->_internal2);
    value = GetRealValueInSymbol( sym );
    *result = value; 
    
    END_FUNCTION("_VisitSymbolToEvaluate", SUCCESS );
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


static double _GetValue( KINETIC_LAW_EVALUATER *evaluater, SPECIES *species ) {
    double result = 0.0;
    KINETIC_LAW_EVALUATION_ELEMENT *element = NULL;
    HASH_TABLE *table = NULL;
    
    START_FUNCTION("_GetValue");

    table = evaluater->table;    
    if( ( element = (KINETIC_LAW_EVALUATION_ELEMENT*)GetValueFromHashTable( species, sizeof( species ), table ) ) != NULL ) {
        result = element->value;
    }
    else {
        result = evaluater->defaultValue;;
    } 
    TRACE_2("The value for %s is %g", GetCharArrayOfString( GetSpeciesNodeName( species ) ), result );
    
    END_FUNCTION("_GetValue", SUCCESS );        
    return result;
}       

static RET_VAL _VisitSpeciesToEvaluateWithCurrentAmounts( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    double *result = NULL;
    double value = 0.0;
    SPECIES *species = NULL;    
    KINETIC_LAW_EVALUATER *evaluater = NULL;
    
    evaluater = (KINETIC_LAW_EVALUATER*)(visitor->_internal1);
    result = (double*)(visitor->_internal2);
    species = GetSpeciesFromKineticLaw( kineticLaw );
    
    value = GetAmountInSpeciesNode( species );
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
    
    value = GetConcentrationInSpeciesNode( species );
    *result = value; 
    
    return SUCCESS;
}


