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
/*
#if defined(DEBUG)
#undef DEBUG
#endif
*/
#include "sbml_front_end_processor.h"
#include "symtab.h"
#include "kinetic_law_support.h"


static RET_VAL _ParseSBML( FRONT_END_PROCESSOR *frontend );
static RET_VAL _GenerateIR( FRONT_END_PROCESSOR *frontend, IR *ir );
static RET_VAL _HandleGlobalParameters( FRONT_END_PROCESSOR *frontend, Model_t *model );
static RET_VAL _HandleInitialAssignments( FRONT_END_PROCESSOR *frontend, Model_t *model, IR *ir );
static RET_VAL _HandleRuleAssignments( FRONT_END_PROCESSOR *frontend, Model_t *model, IR *ir );

static RET_VAL _HandleUnitDefinitions( FRONT_END_PROCESSOR *frontend, Model_t *model );
static RET_VAL _HandleUnitDefinition( FRONT_END_PROCESSOR *frontend, Model_t *model, UnitDefinition_t *unitDef );

static RET_VAL _HandleFunctionDefinitions( FRONT_END_PROCESSOR *frontend, Model_t *model );
static RET_VAL _HandleFunctionDefinition( FRONT_END_PROCESSOR *frontend, Model_t *model, FunctionDefinition_t *functionDef );

static RET_VAL _HandleAlgebraicRules( FRONT_END_PROCESSOR *frontend, Model_t *model );
static RET_VAL _HandleAlgebraicRule( FRONT_END_PROCESSOR *frontend, Model_t *model, Rule_t *ruleDef );

static RET_VAL _HandleRules( FRONT_END_PROCESSOR *frontend, Model_t *model );
static RET_VAL _HandleRule( FRONT_END_PROCESSOR *frontend, Model_t *model, Rule_t *ruleDef );

static RET_VAL _HandleConstraints( FRONT_END_PROCESSOR *frontend, Model_t *model );
static RET_VAL _HandleConstraint( FRONT_END_PROCESSOR *frontend, Model_t *model, Constraint_t *constraintDef );

static RET_VAL _HandleEvents( FRONT_END_PROCESSOR *frontend, Model_t *model );
static RET_VAL _HandleEvent( FRONT_END_PROCESSOR *frontend, Model_t *model, Event_t *eventDef );

static RET_VAL _HandleCompartments( FRONT_END_PROCESSOR *frontend, Model_t *model );
static RET_VAL _HandleCompartment( FRONT_END_PROCESSOR *frontend, Model_t *model, Compartment_t *compartment );

static RET_VAL _CreateSpeciesNodes(  FRONT_END_PROCESSOR *frontend, IR *ir, Model_t *model );
static RET_VAL _CreateSpeciesNode(  FRONT_END_PROCESSOR *frontend, IR *ir, Model_t *model, Species_t *species );

static RET_VAL _CreateReactionNodes( FRONT_END_PROCESSOR *frontend, IR *ir, Model_t *model );
static RET_VAL _CreateReactionNode( FRONT_END_PROCESSOR *frontend, IR *ir, Model_t *model, Reaction_t *reaction );
static RET_VAL _CreateKineticLaw( FRONT_END_PROCESSOR *frontend, REACTION *reactionNode, REACTION_LAW *reactionLaw, Model_t *model, Reaction_t *reaction );
static KINETIC_LAW *_TransformKineticLaw( FRONT_END_PROCESSOR *frontend, ASTNode_t *source, SBML_SYMTAB_MANAGER *manager, HASH_TABLE *table );
static KINETIC_LAW *_TransformOpKineticLaw( FRONT_END_PROCESSOR *frontend, ASTNode_t *source, SBML_SYMTAB_MANAGER *manager, HASH_TABLE *table );
static KINETIC_LAW *_TransformNegKineticLaw( FRONT_END_PROCESSOR *frontend, ASTNode_t *source, SBML_SYMTAB_MANAGER *manager, HASH_TABLE *table );
static KINETIC_LAW *_TransformFunctionKineticLaw( FRONT_END_PROCESSOR *frontend, ASTNode_t *source, SBML_SYMTAB_MANAGER *manager, HASH_TABLE *table );
static KINETIC_LAW *_TransformSymKineticLaw( FRONT_END_PROCESSOR *frontend, ASTNode_t *source, SBML_SYMTAB_MANAGER *manager, HASH_TABLE *table );
static KINETIC_LAW *_TransformIntValueKineticLaw( FRONT_END_PROCESSOR *frontend, ASTNode_t *source, SBML_SYMTAB_MANAGER *manager, HASH_TABLE *table );
static KINETIC_LAW *_TransformRealValueKineticLaw( FRONT_END_PROCESSOR *frontend, ASTNode_t *source, SBML_SYMTAB_MANAGER *manager, HASH_TABLE *table );

static RET_VAL _ResolveNodeLinks( FRONT_END_PROCESSOR *frontend, IR *ir, REACTION *reactionNode,  Reaction_t *reaction, Model_t *model );

static RET_VAL _AddGlobalParamInSymtab( FRONT_END_PROCESSOR *frontend, REB2SAC_SYMTAB *symtab, SBML_SYMTAB_MANAGER *sbmlSymtabManager );

int workingOnFunctions;

RET_VAL ProcessSBMLFrontend( FRONT_END_PROCESSOR *frontend, IR *ir ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("ProcessSBMLFrontend");

    if( IS_FAILED( ( ret = _ParseSBML( frontend ) ) ) ) {
        END_FUNCTION("ProcessSBMLFrontend", ret );
        return ret;
    }
    
    if( IS_FAILED( ( ret = _GenerateIR( frontend, ir ) ) ) ) {
        END_FUNCTION("ProcessSBMLFrontend", ret );
        return ret;
    }
    
    END_FUNCTION("ProcessSBMLFrontend", SUCCESS );
    return ret;
}


RET_VAL CloseSBMLFrontend( FRONT_END_PROCESSOR *frontend ) {
    RET_VAL ret = SUCCESS;
    
    int i = 0;
    START_FUNCTION("CloseSBMLFrontend");
        
    SBMLDocument_free( (SBMLDocument_t *)(frontend->_internal1) );
    
    frontend->_internal1 = NULL;
    
    DeleteHashTable( (HASH_TABLE**)&(frontend->_internal2) );
    
    END_FUNCTION("CloseSBMLFrontend", SUCCESS );
    return ret;
}



static RET_VAL _ParseSBML( FRONT_END_PROCESSOR *frontend ) {
    RET_VAL ret = SUCCESS;
    char *id = NULL;    
    UINT errorNum = 0;
    BOOL error = FALSE;
    SBMLDocument_t *doc = NULL;
    COMPILER_RECORD_T *record = NULL;
    Model_t *model = NULL;
    //    CompModelPlugin *sbmlCompModel = NULL;
    
    START_FUNCTION("_ParseSBML");

    record = frontend->record;
    
    doc = readSBML( GetCharArrayOfString( record->inputPath ) );
    if (( SBMLDocument_getLevel( doc ) != 3) || ( SBMLDocument_getVersion( doc ) != 1)) {
      SBMLDocument_setLevelAndVersion( doc, 3, 1);
      writeSBML( doc, GetCharArrayOfString( record->inputPath ) );
      doc = readSBML( GetCharArrayOfString( record->inputPath ) );
    }

//     SBMLDocument_checkConsistency( doc );
//     errorNum = SBMLDocument_getNumWarnings( doc );
//     if( errorNum > 0 ) {
//         /*this is not actually error*/
//         SBMLDocument_printWarnings( doc, stderr );        
//     }
    /*
    if (doc != NULL) {
      errorNum = SBMLDocument_getNumErrors( doc );
      if( errorNum > 0 ) {
	SBMLDocument_printErrors( doc, stderr );
	error = TRUE;         
      }
    }
    */

//     errorNum = SBMLDocument_getNumFatals( doc );
//     if( errorNum > 0 ) {
//         SBMLDocument_printFatals( doc, stderr );
//         error = TRUE;
//     }

    /* if there are errors do not try to flatten */	
    if (SBMLDocument_getNumErrorsWithSeverity(doc, LIBSBML_SEV_ERROR) > 0) {
      SBMLDocument_printErrors(doc, stderr);
      error = TRUE;
    } else {
      /* need new variables ... */
      ConversionProperties_t* props;
      ConversionOption_t* option1;
      ConversionOption_t* option2;

      /* create a new conversion properties structure */
      props = ConversionProperties_create();

      /* add an option that we want to flatten comp */
      option1 = ConversionOption_create("flatten comp");
      ConversionOption_setType(option1, CNV_TYPE_BOOL);
      ConversionOption_setValue(option1, "true");
      ConversionOption_setDescription(option1, "flatten comp");
      ConversionProperties_addOption(props, option1);

      /* perform the conversion */
      if (SBMLDocument_convert(doc, props) != LIBSBML_OPERATION_SUCCESS) {
    	printf ("Errors found during flattening ... \n");
    	//SBMLDocument_printErrors(doc, stderr);
    	//error = TRUE;
      }
    }

    if( !doc || error ) {
        END_FUNCTION("_ParseSBML", FAILING );
        return ErrorReport( FAILING, "_ParseSBML", "input file error" );                                             
    }
    model = SBMLDocument_getModel( doc );
    //sbmlCompModel = (CompModelPlugin_t)Model_getPlugin(model, "comp");
    frontend->_internal1 = (CADDR_T)doc;                
    
#ifdef DEBUG
    model = SBMLDocument_getModel( doc );
    id = Model_getId( model );
    if( id == NULL ) {
        printf( "model id is not defined%s", NEW_LINE );
    }
    else {    
        printf( "model name is: %s%s", id, NEW_LINE );
    }        
#endif
    END_FUNCTION("_ParseSBML", SUCCESS );
    return ret;
}



static RET_VAL _GenerateIR( FRONT_END_PROCESSOR *frontend, IR *ir ) {
    RET_VAL ret = SUCCESS;
    Model_t *model = NULL;
    SBMLDocument_t *doc = NULL;
    HASH_TABLE *table = NULL;
    UNIT_MANAGER *unitManager = NULL;
    FUNCTION_MANAGER *functionManager = NULL;
    RULE_MANAGER *ruleManager = NULL;
    CONSTRAINT_MANAGER *constraintManager = NULL;
    EVENT_MANAGER *eventManager = NULL;
    COMPARTMENT_MANAGER *compartmentManager = NULL;
    REACTION_LAW_MANAGER *reactionLawManager = NULL;
    SBML_SYMTAB_MANAGER *sbmlSymtabManager = NULL;
    REB2SAC_SYMTAB *symtab = NULL;

    START_FUNCTION("_GenerateIR");
    
    doc = (SBMLDocument_t*)(frontend->_internal1);
    model = SBMLDocument_getModel( doc );
    table = CreateHashTable( 256 );
    frontend->_internal2 = (CADDR_T)table;
    symtab = ir->GetGlobalSymtab( ir );
    frontend->_internal3 = (CADDR_T)symtab;
    
    workingOnFunctions = 0;

    ir->SetModelId( ir, Model_getId( model ) );
    ir->SetModelName( ir, Model_getName( model ) );
    ir->SetModelSubstanceUnits( ir, Model_getSubstanceUnits( model ) );
    ir->SetModelTimeUnits( ir, Model_getTimeUnits( model ) );
    ir->SetModelVolumeUnits( ir, Model_getVolumeUnits( model ) );
    ir->SetModelAreaUnits( ir, Model_getAreaUnits( model ) );
    ir->SetModelLengthUnits( ir, Model_getLengthUnits( model ) );
    ir->SetModelExtentUnits( ir, Model_getExtentUnits( model ) );
        
    if( IS_FAILED( ( ret = _HandleUnitDefinitions( frontend, model ) ) ) ) {
        END_FUNCTION("_GenerateIR", ret );
        return ret;
    }
    if( ( unitManager = GetUnitManagerInstance( frontend->record ) ) == NULL ) {
        return ErrorReport( FAILING, "_GenerateIR", "could not get an instance of unit manager" );
    } 
    if( IS_FAILED( ( ret = ir->SetUnitManager( ir, unitManager ) ) ) ) {
        END_FUNCTION("_GenerateIR", ret );
        return ret;
    } 
    
    if( IS_FAILED( ( ret = _HandleGlobalParameters( frontend, model ) ) ) ) {
        END_FUNCTION("_GenerateIR", ret );
        return ret;
    }
    if( ( sbmlSymtabManager = GetSymtabManagerInstance( frontend->record ) ) == NULL ) {
        return ErrorReport( FAILING, "_GenerateIR", "error on getting symtab manager" ); 
    }

    if( ( symtab->AddRealValueSymbol( symtab, "t", 0, FALSE ) ) == NULL ) {
      return ErrorReport( FAILING, "_PutParametersInGlobalSymtab", 
			  "failed to put parameter t in global symtab" );
    }     
    if( ( symtab->AddRealValueSymbol( symtab, "time", 0, FALSE ) ) == NULL ) {
      return ErrorReport( FAILING, "_PutParametersInGlobalSymtab", 
			  "failed to put parameter time in global symtab" );
    }     
        
    if( IS_FAILED( ( ret = _HandleFunctionDefinitions( frontend, model ) ) ) ) {
        END_FUNCTION("_GenerateIR", ret );
        return ret;
    }
    if( ( functionManager = GetFunctionManagerInstance( frontend->record ) ) == NULL ) {
        return ErrorReport( FAILING, "_GenerateIR", "could not get an instance of function manager" );
    } 
    if( IS_FAILED( ( ret = ir->SetFunctionManager( ir, functionManager ) ) ) ) {
        END_FUNCTION("_GenerateIR", ret );
        return ret;
    } 
    
    if( IS_FAILED( ( ret = _HandleCompartments( frontend, model ) ) ) ) {
        END_FUNCTION("_GenerateIR", ret );
        return ret;
    }    
    if( ( compartmentManager = GetCompartmentManagerInstance( frontend->record ) ) == NULL ) {
        return ErrorReport( FAILING, "_GenerateIR", "could not get an instance of compartment manager" );
    } 
    if( IS_FAILED( ( ret = ir->SetCompartmentManager( ir, compartmentManager ) ) ) ) {
        END_FUNCTION("_GenerateIR", ret );
        return ret;
    }
    
    if( IS_FAILED( ( ret = _AddGlobalParamInSymtab( frontend, symtab, sbmlSymtabManager ) ) ) ) {
        END_FUNCTION("_GenerateIR", ret );
        return ret;
    }
        
    if( IS_FAILED( ( ret = _CreateSpeciesNodes( frontend, ir, model ) ) ) ) {
        END_FUNCTION("_GenerateIR", ret );
        return ret;
    }
        
    if( IS_FAILED( ( ret = _HandleAlgebraicRules( frontend, model ) ) ) ) {
        END_FUNCTION("_GenerateIR", ret );
        return ret;
    }
    
    if( IS_FAILED( ( ret = _CreateReactionNodes( frontend, ir, model ) ) ) ) {
        END_FUNCTION("_GenerateIR", ret );
        return ret;
    }

    if( ( reactionLawManager = GetReactionLawManagerInstance( frontend->record ) ) == NULL ) {
        return ErrorReport( FAILING, "_GenerateIR", "could not get an instance of reactionLaw manager" );
    } 
    if( IS_FAILED( ( ret = ir->SetReactionLawManager( ir, reactionLawManager ) ) ) ) {
        END_FUNCTION("_GenerateIR", ret );
        return ret;
    }
        
    if( IS_FAILED( ( ret = _HandleRules( frontend, model ) ) ) ) {
        END_FUNCTION("_GenerateIR", ret );
        return ret;
    }
    if( ( ruleManager = GetRuleManagerInstance( frontend->record ) ) == NULL ) {
        return ErrorReport( FAILING, "_GenerateIR", "could not get an instance of rule manager" );
    } 
    if( IS_FAILED( ( ret = ir->SetRuleManager( ir, ruleManager ) ) ) ) {
        END_FUNCTION("_GenerateIR", ret );
        return ret;
    } 

    if( IS_FAILED( ( ret = _HandleConstraints( frontend, model ) ) ) ) {
        END_FUNCTION("_GenerateIR", ret );
        return ret;
    }
    if( ( constraintManager = GetConstraintManagerInstance( frontend->record ) ) == NULL ) {
        return ErrorReport( FAILING, "_GenerateIR", "could not get an instance of constraint manager" );
    } 
    if( IS_FAILED( ( ret = ir->SetConstraintManager( ir, constraintManager ) ) ) ) {
        END_FUNCTION("_GenerateIR", ret );
        return ret;
    } 

    if( IS_FAILED( ( ret = _HandleEvents( frontend, model ) ) ) ) {
        END_FUNCTION("_GenerateIR", ret );
        return ret;
    }
    if( ( eventManager = GetEventManagerInstance( frontend->record ) ) == NULL ) {
        return ErrorReport( FAILING, "_GenerateIR", "could not get an instance of event manager" );
    } 
    if( IS_FAILED( ( ret = ir->SetEventManager( ir, eventManager ) ) ) ) {
        END_FUNCTION("_GenerateIR", ret );
        return ret;
    } 

    if( IS_FAILED( ( ret = _HandleInitialAssignments( frontend, model, ir ) ) ) ) {
      END_FUNCTION("_GenerateIR", ret );
      return ret;
    }
    if( IS_FAILED( ( ret = _HandleRuleAssignments( frontend, model, ir ) ) ) ) {
      END_FUNCTION("_GenerateIR", ret );
      return ret;
    }
    
    END_FUNCTION("_GenerateIR", SUCCESS );
    return ret;
}

