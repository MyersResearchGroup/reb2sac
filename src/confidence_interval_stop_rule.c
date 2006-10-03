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
#include "confidence_interval_stop_rule.h"
#include "compiler_def.h"
#include <math.h>

static BOOL _IsConditionMet( CONFIDENCE_INTERVAL_STOP_RULE *stopRule );
static RET_VAL _AddNewSample( CONFIDENCE_INTERVAL_STOP_RULE *stopRule, double sample );    
static RET_VAL _Report( CONFIDENCE_INTERVAL_STOP_RULE *stopRule, FILE *file );    

static BOOL _IsConditionMetBinomial( CONFIDENCE_INTERVAL_STOP_RULE *stopRule );
static RET_VAL _AddNewSampleBinomial( CONFIDENCE_INTERVAL_STOP_RULE *stopRule, double sample );    
static RET_VAL _ReportBinomial( CONFIDENCE_INTERVAL_STOP_RULE *stopRule, FILE *file );    


static BOOL _IsConditionMet( CONFIDENCE_INTERVAL_STOP_RULE *stopRule );
static RET_VAL _Report( CONFIDENCE_INTERVAL_STOP_RULE *stopRule, FILE *file );    
static RET_VAL _AddNewSample( CONFIDENCE_INTERVAL_STOP_RULE *stopRule, double sample );    


static double _GetConfidenceLevel( CONFIDENCE_INTERVAL_STOP_RULE *stopRule );
static double _GetTolerance( CONFIDENCE_INTERVAL_STOP_RULE *stopRule );
static double _GetSampleMean( CONFIDENCE_INTERVAL_STOP_RULE *stopRule );
static double _GetSampleVariance( CONFIDENCE_INTERVAL_STOP_RULE *stopRule );
static double _GetSampleCount( CONFIDENCE_INTERVAL_STOP_RULE *stopRule );
static double _GetBatchSize( CONFIDENCE_INTERVAL_STOP_RULE *stopRule );        

static double _CalculateHalfInterval( CONFIDENCE_INTERVAL_STOP_RULE *stopRule );
static double _CalculateHalfIntervalBinomial( CONFIDENCE_INTERVAL_STOP_RULE *stopRule );


CONFIDENCE_INTERVAL_STOP_RULE *CreateConfidenceIntervalStopRule( double confidenceLevel, double tolerance, int batchSize ) {
    CONFIDENCE_INTERVAL_STOP_RULE *stopRule = NULL;

    START_FUNCTION("CreateConfidenceIntervalStopRule");
    
    if( ( stopRule = (CONFIDENCE_INTERVAL_STOP_RULE*)MALLOC( sizeof(CONFIDENCE_INTERVAL_STOP_RULE) ) ) == NULL ) {
        END_FUNCTION("CreateConfidenceIntervalStopRule", FAILING );    
        return NULL;
    }
    
    /*
    stopRule->IsConditionMet = _IsConditionMet;
    stopRule->AddNewSample = _AddNewSample;
    stopRule->Report = _Report;
    */
    stopRule->IsConditionMet = _IsConditionMetBinomial;
    stopRule->AddNewSample = _AddNewSampleBinomial;
    stopRule->Report = _ReportBinomial;
    
    stopRule->GetConfidenceLevel = _GetConfidenceLevel;
    stopRule->GetTolerance = _GetTolerance;
    stopRule->GetSampleMean = _GetSampleMean;
    stopRule->GetSampleVariance = _GetSampleVariance;
    stopRule->GetSampleCount = _GetSampleCount;
    stopRule->GetBatchSize = _GetBatchSize;
    
    if( IS_REAL_EQUAL( confidenceLevel, 0.95 ) ) {
        stopRule->confidenceLevel = confidenceLevel;
        stopRule->zValue = 1.96;
    } 
    else {
        /* for now just do 95% */
        stopRule->confidenceLevel = 0.95;
        stopRule->zValue = 1.96;
    }
    
    stopRule->tolerance = tolerance;
    stopRule->batchSize = batchSize;
    stopRule->sampleMean = 0.0;
    stopRule->sampleVariance = 0.0;
    stopRule->sampleCount = 0;
    
    END_FUNCTION("CreateConfidenceIntervalStopRule", SUCCESS );    
    return stopRule;
}

RET_VAL FreeConfidenceIntervalStopRule( CONFIDENCE_INTERVAL_STOP_RULE **stopRule ) {
    START_FUNCTION("FreeConfidenceIntervalStopRule");
    FREE( *stopRule );
    END_FUNCTION("FreeConfidenceIntervalStopRule", SUCCESS );    
    return SUCCESS;
}


static BOOL _IsConditionMet( CONFIDENCE_INTERVAL_STOP_RULE *stopRule ) {
    double halfInterval = 0.0;
    double expected = 0.0;
    double sampleMean = stopRule->sampleMean;
    int sampleCount = stopRule->sampleCount;
    
    if( ( sampleCount < 2 ) || ( sampleMean == 0.0 )  ) {
    /*    
    if( sampleCount < 2 ) {
    */
        return FALSE;
    }
    else { 
        halfInterval = _CalculateHalfInterval( stopRule );
        expected = stopRule->tolerance * sampleMean;        
        return ( ( halfInterval <= expected ) ? TRUE : FALSE );
    }
}


