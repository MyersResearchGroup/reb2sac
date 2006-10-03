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

#if 0
#define TESTING 1
#endif

#include "reb2sac.h"
#ifdef TESTING 
#include "util.h"
#include "compiler_def.h"
RET_VAL TestProperties( int argc, char *argv[] );
#endif 


int main( int argc, char *argv[] ) {
    int ret = 0;
#ifdef TESTING 
    ret = TestProperties( argc, argv );

#else    
    ret = Reb2SacMain( argc, argv );
#endif     
    return ret;
}


#ifdef TESTING 
RET_VAL TestProperties( int argc, char *argv[] ) {
    char *reb2sacHome = NULL;
    PROPERTIES *properties = NULL;
    char path[512];
    
    StartLog(COMPILER_LOG_NAME);
    reb2sacHome = getenv( "REB2SAC_HOME" );
    sprintf( path, "%s%cconf%creb2sac.properties", reb2sacHome, FILE_SEPARATOR, FILE_SEPARATOR );    
    if( ( properties = CreateProperties( path ) ) == NULL ) {
        printf("something is wrong" NEW_LINE );
        EndLog();
        return FAILING;
    }    
    
    printf( "value of test1.aaa is %s" NEW_LINE, properties->GetProperty( properties, "test1.aaa" ) );
    
    EndLog();
    return SUCCESS;
}
#endif 
