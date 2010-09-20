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
  
#include "IR.h"
#include "ir2xhtml_transformer.h"
#include <float.h>
 
static IR irObject;


static LINKED_LIST * _GetListOfSpeciesNodes( IR *ir );    
static LINKED_LIST * _GetListOfReactionNodes( IR *ir );

static LINKED_LIST * _GetListOfReactantEdges( IR *ir );    
static LINKED_LIST * _GetListOfModifierEdges( IR *ir );
static LINKED_LIST * _GetListOfProductEdges( IR *ir );

    
static RET_VAL _SetModelId( IR *ir, char *modelId );    
static RET_VAL _SetModelName( IR *ir, char *modelName );    
static RET_VAL _SetModelSubstanceUnits( IR *ir, char *modelSubstanceUnits );    
static RET_VAL _SetModelTimeUnits( IR *ir, char *modelTimeUnits );    
static RET_VAL _SetModelVolumeUnits( IR *ir, char *modelVolumeUnits );    
static RET_VAL _SetModelAreaUnits( IR *ir, char *modelAreaUnits );    
static RET_VAL _SetModelLengthUnits( IR *ir, char *modelLengthUnits );    
static RET_VAL _SetModelExtentUnits( IR *ir, char *modelExtentUnits );    
static STRING *_GetModelId( IR *ir );
static STRING *_GetModelName( IR *ir );
static STRING *_GetModelSubstanceUnits( IR *ir );
static STRING *_GetModelTimeUnits( IR *ir );
static STRING *_GetModelVolumeUnits( IR *ir );
static STRING *_GetModelAreaUnits( IR *ir );
static STRING *_GetModelLengthUnits( IR *ir );
static STRING *_GetModelExtentUnits( IR *ir );

static SPECIES *_CreateSpecies( IR *ir, char *name );    
static REACTION *_CreateReaction( IR *ir, char *name );

static SPECIES* _CloneSpecies( IR *ir, SPECIES *species );    
static REACTION* _CloneReaction( IR *ir, REACTION *reaction );
        
static RET_VAL  _AddSpecies( IR *ir, SPECIES *species ); 
static RET_VAL _AddReaction( IR *ir, REACTION *reaction );

static RET_VAL _RemoveSpecies( IR *ir, SPECIES *species ); 
static RET_VAL _RemoveReaction( IR *ir, REACTION *reaction );
static RET_VAL _RemoveRule( IR *ir, RULE *rule ); 

/*
static RET_VAL _AddReactantInReaction( IR *ir, REACTION *reaction, SPECIES *reactant );
static RET_VAL _AddModifierInReaction( IR *ir, REACTION *reaction, SPECIES *modifier );
static RET_VAL _AddProductInReaction( IR *ir, REACTION *reaction, SPECIES *product );
*/
static RET_VAL _RemoveProductInReaction( IR *ir, REACTION *reaction, SPECIES *product);
static RET_VAL _RemoveModifierInReaction( IR *ir, REACTION *reaction, SPECIES *modifier);
static RET_VAL _RemoveReactantInReaction( IR *ir, REACTION *reaction, SPECIES *reactant);

        


static RET_VAL _AddReactantEdge( IR *ir, REACTION *reaction, SPECIES *reactant, double stoichiometry,
				 REB2SAC_SYMBOL *speciesRef );
static RET_VAL _RemoveReactantEdge( IR *ir, IR_EDGE **reactantEdge );

static RET_VAL _AddModifierEdge( IR *ir, REACTION *reaction, SPECIES *modifier, double stoichiometry );
static RET_VAL _RemoveModifierEdge( IR *ir, IR_EDGE **modifierEdge );
        
static RET_VAL _AddProductEdge( IR *ir, REACTION *reaction, SPECIES *product, double stoichiometry,
				REB2SAC_SYMBOL *speciesRef );
static RET_VAL _RemoveProductEdge( IR *ir, IR_EDGE **productEdge );


static void _ResetChangeFlag( IR *ir );
static BOOL _IsStructureChanged( IR *ir );

static UNIT_MANAGER *_GetUnitManager( IR *ir );
static FUNCTION_MANAGER *_GetFunctionManager( IR *ir );
static RULE_MANAGER *_GetRuleManager( IR *ir );
static CONSTRAINT_MANAGER *_GetConstraintManager( IR *ir );
static EVENT_MANAGER *_GetEventManager( IR *ir );
static COMPARTMENT_MANAGER *_GetCompartmentManager( IR *ir );
static REACTION_LAW_MANAGER *_GetReactionLawManager( IR *ir );
static REB2SAC_SYMTAB *_GetGlobalSymtab( IR *ir );

static RET_VAL _SetUnitManager( IR *ir, UNIT_MANAGER *unitManager );
static RET_VAL _SetFunctionManager( IR *ir, FUNCTION_MANAGER *functionManager );
static RET_VAL _SetRuleManager( IR *ir, RULE_MANAGER *ruleManager );
static RET_VAL _SetConstraintManager( IR *ir, CONSTRAINT_MANAGER *constraintManager );
static RET_VAL _SetEventManager( IR *ir, EVENT_MANAGER *eventManager );
static RET_VAL _SetCompartmentManager( IR *ir, COMPARTMENT_MANAGER *compartmentManager );
static RET_VAL _SetReactionLawManager( IR *ir, REACTION_LAW_MANAGER *reactionLawManager );

            
static RET_VAL _GenerateDotFile( IR *ir, FILE *file );            
static RET_VAL _GenerateSBML( IR *ir, FILE *file, char *filename );           
static RET_VAL _GenerateXHTML( IR *ir, FILE *file );           

// TODO: create function definition print functions
// TODO: create rule definition print functions

static RET_VAL _PrintListOfUnitDefinitionsForSBML( IR *ir, FILE *file, UINT32 tabCount );
static RET_VAL _PrintUnitDefinitionForSBML( UNIT_DEFINITION *unitDef, FILE *file, UINT32 tabCount );
static RET_VAL _PrintUnitForSBML( UNIT *unit, FILE *file, UINT32 tabCount );

static RET_VAL _PrintListOfCompartmentsForSBML( IR *ir, FILE *file, UINT32 tabCount );
static RET_VAL _PrintCompartmentsForSBML( COMPARTMENT *compartment, FILE *file, UINT32 tabCount );

static RET_VAL _PrintListOfSpeciesForSBML( IR *ir, FILE *file, UINT32 tabCount );
static RET_VAL _PrintSpeciesForSBML( SPECIES *species, FILE *file, UINT32 tabCount );

static RET_VAL _PrintListOfParametersForSBML( IR *ir, FILE *file, UINT32 tabCount );
static RET_VAL _PrintParameterForSBML( REB2SAC_SYMBOL *sym, FILE *file, UINT32 tabCount );

static RET_VAL _PrintListOfConstraintsForSBML( IR *ir, FILE *file, UINT32 tabCount );
static RET_VAL _PrintConstraintForSBML( CONSTRAINT *constraint, FILE *file, UINT32 tabCount );

static RET_VAL _PrintListOfRulesForSBML( IR *ir, FILE *file, UINT32 tabCount );
static RET_VAL _PrintRuleForSBML( RULE *rule, FILE *file, UINT32 tabCount );

static RET_VAL _PrintListOfReactionsForSBML( IR *ir, FILE *file, UINT32 tabCount );
static RET_VAL _PrintReactionForSBML( REACTION *reaction, FILE *file, UINT32 tabCount );

static RET_VAL _PrintListOfEventsForSBML( IR *ir, FILE *file, UINT32 tabCount );
static RET_VAL _PrintEventForSBML( EVENT *event, FILE *file, UINT32 tabCount );

static RET_VAL _PrintKineticLawForSBML( KINETIC_LAW *kineticLaw, FILE *file, UINT32 tabCount );
static RET_VAL _PrintMathForSBML( KINETIC_LAW *kineticLaw, FILE *file, UINT32 tabCount );
static RET_VAL _DispatchKineticLawForSBML( KINETIC_LAW *kineticLaw, FILE *file, UINT32 tabCount );
static RET_VAL _PrintOpKineticLawForSBML( KINETIC_LAW *kineticLaw, FILE *file, UINT32 tabCount );
static RET_VAL _PrintUnaryOpKineticLawForSBML( KINETIC_LAW *kineticLaw, FILE *file, UINT32 tabCount );
static RET_VAL _PrintPWKineticLawForSBML( KINETIC_LAW *kineticLaw, FILE *file, UINT32 tabCount );
static RET_VAL _PrintIntKineticLawForSBML( KINETIC_LAW *kineticLaw, FILE *file, UINT32 tabCount );
static RET_VAL _PrintRealKineticLawForSBML( KINETIC_LAW *kineticLaw, FILE *file, UINT32 tabCount );
static RET_VAL _PrintSpeciesKineticLawForSBML( KINETIC_LAW *kineticLaw, FILE *file, UINT32 tabCount );
static RET_VAL _PrintSymbolKineticLawForSBML( KINETIC_LAW *kineticLaw, FILE *file, UINT32 tabCount );
static RET_VAL _PrintCompartmentKineticLawForSBML( KINETIC_LAW *kineticLaw, FILE *file, UINT32 tabCount );

static RET_VAL _PrintCnInEnotationForSBML( FILE *file, UINT32 tabCount, double value );

#define __IR_TAB "    "
static void _PrintTab( FILE *file, UINT32 tabCount );




RET_VAL InitIR(  COMPILER_RECORD_T *record ) {
    
        
    RET_VAL ret = SUCCESS;
    IR *ir = NULL;
    
    START_FUNCTION("InitIR");
    
    if( record->ir != NULL ) {
        END_FUNCTION("CreateIR", SUCCESS );
        return ret;    
    }
    
    ir = &irObject;

    ir->modelId = NULL;
    ir->modelName = NULL;
    ir->modelSubstanceUnits = NULL;
    ir->modelTimeUnits = NULL;
    ir->modelVolumeUnits = NULL;
    ir->modelAreaUnits = NULL;
    ir->modelLengthUnits = NULL;
    ir->modelExtentUnits = NULL;

    if( ( ir->speciesList = CreateLinkedList() ) == NULL ) {                
        return ErrorReport( FAILING, "InitIR", "could not allocate internal data for IR" );
    }
    
    
    if( ( ir->reactionList = CreateLinkedList() ) == NULL ) {
        return ErrorReport( FAILING, "InitIR", "could not allocate internal data for IR" );
    }     

    if( ( ir->reactantEdges = CreateLinkedList() ) == NULL ) {
        return ErrorReport( FAILING, "InitIR", "could not allocate internal data for IR" );
    }     
    if( ( ir->modifierEdges = CreateLinkedList() ) == NULL ) {
        return ErrorReport( FAILING, "InitIR", "could not allocate internal data for IR" );
    }     
    if( ( ir->productEdges = CreateLinkedList() ) == NULL ) {
        return ErrorReport( FAILING, "InitIR", "could not allocate internal data for IR" );
    }     
    
    if( ( ir->globalSymtab = CreateSymtab( NULL ) ) == NULL ) {
        return ErrorReport( FAILING, "InitIR", "could not create a global symtab for IR" );
    }
        
    ir->record = record;

    ir->GetListOfSpeciesNodes = _GetListOfSpeciesNodes;
    ir->GetListOfReactionNodes = _GetListOfReactionNodes;
        
    ir->GetListOfReactantEdges = _GetListOfReactantEdges;    
    ir->GetListOfModifierEdges = _GetListOfModifierEdges;
    ir->GetListOfProductEdges = _GetListOfProductEdges;
    
    ir->SetModelId = _SetModelId;
    ir->SetModelName = _SetModelName;
    ir->SetModelSubstanceUnits = _SetModelSubstanceUnits;
    ir->SetModelTimeUnits = _SetModelTimeUnits;
    ir->SetModelVolumeUnits = _SetModelVolumeUnits;
    ir->SetModelAreaUnits = _SetModelAreaUnits;
    ir->SetModelLengthUnits = _SetModelLengthUnits;
    ir->SetModelExtentUnits = _SetModelExtentUnits;
    ir->GetModelId = _GetModelId;
    ir->GetModelName = _GetModelName;
    ir->GetModelSubstanceUnits = _GetModelSubstanceUnits;
    ir->GetModelTimeUnits = _GetModelTimeUnits;
    ir->GetModelVolumeUnits = _GetModelVolumeUnits;
    ir->GetModelAreaUnits = _GetModelAreaUnits;
    ir->GetModelLengthUnits = _GetModelLengthUnits;
    ir->GetModelExtentUnits = _GetModelExtentUnits;

    ir->CreateSpecies = _CreateSpecies;
    ir->CreateReaction = _CreateReaction;

    ir->CloneSpecies = _CloneSpecies;
    ir->CloneReaction = _CloneReaction;

    ir->RemoveProductInReaction = _RemoveProductInReaction;
    ir->RemoveModifierInReaction = _RemoveModifierInReaction;
    ir->RemoveReactantInReaction = _RemoveReactantInReaction;
    
    ir->AddSpecies = _AddSpecies;
    ir->AddReaction = _AddReaction;
    
    ir->RemoveSpecies = _RemoveSpecies;    
    ir->RemoveReaction = _RemoveReaction;
    ir->RemoveRule = _RemoveRule;    
    
    ir->AddReactantEdge = _AddReactantEdge;
    ir->RemoveReactantEdge = _RemoveReactantEdge;
    
    ir->AddModifierEdge = _AddModifierEdge;    
    ir->RemoveModifierEdge = _RemoveModifierEdge;
        
    ir->AddProductEdge = _AddProductEdge;
    ir->RemoveProductEdge = _RemoveProductEdge;

        
    ir->ResetChangeFlag = _ResetChangeFlag;
    ir->IsStructureChanged = _IsStructureChanged;

    ir->GetUnitManager = _GetUnitManager;
    ir->SetUnitManager = _SetUnitManager;    

    ir->GetFunctionManager = _GetFunctionManager;
    ir->SetFunctionManager = _SetFunctionManager;    

    ir->GetRuleManager = _GetRuleManager;
    ir->SetRuleManager = _SetRuleManager;    

    ir->GetConstraintManager = _GetConstraintManager;
    ir->SetConstraintManager = _SetConstraintManager;    

    ir->GetEventManager = _GetEventManager;
    ir->SetEventManager = _SetEventManager;    
    
    ir->GetCompartmentManager = _GetCompartmentManager;
    ir->SetCompartmentManager = _SetCompartmentManager;    
    ir->GetReactionLawManager = _GetReactionLawManager;
    ir->SetReactionLawManager = _SetReactionLawManager;    
    ir->GetGlobalSymtab = _GetGlobalSymtab;
    
    ir->GenerateDotFile = _GenerateDotFile;
    ir->GenerateSBML = _GenerateSBML; 
    ir->GenerateXHTML = _GenerateXHTML;                    
    
    record->ir = ir; 
      
    END_FUNCTION("CreateIR", SUCCESS );
    return ret;    
}



