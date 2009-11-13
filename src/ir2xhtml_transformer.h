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

#define REB2SAC_XHTML_MATHML_FUNCTION_SYMBOL_FORMAT "<mi>%s</mi>"
#define REB2SAC_XHTML_MATHML_COMPARTMENT_FORMAT "<mi>%s</mi>"
#define REB2SAC_XHTML_MATHML_SPECIES_FORMAT "<mi>%s</mi>"
#define REB2SAC_XHTML_MATHML_SYMBOL_FORMAT "<mi>%s</mi>"
#define REB2SAC_XHTML_MATHML_REAL_VALUE_FORMAT "<mi>%g</mi>"
#define REB2SAC_XHTML_MATHML_INT_VALUE_FORMAT "<mi>%li</mi>"

#define REB2SAC_XHTML_MATHML_OP_PW_FORMAT "<mo>piecewise</mo>"
#define REB2SAC_XHTML_MATHML_OP_EXP_FORMAT "<mo>e</mo>"
#define REB2SAC_XHTML_MATHML_OP_LOG_FORMAT "<mo>log</mo>"
#define REB2SAC_XHTML_MATHML_OP_LN_FORMAT "<mo>ln</mo>"
#define REB2SAC_XHTML_MATHML_OP_FACT_FORMAT "<mo>!</mo>"

#define REB2SAC_XHTML_MATHML_OP_COS_FORMAT "<mo>cos</mo>"
#define REB2SAC_XHTML_MATHML_OP_SIN_FORMAT "<mo>sin</mo>"
#define REB2SAC_XHTML_MATHML_OP_COSH_FORMAT "<mo>cosh</mo>"
#define REB2SAC_XHTML_MATHML_OP_SINH_FORMAT "<mo>sinh</mo>"
#define REB2SAC_XHTML_MATHML_OP_TAN_FORMAT "<mo>tan</mo>"
#define REB2SAC_XHTML_MATHML_OP_TANH_FORMAT "<mo>tanh</mo>"

#define REB2SAC_XHTML_MATHML_OP_COT_FORMAT "<mo>cot</mo>"
#define REB2SAC_XHTML_MATHML_OP_COTH_FORMAT "<mo>coth</mo>"
#define REB2SAC_XHTML_MATHML_OP_CSC_FORMAT "<mo>csc</mo>"
#define REB2SAC_XHTML_MATHML_OP_CSCH_FORMAT "<mo>csch</mo>"
#define REB2SAC_XHTML_MATHML_OP_SEC_FORMAT "<mo>sec</mo>"
#define REB2SAC_XHTML_MATHML_OP_SECH_FORMAT "<mo>sech</mo>"

#define REB2SAC_XHTML_MATHML_OP_ARCCOS_FORMAT "<mo>arccos</mo>"
#define REB2SAC_XHTML_MATHML_OP_ARCSIN_FORMAT "<mo>arcsin</mo>"
#define REB2SAC_XHTML_MATHML_OP_ARCCOSH_FORMAT "<mo>arccosh</mo>"
#define REB2SAC_XHTML_MATHML_OP_ARCSINH_FORMAT "<mo>arcsinh</mo>"
#define REB2SAC_XHTML_MATHML_OP_ARCTAN_FORMAT "<mo>arctan</mo>"
#define REB2SAC_XHTML_MATHML_OP_ARCTANH_FORMAT "<mo>arctanh</mo>"

#define REB2SAC_XHTML_MATHML_OP_ARCCOT_FORMAT "<mo>arccot</mo>"
#define REB2SAC_XHTML_MATHML_OP_ARCCOTH_FORMAT "<mo>arccoth</mo>"
#define REB2SAC_XHTML_MATHML_OP_ARCCSC_FORMAT "<mo>arccsc</mo>"
#define REB2SAC_XHTML_MATHML_OP_ARCCSCH_FORMAT "<mo>arccsch</mo>"
#define REB2SAC_XHTML_MATHML_OP_ARCSEC_FORMAT "<mo>arcsec</mo>"
#define REB2SAC_XHTML_MATHML_OP_ARCSECH_FORMAT "<mo>arcsech</mo>"

