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
#include "species_node.h"


#define SPECIES_NODE_FLAG_NONE 0x00
#define SPECIES_NODE_FLAG_IS_AMOUNT 0x01
#define SPECIES_NODE_FLAG_IS_CONSTANT 0x02
#define SPECIES_NODE_FLAG_HAS_ONLY_SUBSTANCE_UNITS 0x04
#define SPECIES_NODE_FLAG_HAS_BOUNDARY_CONDITION 0x08
#define SPECIES_NODE_FLAG_IS_FAST 0x10
#define SPECIES_NODE_FLAG_KEEP 0x20
#define SPECIES_NODE_FLAG_PRINT 0x40
#define SPECIES_NODE_FLAG_IS_ALGEBRAIC 0x80

static IR_NODE *_Clone( IR_NODE *species );    
static char * _GetType(  );                                                                              
static RET_VAL _ReleaseResource( IR_NODE *species );

RET_VAL InitSpeciesNode( SPECIES *species, char *name ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("InitSpeciesNode");
    
    if( IS_FAILED( InitIRNode( (IR_NODE*)species, name ) ) ) {
        END_FUNCTION("InitSpeciesNode", FAILING );
        return FAILING;
    }
    species->initialAssignment = NULL;
    species->conversionFactor = NULL;
    species->flags = SPECIES_NODE_FLAG_NONE;
    species->Clone = _Clone;
    species->GetType = _GetType;
    species->ReleaseResource = _ReleaseResource;

    END_FUNCTION("InitSpeciesNode", SUCCESS );
    return ret;
}


STRING *GetSpeciesNodeID( SPECIES *species ) {
    START_FUNCTION("GetSpeciesNodeID");
    if( species == NULL ) {
        END_FUNCTION("GetSpeciesNodeID", FAILING );
        return NULL;
    }
    END_FUNCTION("GetSpeciesNodeID", SUCCESS );
#ifdef NAME_FOR_ID
    return species->name;
#else 
    return species->id;
#endif
}

STRING *GetSpeciesNodeName( SPECIES *species ) {        
    START_FUNCTION("GetSpeciesNodeName");
    if( species == NULL ) {
        END_FUNCTION("GetSpeciesNodeName", FAILING );
        return NULL;
    }
    END_FUNCTION("GetSpeciesNodeName", SUCCESS );
    return species->name;
}

RET_VAL SetSpeciesNodeName( SPECIES *species, STRING *name ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("SetSpeciesNodeName");
    if( species == NULL ) {
        return ErrorReport( FAILING, "SetSpeciesNodeName", "input species node is NULL" );
    }
    FreeString( &(species->name) );
    species->name = name;
    return ret;
}


LINKED_LIST *GetReactionsAsReactantInSpeciesNode( SPECIES *species ) {
    START_FUNCTION("GetReactionsAsReactantInSpeciesNode");
    if( species == NULL ) {
        END_FUNCTION("GetReactionsAsReactantInSpeciesNode", FAILING );
        return NULL;
    }
    END_FUNCTION("GetReactionsAsReactantInSpeciesNode", SUCCESS );
    return species->reactionsAsReactant;
}

LINKED_LIST *GetReactionsAsModifierInSpeciesNode( SPECIES *species ) {
    START_FUNCTION("GetReactionsAsModifierInSpeciesNode");
    if( species == NULL ) {
        END_FUNCTION("GetReactionsAsModifierInSpeciesNode", FAILING );
        return NULL;
    }
    END_FUNCTION("GetReactionsAsModifierInSpeciesNode", SUCCESS );
    return species->reactionsAsModifier;
}

LINKED_LIST *GetReactionsAsProductInSpeciesNode( SPECIES *species ) {
    START_FUNCTION("GetReactionsAsProductInSpeciesNode");
    if( species == NULL ) {
        END_FUNCTION("GetReactionsAsProductInSpeciesNode", FAILING );
        return NULL;
    }
    END_FUNCTION("GetReactionsAsProductInSpeciesNode", SUCCESS );
    return species->reactionsAsProduct;
}



STRING *GetTypeInSpeciesNode( SPECIES *species ) {
    START_FUNCTION("GetTypeInSpeciesNode");
    if( species == NULL ) {
        END_FUNCTION("GetTypeInSpeciesNode", FAILING );
        return NULL;
    }
    END_FUNCTION("GetTypeInSpeciesNode", SUCCESS );
    return species->type;
}

