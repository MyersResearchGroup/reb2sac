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
#if !defined(HAVE_SPECIES_CRITICAL_LEVEL_GENERATOR)
#define HAVE_SPECIES_CRITICAL_LEVEL_GENERATOR

#include "common.h"
#include "species_node.h"
#include "vector.h"
#include "species_critical_level.h"
#include "critical_level_finder.h"
#include "critical_level_order_decider.h"

BEGIN_C_NAMESPACE

struct _SPECIES_CRITICAL_LEVEL_GENERATOR;
typedef struct _SPECIES_CRITICAL_LEVEL_GENERATOR SPECIES_CRITICAL_LEVEL_GENERATOR;

struct _SPECIES_CRITICAL_LEVEL_GENERATOR {
    REB2SAC_PROPERTIES *properties;
    CRITICAL_LEVEL_FINDER *finder;
    CRITICAL_LEVEL_ORDER_DECIDER *decider;
    
    SPECIES_CRITICAL_LEVEL *(*Generate)( SPECIES_CRITICAL_LEVEL_GENERATOR *gen, SPECIES *species );
};

SPECIES_CRITICAL_LEVEL_GENERATOR *CreateSpeciesCriticalLevelGenerator( REB2SAC_PROPERTIES *properties );
RET_VAL FreeSpeciesCriticalLevelGenerator( SPECIES_CRITICAL_LEVEL_GENERATOR **pGen );


END_C_NAMESPACE

#endif
