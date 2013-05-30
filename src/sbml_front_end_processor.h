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
#if !defined(HAVE_SBML_FRONT_END_PROCESSOR)
#define HAVE_SBML_FRONT_END_PROCESSOR

#include "common.h"
#include "compiler_def.h"
#include "hash_table.h"
#include "front_end_processor.h"
#include "IR.h"
#include "unit_manager.h"
#include "function_manager.h"
#include "rule_manager.h"
#include "constraint_manager.h"
#include "compartment_manager.h"
#include "reaction_manager.h"
#include "species_node.h"
#include "reaction_node.h"
#include "kinetic_law.h"
#include "sbml_symtab.h"

#include "sbml/ListOf.h"
#include "sbml/SBMLDocument.h"
#include "sbml/SBMLReader.h"
#include "sbml/Model.h" 
#include "sbml/Species.h"
#include "sbml/Reaction.h"
#include "sbml/SpeciesReference.h"
//#include "sbml/ModifierSpeciesReference.h"
#include "sbml/UnitDefinition.h"
#include "sbml/UnitKind.h"
#include "sbml/Compartment.h"
#include "sbml/Parameter.h"
#include "sbml/KineticLaw.h"
#include "sbml/math/ASTNode.h"
#include "sbml/InitialAssignment.h"
#include "sbml/Rule.h"
#include "sbml/Constraint.h"
#include "sbml/Event.h"
#include "sbml/EventAssignment.h"
#include "sbml/Trigger.h"
#include "sbml/Delay.h"
#include "sbml/StoichiometryMath.h"
#include "sbml/FunctionDefinition.h"
#include "sbml/math/FormulaFormatter.h"
#include "sbml/xml/XMLNode.h"
#include "sbml/packages/comp/sbml/Submodel.h"
#include "sbml/packages/comp/extension/CompModelPlugin.h"
#include "sbml/conversion/ConversionOption.h"
#include "sbml/conversion/ConversionProperties.h"

BEGIN_C_NAMESPACE

 
RET_VAL ProcessSBMLFrontend( FRONT_END_PROCESSOR *frontend, IR *ir );
RET_VAL CloseSBMLFrontend( FRONT_END_PROCESSOR *frontend );

END_C_NAMESPACE
     
#endif