static RET_VAL _AddNewSample( CONFIDENCE_INTERVAL_STOP_RULE *stopRule, double sample ) {
    int sampleCount = stopRule->sampleCount + 1;
    double sampleMean;
    double oldMean;
    double sampleVariance;
    double delta; 
    
    
    sample = sample / stopRule->batchSize;    
    
    if( sampleCount < 2 ) {
        stopRule->sampleMean = sample;
        stopRule->sampleVariance = 0.0;
    }
    else {
        oldMean = stopRule->sampleMean;
        sampleMean = oldMean + ( ( sample - oldMean ) / sampleCount );        
        delta = sampleMean - oldMean;
        sampleVariance = stopRule->sampleVariance;
        sampleVariance = ( 1.0 - 1.0 / ( sampleCount - 1.0 ) ) * sampleVariance + sampleCount * delta * delta; 
        stopRule->sampleMean = sampleMean;
        stopRule->sampleVariance = sampleVariance;
    }
    stopRule->sampleCount = sampleCount;
    
    return SUCCESS;
}

static RET_VAL _Report( CONFIDENCE_INTERVAL_STOP_RULE *stopRule, FILE *file ) {
    static BOOL needHeader = TRUE;    
    double halfInterval = _CalculateHalfInterval( stopRule );
    
    if( needHeader ) {
        fprintf( file, "# %s %s %s %s %s" NEW_LINE,
                 "sample-count", 
                 "sample-mean", 
                 "half-interval", 
                 "confidence-level", 
                 "batch-size" );  
        needHeader = FALSE;
    }
    fprintf( file, "%i %g %g %g %i" NEW_LINE,
             stopRule->sampleCount, 
             stopRule->sampleMean, 
             halfInterval, 
             stopRule->confidenceLevel, 
             stopRule->batchSize );  
    fflush( file );
    return SUCCESS;
}



static BOOL _IsConditionMetBinomial( CONFIDENCE_INTERVAL_STOP_RULE *stopRule ) {
    double halfInterval = 0.0;
    double expected = 0.0;
    double sampleMean = stopRule->sampleMean;
    int sampleCount = stopRule->sampleCount;
    
    if( ( sampleCount < 2 ) || ( sampleMean == 0.0 )  ) {
        return FALSE;
    }
    else { 
        halfInterval = _CalculateHalfIntervalBinomial( stopRule );
        expected = stopRule->tolerance * sampleMean;        
        return ( ( halfInterval <= expected ) ? TRUE : FALSE );
    }

}

static RET_VAL _AddNewSampleBinomial( CONFIDENCE_INTERVAL_STOP_RULE *stopRule, double sample ) {
    int sampleCount = stopRule->sampleCount + 1;
    double sampleMean;
    double oldMean;
    
    
    sample = sample / stopRule->batchSize;    
    
    if( sampleCount < 2 ) {
        stopRule->sampleMean = sample;
    }
    else {
        oldMean = stopRule->sampleMean;
        sampleMean = oldMean + ( ( sample - oldMean ) / sampleCount );        
        stopRule->sampleMean = sampleMean;
    }
    stopRule->sampleCount = sampleCount;
    
    return SUCCESS;
}

static RET_VAL _ReportBinomial( CONFIDENCE_INTERVAL_STOP_RULE *stopRule, FILE *file ) {
    static BOOL needHeader = TRUE;    
    double halfInterval = _CalculateHalfIntervalBinomial( stopRule );
    
    if( needHeader ) {
        fprintf( file, "# %s %s %s %s %s" NEW_LINE,
                 "sample-count", 
                 "sample-mean", 
                 "half-interval", 
                 "confidence-level", 
                 "batch-size" );  
        needHeader = FALSE;
    }
    fprintf( file, "%i %g %g %g %i" NEW_LINE,
             stopRule->sampleCount, 
             stopRule->sampleMean, 
             halfInterval, 
             stopRule->confidenceLevel, 
             stopRule->batchSize );  
    fflush( file );
    return SUCCESS;
}




static double _GetConfidenceLevel( CONFIDENCE_INTERVAL_STOP_RULE *stopRule ) {
    return stopRule->confidenceLevel;
}

static double _GetTolerance( CONFIDENCE_INTERVAL_STOP_RULE *stopRule ) {
    return stopRule->tolerance;
}

static double _GetSampleMean( CONFIDENCE_INTERVAL_STOP_RULE *stopRule ) {
    return stopRule->sampleMean;
}

static double _GetSampleVariance( CONFIDENCE_INTERVAL_STOP_RULE *stopRule ) {
    return stopRule->sampleVariance;
}

static double _GetSampleCount( CONFIDENCE_INTERVAL_STOP_RULE *stopRule ) {
    return stopRule->sampleCount;
}

static double _GetBatchSize( CONFIDENCE_INTERVAL_STOP_RULE *stopRule ) {
    return stopRule->batchSize;
}        

static double _CalculateHalfInterval( CONFIDENCE_INTERVAL_STOP_RULE *stopRule ) {
    double halfInterval = stopRule->zValue * sqrt( stopRule->sampleVariance / ( stopRule->sampleCount ) );
    return halfInterval; 
}

static double _CalculateHalfIntervalBinomial( CONFIDENCE_INTERVAL_STOP_RULE *stopRule ) {
    double sampleMean = stopRule->sampleMean;
    double halfInterval = stopRule->zValue * 
                sqrt( ( sampleMean * ( 1.0 - sampleMean ) ) / ( stopRule->sampleCount * stopRule->batchSize ) );
    return halfInterval; 
}

