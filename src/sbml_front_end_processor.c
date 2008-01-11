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

#ifdef DEBUG
#include "sbml/math/FormulaFormatter.h"
#endif

static RET_VAL _ParseSBML( FRONT_END_PROCESSOR *frontend );
static RET_VAL _GenerateIR( FRONT_END_PROCESSOR *frontend, IR *ir );
static RET_VAL _HandleGlobalParameters( FRONT_END_PROCESSOR *frontend, Model_t *model );

static RET_VAL _HandleUnitDefinitions( FRONT_END_PROCESSOR *frontend, Model_t *model );
static RET_VAL _HandleUnitDefinition( FRONT_END_PROCESSOR *frontend, Model_t *model, UnitDefinition_t *unitDef );

static RET_VAL _HandleCompartments( FRONT_END_PROCESSOR *frontend, Model_t *model );
static RET_VAL _HandleCompartment( FRONT_END_PROCESSOR *frontend, Model_t *model, Compartment_t *compartment );

static RET_VAL _CreateSpeciesNodes(  FRONT_END_PROCESSOR *frontend, IR *ir, Model_t *model );
static RET_VAL _CreateSpeciesNode(  FRONT_END_PROCESSOR *frontend, IR *ir, Model_t *model, Species_t *species );

static RET_VAL _CreateReactionNodes( FRONT_END_PROCESSOR *frontend, IR *ir, Model_t *model );
static RET_VAL _CreateReactionNode( FRONT_END_PROCESSOR *frontend, IR *ir, Model_t *model, Reaction_t *reaction );
static RET_VAL _CreateKineticLaw( FRONT_END_PROCESSOR *frontend, REACTION *reactionNode, Model_t *model, Reaction_t *reaction );
static KINETIC_LAW *_TransformKineticLaw( FRONT_END_PROCESSOR *frontend, ASTNode_t *source, SBML_SYMTAB_MANAGER *manager, HASH_TABLE *table );
static KINETIC_LAW *_TransformOpKineticLaw( FRONT_END_PROCESSOR *frontend, ASTNode_t *source, SBML_SYMTAB_MANAGER *manager, HASH_TABLE *table );
static KINETIC_LAW *_TransformFunctionKineticLaw( FRONT_END_PROCESSOR *frontend, ASTNode_t *source, SBML_SYMTAB_MANAGER *manager, HASH_TABLE *table );
static KINETIC_LAW *_TransformSymKineticLaw( FRONT_END_PROCESSOR *frontend, ASTNode_t *source, SBML_SYMTAB_MANAGER *manager, HASH_TABLE *table );
static KINETIC_LAW *_TransformIntValueKineticLaw( FRONT_END_PROCESSOR *frontend, ASTNode_t *source, SBML_SYMTAB_MANAGER *manager, HASH_TABLE *table );
static KINETIC_LAW *_TransformRealValueKineticLaw( FRONT_END_PROCESSOR *frontend, ASTNode_t *source, SBML_SYMTAB_MANAGER *manager, HASH_TABLE *table );

static RET_VAL _ResolveNodeLinks( FRONT_END_PROCESSOR *frontend, IR *ir, REACTION *reactionNode,  Reaction_t *reaction );
//static UINT _GetNumItems( ListOf_t *list );

static RET_VAL _AddGlobalParamInSymtab( FRONT_END_PROCESSOR *frontend, REB2SAC_SYMTAB *symtab, SBML_SYMTAB_MANAGER *sbmlSymtabManager );


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
#ifdef DEBUG
    Model_t *model = NULL;
#endif
    
    START_FUNCTION("_ParseSBML");

    record = frontend->record;
    
    doc = readSBML( GetCharArrayOfString( record->inputPath ) );
    //    SBMLDocument_checkConsistency( doc );
//     errorNum = SBMLDocument_getNumWarnings( doc );
//     if( errorNum > 0 ) {
//         /*this is not actually error*/
//         SBMLDocument_printWarnings( doc, stderr );        
//     }     
    
//     errorNum = SBMLDocument_getNumErrors( doc );
//     if( errorNum > 0 ) {
//         SBMLDocument_printErrors( doc, stderr );
//         error = TRUE;         
//     }