static RET_VAL _HandleGlobalParameters( FRONT_END_PROCESSOR *frontend, Model_t *model ) {
    RET_VAL ret = SUCCESS;
    SBML_SYMTAB_MANAGER *manager = NULL;
    ListOf_t *list = NULL;
    UINT size = 0;
    UINT i = 0;

    START_FUNCTION("_HandleGlobalParameters");

    if( ( manager = GetSymtabManagerInstance( frontend->record ) ) == NULL ) {
        return ErrorReport( FAILING, "_HandleGlobalParameters", "error on getting symtab manager" ); 
    }
    list = Model_getListOfParameters( model );
    if( IS_FAILED( ( ret = manager->SetGlobal( manager, list ) ) ) ) {
        return ErrorReport( FAILING, "_HandleGlobalParameters", "error on setting global" ); 
    }     
    
    END_FUNCTION("_HandleGlobalParameters", SUCCESS );
    return ret;
}

static RET_VAL _HandleInitialAssignments( FRONT_END_PROCESSOR *frontend, Model_t *model, IR *ir ) {
    RET_VAL ret = SUCCESS;
    SBML_SYMTAB_MANAGER *manager = NULL;
    COMPARTMENT_MANAGER *compartmentManager = NULL;
    ListOf_t *list = NULL;
    UINT size = 0;
    UINT i = 0;
    UINT j = 0;
    UINT sizeS = 0;
    LINKED_LIST *listS = NULL;
    UINT sizeC = 0;
    LINKED_LIST *listC = NULL;
    InitialAssignment_t *initialAssignment = NULL;
    Parameter_t *parameter = NULL;
    ASTNode_t *node = NULL;
    KINETIC_LAW *law = NULL;
    HASH_TABLE *table = NULL;
    char * id;
    char * Pid;
    char * Sid;
    char * Cid;
    double Pvalue;
    SPECIES *speciesNode;
    COMPARTMENT *compartmentNode;
    REB2SAC_SYMTAB *symtab = (REB2SAC_SYMTAB*)frontend->_internal3;
    REB2SAC_SYMBOL *symbol = NULL;

    START_FUNCTION("_HandleInitialAssignments");

    if( ( manager = GetSymtabManagerInstance( frontend->record ) ) == NULL ) {
        return ErrorReport( FAILING, "_HandleInitialAssignments", "error on getting symtab manager" ); 
    }
    if( ( compartmentManager = GetCompartmentManagerInstance( frontend->record ) ) == NULL ) {
        return ErrorReport( FAILING, "_HandleInitialAssignments", "could not get an instance of compartment manager" );
    }
    table = (HASH_TABLE*)frontend->_internal2;    
    list = Model_getListOfInitialAssignments( model );
    size = Model_getNumInitialAssignments( model );
    listS = ir->GetListOfSpeciesNodes( ir );
    sizeS = GetLinkedListSize( listS );
    listC = compartmentManager->CreateListOfCompartments( compartmentManager );
    sizeC = GetLinkedListSize( listC );
    for( i = 0; i < size; i++ ) {
      initialAssignment = (InitialAssignment_t*)ListOf_get( list, i );
      id = (char *)InitialAssignment_getSymbol(initialAssignment);
      node = (ASTNode_t *)InitialAssignment_getMath( initialAssignment );
      if( ( law = _TransformKineticLaw( frontend, node, manager, table ) ) == NULL ) {
	return ErrorReport( FAILING, "_HandleInitialAssignments", "failed to create initial assignment for %s", id );        
      }
      if( ( symbol = symtab->Lookup( symtab, id ) ) != NULL ) {
	SetInitialAssignmentInSymbol( symbol, (struct KINETIC_LAW*)law );
      }
      for ( j = 0; j < sizeS; j++ ) {
	speciesNode = (SPECIES*)GetElementByIndex(j,listS);
	Sid = GetCharArrayOfString( GetSpeciesNodeID( speciesNode ));
	if (strcmp(id,Sid)==0) {
	  SetInitialAssignmentInSpeciesNode( speciesNode,(struct KINETIC_LAW*)law );
	}
      }
      for ( j = 0; j < sizeC; j++ ) {
	compartmentNode = (COMPARTMENT*)GetElementByIndex(j,listC);
	Cid = GetCharArrayOfString( GetCompartmentID( compartmentNode ));
	if (strcmp(id,Cid)==0) {
	  SetInitialAssignmentInCompartment( compartmentNode,(struct KINETIC_LAW*)law );
	}
      }
    }
    END_FUNCTION("_HandleInitialAssignments", SUCCESS );
    return ret;
}

static RET_VAL _HandleRuleAssignments( FRONT_END_PROCESSOR *frontend, Model_t *model, IR *ir ) {
    RET_VAL ret = SUCCESS;
    SBML_SYMTAB_MANAGER *manager = NULL;
    COMPARTMENT_MANAGER *compartmentManager = NULL;
    ListOf_t *list = NULL;
    UINT size = 0;
    UINT i = 0;
    UINT j = 0;
    UINT sizeS = 0;
    LINKED_LIST *listS = NULL;
    UINT sizeC = 0;
    LINKED_LIST *listC = NULL;
    Rule_t *rule = NULL;
    Parameter_t *parameter = NULL;
    ASTNode_t *node = NULL;
    KINETIC_LAW *law = NULL;
    HASH_TABLE *table = NULL;
    char * id;
    char * Pid;
    char * Sid;
    char * Cid;
    double Pvalue;
    SPECIES *speciesNode;
    COMPARTMENT *compartmentNode;
    REB2SAC_SYMTAB *symtab = (REB2SAC_SYMTAB*)frontend->_internal3;
    REB2SAC_SYMBOL *symbol = NULL;

    START_FUNCTION("_HandleRuleAssignments");

    if( ( manager = GetSymtabManagerInstance( frontend->record ) ) == NULL ) {
        return ErrorReport( FAILING, "_HandleRuleAssignments", "error on getting symtab manager" ); 
    }
    if( ( compartmentManager = GetCompartmentManagerInstance( frontend->record ) ) == NULL ) {
        return ErrorReport( FAILING, "_HandleRuleAssignments", "could not get an instance of compartment manager" );
    }
    table = (HASH_TABLE*)frontend->_internal2;    
    list = Model_getListOfRules( model );
    size = Model_getNumRules( model );
    listS = ir->GetListOfSpeciesNodes( ir );
    sizeS = GetLinkedListSize( listS );
    listC = compartmentManager->CreateListOfCompartments( compartmentManager );
    sizeC = GetLinkedListSize( listC );
    for( i = 0; i < size; i++ ) {
      rule = (Rule_t*)ListOf_get( list, i );
      if (Rule_isAssignment( rule )) {
	id = (char *)Rule_getVariable( rule );
	node = (ASTNode_t *)Rule_getMath( rule );
	if( ( law = _TransformKineticLaw( frontend, node, manager, table ) ) == NULL ) {
	  return ErrorReport( FAILING, "_HandleRuleAssignments", "failed to create initial assignment for rule %s", id );        
	}
	if( ( symbol = symtab->Lookup( symtab, id ) ) != NULL ) {
	  SetInitialAssignmentInSymbol( symbol, (struct KINETIC_LAW*)law );
	}
	for ( j = 0; j < sizeS; j++ ) {
	  speciesNode = (SPECIES*)GetElementByIndex(j,listS);
	  Sid = GetCharArrayOfString( GetSpeciesNodeID( speciesNode ));
	  if (strcmp(id,Sid)==0) {
	    SetInitialAssignmentInSpeciesNode( speciesNode,(struct KINETIC_LAW*)law );
	  }
	}
	for ( j = 0; j < sizeC; j++ ) {
	  compartmentNode = (COMPARTMENT*)GetElementByIndex(j,listC);
	  Cid = GetCharArrayOfString( GetCompartmentID( compartmentNode ));
	  if (strcmp(id,Cid)==0) {
	    SetInitialAssignmentInCompartment( compartmentNode,(struct KINETIC_LAW*)law );
	  }
	}
      }
    }
    END_FUNCTION("_HandleRuleAssignments", SUCCESS );
    return ret;
}

static RET_VAL _AddGlobalParamInSymtab( FRONT_END_PROCESSOR *frontend, 
                                        REB2SAC_SYMTAB *symtab, 
                                        SBML_SYMTAB_MANAGER *sbmlSymtabManager ) {
    RET_VAL ret = SUCCESS;
    UNIT_MANAGER *unitManager;

    START_FUNCTION("_AddGlobalParamInSymtab");
    unitManager = GetUnitManagerInstance( frontend->record );
    if( IS_FAILED( ( ret = sbmlSymtabManager->PutParametersInGlobalSymtab( sbmlSymtabManager, symtab, unitManager ) ) ) ) {
        END_FUNCTION("_AddGlobalParamInSymtab", ret );
        return ret;
    }
    
    END_FUNCTION("_AddGlobalParamInSymtab", SUCCESS );
    return SUCCESS;

}

static RET_VAL _HandleUnitDefinitions( FRONT_END_PROCESSOR *frontend, Model_t *model ) {
    RET_VAL ret = SUCCESS;
    UINT i = 0;
    UINT size = 0;
    ListOf_t *list = NULL;
    UnitDefinition_t *unitDef = NULL;
    
    START_FUNCTION("_HandleUnitDefinitions");
        
    list = Model_getListOfUnitDefinitions( model );
    size = Model_getNumUnitDefinitions( model );
    for( i = 0; i < size; i++ ) {
        unitDef = (UnitDefinition_t*)ListOf_get( list, i );
        if( IS_FAILED( ( ret = _HandleUnitDefinition( frontend, model, unitDef ) ) ) ) {
            END_FUNCTION("_HandleUnitDefinitions", ret );
            return ret;
        } 
    }
        
    END_FUNCTION("_HandleUnitDefinitions", SUCCESS );
    return ret;
}

static RET_VAL _HandleUnitDefinition( FRONT_END_PROCESSOR *frontend, Model_t *model, UnitDefinition_t *source ) {
    RET_VAL ret = SUCCESS;
    int i = 0;
    int num = 0;
    double exponent = 0;
    int scale = 0;
    double multiplier = 0.0;
    char *id = NULL;
    char *kind = NULL;
    UnitKind_t kindID = 0;
    Unit_t *unit = NULL;
    UNIT_DEFINITION *unitDef = NULL; 
    UNIT_MANAGER *unitManager = NULL;
    HASH_TABLE *table = NULL;
    
    START_FUNCTION("_HandleUnitDefinition");
    
    if( ( unitManager = GetUnitManagerInstance( frontend->record ) ) == NULL ) {
        return ErrorReport( FAILING, "_HandleUnitDefinition", "could not get an instance of unit manager" );
    }
    
    id = (char *)UnitDefinition_getId( source );
    TRACE_1("creating unit definition %s", id );
    if( ( unitDef = unitManager->CreateUnitDefinition( unitManager, id, FALSE ) ) == NULL ) {
        return ErrorReport( FAILING, "_HandleUnitDefinition", "could not allocate unit def %s", id );
    }
    num = UnitDefinition_getNumUnits( source );
    for( i = 0; i < num; i++ ) {
        unit = UnitDefinition_getUnit( source, i );
        kindID = Unit_getKind( unit );
        kind = (char *)UnitKind_toString( kindID );
        exponent = Unit_getExponentAsDouble( unit );
        scale = Unit_getScale( unit );
        multiplier = Unit_getMultiplier( unit );
	//multiplier = 1.0;
        //printf( "adding unit %s (exponent=%g, scale=%d, multiplier=%g) in %s\n", kind, exponent, scale, multiplier, id );           
        //TRACE_5( "adding unit %s (exponent=%i, scale=%i, multiplier=%f) in %s", kind, exponent, scale, multiplier, id );           
        if( IS_FAILED( ( ret = AddUnitInUnitDefinition( unitDef, kind, exponent, scale, multiplier ) ) ) ) {
            END_FUNCTION("_HandleUnitDefinition", ret );
            return ret;
        }
    }                             
    
    END_FUNCTION("_HandleUnitDefinition", SUCCESS );
    return ret;
}

static RET_VAL _HandleFunctionDefinitions( FRONT_END_PROCESSOR *frontend, Model_t *model ) {
    RET_VAL ret = SUCCESS;
    UINT i = 0;
    UINT size = 0;
    ListOf_t *list = NULL;
    FunctionDefinition_t *functionDef = NULL;
    
    START_FUNCTION("_HandleFunctionDefinitions");

    workingOnFunctions = 1;
    
    list = Model_getListOfFunctionDefinitions( model );
    size = Model_getNumFunctionDefinitions( model );
    for( i = 0; i < size; i++ ) {
        functionDef = (FunctionDefinition_t*)ListOf_get( list, i );
        if( IS_FAILED( ( ret = _HandleFunctionDefinition( frontend, model, functionDef ) ) ) ) {
            END_FUNCTION("_HandleFunctionDefinitions", ret );
            return ret;
        } 
    }
    
    workingOnFunctions = 0;
    
    END_FUNCTION("_HandleFunctionDefinitions", SUCCESS );
    return ret;
}

