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
#include "default_ts_species_level_updater.h"
#include "reaction_node.h" 
 
static RET_VAL _Initialize( DEFAULT_TIME_SERIES_SPECIES_LEVEL_UPDATER *updater );
static double _GetNextUpdateTime( DEFAULT_TIME_SERIES_SPECIES_LEVEL_UPDATER *updater );    
static RET_VAL _Update( DEFAULT_TIME_SERIES_SPECIES_LEVEL_UPDATER *updater, double currentTime );    
static RET_VAL _UpdateSpeciesValues( SPECIES *species, char type, double updateAmount );
static RET_VAL _UpdateReactionRateUpdateTime( SPECIES *species, double time );
static RET_VAL _ReleaseResource( DEFAULT_TIME_SERIES_SPECIES_LEVEL_UPDATER *updater );    


static RET_VAL _SetDummyMethods( DEFAULT_TIME_SERIES_SPECIES_LEVEL_UPDATER *updater );
static RET_VAL _Initialize2( DEFAULT_TIME_SERIES_SPECIES_LEVEL_UPDATER *updater );
static double _GetNextUpdateTime2( DEFAULT_TIME_SERIES_SPECIES_LEVEL_UPDATER *updater );    
static RET_VAL _Update2( DEFAULT_TIME_SERIES_SPECIES_LEVEL_UPDATER *updater, double currentTime );    
static RET_VAL _ReleaseResource2( DEFAULT_TIME_SERIES_SPECIES_LEVEL_UPDATER *updater );    

static int _FindSpeciesIndex( char *speciesName, DEFAULT_TIME_SERIES_SPECIES_LEVEL_UPDATER *updater );    
static RET_VAL _CreateEntries( DEFAULT_TIME_SERIES_SPECIES_LEVEL_UPDATER *updater );    
static DEFAULT_TS_SPECIES_LEVEL_UPDATER_RECORD_ENTRY *_CreateEntry( 
        double time, int speciesIndex, char type, double amount );    
 
 
DLLSCOPE TIME_SERIES_SPECIES_LEVEL_UPDATER * STDCALL CreateDefaultTimeSeriesSpeciesLevelUpdater( 
        SPECIES **speciesArray, 
        int speciesSize, 
        REB2SAC_PROPERTIES *properties )
{    
    DEFAULT_TIME_SERIES_SPECIES_LEVEL_UPDATER *updater = NULL;
    char *filename = NULL;
    FILE *file = NULL;
    
    START_FUNCTION("CreateDefaultTimeSeriesSpeciesLevelUpdater");
    
    updater = (DEFAULT_TIME_SERIES_SPECIES_LEVEL_UPDATER*)MALLOC( sizeof(DEFAULT_TIME_SERIES_SPECIES_LEVEL_UPDATER) );
    if( updater == NULL ) {
        END_FUNCTION("CreateDefaultTimeSeriesSpeciesLevelUpdater", FAILING );
        return NULL;    
    }
        
    updater->file = NULL;
    updater->entries = NULL;
    if( ( filename = properties->GetProperty( properties, TIME_SERIES_SPECIES_LEVEL_FILE_KEY ) ) == NULL ) {                
        TRACE_1( "%s is not a valid file name", filename );
        _SetDummyMethods( updater );
        END_FUNCTION("CreateDefaultTimeSeriesSpeciesLevelUpdater", SUCCESS );
        return (TIME_SERIES_SPECIES_LEVEL_UPDATER*)updater;
    }
    if( ( file = fopen( filename, "r" ) ) == NULL ) {
        TRACE_1( "%s is not readable", filename );
        _SetDummyMethods( updater );
        END_FUNCTION("CreateDefaultTimeSeriesSpeciesLevelUpdater", SUCCESS );
        return (TIME_SERIES_SPECIES_LEVEL_UPDATER*)updater;
    }
    
    updater->Initialize = (RET_VAL(*)(TIME_SERIES_SPECIES_LEVEL_UPDATER *))_Initialize;
    updater->GetNextUpdateTime = (double(*)(TIME_SERIES_SPECIES_LEVEL_UPDATER *))_GetNextUpdateTime;
    updater->Update = (RET_VAL(*)(TIME_SERIES_SPECIES_LEVEL_UPDATER *))_Update;
    updater->ReleaseResource = (RET_VAL(*)(TIME_SERIES_SPECIES_LEVEL_UPDATER *))_ReleaseResource;
    
    updater->speciesArray = speciesArray;
    updater->speciesSize = speciesSize;
    updater->file = file;
    updater->lastSpeciesIndex = speciesSize - 1;
    updater->lastSpeciesName = GetSpeciesNodeName( speciesArray[updater->lastSpeciesIndex] );
    
    if( IS_FAILED( ( _CreateEntries( updater ) ) ) ) {
        END_FUNCTION("CreateDefaultTimeSeriesSpeciesLevelUpdater", FAILING );
        return NULL;
    }
    
    
    if( IS_FAILED( ( updater->Initialize( updater ) ) ) ) {
        END_FUNCTION("CreateDefaultTimeSeriesSpeciesLevelUpdater", FAILING );
        return NULL;
    }
    
    END_FUNCTION("CreateDefaultTimeSeriesSpeciesLevelUpdater", SUCCESS );
    return (TIME_SERIES_SPECIES_LEVEL_UPDATER*)updater;
}
        


