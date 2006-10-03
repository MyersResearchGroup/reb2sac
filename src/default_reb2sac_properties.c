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
#include "default_reb2sac_properties.h"
#include "util.h"


static RET_VAL _Init( REB2SAC_PROPERTIES *properties );
static RET_VAL _LoadProperties(  REB2SAC_PROPERTIES *properties );
static RET_VAL _SetProperty( REB2SAC_PROPERTIES *properties, char *key, char *value );
static char * _GetProperty(  REB2SAC_PROPERTIES *properties, char *key );
static RET_VAL _Free(  REB2SAC_PROPERTIES *properties );

static REB2SAC_PROPERTIES instance;


REB2SAC_PROPERTIES * DefaultReb2sacPropertiesConstructor( COMPILER_RECORD_T *record ) {
    
    START_FUNCTION("DefaultReb2sacPropertiesConstructor");
   
    if( instance.record == NULL ) {
        instance.record = record;
        instance.Init = _Init;
        instance.LoadProperties = _LoadProperties;
        instance.SetProperty = _SetProperty;
        instance.GetProperty = _GetProperty;
        instance.Free = _Free;
    }
            
    END_FUNCTION("DefaultReb2sacPropertiesConstructor", SUCCESS );
    return &instance;
}



static RET_VAL _Init( REB2SAC_PROPERTIES *properties ) {
    RET_VAL ret = SUCCESS;
    char *filename = NULL;
    char *dir = NULL;
    char *reb2sacHome = NULL;
    char filenameBuf[256];
    char dirBuf[1024];
    char pathBuf[1024];
    COMPILER_RECORD_T *record = NULL;    
    PROPERTIES *systemProp = NULL;
    STRING *path = NULL;
    FILE *file = NULL;
            
    START_FUNCTION("_Init");
    
    record = properties->record;
    systemProp = record->options;
    if( ( filename = systemProp->GetProperty( systemProp, REB2SAC_PROPERTIES_FILE_KEY ) ) != NULL ) {
        if( ( file = fopen( filename, "r" ) ) != NULL ) {
            if( ( path = CreateString( filename ) ) == NULL ) {
                return ErrorReport( FAILING, "_Init", "could not create string %s", filename ); 
            }
            fclose( file );
            properties->_internal1 = (CADDR_T)path;        
            END_FUNCTION("_Init", SUCCESS );
            return ret;
        }            
    }
    
    if( ( filename = GetFileNameWithoutExtension( GetCharArrayOfString( record->inputPath ), filenameBuf, 0, sizeof( filenameBuf ) ) ) != NULL ) {
        if( ( dir = GetDirectory( GetCharArrayOfString( record->inputPath ), dirBuf, 0, sizeof(dirBuf) ) ) == NULL ) {
            dir = ".";
        }        
        sprintf( pathBuf, "%s%c%s.properties", dir, FILE_SEPARATOR, filename );
        if( ( file = fopen( pathBuf, "r" ) ) != NULL ) {
            if( ( path = CreateString( pathBuf ) ) == NULL ) {
                return ErrorReport( FAILING, "_Init", "could not create string %s", pathBuf ); 
            }
            fclose( file );
            properties->_internal1 = (CADDR_T)path;        
            END_FUNCTION("_Init", SUCCESS );
            return ret;
        }            
    } 
            
    if( ( file = fopen( "reb2sac.properties", "r" ) ) != NULL ) {
        if( ( path = CreateString( "reb2sac.properties" ) ) == NULL ) {
            return ErrorReport( FAILING, "_Init", "could not create string reb2sac.properties" ); 
        }
        fclose( file );
    }    
    else {
        /*        
        if( ( reb2sacHome = getenv( "REB2SAC_HOME" ) ) == NULL ) {
            return ErrorReport( FAILING, "_Init", "environment variable REB2SAC_HOME is not defined" ); 
        }
        sprintf( pathBuf, "%s%cconf%creb2sac.properties", reb2sacHome, FILE_SEPARATOR, FILE_SEPARATOR );
        if( ( file = fopen( pathBuf, "r" ) ) == NULL ) {
            return ErrorReport( FAILING, "_Init", "could not find reb2sac.properties" ); 
        }
        fclose( file );    
        if( ( path = CreateString( pathBuf ) ) == NULL ) {
            return ErrorReport( FAILING, "_Init", "could not create string %s", pathBuf ); 
        }
        */
        path = NULL;
    }
    properties->_internal1 = (CADDR_T)path;
        
    END_FUNCTION("_Init", SUCCESS );
    return ret;
}

static RET_VAL _LoadProperties(  REB2SAC_PROPERTIES *properties ) {
    RET_VAL ret = SUCCESS;
    char *path = NULL;
    PROPERTIES *prop = NULL;
    
    START_FUNCTION("_LoadProperties");
    
    path = GetCharArrayOfString( (STRING*)(properties->_internal1) );
    if( path != NULL ) {
        if( ( prop = CreateProperties( path ) ) == NULL ) {
            return ErrorReport( FAILING, "_LoadProperties", "could not load properties from %s", path );
        } 
    }
    else {
        if( ( prop = CreateEmptyProperties()) == NULL ) {
            return ErrorReport( FAILING, "_LoadProperties", "could not load empty properties" );
        } 
    }
    properties->_internal2 = (CADDR_T)prop;
    
    END_FUNCTION("_LoadProperties", SUCCESS );
    return ret;
}

static char * _GetProperty(  REB2SAC_PROPERTIES *properties, char *key ) {
    char *value = NULL;
    PROPERTIES *prop = NULL;
    
    START_FUNCTION("_GetProperty");
    
    prop = (PROPERTIES*)(properties->_internal2);
    value = prop->GetProperty( prop, key );
    
    END_FUNCTION("_GetProperty", SUCCESS );
    return value;
}

static RET_VAL _SetProperty( REB2SAC_PROPERTIES *properties, char *key, char *value ) {
    RET_VAL ret = SUCCESS;
    PROPERTIES *prop = NULL;
    
    START_FUNCTION("_SetProperty");
    
    prop = (PROPERTIES*)(properties->_internal2);
    if( IS_FAILED( ( ret = prop->SetProperty( prop, key, value ) ) ) ) {
        END_FUNCTION("_SetProperty", ret );
        return ret;
    }
    
    END_FUNCTION("_SetProperty", SUCCESS );
    return ret;
}


static RET_VAL _Free(  REB2SAC_PROPERTIES *properties ) {
    RET_VAL ret = SUCCESS;
    STRING *path = NULL;
    PROPERTIES *prop = NULL;
    
    START_FUNCTION("_Free");
    path = (STRING*)(properties->_internal1);
    if( path != NULL ) {  
        FreeString( &path );
    }
    prop = (PROPERTIES*)(properties->_internal2);
    if( prop != NULL ) {
        ret = prop->Free( prop );
    }
    END_FUNCTION("_Free", SUCCESS );
    return ret;
}

