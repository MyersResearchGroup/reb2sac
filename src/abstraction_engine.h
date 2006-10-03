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
#if !defined(HAVE_ABSTRACTION_ENGINE)
#define HAVE_ABSTRACTION_ENGINE

#include "common.h"
#include "compiler_def.h"
#include "IR.h"
#include "abstraction_method_manager.h"
#include "abstraction_reporter.h"

#include "hash_table.h"
#include "linked_list.h"

BEGIN_C_NAMESPACE

struct _ABSTRACTION_ENGINE;
typedef struct _ABSTRACTION_ENGINE ABSTRACTION_ENGINE;


RET_VAL InitAbstractionEngine( COMPILER_RECORD_T *record, ABSTRACTION_ENGINE *abstractionEngine );

struct _ABSTRACTION_ENGINE {
    COMPILER_RECORD_T *record;
    ABSTRACTION_METHOD_MANAGER *manager;
    ABSTRACTION_METHOD** (*GetRegisteredMethods)( ABSTRACTION_ENGINE *abstractionEngine );
    RET_VAL (*Abstract)( ABSTRACTION_ENGINE *abstractionEngine, IR *ir );
    RET_VAL (*Abstract1)( ABSTRACTION_ENGINE *abstractionEngine, IR *ir );
    RET_VAL (*Abstract2)( ABSTRACTION_ENGINE *abstractionEngine, IR *ir );
    RET_VAL (*Abstract3)( ABSTRACTION_ENGINE *abstractionEngine, IR *ir );
    RET_VAL (*Close)( ABSTRACTION_ENGINE *abstractionEngine );
    
};


END_C_NAMESPACE

#endif 
