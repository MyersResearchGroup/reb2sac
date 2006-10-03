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
#if !defined(HAVE_FRONT_END_PROCESSOR)
#define HAVE_FRONT_END_PROCESSOR

#include "common.h"
#include "compiler_def.h"
#include "IR.h"

BEGIN_C_NAMESPACE

struct _FRONT_END_PROCESSOR;
typedef struct _FRONT_END_PROCESSOR FRONT_END_PROCESSOR;

RET_VAL InitFrontendProcessor( COMPILER_RECORD_T *record,  FRONT_END_PROCESSOR *frontend );

struct _FRONT_END_PROCESSOR {
    COMPILER_RECORD_T *record;
    CADDR_T _internal1;
    CADDR_T _internal2;
    CADDR_T _internal3;
    RET_VAL (*Process)( FRONT_END_PROCESSOR *frontend, IR *ir );
    RET_VAL (*Close)( FRONT_END_PROCESSOR *frontend );
};


END_C_NAMESPACE

#endif 
 

