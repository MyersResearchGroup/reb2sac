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
#include "ts_species_level_updater.h"
#include "default_ts_species_level_updater.h"
 
 
DLLSCOPE TIME_SERIES_SPECIES_LEVEL_UPDATER * STDCALL CreateTimeSeriesSpeciesLevelUpdater( 
        SPECIES **speciesArray, 
        int speciesSize, 
        REB2SAC_PROPERTIES *properties ) 
{
    TIME_SERIES_SPECIES_LEVEL_UPDATER *updater = NULL;
    char *valueString = NULL;

    START_FUNCTION("CreateTimeSeriesSpeciesLevelUpdater");

    if( ( valueString = properties->GetProperty( properties, TIME_SERIES_SPECIES_LEVEL_UPDATER_KEY ) ) == NULL ) {
        updater = CreateDefaultTimeSeriesSpeciesLevelUpdater( speciesArray, speciesSize, properties );
        
        END_FUNCTION("CreateTimeSeriesSpeciesLevelUpdater", SUCCESS );
        return updater;
    }
    
    switch( valueString[0] ) {
        default:
            updater = CreateDefaultTimeSeriesSpeciesLevelUpdater( speciesArray, speciesSize, properties );
        break;
    }        
    END_FUNCTION("CreateTimeSeriesSpeciesLevelUpdater", SUCCESS );
    return updater;
}
        
DLLSCOPE RET_VAL STDCALL FreeTimeSeriesSpeciesLevelUpdater( TIME_SERIES_SPECIES_LEVEL_UPDATER **pUpdater ) {
    RET_VAL ret = SUCCESS;
    TIME_SERIES_SPECIES_LEVEL_UPDATER *updater = *pUpdater;
    
    START_FUNCTION("FreeTimeSeriesSpeciesLevelUpdater");
    
    if( IS_FAILED( ( ret = updater->ReleaseResource( updater ) ) ) ) {
    }
    
    FREE( *pUpdater );
    
    END_FUNCTION("FreeTimeSeriesSpeciesLevelUpdater", SUCCESS );
    return SUCCESS;
}
 
