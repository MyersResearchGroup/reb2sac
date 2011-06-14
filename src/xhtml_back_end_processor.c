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
#include "xhtml_back_end_processor.h"
#include "ir2xhtml_transformer.h"
#include "symtab.h"

static RET_VAL _HandleIR( BACK_END_PROCESSOR *backend, IR *ir, FILE *file );
static RET_VAL _HandleListOfReactions( BACK_END_PROCESSOR *backend, IR *ir, FILE *file );
static RET_VAL _HandleReaction( BACK_END_PROCESSOR *backend, REACTION *reaction, FILE *file );
static RET_VAL _HandleLogicalStatement( BACK_END_PROCESSOR *backend, REACTION *reaction, FILE *file );

static RET_VAL _GenerateProductionLogicalStatements( BACK_END_PROCESSOR *backend, REB2SAC_XHTML_PROCESS_INFO *process, FILE *file );
static RET_VAL _GenerateDegradationLogicalStatements( BACK_END_PROCESSOR *backend, REB2SAC_XHTML_PROCESS_INFO *process, FILE *file );

static RET_VAL _InitProcessInfo( BACK_END_PROCESSOR *backend, SPECIES *target, LINKED_LIST *reactions, REB2SAC_XHTML_PROCESS_INFO *process );
static RET_VAL _CleanProcessInfo( BACK_END_PROCESSOR *backend, REB2SAC_XHTML_PROCESS_INFO *process );


RET_VAL ProcessXHTMLBackend( BACK_END_PROCESSOR *backend, IR *ir ) {
    RET_VAL ret = SUCCESS;
    char *filename;
    LINKED_LIST *list = NULL;
    FILE *file = NULL;

    START_FUNCTION("ProcessXHTMLBackend");
    
    if( ( filename = backend->outputFilename ) == NULL ) {
        filename = REB2SAC_DEFAULT_XHTML_OUTPUT_NAME;
    }
    if( ( file = fopen( filename, "w" ) ) == NULL ) {
        return ErrorReport( FAILING, "ProcessXHTMLBackend", "sbml file open error" ); 
    }
    
    if( IS_FAILED( ( ret = _HandleIR( backend, ir, file ) ) ) ) {
        END_FUNCTION("ProcessXHTMLBackend", ret );
        return ret;
    }
    fclose( file );
    
    END_FUNCTION("ProcessXHTMLBackend", SUCCESS );
        
    return ret;
}

RET_VAL CloseXHTMLBackend( BACK_END_PROCESSOR *backend ) {
    RET_VAL ret = SUCCESS;

    START_FUNCTION("CloseXHTMLBackend");
    
    backend->record = NULL;
        
    END_FUNCTION("CloseXHTMLBackend", SUCCESS );
        
    return ret;
}


