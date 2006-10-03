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
#if !defined(HAVE_CTMC_STATIONARY_ANALYSIS_BACK_END_PROCESSOR)
#define HAVE_CTMC_STATIONARY_ANALYSIS_BACK_END_PROCESSOR

#include "hse_back_end_processor_common.h"

BEGIN_C_NAMESPACE


RET_VAL ProcessCTMCStationaryAnalysisBackend( BACK_END_PROCESSOR *backend, IR *ir );
RET_VAL CloseCTMCStationaryAnalysisBackend( BACK_END_PROCESSOR *backend );

END_C_NAMESPACE
     
#endif
