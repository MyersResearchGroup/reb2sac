/***************************************************************************
                          compiler_def.h  -  description
                             -------------------
    begin                : Thu Sep 11 2003
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

#if !defined(HAVE_COMPILER_DEF)
#define HAVE_COMPILER_DEF

#include "common.h"
#include <math.h>
/*
#include "sbml/SBMLDocument.h"
#include "sbml/Reaction.h"
#include "linked_list.h"
#include "hash_table.h"
*/
#include "util.h"

BEGIN_C_NAMESPACE

#define COMPILER_LOG_NAME "reb2sac"
#define TARGET_EXTENSION "hse"

#define PATH_LENGTH 256


struct _IR;
typedef struct _IR IR;

struct _REB2SAC_PROPERTIES;
typedef struct _REB2SAC_PROPERTIES REB2SAC_PROPERTIES;


#define REB2SAC_OPTION_VALUE_ON "on"
#define REB2SAC_SOURCE_ENCODING_KEY "source.encoding"
#define REB2SAC_TARGET_ENCODING_KEY "target.encoding"
#define REB2SAC_OUT_KEY "out"
#define REB2SAC_PROPERTIES_KEY "reb2sac.properties"

typedef struct {
    STRING *reb2sacHome;
    STRING *inputPath;
    PROPERTIES *options;
    REB2SAC_PROPERTIES *properties;
    IR *ir;
} COMPILER_RECORD_T;

struct _REB2SAC_PROPERTIES {
    COMPILER_RECORD_T *record;
    CADDR_T _internal1;
    CADDR_T _internal2;
    CADDR_T _internal3;
    RET_VAL (*Init)( REB2SAC_PROPERTIES *properties );
    RET_VAL (*LoadProperties)(  REB2SAC_PROPERTIES *properties );
    RET_VAL (*SetProperty)(  REB2SAC_PROPERTIES *properties, char *key, char *value );
    char * (*GetProperty)(  REB2SAC_PROPERTIES *properties, char *key );
    RET_VAL (*Free)(  REB2SAC_PROPERTIES *properties );
};

typedef REB2SAC_PROPERTIES * (*PropertiesConstructorType)( COMPILER_RECORD_T *record );
    

END_C_NAMESPACE

#endif
