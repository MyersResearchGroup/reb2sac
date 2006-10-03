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
#if !defined(HAVE_NARY_LEVEL_BACK_END_PROCESS)
#define HAVE_NARY_LEVEL_BACK_END_PROCESS

#include "common.h"
#include "IR.h"
#include "kinetic_law.h"
#include "back_end_processor.h"
#include "law_of_mass_action_util.h"
#include "strconv.h"
#include "nary_order_transformation_method.h"
#include "nary_order_decider.h"
#include "logical_species_node.h"
#include "critical_concentration_finder.h"
#include "abstraction_method_manager.h"


#define REB2SAC_DEFAULT_NARY_LEVEL_OUTPUT_NAME "out.xml"
#define REB2SAC_NARY_LEVEL_SPECIES_PROPERTIES_FILE_NAME "species.properties"
#define REB2SAC_NARY_LEVEL_SPECIES_UNKNOWN_VALUE "<<unknown>>"

BEGIN_C_NAMESPACE
 
DLLSCOPE RET_VAL STDCALL ProcessNaryLevelBackend( BACK_END_PROCESSOR *backend, IR *ir );
DLLSCOPE RET_VAL STDCALL CloseNaryLevelBackend( BACK_END_PROCESSOR *backend );


DLLSCOPE RET_VAL STDCALL ProcessNaryLevelBackend2( BACK_END_PROCESSOR *backend, IR *ir );
DLLSCOPE RET_VAL STDCALL CloseNaryLevelBackend2( BACK_END_PROCESSOR *backend );

END_C_NAMESPACE
     
#endif

 
 
 
