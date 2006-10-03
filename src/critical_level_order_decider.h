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
#if !defined(HAVE_CRITICAL_LEVEL_ORDER_DECIDER)
#define HAVE_CRITICAL_LEVEL_ORDER_DECIDER

#include "common.h"
#include "compiler_def.h"
#include "IR.h"
#include "species_critical_level.h"

BEGIN_C_NAMESPACE

struct _CRITICAL_LEVEL_ORDER_DECIDER;
typedef struct _CRITICAL_LEVEL_ORDER_DECIDER CRITICAL_LEVEL_ORDER_DECIDER;

struct _CRITICAL_LEVEL_ORDER_DECIDER {
    CADDR_T _internal1;
    CADDR_T _internal2;
    CADDR_T _internal3;
    REB2SAC_PROPERTIES *properties;
    
    INTERMEDIATE_SPECIES_CRITICAL_LEVEL *(*DecideLevels)( CRITICAL_LEVEL_ORDER_DECIDER *decider, 
        SPECIES *species, INTERMEDIATE_SPECIES_CRITICAL_LEVEL *intermediateLevels );        
    RET_VAL (*ReleaseResource)( CRITICAL_LEVEL_ORDER_DECIDER *decider );        
};

CRITICAL_LEVEL_ORDER_DECIDER *CreateCriticalLevelOrderDecider( REB2SAC_PROPERTIES *properties );
RET_VAL FreeCriticalLevelOrderDecider( CRITICAL_LEVEL_ORDER_DECIDER **pOrderDecider );

END_C_NAMESPACE

#endif
