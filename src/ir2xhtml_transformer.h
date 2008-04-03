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
#if !defined(HAVE_IR2XHTML_TRANSFORMER)
#define HAVE_IR2XHTML_TRANSFORMER

#include "common.h"
#include "compiler_def.h"
#include "IR.h"


#define REB2SAC_XHTML_START_FORMAT_ON_LINE \
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>" NEW_LINE \
"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1 plus MathML 2.0//EN\" \"http://www.w3.org/TR/MathML2/dtd/xhtml-math11-f.dtd\">" NEW_LINE \
"<html xmlns=\"http://www.w3.org/1999/xhtml\">" NEW_LINE \
"<body>" NEW_LINE

#define REB2SAC_XHTML_START_FORMAT_OFF_LINE \
"<?xml version=\"1.0\"?>" NEW_LINE \
"<?xml-stylesheet type=\"text/xsl\" href="mathml.xsl\"?>" NEW_LINE  \
"<html xmlns=\"http://www.w3.org/1999/xhtml\">" NEW_LINE \
"<body>" NEW_LINE

#define REB2SAC_XHTML_END_FORMAT \
"</body>" NEW_LINE \
"</html>" NEW_LINE

#define REB2SAC_XHTML_LINE_BREAK \
"<br/>" NEW_LINE


#define REB2SAC_XHTML_MATHML_START_MATH_FORMAT "<math xmlns=\"http://www.w3.org/1998/Math/MathML\" display=\"block\">"
#define REB2SAC_XHTML_MATHML_END_MATH_FORMAT "</math>"

#define REB2SAC_XHTML_MATHML_START_SUBGROUP_FORMAT "<mrow>"
#define REB2SAC_XHTML_MATHML_END_SUBGROUP_FORMAT "</mrow>"

#define REB2SAC_XHTML_MATHML_COMPARTMENT_FORMAT "<mi>%s</mi>"
#define REB2SAC_XHTML_MATHML_SPECIES_FORMAT "<mi>%s</mi>"
#define REB2SAC_XHTML_MATHML_SYMBOL_FORMAT "<mi>%s</mi>"
#define REB2SAC_XHTML_MATHML_REAL_VALUE_FORMAT "<mi>%g</mi>"
#define REB2SAC_XHTML_MATHML_INT_VALUE_FORMAT "<mi>%li</mi>"

#define REB2SAC_XHTML_MATHML_OP_PLUS_FORMAT "<mo>+</mo>"
#define REB2SAC_XHTML_MATHML_OP_MINUS_FORMAT "<mo>-</mo>"
//#define REB2SAC_XHTML_MATHML_OP_TIMES_FORMAT "<mo><mchar name=\"InvisibleTimes\"/></mo>"
#define REB2SAC_XHTML_MATHML_OP_TIMES_FORMAT "<mo>*</mo>"
#define REB2SAC_XHTML_MATHML_START_FRAC_FORMAT "<mfrac>"
#define REB2SAC_XHTML_MATHML_END_FRAC_FORMAT "</mfrac>"
#define REB2SAC_XHTML_MATHML_START_POWER_FORMAT "<msup>"
#define REB2SAC_XHTML_MATHML_END_POWER_FORMAT "</msup>"

#define REB2SAC_XHTML_MATHML_L_PAREN_FORMAT "<mo>(</mo>"
#define REB2SAC_XHTML_MATHML_R_PAREN_FORMAT "<mo>)</mo>"

#define REB2SAC_XHTML_START_CONSTANTS_FORMAT \
"<table border=\"2\"><tr><th>Constant ID</th><th>Value</th></tr>" NEW_LINE
 
#define REB2SAC_XHTML_END_CONSTANTS_FORMAT \
"</table>" NEW_LINE

#define REB2SAC_XHTML_CONSTANT_ENTRY_FORMAT \
"<tr><td>%s</td><td>%g</td></tr>" NEW_LINE

#define REB2SAC_XHTML_START_COMPARTMENT_FORMAT \
"<table border=\"2\"><tr><th>Compartment ID</th><th>Initial Size</th></tr>" NEW_LINE
 
#define REB2SAC_XHTML_END_COMPARTMENT_FORMAT \
"</table>" NEW_LINE

#define REB2SAC_XHTML_COMPARTMENT_ENTRY_FORMAT \
"<tr><td>%s</td><td>%g</td></tr>" NEW_LINE

#define REB2SAC_XHTML_START_SPECIES_FORMAT \
"<table border=\"2\"><tr><th>Species ID</th><th>Compartment</th><th>Initial Value</th></tr>" NEW_LINE
 
#define REB2SAC_XHTML_END_SPECIES_FORMAT \
"</table>" NEW_LINE

#define REB2SAC_XHTML_SPECIES_ENTRY_FORMAT \
"<tr><td>%s</td><td>%s</td><td>%g</td></tr>" NEW_LINE

#define REB2SAC_XHTML_START_REACTION_FORMAT \
"<table border=\"2\"><tr><th>Reaction Name</th><td>%s</td></tr>" NEW_LINE
 
#define REB2SAC_XHTML_END_REACTION_FORMAT \
"</table>" NEW_LINE

#define REB2SAC_XHTML_START_REACTANTS_FORMAT \
"<tr><th>Reactants</th><td>"

#define REB2SAC_XHTML_END_REACTANTS_FORMAT \
"</td></tr>" NEW_LINE

#define REB2SAC_XHTML_START_MODIFIERS_FORMAT \
"<tr><th>Modifiers</th><td>"

#define REB2SAC_XHTML_END_MODIFIERS_FORMAT \
"</td></tr>" NEW_LINE

#define REB2SAC_XHTML_START_PRODUCTS_FORMAT \
"<tr><th>Products</th><td>"

#define REB2SAC_XHTML_END_PRODUCTS_FORMAT \
"</td></tr>" NEW_LINE


#define REB2SAC_XHTML_START_KINETIC_LAW_FORMAT \
"<tr><th>Kinetic Law</th><td>"

#define REB2SAC_XHTML_END_KINETIC_LAW_FORMAT \
"</td></tr>" NEW_LINE


#define REB2SAC_XHTML_SPECIES_FORMAT "%s"
#define REB2SAC_XHTML_MORE_SPECIES_FORMAT ", %s"

#define REB2SAC_XHTML_SPECIES_FORMAT2 "%i%s"
#define REB2SAC_XHTML_MORE_SPECIES_FORMAT2 ", %i%s"

#define __IR2XHTML_TAB "    "

BEGIN_C_NAMESPACE

RET_VAL GenerateXHTMLFromIR( IR *ir, FILE *file );

RET_VAL PrintConstantsInXHTML( IR *ir, FILE *file ); 
RET_VAL PrintListOfReactionsInXHTML( LINKED_LIST *list, FILE *file ); 
RET_VAL PrintReactionInXHTML( REACTION *reaction, FILE *file ); 
RET_VAL PrintListOfReactantsInXHTML( LINKED_LIST *list, FILE *file ); 
RET_VAL PrintListOfProductsInXHTML( LINKED_LIST *list, FILE *file ); 
RET_VAL PrintListOfModifiersInXHTML( LINKED_LIST *list, FILE *file ); 
RET_VAL PrintListOfSpeciesInXHTML( LINKED_LIST *list, FILE *file ); 
RET_VAL PrintCompartmentListInXHTML( LINKED_LIST *list, FILE *file ); 
RET_VAL PrintSpeciesListInXHTML( LINKED_LIST *list, FILE *file ); 
RET_VAL PrintKineticLawInXHTML( KINETIC_LAW *kineticLaw, FILE *file ); 



END_C_NAMESPACE
     
#endif