REB2SAC_SYMBOL *GetConversionFactorInSpeciesNode( SPECIES *species ) {
    START_FUNCTION("GetConversionFactorInSpeciesNode");
    if( species == NULL ) {
        END_FUNCTION("GetConversionFactorInSpeciesNode", FAILING );
        return NULL;
    }
    END_FUNCTION("GetConversionFactorInSpeciesNode", SUCCESS );
    return species->conversionFactor;
}

COMPARTMENT *GetCompartmentInSpeciesNode( SPECIES *species ) {
    START_FUNCTION("GetCompartmentInSpeciesNode");
    if( species == NULL ) {
        END_FUNCTION("GetCompartmentInSpeciesNode", FAILING );
        return NULL;
    }
    END_FUNCTION("GetCompartmentInSpeciesNode", SUCCESS );
    return species->compartment;
}

UNIT_DEFINITION *GetSubstanceUnitsInSpeciesNode( SPECIES *species ) {
    START_FUNCTION("GetSubstanceUnitsInSpeciesNode");
    if( species == NULL ) {
        END_FUNCTION("GetSubstanceUnitsInSpeciesNode", FAILING );
        return NULL;
    }
    END_FUNCTION("GetSubstanceUnitsInSpeciesNode", SUCCESS );
    return species->substanceUnits;
}

UNIT_DEFINITION *GetSpatialSizeUnitsInSpeciesNode( SPECIES *species ) {
    START_FUNCTION("GetSpatialSizeUnitsInSpeciesNode");
    if( species == NULL ) {
        END_FUNCTION("GetSpatialSizeUnitsInSpeciesNode", FAILING );
        return NULL;
    }
    END_FUNCTION("GetSpatialSizeUnitsInSpeciesNode", SUCCESS );
    return species->spatialSizeUnits;
}

struct KINETIC_LAW *GetInitialAssignmentInSpeciesNode( SPECIES *species ) {
    START_FUNCTION("GetInitialAssignmentInSpeciesNode");
    if( species == NULL ) {
        END_FUNCTION("GetInitialAssignmentInSpeciesNode", FAILING );
        return NULL;
    }
    END_FUNCTION("GetInitialAssignmentInSpeciesNode", SUCCESS );
    return species->initialAssignment;
}


BOOL IsInitialQuantityInAmountInSpeciesNode( SPECIES *species ) {
    START_FUNCTION("IsInitialQuantityInAmountInSpeciesNode");
    if( species == NULL ) {
        END_FUNCTION("IsInitialQuantityInAmountInSpeciesNode", FAILING );
        return FALSE;
    }
    END_FUNCTION("IsInitialQuantityInAmountInSpeciesNode", SUCCESS );
    return ( ( ( species->flags & SPECIES_NODE_FLAG_IS_AMOUNT ) != SPECIES_NODE_FLAG_NONE ) ? TRUE : FALSE );
}


BOOL IsSpeciesNodeConstant( SPECIES *species ) {
    START_FUNCTION("IsSpeciesNodeConstant");
    if( species == NULL ) {
        END_FUNCTION("IsSpeciesNodeConstant", FAILING );
        return FALSE;
    }
    END_FUNCTION("IsSpeciesNodeConstant", SUCCESS );
    return ( ( ( species->flags & SPECIES_NODE_FLAG_IS_CONSTANT ) != SPECIES_NODE_FLAG_NONE ) ? TRUE : FALSE );
}


BOOL IsSpeciesNodeAlgebraic( SPECIES *species ) {
    START_FUNCTION("IsSpeciesNodeAlgebraic");
    if( species == NULL ) {
        END_FUNCTION("IsSpeciesNodeAlgebraic", FAILING );
        return FALSE;
    }
    END_FUNCTION("IsSpeciesNodeAlgebraic", SUCCESS );
    return ( ( ( species->flags & SPECIES_NODE_FLAG_IS_ALGEBRAIC ) != SPECIES_NODE_FLAG_NONE ) ? TRUE : FALSE );
}


BOOL IsSpeciesNodeFast( SPECIES *species ) {
    START_FUNCTION("IsSpeciesNodeFast");
    if( species == NULL ) {
        END_FUNCTION("IsSpeciesNodeFast", FAILING );
        return FALSE;
    }
    END_FUNCTION("IsSpeciesNodeFast", SUCCESS );
    return ( ( ( species->flags & SPECIES_NODE_FLAG_IS_FAST ) != SPECIES_NODE_FLAG_NONE ) ? TRUE : FALSE );
}


