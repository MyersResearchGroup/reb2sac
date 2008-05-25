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
#include "abstraction_reporter.h"


static RET_VAL STDCALL _DefaultReportInitial( ABSTRACTION_REPORTER *reporter, IR *ir );
static RET_VAL STDCALL _DefaultReport( ABSTRACTION_REPORTER *reporter, char *abstractionName, IR *ir );
static RET_VAL STDCALL _DefaultReportFinal( ABSTRACTION_REPORTER *reporter, IR *ir );
static RET_VAL STDCALL _DefaultReleaseResource( ABSTRACTION_REPORTER *reporter  );
static RET_VAL STDCALL __DefaultReport( ABSTRACTION_REPORTER *reporter, char *name, IR *ir );


DLLSCOPE ABSTRACTION_REPORTER * STDCALL CreateAbstractionReporter( char *type ) {
    static ABSTRACTION_REPORTER reporter;
    
    START_FUNCTION("CreateAbstractionReporter");
    if( reporter.ReportInitial != NULL ) {
        END_FUNCTION("CreateAbstractionReporter", SUCCESS );
        return &reporter;
    }
    
    if( type == NULL || strcmp( type, "default" ) == 0 ) {
        if( ( reporter.out = fopen( DEFAULT_ABSTRACTION_REPORTER_FILENAME, "w" ) ) == NULL ) {
            TRACE_0( "could not open a default report file");
            END_FUNCTION("CreateAbstractionReporter", FAILING );
            return NULL;
        } 
        reporter.ReportInitial = _DefaultReportInitial;
        reporter.Report = _DefaultReport;
        reporter.ReportFinal = _DefaultReportFinal;
        reporter.ReleaseResource = _DefaultReleaseResource;
    } 
    
    return &reporter;
}

DLLSCOPE RET_VAL STDCALL CloseAbstractionReporter(  ABSTRACTION_REPORTER **rep ) {
    RET_VAL ret = SUCCESS;
    ABSTRACTION_REPORTER *reporter = NULL;
    
    START_FUNCTION("CloseAbstractionReporter");
    
    reporter = *rep;
    if( reporter == NULL ) {
        END_FUNCTION("CloseAbstractionReporter", SUCCESS );
        return ret;
    }
    
    if( IS_FAILED( ( ret = reporter->ReleaseResource( reporter ) ) ) ) {
        END_FUNCTION("CloseAbstractionReporter", ret );
        return ret;
    }
    *rep = NULL;
    END_FUNCTION("CloseAbstractionReporter", SUCCESS );
    return ret;
}

 

static RET_VAL STDCALL _DefaultReportInitial( ABSTRACTION_REPORTER *reporter, IR *ir ) {
    RET_VAL ret = SUCCESS;

    START_FUNCTION("_DefaultReportInitial");
    
    fprintf( reporter->out, "Condition, species size, reaction size" NEW_LINE );
    __DefaultReport( reporter, "initial", ir ); 
    
    END_FUNCTION("_DefaultReportInitial", SUCCESS );
    return ret;
}


static RET_VAL STDCALL _DefaultReport( ABSTRACTION_REPORTER *reporter, char *abstractionName, IR *ir ) {
    RET_VAL ret = SUCCESS;

    START_FUNCTION("_DefaultReport");
    
    __DefaultReport( reporter, abstractionName, ir ); 
    
    END_FUNCTION("_DefaultReport", SUCCESS );
    return ret;
}

static RET_VAL STDCALL _DefaultReportFinal( ABSTRACTION_REPORTER *reporter, IR *ir ) {
    RET_VAL ret = SUCCESS;

    START_FUNCTION("_DefaultReportFinal");
    
    __DefaultReport( reporter, "final", ir ); 
    
    END_FUNCTION("_DefaultReportFinal", SUCCESS );
    return ret;
}

static RET_VAL STDCALL __DefaultReport( ABSTRACTION_REPORTER *reporter, char *name, IR *ir ) {
    RET_VAL ret = SUCCESS;
    LINKED_LIST *list = NULL;
    int speciesSize = 0;    
    int reactionSize = 0;    

    START_FUNCTION("__DefaultReport");
    
    list = ir->GetListOfSpeciesNodes( ir );
    speciesSize = GetLinkedListSize( list );
    list = ir->GetListOfReactionNodes( ir );
    reactionSize = GetLinkedListSize( list );
    
    fprintf( reporter->out, "%s, %i,  %i" NEW_LINE, name, speciesSize, reactionSize ); 
    fflush( reporter->out );
    
    END_FUNCTION("__DefaultReport", SUCCESS );
    return ret;
}


static RET_VAL STDCALL _DefaultReleaseResource( ABSTRACTION_REPORTER *reporter  ) {
    RET_VAL ret = SUCCESS;

    START_FUNCTION("_DefaultReleaseResource");
    
    fclose( reporter->out );
    
    END_FUNCTION("_DefaultReleaseResource", SUCCESS );
    return ret;
}
