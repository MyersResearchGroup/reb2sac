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
#if !defined(HAVE_CTMC_ANALYZER)
#define HAVE_CTMC_ANALYZER


#include "common.h"
#include "markov_chain.h"

BEGIN_C_NAMESPACE

#define CTMC_ANALYSIS_DT_RATE 0.1

typedef struct {
    double currentProb;
    double newProb;
    double outRate;
    double noTranProb;
} CTMC_ANALYSIS_REC; 

struct _CTMC_ANALYZER;
typedef struct _CTMC_ANALYZER CTMC_ANALYZER;

struct _CTMC_ANALYZER {
    CTMC *markovChain;
    double dt;
    double highestOutRate;
    double (*GetDt)( CTMC_ANALYZER *analyzer );    
    RET_VAL (*ResetDt)( CTMC_ANALYZER *analyzer, double dt );
    RET_VAL (*Analyze)( CTMC_ANALYZER *analyzer, double timeLimit );
    CTMC *(*GetResult)( CTMC_ANALYZER *analyzer );
};


CTMC_ANALYZER *CreateCTMCAnalyzer( CTMC *markovChain );
RET_VAL FreeCTMCAnalyzer( CTMC_ANALYZER **panalyzer );


END_C_NAMESPACE

#endif
