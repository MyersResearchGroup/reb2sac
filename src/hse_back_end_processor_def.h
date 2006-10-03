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
#if !defined(HAVE_HSE_BACK_END_PROCESSOR_DEF)
#define HAVE_HSE_BACK_END_PROCESSOR_DEF

#include "common.h"
#include "compiler_def.h"

#define REB2SAC_DEFAULT_HSE_OUTPUT_NAME "out.hse"

#define REB2SAC_HSE_START_MODULE_FORMAT "module %s;"
#define REB2SAC_HSE_END_MODULE_FORMAT "endmodule"
#define REB2SAC_HSE_NO_NAME_MODULE "no_name"
#define REB2SAC_HSE_MODULE_ID_SIZE 256
#define REB2SAC_HSE_NO_ID_REACTION "<none>"

#define REB2SAC_HSE_OUTPUT_STATEMENT_FORMAT "output %s; /* %s */"
#define REB2SAC_HSE_OUTPUT_STATEMENT_WITH_INITIAL_VALUE_TRUE_FORMAT "output %s = { true }; /* Critical Con. is %f */"
#define REB2SAC_HSE_OUTPUT_STATEMENT_WITH_INITIAL_VALUE_FALSE_FORMAT "output %s = { false }; /* Critical Con. is %f */"
#define REB2SAC_HSE_DEFINE_MACRO_FORMAT "define %s = %s;"

#define REB2SAC_HSE_START_MAIN_PROC_FORMAT "process main;"
#define REB2SAC_HSE_END_MAIN_PROC_FORMAT "endprocess"
#define REB2SAC_HSE_START_PROC_FORMAT "process %s;"
#define REB2SAC_HSE_END_PROC_FORMAT "endprocess"

#define REB2SAC_HSE_START_INFINITE_LOOP_FORMAT "*[["
#define REB2SAC_HSE_END_INFINITE_LOOP_FORMAT "]]"

#define REB2SAC_HSE_REACTION_COMMENT_FORMAT_START "\t/* %s */"  NEW_LINE
#define REB2SAC_HSE_REACTION_COMMENT_FORMAT_REACTANTS "\t/* reactants = "
#define REB2SAC_HSE_REACTION_COMMENT_FORMAT_PRODUCTS "\t/* products = "
#define REB2SAC_HSE_REACTION_COMMENT_FORMAT_MODIFIERS "\t/* modifiers = "
#define REB2SAC_HSE_REACTION_COMMENT_FORMAT_KINETIC_LAW "\t/* rate = %s */" NEW_LINE
#define REB2SAC_HSE_REACTION_COMMENT_FORMAT_NO_SPECIES "  "
#define REB2SAC_HSE_REACTION_COMMENT_FORMAT_FIRST_SPECIES "%s"
#define REB2SAC_HSE_REACTION_COMMENT_FORMAT_MORE_SPECIES ", %s"
#define REB2SAC_HSE_REACTION_COMMENT_FORMAT_L_BRACE "{"
#define REB2SAC_HSE_REACTION_COMMENT_FORMAT_R_BRACE "} */" NEW_LINE
#define REB2SAC_HSE_REACTION_COMMENT_FORMAT_COMMA ","
#define REB2SAC_HSE_REACTION_COMMENT_FORMAT_END " */"
#define REB2SAC_HSE_REACTION_COMMENT_FORMAT_DEGRADATION "\t/* degradation */" NEW_LINE
#define REB2SAC_HSE_REACTION_COMMENT_FORMAT_PRODUCTION "\t/* production */" NEW_LINE

#define REB2SAC_HSE_DEFAULT_VALUE_TO_EVALUATE_MODIFIERS 0.5
#define REB2SAC_HSE_DEFAULT_VALUE_TO_EVALUATE 1.0
#define REB2SAC_HSE_VALUE1_TO_EVALUATE_MODIFIERS 0.1
#define REB2SAC_HSE_VALUE2_TO_EVALUATE_MODIFIERS 0.9
#define REB2SAC_HSE_ACTIVATOR_VALUE 1.0
#define REB2SAC_HSE_INHIBITOR_VALUE 0.0


#define REB2SAC_HSE_REACTION_FORMAT_AND " & "
#define REB2SAC_HSE_REACTION_FORMAT_OR "\t| "
#define REB2SAC_HSE_REACTION_FORMAT_SPACES "\t  "
#define REB2SAC_HSE_REACTION_FORMAT_NOT "~"
#define REB2SAC_HSE_REACTION_FORMAT_PLUS "+"
#define REB2SAC_HSE_REACTION_FORMAT_MINUS "-"
#define REB2SAC_HSE_REACTION_FORMAT_ARROW " -> "
#define REB2SAC_HSE_REACTION_FORMAT_SEMICOLON "; "
#define REB2SAC_HSE_REACTION_FORMAT_RATE " ( %s )"
#define REB2SAC_HSE_REACTION_FORMAT_SPECIES "%s"
#define REB2SAC_HSE_REACTION_FORMAT_FALSE "false"
#define REB2SAC_HSE_REACTION_FORMAT_TRUE "true"

#define REB2SAC_HSE_REACTION_FORMAT_AND_XML " &#38; "
#define REB2SAC_HSE_REACTION_FORMAT_NOT_XML "&#126;"

#define REB2SAC_LOGICAL_STATEMENT_OUTPUT_TYPE_DEFAULT 0
#define REB2SAC_LOGICAL_STATEMENT_OUTPUT_TYPE_XML 1

#define REB2SAC_RATE_ZERO_STATEMENT_RET (E_NODATA|FAILING)

     
#endif
