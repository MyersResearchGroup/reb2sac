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
#if !defined(HAVE_CONFIDENCE_INTERVAL_STOP_RULE)
#define HAVE_CONFIDENCE_INTERVAL_STOP_RULE

#include "common.h"

BEGIN_C_NAMESPACE


struct _CONFIDENCE_INTERVAL_STOP_RULE;
typedef struct _CONFIDENCE_INTERVAL_STOP_RULE CONFIDENCE_INTERVAL_STOP_RULE;

struct _CONFIDENCE_INTERVAL_STOP_RULE {
    double confidenceLevel;
    double tolerance;
    double sampleMean;
    double sampleVariance;
    double zValue;
    int sampleCount;
    int batchSize;
    
    BOOL (*IsConditionMet)( CONFIDENCE_INTERVAL_STOP_RULE *stopRule );
    RET_VAL (*Report)( CONFIDENCE_INTERVAL_STOP_RULE *stopRule, FILE *file );    
    RET_VAL (*AddNewSample)( CONFIDENCE_INTERVAL_STOP_RULE *stopRule, double sample );    
    double (*GetConfidenceLevel)( CONFIDENCE_INTERVAL_STOP_RULE *stopRule );
    double (*GetTolerance)( CONFIDENCE_INTERVAL_STOP_RULE *stopRule );
    double (*GetSampleMean)( CONFIDENCE_INTERVAL_STOP_RULE *stopRule );
    double (*GetSampleVariance)( CONFIDENCE_INTERVAL_STOP_RULE *stopRule );
    double (*GetSampleCount)( CONFIDENCE_INTERVAL_STOP_RULE *stopRule );
    double (*GetBatchSize)( CONFIDENCE_INTERVAL_STOP_RULE *stopRule );        
};

CONFIDENCE_INTERVAL_STOP_RULE *CreateConfidenceIntervalStopRule( double confidenceLevel, double tolerance, int batchSize );
RET_VAL FreeConfidenceIntervalStopRule( CONFIDENCE_INTERVAL_STOP_RULE **stopRule );


END_C_NAMESPACE

#endif
