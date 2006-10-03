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

#if !defined(HAVE_MARKOV_ANALYSIS_RESULT_REPORTER)
#define HAVE_MARKOV_ANALYSIS_RESULT_REPORTER

#include "compiler_def.h"
#include "markov_chain.h"
#include "markov_chain_analysis_properties.h"

BEGIN_C_NAMESPACE

struct _MARKOV_ANALYSIS_RESULT_REPORTER;
typedef struct _MARKOV_ANALYSIS_RESULT_REPORTER MARKOV_ANALYSIS_RESULT_REPORTER;

struct _MARKOV_ANALYSIS_RESULT_REPORTER {
    CADDR_T _internal1;
    CADDR_T _internal2;
    CADDR_T _internal3;
    REB2SAC_PROPERTIES *properties;
    
    RET_VAL (*Report)( MARKOV_ANALYSIS_RESULT_REPORTER *reporter, 
                       FILE *file, 
                       MARKOV_CHAIN *markovChain, 
                       CADDR_T extra );        
    RET_VAL (*ReleaseResource)( MARKOV_ANALYSIS_RESULT_REPORTER *reporter );        
};

MARKOV_ANALYSIS_RESULT_REPORTER *CreateMarkovAnalysisResultReporter( REB2SAC_PROPERTIES *properties );
RET_VAL FreeMarkovAnalysisResultReporter( MARKOV_ANALYSIS_RESULT_REPORTER **pReporter );

END_C_NAMESPACE
     
#endif
