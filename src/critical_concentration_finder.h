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
#if !defined(HAVE_CRITICAL_CONCENTRATION_FINDER)
#define HAVE_CRITICAL_CONCENTRATION_FINDER

#include "common.h"
#include "compiler_def.h"
#include "reaction_node.h"
#include "species_node.h"

BEGIN_C_NAMESPACE

struct _CRITICAL_CONCENTRATION_FINDER;
typedef struct _CRITICAL_CONCENTRATION_FINDER CRITICAL_CONCENTRATION_FINDER;

struct _CRITICAL_CONCENTRATION_FINDER {
    double amplifier;
    RET_VAL (STDCALL *AddListOfCriticalConcentrationLevels)( CRITICAL_CONCENTRATION_FINDER *finder, REACTION *reaction, SPECIES *species, LINKED_LIST *list );    
};

DLLSCOPE RET_VAL STDCALL InitCriticalConcentrationFinder(  CRITICAL_CONCENTRATION_FINDER *finder, COMPILER_RECORD_T *record );


END_C_NAMESPACE

#endif
