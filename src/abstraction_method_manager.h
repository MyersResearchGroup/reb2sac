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
#if !defined(HAVE_ABSTRACTION_METHOD_MANAGER)
#define HAVE_ABSTRACTION_METHOD_MANAGER

#include "common.h"
#include "compiler_def.h"
#include "IR.h"

#include "hash_table.h"
#include "linked_list.h"

#include "abstraction_method_properties.h"

BEGIN_C_NAMESPACE

struct _ABSTRACTION_METHOD_MANAGER;
typedef struct _ABSTRACTION_METHOD_MANAGER ABSTRACTION_METHOD_MANAGER;

struct _ABSTRACTION_METHOD;
typedef struct _ABSTRACTION_METHOD ABSTRACTION_METHOD;


struct _ABSTRACTION_METHOD {
    ABSTRACTION_METHOD_MANAGER *manager;    
    CADDR_T _internal1;
    CADDR_T _internal2;
    CADDR_T _internal3;
    
    char * (*GetID)( ABSTRACTION_METHOD *method );
    RET_VAL (*Apply)( ABSTRACTION_METHOD *method, IR *ir );
    RET_VAL (*Free)( ABSTRACTION_METHOD *method );      
    ABSTRACTION_PROPERTY_INFO* (*GetAbstractionPropertiesInfo)( ABSTRACTION_METHOD *method );
};


/* do not move this part */
#include "abstraction_method_constructor.h"


struct _ABSTRACTION_METHOD_MANAGER {
    COMPILER_RECORD_T *record;
    HASH_TABLE *table;
    ABSTRACTION_METHOD *methods[ABSTRACTION_METHOD_MAX + 1];
    
    RET_VAL (*Init)( ABSTRACTION_METHOD_MANAGER *manager, COMPILER_RECORD_T *record );
    RET_VAL (*Free)( ABSTRACTION_METHOD_MANAGER *manager );
    COMPILER_RECORD_T * (*GetCompilerRecord)( ABSTRACTION_METHOD_MANAGER *manager );
    /*
    RET_VAL (*RegisterMethod)(ABSTRACTION_METHOD_MANAGER *manager, ABSTRACTION_METHOD *method );
    */
    ABSTRACTION_METHOD * (*LookupMethod)( ABSTRACTION_METHOD_MANAGER *manager, char *id );
    ABSTRACTION_METHOD ** (*GetMethods)( ABSTRACTION_METHOD_MANAGER *manager );          
};

ABSTRACTION_METHOD_MANAGER *GetAbstractionMethodManagerInstance();


END_C_NAMESPACE

#endif 
