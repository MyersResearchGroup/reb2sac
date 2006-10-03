/***************************************************************************
                          util.c  -  description
                             -------------------
    begin                : Fri Sep 26 2003
    copyright            : (C) 2003 by Hiroyuki Kuwahara
    email                : kuwahara@cs.utah.edu
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#if defined(DEBUG)
#undef DEBUG
#endif

#include "util.h"

/*
#define UTIL_FILE_NAME_SIZE 256
#define UTIL_FILE_EXTENSION_SIZE 8
#define UTIL_DIRECTORY_SIZE 512

static char _fileName[UTIL_FILE_NAME_SIZE];
static char _fileExtension[UTIL_FILE_EXTENSION_SIZE];
static char _directory[UTIL_DIRECTORY_SIZE];
*/
      
DLLSCOPE char * STDCALL GetFileName( char *path, char *buf, int offset, int len ) {
    char *start = NULL;
    char *target = NULL;

    START_FUNCTION("GetFileName");
        
    start = strrchr( path, FILE_SEPARATOR );
    if( start == NULL ) {
        start = path;        
    }
    else {
        start += 1;
    }
    if( *start == '\0' ) {
        END_FUNCTION("GetFileName", SUCCESS );
        return NULL;        
    }
    target = buf + offset;
       
    strncpy( target, start, len );
    target[len - 1] = '\0';
    
    END_FUNCTION("GetFileName", SUCCESS );
    return target;        
}

DLLSCOPE char * STDCALL GetFileNameWithoutExtension( char *path, char *buf, int offset, int len ) {
    int i = 0;
    int size = 0;
    char *p = NULL;
    char *target = NULL;
    char *end = NULL;
        
    START_FUNCTION("GetFileNameWithoutExtension");

    p = strrchr( path, FILE_SEPARATOR );
    if( p == NULL ) {
        p = path;
    }
    else {
        p += 1;
    }
    if( *p == '\0' ) {
        END_FUNCTION("GetFileName", SUCCESS );
        return NULL;
    }
    end = strrchr( p, '.' );
    if( end == NULL ) {
        end = p + strlen(p);        
    }
    else {
                    
    }
    target = buf + offset;
    size = len - 1;
    for( i = 0; i < size; i++, p++ ) {
        if( p == end ) {
            break;            
        }    
        target[i] = *p;        
    }
    target[i] = '\0';
    END_FUNCTION("GetFileNameWithoutExtension", SUCCESS );
    return target;
}

DLLSCOPE char * STDCALL GetFileExtension( char *path, char *buf, int offset, int len ) {
    char *start = NULL;
    char *target = NULL;
    
    START_FUNCTION("GetFileExtension");

    start = strrchr( path, '.' );
    if( start == NULL ) {
        END_FUNCTION("GetFileExtension", SUCCESS );
        return NULL;        
    }
    start++;
    target = buf + offset;
    strncpy( target, start, len );
    target[len - 1] = '\0';

    END_FUNCTION("GetFileExtension", SUCCESS );
    return target;
}


DLLSCOPE char * STDCALL GetDirectory( char *path, char *buf, int offset, int len ) {
    int i = 0;
    int size = 0;
    char *p = NULL;
    char *end = NULL;
    char *target = NULL;
        
    START_FUNCTION("GetDirectory");

    target = buf + offset;
    end = strrchr( path, FILE_SEPARATOR );
    if( end == NULL ) {
        END_FUNCTION("GetDirectory", SUCCESS );
        return NULL;        
    }
    p = &path[0];
    size = len - 2;
    for( i = 0; i < size; i++, p++ ) {
        target[i] = *p;
        if( p == end ) {
            break;
        }
    }
    target[i+1] = '\0';

    END_FUNCTION("GetDirectory", SUCCESS );
    return target;
}


DLLSCOPE STRING * STDCALL CreateEmptyString() {
    STRING *string = NULL;
            
    START_FUNCTION("CreateEmptyString");
    
    string = (STRING*)CALLOC( 1, sizeof(STRING) );
    if( string == NULL ) {
        END_FUNCTION("CreateEmptyString", FAILING );
        return NULL;
    }
        
    END_FUNCTION("CreateEmptyString", SUCCESS );
    return string;
}

