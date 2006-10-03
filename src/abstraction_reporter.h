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
#if !defined(HAVE_ABSTRACTION_REPORTER)
#define HAVE_ABSTRACTION_REPORTER

#include "common.h"
#include "IR.h"

#define DEFAULT_ABSTRACTION_REPORTER_FILENAME "abs_report.csv"
 


BEGIN_C_NAMESPACE

struct  _ABSTRACTION_REPORTER;
typedef struct  _ABSTRACTION_REPORTER ABSTRACTION_REPORTER;

struct _ABSTRACTION_REPORTER {
    FILE *out;
    CADDR_T _internal1;
    CADDR_T _internal2;
    RET_VAL (STDCALL *ReportInitial)( ABSTRACTION_REPORTER *reporter, IR *ir );
    RET_VAL (STDCALL *Report)( ABSTRACTION_REPORTER *reporter, char *abstractionName, IR *ir );
    RET_VAL (STDCALL *ReportFinal)( ABSTRACTION_REPORTER *reporter, IR *ir );
    RET_VAL (STDCALL *ReleaseResource)( ABSTRACTION_REPORTER *reporter  );
};

DLLSCOPE ABSTRACTION_REPORTER * STDCALL CreateAbstractionReporter( char *type );
DLLSCOPE RET_VAL STDCALL CloseAbstractionReporter( ABSTRACTION_REPORTER **rep );

END_C_NAMESPACE

#endif