BOOL HasOnlySubstanceUnitsInSpeciesNode( SPECIES *species ) {
    START_FUNCTION("HasOnlySubstanceUnitsInSpeciesNode");
    if( species == NULL ) {
        END_FUNCTION("HasOnlySubstanceUnitsInSpeciesNode", FAILING );
        return FALSE;
    }
    /*
    if (GetSpatialDimensionsInCompartment( GetCompartmentInSpeciesNode( species ) ) == 0) {
      species->flags = ( species->flags | SPECIES_NODE_FLAG_HAS_ONLY_SUBSTANCE_UNITS );
    }
    */
    END_FUNCTION("HasOnlySubstanceUnitsInSpeciesNode", SUCCESS );
    return ( ( ( species->flags & SPECIES_NODE_FLAG_HAS_ONLY_SUBSTANCE_UNITS ) != SPECIES_NODE_FLAG_NONE ) ? TRUE : FALSE );
}


BOOL HasBoundaryConditionInSpeciesNode( SPECIES *species ) {
    START_FUNCTION("HasBoundaryConditionInSpeciesNode");
    if( species == NULL ) {
        END_FUNCTION("HasBoundaryConditionInSpeciesNode", FAILING );
        return FALSE;
    }
    END_FUNCTION("HasBoundaryConditionInSpeciesNode", SUCCESS );
    return ( ( ( species->flags & SPECIES_NODE_FLAG_HAS_BOUNDARY_CONDITION ) != SPECIES_NODE_FLAG_NONE ) ? TRUE : FALSE );
}

double GetInitialAmountInSpeciesNode( SPECIES *species ) {
    START_FUNCTION("GetInitialAmountInSpeciesNode");
    if( species == NULL ) {
        END_FUNCTION("GetInitialAmountInSpeciesNode", FAILING );
        return 0L;
    }
    END_FUNCTION("GetInitialAmountInSpeciesNode", SUCCESS );
    return species->initialQuantity.amount;
}

double GetInitialConcentrationInSpeciesNode( SPECIES *species ) {
    double size = 1.0;

    START_FUNCTION("GetInitialConcentrationInSpeciesNode");
    if( species == NULL ) {
        END_FUNCTION("GetInitialConcentrationInSpeciesNode", FAILING );
        return 0.0;
    }
    END_FUNCTION("GetInitialConcentrationInSpeciesNode", SUCCESS );
    size = GetSizeInCompartment( GetCompartmentInSpeciesNode( species ) );
    if (size == -1.0) {
      return species->initialQuantity.concentration;
    } else {
      return species->initialQuantity.amount / size;
    }
}

double GetAmountInSpeciesNode( SPECIES *species ) {
    START_FUNCTION("GetAmountInSpeciesNode");
    if( species == NULL ) {
        END_FUNCTION("GetAmountInSpeciesNode", FAILING );
        return 0L;
    }
    END_FUNCTION("GetAmountInSpeciesNode", SUCCESS );
    return species->quantity.amount;
}

double GetConcentrationInSpeciesNode( SPECIES *species ) {
    double size = 1.0;

    START_FUNCTION("GetConcentrationInSpeciesNode");
    if( species == NULL ) {
        END_FUNCTION("GetConcentrationInSpeciesNode", FAILING );
        return 0.0;
    }
    END_FUNCTION("GetConcentrationInSpeciesNode", SUCCESS );
    size = GetCurrentSizeInCompartment( GetCompartmentInSpeciesNode( species ) );
    if (size == -1.0) {
      return species->quantity.concentration;
    } else {
      return species->quantity.amount / size;
    }
}


RET_VAL SetTypeInSpeciesNode( SPECIES *species, char *type ) {
    RET_VAL ret = SUCCESS;
    STRING *typeStr;

    START_FUNCTION("SetTypeInSpeciesNode");
    if( species == NULL ) {
        return ErrorReport( FAILING, "SetTypeInSpeciesNode", "input species node is NULL" );
    }
    
    if( ( typeStr = CreateString( type ) ) == NULL ) {
        END_FUNCTION("SetTypeInSpeciesNode", FAILING );
        return FAILING;
    }
    
    if( species->type != NULL ) {
        FreeString( &(species->type) );
    }

    species->type = typeStr;
    END_FUNCTION("SetTypeInSpeciesNode", SUCCESS );
    return ret;
}

