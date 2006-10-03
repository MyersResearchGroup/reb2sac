/***************************************************************************
 *   Copyright (C) 2004 by Hiroyuki Kuwahara                               *
 *   kuwahara@cs.utah.edu                                                 *
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
#if !defined(HAVE_TS_SPECIES_LEVEL_UPDATER)
#define HAVE_TS_SPECIES_LEVEL_UPDATER

#include "common.h"
#include "compiler_def.h"
#include "species_node.h"

#define TIME_SERIES_SPECIES_LEVEL_UPDATER_KEY "simulation.time.series.species.level.updater"

#define NEXT_SPECIES_LEVEL_UPDATE_INFINITE_TIME (1.0/0.0)

BEGIN_C_NAMESPACE

struct _TIME_SERIES_SPECIES_LEVEL_UPDATER;
typedef struct _TIME_SERIES_SPECIES_LEVEL_UPDATER TIME_SERIES_SPECIES_LEVEL_UPDATER;


struct _TIME_SERIES_SPECIES_LEVEL_UPDATER {
    RET_VAL (*Initialize)( TIME_SERIES_SPECIES_LEVEL_UPDATER *updater );
    double (*GetNextUpdateTime)( TIME_SERIES_SPECIES_LEVEL_UPDATER *updater );    
    RET_VAL (*Update)( TIME_SERIES_SPECIES_LEVEL_UPDATER *updater, double currentTime );    
    RET_VAL (*ReleaseResource)( TIME_SERIES_SPECIES_LEVEL_UPDATER *updater );    
    
    SPECIES **speciesArray;
    int speciesSize;
    REB2SAC_PROPERTIES *prop;
};


DLLSCOPE TIME_SERIES_SPECIES_LEVEL_UPDATER * STDCALL CreateTimeSeriesSpeciesLevelUpdater( 
        SPECIES **speciesArray, 
        int speciesSize, 
        REB2SAC_PROPERTIES *properties );
        
DLLSCOPE RET_VAL STDCALL FreeTimeSeriesSpeciesLevelUpdater( TIME_SERIES_SPECIES_LEVEL_UPDATER **pUpdater );


END_C_NAMESPACE

#endif
