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
#if !defined(HAVE_COMPARTMENT_MANAGER)
#define HAVE_COMPARTMENT_MANAGER

#include "common.h"
#include "linked_list.h"
#include "hash_table.h"
#include "util.h"

#include "unit_manager.h"

BEGIN_C_NAMESPACE

struct _COMPARTMENT;
typedef struct _COMPARTMENT  COMPARTMENT;

struct KINETIC_LAW;

struct _COMPARTMENT {
    STRING *id;
    int spatialDimensions;
    double size;
    double currentSize;
    UNIT_DEFINITION *unit;
    STRING *outside;
    STRING *type;
    COMPARTMENT *outsideCompartment;
    BOOL constant; 
    BOOL print;
    BOOL algebraic;
    struct KINETIC_LAW *initialAssignment;
};


struct _COMPARTMENT_MANAGER;
typedef struct _COMPARTMENT_MANAGER  COMPARTMENT_MANAGER;

struct _COMPARTMENT_MANAGER {
    HASH_TABLE *table;
    COMPILER_RECORD_T *record;

    COMPARTMENT * (*CreateCompartment)( COMPARTMENT_MANAGER *manager, char *id );
    COMPARTMENT * (*LookupCompartment)( COMPARTMENT_MANAGER *manager,  char *id );
    RET_VAL (*ResolveCompartmentLinks)( COMPARTMENT_MANAGER *manager );
    LINKED_LIST *(*CreateListOfCompartments)( COMPARTMENT_MANAGER *manager );
};


COMPARTMENT_MANAGER *GetCompartmentManagerInstance( COMPILER_RECORD_T *record );
RET_VAL CloseCompartmentManager( );

STRING *GetCompartmentID( COMPARTMENT *compartment );

int GetSpatialDimensionsInCompartment( COMPARTMENT *compartment );
RET_VAL SetSpatialDimensionsInCompartment( COMPARTMENT *compartment, int spatialDimensions );

double GetSizeInCompartment( COMPARTMENT *compartment );
RET_VAL SetSizeInCompartment( COMPARTMENT *compartment, double size );

double GetCurrentSizeInCompartment( COMPARTMENT *compartment );
RET_VAL SetCurrentSizeInCompartment( COMPARTMENT *compartment, double size );

UNIT_DEFINITION *GetUnitInCompartment( COMPARTMENT *compartment );
RET_VAL SetUnitInCompartment( COMPARTMENT *compartment, UNIT_DEFINITION *unit );

STRING *GetOutsideInCompartment( COMPARTMENT *compartment );
RET_VAL SetOutsideInCompartment( COMPARTMENT *compartment, char *id );

STRING *GetTypeInCompartment( COMPARTMENT *compartment );
RET_VAL SetTypeInCompartment( COMPARTMENT *compartment, char *type );

struct KINETIC_LAW *GetInitialAssignmentInCompartment( COMPARTMENT *compartment );
RET_VAL SetInitialAssignmentInCompartment( COMPARTMENT *compartment, struct KINETIC_LAW *law );

COMPARTMENT *GetOutsideCompartmentInCompartment( COMPARTMENT *compartment );

BOOL IsCompartmentConstant( COMPARTMENT *compartment );
RET_VAL SetCompartmentConstant( COMPARTMENT *compartment, BOOL constant );

BOOL IsCompartmentAlgebraic( COMPARTMENT *compartment );
RET_VAL SetCompartmentAlgebraic( COMPARTMENT *compartment, BOOL algebraic );

BOOL PrintCompartment( COMPARTMENT *compartment );
RET_VAL SetPrintCompartment( COMPARTMENT *compartment, BOOL print );



END_C_NAMESPACE

#endif
