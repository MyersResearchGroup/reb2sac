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
#if !defined(HAVE_DEFAULT_TS_SPECIES_LEVEL_UPDATER)
#define HAVE_DEFAULT_TS_SPECIES_LEVEL_UPDATER

#include "linked_list.h"
#include "ts_species_level_updater.h"

#define TIME_SERIES_SPECIES_LEVEL_FILE_KEY "simulation.time.series.species.level.file"


BEGIN_C_NAMESPACE

typedef struct {
    double time;
    int speciesIndex;
    char updateType;
    double amount;
} DEFAULT_TS_SPECIES_LEVEL_UPDATER_RECORD_ENTRY;

#define TIME_SERIES_SPECIES_LEVEL_UPDATE_TYPE_PLUS '+'
#define TIME_SERIES_SPECIES_LEVEL_UPDATE_TYPE_MINUS '-'
#define TIME_SERIES_SPECIES_LEVEL_UPDATE_TYPE_TIMES '*'
#define TIME_SERIES_SPECIES_LEVEL_UPDATE_TYPE_DIV '/'
#define TIME_SERIES_SPECIES_LEVEL_UPDATE_TYPE_ASSIGN '='

#define TIME_SERIES_SPECIES_LEVEL_UPDATE_SPECIES_NAME_MAX 2048

struct _DEFAULT_TIME_SERIES_SPECIES_LEVEL_UPDATER;
typedef struct _DEFAULT_TIME_SERIES_SPECIES_LEVEL_UPDATER DEFAULT_TIME_SERIES_SPECIES_LEVEL_UPDATER;


struct _DEFAULT_TIME_SERIES_SPECIES_LEVEL_UPDATER {
    RET_VAL (*Initialize)( TIME_SERIES_SPECIES_LEVEL_UPDATER *updater );
    double (*GetNextUpdateTime)( TIME_SERIES_SPECIES_LEVEL_UPDATER *updater );    
    RET_VAL (*Update)( TIME_SERIES_SPECIES_LEVEL_UPDATER *updater, double currentTime );    
    RET_VAL (*ReleaseResource)( TIME_SERIES_SPECIES_LEVEL_UPDATER *updater );    
    
    SPECIES **speciesArray;
    int speciesSize;
    double nextUpdateTime;
    REB2SAC_PROPERTIES *prop;
    LINKED_LIST *entries;
    DEFAULT_TS_SPECIES_LEVEL_UPDATER_RECORD_ENTRY *currentEntry;
    char *lastSpeciesName;
    int lastSpeciesIndex;
    FILE *file;
};


DLLSCOPE TIME_SERIES_SPECIES_LEVEL_UPDATER * STDCALL CreateDefaultTimeSeriesSpeciesLevelUpdater( 
        SPECIES **speciesArray, 
        int speciesSize, 
        REB2SAC_PROPERTIES *properties );
        

END_C_NAMESPACE

#endif
