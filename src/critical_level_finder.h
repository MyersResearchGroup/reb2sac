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
#if !defined(HAVE_CRITICAL_LEVEL_FINDER)
#define HAVE_CRITICAL_LEVEL_FINDER

#include "common.h"
#include "compiler_def.h"
#include "species_node.h"
#include "species_critical_level.h"

BEGIN_C_NAMESPACE


struct _CRITICAL_LEVEL_FINDER;
typedef struct _CRITICAL_LEVEL_FINDER CRITICAL_LEVEL_FINDER;

struct _CRITICAL_LEVEL_FINDER {
    CADDR_T _internal1;
    CADDR_T _internal2;
    CADDR_T _internal3;
    REB2SAC_PROPERTIES *properties;
    
    INTERMEDIATE_SPECIES_CRITICAL_LEVEL *(*FindLevels)( CRITICAL_LEVEL_FINDER *finder, SPECIES *species );        
    RET_VAL (*ReleaseResource)( CRITICAL_LEVEL_FINDER *finder );        
};

CRITICAL_LEVEL_FINDER *CreateCriticalLevelFinder( REB2SAC_PROPERTIES *properties );
RET_VAL FreeCriticalLevelFinder( CRITICAL_LEVEL_FINDER **pFinder );

END_C_NAMESPACE

#endif
