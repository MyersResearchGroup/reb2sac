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
#if !defined(HAVE_BACK_END_PROCESSOR)
#define HAVE_BACK_END_PROCESSOR

#include "common.h"
#include "compiler_def.h"
#include "IR.h"

/*
* the default is sbml until hse backend processor gets implemented
* #define REB2SAC_DEFAULT_TARGET_ENCODING "sbml"
*/
#define REB2SAC_DEFAULT_TARGET_ENCODING "hse2"


BEGIN_C_NAMESPACE

struct _BACK_END_PROCESSOR;
typedef struct _BACK_END_PROCESSOR BACK_END_PROCESSOR;


RET_VAL InitBackendProcessor( COMPILER_RECORD_T *record, BACK_END_PROCESSOR *backend );

struct _BACK_END_PROCESSOR {
    COMPILER_RECORD_T *record;
    char *outputFilename;
    char *encoding;
    CADDR_T _internal1;
    CADDR_T _internal2;
    CADDR_T _internal3;    
    RET_VAL (*Process)( BACK_END_PROCESSOR *backend, IR *ir );
    RET_VAL (*Close)( BACK_END_PROCESSOR *backend );
    int useMP;
    BOOL useMedian;
    BOOL useBifur;
};

typedef struct {
    char *id;
    int type;
    char *outputNames;
    char *info;
} BACK_END_PROCESSOR_INFO;

#define BACK_END_PROCESSOR_TYPE_COMPILER 0
#define BACK_END_PROCESSOR_TYPE_SIMULATOR 1

BACK_END_PROCESSOR_INFO *GetInfoOnBackEndProcessors();
char *GetTypeNameOfBackEndProcessor( int type );

END_C_NAMESPACE

#endif 