DLLSCOPE STRING * STDCALL CreateString( char *str ) {
    STRING *string = NULL;
            
    START_FUNCTION("CreateString");
 #if 1  
    if( str == NULL ) {
        TRACE_0("trying to create a empty string from null");
        END_FUNCTION("CreateString", FAILING );
        return NULL;
    }
#endif    
    string = (STRING*)MALLOC(  sizeof(STRING) );
    if( string == NULL ) {
        END_FUNCTION("CreateString", FAILING );
        return NULL;
    }
    string->length = strlen( str );
    string->charArray = (char*)MALLOC( string->length + 1 );
    if( string->charArray == NULL ) {
        FREE( string );
        END_FUNCTION("CreateString", FAILING );
        return NULL;
    }
    
    strcpy( string->charArray, str );
    string->charArray[string->length] = '\0';        
    END_FUNCTION("CreateString", SUCCESS );
    return string;
}

UINT GetStringLength( STRING *string ) {    
    START_FUNCTION("GetStringLength");
    
    END_FUNCTION("GetStringLength", SUCCESS );
    return (string == NULL ? (UINT)0 : string->length );
}

DLLSCOPE char * STDCALL GetCharArrayOfString( STRING *string ) {
    START_FUNCTION("GetCharArrayOfString");
    
    END_FUNCTION("GetCharArrayOfString", SUCCESS );
    return (string == NULL ? NULL : string->charArray );
}

DLLSCOPE int STDCALL CompareString( STRING *s1, STRING *s2 ) {
    int result = 0;
    
    START_FUNCTION("CompareString");
    
    result = strcmp( s1->charArray, s2->charArray );
    
    END_FUNCTION("CompareString", SUCCESS );
    return result;
}


DLLSCOPE STRING * STDCALL AppendString( STRING *string, STRING *suffix ) {
    STRING *appended = NULL;
    int len1 = 0;
    int len2 = 0;
    int len = 0;
    char *charArray = NULL;
    START_FUNCTION("AppendString");
    
    len1 = GetStringLength( string );
    len2 = GetStringLength( suffix );
    len = len1 + len2;
    if( ( charArray = (char*)MALLOC( len + 1 ) ) == NULL ) {
        END_FUNCTION("AppendString", FAILING );
        return NULL;
    }
    if( len1 > 0 ) {
        strcpy( charArray, string->charArray );
    }
    if( len2 > 0 ) {
        strcpy( charArray + len1, suffix->charArray );
    }
    charArray[len] = '\0';
    if( ( appended = CreateString( charArray ) ) == NULL ) {
        END_FUNCTION("AppendString", FAILING );
        return NULL;
    }         
    FREE( charArray );
    END_FUNCTION("AppendString", SUCCESS );
    return appended;
}

DLLSCOPE STRING * STDCALL CloneString( STRING *from ) {
    STRING *string = NULL;
    
    START_FUNCTION("CloneString");
    
    if( ( string = CreateString( from->charArray ) ) == NULL ) {
        END_FUNCTION("CloneString", FAILING );
        return NULL;
    }
    
    END_FUNCTION("CloneString", SUCCESS );
    return string;
}


DLLSCOPE void STDCALL FreeString( STRING **string ) {
    START_FUNCTION("FreeString");
    if( *string != NULL ) {
        FREE( (*string)->charArray );    
        FREE( *string );
    }
        
    END_FUNCTION("FreeString", SUCCESS );
}

 

typedef struct {
    char *key;
    char *value;
} PROPERTIES_ENTRY;

static RET_VAL STDCALL _Properties_Load( PROPERTIES *properties, char *path );
static char * STDCALL _Properties_GetPath( PROPERTIES *properties );
static char * STDCALL _Properties_GetProperty( PROPERTIES *properties, char *key );    
static RET_VAL STDCALL _Properties_SetProperty( PROPERTIES *properties, char *key, char *value );
static RET_VAL STDCALL _Properties_RemoveProperty( PROPERTIES *properties, char *key );
static RET_VAL STDCALL _Properties_Free( PROPERTIES *properties );
static RET_VAL STDCALL _Properties_ReadLine( FILE *file, char *buf, int size );
static RET_VAL STDCALL _Properties_Parse( PROPERTIES *properties, char *buf, int size );