#define REB2SAC_XHTML_MATHML_OP_UNIFORM_FORMAT "<mo>uniform</mo>"
#define REB2SAC_XHTML_MATHML_OP_NORMAL_FORMAT "<mo>normal</mo>"
#define REB2SAC_XHTML_MATHML_OP_GAMMA_FORMAT "<mo>gamma</mo>"
#define REB2SAC_XHTML_MATHML_OP_EXPRAND_FORMAT "<mo>exprand</mo>"
#define REB2SAC_XHTML_MATHML_OP_POISSON_FORMAT "<mo>poisson</mo>"
#define REB2SAC_XHTML_MATHML_OP_BINOMIAL_FORMAT "<mo>binomial</mo>"
#define REB2SAC_XHTML_MATHML_OP_LOGNORMAL_FORMAT "<mo>lognormal</mo>"
#define REB2SAC_XHTML_MATHML_OP_CHISQ_FORMAT "<mo>chisq</mo>"
#define REB2SAC_XHTML_MATHML_OP_LAPLACE_FORMAT "<mo>laplace</mo>"
#define REB2SAC_XHTML_MATHML_OP_CAUCHY_FORMAT "<mo>cauchy</mo>"
#define REB2SAC_XHTML_MATHML_OP_RAYLEIGH_FORMAT "<mo>rayleigh</mo>"
#define REB2SAC_XHTML_MATHML_OP_BERNOULLI_FORMAT "<mo>bernoulli</mo>"

#define REB2SAC_XHTML_MATHML_OP_DELAY_FORMAT "<mo>delay</mo>"

#define REB2SAC_XHTML_MATHML_OP_NOT_FORMAT "<mo>&not;</mo>"
#define REB2SAC_XHTML_MATHML_OP_NEG_FORMAT "<mo>-</mo>"
#define REB2SAC_XHTML_MATHML_OP_AND_FORMAT "<mo>and</mo>"
#define REB2SAC_XHTML_MATHML_OP_OR_FORMAT "<mo>or</mo>"
#define REB2SAC_XHTML_MATHML_OP_XOR_FORMAT "<mo>xor</mo>"
//#define REB2SAC_XHTML_MATHML_OP_AND_FORMAT "<mo>&and;</mo>"
//#define REB2SAC_XHTML_MATHML_OP_OR_FORMAT "<mo>&or;</mo>"
//#define REB2SAC_XHTML_MATHML_OP_XOR_FORMAT "<mo>&oplus;</mo>"
#define REB2SAC_XHTML_MATHML_OP_EQ_FORMAT "<mo>=</mo>"
#define REB2SAC_XHTML_MATHML_OP_NEQ_FORMAT "<mo>&ne;</mo>"
#define REB2SAC_XHTML_MATHML_OP_GT_FORMAT "<mo>&gt;</mo>"
#define REB2SAC_XHTML_MATHML_OP_LT_FORMAT "<mo>&lt;</mo>"
#define REB2SAC_XHTML_MATHML_OP_GEQ_FORMAT "<mo>&ge;</mo>"
#define REB2SAC_XHTML_MATHML_OP_LEQ_FORMAT "<mo>&le;</mo>"
#define REB2SAC_XHTML_MATHML_OP_PLUS_FORMAT "<mo>+</mo>"
#define REB2SAC_XHTML_MATHML_OP_MINUS_FORMAT "<mo>-</mo>"
//#define REB2SAC_XHTML_MATHML_OP_TIMES_FORMAT "<mo><mchar name=\"InvisibleTimes\"/></mo>"
#define REB2SAC_XHTML_MATHML_OP_TIMES_FORMAT "<mo>*</mo>"
#define REB2SAC_XHTML_MATHML_START_FRAC_FORMAT "<mfrac>"
#define REB2SAC_XHTML_MATHML_END_FRAC_FORMAT "</mfrac>"
#define REB2SAC_XHTML_MATHML_START_POWER_FORMAT "<msup>"
#define REB2SAC_XHTML_MATHML_END_POWER_FORMAT "</msup>"
#define REB2SAC_XHTML_MATHML_SUP_INT_FORMAT "<sup>%d</sup>"

#define REB2SAC_XHTML_MATHML_ROOT_FORMAT "<mo>1&frasl;</mo>"

#define REB2SAC_XHTML_MATHML_BULLET_FORMAT "<mo>&bull;</mo>"
#define REB2SAC_XHTML_MATHML_COMMA_FORMAT "<mo>,</mo>"
#define REB2SAC_XHTML_MATHML_L_PAREN_FORMAT "<mo>(</mo>"
#define REB2SAC_XHTML_MATHML_R_PAREN_FORMAT "<mo>)</mo>"
#define REB2SAC_XHTML_MATHML_ABS_FORMAT "<mo>|</mo>"
#define REB2SAC_XHTML_MATHML_L_FLOOR_FORMAT "<mo>&lfloor;</mo>"
#define REB2SAC_XHTML_MATHML_R_FLOOR_FORMAT "<mo>&rfloor;</mo>"
#define REB2SAC_XHTML_MATHML_L_CEILING_FORMAT "<mo>&lceil;</mo>"
#define REB2SAC_XHTML_MATHML_R_CEILING_FORMAT "<mo>&rceil;</mo>"