RET_VAL SetConversionFactorInSpeciesNode( SPECIES *species, REB2SAC_SYMBOL *conversionFactor ) {
    RET_VAL ret = SUCCESS;

    START_FUNCTION("SetConversionFactorInSpeciesNode");
    if( species == NULL ) {
        return ErrorReport( FAILING, "SetConversionFactorInSpeciesNode", "input species node is NULL" );
    }
    
    if( species->conversionFactor != NULL ) {
        FreeString( &(species->conversionFactor) );
    }

    species->conversionFactor = conversionFactor;
    END_FUNCTION("SetConversionFactorInSpeciesNode", SUCCESS );
    return ret;
}

RET_VAL SetCompartmentInSpeciesNode( SPECIES *species, COMPARTMENT *compartment ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("SetCompartmentInSpeciesNode");
    if( species == NULL ) {
        return ErrorReport( FAILING, "SetCompartmentInSpeciesNode", "input species node is NULL" );
    }
    species->compartment = compartment;
    END_FUNCTION("SetCompartmentInSpeciesNode", SUCCESS );
    return ret;
}

RET_VAL SetSubstanceUnitsInSpeciesNode( SPECIES *species, UNIT_DEFINITION *substanceUnits ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("SetSubstanceUnitsInSpeciesNode");
    if( species == NULL ) {
        return ErrorReport( FAILING, "SetSubstanceUnitsInSpeciesNode", "input species node is NULL" );
    }
    species->substanceUnits = substanceUnits;
    END_FUNCTION("SetCompartmentInSpeciesNode", SUCCESS );
    return ret;
}


RET_VAL SetSpatialSizeUnitsInSpeciesNode( SPECIES *species, UNIT_DEFINITION *spatialSizeUnits ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("SetSpatialSizeUnitsInSpeciesNode");
    if( species == NULL ) {
        return ErrorReport( FAILING, "SetSpatialSizeUnitsInSpeciesNode", "input species node is NULL" );
    }
    species->spatialSizeUnits = spatialSizeUnits;
    END_FUNCTION("SetSpatialSizeUnitsInSpeciesNode", SUCCESS );
    return ret;
}

RET_VAL SetInitialAmountInSpeciesNode( SPECIES *species, double initialAmount ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("SetInitialAmountInSpeciesNode");
    if( species == NULL ) {
        return ErrorReport( FAILING, "SetInitialAmountInSpeciesNode", "input species node is NULL" );
    }

    species->flags = ( species->flags | SPECIES_NODE_FLAG_IS_AMOUNT );
    species->initialQuantity.amount = initialAmount;
    END_FUNCTION("SetInitialAmountInSpeciesNode", SUCCESS );
    return ret;
}

RET_VAL SetInitialConcentrationInSpeciesNode( SPECIES *species, double initialConcentration) {
    RET_VAL ret = SUCCESS;
    double size = 1.0;

    START_FUNCTION("SetInitialConcentrationInSpeciesNode");
    if( species == NULL ) {
        return ErrorReport( FAILING, "SetInitialConcentrationInSpeciesNode", "input species node is NULL" );
    }

    size = GetSizeInCompartment( GetCompartmentInSpeciesNode( species ) );
    species->initialQuantity.amount = initialConcentration * size;
    species->flags = ( species->flags & (~SPECIES_NODE_FLAG_IS_AMOUNT) );
    //species->initialQuantity.concentration = initialConcentration;
    END_FUNCTION("SetInitialConcentrationInSpeciesNode", SUCCESS );
    return ret;
}

RET_VAL SetAmountInSpeciesNode( SPECIES *species, double amount ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("SetAmountInSpeciesNode");
    if( species == NULL ) {
        return ErrorReport( FAILING, "SetAmountInSpeciesNode", "input species node is NULL" );
    }
    species->quantity.amount = amount;
    END_FUNCTION("SetAmountInSpeciesNode", SUCCESS );
    return ret;
}

RET_VAL SetConcentrationInSpeciesNode( SPECIES *species, double concentration) {
    RET_VAL ret = SUCCESS;
    double size = 1.0;
    
    START_FUNCTION("SetConcentrationInSpeciesNode");
    if( species == NULL ) {
        return ErrorReport( FAILING, "SetConcentrationInSpeciesNode", "input species node is NULL" );
    }
    size = GetCurrentSizeInCompartment( GetCompartmentInSpeciesNode( species ) );
    species->quantity.amount = concentration * size;
    //species->quantity.concentration = concentration;
    //printf("Setting amount of %s to %g\n",
    //   GetCharArrayOfString( GetSpeciesNodeName( species ) ), species->quantity.amount );
    END_FUNCTION("SetConcentrationInSpeciesNode", SUCCESS );
    return ret;
}