static RET_VAL _HandleFunctionDefinition( FRONT_END_PROCESSOR *frontend, Model_t *model, FunctionDefinition_t *source ) {
    RET_VAL ret = SUCCESS;
    int i = 0;
    int num = 0;
    char *id = NULL;
    char *argument = NULL;
    FUNCTION_DEFINITION *functionDef = NULL; 
    FUNCTION_MANAGER *functionManager = NULL;
    HASH_TABLE *table = NULL;
    ASTNode_t *node = NULL;
    KINETIC_LAW *law = NULL;
    SBML_SYMTAB_MANAGER *manager = NULL;
    
    START_FUNCTION("_HandleFunctionDefinition");
    
    if( ( functionManager = GetFunctionManagerInstance( frontend->record ) ) == NULL ) {
        return ErrorReport( FAILING, "_HandleFunctionDefinition", "could not get an instance of function manager" );
    }
    
    id = (char *)FunctionDefinition_getId( source );
    TRACE_1("creating function definition %s", id );
    if( ( functionDef = functionManager->CreateFunctionDefinition( functionManager, id ) ) == NULL ) {
        return ErrorReport( FAILING, "_HandleFunctionDefinition", "could not allocate function def %s", id );
    }

    num = FunctionDefinition_getNumArguments( source );
    for( i = 0; i < num; i++ ) {
      argument = SBML_formulaToString(FunctionDefinition_getArgument( source, i ));
      TRACE_1( "adding argument %s", argument );           
      if( IS_FAILED( ( ret = AddArgumentInFunctionDefinition( functionDef, argument ) ) ) ) {
	END_FUNCTION("_HandleFunctionDefinition", ret );
	return ret;
      }
    }                             
    node = (ASTNode_t *)FunctionDefinition_getBody( source );
    if( ( manager = GetSymtabManagerInstance( frontend->record ) ) == NULL ) {
        return ErrorReport( FAILING, "_HandleFunctionDefinition", "error on getting symtab manager" ); 
    }
    table = (HASH_TABLE*)frontend->_internal2;    
    if( ( law = _TransformKineticLaw( frontend, node, manager, table ) ) == NULL ) {
        return ErrorReport( FAILING, "_HandleFunctionDefinition", "failed to create function for %s", id );        
    }
    if( IS_FAILED( ( ret = AddFunctionInFunctionDefinition( functionDef, law ) ) ) ) {
      END_FUNCTION("_HandleFunctionDefinition", ret );
      return ret;
    }

    END_FUNCTION("_HandleFunctionDefinition", SUCCESS );
    return ret;
}

static RET_VAL _HandleAlgebraicRules( FRONT_END_PROCESSOR *frontend, Model_t *model ) {
    RET_VAL ret = SUCCESS;
    UINT i = 0;
    UINT size = 0;
    ListOf_t *list = NULL;
    Rule_t *ruleDef = NULL;
    
    START_FUNCTION("_HandleAlgebraicRules");
    
    list = Model_getListOfRules( model );
    size = Model_getNumRules( model );
    for( i = 0; i < size; i++ ) {
        ruleDef = (Rule_t*)ListOf_get( list, i );
	if (Rule_isAlgebraic( ruleDef )) {
	  if( IS_FAILED( ( ret = _HandleAlgebraicRule( frontend, model, ruleDef ) ) ) ) {
            END_FUNCTION("_HandleAlgebraicRules", ret );
            return ret;
	  } 
	}
    }
    
    END_FUNCTION("_HandleRules", SUCCESS );
    return ret;
}

static RET_VAL _HandleAlgebraicRule( FRONT_END_PROCESSOR *frontend, Model_t *model, Rule_t *source ) {
    RET_VAL ret = SUCCESS;
    HASH_TABLE *table = NULL;
    ASTNode_t *node = NULL;
    KINETIC_LAW *law = NULL;
    SBML_SYMTAB_MANAGER *manager = NULL;
    RULE *ruleDef = NULL; 
    RULE_MANAGER *ruleManager = NULL;
    SPECIES *speciesNode = NULL;
    REB2SAC_SYMTAB *symtab = (REB2SAC_SYMTAB*)frontend->_internal3;
    REB2SAC_SYMBOL *symbol = NULL;
    COMPARTMENT *compartment = NULL;
    COMPARTMENT_MANAGER *compartmentManager = NULL;
    LINKED_LIST *list = NULL;
    KINETIC_LAW_SUPPORT *kineticLawSupport;
    LINKED_LIST *supportList = NULL;
    STRING *sym;
    char *var;

    START_FUNCTION("_HandleAlgebraicRule");
    
    kineticLawSupport = CreateKineticLawSupport();
    if( ( ruleManager = GetRuleManagerInstance( frontend->record ) ) == NULL ) {
        return ErrorReport( FAILING, "_HandleAlgebraicRule", "could not get an instance of rule manager" );
    }
    if( ( compartmentManager = GetCompartmentManagerInstance( frontend->record ) ) == NULL ) {
        END_FUNCTION("_HandleAlgebraicRule", FAILING );
        return FAILING;
    }
    table = (HASH_TABLE*)frontend->_internal2;
    TRACE_1("creating rule on %s", var );
    if( ( ruleDef = ruleManager->CreateRule( ruleManager, RULE_TYPE_ALGEBRAIC, NULL ) ) == NULL ) {
        return ErrorReport( FAILING, "_HandleRule", "could not allocate algebraic rule");
    }
    node = (ASTNode_t *)Rule_getMath( source );
    if( ( manager = GetSymtabManagerInstance( frontend->record ) ) == NULL ) {
        return ErrorReport( FAILING, "_HandleRule", "error on getting symtab manager" ); 
    }
    if( ( law = _TransformKineticLaw( frontend, node, manager, table ) ) == NULL ) {
        return ErrorReport( FAILING, "_HandleRule", "failed to create rule on %s", var );        
    }
    if( IS_FAILED( ( ret = AddMathInRule( ruleDef, law ) ) ) ) {
      END_FUNCTION("_HandleRuleDefinition", ret );
      return ret;
    }
    if (law != NULL) {
      supportList = kineticLawSupport->Support( kineticLawSupport, law );
      ResetCurrentElement( supportList );
      while( ( sym = (STRING*)GetNextFromLinkedList( supportList ) ) != NULL ) {
	var = GetCharArrayOfString( sym );
	if ( (speciesNode = (SPECIES*)GetValueFromHashTable( var, strlen( var ), table )) != NULL) {
	  if (!IsSpeciesNodeConstant( speciesNode )) {
	    SetAlgebraicInSpeciesNode( speciesNode, TRUE );
	  }
	} else if( ( symbol = symtab->Lookup( symtab, var ) ) != NULL ) {
	  if ((strcmp(GetCharArrayOfString( GetSymbolID(symbol) ),"time")==0) ||
	      (strcmp(GetCharArrayOfString( GetSymbolID(symbol) ),"t")==0)) continue;
	  if (!IsSymbolConstant( symbol )) {
	    SetSymbolAlgebraic( symbol, TRUE );
	  }
	} else if (compartment = compartmentManager->LookupCompartment( compartmentManager, var )) {
	  if (!IsCompartmentConstant( compartment )) {
	    SetCompartmentAlgebraic( compartment, TRUE );
	  }
	}
      }
    }
    FreeKineticLawSupport(&(kineticLawSupport));

    END_FUNCTION("_HandleAlgebraicRule", SUCCESS );
    return ret;
}

static RET_VAL _HandleRules( FRONT_END_PROCESSOR *frontend, Model_t *model ) {
    RET_VAL ret = SUCCESS;
    UINT i = 0;
    UINT size = 0;
    ListOf_t *list = NULL;
    Rule_t *ruleDef = NULL;
    
    START_FUNCTION("_HandleRules");
    
    list = Model_getListOfRules( model );
    size = Model_getNumRules( model );
    for( i = 0; i < size; i++ ) {
        ruleDef = (Rule_t*)ListOf_get( list, i );
	if (!Rule_isAlgebraic( ruleDef )) {
	  if( IS_FAILED( ( ret = _HandleRule( frontend, model, ruleDef ) ) ) ) {
            END_FUNCTION("_HandleRules", ret );
            return ret;
	  } 
	}
    }
    
    END_FUNCTION("_HandleRules", SUCCESS );
    return ret;
}

static RET_VAL _HandleRule( FRONT_END_PROCESSOR *frontend, Model_t *model, Rule_t *source ) {
    RET_VAL ret = SUCCESS;
    int i = 0;
    int num = 0;
    char *var = NULL;
    BYTE type = 0;
    RULE *ruleDef = NULL; 
    RULE_MANAGER *ruleManager = NULL;
    HASH_TABLE *table = NULL;
    ASTNode_t *node = NULL;
    KINETIC_LAW *law = NULL;
    SBML_SYMTAB_MANAGER *manager = NULL;
    SPECIES *speciesNode = NULL;
    REB2SAC_SYMTAB *symtab = (REB2SAC_SYMTAB*)frontend->_internal3;
    REB2SAC_SYMBOL *symbol = NULL;
    COMPARTMENT *compartment = NULL;
    COMPARTMENT_MANAGER *compartmentManager = NULL;

    START_FUNCTION("_HandleRule");
    
    if( ( ruleManager = GetRuleManagerInstance( frontend->record ) ) == NULL ) {
        return ErrorReport( FAILING, "_HandleRule", "could not get an instance of rule manager" );
    }
    if( ( compartmentManager = GetCompartmentManagerInstance( frontend->record ) ) == NULL ) {
        END_FUNCTION("_HandleRule", FAILING );
        return FAILING;
    }
    table = (HASH_TABLE*)frontend->_internal2;    
    if (Rule_isAlgebraic( source )) {
      //printf("Algebraic rules are currently ignored.\n");
      type = RULE_TYPE_ALGEBRAIC;
      var = NULL;
      //return ret;
    } else if (Rule_isAssignment( source )) {
      type = RULE_TYPE_ASSIGNMENT;
      var = (char *)Rule_getVariable( source );
      if ( (speciesNode = (SPECIES*)GetValueFromHashTable( var, strlen( var ), table )) != NULL) {
	SetAlgebraicInSpeciesNode( speciesNode, FALSE );
      } else if( ( symbol = symtab->Lookup( symtab, var ) ) != NULL ) {
	SetSymbolAlgebraic( symbol, FALSE );
      } else if (compartment = compartmentManager->LookupCompartment( compartmentManager, var )) {
	SetCompartmentAlgebraic( compartment, FALSE );
      }
    } else {
      type = RULE_TYPE_RATE_ASSIGNMENT;
      var = (char *)Rule_getVariable( source );
      if ((speciesNode = (SPECIES*)GetValueFromHashTable( var, strlen( var ), table )) != NULL) {
	SetAlgebraicInSpeciesNode( speciesNode, FALSE );
      } else if( ( symbol = symtab->Lookup( symtab, var ) ) != NULL ) {
	SetSymbolAlgebraic( symbol, FALSE );
      } else if (compartment = compartmentManager->LookupCompartment( compartmentManager, var )) {
	SetCompartmentAlgebraic( compartment, FALSE );
      }
    }
    TRACE_1("creating rule on %s", var );
    if( ( ruleDef = ruleManager->CreateRule( ruleManager, type, var ) ) == NULL ) {
        return ErrorReport( FAILING, "_HandleRule", "could not allocate rule on %s", var );
    }

    node = (ASTNode_t *)Rule_getMath( source );
    if( ( manager = GetSymtabManagerInstance( frontend->record ) ) == NULL ) {
        return ErrorReport( FAILING, "_HandleRule", "error on getting symtab manager" ); 
    }
    if( ( law = _TransformKineticLaw( frontend, node, manager, table ) ) == NULL ) {
        return ErrorReport( FAILING, "_HandleRule", "failed to create rule on %s", var );        
    }
    if( IS_FAILED( ( ret = AddMathInRule( ruleDef, law ) ) ) ) {
      END_FUNCTION("_HandleRuleDefinition", ret );
      return ret;
    }

    END_FUNCTION("_HandleRule", SUCCESS );
    return ret;
}

static RET_VAL _HandleConstraints( FRONT_END_PROCESSOR *frontend, Model_t *model ) {
    RET_VAL ret = SUCCESS;
    UINT i = 0;
    UINT size = 0;
    ListOf_t *list = NULL;
    Constraint_t *constraintDef = NULL;
    
    START_FUNCTION("_HandleConstraints");
    
    list = Model_getListOfConstraints( model );
    size = Model_getNumConstraints( model );
    for( i = 0; i < size; i++ ) {
        constraintDef = (Constraint_t*)ListOf_get( list, i );
        if( IS_FAILED( ( ret = _HandleConstraint( frontend, model, constraintDef ) ) ) ) {
            END_FUNCTION("_HandleConstraints", ret );
            return ret;
        } 
    }
    
    END_FUNCTION("_HandleConstraints", SUCCESS );
    return ret;
}

static RET_VAL _HandleConstraint( FRONT_END_PROCESSOR *frontend, Model_t *model, Constraint_t *source ) {
    RET_VAL ret = SUCCESS;
    int i = 0;
    int num = 0;
    char *id = NULL;
    char *message = NULL;
    CONSTRAINT *constraintDef = NULL; 
    CONSTRAINT_MANAGER *constraintManager = NULL;
    HASH_TABLE *table = NULL;
    ASTNode_t *node = NULL;
    KINETIC_LAW *law = NULL;
    SBML_SYMTAB_MANAGER *manager = NULL;
    
    START_FUNCTION("_HandleConstraint");
    
    if( ( constraintManager = GetConstraintManagerInstance( frontend->record ) ) == NULL ) {
        return ErrorReport( FAILING, "_HandleConstraint", "could not get an instance of constraint manager" );
    }
    id = (char *)SBase_getMetaId( (SBase_t*)source );
    TRACE_1("creating constraint %s", id );
    if( ( constraintDef = constraintManager->CreateConstraint( constraintManager, id ) ) == NULL ) {
        return ErrorReport( FAILING, "_HandleConstraint", "could not allocate constraint %s", id );
    }

    node = (ASTNode_t *)Constraint_getMath( source );
    if( ( manager = GetSymtabManagerInstance( frontend->record ) ) == NULL ) {
        return ErrorReport( FAILING, "_HandleConstraint", "error on getting symtab manager" ); 
    }
    table = (HASH_TABLE*)frontend->_internal2;    
    if( ( law = _TransformKineticLaw( frontend, node, manager, table ) ) == NULL ) {
        return ErrorReport( FAILING, "_HandleConstraint", "failed to create constraint %s", id );        
    }
    if( IS_FAILED( ( ret = AddMathInConstraint( constraintDef, law ) ) ) ) {
      END_FUNCTION("_HandleConstraintDefinition", ret );
      return ret;
    }
    message = NULL;
    if (Constraint_isSetMessage( source )) {
      message = (char *)XMLNode_convertXMLNodeToString( (const XMLNode_t*)Constraint_getMessage( source ));
      message = strstr(message,"xhtml") + 7;
      *(strstr(message,"</p>")) = '\0';
      AddMessageInConstraint( constraintDef, message);
    }
	
    END_FUNCTION("_HandleConstraint", SUCCESS );
    return ret;
}

static RET_VAL _HandleEvents( FRONT_END_PROCESSOR *frontend, Model_t *model ) {
    RET_VAL ret = SUCCESS;
    UINT i = 0;
    UINT size = 0;
    ListOf_t *list = NULL;
    Event_t *eventDef = NULL;
    
    START_FUNCTION("_HandleEvents");
    
    list = Model_getListOfEvents( model );
    size = Model_getNumEvents( model );
    for( i = 0; i < size; i++ ) {
        eventDef = (Event_t*)ListOf_get( list, i );
        if( IS_FAILED( ( ret = _HandleEvent( frontend, model, eventDef ) ) ) ) {
            END_FUNCTION("_HandleEvents", ret );
            return ret;
        } 
    }
    
    END_FUNCTION("_HandleEvents", SUCCESS );
    return ret;
}