RET_VAL FreeIR( IR *ir ) {
    RET_VAL ret = SUCCESS;
    LINKED_LIST *list = NULL;
    IR_NODE *node = NULL;
        
    START_FUNCTION("FreeIR");
    
    if( ir == NULL ) {
        END_FUNCTION("FreeIR", SUCCESS );
        return ret;
    }
        
    list = ir->speciesList;
    ResetCurrentElement( list );
    while( ( node = (IR_NODE*)GetNextFromLinkedList( list ) ) != NULL ) {
        if( IS_FAILED( ( ret = ir->RemoveSpecies( ir, (SPECIES*)node ) ) ) ) {
            END_FUNCTION("FreeIR", FAILING );
            return ret;
        }
    }
    FREE( list );
    
    list = ir->reactionList;
    ResetCurrentElement( list );
    while( ( node = (IR_NODE*)GetNextFromLinkedList( list ) ) != NULL ) {
        if( IS_FAILED( ( ret = ir->RemoveReaction( ir, (REACTION*)node ) ) ) ) {
            END_FUNCTION("FreeIR", FAILING );
            return ret;
        }
    }
    FREE( list );
    
    FreeSymtab( &(ir->globalSymtab) );
    
    END_FUNCTION("FreeIR", SUCCESS );
    return ret;
}



DLLSCOPE SPECIES * STDCALL GetSpeciesInIREdge( IR_EDGE *edge ) {    
    return (edge == NULL) ? NULL : (SPECIES*)(edge->species);
}

DLLSCOPE double STDCALL GetStoichiometryInIREdge( IR_EDGE *edge ) {
    return (edge == NULL) ? 0 : edge->stoichiometry;
}

DLLSCOPE REB2SAC_SYMBOL * STDCALL GetSpeciesRefInIREdge( IR_EDGE *edge ) {
    return (edge == NULL) ? 0 : edge->speciesRef;
}

DLLSCOPE REACTION * STDCALL GetReactionInIREdge( IR_EDGE *edge ) {    
    return (edge == NULL) ? NULL : (REACTION*)(edge->reaction);
}

DLLSCOPE RET_VAL STDCALL SetStoichiometryInIREdge( IR_EDGE *edge, double stoichiometry ) {
    edge->stoichiometry = stoichiometry;
    return SUCCESS;
}


static LINKED_LIST * _GetListOfSpeciesNodes( IR *ir ) {
    START_FUNCTION("_GetListOfSpeciesNodes");
    
    END_FUNCTION("_GetListOfSpeciesNodes", SUCCESS );

    return ir->speciesList;
}    

static LINKED_LIST * _GetListOfReactionNodes( IR *ir ) {
    START_FUNCTION("_GetListOfReactionNodes");
    END_FUNCTION("_GetListOfReactionNodes", SUCCESS );

    return ir->reactionList;
}

static LINKED_LIST * _GetListOfReactantEdges( IR *ir ) {
    START_FUNCTION("_GetListOfReactantEdges");
    END_FUNCTION("_GetListOfReactantEdges", SUCCESS );

    return ir->reactantEdges;
}
    
static LINKED_LIST * _GetListOfModifierEdges( IR *ir ) {
    START_FUNCTION("_GetListOfModifierEdges");
    END_FUNCTION("_GetListOfModifierEdges", SUCCESS );

    return ir->modifierEdges;
}

static LINKED_LIST * _GetListOfProductEdges( IR *ir ) {
    START_FUNCTION("_GetListOfProductEdges");
    END_FUNCTION("_GetListOfProductEdges", SUCCESS );

    return ir->productEdges;
}

static RET_VAL _SetModelId( IR *ir, char *modelId ) {
  if ( ( ir->modelId = CreateString( modelId ) ) == NULL ) {
    return FAILING;
  }
  return SUCCESS;
}

static RET_VAL _SetModelName( IR *ir, char *modelName ) {
  if ( ( ir->modelName = CreateString( modelName ) ) == NULL ) {
    return FAILING;
  }
  return SUCCESS;
}

static RET_VAL _SetModelSubstanceUnits( IR *ir, char *modelSubstanceUnits ) {
  if ( ( ir->modelSubstanceUnits = CreateString( modelSubstanceUnits ) ) == NULL ) {
    return FAILING;
  }
  return SUCCESS;
}

static RET_VAL _SetModelTimeUnits( IR *ir, char *modelTimeUnits ) {
  if ( ( ir->modelTimeUnits = CreateString( modelTimeUnits ) ) == NULL ) {
    return FAILING;
  }
  return SUCCESS;
}

static RET_VAL _SetModelVolumeUnits( IR *ir, char *modelVolumeUnits ) {
  if ( ( ir->modelVolumeUnits = CreateString( modelVolumeUnits ) ) == NULL ) {
    return FAILING;
  }
  return SUCCESS;
}

static RET_VAL _SetModelAreaUnits( IR *ir, char *modelAreaUnits ) {
  if ( ( ir->modelAreaUnits = CreateString( modelAreaUnits ) ) == NULL ) {
    return FAILING;
  }
  return SUCCESS;
}

static RET_VAL _SetModelLengthUnits( IR *ir, char *modelLengthUnits ) {
  if ( ( ir->modelLengthUnits = CreateString( modelLengthUnits ) ) == NULL ) {
    return FAILING;
  }
  return SUCCESS;
}

static RET_VAL _SetModelExtentUnits( IR *ir, char *modelExtentUnits ) {
  if ( ( ir->modelExtentUnits = CreateString( modelExtentUnits ) ) == NULL ) {
    return FAILING;
  }
  return SUCCESS;
}

static STRING *_GetModelId( IR *ir ) {
  return ir->modelId;
}

static STRING *_GetModelName( IR *ir ) {
  return ir->modelName;
}

static STRING *_GetModelSubstanceUnits( IR *ir ) {
  return ir->modelSubstanceUnits;
}

static STRING *_GetModelTimeUnits( IR *ir ) {
  return ir->modelTimeUnits;
}

static STRING *_GetModelVolumeUnits( IR *ir ) {
  return ir->modelVolumeUnits;
}

static STRING *_GetModelAreaUnits( IR *ir ) {
  return ir->modelAreaUnits;
}

static STRING *_GetModelLengthUnits( IR *ir ) {
  return ir->modelLengthUnits;
}

static STRING *_GetModelExtentUnits( IR *ir ) {
  return ir->modelExtentUnits;
}
        
static SPECIES *_CreateSpecies( IR *ir, char *name ) {
    SPECIES *speciesNode = NULL;
    
    START_FUNCTION("_CreateSpecies");

    if( ( speciesNode = (SPECIES*)CALLOC( 1, sizeof( SPECIES ) ) ) == NULL ) {
        END_FUNCTION("_CreateSpecies", FAILING );
        return NULL;
    }
        
    if( IS_FAILED( InitSpeciesNode( speciesNode, name ) ) ) {
        FREE( speciesNode );
        END_FUNCTION("_CreateSpecies", FAILING );
        return NULL;
    }
    
    TRACE_1( "species %s is created", name );    
    
    if( IS_FAILED( ( AddElementInLinkedList( (CADDR_T)speciesNode, ir->speciesList ) ) ) ) {        
        FREE( speciesNode );
        END_FUNCTION("_CreateSpecies", FAILING );
        return NULL;
    } 
    
    ir->changeFlag = TRUE;
    END_FUNCTION("_CreateSpecies", SUCCESS );
    
    return speciesNode;
}
    
static REACTION *_CreateReaction( IR *ir, char *name ) {
    REACTION *reactionNode = NULL;
    
    START_FUNCTION("_CreateReaction");

    if( ( reactionNode = (REACTION*)CALLOC( 1, sizeof( REACTION ) ) ) == NULL ) {
        END_FUNCTION("_CreateReaction", FAILING );
        return NULL;
    }
    
    if( IS_FAILED( InitReactionNode( reactionNode, name ) ) ) {
        FREE( reactionNode );
        END_FUNCTION("_CreateReaction", FAILING );
        return NULL;
    }
    
    TRACE_1( "reaction %s is created", name );    
    
    if( IS_FAILED( ( AddElementInLinkedList( (CADDR_T)reactionNode, ir->reactionList ) ) ) ) {
        FREE( reactionNode );
        END_FUNCTION("_CreateReaction", FAILING );
        return NULL;
    } 
                
    ir->changeFlag = TRUE;
    END_FUNCTION("_CreateReaction", SUCCESS );
    
    return reactionNode;
}

static SPECIES* _CloneSpecies( IR *ir, SPECIES *species ) {
    SPECIES *clone = NULL;
    
    START_FUNCTION("_CloneSpecies");
    
    if( ( clone = (SPECIES*)species->Clone( (IR_NODE*)species ) ) == NULL ) {
        END_FUNCTION("_CloneSpecies", FAILING );
        return NULL;
    }
   
    if( IS_FAILED( ( AddElementInLinkedList( (CADDR_T)clone, ir->speciesList ) ) ) ) {        
        FREE( clone );
        END_FUNCTION("_CloneSpecies", FAILING );
        return NULL;
    } 
    ir->changeFlag = TRUE;
        
    END_FUNCTION("_CloneSpecies", SUCCESS );    
    return clone;
}
    
static REACTION* _CloneReaction( IR *ir, REACTION *reaction ) {
    REACTION *clone = NULL;
    
    START_FUNCTION("_CloneReaction");

    if( ( clone = (REACTION*)reaction->Clone( (IR_NODE*)reaction ) ) == NULL ) {
        END_FUNCTION("_CreateReaction", FAILING );
        return NULL;
    }
    
    if( IS_FAILED( ( AddElementInLinkedList( (CADDR_T)clone, ir->reactionList ) ) ) ) {
        FREE( clone );
        END_FUNCTION("_CloneReaction", FAILING );
        return NULL;
    } 
    ir->changeFlag = TRUE;
    
    END_FUNCTION("_CloneReaction", SUCCESS );    
    return clone;
}


static RET_VAL  _AddSpecies( IR *ir, SPECIES *species ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("_AddSpecies");
    
    if( IS_FAILED( ( ret = AddElementInLinkedList( (CADDR_T)species, ir->speciesList ) ) ) ) {
        END_FUNCTION("_AddSpecies", ret );
        return ret;
    } 
    
    END_FUNCTION("_AddSpecies", SUCCESS );
    return ret;    
}

static RET_VAL _RemoveSpecies( IR *ir, SPECIES *species ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("_RemoveSpecies");
    
    if( IS_FAILED( ( ret = RemoveElementFromLinkedList( (CADDR_T)species, ir->speciesList ) ) ) ) {
        END_FUNCTION("_RemoveSpecies", ret );
        return ret;
    } 
        
    if( IS_FAILED( ( ret = species->ReleaseResource( (IR_NODE*)species ) ) ) ) {
        END_FUNCTION("_RemoveSpecies", ret );
        return ret;    
    } 
    
    FREE( species );
            
    ir->changeFlag = TRUE;
    END_FUNCTION("_RemoveSpecies", SUCCESS );
    return ret;    
}