//     errorNum = SBMLDocument_getNumFatals( doc );
//     if( errorNum > 0 ) {
//         SBMLDocument_printFatals( doc, stderr );
//         error = TRUE;
//     }

    if( error ) {
        END_FUNCTION("_ParseSBML", FAILING );
        return ErrorReport( FAILING, "_ParseSBML", "input file error" );                                             
    }
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
    COMPARTMENT_MANAGER *compartmentManager = NULL;
    SBML_SYMTAB_MANAGER *sbmlSymtabManager = NULL;
    REB2SAC_SYMTAB *symtab = NULL;
    
    START_FUNCTION("_GenerateIR");
    
    doc = (SBMLDocument_t*)(frontend->_internal1);
    model = SBMLDocument_getModel( doc );
    table = CreateHashTable( 256 );
    frontend->_internal2 = (CADDR_T)table;
    symtab = ir->GetGlobalSymtab( ir );
    frontend->_internal3 = (CADDR_T)symtab;
    
    
    if( IS_FAILED( ( ret = _HandleGlobalParameters( frontend, model ) ) ) ) {
        END_FUNCTION("_GenerateIR", ret );
        return ret;
    }
    if( ( sbmlSymtabManager = GetSymtabManagerInstance( frontend->record ) ) == NULL ) {
        return ErrorReport( FAILING, "_GenerateIR", "error on getting symtab manager" ); 
    }
        
        
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
    
    
    if( IS_FAILED( ( ret = _CreateReactionNodes( frontend, ir, model ) ) ) ) {
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

static RET_VAL _AddGlobalParamInSymtab( FRONT_END_PROCESSOR *frontend, 
                                        REB2SAC_SYMTAB *symtab, 
                                        SBML_SYMTAB_MANAGER *sbmlSymtabManager ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("_AddGlobalParamInSymtab");
    if( IS_FAILED( ( ret = sbmlSymtabManager->PutParametersInGlobalSymtab( sbmlSymtabManager, symtab ) ) ) ) {
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
    int exponent = 0;
    int scale = 0;
    double multiplier = 0.0;
    double offset = 0.0;    
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
    
    id = UnitDefinition_getId( source );
    TRACE_1("creating unit definition %s", id );
    if( ( unitDef = unitManager->CreateUnitDefinition( unitManager, id ) ) == NULL ) {
        return ErrorReport( FAILING, "_HandleUnitDefinition", "could not allocate unit def %s", id );
    }
    
    num = UnitDefinition_getNumUnits( source );
    for( i = 0; i < num; i++ ) {
        unit = UnitDefinition_getUnit( source, i );
        kindID = Unit_getKind( unit );
        kind = UnitKind_toString( kindID );
        exponent = Unit_getExponent( unit );
        scale = Unit_getScale( unit );
        multiplier = Unit_getMultiplier( unit );
        offset = Unit_getOffset( unit );
        TRACE_6( "adding unit %s (exponent=%i, scale=%i, multiplier=%f, offset=%f) in %s", kind, exponent, scale, multiplier, offset, id );           
        if( IS_FAILED( ( ret = AddUnitInUnitDefinition( unitDef, kind, exponent, scale, multiplier, offset ) ) ) ) {
            END_FUNCTION("_HandleUnitDefinition", ret );
            return ret;
        }
    }                             
    
    END_FUNCTION("_HandleUnitDefinition", SUCCESS );
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
    int spatialDimensions = 3;
    double size = 0.0;
    char *units = NULL;
    UNIT_DEFINITION *unitDef = NULL;
    UNIT_MANAGER *unitManager = NULL;
    char *outside = NULL;
    BOOL constant = TRUE;                
    COMPARTMENT *compartment = NULL;
    COMPARTMENT_MANAGER *manager = NULL;
        
    START_FUNCTION("_HandleCompartment");
    
    if( ( manager = GetCompartmentManagerInstance( frontend->record ) ) == NULL ) {
        return ErrorReport( FAILING, "_HandleCompartment", "could not get an instance of compartment manager" );
    }
    
    id = Compartment_getId( source );
    TRACE_1("creating compartment %s", id );
    if( ( compartment = manager->CreateCompartment( manager, id ) ) == NULL ) {
        return ErrorReport( FAILING, "_HandleCompartment", "could not allocate compartment %s", id );
    }
    
    spatialDimensions = Compartment_getSpatialDimensions( source );
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
        if( IS_FAILED( ( ret = SetSizeInCompartment( compartment, -1.0 ) ) ) ) {
            END_FUNCTION("_HandleCompartment", ret );
            return ret;
        }
    }    
        
    if( (units = Compartment_getUnits( source )) != NULL ) {
        unitManager = GetUnitManagerInstance( frontend->record );
        if( ( unitDef = unitManager->LookupUnitDefinition( unitManager, units ) ) == NULL ) {
            return ErrorReport( FAILING, "_HandleUnitDefinition", "unit def %s is not declared", units );
        }             
        if( IS_FAILED( ( ret = SetUnitInCompartment( compartment, unitDef  ) ) ) ) {
            END_FUNCTION("_HandleCompartment", ret );
            return ret;
        }
    }
        
    if( ( outside = Compartment_getOutside( source ) ) != NULL ) {
        if( IS_FAILED( ( ret = SetOutsideInCompartment( compartment, outside ) ) ) ) {
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
    int charge = 0;    
    SPECIES *speciesNode = NULL; 
    HASH_TABLE *table = NULL;
    COMPARTMENT_MANAGER *compartmentManager = NULL;
    UNIT_MANAGER *unitManager = NULL;

    START_FUNCTION("_CreateSpeciesNode");
    
    table = (HASH_TABLE*)frontend->_internal2;
    
    key = Species_getId( species );
    /*
        use name as the id
    */
    //id = Species_getName( species );
    if( id == NULL ) {
        id = key;
    }
    
    TRACE_2("species id = %s and key for sym is %s", id, key );
    
    if( ( speciesNode = ir->CreateSpecies( ir, id ) ) == NULL ) {
        END_FUNCTION("_CreateSpeciesNode", ret );
        return ret;   
    }
    
    if( IS_FAILED( ( ret = PutInHashTable( key, strlen( key ), speciesNode, table ) ) ) ) {
        END_FUNCTION("_CreateSpeciesNode", ret );
        return ret;   
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
            if( IS_FAILED( ( ret = SetInitialConcentrationInSpeciesNode( speciesNode, -1.0 ) ) ) ) {
                END_FUNCTION("_CreateSpeciesNode", ret );
                return ret;   
            }
        }
    }
    
    
    if( ( unitManager = GetUnitManagerInstance( frontend->record ) ) == NULL ) {
        return ErrorReport( FAILING, "_CreateSpeciesNode", "could not get an instance of unit manager" );
    }
    id = Species_getSubstanceUnits( species );
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
    id = Species_getSpatialSizeUnits( species );
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

                
    if( ( compartmentManager = GetCompartmentManagerInstance( frontend->record ) ) == NULL ) {
        return ErrorReport( FAILING, "_CreateSpeciesNode", "could not get an instance of compartment manager" );
    }
    id = Species_getCompartment( species );
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

    if( Species_isSetCharge( species ) ) {
        charge = Species_getCharge( species );                
        TRACE_1( "\tsetting charge %i", charge );
        if( IS_FAILED( ( ret = SetChargeInSpeciesNode(speciesNode, charge ) ) ) ) {
            END_FUNCTION("_CreateSpeciesNode", ret );
            return ret;   
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
    char *name = NULL;
    REACTION *reactionNode = NULL; 

    START_FUNCTION("_CreateReactionNode");
            
    //if( ( name = Reaction_getName( reaction ) ) == NULL ) {    
    //    name = Reaction_getId( reaction );
    //}
    name = Reaction_getId( reaction );
    TRACE_1("reaction name = %s", name);
    
    if( ( reactionNode = ir->CreateReaction( ir, name ) ) == NULL ) {
        END_FUNCTION("_CreateReactionNode", ret );
        return ret;   
    }    

    if( IS_FAILED( ( ret = _CreateKineticLaw( frontend, reactionNode, model, reaction ) ) ) ) {
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
    
    
    if( IS_FAILED( ( ret = _ResolveNodeLinks( frontend, ir, reactionNode, reaction ) ) ) ) {
        END_FUNCTION("_CreateReactionNode", ret );
        return ret;
    }
                                     
    END_FUNCTION("_CreateReactionNode", SUCCESS );
    return ret;   
}



static RET_VAL _CreateKineticLaw( FRONT_END_PROCESSOR *frontend, REACTION *reactionNode, Model_t *model, Reaction_t *reaction ) {
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
        
    node = KineticLaw_getMath( source );
    if( ( law = _TransformKineticLaw( frontend, node, manager, table ) ) == NULL ) {
        return ErrorReport( FAILING, "_CreateKineticLaw", "failed to create kinetic law for %s", Reaction_getId( reaction ) );        
    }
#ifdef DEBUG
    kineticLawString = ToStringKineticLaw( law );
    printf( "\tsetting kinetic law: %s%s", GetCharArrayOfString( kineticLawString ), NEW_LINE );
    FreeString( &kineticLawString );
#endif             
     
    if( IS_FAILED( ( ret = SetKineticLawInReactionNode( reactionNode, law ) ) ) ) {
        END_FUNCTION("_CreateKineticLaw", ret );
        return ret;   
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
        if( ( law = _TransformOpKineticLaw( frontend, source, manager, table ) ) == NULL ) {
            END_FUNCTION("_TransformKineticLaw", FAILING );
            return NULL;
        }                 
        END_FUNCTION("_TransformKineticLaw", SUCCESS );
        return law;
    }
    else if( ASTNode_isName( source ) ) {
        if( ( law = _TransformSymKineticLaw( frontend, source, manager, table ) ) == NULL ) {
            END_FUNCTION("_TransformKineticLaw", FAILING );
            return NULL;
        }                 
        END_FUNCTION("_TransformKineticLaw", SUCCESS );
        return law;
    }    
    else if( ASTNode_isReal( source ) ) {
        if( ( law = _TransformRealValueKineticLaw( frontend, source, manager, table ) ) == NULL ) {
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
    else if(  ASTNode_isFunction( source ) ) {
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


static KINETIC_LAW *_TransformFunctionKineticLaw( FRONT_END_PROCESSOR *frontend, ASTNode_t *source, SBML_SYMTAB_MANAGER *manager, HASH_TABLE *table ) {
    int i = 0;
    int num = 0;
    KINETIC_LAW *law = NULL;
    KINETIC_LAW **children = NULL;
    ASTNodeType_t type = AST_UNKNOWN;
    ASTNode_t *childNode = NULL;
    
    START_FUNCTION("_TransformFunctionKineticLaw");
    
    num = ASTNode_getNumChildren( source );
    if( ( children = (KINETIC_LAW**)CALLOC( num, sizeof(KINETIC_LAW*) ) ) == NULL ) {
        END_FUNCTION("_TransformFunctionKineticLaw", FAILING );
        return NULL;
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
    
    START_FUNCTION("_TransformSymKineticLaw");
    /* Here!!! */
    
    sym = ASTNode_getName( source );
    if( manager->LookupLocalValue( manager, sym, &realValue ) ) {
        symtab = (REB2SAC_SYMTAB*)(frontend->_internal3);
        if( ( symbol = symtab->AddRealValueSymbol( symtab, sym, realValue, TRUE ) ) == NULL ) {
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
    else if( manager->LookupGlobalValue( manager, sym, &realValue ) ) {
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
    
    if( ( species = (SPECIES*)GetValueFromHashTable( sym, strlen( sym ), table ) ) == NULL ) {
        END_FUNCTION("_TransformSymKineticLaw", FAILING );
        return NULL;
    }
    TRACE_1( "sym %s is a species", sym );
    if( ( law = CreateSpeciesKineticLaw( species ) ) == NULL ) {
        END_FUNCTION("_TransformSymKineticLaw", FAILING );
        return NULL;
    }
    END_FUNCTION("_TransformSymKineticLaw", SUCCESS );
    return law;
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

    realValue = ASTNode_getReal( source );
    if( ( law = CreateRealValueKineticLaw( realValue ) ) == NULL ) {
        END_FUNCTION("_TransformRealValueKineticLaw", FAILING );
        return NULL;
    }
    END_FUNCTION("_TransformRealValueKineticLaw", SUCCESS );
    return law;
}


static RET_VAL _ResolveNodeLinks( FRONT_END_PROCESSOR *frontend, IR *ir, REACTION *reactionNode, Reaction_t *reaction ) {
    RET_VAL ret = SUCCESS;
    UINT i = 0;
    UINT num = 0;
    UINT reactantsNum = 0;
    UINT productsNum = 0;
    UINT modifierNum = 0;
    int stoichiometry = 0;
    char *species = NULL;    
    ListOf_t *reactants = NULL;
    ListOf_t *modifiers = NULL;
    ListOf_t *products = NULL;
    SpeciesReference_t *speciesRef = NULL;
    SpeciesReference_t *modifierRef = NULL;        
    //    ModifierSpeciesReference_t *modifierRef = NULL;        
    SPECIES *speciesNode = NULL;
    HASH_TABLE *table = NULL;
    
    START_FUNCTION("_ResolveNodeLinks");
        
    table = (HASH_TABLE*)frontend->_internal2;    
    
    reactants = Reaction_getListOfReactants( reaction );
    num = Reaction_getNumReactants( reaction );
    for( i = 0; i < num; i++ ) {
        speciesRef = (SpeciesReference_t*)ListOf_get( reactants, i );
        species = SpeciesReference_getSpecies( speciesRef );
        stoichiometry = (int)SpeciesReference_getStoichiometry( speciesRef );
        speciesNode = (SPECIES*)GetValueFromHashTable( species, strlen( species ), table );
        if( speciesNode == NULL ) {
            return ErrorReport( FAILING, "_ResolveNodeLinks", "species node for %s is not created", species );
        }
        if( IS_FAILED( ( ret = ir->AddReactantEdge(  ir, reactionNode, speciesNode, stoichiometry ) ) ) ) {
            END_FUNCTION("_ResolveNodeLinks", SUCCESS );
            return ret;
        }
        TRACE_2( "species %s is a reactant of reaction %s", GetCharArrayOfString( GetSpeciesNodeName( speciesNode ) ), GetCharArrayOfString( GetReactionNodeName( reactionNode ) ) );
    }    
  
                  
    products = Reaction_getListOfProducts( reaction );
    num = Reaction_getNumProducts( reaction );
    for( i = 0; i < num; i++ ) {
        speciesRef = (SpeciesReference_t*)ListOf_get( products, i );
        species = SpeciesReference_getSpecies( speciesRef );
        stoichiometry = (int)SpeciesReference_getStoichiometry( speciesRef );
        speciesNode = (SPECIES*)GetValueFromHashTable( species, strlen( species ), table );
        if( speciesNode == NULL ) {
            return ErrorReport( FAILING, "_ResolveNodeLinks", "species node for %s is not created", species );
        }
        if( IS_FAILED( ( ret = ir->AddProductEdge( ir, reactionNode, speciesNode, stoichiometry ) ) ) ) {
            END_FUNCTION("_ResolveNodeLinks", SUCCESS );
            return ret;
        }
        TRACE_2( "species %s is a product of reaction %s", GetCharArrayOfString( GetSpeciesNodeName( speciesNode ) ), GetCharArrayOfString( GetReactionNodeName( reactionNode ) ) );
    }

    modifiers = Reaction_getListOfModifiers( reaction );
    num = Reaction_getNumModifiers( reaction );
    for( i = 0; i < num; i++ ) {
        modifierRef = (SpeciesReference_t*)ListOf_get( modifiers, i );
	//        modifierRef = (ModifierSpeciesReference_t*)ListOf_get( modifiers, i );
        species = SpeciesReference_getSpecies( modifierRef );
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


// static UINT _GetNumItems( ListOf_t *list ) {
//     UINT i = 0;
//     START_FUNCTION("_GetNumItems");

//     if( list == NULL ) {
//         END_FUNCTION("_GetNumItems", SUCCESS );
//         return 0;        
//     }
//     i = ListOf_getNumItems( list );
//     END_FUNCTION("_GetNumItems", SUCCESS );
//     return i;
// }

 
 
 