static RET_VAL _Initialize( DEFAULT_TIME_SERIES_SPECIES_LEVEL_UPDATER *updater ) {
    RET_VAL ret = SUCCESS;
    DEFAULT_TS_SPECIES_LEVEL_UPDATER_RECORD_ENTRY *entry = NULL;
    LINKED_LIST *entries = updater->entries;
    
    ResetCurrentElement( entries );
    entry = (DEFAULT_TS_SPECIES_LEVEL_UPDATER_RECORD_ENTRY*)GetNextFromLinkedList( entries );
    if( entry == NULL ) {
        updater->currentEntry = NULL;
        updater->nextUpdateTime = NEXT_SPECIES_LEVEL_UPDATE_INFINITE_TIME;
    }
    else {
        updater->currentEntry = entry;        
        updater->nextUpdateTime = entry->time;
    } 

    return ret;
}


static double _GetNextUpdateTime( DEFAULT_TIME_SERIES_SPECIES_LEVEL_UPDATER *updater ) {
    return updater->nextUpdateTime;
}
 
static RET_VAL _Update( DEFAULT_TIME_SERIES_SPECIES_LEVEL_UPDATER *updater, double currentTime ) {
    RET_VAL ret = SUCCESS;
    DEFAULT_TS_SPECIES_LEVEL_UPDATER_RECORD_ENTRY *currentEntry = updater->currentEntry;
    LINKED_LIST *entries = updater->entries;    
    SPECIES *species = NULL;
    char type = 0;
    double updateTime = updater->nextUpdateTime;
    double amount = 0.0;        
        
    if( currentEntry == NULL ) {
        return ret;
    }
    

    while( IS_REAL_EQUAL( currentTime, updateTime ) ) {
        species = (updater->speciesArray)[currentEntry->speciesIndex];
        type = currentEntry->updateType;
        updateTime = currentEntry->time;
        amount = currentEntry->amount;
        
        if( IS_FAILED( ( ret = _UpdateSpeciesValues( species, type, amount ) ) ) ) {
            return ret;
        }
        if( IS_FAILED( ( ret = _UpdateReactionRateUpdateTime( species, updateTime ) ) ) ) {
            return ret;
        }
        currentEntry = (DEFAULT_TS_SPECIES_LEVEL_UPDATER_RECORD_ENTRY*)GetNextFromLinkedList( entries );
        if( currentEntry == NULL ) {
            currentEntry = NULL;
            updateTime = NEXT_SPECIES_LEVEL_UPDATE_INFINITE_TIME;
        }
        else {
            updateTime = currentEntry->time;
        } 
    }
    updater->currentEntry = currentEntry;        
    updater->nextUpdateTime = updateTime;
    
    return SUCCESS;
}

