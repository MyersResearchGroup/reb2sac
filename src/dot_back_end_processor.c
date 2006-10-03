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
#include "dot_back_end_processor.h"
 
 
RET_VAL ProcessDotBackend( BACK_END_PROCESSOR *backend, IR *ir ) {
    RET_VAL ret = SUCCESS;
    char *filename;
    FILE *file = NULL;

    START_FUNCTION("ProcessDotBackend");
    
    if( ( filename = backend->outputFilename ) == NULL ) {
        filename = REB2SAC_DEFAULT_DOT_OUTPUT_NAME;
    }
    if( ( file = fopen( filename, "w" ) ) == NULL ) {
        return ErrorReport( FAILING, "ProcessDotBackend", "sbml file open error" ); 
    }
    if( IS_FAILED( ( ret = ir->GenerateDotFile( ir, file ) ) ) ) {
        END_FUNCTION("ProcessDotBackend", ret );
        return ret;
    }
    fclose( file );
    
    END_FUNCTION("ProcessDotBackend", SUCCESS );
        
    return ret;
}




RET_VAL CloseDotBackend( BACK_END_PROCESSOR *backend ) {
    RET_VAL ret = SUCCESS;

    START_FUNCTION("CloseDotBackend");
    
    backend->record = NULL;
        
    END_FUNCTION("CloseDotBackend", SUCCESS );
        
    return ret;
}
