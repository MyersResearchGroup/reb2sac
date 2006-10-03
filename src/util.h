/***************************************************************************
                          util.h  -  description
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
#if !defined(HAVE_UTIL)
#define HAVE_UTIL

#include "common.h"
#include "hash_table.h"


BEGIN_C_NAMESPACE

/**
 * retrieve filename from the path
 * e.g., /aaa/bbb/ccc/ddd.eee => ddd.eee
 *
 */
DLLSCOPE char * STDCALL GetFileName( char *path, char *buf, int offset, int len );

/**
 * retrieve filename from the path
 * e.g., /aaa/bbb/ccc/ddd.eee => ddd
 *
 */
DLLSCOPE char * STDCALL GetFileNameWithoutExtension( char *path, char *buf, int offset, int len );


/**
 * retrieve filename from the path
 * e.g., /aaa/bbb/ccc/ddd.eee => eee
 *
 */
DLLSCOPE char * STDCALL GetFileExtension( char *path, char *buf, int offset, int len );


/**
 * retrieve filename from the path
 * e.g., /aaa/bbb/ccc/ddd.eee => /aaa/bbb/ccc/
 *
 */
DLLSCOPE char * STDCALL GetDirectory( char *path, char *buf, int offset, int len );




typedef struct {
    char *charArray;
    UINT length;
} STRING;

DLLSCOPE STRING * STDCALL CreateEmptyString();
DLLSCOPE STRING * STDCALL CreateString( char *str );
DLLSCOPE UINT  STDCALL GetStringLength( STRING *string );
DLLSCOPE char * STDCALL GetCharArrayOfString( STRING *string );
DLLSCOPE STRING * STDCALL AppendString( STRING *string, STRING *suffix );
DLLSCOPE STRING * STDCALL CloneString( STRING *from );
DLLSCOPE void  STDCALL FreeString( STRING **string );
DLLSCOPE int  STDCALL CompareString( STRING *s1, STRING *s2 );

struct _PROPERTIES;
typedef struct _PROPERTIES PROPERTIES;

struct _PROPERTIES {
    char *path;
    HASH_TABLE *table;
    char *( STDCALL *GetPath)( PROPERTIES *properties );
    char *( STDCALL *GetProperty)( PROPERTIES *properties, char *key );    
    RET_VAL ( STDCALL *SetProperty)( PROPERTIES *properties, char *key, char *value );
    RET_VAL ( STDCALL *RemoveProperty)( PROPERTIES *properties, char *key );
    RET_VAL ( STDCALL *Free)( PROPERTIES *properties );
};

DLLSCOPE PROPERTIES * STDCALL CreateProperties( char *path );
DLLSCOPE PROPERTIES * STDCALL CreateEmptyProperties( );


END_C_NAMESPACE

#endif