static RET_VAL _RemoveRule( IR *ir, RULE *rule ) {
    RET_VAL ret = SUCCESS;
    RULE_MANAGER *ruleManager;
    LINKED_LIST *ruleList = NULL;

    START_FUNCTION("_RemoveRule");
    
    if( ( ruleManager = ir->GetRuleManager( ir ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not get the rule manager" );
    }
    ruleList = ruleManager->CreateListOfRules( ruleManager );
    if( IS_FAILED( ( ret = RemoveElementFromLinkedList( (CADDR_T)rule, ruleList ) ) ) ) {
        END_FUNCTION("_RemoveRule", ret );
        return ret;
    } 
        
    /*if( IS_FAILED( ( ret = rule->ReleaseResource( (IR_NODE*)rule ) ) ) ) {
        END_FUNCTION("_RemoveRule", ret );
        return ret;    
	} */
    
    FREE( rule );
            
    ir->changeFlag = TRUE;
    END_FUNCTION("_RemoveRule", SUCCESS );
    return ret;    
}

static RET_VAL _AddReaction( IR *ir, REACTION *reaction ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("_AddReaction");
    
    if( IS_FAILED( ( ret = AddElementInLinkedList( (CADDR_T)reaction, ir->reactionList ) ) ) ) {
        END_FUNCTION("_AddReaction", ret );
        return ret;
    } 
    
    END_FUNCTION("_AddReaction", SUCCESS );
    return ret;    
}


static RET_VAL _RemoveReaction( IR *ir, REACTION *reaction ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("_RemoveReaction");
    
    if( IS_FAILED( ( ret = RemoveElementFromLinkedList( (CADDR_T)reaction, ir->reactionList ) ) ) ) {
        END_FUNCTION("_RemoveReaction", ret );
        return ret;
    } 
    
    if( IS_FAILED( ( ret = reaction->ReleaseResource( (IR_NODE*)reaction ) ) ) ) {
        END_FUNCTION("_RemoveReaction", ret );
        return ret;    
    } 
    
    FREE( reaction );
    
    ir->changeFlag = TRUE;
    END_FUNCTION("_RemoveReaction", SUCCESS );
    return ret;    
}

/*
static RET_VAL _AddReactantInReaction( IR *ir, REACTION *reaction, SPECIES *reactant ) {
    RET_VAL ret = SUCCESS;
    IR_NODE *node = NULL;
    LINKED_LIST *list = NULL;
    
    START_FUNCTION("_AddReactantInReaction");
    
    list = GetReactantsInReactionNode( reaction );
    ResetCurrentElement( list );
    while( ( node = (IR_NODE*)GetNextFromLinkedList( list ) ) != NULL ) {
        if( node == reactant ) {
            END_FUNCTION("_AddReactantInReaction", SUCCESS );
            return ret;    
        }
    }
    
    if( IS_FAILED( ( ret = AddElementInLinkedList( (CADDR_T)reactant, reaction->reactants ) ) ) ) {
        END_FUNCTION("_AddReactantInReaction", ret );
        return ret;    
    }
    if( IS_FAILED( ( ret = AddElementInLinkedList( (CADDR_T)reaction, reactant->reactionsAsReactant ) ) ) ) {
        END_FUNCTION("_AddReactantInReaction", ret );
        return ret;    
    }
    
    ir->changeFlag = TRUE;
    END_FUNCTION("_AddReactantInReaction", SUCCESS );
    return ret;    
}


static RET_VAL _AddModifierInReaction( IR *ir, REACTION *reaction, SPECIES *modifier ) {
    RET_VAL ret = SUCCESS;
    IR_NODE *node = NULL;
    LINKED_LIST *list = NULL;
    
    START_FUNCTION("_AddModifierInReaction");
    
    list = GetModifiersInReactionNode( reaction );
    ResetCurrentElement( list );
    while( ( node = (IR_NODE*)GetNextFromLinkedList( list ) ) != NULL ) {
        if( node == modifier ) {
            END_FUNCTION("_AddReactantInReaction", SUCCESS );
            return ret;    
        }
    }
    
    if( IS_FAILED( ( ret = AddElementInLinkedList( (CADDR_T)modifier, reaction->modifiers ) ) ) ) {
        END_FUNCTION("_AddModifierInReaction", ret );
        return ret;    
    }
    if( IS_FAILED( ( ret = AddElementInLinkedList( (CADDR_T)reaction, modifier->reactionsAsModifier ) ) ) ) {
        END_FUNCTION("_AddModifierInReaction", ret );
        return ret;    
    }
    
    ir->changeFlag = TRUE;
    END_FUNCTION("_AddModifierInReaction", SUCCESS );
    return ret;    
}

static RET_VAL _AddProductInReaction( IR *ir, REACTION *reaction, SPECIES *product ) {
    RET_VAL ret = SUCCESS;
    IR_NODE *node = NULL;
    LINKED_LIST *list = NULL;
    
    START_FUNCTION("_AddProductInReaction");
    
    list = GetProductsInReactionNode( reaction );
    ResetCurrentElement( list );
    while( ( node = (IR_NODE*)GetNextFromLinkedList( list ) ) != NULL ) {
        if( node == product ) {
            END_FUNCTION("_AddReactantInReaction", SUCCESS );
            return ret;    
        }
    }
    
    if( IS_FAILED( ( ret = AddElementInLinkedList( (CADDR_T)product, reaction->products ) ) ) ) {
        END_FUNCTION("_AddProductInReaction", ret );
        return ret;    
    }
    if( IS_FAILED( ( ret = AddElementInLinkedList( (CADDR_T)reaction, product->reactionsAsProduct ) ) ) ) {
        END_FUNCTION("_AddProductInReaction", ret );
        return ret;    
    }
    
    ir->changeFlag = TRUE;
    END_FUNCTION("_AddProductInReaction", SUCCESS );
    return ret;    
}
*/

static RET_VAL _RemoveProductInReaction( IR *ir, REACTION *reaction, SPECIES *product ) {
    RET_VAL ret = SUCCESS;
    IR_EDGE *edge = NULL;
    SPECIES *species = NULL;
    LINKED_LIST *edges = NULL;
        
    START_FUNCTION("_RemoveProductInReaction");
    
    edges = GetProductEdges( (IR_NODE*)reaction );
    ResetCurrentElement( edges );
    while( ( edge = GetNextEdge( edges ) ) != NULL ) {
        species = GetSpeciesInIREdge( edge );
        if( species == product ) {
            if( IS_FAILED( ( ret = ir->RemoveProductEdge( ir, &edge ) ) ) ) {
                END_FUNCTION("_RemoveProductInReaction", ret );
                return ret;    
            }
            break;
        }
    }
    ir->changeFlag = TRUE;
    END_FUNCTION("_RemoveProductInReaction", SUCCESS );
    return ret;    
}

static RET_VAL _RemoveModifierInReaction( IR *ir, REACTION *reaction, SPECIES *modifier ) {
    RET_VAL ret = SUCCESS;
    IR_EDGE *edge = NULL;
    SPECIES *species = NULL;
    LINKED_LIST *edges = NULL;
        
    START_FUNCTION("_RemoveModifierInReaction");
    
    edges = GetModifierEdges( (IR_NODE*)reaction );
    ResetCurrentElement( edges );
    while( ( edge = GetNextEdge( edges ) ) != NULL ) {
        species = GetSpeciesInIREdge( edge );
        if( species == modifier ) {
            if( IS_FAILED( ( ret = ir->RemoveModifierEdge( ir, &edge ) ) ) ) {
                END_FUNCTION("_RemoveModifierInReaction", ret );
                return ret;    
            }
            break;
        }
    }
    ir->changeFlag = TRUE;
    END_FUNCTION("_RemoveModifierInReaction", SUCCESS );
    return ret;    
}
        


static RET_VAL _RemoveReactantInReaction( IR *ir, REACTION *reaction, SPECIES *reactant ) {
    RET_VAL ret = SUCCESS;
    IR_EDGE *edge = NULL;
    SPECIES *species = NULL;
    LINKED_LIST *edges = NULL;
        
    START_FUNCTION("_RemoveReactantInReaction");
    
    edges = GetReactantEdges( (IR_NODE*)reaction );
    ResetCurrentElement( edges );
    while( ( edge = GetNextEdge( edges ) ) != NULL ) {
        species = GetSpeciesInIREdge( edge );
        if( species == reactant ) {
            if( IS_FAILED( ( ret = ir->RemoveReactantEdge( ir, &edge ) ) ) ) {
                END_FUNCTION("_RemoveReactantInReaction", ret );
                return ret;    
            }
            break;
        }
    }
    ir->changeFlag = TRUE;
    END_FUNCTION("_RemoveReactantInReaction", SUCCESS );
    return ret;    
}






/*********************/


static RET_VAL _AddReactantEdge( IR *ir, REACTION *reaction, SPECIES *reactant, double stoichiometry,
				 REB2SAC_SYMBOL *speciesRef ) {
    RET_VAL ret = SUCCESS;
    IR_EDGE *edge = NULL;
    LINKED_LIST *list = NULL;
    
    START_FUNCTION("_AddReactantEdge");
    
    list = GetReactantEdges( (IR_NODE*)reaction );
    ResetCurrentElement( list );
    while( ( edge = (IR_EDGE*)GetNextFromLinkedList( list ) ) != NULL ) {
        if( GetSpeciesInIREdge( edge ) == reactant ) {
            edge->stoichiometry = stoichiometry;
            END_FUNCTION("_AddReactantEdge", SUCCESS );
            return ret;    
        }
    }
    
    if( ( edge = CreateReactantEdge( (IR_NODE*)reaction, (IR_NODE*)reactant, stoichiometry, speciesRef
				     ) ) == NULL ) {
        END_FUNCTION("_AddReactantEdge", ret );
        return FAILING;    
    }
    
    if( IS_FAILED( ( ret = AddElementInLinkedList( (CADDR_T)edge, ir->reactantEdges ) ) ) ) {
        END_FUNCTION("_AddReactantEdge", ret );
        return FAILING;    
    }
    
    ir->changeFlag = TRUE;
    END_FUNCTION("_AddReactantEdge", SUCCESS );
    return ret;    
}
        

static RET_VAL _RemoveReactantEdge( IR *ir, IR_EDGE **edge ) {
    RET_VAL ret = SUCCESS;
           
    START_FUNCTION("_RemoveReactantEdge");
    
    if( IS_FAILED( ( ret = FreeReactantEdge(  edge ) ) ) ) {
        END_FUNCTION("_RemoveReactantEdge", ret );
        return ret;    
    }
    
    if( IS_FAILED( ( ret = RemoveElementFromLinkedList( (CADDR_T)(*edge), ir->reactantEdges ) ) ) ) {
        END_FUNCTION("_RemoveReactantEdge", ret );
        return ret;    
    }
    
    ir->changeFlag = TRUE;
    END_FUNCTION("_RemoveReactantEdge", SUCCESS );
    return ret;    
}


static RET_VAL _AddModifierEdge( IR *ir, REACTION *reaction, SPECIES *modifier, double stoichiometry ) {
    RET_VAL ret = SUCCESS;
    LINKED_LIST *list = NULL;
    IR_EDGE *edge = NULL;
    
    START_FUNCTION("_AddModifierEdge");
    
    list = GetModifiersInReactionNode( reaction );
    ResetCurrentElement( list );
    while( ( edge = (IR_EDGE*)GetNextFromLinkedList( list ) ) != NULL ) {
        if( GetSpeciesInIREdge( edge ) == modifier ) {
            edge->stoichiometry = stoichiometry;
            END_FUNCTION("_AddModifierEdge", SUCCESS );
            return ret;    
        }
    }
    
    if( ( edge = CreateModifierEdge( (IR_NODE*)reaction, (IR_NODE*)modifier, stoichiometry ) ) == NULL ) {
        END_FUNCTION("_AddModifierEdge", FAILING );
        return FAILING;    
    }
    
    if( IS_FAILED( ( ret = AddElementInLinkedList( (CADDR_T)edge, ir->modifierEdges ) ) ) ) {
        END_FUNCTION("_AddModifierEdge", ret );
        return FAILING;    
    }
    
    ir->changeFlag = TRUE;
    END_FUNCTION("_AddModifierEdge", SUCCESS );
    return ret;    
}

static RET_VAL _RemoveModifierEdge( IR *ir, IR_EDGE **edge ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("_RemoveModifierEdge");    
    
    if( IS_FAILED( ( ret = FreeModifierEdge( edge ) ) ) ) {
        END_FUNCTION("_RemoveReactantEdge", ret );
        return ret;    
    }
    if( IS_FAILED( ( ret = RemoveElementFromLinkedList( (CADDR_T)(*edge), ir->modifierEdges ) ) ) ) {
        END_FUNCTION("_RemoveReactantEdge", ret );
        return ret;    
    }
    
    ir->changeFlag = TRUE;
    END_FUNCTION("_RemoveModifierEdge", SUCCESS );
    return ret;    
}
        
static RET_VAL _AddProductEdge( IR *ir, REACTION *reaction, SPECIES *product, double stoichiometry, 
				REB2SAC_SYMBOL *speciesRef ) {
    RET_VAL ret = SUCCESS;
    LINKED_LIST *list = NULL;
    IR_EDGE *edge = NULL;
    
    START_FUNCTION("_AddProductEdge");
    
    list = GetProductsInReactionNode( reaction );
    ResetCurrentElement( list );
    while( ( edge = (IR_EDGE*)GetNextFromLinkedList( list ) ) != NULL ) {
        if( GetSpeciesInIREdge( edge ) == product ) {
            edge->stoichiometry = stoichiometry;
            END_FUNCTION("_AddProductEdge", SUCCESS );
            return ret;    
        }
    }
    
    if( ( edge = CreateProductEdge( (IR_NODE*)reaction, (IR_NODE*)product, stoichiometry, speciesRef ) ) == NULL ) {
        END_FUNCTION("_AddProductEdge", FAILING );
        return FAILING;    
    }
    if( IS_FAILED( ( ret = AddElementInLinkedList( (CADDR_T)edge, ir->productEdges ) ) ) ) {
        END_FUNCTION("_AddProductEdge", ret );
        return FAILING;    
    }
    
    ir->changeFlag = TRUE;
    END_FUNCTION("_AddProductEdge", SUCCESS );
    return ret;    
}


static RET_VAL _RemoveProductEdge( IR *ir, IR_EDGE **edge ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("_RemoveProductEdge");
    
    if( IS_FAILED( ( ret = FreeProductEdge(  edge ) ) ) ) {
        END_FUNCTION("_RemoveReactantEdge", ret );
        return ret;    
    }
    if( IS_FAILED( ( ret = RemoveElementFromLinkedList( (CADDR_T)(*edge), ir->productEdges ) ) ) ) {
        END_FUNCTION("_RemoveReactantEdge", ret );
        return ret;    
    }
    ir->changeFlag = TRUE;
    END_FUNCTION("_RemoveProductEdge", SUCCESS );
    return ret;    
}


/*********************/





static void _ResetChangeFlag( IR *ir ) {
    
    START_FUNCTION("_ResetChangeFlag");
            
    ir->changeFlag = FALSE;
    
    END_FUNCTION("_ResetChangeFlag", SUCCESS );
}

static BOOL _IsStructureChanged( IR *ir ) {
    
    START_FUNCTION("_IsStructureChanged");
                
    END_FUNCTION("_IsStructureChanged", SUCCESS );
    return ir->changeFlag;

}



static UNIT_MANAGER *_GetUnitManager( IR *ir ) {
    START_FUNCTION("_GetUnitManager");
            
    if( ir == NULL ) {
        END_FUNCTION("_GetUnitManager", FAILING );
        return NULL;    
    }
        
    END_FUNCTION("_GetUnitManager", SUCCESS );
    return ir->unitManager;
}

static FUNCTION_MANAGER *_GetFunctionManager( IR *ir ) {
    START_FUNCTION("_GetFunctionManager");
            
    if( ir == NULL ) {
        END_FUNCTION("_GetFunctionManager", FAILING );
        return NULL;    
    }
        
    END_FUNCTION("_GetFunctionManager", SUCCESS );
    return ir->functionManager;
}

static RULE_MANAGER *_GetRuleManager( IR *ir ) {
    START_FUNCTION("_GetRuleManager");
            
    if( ir == NULL ) {
        END_FUNCTION("_GetRuleManager", FAILING );
        return NULL;    
    }
        
    END_FUNCTION("_GetRuleManager", SUCCESS );
    return ir->ruleManager;
}

static CONSTRAINT_MANAGER *_GetConstraintManager( IR *ir ) {
    START_FUNCTION("_GetConstraintManager");
            
    if( ir == NULL ) {
        END_FUNCTION("_GetConstraintManager", FAILING );
        return NULL;    
    }
        
    END_FUNCTION("_GetConstraintManager", SUCCESS );
    return ir->constraintManager;
}

static EVENT_MANAGER *_GetEventManager( IR *ir ) {
    START_FUNCTION("_GetEventManager");
            
    if( ir == NULL ) {
        END_FUNCTION("_GetEventManager", FAILING );
        return NULL;    
    }
        
    END_FUNCTION("_GetEventManager", SUCCESS );
    return ir->eventManager;
}

static COMPARTMENT_MANAGER *_GetCompartmentManager( IR *ir ) {
    START_FUNCTION("_GetCompartmentManager");
            
    if( ir == NULL ) {
        END_FUNCTION("_GetCompartmentManager", FAILING );
        return NULL;    
    }
        
    END_FUNCTION("_GetCompartmentManager", SUCCESS );
    return ir->compartmentManager;
}

static REACTION_LAW_MANAGER *_GetReactionLawManager( IR *ir ) {
    START_FUNCTION("_GetReactionLawManager");
            
    if( ir == NULL ) {
        END_FUNCTION("_GetReactionLawManager", FAILING );
        return NULL;    
    }
        
    END_FUNCTION("_GetReactionLawManager", SUCCESS );
    return ir->reactionLawManager;
}

static REB2SAC_SYMTAB *_GetGlobalSymtab( IR *ir ) {
    START_FUNCTION("_GetGlobalSymtab");
            
    if( ir == NULL ) {
        END_FUNCTION("_GetGlobalSymtab", FAILING );
        return NULL;    
    }
        
    END_FUNCTION("_GetGlobalSymtab", SUCCESS );
    return ir->globalSymtab;
}


static RET_VAL _SetUnitManager( IR *ir, UNIT_MANAGER *unitManager ) {
    START_FUNCTION("_SetUnitManager");
            
    if( ir == NULL ) {
        return ErrorReport( FAILING, "_SetUnitManager", "input ir is null" );
    }
    ir->unitManager = unitManager;
            
    END_FUNCTION("_SetUnitManager", SUCCESS );
    return SUCCESS;;
}

static RET_VAL _SetFunctionManager( IR *ir, FUNCTION_MANAGER *functionManager ) {
    START_FUNCTION("_SetFunctionManager");
            
    if( ir == NULL ) {
        return ErrorReport( FAILING, "_SetFunctionManager", "input ir is null" );
    }
    ir->functionManager = functionManager;
            
    END_FUNCTION("_SetFunctionManager", SUCCESS );
    return SUCCESS;;
}

static RET_VAL _SetRuleManager( IR *ir, RULE_MANAGER *ruleManager ) {
    START_FUNCTION("_SetRuleManager");
            
    if( ir == NULL ) {
        return ErrorReport( FAILING, "_SetRuleManager", "input ir is null" );
    }
    ir->ruleManager = ruleManager;
            
    END_FUNCTION("_SetFunctionManager", SUCCESS );
    return SUCCESS;;
}

static RET_VAL _SetConstraintManager( IR *ir, CONSTRAINT_MANAGER *constraintManager ) {
    START_FUNCTION("_SetConstraintManager");
            
    if( ir == NULL ) {
        return ErrorReport( FAILING, "_SetConstraintManager", "input ir is null" );
    }
    ir->constraintManager = constraintManager;
            
    END_FUNCTION("_SetConstraintManager", SUCCESS );
    return SUCCESS;;
}

static RET_VAL _SetEventManager( IR *ir, EVENT_MANAGER *eventManager ) {
    START_FUNCTION("_SetEventManager");
            
    if( ir == NULL ) {
        return ErrorReport( FAILING, "_SetEventManager", "input ir is null" );
    }
    ir->eventManager = eventManager;
            
    END_FUNCTION("_SetEventManager", SUCCESS );
    return SUCCESS;;
}

static RET_VAL _SetCompartmentManager( IR *ir, COMPARTMENT_MANAGER *compartmentManager ) {
    START_FUNCTION("_SetCompartmentManager");
            
    if( ir == NULL ) {
        return ErrorReport( FAILING, "_SetCompartmentManager", "input ir is null" );
    }
    ir->compartmentManager = compartmentManager;
            
    END_FUNCTION("_SetCompartmentManager", SUCCESS );
    return SUCCESS;;
}

static RET_VAL _SetReactionLawManager( IR *ir, REACTION_LAW_MANAGER *reactionLawManager ) {
    START_FUNCTION("_SetReactionLawManager");
            
    if( ir == NULL ) {
        return ErrorReport( FAILING, "_SetReactionLawManager", "input ir is null" );
    }
    ir->reactionLawManager = reactionLawManager;
            
    END_FUNCTION("_SetReactionLawManager", SUCCESS );
    return SUCCESS;;
}


static RET_VAL _GenerateDotFile( IR *ir, FILE *file ) {
    RET_VAL ret = SUCCESS;
    UINT32 i = 0;
    UINT32 num = 0;
    double stoichiometry = 0;
    STRING *kineticLawString = NULL;
    LINKED_LIST *speciesList = NULL;
    LINKED_LIST *reactionList = NULL;
    REACTION *reactionNode = NULL;
    SPECIES *speciesNode = NULL;
    IR_EDGE *edge = NULL;
    LINKED_LIST *edgeList = NULL;
            
    START_FUNCTION("_GenerateDotFile");
    
    speciesList = ir->speciesList;
    reactionList = ir->reactionList;
    
    fprintf( file, "digraph G {%s", NEW_LINE );
    fprintf( file, "\tsize=\"7.0,15,0\";%s", NEW_LINE );
    
#if 0    
    ResetCurrentElement( speciesList );
    while( ( speciesNode = (SPECIES*)GetNextFromLinkedList( speciesList ) ) != NULL ) {
        fprintf( file, "\t%s [label=\"%s\", shape=circle];" NEW_LINE, 
            GetCharArrayOfString( GetSpeciesNodeID( speciesNode ) ), GetCharArrayOfString( GetSpeciesNodeName( speciesNode ) ) );                                         
    }
    
    ResetCurrentElement( reactionList );
    while( ( reactionNode = (REACTION*)GetNextFromLinkedList( reactionList ) ) != NULL ) {
        kineticLawString = ToStringKineticLaw( reactionNode->kineticLaw );
        if( GetStringLength( kineticLawString ) < 50 ) {
            fprintf( file, "\t%s [label=\"%s\\n{%s}\", shape=box];" NEW_LINE, 
                GetCharArrayOfString( GetReactionNodeID( reactionNode ) ), GetCharArrayOfString( GetReactionNodeName( reactionNode ) ), GetCharArrayOfString( kineticLawString ) ); 
        }
        else {
            fprintf( file, "\t%s [label=\"%s\", shape=box];" NEW_LINE, 
                GetCharArrayOfString( GetReactionNodeID( reactionNode ) ), GetCharArrayOfString( GetReactionNodeName( reactionNode ) ) ); 
        }
        FreeString( &kineticLawString );
                                    
        if( reactionNode->isReversible ) {
            edgeList = reactionNode->reactants;
            ResetCurrentElement( edgeList );
            while( ( edge = (IR_EDGE*)GetNextFromLinkedList( edgeList ) ) != NULL ) {
                speciesNode = GetSpeciesInIREdge( edge );
                stoichiometry = GetStoichiometryInIREdge( edge );
                if( stoichiometry == 1.0 ) {
                    fprintf( file, "\t%s -> %s [label = \"r\", dir=both];" NEW_LINE, 
                        GetCharArrayOfString( GetSpeciesNodeID( speciesNode ) ), GetCharArrayOfString( GetReactionNodeID( reactionNode ) ));
                }
                else {
                    fprintf( file, "\t%s -> %s [label = \"%g,r\", dir=both];" NEW_LINE, 
                        GetCharArrayOfString( GetSpeciesNodeID( speciesNode ) ), GetCharArrayOfString( GetReactionNodeID( reactionNode ) ), stoichiometry );
                }
            }
            
            edgeList = reactionNode->products;
            ResetCurrentElement( edgeList );
            while( ( edge = (IR_EDGE*)GetNextFromLinkedList( edgeList ) ) != NULL ) {
                speciesNode = GetSpeciesInIREdge( edge );
                stoichiometry = GetStoichiometryInIREdge( edge );
                if( stoichiometry == 1.0 ) {
                    fprintf( file, "\t%s -> %s [label = \"p\", dir=both];" NEW_LINE, 
                        GetCharArrayOfString( GetReactionNodeID( reactionNode ) ), GetCharArrayOfString( GetSpeciesNodeID( speciesNode ) )  );
                }
                else {
                    fprintf( file, "\t%s -> %s [label = \"%g,p\", dir=both];" NEW_LINE, 
                        GetCharArrayOfString( GetReactionNodeID( reactionNode ) ), GetCharArrayOfString( GetSpeciesNodeID( speciesNode ) ), stoichiometry  );
                }
            }
        }
        else {
            edgeList = reactionNode->reactants;
            ResetCurrentElement( edgeList );
            while( ( edge = (IR_EDGE*)GetNextFromLinkedList( edgeList ) ) != NULL ) {
                speciesNode = GetSpeciesInIREdge( edge );
                stoichiometry = GetStoichiometryInIREdge( edge );
                if( stoichiometry == 1.0 ) {
                    fprintf( file, "\t%s -> %s [label = \"r\"];" NEW_LINE, 
                        GetCharArrayOfString( GetSpeciesNodeID( speciesNode ) ), GetCharArrayOfString( GetReactionNodeID( reactionNode ) ) );
                }
                else {
                    fprintf( file, "\t%s -> %s [label = \"%g,r\"];" NEW_LINE, 
                        GetCharArrayOfString( GetSpeciesNodeID( speciesNode ) ), GetCharArrayOfString( GetReactionNodeID( reactionNode ) ), stoichiometry );
                }
            }
            
            edgeList = reactionNode->products;
            ResetCurrentElement( edgeList );
            while( ( edge = (IR_EDGE*)GetNextFromLinkedList( edgeList ) ) != NULL ) {
                speciesNode = GetSpeciesInIREdge( edge );
                stoichiometry = GetStoichiometryInIREdge( edge );
                if( stoichiometry == 1.0 ) {
                    fprintf( file, "\t%s -> %s [label = \"p\"];" NEW_LINE, 
                        GetCharArrayOfString( GetReactionNodeID( reactionNode ) ), GetCharArrayOfString( GetSpeciesNodeID( speciesNode ) ) );
                }
                else {
                    fprintf( file, "\t%s -> %s [label = \"%g, p\"];" NEW_LINE, 
                        GetCharArrayOfString( GetReactionNodeID( reactionNode ) ), GetCharArrayOfString( GetSpeciesNodeID( speciesNode ) ), stoichiometry );
                }
            }
        }
        
        edgeList = reactionNode->modifiers;
        ResetCurrentElement( edgeList );
        while( ( edge = (IR_EDGE*)GetNextFromLinkedList( edgeList ) ) != NULL ) {
            speciesNode = GetSpeciesInIREdge( edge );
            fprintf( file, "\t%s -> %s [label = \"m\"];" NEW_LINE, 
                GetCharArrayOfString( GetSpeciesNodeID( speciesNode ) ), GetCharArrayOfString( GetReactionNodeID( reactionNode ) ) );
        }                
    }
#endif
    ResetCurrentElement( speciesList );
    while( ( speciesNode = (SPECIES*)GetNextFromLinkedList( speciesList ) ) != NULL ) {
        fprintf( file, "\tx%X [label=\"%s\", shape=circle];" NEW_LINE, 
                 speciesNode, GetCharArrayOfString( GetSpeciesNodeName( speciesNode ) ) );                                         
    }
    
    ResetCurrentElement( reactionList );
    while( ( reactionNode = (REACTION*)GetNextFromLinkedList( reactionList ) ) != NULL ) {
        kineticLawString = ToStringKineticLaw( reactionNode->kineticLaw );
        if( GetStringLength( kineticLawString ) < 50 ) {
            fprintf( file, "\tx%X [label=\"%s\\n{%s}\", shape=box];" NEW_LINE, 
                     reactionNode, GetCharArrayOfString( GetReactionNodeName( reactionNode ) ), GetCharArrayOfString( kineticLawString ) ); 
        }
        else {
            fprintf( file, "\tx%X [label=\"%s\", shape=box];" NEW_LINE, 
                     reactionNode, GetCharArrayOfString( GetReactionNodeName( reactionNode ) ) ); 
        }
        FreeString( &kineticLawString );
                                    
        if( reactionNode->isReversible ) {
            edgeList = reactionNode->reactants;
            ResetCurrentElement( edgeList );
            while( ( edge = (IR_EDGE*)GetNextFromLinkedList( edgeList ) ) != NULL ) {
                speciesNode = GetSpeciesInIREdge( edge );
                stoichiometry = GetStoichiometryInIREdge( edge );
                if( stoichiometry == 1.0 ) {
                    fprintf( file, "\tx%X -> x%X [label = \"r\", dir=both];" NEW_LINE, 
                             speciesNode, reactionNode );
                }
                else {
                    fprintf( file, "\tx%X -> x%X [label = \"%g,r\", dir=both];" NEW_LINE, 
                             speciesNode, reactionNode, stoichiometry );
                }
            }
            
            edgeList = reactionNode->products;
            ResetCurrentElement( edgeList );
            while( ( edge = (IR_EDGE*)GetNextFromLinkedList( edgeList ) ) != NULL ) {
                speciesNode = GetSpeciesInIREdge( edge );
                stoichiometry = GetStoichiometryInIREdge( edge );
                if( stoichiometry == 1 ) {
                    fprintf( file, "\tx%X -> x%X [label = \"p\", dir=both];" NEW_LINE, 
                             reactionNode, speciesNode );
                }
                else {
                    fprintf( file, "\tx%X -> x%X [label = \"%g,p\", dir=both];" NEW_LINE, 
                             reactionNode, speciesNode, stoichiometry  );
                }
            }
        }
        else {
            edgeList = reactionNode->reactants;
            ResetCurrentElement( edgeList );
            while( ( edge = (IR_EDGE*)GetNextFromLinkedList( edgeList ) ) != NULL ) {
                speciesNode = GetSpeciesInIREdge( edge );
                stoichiometry = GetStoichiometryInIREdge( edge );
                if( stoichiometry == 1 ) {
                    fprintf( file, "\tx%X -> x%X [label = \"r\"];" NEW_LINE, 
                             speciesNode, reactionNode );
                }
                else {
                    fprintf( file, "\tx%X -> x%X [label = \"%g,r\"];" NEW_LINE, 
                             speciesNode, reactionNode, stoichiometry );
                }
            }
            
            edgeList = reactionNode->products;
            ResetCurrentElement( edgeList );
            while( ( edge = (IR_EDGE*)GetNextFromLinkedList( edgeList ) ) != NULL ) {
                speciesNode = GetSpeciesInIREdge( edge );
                stoichiometry = GetStoichiometryInIREdge( edge );
                if( stoichiometry == 1 ) {
                    fprintf( file, "\tx%X -> x%X [label = \"p\"];" NEW_LINE, 
                             reactionNode, speciesNode );
                }
                else {
                    fprintf( file, "\tx%X -> x%X [label = \"%g, p\"];" NEW_LINE, 
                             reactionNode, speciesNode, stoichiometry );
                }
            }
        }
        
        edgeList = reactionNode->modifiers;
        ResetCurrentElement( edgeList );
        while( ( edge = (IR_EDGE*)GetNextFromLinkedList( edgeList ) ) != NULL ) {
            speciesNode = GetSpeciesInIREdge( edge );
            fprintf( file, "\tx%X -> x%X [label = \"m\"];" NEW_LINE, 
                     speciesNode, reactionNode );
        }                
    }
    
    fprintf( file, "}%s", NEW_LINE );

    END_FUNCTION("_GenerateDotFile", SUCCESS );
    return ret;    
}
    


static RET_VAL _GenerateSBML( IR *ir, FILE *file, char *filename ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("_GenerateSBML");
    
    fprintf( file, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>%s"
                   "<sbml xmlns=\"http://www.sbml.org/sbml/level2\" level=\"2\" version=\"3\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">%s", 
                   NEW_LINE, NEW_LINE );
    _PrintTab( file, 1 );
    fprintf( file, "<model>%s", NEW_LINE );
    //    fprintf( file, "<model id=\"%s\">%s", filename, NEW_LINE );

    if( IS_FAILED( ( ret = _PrintListOfUnitDefinitionsForSBML( ir, file, 2 ) ) ) ) {
        END_FUNCTION("_GenerateSBML", ret );
        return ret;    
    } 
    fprintf( file, NEW_LINE );

    if( IS_FAILED( ( ret = _PrintListOfCompartmentsForSBML( ir, file, 2 ) ) ) ) {
        END_FUNCTION("_GenerateSBML", ret );
        return ret;    
    } 
    fprintf( file, NEW_LINE );
        
    if( IS_FAILED( ( ret = _PrintListOfSpeciesForSBML( ir, file, 2 ) ) ) ) {
        END_FUNCTION("_GenerateSBML", ret );
        return ret;    
    } 
    fprintf( file, NEW_LINE );
    
    if( IS_FAILED( ( ret = _PrintListOfParametersForSBML( ir, file, 2 ) ) ) ) {
        END_FUNCTION("_GenerateSBML", ret );
        return ret;    
    } 
    fprintf( file, NEW_LINE );

    if( IS_FAILED( ( ret = _PrintListOfRulesForSBML( ir, file, 2 ) ) ) ) {
        END_FUNCTION("_GenerateSBML", ret );
        return ret;    
    } 
    fprintf( file, NEW_LINE );

    if( IS_FAILED( ( ret = _PrintListOfConstraintsForSBML( ir, file, 2 ) ) ) ) {
        END_FUNCTION("_GenerateSBML", ret );
        return ret;    
    } 
    fprintf( file, NEW_LINE );
    
    if( IS_FAILED( ( ret = _PrintListOfReactionsForSBML( ir, file, 2 ) ) ) ) {
        END_FUNCTION("_GenerateSBML", ret );
        return ret;    
    } 

    if( IS_FAILED( ( ret = _PrintListOfEventsForSBML( ir, file, 2 ) ) ) ) {
        END_FUNCTION("_GenerateSBML", ret );
        return ret;    
    } 
    fprintf( file, NEW_LINE );
    
    _PrintTab( file, 1 );
    fprintf( file, "</model>%s", NEW_LINE );
    fprintf( file, "</sbml>%s", NEW_LINE );

    END_FUNCTION("_GenerateSBML", SUCCESS );
    return ret;    
}          


static void _PrintTab( FILE *file, UINT32 tabCount ) {
    UINT32 i = 0;
    
    for( i = 0; i < tabCount; i++ ) {
        fprintf( file, __IR_TAB );
    }
}

static RET_VAL _PrintListOfUnitDefinitionsForSBML( IR *ir, FILE *file, UINT32 tabCount ) {
    RET_VAL ret = SUCCESS;
    UNIT_MANAGER *unitManager = NULL;
    UNIT_DEFINITION *unitDef = NULL;
    LINKED_LIST *unitDefList = NULL;
    int count = 0;
 
    START_FUNCTION("_PrintListOfUnitDefinitionsForSBML");
       
    unitManager = ir->unitManager;
    unitDefList = unitManager->CreateListOfUnitDefinitions( unitManager );
    
    if( GetLinkedListSize( unitDefList ) == 0 ) {
        DeleteLinkedList( &unitDefList );
        END_FUNCTION("_PrintListOfUnitDefinitionsForSBML", SUCCESS );    
        return ret;
    }
    count = 0;
    ResetCurrentElement( unitDefList );
    while( ( unitDef = (UNIT_DEFINITION*)GetNextFromLinkedList( unitDefList ) ) != NULL ) {
      if (IsBuiltInUnitDefinition( unitDef )) continue;
      count++;
    }    
    if (count == 0) {
        DeleteLinkedList( &unitDefList );
        END_FUNCTION("_PrintListOfUnitDefinitionsForSBML", SUCCESS );    
        return ret;
    }

    _PrintTab( file, tabCount );
    fprintf( file, "<listOfUnitDefinitions>%s", NEW_LINE );
    
    ResetCurrentElement( unitDefList );
    while( ( unitDef = (UNIT_DEFINITION*)GetNextFromLinkedList( unitDefList ) ) != NULL ) {
      if (IsBuiltInUnitDefinition( unitDef )) continue;
      if( IS_FAILED( ( ret = _PrintUnitDefinitionForSBML( unitDef, file, tabCount + 1 ) ) ) ) {
	END_FUNCTION("_PrintListOfUnitDefinitionsForSBML", ret );    
	return ret;        
      }                        
    }    
    _PrintTab( file, tabCount );
    fprintf( file, "</listOfUnitDefinitions>%s", NEW_LINE );
                
    DeleteLinkedList( &unitDefList );
    fflush( file );
    
    END_FUNCTION("_PrintListOfUnitDefinitionsForSBML", SUCCESS );    
    return ret;
}

static RET_VAL _PrintUnitDefinitionForSBML( UNIT_DEFINITION *unitDef, FILE *file, UINT32 tabCount ) {
    RET_VAL ret = SUCCESS;
    UNIT *unit = NULL;
    LINKED_LIST *unitList = NULL;
    
    START_FUNCTION("_PrintUnitDefinitionForSBML");
        
    _PrintTab( file, tabCount );
    fprintf( file, "<unitDefinition id=\"%s\">%s", GetCharArrayOfString( GetUnitDefinitionID( unitDef ) ), NEW_LINE );
        
    unitList = GetUnitsInUnitDefinition( unitDef );
    if( ( unitList == NULL ) || ( GetLinkedListSize( unitList ) == 0 ) ) {
        END_FUNCTION("_PrintUnitDefinitionForSBML", SUCCESS );    
        return ret;
    }
    
    _PrintTab( file, tabCount + 1);
    fprintf( file, "<listOfUnits>%s", NEW_LINE );
    
    ResetCurrentElement( unitList );
    while( ( unit = (UNIT*)GetNextFromLinkedList( unitList ) ) != NULL ) {
        if( IS_FAILED( ( ret = _PrintUnitForSBML( unit, file, tabCount + 2 ) ) ) ) {
            END_FUNCTION("_PrintListOfUnitDefinitionsForSBML", ret );    
            return ret;        
        }                        
    }    
    _PrintTab( file, tabCount + 1);
    fprintf( file, "</listOfUnits>%s", NEW_LINE );
    
    _PrintTab( file, tabCount );
    fprintf( file, "</unitDefinition>%s", NEW_LINE );
    
    END_FUNCTION("_PrintUnitDefinitionForSBML", SUCCESS );    
    return ret;
}

static RET_VAL _PrintUnitForSBML( UNIT *unit, FILE *file, UINT32 tabCount ) {
    RET_VAL ret = SUCCESS;
    int exponent = 0;
    int scale = 0;
    double multiplier = 0.0;
    double offset = 0.0;
    
    START_FUNCTION("_PrintUnitForSBML");
    
    _PrintTab( file, tabCount );
    fprintf( file, "<unit kind=\"%s\"", GetCharArrayOfString( GetKindInUnit( unit ) ) );
    
    exponent = GetExponentInUnit( unit );
    if( exponent != 1 ) {
        fprintf( file, " exponent=\"%i\"", exponent );
    }
    
    scale = GetScaleInUnit( unit );
    if( scale != 0 ) {
        fprintf( file, " scale=\"%i\"", scale );
    }
    
    multiplier = GetMultiplierInUnit( unit );
    if( multiplier != 1.0 ) {
        fprintf( file, " multiplier=\"%g\"", multiplier );
    }

    fprintf( file, "/>%s", NEW_LINE );
            
    END_FUNCTION("_PrintUnitForSBML", SUCCESS );
    
    return ret;
}

static RET_VAL _PrintListOfCompartmentsForSBML( IR *ir, FILE *file, UINT32 tabCount ) {
    RET_VAL ret = SUCCESS;
    COMPARTMENT *compartment = NULL;
    LINKED_LIST *list = NULL;
    COMPARTMENT_MANAGER *compartmentManager = NULL;
    
    START_FUNCTION("_PrintListOfCompartmentsForSBML");
    
    compartmentManager = ir->compartmentManager;
    list = compartmentManager->CreateListOfCompartments( compartmentManager );
    
    if( GetLinkedListSize( list ) == 0 ) {
        DeleteLinkedList( &list );
        END_FUNCTION("_PrintListOfCompartmentsForSBML", SUCCESS );    
        return ret;
    }
    _PrintTab( file, tabCount );
    fprintf( file, "<listOfCompartments>%s", NEW_LINE );
    
    ResetCurrentElement( list );
    while( ( compartment = (COMPARTMENT*)GetNextFromLinkedList( list ) ) != NULL ) {
        if( IS_FAILED( ( ret = _PrintCompartmentsForSBML( compartment, file, tabCount + 1 ) ) ) ) {
            END_FUNCTION("_PrintListOfCompartmentsForSBML", ret );    
            return ret;
        }
    }
    
    _PrintTab( file, tabCount );
    fprintf( file, "</listOfCompartments>%s", NEW_LINE );
    fflush( file );    
    DeleteLinkedList( &list );
    
    END_FUNCTION("_PrintListOfCompartmentsForSBML", SUCCESS );    
    return ret;
}

static RET_VAL _PrintCompartmentsForSBML( COMPARTMENT *compartment, FILE *file, UINT32 tabCount ) {
    RET_VAL ret = SUCCESS;
    int spatialDimensions = 0;
    double size = 0.0;
    UNIT_DEFINITION *unit = NULL;
    STRING *outside = NULL;
    BOOL constant = FALSE;                
    
    START_FUNCTION("_PrintCompartmentsForSBML");
    
    _PrintTab( file, tabCount );
    fprintf( file, "<compartment id=\"%s\"", GetCharArrayOfString( GetCompartmentID( compartment ) ) );
    
    spatialDimensions = GetSpatialDimensionsInCompartment( compartment );
    if( spatialDimensions != 3 ) {
        fprintf( file, " spatialDimensions=\"%i\"", spatialDimensions );
    }
    
    size = GetSizeInCompartment( compartment );
    if( size >= 0.0 ) {
        fprintf( file, " size=\"%g\"", size );
    }
            
    unit = GetUnitInCompartment( compartment );
    if( unit != NULL ) {
        fprintf( file, " units=\"%s\"", GetCharArrayOfString( GetUnitDefinitionID( unit ) ) );
    }
    
    outside = GetOutsideInCompartment( compartment );
    if( outside != NULL ) {
        fprintf( file, " outside=\"%s\"", GetCharArrayOfString( outside ) );
    }
    
    constant = IsCompartmentConstant( compartment );
    if( !constant ) {
        fprintf( file, " outside=\"false\"" );
    }
    fprintf( file, "/>%s", NEW_LINE );                
    
    END_FUNCTION("_PrintCompartmentsForSBML", SUCCESS );
    
    return ret;
}


static RET_VAL _PrintListOfSpeciesForSBML( IR *ir, FILE *file, UINT32 tabCount ) {
    RET_VAL ret = SUCCESS;
    SPECIES *species = NULL;
    LINKED_LIST *list = NULL;
    
    START_FUNCTION("_PrintListOfSpeciesForSBML");
    
    list = ir->GetListOfSpeciesNodes( ir );
    if( ( list == NULL ) || ( GetLinkedListSize( list ) == 0 ) ) {
        END_FUNCTION("_PrintListOfSpeciesForSBML", SUCCESS );    
        return ret;
    }    
    
    _PrintTab( file, tabCount );
    fprintf( file, "<listOfSpecies>%s", NEW_LINE );

    ResetCurrentElement( list );
    while( ( species = (SPECIES*)GetNextFromLinkedList( list ) ) != NULL ) {
        if( IS_FAILED( ( ret = _PrintSpeciesForSBML( species, file, tabCount + 1 ) ) ) ) {
            END_FUNCTION("_PrintListOfSpeciesForSBML", ret );    
            return ret;
        }
    }
        
    _PrintTab( file, tabCount );
    fprintf( file, "</listOfSpecies>%s", NEW_LINE );
    
    END_FUNCTION("_PrintListOfSpeciesForSBML", SUCCESS );    
    return ret;
}

static RET_VAL _PrintSpeciesForSBML( SPECIES *species, FILE *file, UINT32 tabCount ) {
    RET_VAL ret = SUCCESS;
    double initialQuantity = 0.0;
    COMPARTMENT *compartment = NULL;
    UNIT_DEFINITION *unitDef = NULL;
    BOOL flag = FALSE;
    char buf[64];
    
    START_FUNCTION("_PrintSpeciesForSBML");
    
    _PrintTab( file, tabCount );
    fprintf( file, "<species id=\"%s\"", GetCharArrayOfString( GetSpeciesNodeID( species ) ) );
    
    fprintf( file, " name=\"%s\"", GetCharArrayOfString( GetSpeciesNodeName( species ) ) );
    
    compartment = GetCompartmentInSpeciesNode( species );
    fprintf( file, " compartment=\"%s\"", GetCharArrayOfString( GetCompartmentID( compartment ) ) );
    
    if( IsInitialQuantityInAmountInSpeciesNode( species ) ) {
        initialQuantity = GetInitialAmountInSpeciesNode( species );
        if( initialQuantity >= 0.0 ) {
            fprintf( file, " initialAmount=\"%g\"", initialQuantity );
        }
    }
    else {
        initialQuantity = GetInitialConcentrationInSpeciesNode( species );
        if( initialQuantity >= 0.0 ) {
            fprintf( file, " initialConcentration=\"%g\"", initialQuantity );
        }
    }

    unitDef = GetSubstanceUnitsInSpeciesNode( species );
    if( unitDef != NULL ) {
        fprintf( file, " substanceUnits=\"%s\"", GetCharArrayOfString( GetUnitDefinitionID( unitDef ) ) );
    } 
        
    unitDef = GetSpatialSizeUnitsInSpeciesNode( species );
    if( unitDef != NULL ) {
        fprintf( file, " spatialSizeUnits=\"%s\"", GetCharArrayOfString( GetUnitDefinitionID( unitDef ) ) );
    } 
    
    flag = HasOnlySubstanceUnitsInSpeciesNode( species );
    if( flag ) {
        fprintf( file, " hasOnlySubstanceUnits=\"true\"" );
    }
    
    flag = HasBoundaryConditionInSpeciesNode( species );
    if( flag ) {
        fprintf( file, " boundaryCondition=\"true\"" );
    }
    
    flag = IsSpeciesNodeConstant( species );
    if( flag ) {
        fprintf( file, " constant=\"true\"" );
    }
        
    fprintf( file, "/>%s", NEW_LINE );                
            
    END_FUNCTION("_PrintSpeciesForSBML", SUCCESS );
    
    return ret;
}

static RET_VAL _PrintListOfParametersForSBML( IR *ir, FILE *file, UINT32 tabCount ) {
    RET_VAL ret = SUCCESS;
    REB2SAC_SYMTAB *symtab = NULL;
    REB2SAC_SYMBOL *sym = NULL;
    LINKED_LIST *list = NULL;
    
    START_FUNCTION("_PrintListOfParametersForSBML");
    
    symtab = ir->GetGlobalSymtab( ir );
    if( ( list = symtab->GenerateListOfSymbols( symtab ) ) == NULL ) {
        return ErrorReport( FAILING, "_PrintListOfParametersForSBML", "could not generate a list of symbols" );
    }
    
    if( GetLinkedListSize( list ) <=  1 ) {
        END_FUNCTION("_PrintListOfParametersForSBML", SUCCESS );    
        return ret;
    }    
    
    _PrintTab( file, tabCount );
    fprintf( file, "<listOfParameters>%s", NEW_LINE );

    ResetCurrentElement( list );
    while( ( sym = (REB2SAC_SYMBOL*)GetNextFromLinkedList( list ) ) != NULL ) {
      if ( strcmp(GetCharArrayOfString ( GetSymbolID( sym ) ),"t")==0) continue;
      if ( strcmp(GetCharArrayOfString ( GetSymbolID( sym ) ),"time")==0) continue;
      if( IS_FAILED( ( ret = _PrintParameterForSBML( sym, file, tabCount + 1 ) ) ) ) {
	END_FUNCTION("_PrintListOfParametersForSBML", ret );    
	return ret;
      }
    }
        
    _PrintTab( file, tabCount );
    fprintf( file, "</listOfParameters>%s", NEW_LINE );
    
    END_FUNCTION("_PrintListOfParametersForSBML", SUCCESS );    
    return ret;
}

static RET_VAL _PrintParameterForSBML( REB2SAC_SYMBOL *sym, FILE *file, UINT32 tabCount ) {
    RET_VAL ret = SUCCESS;
    double value = 0.0;
    char *id = NULL;
    
    START_FUNCTION("_PrintParameterForSBML");
#if 0    
    TRACE_1("printing paramter %s", GetCharArrayOfString( GetSymbolID( sym ) ) );
#endif     
    if( !IsSymbolConstant( sym ) ) {
        END_FUNCTION("_PrintParameterForSBML", SUCCESS );    
        return ret;
    }
    if( !IsRealValueSymbol( sym ) ) {
        END_FUNCTION("_PrintParameterForSBML", SUCCESS );    
        return ret;
    }
    
    id = GetCharArrayOfString( GetSymbolID( sym ) );
    value = GetRealValueInSymbol( sym );
    
    _PrintTab( file, tabCount );
    fprintf( file, "<parameter id=\"%s\" value=\"%g\"/>" NEW_LINE, id, value );
            
    END_FUNCTION("_PrintParameterForSBML", SUCCESS );    
    return ret;
}

static RET_VAL _PrintListOfEventsForSBML( IR *ir, FILE *file, UINT32 tabCount ) {
    RET_VAL ret = SUCCESS;
    EVENT *event = NULL;
    EVENT_MANAGER *eventManager;
    LINKED_LIST *list = NULL;
    
    START_FUNCTION("_PrintListOfEventsForSBML");

    if( ( eventManager = ir->GetEventManager( ir ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not get the event manager" );
    }
    list = eventManager->CreateListOfEvents( eventManager );
    if( ( list == NULL ) || ( GetLinkedListSize( list ) == 0 ) ) {
        END_FUNCTION("_PrintListOfRulesForSBML", SUCCESS );    
        return ret;
    }    

    _PrintTab( file, tabCount );
    fprintf( file, "<listOfEvents>%s", NEW_LINE );
    ResetCurrentElement( list );
    while( ( event = (EVENT*)GetNextFromLinkedList( list ) ) != NULL ) {
      if( IS_FAILED( ( ret = _PrintEventForSBML( event, file, tabCount+ 1 ) ) ) ) {
            END_FUNCTION("_PrintListOfEventsForSBML", ret );    
            return ret;
        }
    }
    
    _PrintTab( file, tabCount );
    fprintf( file, "</listOfEvents>%s", NEW_LINE );
    
    END_FUNCTION("_PrintListOfEventsForSBML", SUCCESS );
    return ret;
}

static RET_VAL _PrintEventForSBML( EVENT *event, FILE *file, UINT32 tabCount ) {
    RET_VAL ret = SUCCESS;
    KINETIC_LAW *kineticLaw;
    KINETIC_LAW *priority;
    EVENT_ASSIGNMENT *assignment;
    LINKED_LIST *assignments;

    START_FUNCTION("_PrintEventForSBML");

    _PrintTab( file, tabCount );
    fprintf( file, "<event id=\"%s\">%s",
	     GetCharArrayOfString( GetEventId( event ) ),
	     NEW_LINE );
    kineticLaw = GetTriggerInEvent( event );
    if (kineticLaw) {
      _PrintTab( file, tabCount + 1 );
      fprintf( file, "<trigger>%s", NEW_LINE );
      if( IS_FAILED( ( ret = _PrintMathForSBML( kineticLaw, file, tabCount + 2 ) ) ) ) {
        END_FUNCTION("_PrintRuleForXHTML", ret );
        return ret;
      }    
      _PrintTab( file, tabCount + 1 );
      fprintf( file, "</trigger>%s", NEW_LINE );
    }
    kineticLaw = GetDelayInEvent( event );
    if (kineticLaw) {
      _PrintTab( file, tabCount + 1 );
      fprintf( file, "<delay>%s", NEW_LINE );
      if( IS_FAILED( ( ret = _PrintMathForSBML( kineticLaw, file, tabCount + 2 ) ) ) ) {
        END_FUNCTION("_PrintRuleForXHTML", ret );
        return ret;
      }    
      _PrintTab( file, tabCount + 1 );
      fprintf( file, "</delay>%s", NEW_LINE );
    } 
    assignments = GetEventAssignments( event );
    ResetCurrentElement( assignments );
    _PrintTab( file, tabCount + 1 );
    fprintf( file, "<listOfEventAssignments>%s", NEW_LINE );
    while( ( assignment = (EVENT_ASSIGNMENT*)GetNextFromLinkedList( assignments ) ) != NULL ) {
      _PrintTab( file, tabCount + 2 );
      fprintf( file, "<eventAssignment variable=\"%s\">%s", 
	       GetCharArrayOfString( assignment->var ),
	       NEW_LINE );
      kineticLaw = assignment->assignment;
      if (kineticLaw) {
	if( IS_FAILED( ( ret = _PrintMathForSBML( kineticLaw, file, tabCount + 3 ) ) ) ) {
	  END_FUNCTION("_PrintRuleForXHTML", ret );
	  return ret;
	}    
      }
      _PrintTab( file, tabCount + 2 );
      fprintf( file, "</eventAssignment>%s", NEW_LINE );
    }
    _PrintTab( file, tabCount + 1 );
    fprintf( file, "</listOfEventAssignments>%s", NEW_LINE );

    _PrintTab( file, tabCount );
    fprintf( file, "</event>%s", NEW_LINE );
            
    END_FUNCTION("_PrintEventForSBML", SUCCESS );
    
    return ret;
}

static RET_VAL _PrintListOfRulesForSBML( IR *ir, FILE *file, UINT32 tabCount ) {
    RET_VAL ret = SUCCESS;
    RULE *rule = NULL;
    RULE_MANAGER *ruleManager;
    LINKED_LIST *list = NULL;
    
    START_FUNCTION("_PrintListOfRulesForSBML");

    if( ( ruleManager = ir->GetRuleManager( ir ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not get the rule manager" );
    }
    list = ruleManager->CreateListOfRules( ruleManager );
    if( ( list == NULL ) || ( GetLinkedListSize( list ) == 0 ) ) {
        END_FUNCTION("_PrintListOfRulesForSBML", SUCCESS );    
        return ret;
    }    

    _PrintTab( file, tabCount );
    fprintf( file, "<listOfRules>%s", NEW_LINE );
    ResetCurrentElement( list );
    while( ( rule = (RULE*)GetNextFromLinkedList( list ) ) != NULL ) {
      if( IS_FAILED( ( ret = _PrintRuleForSBML( rule, file, tabCount+ 1 ) ) ) ) {
            END_FUNCTION("_PrintListOfRulesForSBML", ret );    
            return ret;
        }
    }
    
    _PrintTab( file, tabCount );
    fprintf( file, "</listOfRules>%s", NEW_LINE );
    
    END_FUNCTION("_PrintListOfRulesForSBML", SUCCESS );
    return ret;
}

static RET_VAL _PrintRuleForSBML( RULE *rule, FILE *file, UINT32 tabCount ) {
    RET_VAL ret = SUCCESS;
    KINETIC_LAW *kineticLaw;
    BYTE ruleType;

    START_FUNCTION("_PrintRuleForSBML");

    ruleType = GetRuleType( rule );
    _PrintTab( file, tabCount );
    if (ruleType == RULE_TYPE_ALGEBRAIC) {
      fprintf( file, "<algebraicRule>%s", NEW_LINE );
    } else if (ruleType == RULE_TYPE_ASSIGNMENT) {
      fprintf( file, "<assignmentRule variable=\"%s\">%s",
	       GetCharArrayOfString( GetRuleVar( rule ) ),
	       NEW_LINE );
    } else {
      fprintf( file, "<rateRule variable=\"%s\">%s",
	       GetCharArrayOfString( GetRuleVar( rule ) ),
	       NEW_LINE );
    }
    kineticLaw = GetMathInRule( rule );
    if( IS_FAILED( ( ret = _PrintMathForSBML( kineticLaw, file, tabCount + 1 ) ) ) ) {
        END_FUNCTION("_PrintRuleForXHTML", ret );
        return ret;
    }    
    _PrintTab( file, tabCount );
    if (ruleType == RULE_TYPE_ALGEBRAIC) {
      fprintf( file, "</algebraicRule>%s", NEW_LINE );
    } else if (ruleType == RULE_TYPE_ASSIGNMENT) {
      fprintf( file, "</assignmentRule>%s", NEW_LINE );
    } else {
      fprintf( file, "</rateRule>%s", NEW_LINE );
    }
            
    END_FUNCTION("_PrintRuleForSBML", SUCCESS );
    
    return ret;
}

static RET_VAL _PrintListOfConstraintsForSBML( IR *ir, FILE *file, UINT32 tabCount ) {
    RET_VAL ret = SUCCESS;
    CONSTRAINT *constraint = NULL;
    CONSTRAINT_MANAGER *constraintManager;
    LINKED_LIST *list = NULL;
    
    START_FUNCTION("_PrintListOfConstraintForSBML");

    if( ( constraintManager = ir->GetConstraintManager( ir ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not get the constraint manager" );
    }
    list = constraintManager->CreateListOfConstraints( constraintManager );
    if( ( list == NULL ) || ( GetLinkedListSize( list ) == 0 ) ) {
        END_FUNCTION("_PrintListOfConstraintsForSBML", SUCCESS );    
        return ret;
    }    

    _PrintTab( file, tabCount );
    fprintf( file, "<listOfConstraints>%s", NEW_LINE );
    ResetCurrentElement( list );
    while( ( constraint = (CONSTRAINT*)GetNextFromLinkedList( list ) ) != NULL ) {
      if( IS_FAILED( ( ret = _PrintConstraintForSBML( constraint, file, tabCount+ 1 ) ) ) ) {
            END_FUNCTION("_PrintListOfConstraintsForSBML", ret );    
            return ret;
        }
    }
    
    _PrintTab( file, tabCount );
    fprintf( file, "</listOfConstraints>%s", NEW_LINE );
    
    END_FUNCTION("_PrintListOfConstraintForSBML", SUCCESS );
    return ret;
}

static RET_VAL _PrintConstraintForSBML( CONSTRAINT *constraint, FILE *file, UINT32 tabCount ) {
    RET_VAL ret = SUCCESS;
    KINETIC_LAW *kineticLaw;

    START_FUNCTION("_PrintConstraintForSBML");

    _PrintTab( file, tabCount );
    fprintf( file, "<constraint metaid=\"%s\">%s",
	     GetCharArrayOfString( GetConstraintId( constraint ) ),
	     NEW_LINE );

    kineticLaw = GetMathInConstraint( constraint );
    if( IS_FAILED( ( ret = _PrintMathForSBML( kineticLaw, file, tabCount ) ) ) ) {
        END_FUNCTION("_PrintRuleForXHTML", ret );
        return ret;
    }    

    _PrintTab( file, tabCount );
    fprintf( file, "</constraint>" );
    fprintf( file, NEW_LINE );
            
    END_FUNCTION("_PrintConstraintForSBML", SUCCESS );
    
    return ret;
}

static RET_VAL _PrintListOfReactionsForSBML( IR *ir, FILE *file, UINT32 tabCount ) {
    RET_VAL ret = SUCCESS;
    REACTION *reaction = NULL;
    LINKED_LIST *list = NULL;
    
    START_FUNCTION("_PrintListOfReactionsForSBML");
    
    list = ir->GetListOfReactionNodes( ir );
    if( ( list == NULL ) || ( GetLinkedListSize( list ) == 0 ) ) {
        END_FUNCTION("_PrintListOfReactionsForSBML", SUCCESS );    
        return ret;
    }    
    
    _PrintTab( file, tabCount );
    fprintf( file, "<listOfReactions>%s", NEW_LINE );

    ResetCurrentElement( list );
    while( ( reaction = (REACTION*)GetNextFromLinkedList( list ) ) != NULL ) {
        if( IS_FAILED( ( ret = _PrintReactionForSBML( reaction, file, tabCount + 1 ) ) ) ) {
            END_FUNCTION("_PrintListOfSpeciesForSBML", ret );    
            return ret;
        }
    }
        
    _PrintTab( file, tabCount );
    fprintf( file, "</listOfReactions>%s", NEW_LINE );
    
    END_FUNCTION("_PrintListOfReactionsForSBML", SUCCESS );
    
    return ret;
}
 
static RET_VAL _PrintReactionForSBML( REACTION *reaction, FILE *file, UINT32 tabCount ) {
    RET_VAL ret = SUCCESS;
    LINKED_LIST *list = NULL;
    SPECIES *species = NULL;
    IR_EDGE *edge = NULL;
    KINETIC_LAW *kineticLaw = NULL;
    
    START_FUNCTION("_PrintReactionForSBML");
    
    _PrintTab( file, tabCount );
    fprintf( file, "<reaction id=\"%s\"", GetCharArrayOfString( GetReactionNodeID( reaction ) ) );
    
    if( IsReactionReversibleInReactionNode( reaction ) ) {
        fprintf( file, " name=\"%s\">%s", GetCharArrayOfString( GetReactionNodeName( reaction ) ), NEW_LINE );
    }
    else {
        fprintf( file, " name=\"%s\" reversible=\"false\">%s", GetCharArrayOfString( GetReactionNodeName( reaction ) ), NEW_LINE );
    }
    
    list = GetReactantEdges( (IR_NODE*)reaction );
    if( GetLinkedListSize( list ) != 0 ) {
        _PrintTab( file, tabCount + 1 );
        fprintf( file, "<listOfReactants>%s",  NEW_LINE );
        
        ResetCurrentElement( list );
        while( ( edge = (IR_EDGE*)GetNextFromLinkedList( list ) ) != NULL ) {
            species = GetSpeciesInIREdge( edge );
            _PrintTab( file, tabCount + 2 );            
            fprintf( file, "<speciesReference species=\"%s\" stoichiometry=\"%g\"/>" NEW_LINE, GetCharArrayOfString( GetSpeciesNodeID( species ) ), edge->stoichiometry  );    
        }
        _PrintTab( file, tabCount + 1 );
        fprintf( file, "</listOfReactants>%s",  NEW_LINE );
    }
    
    list = GetProductsInReactionNode( reaction );
    if( GetLinkedListSize( list ) != 0 ) {
        _PrintTab( file, tabCount + 1 );
        fprintf( file, "<listOfProducts>%s",  NEW_LINE );
        
        ResetCurrentElement( list );
        while( ( edge = (IR_EDGE*)GetNextFromLinkedList( list ) ) != NULL ) {
            species = GetSpeciesInIREdge( edge );
            _PrintTab( file, tabCount + 2 );            
            fprintf( file, "<speciesReference species=\"%s\" stoichiometry=\"%g\"/>" NEW_LINE, GetCharArrayOfString( GetSpeciesNodeID( species ) ), edge->stoichiometry  );    
        }
        _PrintTab( file, tabCount + 1 );
        fprintf( file, "</listOfProducts>%s",  NEW_LINE );
    }

    list = GetModifiersInReactionNode( reaction );
    if( GetLinkedListSize( list ) != 0 ) {
        _PrintTab( file, tabCount + 1 );
        fprintf( file, "<listOfModifiers>%s",  NEW_LINE );
        
        ResetCurrentElement( list );
        while( ( edge = (IR_EDGE*)GetNextFromLinkedList( list ) ) != NULL ) {
            species = GetSpeciesInIREdge( edge );
            _PrintTab( file, tabCount + 2 );            
            fprintf( file, "<modifierSpeciesReference species=\"%s\"/>%s", GetCharArrayOfString( GetSpeciesNodeID( species ) ), NEW_LINE );    
        }
        _PrintTab( file, tabCount + 1 );
        fprintf( file, "</listOfModifiers>%s",  NEW_LINE );
    }
    
    kineticLaw = GetKineticLawInReactionNode( reaction );        
    if( IS_FAILED( ( ret = _PrintKineticLawForSBML( kineticLaw, file, tabCount + 1 ) ) ) ) {
        END_FUNCTION("_PrintReactionForSBML", ret );    
        return ret;
    }
    
    _PrintTab( file, tabCount );
    fprintf( file, "</reaction>%s", NEW_LINE );
    
    END_FUNCTION("_PrintReactionForSBML", SUCCESS );
    
    return ret;
}

static RET_VAL _PrintKineticLawForSBML( KINETIC_LAW *kineticLaw, FILE *file, UINT32 tabCount ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("_PrintKineticLawForSBML");
    
    _PrintTab( file, tabCount );
    fprintf( file, "<kineticLaw>%s", NEW_LINE );
    
    _PrintTab( file, tabCount + 1);
    fprintf( file, "<math xmlns=\"http://www.w3.org/1998/Math/MathML\">%s", NEW_LINE );

    if( IS_FAILED( ( ret = _DispatchKineticLawForSBML( kineticLaw, file, tabCount + 2 ) ) ) ) {
        END_FUNCTION("_PrintKineticLawForSBML", ret );    
        return ret;
    }
    
    _PrintTab( file, tabCount + 1);
    fprintf( file, "</math>%s", NEW_LINE );
        
    _PrintTab( file, tabCount );
    fprintf( file, "</kineticLaw>%s", NEW_LINE );
    
    END_FUNCTION("_PrintKineticLawForSBML", SUCCESS );    
    return ret;
}

static RET_VAL _PrintMathForSBML( KINETIC_LAW *kineticLaw, FILE *file, UINT32 tabCount ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("_PrintMathForSBML");
    
    _PrintTab( file, tabCount + 1);
    fprintf( file, "<math xmlns=\"http://www.w3.org/1998/Math/MathML\">%s", NEW_LINE );

    if( IS_FAILED( ( ret = _DispatchKineticLawForSBML( kineticLaw, file, tabCount + 2 ) ) ) ) {
        END_FUNCTION("_PrintMathForSBML", ret );    
        return ret;
    }
    
    _PrintTab( file, tabCount + 1);
    fprintf( file, "</math>%s", NEW_LINE );
    
    END_FUNCTION("_PrintMathForSBML", SUCCESS );    
    return ret;
}

static RET_VAL _DispatchKineticLawForSBML( KINETIC_LAW *kineticLaw, FILE *file, UINT32 tabCount ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("_DispatchKineticLawForSBML");
    
    if( IsOpKineticLaw( kineticLaw ) ) {
        if( IS_FAILED( ( ret = _PrintOpKineticLawForSBML( kineticLaw, file, tabCount  ) ) ) ) {
            END_FUNCTION("_DispatchKineticLawForSBML", ret );    
            return ret;
        }
    }
    else if( IsUnaryOpKineticLaw( kineticLaw ) ) {
        if( IS_FAILED( ( ret = _PrintUnaryOpKineticLawForSBML( kineticLaw, file, tabCount  ) ) ) ) {
            END_FUNCTION("_DispatchKineticLawForSBML", ret );    
            return ret;
        }
    }
    else if( IsPWKineticLaw( kineticLaw ) ) {
        if( IS_FAILED( ( ret = _PrintPWKineticLawForSBML( kineticLaw, file, tabCount  ) ) ) ) {
            END_FUNCTION("_DispatchKineticLawForSBML", ret );    
            return ret;
        }
    }
    else if( IsSpeciesKineticLaw( kineticLaw ) ) {
        if( IS_FAILED( ( ret = _PrintSpeciesKineticLawForSBML( kineticLaw, file, tabCount  ) ) ) ) {
            END_FUNCTION("_DispatchKineticLawForSBML", ret );    
            return ret;
        }
    }
    else if( IsRealValueKineticLaw( kineticLaw ) ) {
        if( IS_FAILED( ( ret = _PrintRealKineticLawForSBML( kineticLaw, file, tabCount ) ) ) ) {
            END_FUNCTION("_DispatchKineticLawForSBML", ret );    
            return ret;
        }
    }
    else if( IsSymbolKineticLaw( kineticLaw ) ) {
        if( IS_FAILED( ( ret = _PrintSymbolKineticLawForSBML( kineticLaw, file, tabCount  ) ) ) ) {
            END_FUNCTION("_DispatchKineticLawForSBML", ret );    
            return ret;
        }
    } 
    else if( IsCompartmentKineticLaw( kineticLaw ) ) {
        if( IS_FAILED( ( ret = _PrintCompartmentKineticLawForSBML( kineticLaw, file, tabCount  ) ) ) ) {
            END_FUNCTION("_DispatchKineticLawForSBML", ret );    
            return ret;
        }
    } 
    else if( IsIntValueKineticLaw( kineticLaw ) ) {
        if( IS_FAILED( ( ret = _PrintIntKineticLawForSBML( kineticLaw, file, tabCount  ) ) ) ) {
            END_FUNCTION("_DispatchKineticLawForSBML", ret );    
            return ret;
        }
    } 
    else {
        return ErrorReport( FAILING, "_DispatchKineticLawForSBML", "invalid kinetic law" );
    }   
    
    END_FUNCTION("_DispatchKineticLawForSBML", SUCCESS );    
    return ret;
}


static RET_VAL _PrintOpKineticLawForSBML( KINETIC_LAW *kineticLaw, FILE *file, UINT32 tabCount ) {
    RET_VAL ret = SUCCESS;
    BYTE opType = 0;
    KINETIC_LAW *left = NULL;
    KINETIC_LAW *right = NULL;
    
    START_FUNCTION("_PrintOpKineticLawForSBML");

    _PrintTab( file, tabCount );
    fprintf( file, "<apply>%s", NEW_LINE );
    
    opType = GetOpTypeFromKineticLaw( kineticLaw );
    _PrintTab( file, tabCount + 1);
    switch( opType ) {
        case KINETIC_LAW_OP_GT:
            fprintf( file, "<gt/>%s", NEW_LINE );
        break;
        case KINETIC_LAW_OP_LT:
            fprintf( file, "<lt/>%s", NEW_LINE );
        break;
        case KINETIC_LAW_OP_GEQ:
            fprintf( file, "<geq/>%s", NEW_LINE );
        break;
        case KINETIC_LAW_OP_LEQ:
            fprintf( file, "<leq/>%s", NEW_LINE );
        break;
        case KINETIC_LAW_OP_EQ:
            fprintf( file, "<eq/>%s", NEW_LINE );
        break;
        case KINETIC_LAW_OP_NEQ:
            fprintf( file, "<neq/>%s", NEW_LINE );
        break;
	/*        case KINETIC_LAW_OP_AND:
            fprintf( file, "<and/>%s", NEW_LINE );
        break;
        case KINETIC_LAW_OP_OR:
            fprintf( file, "<or/>%s", NEW_LINE );
        break;
        case KINETIC_LAW_OP_XOR:
            fprintf( file, "<xor/>%s", NEW_LINE );
	    break;*/
        case KINETIC_LAW_OP_PLUS:
            fprintf( file, "<plus/>%s", NEW_LINE );
        break;
        case KINETIC_LAW_OP_MINUS:
            fprintf( file, "<minus/>%s", NEW_LINE );
        break;
        case KINETIC_LAW_OP_TIMES:
            fprintf( file, "<times/>%s", NEW_LINE );
        break;
        case KINETIC_LAW_OP_DIVIDE:
            fprintf( file, "<divide/>%s", NEW_LINE );
        break;
        case KINETIC_LAW_OP_POW:
            fprintf( file, "<power/>%s", NEW_LINE );
        break;
        case KINETIC_LAW_OP_DELAY:
            fprintf( file, "<delay/>%s", NEW_LINE );
        break;
        case KINETIC_LAW_OP_ROOT:
            fprintf( file, "<root/>%s", NEW_LINE );
        break;
        default:
        return ErrorReport( FAILING, "_PrintOpKineticLawForSBML", "op type %c is not valid", opType );
    }
    
    left = GetOpLeftFromKineticLaw( kineticLaw );
    right = GetOpRightFromKineticLaw( kineticLaw );
    
    if( IS_FAILED( ( ret = _DispatchKineticLawForSBML( left, file, tabCount + 1 ) ) ) ) {
        END_FUNCTION("_PrintOpKineticLawForSBML", SUCCESS );    
        return ret;
    }

    if( IS_FAILED( ( ret = _DispatchKineticLawForSBML( right, file, tabCount + 1 ) ) ) ) {
        END_FUNCTION("_PrintOpKineticLawForSBML", SUCCESS );    
        return ret;
    }
            
    _PrintTab( file, tabCount );
    fprintf( file, "</apply>%s", NEW_LINE );
    
    END_FUNCTION("_PrintOpKineticLawForSBML", SUCCESS );    
    return ret;
}

static RET_VAL _PrintUnaryOpKineticLawForSBML( KINETIC_LAW *kineticLaw, FILE *file, UINT32 tabCount ) {
    RET_VAL ret = SUCCESS;
    BYTE opType = 0;
    KINETIC_LAW *child = NULL;
    
    START_FUNCTION("_PrintUnaryOpKineticLawForSBML");

    _PrintTab( file, tabCount );
    fprintf( file, "<apply>%s", NEW_LINE );
    
    opType = GetUnaryOpTypeFromKineticLaw( kineticLaw );
    _PrintTab( file, tabCount + 1);
    switch( opType ) {
        case KINETIC_LAW_UNARY_OP_COS:
            fprintf( file, "<cos/>%s", NEW_LINE );
        break;
        case KINETIC_LAW_UNARY_OP_COSH:
            fprintf( file, "<cosh/>%s", NEW_LINE );
        break;
        case KINETIC_LAW_UNARY_OP_SIN:
            fprintf( file, "<sin/>%s", NEW_LINE );
        break;
        case KINETIC_LAW_UNARY_OP_SINH:
            fprintf( file, "<sinh/>%s", NEW_LINE );
        break;
        case KINETIC_LAW_UNARY_OP_TAN:
            fprintf( file, "<tan/>%s", NEW_LINE );
        break;
        case KINETIC_LAW_UNARY_OP_TANH:
            fprintf( file, "<tanh/>%s", NEW_LINE );
        break;
        case KINETIC_LAW_UNARY_OP_COT:
            fprintf( file, "<cot/>%s", NEW_LINE );
        break;
        case KINETIC_LAW_UNARY_OP_COTH:
            fprintf( file, "<coth/>%s", NEW_LINE );
        break;
        case KINETIC_LAW_UNARY_OP_CSC:
            fprintf( file, "<csc/>%s", NEW_LINE );
        break;
        case KINETIC_LAW_UNARY_OP_CSCH:
            fprintf( file, "<csch/>%s", NEW_LINE );
        break;
        case KINETIC_LAW_UNARY_OP_SEC:
            fprintf( file, "<sec/>%s", NEW_LINE );
        break;
        case KINETIC_LAW_UNARY_OP_SECH:
            fprintf( file, "<sech/>%s", NEW_LINE );
        break;
        case KINETIC_LAW_UNARY_OP_ARCCOS:
            fprintf( file, "<arccos/>%s", NEW_LINE );
        break;
        case KINETIC_LAW_UNARY_OP_ARCCOSH:
            fprintf( file, "<arccosh/>%s", NEW_LINE );
        break;
        case KINETIC_LAW_UNARY_OP_ARCSIN:
            fprintf( file, "<arcsin/>%s", NEW_LINE );
        break;
        case KINETIC_LAW_UNARY_OP_ARCSINH:
            fprintf( file, "<arcsinh/>%s", NEW_LINE );
        break;
        case KINETIC_LAW_UNARY_OP_ARCTAN:
            fprintf( file, "<arctan/>%s", NEW_LINE );
        break;
        case KINETIC_LAW_UNARY_OP_ARCTANH:
            fprintf( file, "<arctanh/>%s", NEW_LINE );
        break;
        case KINETIC_LAW_UNARY_OP_ARCCOT:
            fprintf( file, "<arccot/>%s", NEW_LINE );
        break;
        case KINETIC_LAW_UNARY_OP_ARCCOTH:
            fprintf( file, "<arccoth/>%s", NEW_LINE );
        break;
        case KINETIC_LAW_UNARY_OP_ARCCSC:
            fprintf( file, "<arccsc/>%s", NEW_LINE );
        break;
        case KINETIC_LAW_UNARY_OP_ARCCSCH:
            fprintf( file, "<arccsch/>%s", NEW_LINE );
        break;
        case KINETIC_LAW_UNARY_OP_ARCSEC:
            fprintf( file, "<arcsec/>%s", NEW_LINE );
        break;
        case KINETIC_LAW_UNARY_OP_ARCSECH:
            fprintf( file, "<arcsech/>%s", NEW_LINE );
        break;
        case KINETIC_LAW_UNARY_OP_LN:
            fprintf( file, "<ln/>%s", NEW_LINE );
        break;
        case KINETIC_LAW_UNARY_OP_LOG:
            fprintf( file, "<log/>%s", NEW_LINE );
        break;
        case KINETIC_LAW_UNARY_OP_FACTORIAL:
            fprintf( file, "<factorial/>%s", NEW_LINE );
        break;
        case KINETIC_LAW_UNARY_OP_ABS:
            fprintf( file, "<abs/>%s", NEW_LINE );
        break;
        case KINETIC_LAW_UNARY_OP_FLOOR:
            fprintf( file, "<floor/>%s", NEW_LINE );
        break;
        case KINETIC_LAW_UNARY_OP_CEILING:
            fprintf( file, "<ceiling/>%s", NEW_LINE );
        break;
        case KINETIC_LAW_UNARY_OP_NOT:
            fprintf( file, "<not/>%s", NEW_LINE );
        break;
        case KINETIC_LAW_UNARY_OP_NEG:
            fprintf( file, "<minus/>%s", NEW_LINE );
        break;
        case KINETIC_LAW_UNARY_OP_EXP:
            fprintf( file, "<exp/>%s", NEW_LINE );
        break;
        default:
        return ErrorReport( FAILING, "_PrintOpKineticLawForSBML", "op type %c is not valid", opType );
    }
    
    child = GetUnaryOpChildFromKineticLaw( kineticLaw );
    
    if( IS_FAILED( ( ret = _DispatchKineticLawForSBML( child, file, tabCount + 1 ) ) ) ) {
        END_FUNCTION("_PrintUnaryOpKineticLawForSBML", SUCCESS );    
        return ret;
    }
            
    _PrintTab( file, tabCount );
    fprintf( file, "</apply>%s", NEW_LINE );
    
    END_FUNCTION("_PrintUnaryOpKineticLawForSBML", SUCCESS );    
    return ret;
}

static RET_VAL _PrintPWKineticLawForSBML( KINETIC_LAW *kineticLaw, FILE *file, UINT32 tabCount ) {
    RET_VAL ret = SUCCESS;
    KINETIC_LAW *child = NULL;
    LINKED_LIST *children = NULL;
    UINT32 size = 0;
    int i;

    START_FUNCTION("_PrintPWKineticLawForSBML");
    
    children = GetPWChildrenFromKineticLaw( kineticLaw );
    size = GetLinkedListSize( children );
    switch( GetPWTypeFromKineticLaw( kineticLaw ) ) {
    case KINETIC_LAW_OP_PW:
      _PrintTab( file, tabCount );
      fprintf( file, "<piecewise>%s", NEW_LINE );
      for (i = 0; i < size-1; i+=2) {
	_PrintTab( file, tabCount + 1 );
	fprintf( file, "<piece>%s", NEW_LINE );
	child = (KINETIC_LAW*)GetElementByIndex( i, children );
	if( IS_FAILED( ( ret = _DispatchKineticLawForSBML( child, file, tabCount + 2 ) ) ) ) {
	  END_FUNCTION("_PrintUnaryOpKineticLawForSBML", SUCCESS );    
	  return ret;
	}
	child = (KINETIC_LAW*)GetElementByIndex( i+1, children );
	if( IS_FAILED( ( ret = _DispatchKineticLawForSBML( child, file, tabCount + 2 ) ) ) ) {
	  END_FUNCTION("_PrintUnaryOpKineticLawForSBML", SUCCESS );    
	  return ret;
	}
	_PrintTab( file, tabCount + 1 );
	fprintf( file, "</piece>%s", NEW_LINE );
      }
      if (size % 2 == 1) {
	_PrintTab( file, tabCount + 1 );
	fprintf( file, "<otherwise>%s", NEW_LINE );
	child = (KINETIC_LAW*)GetElementByIndex( size - 1, children );
	if( IS_FAILED( ( ret = _DispatchKineticLawForSBML( child, file, tabCount + 2 ) ) ) ) {
	  END_FUNCTION("_PrintUnaryOpKineticLawForSBML", SUCCESS );    
	  return ret;
	}
	_PrintTab( file, tabCount + 1 );
	fprintf( file, "</otherwise>%s", NEW_LINE );
      }
      _PrintTab( file, tabCount );
      fprintf( file, "</piecewise>%s", NEW_LINE );
      break;
    case KINETIC_LAW_OP_AND:
      _PrintTab( file, tabCount );
      fprintf( file, "<apply>%s", NEW_LINE );
      _PrintTab( file, tabCount );
      fprintf( file, "<and/>%s", NEW_LINE );
      for (i = 0; i < size; i++) {
	child = (KINETIC_LAW*)GetElementByIndex( i, children );
	if( IS_FAILED( ( ret = _DispatchKineticLawForSBML( child, file, tabCount + 1 ) ) ) ) {
	  END_FUNCTION("_PrintUnaryOpKineticLawForSBML", SUCCESS );    
	  return ret;
	}
      }
      _PrintTab( file, tabCount );
      fprintf( file, "</apply>%s", NEW_LINE );
      break;
    case KINETIC_LAW_OP_OR:
      _PrintTab( file, tabCount );
      fprintf( file, "<apply>%s", NEW_LINE );
      _PrintTab( file, tabCount );
      fprintf( file, "<or/>%s", NEW_LINE );
      for (i = 0; i < size; i++) {
	child = (KINETIC_LAW*)GetElementByIndex( i, children );
	if( IS_FAILED( ( ret = _DispatchKineticLawForSBML( child, file, tabCount + 1 ) ) ) ) {
	  END_FUNCTION("_PrintUnaryOpKineticLawForSBML", SUCCESS );    
	  return ret;
	}
      }
      _PrintTab( file, tabCount );
      fprintf( file, "</apply>%s", NEW_LINE );
      break;
    case KINETIC_LAW_OP_XOR:
      _PrintTab( file, tabCount );
      fprintf( file, "<apply>%s", NEW_LINE );
      _PrintTab( file, tabCount );
      fprintf( file, "<xor/>%s", NEW_LINE );
      for (i = 0; i < size; i++) {
	child = (KINETIC_LAW*)GetElementByIndex( i, children );
	if( IS_FAILED( ( ret = _DispatchKineticLawForSBML( child, file, tabCount + 1 ) ) ) ) {
	  END_FUNCTION("_PrintUnaryOpKineticLawForSBML", SUCCESS );    
	  return ret;
	}
      }
      _PrintTab( file, tabCount );
      fprintf( file, "</apply>%s", NEW_LINE );
      break;
    }
    END_FUNCTION("_PrintUnaryOpKineticLawForSBML", SUCCESS );    
    return ret;
}

static RET_VAL _PrintIntKineticLawForSBML( KINETIC_LAW *kineticLaw, FILE *file, UINT32 tabCount ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("_PrintIntKineticLawForSBML");

    _PrintTab( file, tabCount );
    fprintf( file, "<cn>%li</cn>%s", GetIntValueFromKineticLaw( kineticLaw ), NEW_LINE );
    
    END_FUNCTION("_PrintIntKineticLawForSBML", SUCCESS );    
    return ret;
}

static RET_VAL _PrintRealKineticLawForSBML( KINETIC_LAW *kineticLaw, FILE *file, UINT32 tabCount ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("_PrintRealKineticLawForSBML");

    if( IS_FAILED( ( ret = _PrintCnInEnotationForSBML( file, tabCount, GetRealValueFromKineticLaw( kineticLaw ) ) ) ) ) {
        END_FUNCTION("_PrintRealKineticLawForSBML", ret );    
        return ret;
    }
    END_FUNCTION("_PrintRealKineticLawForSBML", SUCCESS );    
    return ret;
}

static RET_VAL _PrintSpeciesKineticLawForSBML( KINETIC_LAW *kineticLaw, FILE *file, UINT32 tabCount ) {
    RET_VAL ret = SUCCESS;
    SPECIES *species = NULL;
    
    START_FUNCTION("_PrintRealKineticLawForSBML");
    
    species = GetSpeciesFromKineticLaw( kineticLaw );
    _PrintTab( file, tabCount );
    fprintf( file, "<ci>%s</ci>%s", GetCharArrayOfString( GetSpeciesNodeID( species ) ), NEW_LINE );
    
    END_FUNCTION("_PrintRealKineticLawForSBML", SUCCESS );    
    return ret;
}

static RET_VAL _PrintSymbolKineticLawForSBML( KINETIC_LAW *kineticLaw, FILE *file, UINT32 tabCount ) {
    RET_VAL ret = SUCCESS;
    REB2SAC_SYMBOL *sym = NULL;
    
    START_FUNCTION("_PrintSymbolKineticLawForSBML");
    
    sym = GetSymbolFromKineticLaw( kineticLaw );
    _PrintTab( file, tabCount );
    fprintf( file, "<ci>%s</ci>%s", GetCharArrayOfString( GetSymbolID( sym ) ), NEW_LINE );
    
    END_FUNCTION("_PrintSymbolKineticLawForSBML", SUCCESS );    
    return ret;
}


static RET_VAL _PrintCompartmentKineticLawForSBML( KINETIC_LAW *kineticLaw, FILE *file, UINT32 tabCount ) {
    RET_VAL ret = SUCCESS;
    COMPARTMENT *sym = NULL;
    
    START_FUNCTION("_PrintSymbolKineticLawForSBML");
    
    sym = GetCompartmentFromKineticLaw( kineticLaw );
    _PrintTab( file, tabCount );
    fprintf( file, "<ci>%s</ci>%s", GetCharArrayOfString( GetCompartmentID( sym ) ), NEW_LINE );
    
    END_FUNCTION("_PrintSymbolKineticLawForSBML", SUCCESS );    
    return ret;
}


static RET_VAL _PrintCnInEnotationForSBML( FILE *file, UINT32 tabCount, double value ) {
    RET_VAL ret = SUCCESS;
    char buf[128];
    char *exp = 0;
        
    START_FUNCTION("_PrintCnInEnotationForSBML");
    
    _PrintTab( file, tabCount );
    
    sprintf( buf, "%g", value );
    exp = strchr( buf, 'e' );
    if( exp == NULL ) {
        fprintf( file, "<cn>%s</cn>%s", buf, NEW_LINE );
    }
    else {
        exp[0] = '\0';
        exp += 1;
        fprintf( file, "<cn type=\"e-notation\">%s<sep/>%s</cn>%s", buf, exp, NEW_LINE );
    }
    
    END_FUNCTION("_PrintCnInEnotationForSBML", SUCCESS );    
    return ret;
}

 
 
static RET_VAL _GenerateXHTML( IR *ir, FILE *file ) {
    RET_VAL ret = SUCCESS;
    START_FUNCTION("_GenerateXHTML");

    if( IS_FAILED( ( ret = GenerateXHTMLFromIR( ir, file ) ) ) ) {
        END_FUNCTION("_GenerateXHTML", ret );    
        return ret;
    }

    END_FUNCTION("_GenerateXHTML", SUCCESS );    
    return ret;
}          