#define REB2SAC_XHTML_START_CONSTANTS_FORMAT \
"<table border=\"2\"><tr><th>Parameter ID</th><th>Initial Value</th><th>Units</th><th>Constant</th></tr>" NEW_LINE
 
#define REB2SAC_XHTML_END_CONSTANTS_FORMAT \
"</table>" NEW_LINE

#define REB2SAC_XHTML_START_CONSTANT_ENTRY_FORMAT \
"<tr><td>%s</td><td><center>"

#define REB2SAC_XHTML_END_CONSTANT_ENTRY_FORMAT \
"</center></td><td>%s</td><td>%s</td></tr>" NEW_LINE

#define REB2SAC_XHTML_START_FUNCTION_FORMAT \
"<table border=\"2\"><tr><th>Functions</th></tr>" NEW_LINE
 
#define REB2SAC_XHTML_END_FUNCTION_FORMAT \
"</table>" NEW_LINE

#define REB2SAC_XHTML_START_FUNCTION_ENTRY_FORMAT \
"<tr><td>"

#define REB2SAC_XHTML_END_FUNCTION_ENTRY_FORMAT \
"</td></tr>" NEW_LINE

#define REB2SAC_XHTML_START_UNITDEF_FORMAT \
"<table border=\"2\"><tr><th>Unit ID</th><th>Definition</th></tr>" NEW_LINE
 
#define REB2SAC_XHTML_END_UNITDEF_FORMAT \
"</table>" NEW_LINE

#define REB2SAC_XHTML_START_UNITDEF_ENTRY_FORMAT \
"<tr><td>%s</td><td>"
 
#define REB2SAC_XHTML_END_UNITDEF_ENTRY_FORMAT \
"</td></tr>" NEW_LINE

#define REB2SAC_XHTML_START_RULE_FORMAT \
"<table border=\"2\"><tr><th>Rules</th></tr>" NEW_LINE
 
#define REB2SAC_XHTML_END_RULE_FORMAT \
"</table>" NEW_LINE

#define REB2SAC_XHTML_START_ALGRULE_ENTRY_FORMAT \
"<tr><td>0 = "

#define REB2SAC_XHTML_START_ASSIGNRULE_ENTRY_FORMAT \
"<tr><td>%s = "

#define REB2SAC_XHTML_START_RATERULE_ENTRY_FORMAT \
"<tr><td>d(%s)/dt = "

#define REB2SAC_XHTML_START_RULE_ENTRY_FORMAT \
"<tr><td>"

#define REB2SAC_XHTML_END_RULE_ENTRY_FORMAT \
"</td></tr>" NEW_LINE

#define REB2SAC_XHTML_START_CONSTRAINT_FORMAT \
"<table border=\"2\"><tr><th>Constraint ID</th><th>Constraint</th><th>Message</th></tr>" NEW_LINE
/*"<table border=\"2\"><tr><th>Constraint ID</th><th>Message</th><th>Constraint</th></tr>" NEW_LINE*/
 
#define REB2SAC_XHTML_END_CONSTRAINT_FORMAT \
"</table>" NEW_LINE

#define REB2SAC_XHTML_START_CONSTRAINT_ENTRY_FORMAT \
"<tr><td>%s</td><td>"
/*"<tr><td>%s</td><td>%s</td><td>"*/

#define REB2SAC_XHTML_END_CONSTRAINT_ENTRY_FORMAT \
"</td><td>%s</td></tr>" NEW_LINE

#define REB2SAC_XHTML_START_COMPARTMENT_FORMAT \
"<table border=\"2\"><tr><th>Compartment ID</th><th>Type</th><th>Dimensions</th><th>Initial Size</th><th>Units</th><th>Outside</th><th>Constant</th></tr>" NEW_LINE
 
#define REB2SAC_XHTML_END_COMPARTMENT_FORMAT \
"</table>" NEW_LINE

#define REB2SAC_XHTML_START_COMPARTMENT_ENTRY_FORMAT \
"<tr><td>%s</td><td>%s</td><td><center>%d</center></td><td><center>"

#define REB2SAC_XHTML_END_COMPARTMENT_ENTRY_FORMAT \
"</center></td><td>%s</td><td>%s</td><td>%s</td></tr>" NEW_LINE

