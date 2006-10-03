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
#include "species_critical_level.h"

RET_VAL FreeIntermediateSpeciesCriticalLevel( INTERMEDIATE_SPECIES_CRITICAL_LEVEL **pCriticalLevel ) {
    INTERMEDIATE_SPECIES_CRITICAL_LEVEL *criticalLevel = *pCriticalLevel;
    
    FREE( criticalLevel->levels );
    FREE( criticalLevel );
    
    return SUCCESS;    
}


RET_VAL FreeSpeciesCriticalLevel( SPECIES_CRITICAL_LEVEL **pCriticalLevel ) {
    SPECIES_CRITICAL_LEVEL *criticalLevel = *pCriticalLevel;
    
    START_FUNCTION("FreeSpeciesCriticalLevel");
    
    if( criticalLevel == NULL ) {
        END_FUNCTION("FreeSpeciesCriticalLevel", SUCCESS );
        return SUCCESS;
    }
    
    FREE( criticalLevel->levels );    
    FREE( criticalLevel );
    
    END_FUNCTION("FreeSpeciesCriticalLevel", SUCCESS );
    return SUCCESS;
}

