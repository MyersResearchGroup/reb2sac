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
#if !defined(HAVE_MARKOV_CHAIN_ANALYSIS_PROPERTIES)
#define HAVE_MARKOV_CHAIN_ANALYSIS_PROPERTIES

#include "common.h"

BEGIN_C_NAMESPACE

#define MARKOV_CHAIN_ANALYSIS_TIME_LIMIT_KEY "markov.chain.time.limit"
#define DEFAULT_MARKOV_CHAIN_ANALYSIS_TIME_LIMIT 0.01

#define DEFAULT_CTMC_ANALYSIS_OUTPUT_NAME "markov-transient-analysis.txt"

#define MARKOV_ANALYSIS_RESULT_REPORTER_KEY "markov.analysis.result.reporter"

END_C_NAMESPACE

#endif
