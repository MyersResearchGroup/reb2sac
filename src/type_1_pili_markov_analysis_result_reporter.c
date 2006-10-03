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
#include "type_1_pili_markov_analysis_result_reporter.h"
#include "ir2ctmc_transformer.h"
#include "species_critical_level.h"
#include "ctmc_analyzer.h"


static int _FindSwitchIndex( SPECIES_CRITICAL_LEVEL **criticalLevelArray, int size );
static double _GetStateProb( CTMC_STATE *state );

RET_VAL ReportType1PiliAnalysisResult( MARKOV_ANALYSIS_RESULT_REPORTER *reporter, 
                                       FILE *file,
                                       MARKOV_CHAIN *markovChain, 
                                       CADDR_T extra ) 
{
    RET_VAL ret = SUCCESS;
    CTMC *ctmc = (CTMC*)markovChain;
    IR2CTMC_TRANSFORMER *transformer = (IR2CTMC_TRANSFORMER*)extra;
    int i = 0;
    int stateSize = ctmc->stateSize;
    int switchIndex = 0;
    int criticalLevelArraySize = 0;
    double prob0 = 0.0;
    double prob1 = 0.0;
    double level0 = 0.0;
    double *currentLevelArray = NULL;
    SPECIES *species = NULL;
    SPECIES_CRITICAL_LEVEL *criticalLevel = NULL;
    SPECIES_CRITICAL_LEVEL **criticalLevelArray = NULL;
    CTMC_STATE *state = NULL;
    CTMC_STATE *states = ctmc->states;
    
    criticalLevelArraySize = transformer->GetCriticalLevelArraySize( transformer );
    criticalLevelArray = transformer->GetCriticalLevelArray( transformer );
        
    switchIndex = _FindSwitchIndex( criticalLevelArray, criticalLevelArraySize );
    if( switchIndex < 0 ) {
        return ErrorReport( FAILING, "ReportType1PiliAnalysisResult", "no switch species in the model" );
    }
    
    criticalLevel = criticalLevelArray[switchIndex];
    if( criticalLevel->size != 2 ) {
        return ErrorReport( FAILING, "ReportType1PiliAnalysisResult", "switch must have exactly 2 levels (off and on)" );
    }
    
    level0 = criticalLevel->levels[0];
    
    currentLevelArray = transformer->GetCurrentLevelArray( transformer );    
    if( IS_FAILED( ( ret = transformer->ResetCurrentLevelArray( transformer ) ) ) ) {
        return ret;
    }
    state = states + 0;    
    if( IS_REAL_EQUAL( currentLevelArray[switchIndex], level0 ) ) {
        prob0 += _GetStateProb( state );
    }
    else {
        prob1 += _GetStateProb( state );
    }    
    for( i = 1; i < stateSize; i++ ){        
        transformer->IncrementCurrentLevelArray( transformer );
        state = states + i;
        if( IS_REAL_EQUAL( currentLevelArray[switchIndex], level0 ) ) {
            prob0 += _GetStateProb( state );
        }
        else {
            prob1 += _GetStateProb( state );
        }    
    }
    fprintf( file, "# switch-0-probability switch-1-probability" NEW_LINE );
    fprintf( file, "%g %g" NEW_LINE, prob0, prob1 );
    
    return SUCCESS;    
}

RET_VAL ReleaseType1PiliAnalysisResultResource( MARKOV_ANALYSIS_RESULT_REPORTER *reporter ) {
    return SUCCESS;    
}

static int _FindSwitchIndex( SPECIES_CRITICAL_LEVEL **criticalLevelArray, int size ) {
    int i = 0;
    char *nameString = NULL;
    SPECIES *species = NULL;
    SPECIES_CRITICAL_LEVEL *criticalLevel = NULL;
    
    for( ; i < size; i++ ) {
        criticalLevel = criticalLevelArray[i];
        species = criticalLevel->species;
        nameString = GetCharArrayOfString( GetSpeciesNodeName( species ) );
        if( strcmp( nameString, "switch" ) == 0 ) {
            return i;
        }                    
    }
    return -1;
}

static double _GetStateProb( CTMC_STATE *state ) {
    CTMC_ANALYSIS_REC *rec = (CTMC_ANALYSIS_REC*)(state->analysisRecord);
    return rec->currentProb;    
}

