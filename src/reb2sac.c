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
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "reb2sac.h"

static RET_VAL InitCompiler( int argc, char *argv[], COMPILER_RECORD_T *record );
static RET_VAL CompilerMain( COMPILER_RECORD_T *record );
static int EndCompiler( RET_VAL ret, COMPILER_RECORD_T *record );

static REB2SAC_PROPERTIES *_CreateReb2sacProperties( COMPILER_RECORD_T *record );
static RET_VAL _CreateReb2sacOptions( int argc, char *argv[], COMPILER_RECORD_T *record );
static void _PrintWrongInputMessage( FILE *file );


int Reb2SacMain(int argc, char *argv[]) {
    static COMPILER_RECORD_T record;
    RET_VAL ret = SUCCESS;
    int exitCode = 0;

    CreateRandomNumberGenerators();
    if( IS_OK( ( ret = InitCompiler( argc, argv, &record ) ) ) ) {
        ret = CompilerMain( &record );
    }
    exitCode = EndCompiler( ret, &record );
    
#ifdef DEBUG
    if( exitCode == EXIT_SUCCESS ) {
        printf("Hello, world!" NEW_LINE );
    }
#endif
    FreeRandomNumberGenerators();

    return exitCode;
}

static RET_VAL InitCompiler( int argc, char *argv[], COMPILER_RECORD_T *record ) {
    RET_VAL ret = SUCCESS;
    char *outDir = NULL;
        
    StartLog(COMPILER_LOG_NAME);
    START_FUNCTION("InitCompiler");
    
    if( argc == 1 ) {
        _PrintWrongInputMessage( stderr );
        END_FUNCTION("InitCompiler", FAILING );
        return FAILING;
    }
    
    if( IS_FAILED( ( ret = _CreateReb2sacOptions( argc, argv,  record ) )  ) ) {
        _PrintWrongInputMessage( stderr );
        END_FUNCTION("InitCompiler", ret );
        return ret;
    }
    
    if( ( record->properties = _CreateReb2sacProperties(  record ) ) == NULL ) {
        END_FUNCTION("InitCompiler", FAILING );
        return FAILING;
    }
    
    if( IS_FAILED( ( ret = InitIR( record ) ) ) ) {
        END_FUNCTION("InitCompiler", ret );
        return ret;
    }
        
    END_FUNCTION("InitCompiler", ret );
    return ret;
}


static RET_VAL CompilerMain( COMPILER_RECORD_T *record ) {
    RET_VAL ret = SUCCESS;
    static FRONT_END_PROCESSOR frontend;
    static ABSTRACTION_ENGINE abstractionEngine;
    static BACK_END_PROCESSOR backend;
    IR *ir = NULL;
    REB2SAC_PROPERTIES *properties = NULL;
          
    START_FUNCTION("CompilerMain");
            
    ir = record->ir;
    
    properties = record->properties;
    if( IS_FAILED( ( ret = properties->LoadProperties( properties )  ) ) ) {
        END_FUNCTION("CompilerMain", ret );
        return ret;
    }
    
    if( IS_FAILED( ( ret = InitFrontendProcessor( record, &frontend ) ) ) ) {
        END_FUNCTION("CompilerMain", ret );
        return ret;
    }
        
    if( IS_FAILED( ( ret = InitBackendProcessor( record, &backend ) ) ) ) {
        END_FUNCTION("CompilerMain", ret );
        return ret;
    }
    
        
    if( IS_FAILED( ( ret = InitAbstractionEngine( record, &abstractionEngine ) ) ) ) {
        END_FUNCTION("CompilerMain", ret );
        return ret;
    }
                
    if( IS_FAILED( ( ret = frontend.Process( &frontend, ir ) ) ) ) {
        END_FUNCTION("CompilerMain", ret );
        return ret;
    }
           
        
    if( IS_FAILED( ( ret = abstractionEngine.Abstract( &abstractionEngine, ir ) ) ) ) {
        END_FUNCTION("CompilerMain", ret );
        return ret;
    }
            
        
    if( IS_FAILED( ( ret = backend.Process( &backend, ir ) ) ) ) {
        END_FUNCTION("CompilerMain", ret );
        return ret;
    }
        
    
    
    if( IS_FAILED( ( ret = abstractionEngine.Close( &abstractionEngine  ) ) ) ) {
        END_FUNCTION("CompilerMain", ret );
        return ret;
    }
        
    
    if( IS_FAILED( ( ret = frontend.Close( &frontend )  ) ) ) {
        END_FUNCTION("CompilerMain", ret );
        return ret;
    }
    
    if( IS_FAILED( ( ret = backend.Close( &backend  ) ) ) ) {
        END_FUNCTION("CompilerMain", ret );
        return ret;
    }
               
    END_FUNCTION("CompilerMain", ret );
    return ret;
}

