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
#if !defined(HAVE_KINETIC_LAW_EVALUATER)
#define HAVE_KINETIC_LAW_EVALUATER

#include "common.h"
#include "hash_table.h"
#include "species_node.h"
#include "kinetic_law.h"

BEGIN_C_NAMESPACE

typedef struct {
    SPECIES *species;
    double value;
} KINETIC_LAW_EVALUATION_ELEMENT;


struct _KINETIC_LAW_EVALUATER;
typedef struct _KINETIC_LAW_EVALUATER KINETIC_LAW_EVALUATER;

struct _KINETIC_LAW_EVALUATER {
    HASH_TABLE *table;
    double defaultValue;
    RET_VAL (*SetSpeciesValue)( KINETIC_LAW_EVALUATER*evaluater, SPECIES *species, double value );
    RET_VAL (*RemoveSpeciesValue)( KINETIC_LAW_EVALUATER *evaluater, SPECIES *species );
    RET_VAL (*SetDefaultSpeciesValue)( KINETIC_LAW_EVALUATER *evaluater, double value ); 
    double (*Evaluate)( KINETIC_LAW_EVALUATER *evaluater, KINETIC_LAW *kineticLaw );       
    double (*EvaluateWithCurrentAmounts)( KINETIC_LAW_EVALUATER *evaluater, KINETIC_LAW *kineticLaw );       
    double (*EvaluateWithCurrentConcentrations)( KINETIC_LAW_EVALUATER *evaluater, KINETIC_LAW *kineticLaw );       
};

KINETIC_LAW_EVALUATER *CreateKineticLawEvaluater();
RET_VAL FreeKineticLawEvaluater( KINETIC_LAW_EVALUATER **evaluater );

END_C_NAMESPACE

#endif


