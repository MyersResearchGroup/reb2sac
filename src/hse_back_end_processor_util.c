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
#include "hse_back_end_processor_util.h"

char *GetLogicalSpeciesID( BACK_END_PROCESSOR *backend, SPECIES *species ) {
    char *id = NULL;
    
    START_FUNCTION("GetLogicalSpeciesID");
    
    id = GetCharArrayOfString( GetSpeciesNodeName( species ) );
        
    END_FUNCTION("GetLogicalSpeciesID", SUCCESS );
    return id;
}


char *GenerateRealNumStringForHse( double value ) {
    static char buf[64];
    double intPart = 0.0;
    double fracPart = 0.0;
    
    START_FUNCTION("GenerateRealNumStringForHse");
#if 1
    fracPart = modf( value, &intPart );
    if( IS_REAL_EQUAL( fracPart, 0.0 ) ) {
        sprintf( buf, "%.1f", value );  
    } 
    else {
        sprintf( buf, "%g", value );
    }
#else
    sprintf( buf, "%g", value );
#endif
    
    END_FUNCTION("GenerateRealNumStringForHse", SUCCESS );
    return buf;
}