static RET_VAL _HandleEvent( FRONT_END_PROCESSOR *frontend, Model_t *model, Event_t *source ) {
    RET_VAL ret = SUCCESS;
    int i = 0;
    int num = 0;
    char *id = NULL;
    char *message = NULL;
    char *annotation = NULL;
    EVENT *eventDef = NULL; 
    EVENT_MANAGER *eventManager = NULL;
    HASH_TABLE *table = NULL;
    ASTNode_t *node = NULL;
    KINETIC_LAW *law = NULL;
    SBML_SYMTAB_MANAGER *manager = NULL;
    Trigger_t *trigger = NULL;
    Delay_t *delay = NULL;
    Priority_t *priority = NULL;
    UINT size = 0;
    ListOf_t *list = NULL;
    EventAssignment_t *eventAssignmentDef;
    char *var = NULL;
    SPECIES *speciesNode = NULL;
    REB2SAC_SYMTAB *symtab = (REB2SAC_SYMTAB*)frontend->_internal3;
    REB2SAC_SYMBOL *symbol = NULL;
    COMPARTMENT *compartment = NULL;
    COMPARTMENT_MANAGER *compartmentManager = NULL;

    START_FUNCTION("_HandleEvent");
    
    if( ( eventManager = GetEventManagerInstance( frontend->record ) ) == NULL ) {
        return ErrorReport( FAILING, "_HandleEvent", "could not get an instance of event manager" );
    }
    if( ( compartmentManager = GetCompartmentManagerInstance( frontend->record ) ) == NULL ) {
        END_FUNCTION("_HandleEvent", FAILING );
        return FAILING;
    }
    id = (char *)Event_getId( source );
    if( ( eventDef = eventManager->CreateEvent( eventManager, id ) ) == NULL ) {
        return ErrorReport( FAILING, "_HandleEvent", "could not allocate event %s", id );
    }

    if( ( manager = GetSymtabManagerInstance( frontend->record ) ) == NULL ) {
        return ErrorReport( FAILING, "_HandleEvent", "error on getting symtab manager" ); 
    }
    table = (HASH_TABLE*)frontend->_internal2;    

    SetUseValuesFromTriggerTime( eventDef, Event_getUseValuesFromTriggerTime( source ) );
    if (Event_isSetTrigger( source ) ) {
      trigger = Event_getTrigger( source );
      annotation = SBase_getAnnotationString( trigger );
      if (annotation != NULL && strstr(annotation,"TriggerCanBeDisabled") != NULL) {
	SetTriggerCanBeDisabled( eventDef, TRUE );
      } else {
	SetTriggerCanBeDisabled( eventDef, FALSE );
      }
      if (annotation != NULL && strstr(annotation,"TriggerInitiallyFalse") != NULL) {
	SetTriggerInitialValue( eventDef, FALSE );
      } else {
	SetTriggerInitialValue( eventDef, TRUE );
      }

      /* new libsbml */
      if (Trigger_getPersistent( trigger )) {
	SetTriggerCanBeDisabled( eventDef, FALSE );
      } else {
	SetTriggerCanBeDisabled( eventDef, TRUE );
      }
      if (Trigger_getInitialValue( trigger )) {
	SetTriggerInitialValue( eventDef, TRUE );
      } else {
	SetTriggerInitialValue( eventDef, FALSE );
      }

      node  = (ASTNode_t *)Trigger_getMath( trigger );
      if( ( law = _TransformKineticLaw( frontend, node, manager, table ) ) == NULL ) {
        return ErrorReport( FAILING, "_HandleEvent", "failed to create event %s", id );        
      }
      if( IS_FAILED( ( ret = AddTriggerInEvent( eventDef, law ) ) ) ) {
	END_FUNCTION("_HandleEvent", ret );
	return ret;
      }
    }

    if (Event_isSetDelay( source ) ) {
      delay = Event_getDelay( source );
      node  = (ASTNode_t *)Delay_getMath( delay );
      if ((ASTNode_getType( node )== AST_FUNCTION) && (strcmp(ASTNode_getName( node ),"priority")==0)) {
	  if( ( law = _TransformKineticLaw( frontend, ASTNode_getLeftChild( node ), manager, table ) ) == NULL ) {
	    return ErrorReport( FAILING, "_HandleEvent", "failed to create event %s", id );        
	  }
	  if( IS_FAILED( ( ret = AddDelayInEvent( eventDef, law ) ) ) ) {
	    END_FUNCTION("_HandleEvent", ret );
	    return ret;
	  }
	  if( ( law = _TransformKineticLaw( frontend, ASTNode_getRightChild( node ), manager, table ) ) == NULL ) {
	    return ErrorReport( FAILING, "_HandleEvent", "failed to create event %s", id );        
	  }
	  if( IS_FAILED( ( ret = AddPriorityInEvent( eventDef, law ) ) ) ) {
	    END_FUNCTION("_HandleEvent", ret );
	    return ret;
	  }
      } else {
	if( ( law = _TransformKineticLaw( frontend, node, manager, table ) ) == NULL ) {
	  return ErrorReport( FAILING, "_HandleEvent", "failed to create event %s", id );        
	}
	if( IS_FAILED( ( ret = AddDelayInEvent( eventDef, law ) ) ) ) {
	  END_FUNCTION("_HandleEvent", ret );
	  return ret;
	}
      }
    }

    /* new libsbml */
    if (Event_isSetPriority( source ) ) {
      priority = Event_getPriority( source );
      node  = (ASTNode_t *)Priority_getMath( priority );
      if( ( law = _TransformKineticLaw( frontend, node, manager, table ) ) == NULL ) {
	return ErrorReport( FAILING, "_HandleEvent", "failed to create event %s", id );        
      }
      if( IS_FAILED( ( ret = AddPriorityInEvent( eventDef, law ) ) ) ) {
	END_FUNCTION("_HandleEvent", ret );
	return ret;
      }
    }

    list = Event_getListOfEventAssignments( source );
    size = Event_getNumEventAssignments( source );
    for( i = 0; i < size; i++ ) {
        eventAssignmentDef = (EventAssignment_t*)ListOf_get( list, i );
	node  = (ASTNode_t *)EventAssignment_getMath( eventAssignmentDef );
	if( ( law = _TransformKineticLaw( frontend, node, manager, table ) ) == NULL ) {
	  return ErrorReport( FAILING, "_HandleEvent", "failed to create event %s", id );        
	}
	var = (char *)EventAssignment_getVariable(eventAssignmentDef);
	if ((speciesNode = (SPECIES*)GetValueFromHashTable( var, strlen( var ), table )) != NULL) {
	  SetAlgebraicInSpeciesNode( speciesNode, FALSE );
	} else if( ( symbol = symtab->Lookup( symtab, var ) ) != NULL ) {
	  SetSymbolAlgebraic( symbol, FALSE );
	} else if (compartment = compartmentManager->LookupCompartment( compartmentManager, var )) {
	  SetCompartmentAlgebraic( compartment, FALSE );
	}
	if( IS_FAILED( ( ret = AddEventAssignmentToEvent( eventDef, (char*)EventAssignment_getVariable(eventAssignmentDef), law ) ) ) ) {
	  return ErrorReport( FAILING, "_HandleEvent", "could not allocate event %s", id );
	}
    }
	
    END_FUNCTION("_HandleEvent", SUCCESS );
    return ret;
}


static RET_VAL _HandleCompartments( FRONT_END_PROCESSOR *frontend, Model_t *model ) {
    RET_VAL ret = SUCCESS;
    UINT i = 0;
    UINT size = 0;
    ListOf_t *list = NULL;
    Compartment_t *compartment = NULL;
    COMPARTMENT_MANAGER *manager = NULL;
    
    START_FUNCTION("_HandleCompartments");
        
    list = Model_getListOfCompartments( model );
    size = Model_getNumCompartments( model );
    for( i = 0; i < size; i++ ) {
        compartment = (Compartment_t*)ListOf_get( list, i );
        if( IS_FAILED( ( ret = _HandleCompartment( frontend, model, compartment ) ) ) ) {
            END_FUNCTION("_HandleCompartments", ret );
            return ret;
        } 
    }
    if( ( manager = GetCompartmentManagerInstance( frontend->record ) ) == NULL ) {
        return ErrorReport( FAILING, "_HandleCompartments", "could not get an instance of compartment manager" );
    }
    if( IS_FAILED( ( ret = manager->ResolveCompartmentLinks( manager ) ) ) ) {
        END_FUNCTION("_HandleCompartments", ret );
        return ret;
    }
            
    END_FUNCTION("_HandleCompartments", SUCCESS );
    return ret;
}

static RET_VAL _HandleCompartment( FRONT_END_PROCESSOR *frontend, Model_t *model, Compartment_t *source ) {
    RET_VAL ret = SUCCESS;
    char *id = NULL;    
    double spatialDimensions = 3.0;
    double size = 0.0;
    char *units = NULL;
    UNIT_DEFINITION *unitDef = NULL;
    UNIT_MANAGER *unitManager = NULL;
    char *outside = NULL;
    char *type = NULL;
    BOOL constant = TRUE;                
    COMPARTMENT *compartment = NULL;
    COMPARTMENT_MANAGER *manager = NULL;
        
    START_FUNCTION("_HandleCompartment");
    
    if( ( manager = GetCompartmentManagerInstance( frontend->record ) ) == NULL ) {
        return ErrorReport( FAILING, "_HandleCompartment", "could not get an instance of compartment manager" );
    }
    
    id = (char*)Compartment_getId( source );
    TRACE_1("creating compartment %s", id );
    if( ( compartment = manager->CreateCompartment( manager, id ) ) == NULL ) {
        return ErrorReport( FAILING, "_HandleCompartment", "could not allocate compartment %s", id );
    }
    
    spatialDimensions = Compartment_getSpatialDimensionsAsDouble( source );
    if( IS_FAILED( ( ret = SetSpatialDimensionsInCompartment( compartment, spatialDimensions ) ) ) ) {
        END_FUNCTION("_HandleCompartment", ret );
        return ret;
    }
    
    if( Compartment_isSetSize( source ) ) {
        size = Compartment_getSize( source );
        if( IS_FAILED( ( ret = SetSizeInCompartment( compartment, size ) ) ) ) {
            END_FUNCTION("_HandleCompartment", ret );
            return ret;
        }
    }
    else {
        if( IS_FAILED( ( ret = SetSizeInCompartment( compartment, 1.0 ) ) ) ) {
            END_FUNCTION("_HandleCompartment", ret );
            return ret;
        }
    }    
    if( IS_FAILED( ( ret = SetCurrentRateInCompartment( compartment, 0.0 ) ) ) ) {
      END_FUNCTION("_HandleCompartment", ret );
      return ret;
    }
        
    if( (units = (char*)Compartment_getUnits( source )) != NULL ) {
        unitManager = GetUnitManagerInstance( frontend->record );
        if( ( unitDef = unitManager->LookupUnitDefinition( unitManager, units ) ) == NULL ) {
            return ErrorReport( FAILING, "_HandleUnitDefinition", "unit def %s is not declared", units );
        }             
        if( IS_FAILED( ( ret = SetUnitInCompartment( compartment, unitDef  ) ) ) ) {
            END_FUNCTION("_HandleCompartment", ret );
            return ret;
        }
    }
        
    if( ( outside = (char*)Compartment_getOutside( source ) ) != NULL ) {
        if( IS_FAILED( ( ret = SetOutsideInCompartment( compartment, outside ) ) ) ) {
            END_FUNCTION("_HandleCompartment", ret );
            return ret;
        }
    }

    if( ( type = (char*)Compartment_getCompartmentType( source ) ) != NULL ) {
        if( IS_FAILED( ( ret = SetTypeInCompartment( compartment, type ) ) ) ) {
            END_FUNCTION("_HandleCompartment", ret );
            return ret;
        }
    }
    
    constant = (Compartment_getConstant( source ) == 0 ? FALSE : TRUE);
    if( IS_FAILED( ( ret = SetCompartmentConstant( compartment, constant ) ) ) ) {
        END_FUNCTION("_HandleCompartment", ret );
        return ret;
    }
    
    END_FUNCTION("_HandleCompartment", SUCCESS );
    return ret;
}



static RET_VAL _CreateSpeciesNodes(  FRONT_END_PROCESSOR *frontend, IR *ir, Model_t *model ) {
    RET_VAL ret = SUCCESS;
    UINT i = 0;
    UINT size = 0;
    ListOf_t *list = NULL;
    Species_t *species = NULL;
    
    START_FUNCTION("_CreateSpeciesNodes");
        
    
    list = Model_getListOfSpecies( model );
    size = Model_getNumSpecies( model );
    for( i = 0; i < size; i++ ) {
        species = (Species_t*)ListOf_get( list, i );
        if( IS_FAILED( ( ret = _CreateSpeciesNode( frontend, ir, model, species ) ) ) ) {
            END_FUNCTION("_CreateSpeciesNodes", ret );
            return ret;
        } 
    }
    END_FUNCTION("_CreateSpeciesNodes", SUCCESS );
    return ret;
}