DLLSCOPE PROPERTIES * STDCALL CreateProperties( char *path ) {
    PROPERTIES *properties = NULL;
    
    START_FUNCTION("CreateProperties");
    
    if( ( properties = (PROPERTIES*)MALLOC( sizeof(PROPERTIES) ) ) == NULL ) {
        END_FUNCTION("CreateProperties", FAILING );
        return NULL;
    }
    if( ( properties->table = CreateHashTable( 256 ) ) == NULL ) {
        END_FUNCTION("CreateProperties", FAILING );
        return NULL;
    }
    properties->path = NULL;
    properties->GetPath = _Properties_GetPath;
    properties->GetProperty = _Properties_GetProperty;
    properties->SetProperty = _Properties_SetProperty;
    properties->RemoveProperty = _Properties_RemoveProperty;
    properties->Free = _Properties_Free;

    if( IS_FAILED( _Properties_Load( properties, path ) ) ) {
        _Properties_Free( properties );
        END_FUNCTION("CreateProperties", FAILING );
        return NULL;
    }
                    
    END_FUNCTION("CreateProperties", SUCCESS );
    return properties;
}

DLLSCOPE PROPERTIES * STDCALL CreateEmptyProperties( ) {
    PROPERTIES *properties = NULL;
    
    START_FUNCTION("CreateEmptyProperties");
    
    if( ( properties = (PROPERTIES*)MALLOC( sizeof(PROPERTIES) ) ) == NULL ) {
        END_FUNCTION("CreateEmptyProperties", FAILING );
        return NULL;
    }
    if( ( properties->table = CreateHashTable( 256 ) ) == NULL ) {
        END_FUNCTION("CreateEmptyProperties", FAILING );
        return NULL;
    }
    properties->path = NULL;
    properties->GetPath = _Properties_GetPath;
    properties->GetProperty = _Properties_GetProperty;
    properties->SetProperty = _Properties_SetProperty;
    properties->RemoveProperty = _Properties_RemoveProperty;
    properties->Free = _Properties_Free;
                
    END_FUNCTION("CreateEmptyProperties", SUCCESS );
    return properties;
}

static RET_VAL STDCALL _Properties_Load( PROPERTIES *properties, char *path ) {
    RET_VAL ret = SUCCESS;
    char buf[4096+1];
    char *newPath = NULL;
    FILE *file = NULL;
    
    START_FUNCTION("_Properties_Load");

    
    if( ( newPath = (char*)MALLOC( strlen( path ) + 1 ) ) == NULL ) {
        return ErrorReport( FAILING, "_Properties_Load", "could not allocate memory for %s", path );
    }
    properties->path = newPath;
    
    if( ( file = fopen( path, "r" ) ) == NULL ) {
        return ErrorReport( FAILING, "_Properties_Load", "file %s could not be opened" );
    }
    
    
    while( !feof( file ) ) {
        if( ferror( file ) ) {
            return ErrorReport( FAILING, "_Properties_Load", "file error in %s", path );
        }
        if( IS_FAILED( ( ret = _Properties_ReadLine( file, buf, sizeof( buf ) ) ) ) ) {
            if( ret == ( E_NODATA | E_END_OF_FILE )  ) {
                if( buf[0] != '\0' ) {
                    TRACE_1("read line is: %s", buf );
                    if( IS_FAILED( ( ret = _Properties_Parse( properties, buf, strlen( buf ) ) ) ) ) {
                        END_FUNCTION("_Properties_Load", ret );
                        return ret;
                    } 
                }
                TRACE_0("end of file" );
                    END_FUNCTION("_Properties_Load", SUCCESS );
                    return SUCCESS;
            }
            END_FUNCTION("_Properties_Load", ret );
            return ret;
        }
        TRACE_1("read line is: %s", buf );
        if( IS_FAILED( ( ret = _Properties_Parse( properties, buf, strlen( buf ) ) ) ) ) {
            END_FUNCTION("_Properties_Load", ret );
            return ret;
        } 
    }
        
    END_FUNCTION("_Properties_Load", SUCCESS );
    return ret;
}

