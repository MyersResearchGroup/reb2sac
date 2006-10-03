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
#if !defined(HAVE_NARY_ORDER_TRASFORMATION_METHOD)
#define HAVE_NARY_ORDER_TRASFORMATION_METHOD
#include "common.h"
#include "species_node.h"
#include "kinetic_law.h"

BEGIN_C_NAMESPACE


typedef struct {
    SPECIES *logicalSpecies;
    double concentration;
    KINETIC_LAW *conKineticLaw;
} CRITICAL_CONCENTRATION_ELEMENT;

typedef struct {
    CRITICAL_CONCENTRATION_ELEMENT *elements;
    SPECIES *species;
    int len;
} CRITICAL_CONCENTRATION_INFO;


END_C_NAMESPACE

#endif

 
 
 
