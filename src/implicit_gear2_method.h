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
#if !defined(HAVE_IMPLICIT_GEAR2_METHOD)
#define HAVE_IMPLICIT_GEAR2_METHOD

#include "simulation_method.h"

BEGIN_C_NAMESPACE

//#define IMPLICIT_GEAR2_ABSOLUTE_ERROR 1.0e-9
#define IMPLICIT_GEAR2_LOCAL_ERROR 0.0
#define IMPLICIT_GEAR2_H 1.0e-9

DLLSCOPE RET_VAL STDCALL DoImplicitGear2Simulation( BACK_END_PROCESSOR *backend, IR *ir );
DLLSCOPE RET_VAL STDCALL CloseImplicitGear2Simulation( BACK_END_PROCESSOR *backend );

typedef struct {
    REACTION **reactionArray;
    UINT32 reactionsSize;
    SPECIES **speciesArray;
    UINT32 speciesSize;
    RULE **ruleArray;
    UINT32 rulesSize;
    REACTION *nextReaction;    
    SIMULATION_PRINTER *printer;
    SIMULATION_RUN_TERMINATION_DECIDER *decider;
    double *concentrations;
    double time;
    double printInterval;
    double nextPrintTime;    
    double timeLimit;
    double absoluteError;
    KINETIC_LAW_EVALUATER *evaluator;
    char *outDir; 
} IMPLICIT_GEAR2_SIMULATION_RECORD;



END_C_NAMESPACE

#endif