static RET_VAL _CreateSpeciesNode(  FRONT_END_PROCESSOR *frontend, IR *ir, Model_t *model, Species_t *species ) {
    RET_VAL ret = SUCCESS;
    char *id = NULL;
    char *key = NULL;
    double initialQuantity = 0.0;
    COMPARTMENT *compartment = NULL;
    UNIT_DEFINITION *units = NULL;
    SPECIES *speciesNode = NULL; 
    HASH_TABLE *table = NULL;
    COMPARTMENT_MANAGER *compartmentManager = NULL;
    UNIT_MANAGER *unitManager = NULL;
    UINT i = 0;
    UINT size = 0;
    ListOf_t *list = NULL;
    InitialAssignment_t *initialAssignment = NULL;
    ASTNode_t *node = NULL;
    KINETIC_LAW *law = NULL;
    SBML_SYMTAB_MANAGER *manager = NULL;
    char *type = NULL;
    char *convFactorStr = NULL;
    REB2SAC_SYMBOL *convFactor = NULL;
    REB2SAC_SYMTAB *symtab = NULL;

    START_FUNCTION("_CreateSpeciesNode");
    
    table = (HASH_TABLE*)frontend->_internal2;
    
    key = (char*)Species_getId( species );
    /*
        use name as the id
    */
    /* id = Species_getName( species );
       if( id == NULL ) { */
    id = key;
    /*}*/
    
    TRACE_2("species id = %s and key for sym is %s", id, key );
    
    if( ( speciesNode = ir->CreateSpecies( ir, id ) ) == NULL ) {
        END_FUNCTION("_CreateSpeciesNode", ret );
        return ret;   
    }
    
    if( IS_FAILED( ( ret = PutInHashTable( key, strlen( key ), (CADDR_T)speciesNode, table ) ) ) ) {
        END_FUNCTION("_CreateSpeciesNode", ret );
        return ret;   
    }  
                
    if( ( compartmentManager = GetCompartmentManagerInstance( frontend->record ) ) == NULL ) {
        return ErrorReport( FAILING, "_CreateSpeciesNode", "could not get an instance of compartment manager" );
    }
    id = (char*)Species_getCompartment( species );
    if( id != NULL ) {
        if( ( compartment = compartmentManager->LookupCompartment( compartmentManager, id ) ) == NULL ) {
            return ErrorReport( FAILING, "_CreateSpeciesNode", "compartment %s is not defined", id );
        }
        TRACE_1( "\tsetting compartment %s", id );
        if( IS_FAILED( ( ret = SetCompartmentInSpeciesNode( speciesNode, compartment ) ) ) ) {
            END_FUNCTION("_CreateSpeciesNode", ret );
            return ret;   
        }
    }        

    if( Species_isSetInitialAmount( species ) ) {
        initialQuantity = Species_getInitialAmount( species );
        TRACE_1( "\tsetting initial amount %g", initialQuantity );
        if( IS_FAILED( ( ret = SetInitialAmountInSpeciesNode( speciesNode, initialQuantity ) ) ) ) {
            END_FUNCTION("_CreateSpeciesNode", ret );
            return ret;   
        }
    }
    else {
        if( Species_isSetInitialConcentration( species ) ) {
            initialQuantity = Species_getInitialConcentration( species );
            TRACE_1( "\tsetting initial concentration %g", initialQuantity );
            if( IS_FAILED( ( ret = SetInitialConcentrationInSpeciesNode( speciesNode, initialQuantity ) ) ) ) {
                END_FUNCTION("_CreateSpeciesNode", ret );
                return ret;   
            }
        }
        else {
            if( IS_FAILED( ( ret = SetInitialConcentrationInSpeciesNode( speciesNode, 0.0 ) ) ) ) {
                END_FUNCTION("_CreateSpeciesNode", ret );
                return ret;   
            }
        }
    }
    if( IS_FAILED( ( ret = SetRateInSpeciesNode( speciesNode, 0.0 ) ) ) ) {
      END_FUNCTION("_CreateSpeciesNode", ret );
      return ret;   
    }
    
    if( ( unitManager = GetUnitManagerInstance( frontend->record ) ) == NULL ) {
        return ErrorReport( FAILING, "_CreateSpeciesNode", "could not get an instance of unit manager" );
    }
    id = (char*)Species_getSubstanceUnits( species );
    if( id != NULL ) {
        if( ( units = unitManager->LookupUnitDefinition( unitManager, id ) ) == NULL ) {
            return ErrorReport( FAILING, "_CreateSpeciesNode", "unit %s is not defined", id );
        }
        TRACE_1( "\tsetting substance units %s", id );
        if( IS_FAILED( ( ret = SetSubstanceUnitsInSpeciesNode( speciesNode, units ) ) ) ) {
            END_FUNCTION("_CreateSpeciesNode", ret );
            return ret;   
        }
    }        
    id = (char*)Species_getSpatialSizeUnits( species );
    if( id != NULL ) {
        if( ( units = unitManager->LookupUnitDefinition( unitManager, id ) ) == NULL ) {
            return ErrorReport( FAILING, "_CreateSpeciesNode", "unit %s is not defined", id );
        }
        TRACE_1( "\tsetting spatial size units %s", id );
        if( IS_FAILED( ( ret = SetSpatialSizeUnitsInSpeciesNode( speciesNode, units ) ) ) ) {
            END_FUNCTION("_CreateSpeciesNode", ret );
            return ret;   
        }
    }        

    if( Species_getSpeciesType( species ) ) {
        type = (char *)Species_getSpeciesType( species );                
        TRACE_1( "\tsetting type %s", type );
        if( IS_FAILED( ( ret = SetTypeInSpeciesNode(speciesNode, type ) ) ) ) {
            END_FUNCTION("_CreateSpeciesNode", ret );
            return ret;   
        }
    }
    if (Species_isSetConversionFactor( species )) {
      convFactorStr = (char*)Species_getConversionFactor( species );
      TRACE_1( "\tsetting conversion factor %s", convFactorStr );
      symtab = (REB2SAC_SYMTAB*)(frontend->_internal3);
      if( ( convFactor = symtab->Lookup( symtab, convFactorStr ) ) != NULL ) {
	if( IS_FAILED( ( ret = SetConversionFactorInSpeciesNode(speciesNode, convFactor ) ) ) ) {
	  END_FUNCTION("_CreateSpeciesNode", ret );
	  return ret;   
	}
      }
    } else if (Model_isSetConversionFactor( model )) {
      convFactorStr = (char*)Model_getConversionFactor( model );
      TRACE_1( "\tsetting conversion factor %s", convFactorStr );
      symtab = (REB2SAC_SYMTAB*)(frontend->_internal3);
      if( ( convFactor = symtab->Lookup( symtab, convFactorStr ) ) != NULL ) {
	if( IS_FAILED( ( ret = SetConversionFactorInSpeciesNode(speciesNode, convFactor ) ) ) ) {
	  END_FUNCTION("_CreateSpeciesNode", ret );
	  return ret;   
	}
      }
    }
    if( Species_getConstant( species ) ) {
        TRACE_0( "\tsetting constant true" );
        if( IS_FAILED( ( ret = SetSpeciesNodeConstant( speciesNode, TRUE ) ) ) ) {
            END_FUNCTION("_CreateSpeciesNode", ret );
            return ret;   
        }
    }
    else {
        TRACE_0( "\tsetting constant false" );
        if( IS_FAILED( ( ret = SetSpeciesNodeConstant( speciesNode, FALSE ) ) ) ) {
            END_FUNCTION("_CreateSpeciesNode", ret );
            return ret;   
        }
    }
    
    if( Species_getHasOnlySubstanceUnits( species ) ) {
        TRACE_0( "\tsetting has only substance unit true" );
        if( IS_FAILED( ( ret = SetOnlySubstanceUnitsInSpeciesNode( speciesNode, TRUE ) ) ) ) {
            END_FUNCTION("_CreateSpeciesNode", ret );
            return ret;   
        }
    }
    else {
        TRACE_0( "\tsetting has only substance unit false" );
        if( IS_FAILED( ( ret = SetOnlySubstanceUnitsInSpeciesNode( speciesNode, FALSE ) ) ) ) {
            END_FUNCTION("_CreateSpeciesNode", ret );
            return ret;   
        }
    }
    
    if( Species_getBoundaryCondition( species ) ) {
        TRACE_0( "\tsetting boundary condition true" );
        if( IS_FAILED( ( ret = SetBoundaryConditionInSpeciesNode( speciesNode, TRUE ) ) ) ) {
            END_FUNCTION("_CreateSpeciesNode", ret );
            return ret;   
        }
    }
    else {
        TRACE_0( "\tsetting boundary condition false" );
        if( IS_FAILED( ( ret = SetBoundaryConditionInSpeciesNode( speciesNode, FALSE ) ) ) ) {
            END_FUNCTION("_CreateSpeciesNode", ret );
            return ret;   
        }
    }
                    
    END_FUNCTION("_CreateSpeciesNode", SUCCESS );
    return ret;   
}


static RET_VAL _CreateReactionNodes( FRONT_END_PROCESSOR *frontend, IR *ir, Model_t *model ) {
    RET_VAL ret = SUCCESS;
    UINT i = 0;
    UINT size = 0;
    ListOf_t *list = NULL;
    Reaction_t *reaction = NULL;
    
    START_FUNCTION("_CreateReactionNodes");
            
    list = Model_getListOfReactions( model );
    size = Model_getNumReactions( model );
    for( i = 0; i < size; i++ ) {
        reaction = (Reaction_t*)ListOf_get( list, i );
        if( IS_FAILED( ( ret = _CreateReactionNode( frontend, ir, model, reaction ) ) ) ) {
            END_FUNCTION("_CreateSpeciesNodes", ret );
            return ret;
        } 
    }
    
    END_FUNCTION("_CreateReactionNodes", SUCCESS );
    return ret;
}


static RET_VAL _CreateReactionNode( FRONT_END_PROCESSOR *frontend, IR *ir, Model_t *model, Reaction_t *reaction ) {
    RET_VAL ret = SUCCESS;
    char *id = NULL;
    REACTION *reactionNode = NULL; 
    REACTION_LAW *reactionLaw = NULL; 
    REACTION_LAW_MANAGER *manager = NULL;

    START_FUNCTION("_CreateReactionNode");
            
            
    if( ( manager = GetReactionLawManagerInstance( frontend->record ) ) == NULL ) {
        return ErrorReport( FAILING, "_CreateReactionNode", "could not get an instance of reactionLaw manager" );
    }

    /*if( ( name = Reaction_getName( reaction ) ) == NULL ) {    
        name = Reaction_getId( reaction );
	}*/
    id = (char*)Reaction_getId( reaction );
    TRACE_1("reaction id = %s", id);
    
    if( ( reactionLaw = manager->CreateReactionLaw( manager, id ) ) == NULL ) {
        return ErrorReport( FAILING, "_CreateReactionNode", "could not allocate reactionLaw %s", id );
    }
    
    if( ( reactionNode = ir->CreateReaction( ir, id ) ) == NULL ) {
        END_FUNCTION("_CreateReactionNode", ret );
        return ret;   
    }    
    if (Reaction_isSetCompartment( reaction ) != NULL) {
      if( IS_FAILED( ( ret = SetReactionNodeCompartment( reactionNode, Reaction_getCompartment( reaction ) ) ) ) ) {
	END_FUNCTION("_CreateReactionNode", ret );
	return ret;
      }
    }

    if( Reaction_getFast( reaction ) ) {
        TRACE_0("\tsetting reversible true");
        if( IS_FAILED( ( ret = SetReactionFastInReactionNode( reactionNode, TRUE ) ) ) ) {
            END_FUNCTION("_CreateReactionNode", ret );
            return ret;
        }
    }
    else {
        TRACE_0("\tsetting reversible false");
        if( IS_FAILED( ( ret = SetReactionFastInReactionNode( reactionNode, FALSE ) ) ) ) {
            END_FUNCTION("_CreateReactionNode", ret );
            return ret;
        }
    }
        
    if( IS_FAILED( ( ret = _ResolveNodeLinks( frontend, ir, reactionNode, reaction, model ) ) ) ) {
        END_FUNCTION("_CreateReactionNode", ret );
        return ret;
    }

    if( IS_FAILED( ( ret = _CreateKineticLaw( frontend, reactionNode, reactionLaw, model, reaction ) ) ) ) {
        END_FUNCTION("_CreateReactionNode", ret );
        return ret;
    }
        
    if( Reaction_getReversible( reaction ) ) {
        TRACE_0("\tsetting reversible true");
        if( IS_FAILED( ( ret = SetReactionReversibleInReactionNode( reactionNode, TRUE ) ) ) ) {
            END_FUNCTION("_CreateReactionNode", ret );
            return ret;
        }
    }
    else {
        TRACE_0("\tsetting reversible false");
        if( IS_FAILED( ( ret = SetReactionReversibleInReactionNode( reactionNode, FALSE ) ) ) ) {
            END_FUNCTION("_CreateReactionNode", ret );
            return ret;
        }
    }
                                     
    END_FUNCTION("_CreateReactionNode", SUCCESS );
    return ret;   
}



static RET_VAL _CreateKineticLaw( FRONT_END_PROCESSOR *frontend, REACTION *reactionNode, REACTION_LAW *reactionLaw, Model_t *model, Reaction_t *reaction ) {
    RET_VAL ret = SUCCESS;
    SBML_SYMTAB_MANAGER *manager = NULL;
    KineticLaw_t *source =NULL;
    ASTNode_t *node = NULL;
    ListOf_t *list = NULL;
    HASH_TABLE *table = NULL;
    KINETIC_LAW *law = NULL;
#ifdef DEBUG
    STRING *kineticLawString = NULL;
#endif        
    
    START_FUNCTION("_CreateKineticLaw");
               
    table = (HASH_TABLE*)frontend->_internal2;    
    
    if( ( manager = GetSymtabManagerInstance( frontend->record ) ) == NULL ) {
        return ErrorReport( FAILING, "_CreateKineticLaw", "error on getting symtab manager" ); 
    }
    source = Reaction_getKineticLaw( reaction );
    
    list = KineticLaw_getListOfParameters( source );
    if( IS_FAILED( ( ret = manager->SetLocal( manager, list ) ) ) ) {
        return ErrorReport( FAILING, "_CreateKineticLaw", "error on setting local" ); 
    }     
    if( IS_FAILED( ( ret = manager->SetLocalID( manager, (char *)Reaction_getId( reaction ) ) ) ) ) {
        return ErrorReport( FAILING, "_CreateKineticLaw", "error on setting local" ); 
    }     
        
    node = (ASTNode_t*)KineticLaw_getMath( source );
    if( ( law = _TransformKineticLaw( frontend, node, manager, table ) ) == NULL ) {
        return ErrorReport( FAILING, "_CreateKineticLaw", "failed to create kinetic law for %s", Reaction_getId( reaction ) );        
    }
#ifdef DEBUG
    kineticLawString = ToStringKineticLaw( law );
    printf( "\tcreate kinetic law: %s%s", GetCharArrayOfString( kineticLawString ), NEW_LINE );
    FreeString( &kineticLawString );
#endif             
     
    if( IS_FAILED( ( ret = SetKineticLawInReactionNode( reactionNode, law ) ) ) ) {
        END_FUNCTION("_CreateKineticLaw", ret );
        return ret;   
    }   

    if( IS_FAILED( ( ret = SetKineticLawInReactionLaw( reactionLaw, law ) ) ) ) {
        END_FUNCTION("_CreateKineticLaw", ret );
        return ret;   
    }   
    if( IS_FAILED( ( ret = manager->SetLocal( manager, NULL ) ) ) ) {
        return ErrorReport( FAILING, "_CreateKineticLaw", "error on setting local" ); 
    }     
                                            
    END_FUNCTION("_CreateKineticLaw", SUCCESS );
    return ret;   
}



static KINETIC_LAW *_TransformKineticLaw( FRONT_END_PROCESSOR *frontend, ASTNode_t *source, SBML_SYMTAB_MANAGER *manager, HASH_TABLE *table ) {
    KINETIC_LAW *law = NULL;
    
    START_FUNCTION("_TransformKineticLaw");

    if( source == NULL ) {
        TRACE_0("the source kinetic law is null");
        END_FUNCTION("_TransformKineticLaw", FAILING );
        return NULL;
    }
    
    if( ASTNode_isOperator( source ) ) {
        if ( ( ASTNode_getType( source ) == AST_MINUS ) && ( ASTNode_getNumChildren( source ) ) == 1 ) {
	  if( ( law = _TransformNegKineticLaw( frontend, source, manager, table ) ) == NULL ) {
            END_FUNCTION("_TransformKineticLaw", FAILING );
            return NULL;
	  }                 
        } else {
	  if ( ( ( ASTNode_getType( source ) != AST_PLUS) && ( ASTNode_getType( source ) != AST_TIMES) ) ||
	       ( ( ASTNode_getNumChildren( source ) ) == 2 ) ) {
	    if( ( law = _TransformOpKineticLaw( frontend, source, manager, table ) ) == NULL ) {
	      END_FUNCTION("_TransformKineticLaw", FAILING );
	      return NULL;
	    }
	  } else {
	    if( ( law = _TransformFunctionKineticLaw( frontend, source, manager, table ) ) == NULL ) {
	      END_FUNCTION("_TransformKineticLaw", FAILING );
	      return NULL;
	    }
	  }        
	}        
        END_FUNCTION("_TransformKineticLaw", SUCCESS );
        return law;
    }
    else if( ASTNode_isReal( source ) || ASTNode_isConstant( source ) || 
	     (ASTNode_getType( source ) == AST_NAME_AVOGADRO) ) {
        if( ( law = _TransformRealValueKineticLaw( frontend, source, manager, table ) ) == NULL ) {
            END_FUNCTION("_TransformKineticLaw", FAILING );
            return NULL;
        }                 
        END_FUNCTION("_TransformKineticLaw", SUCCESS );
        return law;
    } 
    else if( ASTNode_isName( source ) && (ASTNode_getType( source ) != AST_FUNCTION_DELAY)) {
        if( ( law = _TransformSymKineticLaw( frontend, source, manager, table ) ) == NULL ) {
            END_FUNCTION("_TransformKineticLaw", FAILING );
            return NULL;
        }                 
        END_FUNCTION("_TransformKineticLaw", SUCCESS );
        return law;
    }    
    else if( ASTNode_isInteger( source ) ) {
        if( ( law = _TransformIntValueKineticLaw( frontend, source, manager, table ) ) == NULL ) {
            END_FUNCTION("_TransformKineticLaw", FAILING );
            return NULL;
        }                 
        END_FUNCTION("_TransformKineticLaw", SUCCESS );
        return law;
    }    
    else if(  ASTNode_isFunction( source ) || ASTNode_isLogical( source ) || ASTNode_isRelational( source ) || (ASTNode_getType( source ) == AST_FUNCTION_DELAY)) {
        if( ( law = _TransformFunctionKineticLaw( frontend, source, manager, table ) ) == NULL ) {
            END_FUNCTION("_TransformKineticLaw", FAILING );
            return NULL;
        }                 
        END_FUNCTION("_TransformKineticLaw", SUCCESS );
        return law;
    } 
#if 0    
    else if(  ASTNode_getType( source ) == AST_FUNCTION_POWER ) {
        if( ( law = _TransformFunctionKineticLaw( frontend, source, manager, table ) ) == NULL ) {
            END_FUNCTION("_TransformKineticLaw", FAILING );
            return NULL;
        }                 
        END_FUNCTION("_TransformKineticLaw", SUCCESS );
        return law;
    } 
    else if(  ASTNode_isUnknown( source ) ) {
        if( ( law = _TransformFunctionKineticLaw( frontend, source, manager, table ) ) == NULL ) {
            END_FUNCTION("_TransformKineticLaw", FAILING );
            return NULL;
        }                 
        END_FUNCTION("_TransformKineticLaw", SUCCESS );
        return law;
    } 
#endif
        
    TRACE_1( "source %s has error", SBML_formulaToString( source ) );
    END_FUNCTION("_TransformKineticLaw", FAILING );
    return NULL;
}

