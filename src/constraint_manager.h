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
#if !defined(HAVE_CONSTRAINT_MANAGER)
#define HAVE_CONSTRAINT_MANAGER

#include "common.h"
#include "linked_list.h"
#include "util.h"

#include "compiler_def.h"
#include "kinetic_law.h"

BEGIN_C_NAMESPACE

typedef struct {
    STRING *id;
    KINETIC_LAW *math;
    STRING *message;
} CONSTRAINT;

struct _CONSTRAINT_MANAGER;
typedef struct _CONSTRAINT_MANAGER CONSTRAINT_MANAGER;

struct _CONSTRAINT_MANAGER {
    LINKED_LIST *constraints;   
    COMPILER_RECORD_T *record;
    
    CONSTRAINT * (*CreateConstraint)( CONSTRAINT_MANAGER *manager, char *id );
    LINKED_LIST *(*CreateListOfConstraints)( CONSTRAINT_MANAGER *manager );                  
};

STRING *GetConstraintId( CONSTRAINT *constraintDef );
KINETIC_LAW *GetMathInConstraint( CONSTRAINT *constraintDef );
STRING *GetConstraintMessage( CONSTRAINT *constraintDef );
RET_VAL AddMathInConstraint( CONSTRAINT *constraintDef, KINETIC_LAW *math );
RET_VAL AddMessageInConstraint( CONSTRAINT *constraintDef, char *message );

CONSTRAINT_MANAGER *GetConstraintManagerInstance( COMPILER_RECORD_T *record );
RET_VAL CloseConstraintManager( );

END_C_NAMESPACE

#endif