#define REB2SAC_XHTML_START_SPECIES_FORMAT \
"<table border=\"2\"><tr><th>Species ID</th><th>Type</th><th>Compartment</th><th>Initial Value</th><th>Units</th><th>Boundary</th><th>Constant</th><th>HasOnlySubstanceUnits</th></tr>" NEW_LINE
 
#define REB2SAC_XHTML_END_SPECIES_FORMAT \
"</table>" NEW_LINE

#define REB2SAC_XHTML_START_SPECIES_ENTRY_FORMAT \
"<tr><td>%s</td><td>%s</td><td>%s</td><td><center>"

#define REB2SAC_XHTML_END_SPECIES_ENTRY_FORMAT \
"</center></td><td>%s</td><td>%s</td><td>%s</td><td>%s</td></tr>" NEW_LINE

#define REB2SAC_XHTML_START_EVENT_FORMAT \
"<table border=\"2\"><tr><th>Event ID</th><th>Trigger</th><th>Delay</th><th>Assignments</th></tr>" NEW_LINE
 
#define REB2SAC_XHTML_END_EVENT_FORMAT \
"</table>" NEW_LINE

#define REB2SAC_XHTML_START_EVENT_ENTRY_FORMAT \
"<tr><td>%s</td><td>"

#define REB2SAC_XHTML_SEPARATOR_FORMAT \
"</td><td>"

#define REB2SAC_XHTML_END_EVENT_ENTRY_FORMAT \
"</td></tr>" NEW_LINE

#define REB2SAC_XHTML_START_ASSIGN_FORMAT \
"<mo>%s=</mo>"

#define REB2SAC_XHTML_END_ASSIGN_FORMAT \
"" 

#define REB2SAC_XHTML_START_REACTION_FORMAT \
"<table border=\"2\"><tr><th>Reaction ID</th><th>Rev</th><th>Fast</th><th>Reactants</th><th>Products</th><th>Modifiers</th><th>Kinetic Law</th></tr>" NEW_LINE

#define REB2SAC_XHTML_END_REACTION_FORMAT \
"</table>" NEW_LINE

#define REB2SAC_XHTML_START_REACTION_ENTRY_FORMAT \
"<tr><td>%s</td><td>%s</td><td>%s</td><td>"

#define REB2SAC_XHTML_SEPARATOR_FORMAT \
"</td><td>"

#define REB2SAC_XHTML_END_REACTION_ENTRY_FORMAT \
"</td></tr>" NEW_LINE

#define REB2SAC_XHTML_SPECIES_FORMAT "%s"
#define REB2SAC_XHTML_MORE_SPECIES_FORMAT ", %s"

#define REB2SAC_XHTML_SPECIES_FORMAT2 "%g %s"
#define REB2SAC_XHTML_MORE_SPECIES_FORMAT2 ", %g %s"

#define __IR2XHTML_TAB "    "

BEGIN_C_NAMESPACE

RET_VAL GenerateXHTMLFromIR( IR *ir, FILE *file );

RET_VAL PrintEventsInXHTML( IR *ir, FILE *file ); 
RET_VAL PrintConstantsInXHTML( IR *ir, FILE *file ); 
RET_VAL PrintListOfReactionsInXHTML( LINKED_LIST *list, FILE *file ); 
RET_VAL PrintReactionInXHTML( REACTION *reaction, FILE *file ); 
RET_VAL PrintListOfReactantsInXHTML( LINKED_LIST *list, FILE *file ); 
RET_VAL PrintListOfProductsInXHTML( LINKED_LIST *list, FILE *file ); 
RET_VAL PrintListOfModifiersInXHTML( LINKED_LIST *list, FILE *file ); 
RET_VAL PrintListOfSpeciesInXHTML( LINKED_LIST *list, FILE *file ); 
RET_VAL PrintCompartmentListInXHTML( LINKED_LIST *list, FILE *file ); 
RET_VAL PrintFunctionListInXHTML( LINKED_LIST *list, FILE *file ); 
RET_VAL PrintUnitListInXHTML( LINKED_LIST *list, FILE *file ); 
RET_VAL PrintRuleListInXHTML( LINKED_LIST *list, FILE *file ); 
RET_VAL PrintSpeciesListInXHTML( LINKED_LIST *list, FILE *file ); 
RET_VAL PrintKineticLawInXHTML( KINETIC_LAW *kineticLaw, FILE *file ); 



END_C_NAMESPACE
     
#endif













