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

#if !defined(HAVE_SPECIES_NODE)
#define HAVE_SPECIES_NODE

#include "common.h"

#include "ir_node.h"
#include "unit_manager.h"
#include "compartment_manager.h"

BEGIN_C_NAMESPACE

struct _SPECIES;
typedef struct _SPECIES SPECIES;

struct KINETIC_LAW;

struct  _SPECIES {
    STRING *id;
    STRING *name;
    GRAPH_FLAG graphFlag;
    CADDR_T temp;
    LINKED_LIST *reactionsAsReactant;
    LINKED_LIST *reactionsAsModifier;
    LINKED_LIST *reactionsAsProduct;
        
    char * (*GetType)( );                                                                              
    IR_NODE *(*Clone)( IR_NODE *node );    
    RET_VAL (*ReleaseResource)( IR_NODE *node );
    
    COMPARTMENT *compartment;
    union {
        double amount;
        double concentration;
    } initialQuantity;
    union {
        double amount;
        double concentration;
    } quantity;
    UNIT_DEFINITION *substanceUnits;
    UNIT_DEFINITION *spatialSizeUnits;
    int charge;
    BYTE flags;    
    STRING *type;
    struct KINETIC_LAW *initialAssignment;
};

RET_VAL InitSpeciesNode( SPECIES *species, char *name );

STRING *GetSpeciesNodeID( SPECIES *species );

STRING *GetSpeciesNodeName( SPECIES *species );
RET_VAL SetSpeciesNodeName( SPECIES *species, STRING *name );

LINKED_LIST *GetReactionsAsReactantInSpeciesNode( SPECIES *species );
LINKED_LIST *GetReactionsAsModifierInSpeciesNode( SPECIES *species );
LINKED_LIST *GetReactionsAsProductInSpeciesNode( SPECIES *species );

STRING *GetTypeInSpeciesNode( SPECIES *species );
COMPARTMENT *GetCompartmentInSpeciesNode( SPECIES *species );
UNIT_DEFINITION *GetSubstanceUnitsInSpeciesNode( SPECIES *species );
UNIT_DEFINITION *GetSpatialSizeUnitsInSpeciesNode( SPECIES *species );
int GetChargeInSpeciesNode( SPECIES *species );
double GetInitialAmountInSpeciesNode( SPECIES *species );
double GetInitialConcentrationInSpeciesNode( SPECIES *species );
double GetAmountInSpeciesNode( SPECIES *species );
double GetConcentrationInSpeciesNode( SPECIES *species );
struct KINETIC_LAW *GetInitialAssignmentInSpeciesNode( SPECIES *species );

BOOL IsInitialQuantityInAmountInSpeciesNode( SPECIES *species );
BOOL IsSpeciesNodeConstant( SPECIES *species );
BOOL HasOnlySubstanceUnitsInSpeciesNode( SPECIES *species );
BOOL HasBoundaryConditionInSpeciesNode( SPECIES *species );
BOOL IsChargeSetInSpeciesNode( SPECIES *species );

RET_VAL SetTypeInSpeciesNode( SPECIES *species, char *type );
RET_VAL SetCompartmentInSpeciesNode( SPECIES *species, COMPARTMENT *compartment );
RET_VAL SetSubstanceUnitsInSpeciesNode( SPECIES *species, UNIT_DEFINITION *substanceUnits );
RET_VAL SetSpatialSizeUnitsInSpeciesNode( SPECIES *species, UNIT_DEFINITION *spatialSizeUnits );
RET_VAL SetChargeInSpeciesNode( SPECIES *species, int charge );
RET_VAL SetInitialAmountInSpeciesNode( SPECIES *species, double initialAmount );
RET_VAL SetInitialConcentrationInSpeciesNode( SPECIES *species, double initialConcentration);
RET_VAL SetAmountInSpeciesNode( SPECIES *species, double amount );
RET_VAL SetConcentrationInSpeciesNode( SPECIES *species, double concentration);
RET_VAL SetSpeciesNodeConstant( SPECIES *species, BOOL flag );
RET_VAL SetOnlySubstanceUnitsInSpeciesNode( SPECIES *species, BOOL flag );
RET_VAL SetBoundaryConditionInSpeciesNode( SPECIES *species, BOOL flag );
RET_VAL SetInitialAssignmentInSpeciesNode( SPECIES *species, struct KINETIC_LAW *law );

BOOL IsKeepFlagSetInSpeciesNode( SPECIES *species );
RET_VAL SetKeepFlagInSpeciesNode( SPECIES *species, BOOL flag );

BOOL IsPrintFlagSetInSpeciesNode( SPECIES *species );
RET_VAL SetPrintFlagInSpeciesNode( SPECIES *species, BOOL flag );

RET_VAL ReleaseResourcesInSpeciesNode( SPECIES *species );

END_C_NAMESPACE

#endif

