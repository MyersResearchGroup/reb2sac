/***************************************************************************
 *   Copyright (C) 2004 by Hiroyuki Kuwahara                               *
 *   kuwahara@cs.utah.edu                                                 *
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
#if !defined(HAVE_SPECIES_CRITICAL_LEVEL)
#define HAVE_SPECIES_CRITICAL_LEVEL

#include "common.h"
#include "species_node.h"
#include "vector.h"

BEGIN_C_NAMESPACE

#define DEFAULT_LOWEST_CRITICAL_LEVEL 0.0

typedef struct {
    double *levels;
    int size;
} INTERMEDIATE_SPECIES_CRITICAL_LEVEL;

RET_VAL FreeIntermediateSpeciesCriticalLevel( INTERMEDIATE_SPECIES_CRITICAL_LEVEL **pCriticalLevel );


typedef struct {
    SPECIES *species;
    double *levels;
    int size;
    int initialLevelIndex;
} SPECIES_CRITICAL_LEVEL;

RET_VAL FreeSpeciesCriticalLevel( SPECIES_CRITICAL_LEVEL **pCriticalLevel );

END_C_NAMESPACE

#endif