RET_VAL SetInitialAssignmentInSpeciesNode( SPECIES *species, struct KINETIC_LAW *law) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("SetInitialAssignmentInSpeciesNode");
    if( species == NULL ) {
        return ErrorReport( FAILING, "SetInitialAssignmentInSpeciesNode", "input species node is NULL" );
    }
    species->initialAssignment = law;
    END_FUNCTION("SetInitialAssignmentInSpeciesNode", SUCCESS );
    return ret;
}



RET_VAL SetSpeciesNodeConstant( SPECIES *species, BOOL flag ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("SetSpeciesNodeConstant");
    if( species == NULL ) {
        return ErrorReport( FAILING, "SetSpeciesNodeConstant", "input species node is NULL" );
    }
    if( flag ) {
        species->flags = ( species->flags | SPECIES_NODE_FLAG_IS_CONSTANT );
        species->flags = ( species->flags & (~SPECIES_NODE_FLAG_IS_ALGEBRAIC) );
    }
    else {
        species->flags = ( species->flags & (~SPECIES_NODE_FLAG_IS_CONSTANT) );
    }
    
    END_FUNCTION("SetSpeciesNodeConstant", SUCCESS );
    return ret;
}

RET_VAL SetAlgebraicInSpeciesNode( SPECIES *species, BOOL flag ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("SetSpeciesNodeAlgebraic");
    if( species == NULL ) {
        return ErrorReport( FAILING, "SetSpeciesNodeAlgebraic", "input species node is NULL" );
    }
    if( flag ) {
        species->flags = ( species->flags | SPECIES_NODE_FLAG_IS_ALGEBRAIC );
    }
    else {
        species->flags = ( species->flags & (~SPECIES_NODE_FLAG_IS_ALGEBRAIC) );
    }
    
    END_FUNCTION("SetSpeciesNodeAlgebraic", SUCCESS );
    return ret;
}

RET_VAL SetFastInSpeciesNode( SPECIES *species, BOOL flag ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("SetSpeciesNodeFast");
    if( species == NULL ) {
        return ErrorReport( FAILING, "SetSpeciesNodeFast", "input species node is NULL" );
    }
    if( flag ) {
        species->flags = ( species->flags | SPECIES_NODE_FLAG_IS_FAST );
    }
    else {
        species->flags = ( species->flags & (~SPECIES_NODE_FLAG_IS_FAST) );
    }
    
    END_FUNCTION("SetSpeciesNodeFast", SUCCESS );
    return ret;
}

RET_VAL SetOnlySubstanceUnitsInSpeciesNode( SPECIES *species, BOOL flag ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("SetOnlySubstanceUnitsInSpeciesNode");
    if( species == NULL ) {
        return ErrorReport( FAILING, "SetOnlySubstanceUnitsInSpeciesNode", "input species node is NULL" );
    }
    if( flag ) {
        species->flags = ( species->flags | SPECIES_NODE_FLAG_HAS_ONLY_SUBSTANCE_UNITS );
    }
    else {
        species->flags = ( species->flags & (~SPECIES_NODE_FLAG_HAS_ONLY_SUBSTANCE_UNITS) );
    }
    
    END_FUNCTION("SetOnlySubstanceUnitsInSpeciesNode", SUCCESS );
    return ret;
}

RET_VAL SetBoundaryConditionInSpeciesNode( SPECIES *species, BOOL flag ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("SetBoundaryConditionInSpeciesNode");
    if( species == NULL ) {
        return ErrorReport( FAILING, "SetBoundaryConditionInSpeciesNode", "input species node is NULL" );
    }
    if( flag ) {
        species->flags = ( species->flags | SPECIES_NODE_FLAG_HAS_BOUNDARY_CONDITION );
    }
    else {
        species->flags = ( species->flags & (~SPECIES_NODE_FLAG_HAS_BOUNDARY_CONDITION) );
    }
    
    END_FUNCTION("SetBoundaryConditionInSpeciesNode", SUCCESS );
    return ret;
}