static RET_VAL _HandleIR( BACK_END_PROCESSOR *backend, IR *ir, FILE *file ) {
    RET_VAL ret = SUCCESS;
    REACTION *reaction = NULL;
    LINKED_LIST *list = NULL;
    COMPARTMENT_MANAGER *compartmentManager;
    FUNCTION_MANAGER *functionManager;
    UNIT_MANAGER *unitManager;
    RULE_MANAGER *ruleManager;
    CONSTRAINT_MANAGER *constraintManager;
    EVENT_MANAGER *eventManager;
    
    START_FUNCTION("_HandleIR");

    fprintf( file, REB2SAC_XHTML_START_FORMAT_ON_LINE );
    fprintf( file, NEW_LINE );
    fprintf( file, NEW_LINE );
    fprintf( file, REB2SAC_XHTML_MODEL_ID_FORMAT, GetCharArrayOfString( ir->GetModelId( ir ) ));
    if (ir->GetModelName( ir ) != NULL) {
      fprintf( file, REB2SAC_XHTML_MODEL_NAME_FORMAT, GetCharArrayOfString( ir->GetModelName( ir ) ));
    }
    if (ir->GetModelSubstanceUnits( ir ) != NULL) {
      fprintf( file, REB2SAC_XHTML_MODEL_SUBSTANCE_UNITS_FORMAT, 
	       GetCharArrayOfString( ir->GetModelSubstanceUnits( ir ) ));
    }
    if (ir->GetModelTimeUnits( ir ) != NULL) {
      fprintf( file, REB2SAC_XHTML_MODEL_TIME_UNITS_FORMAT, 
	       GetCharArrayOfString( ir->GetModelTimeUnits( ir ) ));
    }
    if (ir->GetModelVolumeUnits( ir ) != NULL) {
      fprintf( file, REB2SAC_XHTML_MODEL_VOLUME_UNITS_FORMAT, 
	       GetCharArrayOfString( ir->GetModelVolumeUnits( ir ) ));
    }
    if (ir->GetModelAreaUnits( ir ) != NULL) {
      fprintf( file, REB2SAC_XHTML_MODEL_AREA_UNITS_FORMAT, 
	       GetCharArrayOfString( ir->GetModelAreaUnits( ir ) ));
    }
    if (ir->GetModelLengthUnits( ir ) != NULL) {
      fprintf( file, REB2SAC_XHTML_MODEL_LENGTH_UNITS_FORMAT, 
	       GetCharArrayOfString( ir->GetModelLengthUnits( ir ) ));
    }
    if (ir->GetModelExtentUnits( ir ) != NULL) {
      fprintf( file, REB2SAC_XHTML_MODEL_EXTENT_UNITS_FORMAT, 
	       GetCharArrayOfString( ir->GetModelExtentUnits( ir ) ));
    }
    fprintf( file, REB2SAC_XHTML_END_MODEL_FORMAT );
    fprintf( file, NEW_LINE );
    fprintf( file, REB2SAC_XHTML_LINE_BREAK );
    fprintf( file, REB2SAC_XHTML_LINE_BREAK );

    if( ( functionManager = ir->GetFunctionManager( ir ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not get the function manager" );
    }
    list = functionManager->CreateListOfFunctionDefinitions( functionManager );
    if( IS_FAILED( ( ret = PrintFunctionListInXHTML( list, file ) ) ) ) {
        END_FUNCTION("GenerateXHTMLFromIR", ret );
        return ret;
    }

    if( ( unitManager = ir->GetUnitManager( ir ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not get the unit manager" );
    }
    list = unitManager->CreateListOfUnitDefinitions( unitManager );
    if( IS_FAILED( ( ret = PrintUnitListInXHTML( list, file ) ) ) ) {
        END_FUNCTION("GenerateXHTMLFromIR", ret );
        return ret;
    }

    if( ( compartmentManager = ir->GetCompartmentManager( ir ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not get the compartment manager" );
    }
    list = compartmentManager->CreateListOfCompartments( compartmentManager );
    if( IS_FAILED( ( ret = PrintCompartmentListInXHTML( list, file ) ) ) ) {
        END_FUNCTION("GenerateXHTMLFromIR", ret );
        return ret;
    }
        
    list = ir->GetListOfSpeciesNodes( ir );
    if( IS_FAILED( ( ret = PrintSpeciesListInXHTML( list, file ) ) ) ) {
        END_FUNCTION("GenerateXHTMLFromIR", ret );
        return ret;
    }

    if( IS_FAILED( ( ret = PrintConstantsInXHTML( ir, file ) ) ) ) {
        END_FUNCTION("GenerateXHTMLFromIR", ret );
        return ret;
    }

    if( ( ruleManager = ir->GetRuleManager( ir ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not get the rule manager" );
    }
    list = ruleManager->CreateListOfRules( ruleManager );
    if( IS_FAILED( ( ret = PrintRuleListInXHTML( list, file ) ) ) ) {
        END_FUNCTION("GenerateXHTMLFromIR", ret );
        return ret;
    }

    if( ( constraintManager = ir->GetConstraintManager( ir ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not get the constraint manager" );
    }
    list = constraintManager->CreateListOfConstraints( constraintManager );
    if( IS_FAILED( ( ret = PrintConstraintListInXHTML( list, file ) ) ) ) {
        END_FUNCTION("GenerateXHTMLFromIR", ret );
        return ret;
    }
    
    if( IS_FAILED( ( ret = _HandleListOfReactions( backend, ir, file ) ) ) ) {
        END_FUNCTION("_HandleIR", ret );
        return ret;
    }

    if( ( eventManager = ir->GetEventManager( ir ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not get the event manager" );
    }
    list = eventManager->CreateListOfEvents( eventManager );
    if( IS_FAILED( ( ret = PrintEventListInXHTML( list, file ) ) ) ) {
        END_FUNCTION("GenerateXHTMLFromIR", ret );
        return ret;
    }
    
    fprintf( file, NEW_LINE );
    fprintf( file, REB2SAC_XHTML_END_FORMAT );
    fprintf( file, NEW_LINE );
    
    END_FUNCTION("_HandleIR", SUCCESS );
    return ret;
}


static RET_VAL _HandleListOfReactions( BACK_END_PROCESSOR *backend, IR *ir, FILE *file ) {
    RET_VAL ret = SUCCESS;
    SPECIES *target = NULL;
    LINKED_LIST *list = NULL;
    LINKED_LIST *reactions = NULL;
    REB2SAC_XHTML_PROCESS_INFO process;
    
    START_FUNCTION("_HandleListOfReactions");

    list = ir->GetListOfReactionNodes( ir );
    if( ( reactions = CloneLinkedList( list ) ) == NULL ) {
        return ErrorReport( FAILING, "_GenerateMainProcess", "could not create a clone of the reaction list" );
    }

    fprintf( file, REB2SAC_XHTML_START_REACTION_FORMAT );
    
    list = ir->GetListOfSpeciesNodes( ir );
    ResetCurrentElement( list );
    while( ( target = (SPECIES*)GetNextFromLinkedList( list ) ) != NULL ) {
        if( IS_FAILED( ( ret = _InitProcessInfo( backend, target, reactions, &process ) ) ) ) {
            END_FUNCTION("_GenerateMainProcess", ret );        
            return ret;
        }
        if( IS_FAILED( ( ret = _GenerateProductionLogicalStatements( backend, &process, file ) ) ) ) {
            END_FUNCTION("_GenerateProcess", ret );        
            return ret;
        }
        if( IS_FAILED( ( ret = _GenerateDegradationLogicalStatements( backend, &process, file ) ) ) ) {
            END_FUNCTION("_GenerateProcess", ret );        
            return ret;
        }
	//        fprintf( file, NEW_LINE );
        
        if( IS_FAILED( ( ret = _CleanProcessInfo( backend, &process ) ) ) ) {
            END_FUNCTION("_GenerateMainProcess", ret );        
            return ret;
        }        
    }

    fprintf( file, REB2SAC_XHTML_END_REACTION_FORMAT );
    fprintf( file, NEW_LINE );
    fprintf( file, REB2SAC_XHTML_LINE_BREAK );
    fprintf( file, REB2SAC_XHTML_LINE_BREAK );
    
    DeleteLinkedList( &reactions );    
    END_FUNCTION("_HandleListOfReactions", SUCCESS );
    return ret;
}

static RET_VAL _HandleReaction( BACK_END_PROCESSOR *backend, REACTION *reaction, FILE *file ) {
    RET_VAL ret = SUCCESS;
    KINETIC_LAW *kineticLaw = NULL;
    LINKED_LIST *list = NULL;
    
    START_FUNCTION("_HandleReaction");

    fprintf( file, REB2SAC_XHTML_START_REACTION_ENTRY_FORMAT, GetCharArrayOfString( GetReactionNodeName( reaction ) ),
	     GetCharArrayOfString( GetReactionNodeCompartment( reaction ) ),
	     IsReactionReversibleInReactionNode( reaction ) ? "True" : "False",
	     IsReactionFastInReactionNode( reaction ) ? "True" : "False");
    
    list = GetReactantEdges( (IR_NODE*)reaction );
    if( IS_FAILED( ( ret = PrintListOfReactantsInXHTML( list, file ) ) ) ) {
        END_FUNCTION("_HandleReaction", ret );
        return ret;
    }    

    fprintf( file, REB2SAC_XHTML_SEPARATOR_FORMAT );
    
    list = GetProductEdges( (IR_NODE*)reaction );
    if( IS_FAILED( ( ret = PrintListOfProductsInXHTML( list, file ) ) ) ) {
        END_FUNCTION("_HandleReaction", ret );
        return ret;
    }    

    fprintf( file, REB2SAC_XHTML_SEPARATOR_FORMAT );

    list = GetModifierEdges( (IR_NODE*)reaction );
    if( IS_FAILED( ( ret = PrintListOfModifiersInXHTML( list, file ) ) ) ) {
        END_FUNCTION("_HandleReaction", ret );
        return ret;
    }    

    fprintf( file, REB2SAC_XHTML_SEPARATOR_FORMAT );
        
    kineticLaw = GetKineticLawInReactionNode( reaction );
    if( IS_FAILED( ( ret = PrintKineticLawInXHTML( kineticLaw, file ) ) ) ) {
        END_FUNCTION("_HandleReaction", ret );
        return ret;
    }    
    
/*    
    if( IS_FAILED( ( ret = _HandleLogicalStatement( backend, reaction, file ) ) ) ) {
        END_FUNCTION("_HandleReaction", ret );
        return ret;
    }        
*/
    
    fprintf( file, REB2SAC_XHTML_END_REACTION_ENTRY_FORMAT );
    fprintf( file, NEW_LINE );
    
    END_FUNCTION("_HandleReaction", SUCCESS );
    return ret;
}


static RET_VAL _HandleLogicalStatement( BACK_END_PROCESSOR *backend, REACTION *reaction, FILE *file ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("_HandleLogicalStatement");
    
    fprintf( file, REB2SAC_XHTML_START_LOGICAL_STATEMENT_FORMAT );
    
    if( IS_FAILED( ( ret = HandleLogicalStatementInXML( backend, reaction, file ) ) ) ) {
        END_FUNCTION("_HandleReaction", ret );
        return ret;
    }        
        
    fprintf( file, REB2SAC_XHTML_END_LOGICAL_STATEMENT_FORMAT );
    
    END_FUNCTION("_HandleLogicalStatement", SUCCESS );
    return ret;
}


static RET_VAL _GenerateProductionLogicalStatements( BACK_END_PROCESSOR *backend, REB2SAC_XHTML_PROCESS_INFO *process, FILE *file ) {
    RET_VAL ret = SUCCESS;
    REACTION *reaction = NULL;
    LINKED_LIST *reactionList = NULL;
    
    START_FUNCTION("_GenerateProductionLogicalStatements");
    
    reactionList = process->productions;
        
    ResetCurrentElement( reactionList );
    while( ( reaction = (REACTION*)GetNextFromLinkedList( reactionList ) ) != NULL ) {        
        if( IS_FAILED( ( ret = _HandleReaction( backend, reaction, file ) ) ) ) {
            END_FUNCTION("_GenerateProductionLogicalStatements", ret );
            return ret;
        }
    }
    END_FUNCTION("_GenerateProductionLogicalStatements", SUCCESS );        
    return SUCCESS;
}


static RET_VAL _GenerateDegradationLogicalStatements( BACK_END_PROCESSOR *backend, REB2SAC_XHTML_PROCESS_INFO *process, FILE *file ) {
    RET_VAL ret = SUCCESS;
    REACTION *reaction = NULL;
    LINKED_LIST *reactionList = NULL;
   
    START_FUNCTION("_GenerateDegradationLogicalStatements");
    
    reactionList = process->degradations;
    
    ResetCurrentElement( reactionList );
    while( ( reaction = (REACTION*)GetNextFromLinkedList( reactionList ) ) != NULL ) {        
        if( IS_FAILED( ( ret = _HandleReaction( backend, reaction, file ) ) ) ) {
            END_FUNCTION("_GenerateDegradationLogicalStatements", ret );
            return ret;
        }
    }
    
    END_FUNCTION("_GenerateDegradationLogicalStatements", SUCCESS );        
    return SUCCESS;
}


static RET_VAL _InitProcessInfo( BACK_END_PROCESSOR *backend, SPECIES *target, LINKED_LIST *reactions, REB2SAC_XHTML_PROCESS_INFO *process ) {
    RET_VAL ret = SUCCESS;
    REACTION *reaction = NULL;
    IR_EDGE *edge = NULL;
    SPECIES *species = NULL;
    LINKED_LIST *list = NULL;
    LINKED_LIST *productions = NULL;
    LINKED_LIST *degradations = NULL;
    
    START_FUNCTION("_InitProcessInfo");
    
    if( ( productions = CreateLinkedList() ) == NULL ) {
        return ErrorReport( FAILING, "_InitProcessInfo", "could not create a list of productions for %s", GetCharArrayOfString( GetSpeciesNodeName( species ) ) );
    }
    if( ( degradations = CreateLinkedList() ) == NULL ) {
        return ErrorReport( FAILING, "_InitProcessInfo", "could not create a list of degradations for %s", GetCharArrayOfString( GetSpeciesNodeName( species ) ) );
    }
    
    ResetCurrentElement( reactions );
    while( ( reaction = (REACTION*)GetNextFromLinkedList( reactions ) ) != NULL ) {
        list = GetProductEdges( (IR_NODE*)reaction );
        ResetCurrentElement( list );
        while( ( edge = GetNextEdge( list ) ) != NULL ) {
            species = GetSpeciesInIREdge( edge );
            if( species == target ) {
	      if( IS_FAILED( ( ret = AddElementInLinkedList( (CADDR_T)reaction, productions ) ) ) ) {
                    END_FUNCTION("_InitProcessInfo", ret );        
                    return ret;
                }
                if( IS_FAILED( ( ret = RemoveElementFromLinkedList( (CADDR_T)reaction, reactions ) ) ) ) {
                    END_FUNCTION("_InitProcessInfo", ret );        
                    return ret;
                }
            }
        }
    }
        
    ResetCurrentElement( reactions );
    while( ( reaction = (REACTION*)GetNextFromLinkedList( reactions ) ) != NULL ) {
        list = GetReactantEdges( (IR_NODE*)reaction );
        ResetCurrentElement( list );
        while( ( edge = GetNextEdge( list ) ) != NULL ) {
            species = GetSpeciesInIREdge( edge );
            if( species == target ) {
                if( IS_FAILED( ( ret = AddElementInLinkedList( (CADDR_T)reaction, degradations ) ) ) ) {
                    END_FUNCTION("_InitProcessInfo", ret );        
                    return ret;
                }
                if( IS_FAILED( ( ret = RemoveElementFromLinkedList( (CADDR_T)reaction, reactions ) ) ) ) {
                    END_FUNCTION("_InitProcessInfo", ret );        
                    return ret;
                }
            }
        }
    }
    
    process->target = target;
    process->productions = productions;
    process->degradations = degradations;
        
    END_FUNCTION("_InitProcessInfo", SUCCESS );        
    return ret;
}

static RET_VAL _CleanProcessInfo( BACK_END_PROCESSOR *backend, REB2SAC_XHTML_PROCESS_INFO *process ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("_CleanProcessInfo");
    
    DeleteLinkedList( &(process->productions) );
    DeleteLinkedList( &(process->degradations) );
    
    END_FUNCTION("_CleanProcessInfo", SUCCESS );        
    return ret;
}