static RET_VAL _UpdateSpeciesValues( SPECIES *species, char type, double updateAmount ) {
    RET_VAL ret = SUCCESS;
    double amount = 0;    
    
    amount = GetAmountInSpeciesNode( species );
    switch( type ){
        case TIME_SERIES_SPECIES_LEVEL_UPDATE_TYPE_PLUS:
            SetAmountInSpeciesNode( species, amount + updateAmount );
        break;
            
        case TIME_SERIES_SPECIES_LEVEL_UPDATE_TYPE_MINUS:
            if( amount < updateAmount ) {
                SetAmountInSpeciesNode( species, 0.0 );
            }
            else {
                SetAmountInSpeciesNode( species, amount - updateAmount );
            }
        break;
        
        case TIME_SERIES_SPECIES_LEVEL_UPDATE_TYPE_TIMES:
            SetAmountInSpeciesNode( species, amount * updateAmount );
        break;
        
        case TIME_SERIES_SPECIES_LEVEL_UPDATE_TYPE_DIV:
            amount = amount / updateAmount;
            SetAmountInSpeciesNode( species, (double)((long)amount) );
        break;
        
        case TIME_SERIES_SPECIES_LEVEL_UPDATE_TYPE_ASSIGN:
            SetAmountInSpeciesNode( species, updateAmount );
        break;
    }    
    
    TRACE_2( "the amount of %s modified to %g", 
                 GetCharArrayOfString( GetSpeciesNodeName( species ) ), 
                 GetAmountInSpeciesNode( species ) );
    return ret;            
}


static RET_VAL _UpdateReactionRateUpdateTime( SPECIES *species, double time ) {
    RET_VAL ret = SUCCESS;
    IR_EDGE *edge = NULL;
    IR_EDGE *updateEdge = NULL;
    REACTION *reaction = NULL;
    LINKED_LIST *edges = NULL;
    LINKED_LIST *updateEdges = NULL;

    updateEdges = GetReactantEdges( (IR_NODE*)species );
    ResetCurrentElement( updateEdges );
    while( ( updateEdge = GetNextEdge( updateEdges ) ) != NULL ) {
      reaction = (REACTION*)GetReactionInIREdge( updateEdge );
        if( IS_FAILED( ( ret = SetReactionRateUpdatedTime( reaction, time ) ) ) ) {
            return ret;                
        }
    }                
    
    updateEdges = GetModifierEdges( (IR_NODE*)species );
    ResetCurrentElement( updateEdges );
    while( ( updateEdge = GetNextEdge( updateEdges ) ) != NULL ) {
        reaction = (REACTION*)GetReactionInIREdge( updateEdge );
        if( IS_FAILED( ( ret = SetReactionRateUpdatedTime( reaction, time ) ) ) ) {
            return ret;                
        }
    }                
    
    updateEdges = GetProductEdges( (IR_NODE*)species );
    ResetCurrentElement( updateEdges );
    while( ( updateEdge = GetNextEdge( updateEdges ) ) != NULL ) {
        reaction = (REACTION*)GetReactionInIREdge( updateEdge );
        if( IS_FAILED( ( ret = SetReactionRateUpdatedTime( reaction, time ) ) ) ) {
            return ret;                
        }
    }                
        
    return ret;            
}



static RET_VAL _ReleaseResource( DEFAULT_TIME_SERIES_SPECIES_LEVEL_UPDATER *updater ) {
    RET_VAL ret = SUCCESS;
    LINKED_LIST *entries = updater->entries;    
    DEFAULT_TS_SPECIES_LEVEL_UPDATER_RECORD_ENTRY *entry = NULL;
    
    ResetCurrentElement( entries );
    while( ( entry = (DEFAULT_TS_SPECIES_LEVEL_UPDATER_RECORD_ENTRY*)GetNextFromLinkedList( entries ) ) != NULL ) {
        FREE( entry );
    }
    DeleteLinkedList( &entries );     

    return ret;
}  




static RET_VAL _SetDummyMethods( DEFAULT_TIME_SERIES_SPECIES_LEVEL_UPDATER *updater ) {
    RET_VAL ret = SUCCESS;
    
    updater->Initialize = (RET_VAL(*)(TIME_SERIES_SPECIES_LEVEL_UPDATER *))_Initialize2;
    updater->GetNextUpdateTime = (double(*)(TIME_SERIES_SPECIES_LEVEL_UPDATER *))_GetNextUpdateTime2;
    updater->Update = (RET_VAL(*)(TIME_SERIES_SPECIES_LEVEL_UPDATER *))_Update2;
    updater->ReleaseResource = (RET_VAL(*)(TIME_SERIES_SPECIES_LEVEL_UPDATER *))_ReleaseResource2;
    
    return ret;
}


