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

#if !defined(HAVE_LOGICAL_SPECIES_NODE)
#define HAVE_LOGICAL_SPECIES_NODE

#include "common.h"
#include "util.h"
#include "species_node.h"

BEGIN_C_NAMESPACE

#define LOGICAL_SPECIES_TYPE_ID "logical-species"

struct _LOGICAL_SPECIES;
typedef struct _LOGICAL_SPECIES LOGICAL_SPECIES;

struct  _LOGICAL_SPECIES {
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
        
    int order;
    int highestLevel;
    double criticalConcentration;
    STRING *originalSpeciesName;    
};

LOGICAL_SPECIES *CreateLogicalSpeciesFromSpecies( SPECIES *species, STRING *newName, int order, int highestLevel, double criticalConcentration );
int GetOrderInLogicalSpecies( LOGICAL_SPECIES *species );
int GetHighestLevelInLogicalSpecies( LOGICAL_SPECIES *species );
double GetCriticalConcentrationInLogicalSpecies( LOGICAL_SPECIES *species );
STRING *GetOriginalSpeciesName(LOGICAL_SPECIES *species);
BOOL IsLogicalSpecies( SPECIES *species );
END_C_NAMESPACE

#endif