static KINETIC_LAW *_TransformOpKineticLaw( FRONT_END_PROCESSOR *frontend, ASTNode_t *source, SBML_SYMTAB_MANAGER *manager, HASH_TABLE *table ) {
    int num = 0;
    KINETIC_LAW *law = NULL;
    KINETIC_LAW *left = NULL;
    KINETIC_LAW *right = NULL;
    ASTNodeType_t type = AST_UNKNOWN;
    ASTNode_t *rightNode = NULL;
    ASTNode_t *leftNode = NULL;
    
    START_FUNCTION("_TransformOpKineticLaw");
    
    num = ASTNode_getNumChildren( source );
    if( num != 2 ) {
        END_FUNCTION("_TransformOpKineticLaw", FAILING );
        return NULL;
    }
    
    leftNode = ASTNode_getLeftChild( source );
    if( ( left = _TransformKineticLaw( frontend, leftNode, manager, table ) ) == NULL ) {
        END_FUNCTION("_TransformOpKineticLaw", FAILING );
        return NULL;
    } 
                    
    rightNode = ASTNode_getRightChild( source );
    if( ( right = _TransformKineticLaw( frontend, rightNode, manager, table ) ) == NULL ) {
        FreeKineticLaw( &left );
        END_FUNCTION("_TransformOpKineticLaw", FAILING );
        return NULL;
    } 
    
    type = ASTNode_getType( source );    
    switch( type ) {
        case AST_PLUS:                
            if( ( law = CreateOpKineticLaw( KINETIC_LAW_OP_PLUS, left, right ) ) == NULL ) {
                FreeKineticLaw( &right );
                FreeKineticLaw( &left );
                END_FUNCTION("_TransformOpKineticLaw", FAILING );
                return NULL;
            }
            END_FUNCTION("_TransformOpKineticLaw", SUCCESS );
        return law;

        case AST_MINUS:
            if( ( law = CreateOpKineticLaw( KINETIC_LAW_OP_MINUS, left, right ) ) == NULL ) {
                FreeKineticLaw( &right );
                FreeKineticLaw( &left );
                END_FUNCTION("_TransformOpKineticLaw", FAILING );
                return NULL;
            }
            END_FUNCTION("_TransformOpKineticLaw", SUCCESS );
        return law;

        case AST_TIMES:
            if( ( law = CreateOpKineticLaw( KINETIC_LAW_OP_TIMES, left, right ) ) == NULL ) {
                FreeKineticLaw( &right );
                FreeKineticLaw( &left );
                END_FUNCTION("_TransformOpKineticLaw", FAILING );
                return NULL;
            }
            END_FUNCTION("_TransformOpKineticLaw", SUCCESS );
        return law;

        case AST_DIVIDE:
            if( ( law = CreateOpKineticLaw( KINETIC_LAW_OP_DIVIDE, left, right ) ) == NULL ) {
                FreeKineticLaw( &right );
                FreeKineticLaw( &left );
                END_FUNCTION("_TransformOpKineticLaw", FAILING );
                return NULL;
            }
            END_FUNCTION("_TransformOpKineticLaw", SUCCESS );
        return law;

        case AST_POWER:
            if( ( law = CreateOpKineticLaw( KINETIC_LAW_OP_POW, left, right ) ) == NULL ) {
                FreeKineticLaw( &right );
                FreeKineticLaw( &left );
                END_FUNCTION("_TransformOpKineticLaw", FAILING );
                return NULL;
            }
            END_FUNCTION("_TransformOpKineticLaw", SUCCESS );
        return law;

        default:
            FreeKineticLaw( &right );
            FreeKineticLaw( &left );
            END_FUNCTION("_TransformOpKineticLaw", FAILING );
        return NULL;
    }

    END_FUNCTION("_TransformOpKineticLaw", FAILING );
    return NULL;
}

static KINETIC_LAW *_TransformNegKineticLaw( FRONT_END_PROCESSOR *frontend, ASTNode_t *source, SBML_SYMTAB_MANAGER *manager, HASH_TABLE *table ) {
    KINETIC_LAW *law = NULL;
    KINETIC_LAW *child = NULL;
    ASTNodeType_t type = AST_UNKNOWN;
    ASTNode_t *childNode = NULL;
    
    START_FUNCTION("_TransformNegKineticLaw");

    childNode = ASTNode_getChild( source, 0 );
    if( ( child = _TransformKineticLaw( frontend, childNode, manager, table ) ) == NULL ) {
      END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
      return NULL;
    }  
    if( ( law = CreateUnaryOpKineticLaw( KINETIC_LAW_UNARY_OP_NEG, child ) ) == NULL ) {
      END_FUNCTION("_TransformNegKineticLaw", FAILING );
      return NULL;
    }
    END_FUNCTION("_TransformNegKineticLaw", SUCCESS );
    return law;
}

