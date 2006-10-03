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
#if !defined(HAVE_LAW_OF_MASS_ACTION_UTIL)
#define HAVE_LAW_OF_MASS_ACTION_UTIL

#include "common.h"
#include "reaction_node.h"
#include "species_node.h"
#include "kinetic_law.h"
#include "symtab.h"

BEGIN_C_NAMESPACE
 
BOOL FindRateConstantRatio( KINETIC_LAW *kineticLaw, double *result );
KINETIC_LAW *CreateRateConstantRatioKineticLaw( KINETIC_LAW *kineticLaw );
KINETIC_LAW *CreateDissociationConstantRatioKineticLaw( KINETIC_LAW *kineticLaw );
BOOL FindRateConstant( KINETIC_LAW *kineticLaw, double *result );
KINETIC_LAW *CreateRateConstantKineticLaw( KINETIC_LAW *kineticLaw );

#if 0
BOOL FindCriticalConcentrationLevel( REACTION *reaction, SPECIES *species, double *result );
#endif

KINETIC_LAW *CreateMassActionRatioKineticLaw( REACTION *reaction, SPECIES *op );
KINETIC_LAW *CreateMassActionRatioKineticLawWithRateConstantInKineticLaw( REACTION *reaction, SPECIES *op );

KINETIC_LAW *CreateTotalConcentrationKineticLaw( SPECIES *enzyme, REB2SAC_SYMTAB *symtab, double totalCon );
KINETIC_LAW *CreateConcentrationKineticLaw( SPECIES *species, REB2SAC_SYMTAB *symtab, double concentration );

RET_VAL AddListOfCriticalConcentrationLevels( REACTION *reaction, SPECIES *species, LINKED_LIST *list );

END_C_NAMESPACE

#endif