static int EndCompiler( RET_VAL ret, COMPILER_RECORD_T *record ) {
    PROPERTIES *prop = NULL;
    REB2SAC_PROPERTIES *reb2sacProp = NULL;
    STRING *reb2sacHome = NULL;
    STRING *inputPath = NULL;

    START_FUNCTION("EndCompiler");

    FreeIR( record->ir );
    if( ( prop = record->options ) != NULL ) {
        prop->Free( prop );
    }
    if( ( reb2sacProp = record->properties ) != NULL ) {
        reb2sacProp->Free( reb2sacProp );
    }
    if( ( reb2sacHome = record->reb2sacHome ) != NULL ) {
        FreeString( &reb2sacHome );
    }
    if( ( inputPath = record->inputPath ) != NULL ) {
        FreeString( &inputPath );
    }    
    
    END_FUNCTION("EndCompiler", ret );
    EndLog();
    
    if( IS_FAILED( ret ) ) {
        return -1;
    }
    else {
        return EXIT_SUCCESS;
    }
}


static REB2SAC_PROPERTIES *_CreateReb2sacProperties( COMPILER_RECORD_T *record ) {
    RET_VAL ret = SUCCESS;
    char *value = NULL;
    PROPERTIES *options = NULL;
    REB2SAC_PROPERTIES *properties = NULL;
       
    START_FUNCTION("_CreateReb2sacProperties");
    
    options = record->options;    
    if( ( value = options->GetProperty( options, REB2SAC_PROPERTIES_KEY ) ) == NULL ) {
        value = "default";
    }
    
    if( strcmp( value, "default" ) == 0 ) {
      properties = (REB2SAC_PROPERTIES*)DefaultReb2sacPropertiesConstructor( record );
        if( IS_FAILED( ( ret = properties->Init( properties ) ) ) ) {
            END_FUNCTION("_CreateReb2sacProperties", ret );
            return NULL;
        }
        END_FUNCTION("_CreateReb2sacProperties", SUCCESS );
        return properties;
    }
    else {
    }
    
    END_FUNCTION("_CreateReb2sacProperties", FAILING );
    return NULL;
}

static RET_VAL _CreateReb2sacOptions( int argc, char *argv[], COMPILER_RECORD_T *record ) {
    RET_VAL ret = SUCCESS;
    int i = 1;
    int pos = 0;
    int endPos = 0;
    int len = 0;
    int max = 0;
    char *key = NULL;
    char *value = NULL;
    char *arg = NULL;
    FILE *file = NULL;
    PROPERTIES *options = NULL;
        
    START_FUNCTION("_CreateReb2sacOptions");
    
    max = argc - 1;
    if( ( file = fopen( argv[max], "r" ) ) == NULL ) {
        return ErrorReport( FAILING, "_CreateReb2sacOptions", "could not read from file %s", argv[max] );
    }    
    fclose( file );
    
    TRACE_1("input filename is %s", argv[max] );
    if( ( record->inputPath = CreateString( argv[max] ) ) == NULL ) {
        return ErrorReport( FAILING, "_CreateReb2sacOptions", "could not store string %s", argv[max] );
    }    
    
    if( ( record->options = CreateEmptyProperties() ) == NULL ) {
        return ErrorReport( FAILING, "_CreateReb2sacOptions", "could not create properties for options" );
    }
    options = record->options;
        
    for( i = 1; i < max; i++ ) {
        arg = argv[i];
        len = strlen( arg );
        TRACE_1("handling option %s", arg );
        if( len < 5 ) {
            return ErrorReport( FAILING, "_CreateReb2sacOptions", "%s is not a valid option", arg );
        }
        if( ( arg[0] != '-' ) || ( arg[1] != '-' ) ) {
            return ErrorReport( FAILING, "_CreateReb2sacOptions", "%s is not a valid option", arg );
        } 
        
        key = arg + 2;
        if( ( value = strchr( arg, '=' ) ) == NULL ) {
            return ErrorReport( FAILING, "_CreateReb2sacOptions", "%s is not a valid option", arg );
        }
        value[0] = '\0';
        value += 1;
        if( IS_FAILED( ( ret = options->SetProperty( options, key, value ) ) ) ) {
            END_FUNCTION("_CreateReb2sacOptions", ret );
            return ret;
        }                                
    }
            
    END_FUNCTION("_CreateReb2sacOptions", SUCCESS );
    return ret;
}

static void _PrintWrongInputMessage( FILE *file ) {
    START_FUNCTION("_PrintWrongInputMessage");

    fprintf( file, 
    "usage: reb2sac [options] <source-filename>" NEW_LINE
    "where options are:" NEW_LINE 
    "--<key>=<value> predefined keys are:" NEW_LINE
    "source.encoding, target.encoding, out and reb2sac.properties" NEW_LINE
    "--source.encoding=sbml is default" NEW_LINE
    "--target.encoding=hse2 is default" NEW_LINE
    "--out=<target-filename> if this option is not provided, the target filename is specified by backend" NEW_LINE
    "--reb2sac.properties=default is default" NEW_LINE
    "--reb2sac.properties.file can be used to specfied a properties file for default reb2sac properties handler" NEW_LINE 
    );
    END_FUNCTION("_PrintWrongInputMessage", SUCCESS );
}


REB2SAC *GetReb2sacInstance() {
    static REB2SAC instance;
    
    return &instance;
}
