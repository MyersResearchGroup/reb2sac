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
#if !defined(HAVE_REB2SAC)
#define HAVE_REB2SAC

#include "common.h"
#include "compiler_def.h"
#include "IR.h"
#include "front_end_processor.h"
#include "abstraction_engine.h"
#include "back_end_processor.h"
#include "util.h"

#if defined(__cplusplus)
extern "C" {
#endif

int Reb2SacMain( int argc, char *argv[] );



struct _REB2SAC;
typedef struct _REB2SAC REB2SAC;

struct _REB2SAC {
    COMPILER_RECORD_T *record;
    FRONT_END_PROCESSOR frontend;
    ABSTRACTION_ENGINE abstractionEngine;
    BACK_END_PROCESSOR backend;
    
    RET_VAL (*Init)( REB2SAC *reb2sac, int argc, char *argv[] );
    RET_VAL (*LoadProperties)( REB2SAC *reb2sac );
    RET_VAL (*HandleFrontEnd)( REB2SAC *reb2sac );
    RET_VAL (*Abstract)( REB2SAC *reb2sac );
    RET_VAL (*Abstract1)( REB2SAC *reb2sac );
    RET_VAL (*Abstract2)( REB2SAC *reb2sac );
    RET_VAL (*Abstract3)( REB2SAC *reb2sac );
    RET_VAL (*HandleBackEnd)( REB2SAC *reb2sac );
        
    PROPERTIES* (*GetOptions)( REB2SAC *reb2sac );
    REB2SAC_PROPERTIES* (*GetProperties)( REB2SAC *reb2sac );
    FRONT_END_PROCESSOR* (*GetFrontEndProcessor)( REB2SAC *reb2sac );
    BACK_END_PROCESSOR* (*GetBackEndProcessor)( REB2SAC *reb2sac );
    ABSTRACTION_ENGINE* (*GetAbstractionEngine)( REB2SAC *reb2sac );
    IR* (*GetIR)( REB2SAC *reb2sac );
    
    int (*Main)( int argc, char *argv[] );    
};

DLLSCOPE REB2SAC *GetReb2sacInstance();

#if defined(__cplusplus)
}
#endif

#endif