BOOL IsKeepFlagSetInSpeciesNode( SPECIES *species ) {
    START_FUNCTION("IsKeepFlagSetInSpeciesNode");
    if( species == NULL ) {
        END_FUNCTION("IsKeepFlagSetInSpeciesNode", FAILING );
        return FALSE;
    }
    END_FUNCTION("IsKeepFlagSetInSpeciesNode", SUCCESS );
    return ( ( ( species->flags & SPECIES_NODE_FLAG_KEEP ) != SPECIES_NODE_FLAG_NONE ) ? TRUE : FALSE );
}


RET_VAL SetKeepFlagInSpeciesNode( SPECIES *species, BOOL flag ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("SetKeepFlagInSpeciesNode");
    if( species == NULL ) {
        return ErrorReport( FAILING, "SetKeepFlagInSpeciesNode", "input species node is NULL" );
    }
    if( flag ) {
        species->flags = ( species->flags | SPECIES_NODE_FLAG_KEEP );
    }
    else {
        species->flags = ( species->flags & (~SPECIES_NODE_FLAG_KEEP) );
    }
    
    END_FUNCTION("SetKeepFlagInSpeciesNode", SUCCESS );
    return ret;
}

RET_VAL SetPrintFlagInSpeciesNode( SPECIES *species, BOOL flag ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("SetPrintFlagInSpeciesNode");
    if( species == NULL ) {
        return ErrorReport( FAILING, "SetPrintFlagInSpeciesNode", "input species node is NULL" );
    }
    if( flag ) {
        species->flags = ( species->flags | SPECIES_NODE_FLAG_PRINT );
    }
    else {
        species->flags = ( species->flags & (~SPECIES_NODE_FLAG_PRINT) );
    }
    
    END_FUNCTION("SetPrintFlagInSpeciesNode", SUCCESS );
    return ret;
}

BOOL IsPrintFlagSetInSpeciesNode( SPECIES *species ) {
    START_FUNCTION("IsPrintSetInSpeciesNode");
    if( species == NULL ) {
        END_FUNCTION("IsPrintSetInSpeciesNode", FAILING );
        return FALSE;
    }
    END_FUNCTION("IsPrintSetInSpeciesNode", SUCCESS );
    return ( ( ( species->flags & SPECIES_NODE_FLAG_PRINT ) != SPECIES_NODE_FLAG_NONE ) ? TRUE : FALSE );
}

RET_VAL ReleaseResourcesInSpeciesNode( SPECIES *species ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("ReleaseResourcesInSpeciesNode");
    if( species == NULL ) {
        return ErrorReport( FAILING, "ReleaseResourcesInSpeciesNode", "input species node is NULL" );
    }
    if( IS_FAILED( ( ret = ReleaseIRNodeResources( (IR_NODE*)species ) ) ) ) {
        END_FUNCTION("ReleaseResourcesInSpeciesNode", ret );
        return ret;
    }
        
    END_FUNCTION("ReleaseResourcesInSpeciesNode", SUCCESS );
    return ret;
}



static IR_NODE *_Clone( IR_NODE *species ) {
    SPECIES *clone = NULL;
    
    START_FUNCTION("_Clone");

    if( ( clone = (SPECIES*)MALLOC( sizeof( SPECIES ) ) ) == NULL ) {
        END_FUNCTION("_Clone", FAILING );
        return NULL;
    }
    
    if( IS_FAILED( CopyIRNode( (IR_NODE*)species, (IR_NODE*)clone ) ) ) {
        END_FUNCTION("_Clone", FAILING );
        return NULL;
    }
    
    
    
    memcpy( (CADDR_T)clone + sizeof(IR_NODE), (CADDR_T)species + sizeof(IR_NODE), sizeof(SPECIES) - sizeof(IR_NODE) );   
    
               
    END_FUNCTION("_Clone", SUCCESS );    
    return (IR_NODE*)clone;
}

static char * _GetType( ) {
    START_FUNCTION("_GetType");
    END_FUNCTION("_GetType", SUCCESS );    
    return "species";
}
                                                                          
static RET_VAL _ReleaseResource( IR_NODE *species ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("_ReleaseResource");
    if( species == NULL ) {
        return ErrorReport( FAILING, "_ReleaseResource", "input species node is NULL" );
    }
    if( IS_FAILED( ( ret = ReleaseIRNodeResources( (IR_NODE*)species ) ) ) ) {
        END_FUNCTION("_ReleaseResource", ret );
        return ret;
    }
        
    END_FUNCTION("_ReleaseResource", SUCCESS );
    return ret;
}


