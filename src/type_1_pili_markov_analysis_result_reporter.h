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
#if !defined(HAVE_TYPE_1_PILI_MARKOV_ANALYSIS_RESULT_REPORTER)
#define HAVE_TYPE_1_PILI_MARKOV_ANALYSIS_RESULT_REPORTER

#include "markov_analysis_result_reporter.h"

BEGIN_C_NAMESPACE

RET_VAL ReportType1PiliAnalysisResult( MARKOV_ANALYSIS_RESULT_REPORTER *reporter,
                                       FILE *file,
                                       MARKOV_CHAIN *markovChain, 
                                       CADDR_T extra ); 
RET_VAL ReleaseType1PiliAnalysisResultResource( MARKOV_ANALYSIS_RESULT_REPORTER *reporter );        

END_C_NAMESPACE
     
#endif
