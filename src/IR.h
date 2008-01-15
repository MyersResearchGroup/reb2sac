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
#if !defined(HAVE_IR)
#define HAVE_IR

#include "common.h"
#include "util.h"
#include "compiler_def.h"
#include "linked_list.h"
#include "ir_node.h"
#include "species_node.h"
#include "reaction_node.h"
#include "symtab.h"
#include "function_manager.h"
#include "rule_manager.h"

BEGIN_C_NAMESPACE


/*
struct _IR;
typedef struct _IR IR;
*/

struct _IR {            
    LINKED_LIST *speciesList;
    LINKED_LIST *reactionList;
    LINKED_LIST *reactantEdges;
    LINKED_LIST *modifierEdges;
    LINKED_LIST *productEdges;
    BOOL changeFlag;

    COMPILER_RECORD_T *record;
    UNIT_MANAGER *unitManager;
    FUNCTION_MANAGER *functionManager;
    RULE_MANAGER *ruleManager;
    COMPARTMENT_MANAGER *compartmentManager;
    REB2SAC_SYMTAB *globalSymtab;
    
    LINKED_LIST * (*GetListOfSpeciesNodes)( IR *ir );    
    LINKED_LIST * (*GetListOfReactionNodes)( IR *ir );
        
    LINKED_LIST * (*GetListOfReactantEdges)( IR *ir );    
    LINKED_LIST * (*GetListOfModifierEdges)( IR *ir );
    LINKED_LIST * (*GetListOfProductEdges)( IR *ir );
    
    SPECIES* (*CreateSpecies)( IR *ir, char *name );    
    REACTION* (*CreateReaction)( IR *ir, char *name );
    
    SPECIES* (*CloneSpecies)( IR *ir, SPECIES *species );    
    REACTION* (*CloneReaction)( IR *ir, REACTION *reaction );
    
    RET_VAL  (*AddSpecies)( IR *ir, SPECIES *species ); 
    RET_VAL (*AddReaction)( IR *ir, REACTION *reaction );
    
    RET_VAL (*RemoveSpecies)( IR *ir, SPECIES *species );     
    RET_VAL (*RemoveReaction)( IR *ir, REACTION *reaction );

        
    RET_VAL (*AddReactantInReaction)( IR *ir, REACTION *reaction, SPECIES *reactant );
    RET_VAL (*RemoveReactantInReaction)( IR *ir, REACTION *reaction, SPECIES *reactant );
    
    RET_VAL (*AddModifierInReaction)( IR *ir, REACTION *reaction, SPECIES *modifier );
    RET_VAL (*RemoveModifierInReaction)( IR *ir, REACTION *reaction, SPECIES *modifier );
            
    RET_VAL (*AddProductInReaction)( IR *ir, REACTION *reaction, SPECIES *product );
    RET_VAL (*RemoveProductInReaction)( IR *ir, REACTION *reaction, SPECIES *product );
    
    
    
    RET_VAL (*AddReactantEdge)( IR *ir, REACTION *reaction, SPECIES *reactant, int stoichiometry );
    RET_VAL (*RemoveReactantEdge)( IR *ir, IR_EDGE **reactantEdge );
    
    RET_VAL (*AddModifierEdge)( IR *ir, REACTION *reaction, SPECIES *modifier, int stoichiometry );
    RET_VAL (*RemoveModifierEdge)( IR *ir, IR_EDGE **modifierEdge );
            
    RET_VAL (*AddProductEdge)( IR *ir, REACTION *reaction, SPECIES *product, int stoichiometry );
    RET_VAL (*RemoveProductEdge)( IR *ir, IR_EDGE **productEdge );
    
    void (*ResetChangeFlag)( IR *ir );
    BOOL (*IsStructureChanged)( IR *ir );
    
    UNIT_MANAGER * (*GetUnitManager)( IR *ir );
    FUNCTION_MANAGER * (*GetFunctionManager)( IR *ir );
    RULE_MANAGER * (*GetRuleManager)( IR *ir );
    COMPARTMENT_MANAGER * (*GetCompartmentManager)( IR *ir );
    REB2SAC_SYMTAB *(*GetGlobalSymtab)( IR *ir );
    
    RET_VAL (*SetUnitManager)( IR *ir, UNIT_MANAGER *unitManager );
    RET_VAL (*SetFunctionManager)( IR *ir, FUNCTION_MANAGER *functionManager );
    RET_VAL (*SetRuleManager)( IR *ir, RULE_MANAGER *ruleManager );
    RET_VAL (*SetCompartmentManager)( IR *ir, COMPARTMENT_MANAGER *compartmentManager );
        
    
    RET_VAL (*GenerateDotFile)( IR *ir, FILE *file ); 
    RET_VAL (*GenerateSBML)( IR *ir, FILE *file );           
    RET_VAL (*GenerateXHTML)( IR *ir, FILE *file );           
};


RET_VAL InitIR(  COMPILER_RECORD_T *record );
RET_VAL FreeIR( IR *ir );

DLLSCOPE SPECIES * STDCALL GetSpeciesInIREdge( IR_EDGE *edge );
DLLSCOPE int STDCALL GetStoichiometryInIREdge( IR_EDGE *edge );
DLLSCOPE REACTION * STDCALL GetReactionInIREdge( IR_EDGE *edge );  
DLLSCOPE RET_VAL STDCALL SetStoichiometryInIREdge( IR_EDGE *edge, int stoichiometry );


END_C_NAMESPACE
 
#endif

