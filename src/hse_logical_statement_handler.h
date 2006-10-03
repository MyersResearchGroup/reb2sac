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
#if !defined(HAVE_HSE_LOGICAL_STATEMENT_HANDLER)
#define HAVE_HSE_LOGICAL_STATEMENT_HANDLER

#include "common.h"
#include "compiler_def.h"
#include "kinetic_law_evaluater.h"
#include "back_end_processor.h"
#include "hse_back_end_processor_common.h"

BEGIN_C_NAMESPACE

typedef struct {
    REACTION *reaction;
    LINKED_LIST *inhibitors;
    LINKED_LIST *catalysts;
    BOOL isRateZero;
    double rate;
    KINETIC_LAW_EVALUATER *evaluater;
    int type;
} REB2SAC_HSE_REACTION_RECORD;


struct _HSE_LOGICAL_STATEMENT_HANDLER;
typedef struct _HSE_LOGICAL_STATEMENT_HANDLER HSE_LOGICAL_STATEMENT_HANDLER;


struct _HSE_LOGICAL_STATEMENT_HANDLER {
    BACK_END_PROCESSOR *backend;
    RET_VAL (*Handle)( HSE_LOGICAL_STATEMENT_HANDLER *handler, REB2SAC_HSE_REACTION_RECORD *reactionRec, FILE *file );
};

RET_VAL HandleLogicalStatement( BACK_END_PROCESSOR *backend, REACTION *reaction, int index, FILE *file );
RET_VAL HandleLogicalStatementInXML( BACK_END_PROCESSOR *backend, REACTION *reaction, FILE *file );

END_C_NAMESPACE


#endif
