static RET_VAL STDCALL _Properties_ReadLine( FILE *file, char *buf, int size ) {
    int i = 0;
    int len = 0;
    int remainingSize = 0;
    char *head = NULL;
    
    START_FUNCTION("_Properties_ReadLine");
    
    remainingSize = size;
    head = buf;
    head[0] = '\0';
    
            
    while( !ferror( file ) ){
        if( feof( file ) ) {
            if( head[0] == '\\' ) {
                head[0] = '\0';
            }
            END_FUNCTION("_Properties_ReadLine", SUCCESS );
            return E_NODATA | E_END_OF_FILE;
        }
        if( fgets( head, remainingSize, file ) == NULL ) {
            if( feof( file ) ) {
                END_FUNCTION("_Properties_ReadLine", SUCCESS );
                return E_NODATA | E_END_OF_FILE;
            }
            END_FUNCTION("_Properties_ReadLine", FAILING );
            return FAILING;
        }        
        if( head[0] == '#' ) {
            continue;
        }
        
        len = strlen( head );
        for( i = 0; i < len; i++ ) {            
            if( !isspace( head[i] ) ) {
                break;
            }
        }
        if( i == len ) {
            head[0] = '\0';
            if( head != buf ) {
                END_FUNCTION("_Properties_ReadLine", SUCCESS );
                return SUCCESS;
            }
            continue;
        }

        i = len - 1;
        while( isspace( head[i] ) ) {
            i--;
        }
        if( head[i] == '\\' ) {
            head += i;
            remainingSize -= i;
        }
        else {
            head[i+1] = '\0';
            END_FUNCTION("_Properties_ReadLine", SUCCESS );
            return SUCCESS;
        }
    }
    END_FUNCTION("_Properties_ReadLine", SUCCESS );
    return SUCCESS;
}


static RET_VAL STDCALL _Properties_Parse( PROPERTIES *properties, char *buf, int size ) {
    RET_VAL ret = SUCCESS;
    char c = 0;
    int i = 0;
    int j = 0;
    char *key = NULL;
    char *value = NULL;

    START_FUNCTION("_Properties_Parse");        

    for( i = 0; i < size; i++ ) {
        if( !isspace( buf[i] ) ) {
            break;
        }
    }
    if( i == size ) {
        return ErrorReport( FAILING, "_Properties_Parse", "line %s is not a valid property entry", buf );
    }
    
    key = buf + i;
    for( ; i < size; i++ ) {
        c = buf[i];
        if( c == '=' ) {
            j = i - 1;
            while( isspace( buf[j] ) ) {
                j--;
            }
            buf[j+1] = '\0';    
            if( (i + 1) >= size ) {
                return ErrorReport( FAILING, "_Properties_Parse", "line %s is not a valid property entry", buf );
            }
            for( i = i + 1; i < size; i++ ) {
                if( !isspace( buf[i] ) ) {
                    break;
                }
            }
            if( i == size ) {
                return ErrorReport( FAILING, "_Properties_Parse", "line %s is not a valid property entry", buf );
            }
            value = buf + i;
            break;
        }        
    }
    if( i == size ) {
        return ErrorReport( FAILING, "_Properties_Parse", "line %s is not a valid property entry", buf );
    }
    
    i = size - 1;
    while( isspace( buf[i] ) ) {
        i--;
    }
    buf[i + 1] = '\0';
    
    if( IS_FAILED( ( ret = _Properties_SetProperty( properties, key, value ) ) ) ) {
        END_FUNCTION("_Properties_Parse", ret );
        return ret;
    }
        
    END_FUNCTION("_Properties_Parse", SUCCESS );
    return ret;
}



static char * STDCALL _Properties_GetPath( PROPERTIES *properties ) {
    START_FUNCTION("_Properties_GetPath");
    
    END_FUNCTION("_Properties_GetPath", SUCCESS );
    return properties->path;
}

static char * STDCALL _Properties_GetProperty( PROPERTIES *properties, char *key ) {
    PROPERTIES_ENTRY *entry = NULL;
    HASH_TABLE *table = NULL;
    
    START_FUNCTION("_Properties_GetProperty");
    
    table = properties->table;
    entry = (PROPERTIES_ENTRY*)GetValueFromHashTable( key, strlen( key ), table );    
    if( entry == NULL ) {
        END_FUNCTION("_Properties_GetProperty", FAILING );
        return NULL;
    }
    END_FUNCTION("_Properties_GetProperty", SUCCESS );
    return entry->value;
}
    
