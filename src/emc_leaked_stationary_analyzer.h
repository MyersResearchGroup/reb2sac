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
#if !defined(HAVE_EMC_LEAKED_STATIONARY_ANALYZER)
#define HAVE_EMC_LEAKED_STATIONARY_ANALYZER


#include "common.h"
#include "markov_chain.h"

BEGIN_C_NAMESPACE

#define DEFAULT_EMC_LEAKED_STATIONARY_ANALYSIS_TOLERANCE 0.0001

typedef struct {
    double currentProb;
    double newProb;
    double leakProb;
    double currentLeakedProb;
    double newLeakedProb;
} EMC_LEAKED_STATIONARY_ANALYSIS_REC; 

struct _EMC_LEAKED_STATIONARY_ANALYZER;
typedef struct _EMC_LEAKED_STATIONARY_ANALYZER EMC_LEAKED_STATIONARY_ANALYZER;

struct _EMC_LEAKED_STATIONARY_ANALYZER {
    EMC *markovChain;
    double tolerance;
    double leakRate;
    RET_VAL (*SetTolerance)( EMC_LEAKED_STATIONARY_ANALYZER *analyzer, double tolerance );
    double (*GetTolerance)( EMC_LEAKED_STATIONARY_ANALYZER *analyzer );
    RET_VAL (*Analyze)( EMC_LEAKED_STATIONARY_ANALYZER *analyzer, int maxIteration );
    EMC *(*GetResult)( EMC_LEAKED_STATIONARY_ANALYZER *analyzer );
};


EMC_LEAKED_STATIONARY_ANALYZER *CreateEMCLeakedStationaryAnalyzer( CTMC *ctmc, double leakRate );
RET_VAL FreeEMCLeakedStationaryAnalyzer( EMC_LEAKED_STATIONARY_ANALYZER **panalyzer );


END_C_NAMESPACE

#endif
