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
#if !defined(HAVE_NEXT_REACTION_SIMULATION)
#define HAVE_NEXT_REACTION_SIMULATION

#include "simulation_method.h"

BEGIN_C_NAMESPACE


DLLSCOPE RET_VAL STDCALL DoNextReactionSimulation( BACK_END_PROCESSOR *backend, IR *ir );
DLLSCOPE RET_VAL STDCALL CloseNextReactionSimulator( BACK_END_PROCESSOR *backend );


typedef struct {
    REACTION **reactionArray;
    UINT32 reactionsSize;
    SPECIES **speciesArray;
    UINT32 speciesSize;
    REACTION *nextReaction;    
    SIMULATION_PRINTER *printer;
    SIMULATION_RUN_TERMINATION_DECIDER *decider;
    double time;
    double printInterval;
    double nextPrintTime;    
    double timeLimit;
    KINETIC_LAW_EVALUATER *evaluator;
    double totalPropensities;
    UINT32 seed;
    UINT32 runs; 
    char *outDir; 
} NEXT_REACTION_SIMULATION_RECORD;



END_C_NAMESPACE

#endif