static RET_VAL STDCALL _Properties_SetProperty( PROPERTIES *properties, char *key, char *value ) {
    UINT32 len = 0;
    RET_VAL ret = SUCCESS;
    PROPERTIES_ENTRY *entry = NULL;
    HASH_TABLE *table = NULL;
            
    START_FUNCTION("_Properties_SetProperty");
    
    _Properties_RemoveProperty( properties, key );
    
    if( ( entry = (PROPERTIES_ENTRY*)MALLOC( sizeof(PROPERTIES_ENTRY) ) ) == NULL ) {
        return ErrorReport( FAILING, "_Properties_SetProperty", "failed to allocate properties entry for %s and %s", key, value );
    }
    len = strlen(key);
    if( ( entry->key = (char*)MALLOC( len + 1 ) ) == NULL ) {
        return ErrorReport( FAILING, "_Properties_SetProperty", "failed to allocate properties entry key for %s", key );
    }
    strcpy( entry->key, key );
    
    if( ( entry->value = (char*)MALLOC( strlen(value) + 1 ) ) == NULL ) {
        return ErrorReport( FAILING, "_Properties_SetProperty", "failed to allocate properties entry value for %s", value );
    }
    strcpy( entry->value, value );
    
    table = properties->table;
    TRACE_2( "putting key %s and value %s in the properties", entry->key, entry->value );            
    if( IS_FAILED( ( ret = PutInHashTable( entry->key, len, entry, table ) ) ) ) {
        END_FUNCTION("_Properties_SetProperty", ret );
        return ret;
    } 
    
    END_FUNCTION("_Properties_SetProperty", SUCCESS );
    return ret;
}

static RET_VAL STDCALL _Properties_RemoveProperty( PROPERTIES *properties, char *key ) {
    RET_VAL ret = SUCCESS;
    UINT32 len = 0;
    PROPERTIES_ENTRY *entry = NULL;
    HASH_TABLE *table = NULL;
        
    START_FUNCTION("_Properties_RemoveProperty");
    
    len = strlen( key );    
    table = properties->table;
    if( ( entry = (PROPERTIES_ENTRY*)GetValueFromHashTable( key, len, table ) ) == NULL ) {
        TRACE_1("key %s is not in properties", key );
        END_FUNCTION("_Properties_RemoveProperty", SUCCESS );
        return ret;
    }
            
    if( IS_FAILED( ( ret = RemoveFromHashTable( key, len, table ) ) ) ) {
        END_FUNCTION("_Properties_RemoveProperty", ret );
        return ret;
    }

    TRACE_2( "key %s and value %s are removed from the properties", entry->key, entry->value );
    FREE( entry->key );
    FREE( entry->value );
    FREE( entry );
        
    END_FUNCTION("_Properties_RemoveProperty", SUCCESS );
    return ret;
}

static RET_VAL STDCALL _Properties_Free( PROPERTIES *properties ) {
    RET_VAL ret = SUCCESS;
    PROPERTIES_ENTRY *entry = NULL;
    LINKED_LIST *list = NULL;
    HASH_TABLE *table = NULL;
        
    START_FUNCTION("_Properties_Free");
    
    table = properties->table;
    if( ( list = GenerateValueList( table ) ) == NULL ) {
        return ErrorReport( FAILING, "_Properties_Free", "failed to create a list to delete properties" );
    }
    
    ResetCurrentElement( list );
    while( ( entry = (PROPERTIES_ENTRY*)GetNextFromLinkedList( list ) ) != NULL ) {
        FREE( entry->key );
        FREE( entry->value );
        FREE( entry );
    }
    DeleteLinkedList( &list );
    DeleteHashTable( &table );
    if( properties->path != NULL ) {
        FREE( properties->path );
    }
    BZERO( properties, sizeof(PROPERTIES) );
    
    END_FUNCTION("_Properties_Free", SUCCESS );
    return ret;
}




