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

#include "kinetic_law_support.h"


static LINKED_LIST* _Support( KINETIC_LAW_SUPPORT *support, KINETIC_LAW *kineticLaw );       

static RET_VAL _VisitPWToSupport( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitOpToSupport( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitUnaryOpToSupport( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitIntToSupport( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitRealToSupport( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSpeciesToSupport( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitCompartmentToSupport( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSymbolToSupport( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitFunctionSymbolToSupport( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );

KINETIC_LAW_SUPPORT *CreateKineticLawSupport() {
    KINETIC_LAW_SUPPORT *support = NULL;
    
    START_FUNCTION("FreeKineticLawSupport");
    
    if( ( support = (KINETIC_LAW_SUPPORT*)MALLOC( sizeof(KINETIC_LAW_SUPPORT) ) ) == NULL ) {
        END_FUNCTION("FreeKineticLawSupport", FAILING );        
        return NULL;
    }
    
    support->Support =_Support;
    
    END_FUNCTION("FreeKineticLawSupport", SUCCESS );        
    return support;
}

RET_VAL FreeKineticLawSupport( KINETIC_LAW_SUPPORT **support ) {
    RET_VAL ret = SUCCESS;
    KINETIC_LAW_SUPPORT *target = NULL;

    START_FUNCTION("FreeKineticLawSupport");
    
    target = *support;    
    if( target == NULL ) {
        END_FUNCTION("FreeKineticLawSupport", SUCCESS );        
        return ret;
    }
    
    FREE( *support );
        
    END_FUNCTION("FreeKineticLawSupport", SUCCESS );        
    return ret;
}


static LINKED_LIST* _Support( KINETIC_LAW_SUPPORT *support, KINETIC_LAW *kineticLaw ) {
    static KINETIC_LAW_VISITOR visitor;
    RET_VAL ret = SUCCESS;

    START_FUNCTION("_Support");

    LINKED_LIST *result = CreateLinkedList();
    
    if( visitor.VisitOp == NULL ) {
        visitor.VisitPW = _VisitPWToSupport;
        visitor.VisitOp = _VisitOpToSupport;
        visitor.VisitUnaryOp = _VisitUnaryOpToSupport;
        visitor.VisitInt = _VisitIntToSupport;
        visitor.VisitReal = _VisitRealToSupport;
        visitor.VisitSpecies = _VisitSpeciesToSupport;
        visitor.VisitCompartment = _VisitCompartmentToSupport;
        visitor.VisitSymbol = _VisitSymbolToSupport;
        visitor.VisitFunctionSymbol = _VisitFunctionSymbolToSupport;
    }
    
    visitor._internal1 = (CADDR_T)support;
    visitor._internal2 = (CADDR_T)result;
    
    if( IS_FAILED( kineticLaw->Accept( kineticLaw, &visitor ) ) ) {
        END_FUNCTION("_Support", FAILING );
        return NULL;
    } 
        
    END_FUNCTION("_Support", SUCCESS );        
    return result;
}     


static RET_VAL _VisitPWToSupport( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    BYTE opType = 0x00;
    KINETIC_LAW *child = NULL;
    LINKED_LIST *children = NULL;
    UINT num = 0;
    UINT i = 0;
    
    START_FUNCTION("_VisitPWToSupport");
    
    children = GetPWChildrenFromKineticLaw( kineticLaw );
    num = GetLinkedListSize( children );
    opType = GetPWTypeFromKineticLaw( kineticLaw );
    switch( opType ) {
    case KINETIC_LAW_OP_PW:
      for ( i = 1; i < num; i+=2 ) {
	child = (KINETIC_LAW*)GetElementByIndex( i,children );
	if( IS_FAILED( ( ret = child->Accept( child, visitor ) ) ) ) {
	  END_FUNCTION("_VisitPWToSupport", ret );
	  return ret;
	}
	child = (KINETIC_LAW*)GetElementByIndex( i-1,children );
	if( IS_FAILED( ( ret = child->Accept( child, visitor ) ) ) ) {
	  END_FUNCTION("_VisitPWToSupport", ret );
	  return ret;
	}
      }
      if (i==num) {
	child = (KINETIC_LAW*)GetElementByIndex( i-1,children );
	if( IS_FAILED( ( ret = child->Accept( child, visitor ) ) ) ) {
	  END_FUNCTION("_VisitPWToSupport", ret );
	  return ret;
	}
      }
      break;
    case KINETIC_LAW_OP_XOR:
      for ( i = 0; i < num; i++ ) {
	child = (KINETIC_LAW*)GetElementByIndex( i,children );
	if( IS_FAILED( ( ret = child->Accept( child, visitor ) ) ) ) {
	  END_FUNCTION("_VisitPWToEvaluate", ret );
	  return ret;
	}
      }
      break;
    case KINETIC_LAW_OP_OR:
      for ( i = 0; i < num; i++ ) {
	child = (KINETIC_LAW*)GetElementByIndex( i,children );
	if( IS_FAILED( ( ret = child->Accept( child, visitor ) ) ) ) {
	  END_FUNCTION("_VisitPWToEvaluate", ret );
	  return ret;
	}
      }
      break;
    case KINETIC_LAW_OP_AND:
      for ( i = 0; i < num; i++ ) {
	child = (KINETIC_LAW*)GetElementByIndex( i,children );
	if( IS_FAILED( ( ret = child->Accept( child, visitor ) ) ) ) {
	  END_FUNCTION("_VisitPWToEvaluate", ret );
	  return ret;
	}
      }
      break;
    default:
      END_FUNCTION("_VisitUnaryOpToSupport", E_WRONGDATA );
      return E_WRONGDATA;
    }
    END_FUNCTION("_VisitOpToSupport", SUCCESS );
    return ret;
}

static RET_VAL _VisitOpToSupport( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    BYTE opType = 0x00;
    KINETIC_LAW *left = NULL;
    KINETIC_LAW *right = NULL;
    
    START_FUNCTION("_VisitOpToSupport");
    
    left = GetOpLeftFromKineticLaw( kineticLaw );
    if( IS_FAILED( ( ret = left->Accept( left, visitor ) ) ) ) {
        END_FUNCTION("_VisitOpToSupport", ret );
        return ret;
    }
    
    right = GetOpRightFromKineticLaw( kineticLaw );       
    if( IS_FAILED( ( ret = right->Accept( right, visitor ) ) ) ) {
        END_FUNCTION("_VisitOpToSupport", ret );
        return ret;
    }            

    END_FUNCTION("_VisitOpToSupport", SUCCESS );
    return ret;
}

static RET_VAL _VisitUnaryOpToSupport( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    BYTE opType = 0x00;
    KINETIC_LAW *child = NULL;
    UINT i = 0;

    START_FUNCTION("_VisitUnaryOpToSupport");
    
    child = GetUnaryOpChildFromKineticLaw( kineticLaw );
    if( IS_FAILED( ( ret = child->Accept( child, visitor ) ) ) ) {
        END_FUNCTION("_VisitUnaryOpToSupport", ret );
        return ret;
    }

    END_FUNCTION("_VisitUnaryOpToSupport", SUCCESS );
    return ret;
}

static RET_VAL _VisitIntToSupport( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    START_FUNCTION("_VisitIntToSupport");
    
    END_FUNCTION("_VisitIntToSupport", SUCCESS );
    return SUCCESS;
}

static RET_VAL _VisitRealToSupport( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    START_FUNCTION("_VisitRealToSupport");
    
    END_FUNCTION("_VisitRealToSupport", SUCCESS );
    return SUCCESS;
}

static RET_VAL _VisitSymbolToSupport( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    LINKED_LIST *result = NULL;
    REB2SAC_SYMBOL *sym = NULL;
    STRING *symStr = NULL;

    START_FUNCTION("_VisitSymbolToSupport");
    
    sym = GetSymbolFromKineticLaw( kineticLaw );
    
    result = (LINKED_LIST*)(visitor->_internal2);
    symStr = GetSymbolID( sym );
    AddElementInLinkedList( (CADDR_T) symStr, result );

    END_FUNCTION("_VisitSymbolToSupport", SUCCESS );
    return SUCCESS;
}

static RET_VAL _VisitFunctionSymbolToSupport( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    
    START_FUNCTION("_VisitFunctionSymbolToSupport");
    
    END_FUNCTION("_VisitFunctionSymbolToSupport", SUCCESS );
    return SUCCESS;
}

static RET_VAL _VisitSpeciesToSupport( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    LINKED_LIST *result = NULL;
    SPECIES *species = NULL;    
    STRING *speciesStr = NULL;

    START_FUNCTION("_VisitSpeciesToSupport");
    
    result = (LINKED_LIST*)(visitor->_internal2);
    species = GetSpeciesFromKineticLaw( kineticLaw );
    speciesStr = GetSpeciesNodeID( species );
    AddElementInLinkedList( (CADDR_T) speciesStr, result );

    END_FUNCTION("_VisitSpeceisToSupport", SUCCESS );
    return SUCCESS;
}

static RET_VAL _VisitCompartmentToSupport( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    LINKED_LIST *result = NULL;
    COMPARTMENT *compartment = NULL;    
    STRING *compartmentStr = NULL;

    START_FUNCTION("_VisitCompartmentToSupport");
    
    result = (LINKED_LIST*)(visitor->_internal2);
    compartment = GetCompartmentFromKineticLaw( kineticLaw );
    compartmentStr = GetCompartmentID( compartment );
    AddElementInLinkedList( (CADDR_T) compartmentStr, result );

    END_FUNCTION("_VisitCompartmentToSupport", SUCCESS );
    return SUCCESS;
}



