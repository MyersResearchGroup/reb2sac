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

static char * _GetPowKineticLawTransformationMethodID( ABSTRACTION_METHOD *method );
static RET_VAL _ApplyPowKineticLawTransformationMethod( ABSTRACTION_METHOD *method, IR *ir );      

static RET_VAL _TransformPowOfMultiplicationTerm( ABSTRACTION_METHOD *method, IR *ir, REACTION *reaction );
static RET_VAL _VisitOpToTransformPowOfMultiplicationTerm( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitIntToTransformPowOfMultiplicationTerm( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitRealToTransformPowOfMultiplicationTerm( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSpeciesToTransformPowOfMultiplicationTerm( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSymbolToTransformPowOfMultiplicationTerm( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );

static BOOL _IsMultiplicationTerm( KINETIC_LAW *kineticLaw );
static RET_VAL _VisitOpForMultiplicationTerm( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitIntForMultiplicationTerm( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitRealForMultiplicationTerm( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSpeciesForMultiplicationTerm( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSymbolForMultiplicationTerm( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );

static KINETIC_LAW *_CreateReplacement( KINETIC_LAW *kineticLaw, double power );
static RET_VAL _VisitOpToCreateReplacement( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitIntToCreateReplacement( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitRealToCreateReplacement( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSpeciesToCreateReplacement( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSymbolToCreateReplacement( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );


ABSTRACTION_METHOD *PowKineticLawTransformationMethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager ) {
    static ABSTRACTION_METHOD method;
    
    START_FUNCTION("PowKineticLawTransformationMethodConstructor");

    if( method.manager == NULL ) {
        method.manager = manager;
        method.GetID = _GetPowKineticLawTransformationMethodID;
        method.Apply = _ApplyPowKineticLawTransformationMethod;       
    }
    
    TRACE_0( "PowKineticLawTransformationMethodConstructor invoked" );
    
    END_FUNCTION("PowKineticLawTransformationMethodConstructor", SUCCESS );
    return &method;
}



static char * _GetPowKineticLawTransformationMethodID( ABSTRACTION_METHOD *method ) {
    START_FUNCTION("_GetPowKineticLawTransformationMethodID");
    
    END_FUNCTION("_GetPowKineticLawTransformationMethodID", SUCCESS );
    return "pow-kinetic-law-transformer";
}



static RET_VAL _ApplyPowKineticLawTransformationMethod( ABSTRACTION_METHOD *method, IR *ir ) {
    RET_VAL ret = SUCCESS;
    REACTION *reaction = NULL;
    LINKED_LIST *reactionList = NULL;
    
    START_FUNCTION("_ApplyPowKineticLawTransformationMethod");
    
    reactionList = ir->GetListOfReactionNodes( ir );
    ResetCurrentElement( reactionList );    
    while( ( reaction = (REACTION*)GetNextFromLinkedList( reactionList ) ) ) {
        if( IS_FAILED( ( ret = _TransformPowOfMultiplicationTerm( method, ir, reaction ) ) ) ) {
            END_FUNCTION("_ApplyPowKineticLawTransformationMethod", ret );
            return ret;
        }
    }
         
    END_FUNCTION("_ApplyPowKineticLawTransformationMethod", SUCCESS );
    return ret;
}      

static RET_VAL _TransformPowOfMultiplicationTerm( ABSTRACTION_METHOD *method, IR *ir, REACTION *reaction ) {
    static KINETIC_LAW_VISITOR visitor;
    RET_VAL ret = SUCCESS;    
    KINETIC_LAW *kineticLaw = NULL;
#ifdef DEBUG
    STRING *kineticLawString = NULL;
#endif        

    START_FUNCTION("_TransformPowOfMultiplicationTerm");

    if( visitor.VisitOp == NULL ) {
        visitor.VisitOp = _VisitOpToTransformPowOfMultiplicationTerm;
        visitor.VisitInt = _VisitIntToTransformPowOfMultiplicationTerm;
        visitor.VisitReal = _VisitRealToTransformPowOfMultiplicationTerm;
        visitor.VisitSpecies = _VisitSpeciesToTransformPowOfMultiplicationTerm;
        visitor.VisitSymbol = _VisitSymbolToTransformPowOfMultiplicationTerm;
    }    

    kineticLaw = GetKineticLawInReactionNode( reaction );
    
#ifdef DEBUG
    kineticLawString = ToStringKineticLaw( kineticLaw );
    printf("before: kinetic law of %s is: %s" NEW_LINE, GetCharArrayOfString( GetReactionNodeName( reaction ) ), GetCharArrayOfString( kineticLawString ) );
    FreeString( &kineticLawString );
#endif        
    
    if( IS_FAILED( ( ret = kineticLaw->Accept( kineticLaw, &visitor ) ) ) ) {    
        END_FUNCTION("_TransformPowOfMultiplicationTerm", ret );
        return ret;
    }     
#ifdef DEBUG
    kineticLawString = ToStringKineticLaw( kineticLaw );
    printf("after: kinetic law of %s is: %s" NEW_LINE, GetCharArrayOfString( GetReactionNodeName( reaction ) ), GetCharArrayOfString( kineticLawString ) );
    FreeString( &kineticLawString );
#endif        
    END_FUNCTION("_TransformPowOfMultiplicationTerm", SUCCESS );
    return ret;
}

static RET_VAL _VisitOpToTransformPowOfMultiplicationTerm( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    double rightValue = 0.0;
    KINETIC_LAW *left = NULL;    
    KINETIC_LAW *right = NULL;
    KINETIC_LAW *newKineticLaw = NULL;
    KINETIC_LAW *leftReplacement = NULL;    
    KINETIC_LAW *rightReplacement = NULL;    
    REB2SAC_SYMBOL *sym = NULL;    
#ifdef DEBUG
    STRING *kineticLawString = NULL;
#endif
                
    START_FUNCTION("_VisitOpToTransformPowOfMultiplicationTerm");
    
    left = GetOpLeftFromKineticLaw( kineticLaw );
    if( IS_FAILED( ( ret = left->Accept( left, visitor ) ) ) ) {
        END_FUNCTION("_VisitIntToTransformPowOfMultiplicationTerm", ret );
        return ret;
    }
    
    right = GetOpRightFromKineticLaw( kineticLaw );
    if( IS_FAILED( ( ret = right->Accept( right, visitor ) ) ) ) {
        END_FUNCTION("_VisitIntToTransformPowOfMultiplicationTerm", ret );
        return ret;
    }
    
    switch( GetOpTypeFromKineticLaw( kineticLaw ) ) {
        case KINETIC_LAW_OP_POW:
            if( !_IsMultiplicationTerm( left ) ) {
                END_FUNCTION("_VisitOpToTransformPowOfMultiplicationTerm", SUCCESS );
                return ret;
            }
            if( !IsConstantValueKineticLaw( right ) ) {
                END_FUNCTION("_VisitOpToTransformPowOfMultiplicationTerm", SUCCESS );
                return ret;
            }
            if( IsRealValueKineticLaw( right ) ) {
                rightValue = GetRealValueFromKineticLaw( right );
            }
            else if( IsSymbolKineticLaw( right ) ) {
                sym = GetSymbolFromKineticLaw( right );
                if( !IsRealValueSymbol( sym ) ) {
                    END_FUNCTION("_VisitOpToTransformPowOfMultiplicationTerm", SUCCESS );
                    return ret;
                }
                rightValue = GetRealValueInSymbol( sym );
            }
            else if( IsIntValueKineticLaw( right ) ) {
                rightValue = (double)GetIntValueFromKineticLaw( right );
            }
            else {
            }
            if( ( newKineticLaw = _CreateReplacement( left, rightValue ) ) == NULL ) {
                return FAILING;
            }
#ifdef DEBUG
            kineticLawString = ToStringKineticLaw( newKineticLaw );
            printf("replace kinetic law is: %s" NEW_LINE, GetCharArrayOfString( kineticLawString ) );
            FreeString( &kineticLawString );
#endif        
            if( ( leftReplacement = CloneKineticLaw( GetOpLeftFromKineticLaw( newKineticLaw ) ) ) == NULL ) {
                return FAILING;
            }
            if( ( rightReplacement = CloneKineticLaw( GetOpRightFromKineticLaw( newKineticLaw ) ) ) == NULL ) {
                return FAILING;
            }
            if( IS_FAILED( ( ret = SetOpKineticLaw( kineticLaw, 
                                                    GetOpTypeFromKineticLaw( newKineticLaw ),
                                                    leftReplacement,
                                                    rightReplacement ) ) ) ) {
                return FAILING;                
            }
            FreeKineticLaw( &newKineticLaw );
                        
        break;        
        
        default:
            END_FUNCTION("_VisitOpToTransformPowOfMultiplicationTerm", SUCCESS );
        return ret;       
    }
                
    END_FUNCTION("_VisitOpToTransformPowOfMultiplicationTerm", SUCCESS );
    return ret;
}

static RET_VAL _VisitIntToTransformPowOfMultiplicationTerm( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;        
    return ret;
}

static RET_VAL _VisitRealToTransformPowOfMultiplicationTerm( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;    
    return ret;
}

static RET_VAL _VisitSpeciesToTransformPowOfMultiplicationTerm( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;    
    return ret;
}

static RET_VAL _VisitSymbolToTransformPowOfMultiplicationTerm( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;    
    return ret;
}



static BOOL _IsMultiplicationTerm( KINETIC_LAW *kineticLaw ) {
    static KINETIC_LAW_VISITOR visitor;
    RET_VAL ret = SUCCESS;    
    BOOL isMultiplicationTerm = TRUE;

    if( !IsOpKineticLaw( kineticLaw ) ) {
        return FALSE;
    }
    if( visitor.VisitOp == NULL ) {
        visitor.VisitOp = _VisitOpForMultiplicationTerm;
        visitor.VisitInt = _VisitIntForMultiplicationTerm;
        visitor.VisitReal = _VisitRealForMultiplicationTerm;
        visitor.VisitSpecies = _VisitSpeciesForMultiplicationTerm;
        visitor.VisitSymbol = _VisitSymbolForMultiplicationTerm;
    }    

    visitor._internal1 = (CADDR_T)(&isMultiplicationTerm);
    if( IS_FAILED( ( ret = kineticLaw->Accept( kineticLaw, &visitor ) ) ) ) {    
        return FALSE;
    }
         
    return isMultiplicationTerm;
}

static RET_VAL _VisitOpForMultiplicationTerm( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    BOOL *isMultiplicationTerm = (BOOL*)(visitor->_internal1);
    KINETIC_LAW *left = NULL;    
    KINETIC_LAW *right = NULL;
        
    left = GetOpLeftFromKineticLaw( kineticLaw );
    if( IS_FAILED( ( ret = left->Accept( left, visitor ) ) ) ) {
        return ret;
    }
    
    right = GetOpRightFromKineticLaw( kineticLaw );
    if( IS_FAILED( ( ret = right->Accept( right, visitor ) ) ) ) {
        return ret;
    }
    if( !(*isMultiplicationTerm) ) {
        return ret;
    } 
    
    switch( GetOpTypeFromKineticLaw( kineticLaw ) ) {
        case KINETIC_LAW_OP_TIMES:
        case KINETIC_LAW_OP_DIVIDE:
        case KINETIC_LAW_OP_POW:
        break;        
        
        default:
            *isMultiplicationTerm = FALSE;                                
        break;        
    }
    
    return SUCCESS;
}

static RET_VAL _VisitIntForMultiplicationTerm( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    return SUCCESS;
}

static RET_VAL _VisitRealForMultiplicationTerm( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    return SUCCESS;
}

static RET_VAL _VisitSpeciesForMultiplicationTerm( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    return SUCCESS;
}

static RET_VAL _VisitSymbolForMultiplicationTerm( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    return SUCCESS;
}


static KINETIC_LAW *_CreateReplacement( KINETIC_LAW *kineticLaw, double power ) {
    static KINETIC_LAW_VISITOR visitor;
    RET_VAL ret = SUCCESS;    
    BOOL isMultiplicationTerm = FALSE;

    if( visitor.VisitOp == NULL ) {
        visitor.VisitOp = _VisitOpToCreateReplacement;
        visitor.VisitInt = _VisitIntToCreateReplacement;
        visitor.VisitReal = _VisitRealToCreateReplacement;
        visitor.VisitSpecies = _VisitSpeciesToCreateReplacement;
        visitor.VisitSymbol = _VisitSymbolToCreateReplacement;
    }    
    
    visitor._internal1 = NULL;
    visitor._internal2 = (CADDR_T)(&power);

    if( IS_FAILED( ( ret = kineticLaw->Accept( kineticLaw, &visitor ) ) ) ) {    
        return NULL;
    }
         
    return (KINETIC_LAW*)(visitor._internal1);
}

static RET_VAL _VisitOpToCreateReplacement( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    BYTE opType = (BYTE)0;
    BOOL isLeftOp = FALSE;
    BOOL isRightOp = FALSE;
    double rightValue = 0.0;
    double power = *((double*)(visitor->_internal2));
    KINETIC_LAW *newKineticLaw = NULL;    
    KINETIC_LAW *left = NULL;    
    KINETIC_LAW *right = NULL;
    KINETIC_LAW *leftReplacement = NULL;    
    KINETIC_LAW *rightReplacement = NULL;
    REB2SAC_SYMBOL *sym = NULL;    
        
    
    opType = GetOpTypeFromKineticLaw( kineticLaw );
    left = GetOpLeftFromKineticLaw( kineticLaw );
    if( IS_FAILED( ( ret = left->Accept( left, visitor ) ) ) ) {
        return ret;
    }    
    leftReplacement = (KINETIC_LAW*)(visitor->_internal1);
    
    right = GetOpRightFromKineticLaw( kineticLaw );
    if( IS_FAILED( ( ret = right->Accept( right, visitor ) ) ) ) {
        return ret;
    }
    rightReplacement = (KINETIC_LAW*)(visitor->_internal1);
    
    isLeftOp = IsOpKineticLaw( leftReplacement );
    isRightOp = IsOpKineticLaw( rightReplacement );
    
    if( isLeftOp && isRightOp ) {
        if( ( newKineticLaw = CreateOpKineticLaw( opType, leftReplacement, rightReplacement ) ) == NULL ) {
            return ErrorReport( FAILING, "_VisitOpToCreateReplacement", 
                                "failed to create an op kinetic law" );
        }
        visitor->_internal1 = (CADDR_T)(newKineticLaw);    
        return ret;
    }
    
    
    switch( opType ) {
        case KINETIC_LAW_OP_TIMES:
        case KINETIC_LAW_OP_DIVIDE:
            if( !isLeftOp ) {
                if( ( leftReplacement = CreateOpKineticLaw( 
                    KINETIC_LAW_OP_POW, leftReplacement, CreateRealValueKineticLaw( power ) ) ) 
                    == NULL ) {
                    return ErrorReport( FAILING, "_VisitOpToCreateReplacement", 
                                        "failed to create a left op kinetic law" );
                }
            }
            if( !isRightOp ) {
                if( ( rightReplacement = CreateOpKineticLaw( 
                    KINETIC_LAW_OP_POW, rightReplacement, CreateRealValueKineticLaw( power ) ) ) 
                    == NULL ) {
                    return ErrorReport( FAILING, "_VisitOpToCreateReplacement", 
                                            "failed to create a right op kinetic law" );
                }
            }
            if( ( newKineticLaw = CreateOpKineticLaw( opType, leftReplacement, rightReplacement ) ) == NULL ) {
                return ErrorReport( FAILING, "_VisitOpToCreateReplacement", 
                                    "failed to create an op kinetic law" );
            }
            visitor->_internal1 = (CADDR_T)(newKineticLaw);    
            return ret;
        break;
            
        case KINETIC_LAW_OP_POW:
            if( isLeftOp ) {
                if( ( newKineticLaw = CreateOpKineticLaw( opType, leftReplacement, rightReplacement ) ) == NULL ) {
                    return ErrorReport( FAILING, "_VisitOpToCreateReplacement", 
                                        "failed to create an op kinetic law" );
                }
                visitor->_internal1 = (CADDR_T)(newKineticLaw);    
                return ret;
            }
            if( IsConstantValueKineticLaw( rightReplacement ) ) {
                if( IsRealValueKineticLaw( rightReplacement ) ) {
                    rightValue = GetRealValueFromKineticLaw( rightReplacement );
                    if( IS_FAILED( ( ret = SetRealValueKineticLaw( rightReplacement, rightValue * power ) ) ) ) {
                        return ErrorReport( FAILING, "_VisitOpToCreateReplacement", 
                                            "failed to set real value in kinetic law" );
                    } 
                    if( ( newKineticLaw = CreateOpKineticLaw( opType, leftReplacement, rightReplacement ) ) == NULL ) {
                        return ErrorReport( FAILING, "_VisitOpToCreateReplacement", 
                                            "failed to create an op kinetic law" );
                    }                    
                    visitor->_internal1 = (CADDR_T)(newKineticLaw);    
                    return ret;
                }
                else if( IsIntValueKineticLaw( right ) ) {
                    rightValue = (double)GetIntValueFromKineticLaw( right );
                    if( IS_FAILED( ( ret = SetRealValueKineticLaw( rightReplacement, rightValue * power ) ) ) ) {
                        return ErrorReport( FAILING, "_VisitOpToCreateReplacement", 
                                            "failed to set real value in kinetic law" );
                    } 
                    if( ( newKineticLaw = CreateOpKineticLaw( opType, leftReplacement, rightReplacement ) ) == NULL ) {
                        return ErrorReport( FAILING, "_VisitOpToCreateReplacement", 
                                            "failed to create an op kinetic law" );
                    }                    
                    visitor->_internal1 = (CADDR_T)(newKineticLaw);    
                    return ret;
                }
                else if( IsSymbolKineticLaw( right ) ) {
                    sym = GetSymbolFromKineticLaw( right );
                    if( !IsRealValueSymbol( sym ) ) {
                        return ErrorReport( FAILING, "_VisitOpToCreateReplacement", 
                                            "failed to constant sym %s does not have a real value",
                                            GetCharArrayOfString( GetSymbolID( sym ) ) );
                    }
                    rightValue = GetRealValueInSymbol( sym );
                    if( IS_FAILED( ( ret = SetRealValueKineticLaw( rightReplacement, rightValue * power ) ) ) ) {
                        return ErrorReport( FAILING, "_VisitOpToCreateReplacement", 
                                            "failed to set real value in kinetic law" );
                    } 
                    if( ( newKineticLaw = CreateOpKineticLaw( opType, leftReplacement, rightReplacement ) ) == NULL ) {
                        return ErrorReport( FAILING, "_VisitOpToCreateReplacement", 
                                            "failed to create an op kinetic law" );
                    }                    
                    visitor->_internal1 = (CADDR_T)(newKineticLaw);    
                    return ret;
                }
                else {
                    return ErrorReport( FAILING, "_VisitOpToCreateReplacement", "unrecognized kinetic law" );
                }
            }
            else {
                if( ( rightReplacement = CreateOpKineticLaw( 
                      KINETIC_LAW_OP_TIMES, rightReplacement, CreateRealValueKineticLaw( power ) ) ) 
                      == NULL ) {
                    return FAILING;
                }
                if( ( newKineticLaw = CreateOpKineticLaw( opType, leftReplacement, rightReplacement ) ) == NULL ) {
                    return FAILING;
                }
                visitor->_internal1 = (CADDR_T)(newKineticLaw);    
                return ret;
            }
        break;        
        
        default:
        return ErrorReport( FAILING, "_VisitOpToCreateReplacement", "invalid op type" );
    }
    
    return ErrorReport( FAILING, "_VisitOpToCreateReplacement", "invalid op type" );
}

static RET_VAL _VisitIntToCreateReplacement( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    KINETIC_LAW *newKineticLaw = NULL;
    
    if( ( newKineticLaw = CreateIntValueKineticLaw( GetIntValueFromKineticLaw( kineticLaw ) ) ) == NULL ) {
        return ErrorReport( FAILING, "_VisitIntToCreateReplacement", "failed to create clone" );
    } 
    
    visitor->_internal1 = (CADDR_T)(newKineticLaw);
    
    return ret;
}

static RET_VAL _VisitRealToCreateReplacement( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    KINETIC_LAW *newKineticLaw = NULL;
    
    if( ( newKineticLaw = CreateRealValueKineticLaw( GetRealValueFromKineticLaw( kineticLaw ) ) ) == NULL ) {
        return ErrorReport( FAILING, "_VisitIntToCreateReplacement", "failed to create clone" );
    } 
    
    visitor->_internal1 = (CADDR_T)(newKineticLaw);
    
    return ret;
}

static RET_VAL _VisitSpeciesToCreateReplacement( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    KINETIC_LAW *newKineticLaw = NULL;
    
    if( ( newKineticLaw = CreateSpeciesKineticLaw( GetSpeciesFromKineticLaw( kineticLaw ) ) ) == NULL ) {
        return ErrorReport( FAILING, "_VisitIntToCreateReplacement", "failed to create clone" );
    } 
    
    visitor->_internal1 = (CADDR_T)(newKineticLaw);
    
    return ret;
}

static RET_VAL _VisitSymbolToCreateReplacement( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    KINETIC_LAW *newKineticLaw = NULL;
    
    if( ( newKineticLaw = CreateSymbolKineticLaw( GetSymbolFromKineticLaw( kineticLaw ) ) ) == NULL ) {
        return ErrorReport( FAILING, "_VisitIntToCreateReplacement", "failed to create clone" );
    } 
    
    visitor->_internal1 = (CADDR_T)(newKineticLaw);
    
    return ret;
}