static KINETIC_LAW *_TransformFunctionKineticLaw( FRONT_END_PROCESSOR *frontend, ASTNode_t *source, SBML_SYMTAB_MANAGER *manager, HASH_TABLE *table ) {
    int i = 0;
    int num = 0;
    KINETIC_LAW *law = NULL;
    KINETIC_LAW **children = NULL;
    LINKED_LIST *childrenLL = NULL;
    ASTNodeType_t type = AST_UNKNOWN;
    ASTNode_t *childNode = NULL;
    char * funcId = NULL;
    FUNCTION_MANAGER *functionManager = NULL;
    FUNCTION_DEFINITION *functionDef = NULL; 
    REB2SAC_SYMBOL *symbol = NULL;
    REB2SAC_SYMTAB *symtab = NULL;
    
    START_FUNCTION("_TransformFunctionKineticLaw");
    
    num = ASTNode_getNumChildren( source );
    if (num > 0) {
      if( ( children = (KINETIC_LAW**)CALLOC( num, sizeof(KINETIC_LAW*) ) ) == NULL ) {
        END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
        return NULL;
      }
    } else {
      children = NULL;
    }
    
    for( i = 0; i < num; i++ ) {
        childNode = ASTNode_getChild( source, i );
        if( ( children[i] = _TransformKineticLaw( frontend, childNode, manager, table ) ) == NULL ) {
            END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
            return NULL;
        }  
    }
     
    type = ASTNode_getType( source );  
    switch( type ) {
        case AST_FUNCTION:
	  funcId = (char*)ASTNode_getName( source );
	  if (strcmp(funcId,"uniform")==0) {
	    if( num != 2 ) {
	      END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	      return NULL;
	    }
	    if( ( law = CreateOpKineticLaw( KINETIC_LAW_OP_UNIFORM, children[0], children[1] ) ) == NULL ) {
	      END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	      return NULL;
	    }
	    FREE( children );
	    END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	    return law;
	  } else if (strcmp(funcId,"normal")==0) {
	    if( num != 2 ) {
	      END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	      return NULL;
	    }
	    if( ( law = CreateOpKineticLaw( KINETIC_LAW_OP_NORMAL, children[0], children[1] ) ) == NULL ) {
	      END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	      return NULL;
	    }
	    FREE( children );
	    END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	    return law;
	  } else if (strcmp(funcId,"binomial")==0) {
	    if( num != 2 ) {
	      END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	      return NULL;
	    }
	    if( ( law = CreateOpKineticLaw( KINETIC_LAW_OP_BINOMIAL, children[0], children[1] ) ) == NULL ) {
	      END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	      return NULL;
	    }
	    FREE( children );
	    END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	    return law;
	  } else if (strcmp(funcId,"lognormal")==0) {
	    if( num != 2 ) {
	      END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	      return NULL;
	    }
	    if( ( law = CreateOpKineticLaw( KINETIC_LAW_OP_LOGNORMAL, children[0], children[1] ) ) == NULL ) {
	      END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	      return NULL;
	    }
	    FREE( children );
	    END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	    return law;
	  } else if (strcmp(funcId,"chisq")==0) {
	    if( num != 1 ) {
	      END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	      return NULL;
	    }
	    if( ( law = CreateUnaryOpKineticLaw( KINETIC_LAW_UNARY_OP_CHISQ, children[0] ) ) == NULL ) {
	      END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	      return NULL;
	    }
	    FREE( children );
	    END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	    return law;
	  } else if (strcmp(funcId,"exponential")==0) {
	    if( num != 1 ) {
	      END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	      return NULL;
	    }
	    if( ( law = CreateUnaryOpKineticLaw( KINETIC_LAW_UNARY_OP_EXPRAND, children[0] ) ) == NULL ) {
	      END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	      return NULL;
	    }
	    FREE( children );
	    END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	    return law;
	  } else if (strcmp(funcId,"poisson")==0) {
	    if( num != 1 ) {
	      END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	      return NULL;
	    }
	    if( ( law = CreateUnaryOpKineticLaw( KINETIC_LAW_UNARY_OP_POISSON, children[0] ) ) == NULL ) {
	      END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	      return NULL;
	    }
	    FREE( children );
	    END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	    return law;
	  } else if (strcmp(funcId,"laplace")==0) {
	    if( num != 1 ) {
	      END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	      return NULL;
	    }
	    if( ( law = CreateUnaryOpKineticLaw( KINETIC_LAW_UNARY_OP_LAPLACE, children[0] ) ) == NULL ) {
	      END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	      return NULL;
	    }
	    FREE( children );
	    END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	    return law;
	  } else if (strcmp(funcId,"cauchy")==0) {
	    if( num != 1 ) {
	      END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	      return NULL;
	    }
	    if( ( law = CreateUnaryOpKineticLaw( KINETIC_LAW_UNARY_OP_CAUCHY, children[0] ) ) == NULL ) {
	      END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	      return NULL;
	    }
	    FREE( children );
	    END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	    return law;
	  } else if (strcmp(funcId,"rayleigh")==0) {
	    if( num != 1 ) {
	      END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	      return NULL;
	    }
	    if( ( law = CreateUnaryOpKineticLaw( KINETIC_LAW_UNARY_OP_RAYLEIGH, children[0] ) ) == NULL ) {
	      END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	      return NULL;
	    }
	    FREE( children );
	    END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	    return law;
	  } else if (strcmp(funcId,"bernoulli")==0) {
	    if( num != 1 ) {
	      END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	      return NULL;
	    }
	    if( ( law = CreateUnaryOpKineticLaw( KINETIC_LAW_UNARY_OP_BERNOULLI, children[0] ) ) == NULL ) {
	      END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	      return NULL;
	    }
	    FREE( children );
	    END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	    return law;
	  } else if (strcmp(funcId,"gamma")==0) {
	    if( num != 2 ) {
	      END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	      return NULL;
	    }
	    if( ( law = CreateOpKineticLaw( KINETIC_LAW_OP_GAMMA, children[0], children[1] ) ) == NULL ) {
	      END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	      return NULL;
	    }
	    FREE( children );
	    END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	    return law;
	  } else if (strcmp(funcId,"rate")==0) {
	    if( num != 1 ) {
	      END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	      return NULL;
	    }
	    if( ( law = CreateUnaryOpKineticLaw( KINETIC_LAW_UNARY_OP_RATE, children[0] ) ) == NULL ) {
	      END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	      return NULL;
	    }
	    FREE( children );
	    END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	    return law;
	  } else if (strcmp(funcId,"BITAND")==0) {
	    if( num != 2 ) {
	      END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	      return NULL;
	    }
	    if( ( law = CreateOpKineticLaw( KINETIC_LAW_OP_BITWISE_AND, children[0], children[1] ) ) == NULL ) {
	      END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	      return NULL;
	    }
	    FREE( children );
	    END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	    return law;
	  } else if (strcmp(funcId,"BITOR")==0) {
	    if( num != 2 ) {
	      END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	      return NULL;
	    }
	    if( ( law = CreateOpKineticLaw( KINETIC_LAW_OP_BITWISE_OR, children[0], children[1] ) ) == NULL ) {
	      END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	      return NULL;
	    }
	    FREE( children );
	    END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	    return law;
	  } else if (strcmp(funcId,"BITXOR")==0) {
	    if( num != 2 ) {
	      END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	      return NULL;
	    }
	    if( ( law = CreateOpKineticLaw( KINETIC_LAW_OP_BITWISE_XOR, children[0], children[1] ) ) == NULL ) {
	      END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	      return NULL;
	    }
	    FREE( children );
	    END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	    return law;
	  } else if (strcmp(funcId,"BIT")==0) {
	    if( num != 2 ) {
	      END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	      return NULL;
	    }
	    if( ( law = CreateOpKineticLaw( KINETIC_LAW_OP_BIT, children[0], children[1] ) ) == NULL ) {
	      END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	      return NULL;
	    }
	    FREE( children );
	    END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	    return law;
	  } else if (strcmp(funcId,"MOD")==0) {
	    if( num != 2 ) {
	      END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	      return NULL;
	    }
	    if( ( law = CreateOpKineticLaw( KINETIC_LAW_OP_MOD, children[0], children[1] ) ) == NULL ) {
	      END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	      return NULL;
	    }
	    FREE( children );
	    END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	    return law;
	  } else if (strcmp(funcId,"BITNOT")==0) {
	    if( num != 1 ) {
	      END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	      return NULL;
	    }
	    if( ( law = CreateUnaryOpKineticLaw( KINETIC_LAW_UNARY_OP_BITWISE_NOT, children[0] ) ) == NULL ) {
	      END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	      return NULL;
	    }
	    FREE( children );
	    END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	    return law;
	  } else if (strcmp(funcId,"INT")==0) {
	    if( num != 1 ) {
	      END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	      return NULL;
	    }
	    if( ( law = CreateUnaryOpKineticLaw( KINETIC_LAW_UNARY_OP_INT, children[0] ) ) == NULL ) {
	      END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	      return NULL;
	    }
	    FREE( children );
	    END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	    return law;
	  }
	  functionManager = GetFunctionManagerInstance( frontend->record );
	  if( ( functionDef = functionManager->LookupFunctionDefinition( functionManager, funcId ) ) == NULL ) {
            return NULL; //ErrorReport( FAILING, "_HandleFunctionDefinition", "function def %s is not declared", funcId );
	  }
	  if( ( law = CreateFunctionKineticLaw( funcId, functionDef->function, functionDef->arguments, children, num ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  FREE( children );
	  END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	  return law;
        case AST_FUNCTION_PIECEWISE:
	  childrenLL = CreateLinkedList();
	  for (i = 0; i < num; i++) {
	    AddElementInLinkedList( (CADDR_T)children[i], childrenLL );
	  }
	  if( ( law = CreatePWKineticLaw( KINETIC_LAW_OP_PW, childrenLL ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  FREE( children );
	  END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	  return law;
        case AST_FUNCTION_DELAY:                    
	  if( num != 2 ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  symtab = (REB2SAC_SYMTAB*)(frontend->_internal3);
	  if( ( symbol = symtab->Lookup( symtab, "t" ) ) == NULL ) {
            END_FUNCTION("_TransformSymKineticLaw", FAILING );
            return NULL;
	  }
	  if( ( law = CreateDelayKineticLaw( KINETIC_LAW_OP_DELAY, children[0], children[1], symbol ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  FREE( children );
	  END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	  return law;
        case AST_FUNCTION_POWER:
	  if( num != 2 ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  if( ( law = CreateOpKineticLaw( KINETIC_LAW_OP_POW, children[0], children[1] ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  FREE( children );
	  END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	  return law;
        case AST_FUNCTION_ROOT:                    
	  if( num != 2 ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  if( ( law = CreateOpKineticLaw( KINETIC_LAW_OP_ROOT, children[0], children[1] ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  FREE( children );
	  END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	  return law;
        case AST_PLUS:
	  childrenLL = CreateLinkedList();
	  for (i = 0; i < num; i++) {
	    AddElementInLinkedList( (CADDR_T)children[i], childrenLL );
	  }
	  if( ( law = CreatePWKineticLaw( KINETIC_LAW_OP_PLUS, childrenLL ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  FREE( children );
	  END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	  return law;
        case AST_TIMES:
	  childrenLL = CreateLinkedList();
	  for (i = 0; i < num; i++) {
	    AddElementInLinkedList( (CADDR_T)children[i], childrenLL );
	  }
	  if( ( law = CreatePWKineticLaw( KINETIC_LAW_OP_TIMES, childrenLL ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  FREE( children );
	  END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	  return law;
        case AST_LOGICAL_AND:
	  childrenLL = CreateLinkedList();
	  for (i = 0; i < num; i++) {
	    AddElementInLinkedList( (CADDR_T)children[i], childrenLL );
	  }
	  if( ( law = CreatePWKineticLaw( KINETIC_LAW_OP_AND, childrenLL ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  /*if( num != 2 ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  if( ( law = CreateOpKineticLaw( KINETIC_LAW_OP_AND, children[0], children[1] ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	    } */
	  FREE( children );
	  END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	  return law;
        case AST_LOGICAL_OR:
	  childrenLL = CreateLinkedList();
	  for (i = 0; i < num; i++) {
	    AddElementInLinkedList( (CADDR_T)children[i], childrenLL );
	  }
	  if( ( law = CreatePWKineticLaw( KINETIC_LAW_OP_OR, childrenLL ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  /*if( num != 2 ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  if( ( law = CreateOpKineticLaw( KINETIC_LAW_OP_OR, children[0], children[1] ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	    }*/
	  FREE( children );
	  END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	  return law;
        case AST_LOGICAL_XOR:
	  childrenLL = CreateLinkedList();
	  for (i = 0; i < num; i++) {
	    AddElementInLinkedList( (CADDR_T)children[i], childrenLL );
	  }
	  if( ( law = CreatePWKineticLaw( KINETIC_LAW_OP_XOR, childrenLL ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  /*if( num != 2 ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  if( ( law = CreateOpKineticLaw( KINETIC_LAW_OP_XOR, children[0], children[1] ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	    }*/
	  FREE( children );
	  END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	  return law;
        case AST_LOGICAL_NOT:
	  if( num != 1 ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  if( ( law = CreateUnaryOpKineticLaw( KINETIC_LAW_UNARY_OP_NOT, children[0] ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  FREE( children );
	  END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	  return law;
        case AST_FUNCTION_ABS:
	  if( num != 1 ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  if( ( law = CreateUnaryOpKineticLaw( KINETIC_LAW_UNARY_OP_ABS, children[0] ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  FREE( children );
	  END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	  return law;
        case AST_FUNCTION_COT:
	  if( num != 1 ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  if( ( law = CreateUnaryOpKineticLaw( KINETIC_LAW_UNARY_OP_COT, children[0] ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  FREE( children );
	  END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	  return law;
        case AST_FUNCTION_COTH:
	  if( num != 1 ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  if( ( law = CreateUnaryOpKineticLaw( KINETIC_LAW_UNARY_OP_COTH, children[0] ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  FREE( children );
	  END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	  return law;
        case AST_FUNCTION_CSC:
	  if( num != 1 ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  if( ( law = CreateUnaryOpKineticLaw( KINETIC_LAW_UNARY_OP_CSC, children[0] ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  FREE( children );
	  END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	  return law;
        case AST_FUNCTION_CSCH:
	  if( num != 1 ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  if( ( law = CreateUnaryOpKineticLaw( KINETIC_LAW_UNARY_OP_CSCH, children[0] ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  FREE( children );
	  END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	  return law;
        case AST_FUNCTION_SEC:
	  if( num != 1 ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  if( ( law = CreateUnaryOpKineticLaw( KINETIC_LAW_UNARY_OP_SEC, children[0] ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  FREE( children );
	  END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	  return law;
        case AST_FUNCTION_SECH:
	  if( num != 1 ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  if( ( law = CreateUnaryOpKineticLaw( KINETIC_LAW_UNARY_OP_SECH, children[0] ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  FREE( children );
	  END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	  return law;
        case AST_FUNCTION_COS:
	  if( num != 1 ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  if( ( law = CreateUnaryOpKineticLaw( KINETIC_LAW_UNARY_OP_COS, children[0] ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  FREE( children );
	  END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	  return law;
        case AST_FUNCTION_COSH:
	  if( num != 1 ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  if( ( law = CreateUnaryOpKineticLaw( KINETIC_LAW_UNARY_OP_COSH, children[0] ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  FREE( children );
	  END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	  return law;
        case AST_FUNCTION_SIN:
	  if( num != 1 ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  if( ( law = CreateUnaryOpKineticLaw( KINETIC_LAW_UNARY_OP_SIN, children[0] ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  FREE( children );
	  END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	  return law;
        case AST_FUNCTION_SINH:
	  if( num != 1 ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  if( ( law = CreateUnaryOpKineticLaw( KINETIC_LAW_UNARY_OP_SINH, children[0] ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  FREE( children );
	  END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	  return law;
        case AST_FUNCTION_TAN:
	  if( num != 1 ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  if( ( law = CreateUnaryOpKineticLaw( KINETIC_LAW_UNARY_OP_TAN, children[0] ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  FREE( children );
	  END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	  return law;
        case AST_FUNCTION_TANH:
	  if( num != 1 ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  if( ( law = CreateUnaryOpKineticLaw( KINETIC_LAW_UNARY_OP_TANH, children[0] ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  FREE( children );
	  END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	  return law;
        case AST_FUNCTION_ARCCOT:
	  if( num != 1 ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  if( ( law = CreateUnaryOpKineticLaw( KINETIC_LAW_UNARY_OP_ARCCOT, children[0] ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  FREE( children );
	  END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	  return law;
        case AST_FUNCTION_ARCCOTH:
	  if( num != 1 ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  if( ( law = CreateUnaryOpKineticLaw( KINETIC_LAW_UNARY_OP_ARCCOTH, children[0] ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  FREE( children );
	  END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	  return law;
        case AST_FUNCTION_ARCCSC:
	  if( num != 1 ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  if( ( law = CreateUnaryOpKineticLaw( KINETIC_LAW_UNARY_OP_ARCCSC, children[0] ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  FREE( children );
	  END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	  return law;
        case AST_FUNCTION_ARCCSCH:
	  if( num != 1 ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  if( ( law = CreateUnaryOpKineticLaw( KINETIC_LAW_UNARY_OP_ARCCSCH, children[0] ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  FREE( children );
	  END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	  return law;
        case AST_FUNCTION_ARCSEC:
	  if( num != 1 ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  if( ( law = CreateUnaryOpKineticLaw( KINETIC_LAW_UNARY_OP_ARCSEC, children[0] ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  FREE( children );
	  END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	  return law;
        case AST_FUNCTION_ARCSECH:
	  if( num != 1 ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  if( ( law = CreateUnaryOpKineticLaw( KINETIC_LAW_UNARY_OP_ARCSECH, children[0] ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  FREE( children );
	  END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	  return law;
        case AST_FUNCTION_ARCCOS:
	  if( num != 1 ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  if( ( law = CreateUnaryOpKineticLaw( KINETIC_LAW_UNARY_OP_ARCCOS, children[0] ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  FREE( children );
	  END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	  return law;
        case AST_FUNCTION_ARCCOSH:
	  if( num != 1 ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  if( ( law = CreateUnaryOpKineticLaw( KINETIC_LAW_UNARY_OP_ARCCOSH, children[0] ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  FREE( children );
	  END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	  return law;
        case AST_FUNCTION_ARCSIN:
	  if( num != 1 ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  if( ( law = CreateUnaryOpKineticLaw( KINETIC_LAW_UNARY_OP_ARCSIN, children[0] ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  FREE( children );
	  END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	  return law;
        case AST_FUNCTION_ARCSINH:
	  if( num != 1 ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  if( ( law = CreateUnaryOpKineticLaw( KINETIC_LAW_UNARY_OP_ARCSINH, children[0] ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  FREE( children );
	  END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	  return law;
        case AST_FUNCTION_ARCTAN:
	  if( num != 1 ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  if( ( law = CreateUnaryOpKineticLaw( KINETIC_LAW_UNARY_OP_ARCTAN, children[0] ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  FREE( children );
	  END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	  return law;
        case AST_FUNCTION_ARCTANH:
	  if( num != 1 ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  if( ( law = CreateUnaryOpKineticLaw( KINETIC_LAW_UNARY_OP_ARCTANH, children[0] ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  FREE( children );
	  END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	  return law;
        case AST_FUNCTION_CEILING:
	  if( num != 1 ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  if( ( law = CreateUnaryOpKineticLaw( KINETIC_LAW_UNARY_OP_CEILING, children[0] ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  FREE( children );
	  END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	  return law;
        case AST_FUNCTION_EXP:
	  if( num != 1 ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  if( ( law = CreateUnaryOpKineticLaw( KINETIC_LAW_UNARY_OP_EXP, children[0] ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  FREE( children );
	  END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	  return law;
        case AST_FUNCTION_FACTORIAL:
	  if( num != 1 ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  if( ( law = CreateUnaryOpKineticLaw( KINETIC_LAW_UNARY_OP_FACTORIAL, children[0] ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  FREE( children );
	  END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	  return law;
        case AST_FUNCTION_FLOOR:
	  if( num != 1 ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  if( ( law = CreateUnaryOpKineticLaw( KINETIC_LAW_UNARY_OP_FLOOR, children[0] ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  FREE( children );
	  END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	  return law;
        case AST_FUNCTION_LN:
	  if( num != 1 ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  if( ( law = CreateUnaryOpKineticLaw( KINETIC_LAW_UNARY_OP_LN, children[0] ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  FREE( children );
	  END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	  return law;
        case AST_FUNCTION_LOG:
	  if( num != 2 ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  if( ( law = CreateOpKineticLaw( KINETIC_LAW_OP_LOG, children[0], children[1] ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  FREE( children );
	  END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	  return law;
        case AST_RELATIONAL_EQ:
	  if( num != 2 ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  if( ( law = CreateOpKineticLaw( KINETIC_LAW_OP_EQ, children[0], children[1] ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  FREE( children );
	  END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	  return law;
        case AST_RELATIONAL_NEQ:
	  if( num != 2 ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  if( ( law = CreateOpKineticLaw( KINETIC_LAW_OP_NEQ, children[0], children[1] ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  FREE( children );
	  END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	  return law;
        case AST_RELATIONAL_GEQ:
	  if( num != 2 ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  if( ( law = CreateOpKineticLaw( KINETIC_LAW_OP_GEQ, children[0], children[1] ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  FREE( children );
	  END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	  return law;
        case AST_RELATIONAL_GT:
	  if( num != 2 ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  if( ( law = CreateOpKineticLaw( KINETIC_LAW_OP_GT, children[0], children[1] ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  FREE( children );
	  END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	  return law;
        case AST_RELATIONAL_LEQ:
	  if( num != 2 ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  if( ( law = CreateOpKineticLaw( KINETIC_LAW_OP_LEQ, children[0], children[1] ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  FREE( children );
	  END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	  return law;
        case AST_RELATIONAL_LT:
	  if( num != 2 ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  if( ( law = CreateOpKineticLaw( KINETIC_LAW_OP_LT, children[0], children[1] ) ) == NULL ) {
	    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	    return NULL;
	  }
	  FREE( children );
	  END_FUNCTION("_TransformFunctionKineticLaw", SUCCESS );
	  return law;
        default:
	  END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
	  return NULL;
    }

    END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
    return NULL;
}


static KINETIC_LAW *_TransformSymKineticLaw( FRONT_END_PROCESSOR *frontend, ASTNode_t *source, SBML_SYMTAB_MANAGER *manager, HASH_TABLE *table ) {
    char *sym = NULL;
    double realValue = 0.0;
    SPECIES *species = NULL;
    KINETIC_LAW *law = NULL;
    REB2SAC_SYMBOL *symbol = NULL;
    REB2SAC_SYMTAB *symtab = NULL;
    COMPARTMENT_MANAGER *compartmentManager = NULL;
    COMPARTMENT *compartment = NULL;
    REACTION_LAW_MANAGER *reactionLawManager = NULL;
    REACTION_LAW *reactionLaw = NULL;
    //STRING *kineticLawString = NULL;
    UNIT_MANAGER *unitManager = NULL;
    UNIT_DEFINITION *units = NULL;
    char *unitsID;
    char *localID;
    char buf[256];

    START_FUNCTION("_TransformSymKineticLaw");

    if (ASTNode_getType( source ) == AST_NAME_TIME) {
      ASTNode_setName( source, "t" ); 
      if (workingOnFunctions) {
        symtab = (REB2SAC_SYMTAB*)(frontend->_internal3);
        if( ( symbol = symtab->Lookup( symtab, "t" ) ) == NULL ) {
            END_FUNCTION("_TransformSymKineticLaw", FAILING );
            return NULL;
        }
        if( ( law = CreateSymbolKineticLaw( symbol ) ) == NULL ) {
            END_FUNCTION("_TransformSymKineticLaw", FAILING );
            return NULL;
        }
        TRACE_2( "sym %s = %f", GetCharArrayOfString( GetSymbolID( symbol ) ), realValue );
        END_FUNCTION("_TransformSymKineticLaw", SUCCESS );
        return law;
      }
    }    
    sym = (char*)ASTNode_getName( source );
    if (workingOnFunctions) {
      if( ( law = CreateFunctionSymbolKineticLaw( sym ) ) == NULL ) {
	END_FUNCTION("_TransformSymKineticLaw", FAILING );
	return NULL;
      }
      return law;
    }
    if( manager->LookupLocalValue( manager, sym, &realValue, &unitsID ) ) {
        symtab = (REB2SAC_SYMTAB*)(frontend->_internal3);
	manager->LookupLocalID( manager, &localID );
	sprintf(buf,"%s_%s",localID,sym);
        if( ( symbol = symtab->AddRealValueSymbol( symtab, buf, realValue, TRUE ) ) == NULL ) {
            END_FUNCTION("_TransformSymKineticLaw", FAILING );
            return NULL;
        }
	if( unitsID != NULL ) {
	  if( ( unitManager = GetUnitManagerInstance( frontend->record ) ) == NULL ) {
	    return NULL;
	  }
	  if( ( units = unitManager->LookupUnitDefinition( unitManager, unitsID ) ) == NULL ) {
            return NULL;
	  }
	  SetUnitsInSymbol( symbol, units  );
	} 
        if( ( law = CreateSymbolKineticLaw( symbol ) ) == NULL ) {
            END_FUNCTION("_TransformSymKineticLaw", FAILING );
            return NULL;
        }
        TRACE_2( "sym %s = %f", GetCharArrayOfString( GetSymbolID( symbol ) ), realValue );
        END_FUNCTION("_TransformSymKineticLaw", SUCCESS );
        return law;
    }
    else if( strcmp(sym,"t")==0 || strcmp(sym,"time")==0 || 
	     manager->LookupGlobalValue( manager, sym, &realValue ) ) {
        symtab = (REB2SAC_SYMTAB*)(frontend->_internal3);
        if( ( symbol = symtab->Lookup( symtab, sym ) ) == NULL ) {
            END_FUNCTION("_TransformSymKineticLaw", FAILING );
            return NULL;
        }
        if( ( law = CreateSymbolKineticLaw( symbol ) ) == NULL ) {
            END_FUNCTION("_TransformSymKineticLaw", FAILING );
            return NULL;
        }
        TRACE_2( "sym %s = %f", GetCharArrayOfString( GetSymbolID( symbol ) ), realValue );
        END_FUNCTION("_TransformSymKineticLaw", SUCCESS );
        return law;
    }        

    if( ( compartmentManager = GetCompartmentManagerInstance( frontend->record ) ) == NULL ) {
        END_FUNCTION("_TransformSymKineticLaw", FAILING );
        return NULL;
    }
    if (compartment = compartmentManager->LookupCompartment( compartmentManager, sym )) {
      TRACE_1( "sym %s is a compartment", GetCharArrayOfString( GetCompartmentID( compartment) ) );
      if( ( law = CreateCompartmentKineticLaw( compartment ) ) == NULL ) {
        END_FUNCTION("_TransformSymKineticLaw", FAILING );
        return NULL;
      }
      END_FUNCTION("_TransformSymKineticLaw", SUCCESS );
      return law;
    }

    if( ( species = (SPECIES*)GetValueFromHashTable( sym, strlen( sym ), table ) ) ) {
      TRACE_1( "sym %s is a species", sym );
      if( ( law = CreateSpeciesKineticLaw( species ) ) == NULL ) {
        END_FUNCTION("_TransformSymKineticLaw", FAILING );
        return NULL;
      }
      END_FUNCTION("_TransformSymKineticLaw", SUCCESS );
      return law;
    }

    if( ( reactionLawManager = GetReactionLawManagerInstance( frontend->record ) ) == NULL ) {
        END_FUNCTION("_TransformSymKineticLaw", FAILING );
        return NULL;
    }
    if (reactionLaw = reactionLawManager->LookupReactionLaw( reactionLawManager, sym )) {
      TRACE_1( "sym %s is a reaction", GetCharArrayOfString( GetReactionLawID( reactionLaw) ) );
      //printf( "sym %s is a reaction\n", GetCharArrayOfString( GetReactionLawID( reactionLaw) ) );
      law = CloneKineticLaw( GetKineticLawInReactionLaw( reactionLaw ) );
      END_FUNCTION("_TransformSymKineticLaw", SUCCESS );
      //kineticLawString = ToStringKineticLaw( law );
      //printf( "\tusing kinetic law: %s%s", GetCharArrayOfString( kineticLawString ), NEW_LINE );
      //FreeString( &kineticLawString );
      return law;
    }
    /* This is one last gasp to deal with reactant/product ids */
    symtab = (REB2SAC_SYMTAB*)(frontend->_internal3);
    if( ( symbol = symtab->Lookup( symtab, sym ) ) != NULL ) {
      if( ( law = CreateSymbolKineticLaw( symbol ) ) == NULL ) {
	END_FUNCTION("_TransformSymKineticLaw", FAILING );
	return NULL;
      }
      TRACE_2( "sym %s = %f", GetCharArrayOfString( GetSymbolID( symbol ) ), realValue );
      END_FUNCTION("_TransformSymKineticLaw", SUCCESS );
      return law;
    }        
    printf("cannot find symbol %s\n",sym);
    
    END_FUNCTION("_TransformSymKineticLaw", FAILING );
    return NULL;
}

static KINETIC_LAW *_TransformIntValueKineticLaw( FRONT_END_PROCESSOR *frontend, ASTNode_t *source, SBML_SYMTAB_MANAGER *manager, HASH_TABLE *table ) {
    long intValue = 0L;
    KINETIC_LAW *law = NULL;
    
    START_FUNCTION("_TransformIntValueKineticLaw");

    intValue = ASTNode_getInteger( source );
    if( ( law = CreateIntValueKineticLaw( intValue ) ) == NULL ) {
        END_FUNCTION("_TransformIntValueKineticLaw", FAILING );
        return NULL;
    }
    END_FUNCTION("_TransformIntValueKineticLaw", SUCCESS );
    return law;
}

static KINETIC_LAW *_TransformRealValueKineticLaw( FRONT_END_PROCESSOR *frontend, ASTNode_t *source, SBML_SYMTAB_MANAGER *manager, HASH_TABLE *table ) {
    double realValue = 0L;
    KINETIC_LAW *law = NULL;
    
    START_FUNCTION("_TransformRealValueKineticLaw");
    
    if (ASTNode_isReal( source )) {
      realValue = ASTNode_getReal( source );
    } else if (ASTNode_getType( source ) == AST_NAME_AVOGADRO) {
      realValue = 6.02214179*pow(10,23);
    } else if (ASTNode_getType( source ) == AST_CONSTANT_PI) {
      realValue = 4.*atan(1.);
    } else if (ASTNode_getType( source ) == AST_CONSTANT_E) {
      realValue = exp(1);
    } else if (ASTNode_getType( source ) == AST_CONSTANT_TRUE) {
      realValue = 1.0;
    } else if (ASTNode_getType( source ) == AST_CONSTANT_FALSE) {
      realValue = 0.0;
    } else {
      END_FUNCTION("_TransformRealValueKineticLaw", FAILING );
      return NULL;
    }
    if( ( law = CreateRealValueKineticLaw( realValue ) ) == NULL ) {
        END_FUNCTION("_TransformRealValueKineticLaw", FAILING );
        return NULL;
    }
    END_FUNCTION("_TransformRealValueKineticLaw", SUCCESS );
    return law;
}


static RET_VAL _ResolveNodeLinks( FRONT_END_PROCESSOR *frontend, IR *ir, REACTION *reactionNode, 
				  Reaction_t *reaction, Model_t *model ) {
    RET_VAL ret = SUCCESS;
    UINT i = 0;
    UINT num = 0;
    UINT reactantsNum = 0;
    UINT productsNum = 0;
    UINT modifierNum = 0;
    double stoichiometry = 0;
    char *species = NULL;    
    ListOf_t *reactants = NULL;
    ListOf_t *modifiers = NULL;
    ListOf_t *products = NULL;
    SpeciesReference_t *speciesRef = NULL;
    SpeciesReference_t *modifierRef = NULL;        
    //    ModifierSpeciesReference_t *modifierRef = NULL;        
    SPECIES *speciesNode = NULL;
    HASH_TABLE *table = NULL;
    ASTNode_t *node;
    KINETIC_LAW *law;
    SBML_SYMTAB_MANAGER *manager = NULL;
    Species_t *Species = NULL;
    char* convFactor = NULL;
    Parameter_t *param = NULL;
    char* speciesRefId = NULL;
    REB2SAC_SYMTAB *symtab = (REB2SAC_SYMTAB*)frontend->_internal3;
    REB2SAC_SYMBOL *SpeciesRef = NULL;
    BOOL constant;

    START_FUNCTION("_ResolveNodeLinks");
        
    table = (HASH_TABLE*)frontend->_internal2;    
    if( ( manager = GetSymtabManagerInstance( frontend->record ) ) == NULL ) {
      return ErrorReport( FAILING, "_ResolveNodeLinks", "error on getting symtab manager" ); 
    }
    reactants = Reaction_getListOfReactants( reaction );
    num = Reaction_getNumReactants( reaction );
    for( i = 0; i < num; i++ ) {
        speciesRef = (SpeciesReference_t*)ListOf_get( reactants, i );
        species = (char*)SpeciesReference_getSpecies( speciesRef );
	speciesRefId = NULL;
	SpeciesRef = NULL;
	constant = FALSE;
	if (SpeciesReference_isSetStoichiometryMath( speciesRef ) ) {
	  StoichiometryMath_t *sm = (StoichiometryMath_t *)SpeciesReference_getStoichiometryMath( speciesRef );
	  node = (ASTNode_t *)StoichiometryMath_getMath( sm );
	  if( ( law = _TransformKineticLaw( frontend, node, manager, table ) ) == NULL ) {
	    return ErrorReport( FAILING, "_ResolveNodeLinks", "failed to create stoichiometry math" );        
	  }
	  SimplifyInitialAssignment(law);
	  if (law->valueType == KINETIC_LAW_VALUE_TYPE_REAL) {
	    stoichiometry = GetRealValueFromKineticLaw(law); 
	  } else if (law->valueType == KINETIC_LAW_VALUE_TYPE_INT) {
	    stoichiometry = (double)GetIntValueFromKineticLaw(law); 
	  } 
	} else {
	  stoichiometry = SpeciesReference_getStoichiometry( speciesRef );
	  constant = SpeciesReference_getConstant( speciesRef );
	  Species = Model_getSpeciesById( model, species ); 
	  if (SpeciesReference_isSetId( speciesRef )) {
	    speciesRefId = (char*)SpeciesReference_getId( speciesRef );
	    if( ( SpeciesRef = symtab->AddSpeciesRefSymbol( symtab, speciesRefId, stoichiometry, constant ) ) == NULL ) {
	      return ErrorReport( FAILING, "_ResolveNodeLinks", 
				  "failed to put parameter time in global symtab" );
	    }     
	  }
	}
        speciesNode = (SPECIES*)GetValueFromHashTable( species, strlen( species ), table );
        if( speciesNode == NULL ) {
            return ErrorReport( FAILING, "_ResolveNodeLinks", "species node for %s is not created", species );
        }
	SetAlgebraicInSpeciesNode( speciesNode, FALSE );
        if( IS_FAILED( ( ret = ir->AddReactantEdge(  ir, reactionNode, speciesNode, stoichiometry, SpeciesRef, constant ) ) ) ) {
            END_FUNCTION("_ResolveNodeLinks", SUCCESS );
            return ret;
        }
	if (IsReactionFastInReactionNode( reactionNode )) {
	  SetFastInSpeciesNode( speciesNode, TRUE );
	}
        TRACE_2( "species %s is a reactant of reaction %s", GetCharArrayOfString( GetSpeciesNodeName( speciesNode ) ), GetCharArrayOfString( GetReactionNodeName( reactionNode ) ) );
    }    
  
                  
    products = Reaction_getListOfProducts( reaction );
    num = Reaction_getNumProducts( reaction );
    for( i = 0; i < num; i++ ) {
        speciesRef = (SpeciesReference_t *)ListOf_get( products, i );
        species = (char*)SpeciesReference_getSpecies( speciesRef );
	speciesRefId = NULL;
	SpeciesRef = NULL;
	constant = FALSE;
	if (SpeciesReference_isSetStoichiometryMath( speciesRef ) ) {
	  StoichiometryMath_t *sm = SpeciesReference_getStoichiometryMath( speciesRef );
	  node = (ASTNode_t *)StoichiometryMath_getMath( sm );
	  if( ( law = _TransformKineticLaw( frontend, node, manager, table ) ) == NULL ) {
	    return ErrorReport( FAILING, "_ResolveNodeLinks", "failed to create stoichiometry math" );        
	  }
	  SimplifyInitialAssignment(law);
	  if (law->valueType == KINETIC_LAW_VALUE_TYPE_REAL) {
	    stoichiometry = GetRealValueFromKineticLaw(law); 
	  } else if (law->valueType == KINETIC_LAW_VALUE_TYPE_INT) {
	    stoichiometry = (double)GetIntValueFromKineticLaw(law); 
	  } 
	} else {
	  stoichiometry = SpeciesReference_getStoichiometry( speciesRef );
	  constant = SpeciesReference_getConstant( speciesRef );
	  Species = Model_getSpeciesById( model, species ); 
	  if (SpeciesReference_isSetId( speciesRef )) {
	    speciesRefId = (char*)SpeciesReference_getId( speciesRef );
	    if( ( SpeciesRef = symtab->AddSpeciesRefSymbol( symtab, speciesRefId, stoichiometry, constant ) ) == NULL ) {
	      return ErrorReport( FAILING, "_ResolveNodeLinks", 
				  "failed to put parameter time in global symtab" );
	    }     
	  }
	}
        speciesNode = (SPECIES*)GetValueFromHashTable( species, strlen( species ), table );
        if( speciesNode == NULL ) {
            return ErrorReport( FAILING, "_ResolveNodeLinks", "species node for %s is not created", species );
        }
	SetAlgebraicInSpeciesNode( speciesNode, FALSE );
        if( IS_FAILED( ( ret = ir->AddProductEdge( ir, reactionNode, speciesNode, stoichiometry, SpeciesRef, constant ) ) ) ) {
            END_FUNCTION("_ResolveNodeLinks", SUCCESS );
            return ret;
        }
	if (IsReactionFastInReactionNode( reactionNode )) {
	  SetFastInSpeciesNode( speciesNode, TRUE );
	}
        TRACE_2( "species %s is a product of reaction %s", GetCharArrayOfString( GetSpeciesNodeName( speciesNode ) ), GetCharArrayOfString( GetReactionNodeName( reactionNode ) ) );
    }

    modifiers = Reaction_getListOfModifiers( reaction );
    num = Reaction_getNumModifiers( reaction );
    for( i = 0; i < num; i++ ) {
        modifierRef = (SpeciesReference_t *)ListOf_get( modifiers, i );
	//        modifierRef = (ModifierSpeciesReference_t*)ListOf_get( modifiers, i );
        species = (char *)SpeciesReference_getSpecies( modifierRef );
        speciesNode = (SPECIES*)GetValueFromHashTable( species, strlen( species ), table );
        if( speciesNode == NULL ) {
            return ErrorReport( FAILING, "_ResolveNodeLinks", "species node for %s is not created", species );
        }
        if( IS_FAILED( ( ret = ir->AddModifierEdge( ir, reactionNode, speciesNode, 1 ) ) ) ) {
            END_FUNCTION("_ResolveNodeLinks", SUCCESS );
            return ret;
        }
        TRACE_2( "species %s is a modifier of reaction %s", GetCharArrayOfString( GetSpeciesNodeName( speciesNode ) ), GetCharArrayOfString( GetReactionNodeName( reactionNode ) ) );
    }    
    END_FUNCTION("_ResolveNodeLinks", SUCCESS );
    return ret;
}
 
 
