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

#include "abstraction_method_properties.h"
#include "front_end_processor.h"
#include "sbml_front_end_processor.h"

static RET_VAL _AddPreProcessingMethods( COMPILER_RECORD_T *record, char *methodIDs[] );

static char *__SBML_PRE_PROCESSING_METHODS[] = {
  /*    "modifier-structure-transformer",
	"modifier-constant-propagation",*/
    NULL    
};


RET_VAL InitFrontendProcessor( COMPILER_RECORD_T *record, FRONT_END_PROCESSOR *frontend ) {
    RET_VAL ret = SUCCESS;
    char *encoding = NULL;
    PROPERTIES *options = NULL;
    
    START_FUNCTION("InitFrontendProcessor");
    

    frontend->record = record;
    options = record->options;
    
    if( ( encoding = options->GetProperty( options, REB2SAC_SOURCE_ENCODING_KEY ) ) == NULL ) {
        encoding = "sbml";
    }    
        
    switch( encoding[0] ) {
        case 's':
            if( strcmp( encoding, "sbml" ) == 0 ) {
                if( IS_FAILED( ( ret = _AddPreProcessingMethods( record, __SBML_PRE_PROCESSING_METHODS ) ) ) ) {
                    return ret;
                }
                frontend->Process = ProcessSBMLFrontend;
                frontend->Close = CloseSBMLFrontend;
            }
        break;
        
        default:
            fprintf( stderr, "source encoding type %s is invalid", encoding ); 
            return ErrorReport( FAILING, "InitFrontendProcessor", "source encoding type %s is invalid", encoding );
        break;    
    }
   
    END_FUNCTION("InitFrontendProcessor", SUCCESS );
    return ret;
}

static RET_VAL _AddPreProcessingMethods( COMPILER_RECORD_T *record, char *methodIDs[] ) {
    RET_VAL ret = SUCCESS;
    int i = 0;
    int j = 0;
    char buf[512];
    char *methodID = NULL;
    REB2SAC_PROPERTIES *properties = NULL;
        
    properties = record->properties;        
    for( i = 1, j = 0; methodIDs[j] != NULL; i++ ) {        
        sprintf( buf, "%s1.%i", REB2SAC_ABSTRACTION_METHOD_KEY_PREFIX, i );
        if( ( methodID = properties->GetProperty( properties, buf ) ) != NULL ) {
            continue;
        }
        if( IS_FAILED( ( ret = properties->SetProperty( properties, buf, methodIDs[j] ) ) ) ) {
            return ret;
        }
        j++;
    }
    
    return SUCCESS;
}




 
