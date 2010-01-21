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
#if !defined(HAVE_EULER_METHOD)
#define HAVE_EULER_METHOD

#include "simulation_method.h"

BEGIN_C_NAMESPACE


DLLSCOPE RET_VAL STDCALL DoEulerSimulation( BACK_END_PROCESSOR *backend, IR *ir );
DLLSCOPE RET_VAL STDCALL CloseEulerSimulation( BACK_END_PROCESSOR *backend );

typedef struct {
    REACTION **reactionArray;
    UINT32 reactionsSize;
    SPECIES **speciesArray;
    UINT32 speciesSize;
    RULE **ruleArray;
    UINT32 rulesSize;
    UINT32 algebraicRulesSize;
    UINT32 numberFastSpecies;
    UINT32 numberFastReactions;
    double *fastCons;
    COMPARTMENT **compartmentArray;
    UINT32 compartmentsSize;
    REB2SAC_SYMBOL **symbolArray;
    UINT32 symbolsSize;
    CONSTRAINT **constraintArray;
    UINT32 constraintsSize;
    EVENT **eventArray;
    UINT32 eventsSize;
    REACTION *nextReaction;    
    SIMULATION_PRINTER *printer;
    SIMULATION_RUN_TERMINATION_DECIDER *decider;
    double time;
    double timeStep;
    UINT32 currentStep;
    UINT32 numberSteps;
    double minPrintInterval;
    double nextPrintTime;    
    double timeLimit;
    KINETIC_LAW_EVALUATER *evaluator;
    KINETIC_LAW_FIND_NEXT_TIME *findNextTime;
    UINT32 seed;
    UINT32 runs; 
    char *outDir; 
    int startIndex;
} EULER_SIMULATION_RECORD;



END_C_NAMESPACE

#endif