static RET_VAL _Initialize2( DEFAULT_TIME_SERIES_SPECIES_LEVEL_UPDATER *updater ) {
    return SUCCESS;
}

static double _GetNextUpdateTime2( DEFAULT_TIME_SERIES_SPECIES_LEVEL_UPDATER *updater ) {
    return NEXT_SPECIES_LEVEL_UPDATE_INFINITE_TIME;
}

static RET_VAL _Update2( DEFAULT_TIME_SERIES_SPECIES_LEVEL_UPDATER *updater, double currentTime ) {
    return SUCCESS;
}

static RET_VAL _ReleaseResource2( DEFAULT_TIME_SERIES_SPECIES_LEVEL_UPDATER *updater ) {
    return SUCCESS;
}




static RET_VAL _CreateEntries( DEFAULT_TIME_SERIES_SPECIES_LEVEL_UPDATER *updater ) {
    RET_VAL ret = SUCCESS;
    FILE *file = updater->file;
    int num = 0;
    double time = 0.0;
    char speciesName[TIME_SERIES_SPECIES_LEVEL_UPDATE_SPECIES_NAME_MAX];
    char buf[4096];
    int speciesIndex = 0;
    char type = 0;
    double amount = 0.0;
    LINKED_LIST *entries = NULL;
    DEFAULT_TS_SPECIES_LEVEL_UPDATER_RECORD_ENTRY *entry = NULL;
    
    if( ( entries = CreateLinkedList() ) == NULL ) {
        return ErrorReport( FAILING, "_CreateEntries", "could not create a linked list for user defined update list" );
    }
    
    while( fgets( buf, sizeof(buf), file ) != NULL ) {
        num = sscanf( buf, "%lf %s %[=+-*/]%lf", &time, speciesName, &type, &amount );
        if( num != 4 ) {
            continue;
        }
        if( ( speciesIndex = _FindSpeciesIndex( speciesName, updater ) ) < 0 ) {
            continue;
        }
        if( ( entry = _CreateEntry( time, speciesIndex, type, amount ) ) == NULL ) {
            return ErrorReport( FAILING, "_CreateEntries", "could not create an entry for %g %s %c%g",
                              time, speciesName, type, amount );
        }
        
        if( IS_FAILED( ( ret = AddElementInLinkedList( (CADDR_T)entry, entries ) ) ) ) {
            return ret;
        }
    }
    
    updater->entries = entries;
    
    return SUCCESS;
}


static int _FindSpeciesIndex( char *speciesName, DEFAULT_TIME_SERIES_SPECIES_LEVEL_UPDATER *updater ) {
    int i = 0;
    SPECIES *species = NULL;
    char *name = NULL;
    SPECIES **speciesArray = updater->speciesArray;
    int speciesSize = updater->speciesSize;
    char *lastSpeciesName = updater->lastSpeciesName;
    
    if( strcmp( speciesName, lastSpeciesName ) == 0 ) {
        return updater->lastSpeciesIndex;
    }
    
    for( i = 0; i < speciesSize; i++ ) {
        species = speciesArray[i];
        name = GetCharArrayOfString( GetSpeciesNodeName( species ) );
        if( strcmp( speciesName, name ) == 0 ) {
            updater->lastSpeciesName = name;
            updater->lastSpeciesIndex = i;
            return i;
        } 
    }
    
    return -1;    
}


static DEFAULT_TS_SPECIES_LEVEL_UPDATER_RECORD_ENTRY *_CreateEntry( 
        double time, int speciesIndex, 
        char type, double amount ) 
{    
    DEFAULT_TS_SPECIES_LEVEL_UPDATER_RECORD_ENTRY *entry = NULL;
    
    entry = (DEFAULT_TS_SPECIES_LEVEL_UPDATER_RECORD_ENTRY*)MALLOC( sizeof(DEFAULT_TS_SPECIES_LEVEL_UPDATER_RECORD_ENTRY) );
    if( entry == NULL ) {
        return NULL;
    }
    
    TRACE_4( "Creating an entry: %g %i %c%g" NEW_LINE, time, speciesIndex, type, amount );
            
    entry->time = time;
    entry->speciesIndex = speciesIndex;
    entry->updateType = type;
    entry->amount = amount;
    
    return entry;
}


