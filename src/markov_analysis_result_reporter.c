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
#include "markov_analysis_result_reporter.h"
#include "type_1_pili_markov_analysis_result_reporter.h"

MARKOV_ANALYSIS_RESULT_REPORTER *CreateMarkovAnalysisResultReporter( REB2SAC_PROPERTIES *properties ) {
    MARKOV_ANALYSIS_RESULT_REPORTER *reporter = NULL;
    char *reporterName = NULL;
    
    START_FUNCTION("CreateMarkovAnalysisResultReporter");
    
    if( ( reporterName = properties->GetProperty( properties, MARKOV_ANALYSIS_RESULT_REPORTER_KEY ) ) == NULL ) {
        reporterName = "type.1.pili.transient";
    } 
    
    if( ( reporter = (MARKOV_ANALYSIS_RESULT_REPORTER*)MALLOC( sizeof(MARKOV_ANALYSIS_RESULT_REPORTER) ) ) == NULL ) {
        return NULL;
    }
        
    switch( reporterName[0] ) {
        case 't':
            if( strcmp( reporterName, "type.1.pili.transient" ) == 0 ) {                
                reporter->Report = ReportType1PiliAnalysisResult;
                reporter->ReleaseResource = ReleaseType1PiliAnalysisResultResource;
                
            }
            break;
                
        default:
            fprintf( stderr, "target encoding type %s is invalid", reporterName );
            END_FUNCTION("CreateMarkovAnalysisResultReporter", FAILING );
            return NULL; 
    }
    
    END_FUNCTION("CreateMarkovAnalysisResultReporter", SUCCESS );
    
    return reporter;

}

RET_VAL FreeMarkovAnalysisResultReporter( MARKOV_ANALYSIS_RESULT_REPORTER **pReporter ) {
    MARKOV_ANALYSIS_RESULT_REPORTER *reporter = *pReporter;
    
    reporter->ReleaseResource( reporter );
    FREE( reporter );
    
    return SUCCESS;
}
