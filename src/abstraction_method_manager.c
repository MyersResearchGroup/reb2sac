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
#include "abstraction_method_manager.h"


static ABSTRACTION_METHOD_MANAGER instance;

static RET_VAL _Init( ABSTRACTION_METHOD_MANAGER *manager, COMPILER_RECORD_T *record );
static RET_VAL _Free( ABSTRACTION_METHOD_MANAGER *manager );

static COMPILER_RECORD_T * _GetCompilerRecord( ABSTRACTION_METHOD_MANAGER *manager );
static RET_VAL _RegisterMethod(ABSTRACTION_METHOD_MANAGER *manager, ABSTRACTION_METHOD *method );
static ABSTRACTION_METHOD * _LookupMethod( ABSTRACTION_METHOD_MANAGER *manager, char *id );
static ABSTRACTION_METHOD **  _GetMethods( ABSTRACTION_METHOD_MANAGER *manager );          

static RET_VAL _FreeMethod( ABSTRACTION_METHOD *method );      

ABSTRACTION_METHOD_MANAGER *GetAbstractionMethodManagerInstance() {
    
    START_FUNCTION("GetAbstractionMethodManagerInstance");

    if( instance.Init == NULL) {
        instance.Init = _Init;
        instance.GetCompilerRecord = _GetCompilerRecord;
        /*
        instance.RegisterMethod = _RegisterMethod;
        */
        instance.LookupMethod = _LookupMethod;
        instance.GetMethods = _GetMethods; 
        instance.Free = _Free;
    }
    
    END_FUNCTION("GetAbstractionMethodManagerInstance", SUCCESS );
    return &instance;
}




static RET_VAL _Init( ABSTRACTION_METHOD_MANAGER *manager, COMPILER_RECORD_T *record ) {
    RET_VAL ret = SUCCESS;
    int i = 0;
    AbstractionMethodConstructorType constructor = NULL;
    ABSTRACTION_METHOD *method = NULL;
    
    START_FUNCTION("_Init");
    
    manager->record = record;
    
    if( ( manager->table = CreateHashTable( 64 ) ) == NULL ) {
            return ErrorReport( FAILING, "_Init", "failed to create abstraction method lookup table" );
    } 
    TRACE_1( "size of abstractionMethodConstrcutor table is %i", ABSTRACTION_METHOD_MAX );
    while( (constructor = __abstractionMethodConstrcutorTable[i]) != NULL ) {
        if( ( method = constructor( manager ) ) == NULL ) {
            return ErrorReport( FAILING, "_Init", "failed to create abstraction method" );
        } 
        if( IS_FAILED( ( ret = _RegisterMethod( manager, method ) ) ) ) {
            END_FUNCTION("_Init", ret );    
            return ret;
        }   
        i++;
    }
    
    END_FUNCTION("_Init", SUCCESS );    
    return ret;
}

static RET_VAL _Free( ABSTRACTION_METHOD_MANAGER *manager ) {
    RET_VAL ret = SUCCESS;
    int i = 0;
    ABSTRACTION_METHOD *method = NULL;
    ABSTRACTION_METHOD **methods = NULL;
    
    START_FUNCTION("_Free");

    methods = manager->methods;
    for( i = 0; i < ABSTRACTION_METHOD_MAX; i++ ) {
        method = methods[i];
        if( IS_FAILED( ( ret = method->Free( method ) ) ) ) {
            END_FUNCTION("_Free", ret );
            return ret;
        }
    }
    
    DeleteHashTable( &(manager->table) );
    manager->Init = NULL;
    
    END_FUNCTION("_Free", SUCCESS );
    return ret;
}



static COMPILER_RECORD_T * _GetCompilerRecord( ABSTRACTION_METHOD_MANAGER *manager ) {
    START_FUNCTION("_GetCompilerRecord");    
    END_FUNCTION("_GetCompilerRecord", SUCCESS );

    return manager->record;
}

static RET_VAL _RegisterMethod(ABSTRACTION_METHOD_MANAGER *manager, ABSTRACTION_METHOD *method ) {
    RET_VAL ret = SUCCESS;
    static int i = 0;
    char *id = NULL;
    
    START_FUNCTION("_RegisterMethod");

    id = method->GetID( method );
    TRACE_2("%i Registering abstraction method %s", (i+1), id );
    if( IS_FAILED( ( ret = PutInHashTable( id, strlen(id), (CADDR_T)method, manager->table ) ) ) ) {
        END_FUNCTION("_RegisterMethod", SUCCESS );    
        return ret;
    }
    if( method->Free == NULL ) {
        method->Free = _FreeMethod;
    }
    
    manager->methods[i] = method;
    i++; 
                    
    END_FUNCTION("_RegisterMethod", SUCCESS );    
    return ret;
}

static ABSTRACTION_METHOD * _LookupMethod( ABSTRACTION_METHOD_MANAGER *manager, char *id ) {
    ABSTRACTION_METHOD *method = NULL;
    
    START_FUNCTION("_LookupMethod");

    method = (ABSTRACTION_METHOD*)GetValueFromHashTable( id, strlen(id), manager->table ); 
    END_FUNCTION("_LookupMethod", SUCCESS );
    return method;
}

static ABSTRACTION_METHOD ** _GetMethods( ABSTRACTION_METHOD_MANAGER *manager ) {
    START_FUNCTION("_GetMethods");

    END_FUNCTION("_GetMethods", SUCCESS );
    return manager->methods;
}          




/*
 dummy abstraction method
*/

static char * _GetDummyMethodID( ABSTRACTION_METHOD *method );
static RET_VAL _ApplyDummyMethod( ABSTRACTION_METHOD *method, IR *ir );      

ABSTRACTION_METHOD *DummyAbstractionMethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager ) {
    static ABSTRACTION_METHOD method;
    
    START_FUNCTION("DummyAbstractionMethodConstructor");

    if( method.manager == NULL ) {
        method.manager = manager;
        method.GetID = _GetDummyMethodID;
        method.Apply = _ApplyDummyMethod;
    }
    
    TRACE_0( "dummy abstraction method constructor invoked" );
    
    END_FUNCTION("DummyAbstractionMethodConstructor", SUCCESS );
    return &method;
}

static char * _GetDummyMethodID( ABSTRACTION_METHOD *method ) {
    START_FUNCTION("_GetDummyMethodID");

    TRACE_0( "getting dummy method ID" );
    
    END_FUNCTION("_GetDummyMethodID", SUCCESS );
    return "dummy-abstraction-method";
}



static RET_VAL _ApplyDummyMethod( ABSTRACTION_METHOD *method, IR *ir ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("_ApplyDummyMethod");

    END_FUNCTION("_ApplyDummyMethod", SUCCESS );
    return ret;
}      


static RET_VAL _FreeMethod( ABSTRACTION_METHOD *method ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("_FreeMethod");

    TRACE_1("The default free method is called for method %s", method->GetID( method ) );
    
    END_FUNCTION("_FreeMethod", SUCCESS );
    return ret;
}      



