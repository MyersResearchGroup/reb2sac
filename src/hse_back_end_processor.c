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
#include "hse_back_end_processor.h"
#include "hse_logical_statement_handler.h"
#include "logical_species_node.h"
#include <math.h> 

static RET_VAL _GenerateCode( BACK_END_PROCESSOR *backend, IR *ir, FILE *file );
static RET_VAL _HandleStartModule( BACK_END_PROCESSOR *backend, IR *ir, FILE *file );
static RET_VAL _HandleEndModule( BACK_END_PROCESSOR *backend, IR *ir, FILE *file );

static RET_VAL _GenerateDefineMacros( BACK_END_PROCESSOR *backend, IR *ir, FILE *file );
static RET_VAL _GenerateDefineMacro( BACK_END_PROCESSOR *backend, REB2SAC_SYMBOL *sym, FILE *file );

static RET_VAL _GenerateOutputStatements( BACK_END_PROCESSOR *backend, LINKED_LIST *speciesList, FILE *file );
static RET_VAL _GenerateOutputStatement( BACK_END_PROCESSOR *backend, SPECIES *species, FILE *file );

static RET_VAL _GenerateMainProcess( BACK_END_PROCESSOR *backend, IR *ir, FILE *file );
static RET_VAL _GenerateLogicalStatementComment( BACK_END_PROCESSOR *backend, REACTION *reaction, FILE *file );

static RET_VAL _GenerateProductionLogicalStatements( BACK_END_PROCESSOR *backend, REB2SAC_HSE_PROCESS_INFO *process, int *index, FILE *file );
static RET_VAL _GenerateDegradationLogicalStatements( BACK_END_PROCESSOR *backend, REB2SAC_HSE_PROCESS_INFO *process, int *index, FILE *file );
static RET_VAL _GenerateLogicalStatement( BACK_END_PROCESSOR *backend, REACTION *reaction, int index, FILE *file );

static RET_VAL _InitProcessInfo( BACK_END_PROCESSOR *backend, SPECIES *target, LINKED_LIST *reactions, REB2SAC_HSE_PROCESS_INFO *process );
static RET_VAL _CleanProcessInfo( BACK_END_PROCESSOR *backend, REB2SAC_HSE_PROCESS_INFO *process );

static double _GetInitialConcentration( SPECIES *species );

 
RET_VAL ProcessHseBackend( BACK_END_PROCESSOR *backend, IR *ir ) {
    RET_VAL ret = SUCCESS;
    char *filename;
    FILE *file = NULL;
    COMPILER_RECORD_T *record = NULL;

    START_FUNCTION("ProcessHseBackend");
    
    if( !IsTransformableToHse( backend, ir ) ) {
        record = backend->record;
        return ErrorReport( FAILING, "ProcessHseBackend", "%s cannot be transformed to stochastic asynchronous circuit model", GetCharArrayOfString( record->inputPath ) );
    }   
    
    if( ( filename = backend->outputFilename ) == NULL ) {
        filename = REB2SAC_DEFAULT_HSE_OUTPUT_NAME;
    }
    if( ( file = fopen( filename, "w" ) ) == NULL ) {
        return ErrorReport( FAILING, "ProcessHseBackend", "sbml file open error" ); 
    }
    
    if( IS_FAILED( ( ret = _GenerateCode( backend, ir, file ) ) ) ) {
        END_FUNCTION("ProcessHseBackend", ret );        
        return ret;
    }
    
    fclose( file );
    
    END_FUNCTION("ProcessHseBackend", SUCCESS );        
    return ret;
}




RET_VAL CloseHseBackend( BACK_END_PROCESSOR *backend ) {
    RET_VAL ret = SUCCESS;

    START_FUNCTION("CloseHseBackend");
    
    backend->record = NULL;
        
    END_FUNCTION("CloseHseBackend", SUCCESS );
        
    return ret;
}


static RET_VAL _GenerateCode( BACK_END_PROCESSOR *backend, IR *ir, FILE *file ) {
    RET_VAL ret = SUCCESS;
    LINKED_LIST *list = NULL;

    START_FUNCTION("_GenerateCode");
    
    if( IS_FAILED( ( ret = _HandleStartModule( backend, ir, file ) ) ) ) {
        END_FUNCTION("_GenerateCode", ret );        
        return ret;
    }
    fprintf( file, NEW_LINE );
#if 0    
    if( IS_FAILED( ( ret = _GenerateDefineMacros( backend, ir, file ) ) ) ) {
        END_FUNCTION("_GenerateCode", ret );        
        return ret;
    }
    fprintf( file, NEW_LINE );
#endif
    
    list = ir->GetListOfSpeciesNodes( ir );
    if( IS_FAILED( ( ret = _GenerateOutputStatements( backend, list, file ) ) ) ) {
        END_FUNCTION("_GenerateCode", ret );        
        return ret;
    }
    fprintf( file, NEW_LINE );

    if( IS_FAILED( ( ret = _GenerateMainProcess( backend, ir, file ) ) ) ) {
        END_FUNCTION("_GenerateCode", ret );        
        return ret;
    }            
    fprintf( file, NEW_LINE );
    
    if( IS_FAILED( ( ret = _HandleEndModule( backend, ir, file ) ) ) ) {
        END_FUNCTION("_GenerateCode", ret );        
        return ret;
    }
    
    END_FUNCTION("_GenerateCode", SUCCESS );        
    return ret;
}

static RET_VAL _HandleStartModule( BACK_END_PROCESSOR *backend, IR *ir, FILE *file ) {
    RET_VAL ret = SUCCESS;
    char *name = NULL;
    char buf[128];
    STRING *inputPath = NULL;

    START_FUNCTION("_HandleStartModule");
    
    inputPath = backend->record->inputPath;
    if( ( name = GetFileNameWithoutExtension( GetCharArrayOfString( inputPath ), buf, 0, sizeof(buf) ) ) == NULL ) {
        name = REB2SAC_HSE_NO_NAME_MODULE;
    }
    
    fprintf( file, REB2SAC_HSE_START_MODULE_FORMAT, name );
    fprintf( file, NEW_LINE );
    
    END_FUNCTION("_HandleStartModule", SUCCESS );        
    return ret;
}

static RET_VAL _HandleEndModule( BACK_END_PROCESSOR *backend, IR *ir, FILE *file ) {
    RET_VAL ret = SUCCESS;

    START_FUNCTION("_HandleEndModule");
    
    fprintf( file, REB2SAC_HSE_END_MODULE_FORMAT );
    fprintf( file, NEW_LINE );
    
    END_FUNCTION("_HandleEndModule", SUCCESS );        
    return ret;
}


static RET_VAL _GenerateDefineMacros( BACK_END_PROCESSOR *backend, IR *ir, FILE *file ) {
    RET_VAL ret = SUCCESS;
    REB2SAC_SYMBOL *symbol = NULL;
    REB2SAC_SYMTAB *symtab = NULL;
    LINKED_LIST *list = NULL;

        
    START_FUNCTION("_GenerateDefineMacros");
    
    symtab = ir->GetGlobalSymtab( ir );
    if( ( list = symtab->GenerateListOfSymbols( symtab ) ) == NULL ) {
        return ErrorReport( FAILING, "_GenerateDefineMacros", "could not generate a list of symbols" );
    }
    ResetCurrentElement( list );
    while( ( symbol = (REB2SAC_SYMBOL*)GetNextFromLinkedList( list ) ) != NULL ) {
        if( IS_FAILED( ( ret = _GenerateDefineMacro( backend, symbol, file ) ) ) ) {
            END_FUNCTION("_GenerateDefineMacros", ret );        
            return ret;
        }
    }
    DeleteLinkedList( &list );
    END_FUNCTION("_GenerateDefineMacros", SUCCESS );        
    return ret;
}


static RET_VAL _GenerateDefineMacro( BACK_END_PROCESSOR *backend, REB2SAC_SYMBOL *sym, FILE *file ) {
    RET_VAL ret = SUCCESS;
    char *id = NULL;
    double value = 0.0;

    START_FUNCTION("_GenerateDefineMacro");
    
    if( !IsSymbolConstant( sym ) ) {
        END_FUNCTION("_GenerateDefineMacro", SUCCESS );        
        return ret;
    }
    if( !IsRealValueSymbol( sym ) ) {
        END_FUNCTION("_GenerateDefineMacro", SUCCESS );        
        return ret;
    }
    
    id = GetCharArrayOfString( GetSymbolID( sym ) );
    value = GetRealValueInSymbol( sym );
    
    fprintf( file, REB2SAC_HSE_DEFINE_MACRO_FORMAT, id, GenerateRealNumStringForHse( value ) );
    fprintf( file, NEW_LINE );
    
    END_FUNCTION("_GenerateDefineMacro", SUCCESS );        
    return ret;
}


static RET_VAL _GenerateOutputStatements( BACK_END_PROCESSOR *backend, LINKED_LIST *speciesList, FILE *file ) {
    RET_VAL ret = SUCCESS;
    SPECIES *species = NULL;
    
    START_FUNCTION("_GenerateOutputStatements");
    
    ResetCurrentElement( speciesList );
    while( ( species = (SPECIES*)GetNextFromLinkedList( speciesList ) ) != NULL ) {
        if( IS_FAILED( ( ret = _GenerateOutputStatement( backend, species, file ) ) ) ) {
            END_FUNCTION("_GenerateOutputStatements", ret );        
            return ret;
        }
    }
    
    END_FUNCTION("_GenerateOutputStatements", SUCCESS );        
    return ret;
}


static RET_VAL _GenerateOutputStatement( BACK_END_PROCESSOR *backend, SPECIES *species, FILE *file ) {
    RET_VAL ret = SUCCESS;
    char *outputID = NULL;
    double initValue = 0.0;

    START_FUNCTION("_GenerateOutputStatement");
    
    outputID = GetLogicalSpeciesID( backend, species );
    initValue = _GetInitialConcentration( species );
    
    outputID = GetLogicalSpeciesID( backend, species );
    initValue = _GetInitialConcentration( species );
    if( IS_REAL_EQUAL( initValue, 0.0 ) ) {
        fprintf( file, REB2SAC_HSE_OUTPUT_STATEMENT_WITH_INITIAL_VALUE_FALSE_FORMAT, outputID, GetCriticalConcentrationInLogicalSpecies( (LOGICAL_SPECIES*)species ) );
    } 
    else {
        fprintf( file, REB2SAC_HSE_OUTPUT_STATEMENT_WITH_INITIAL_VALUE_TRUE_FORMAT, outputID, GetCriticalConcentrationInLogicalSpecies( (LOGICAL_SPECIES*)species ) );
    }

    fprintf( file, NEW_LINE );
    
    END_FUNCTION("_GenerateOutputStatement", SUCCESS );        
    return ret;
}



static RET_VAL _GenerateMainProcess( BACK_END_PROCESSOR *backend, IR *ir, FILE *file ) {
    RET_VAL ret = SUCCESS;
    int i = 0;
    SPECIES *target = NULL;
    LINKED_LIST *list = NULL;
    LINKED_LIST *reactions = NULL;
    REB2SAC_HSE_PROCESS_INFO process;

    START_FUNCTION("_GenerateMainProcess");
    
    list = ir->GetListOfReactionNodes( ir );
    if( ( reactions = CloneLinkedList( list ) ) == NULL ) {
        return ErrorReport( FAILING, "_GenerateMainProcess", "could not create a clone of the reaction list" );
    }
    
    fprintf( file, REB2SAC_HSE_START_MAIN_PROC_FORMAT );
    fprintf( file, NEW_LINE );
    fprintf( file, REB2SAC_HSE_START_INFINITE_LOOP_FORMAT );
    fprintf( file, NEW_LINE );

    list = ir->GetListOfSpeciesNodes( ir );
    ResetCurrentElement( list );
    while( ( target = (SPECIES*)GetNextFromLinkedList( list ) ) != NULL ) {
        if( IS_FAILED( ( ret = _InitProcessInfo( backend, target, reactions, &process ) ) ) ) {
            END_FUNCTION("_GenerateMainProcess", ret );        
            return ret;
        }
        if( IS_FAILED( ( ret = _GenerateProductionLogicalStatements( backend, &process, &i, file ) ) ) ) {
            END_FUNCTION("_GenerateProcess", ret );        
            return ret;
        }
        if( IS_FAILED( ( ret = _GenerateDegradationLogicalStatements( backend, &process, &i, file ) ) ) ) {
            END_FUNCTION("_GenerateProcess", ret );        
            return ret;
        }
        fprintf( file, NEW_LINE );
        
        if( IS_FAILED( ( ret = _CleanProcessInfo( backend, &process ) ) ) ) {
            END_FUNCTION("_GenerateMainProcess", ret );        
            return ret;
        }        
    }
    
    DeleteLinkedList( &reactions );    
        
    fprintf( file, REB2SAC_HSE_END_INFINITE_LOOP_FORMAT );
    fprintf( file, NEW_LINE );
    fprintf( file, REB2SAC_HSE_END_MAIN_PROC_FORMAT );
    fprintf( file, NEW_LINE );
    
    END_FUNCTION("_GenerateMainProcess", SUCCESS );        
    return ret;
}



static RET_VAL _GenerateLogicalStatementComment( BACK_END_PROCESSOR *backend, REACTION *reaction, FILE *file ) {
    RET_VAL ret = SUCCESS;
    char *name = NULL;
    SPECIES *species = NULL;
    KINETIC_LAW *kineticLaw = NULL;
    STRING *kineticLawString = NULL;
    LINKED_LIST *list = NULL;    
    
    START_FUNCTION("_GenerateLogicalStatementComment");

    
    name = GetCharArrayOfString( GetReactionNodeName( reaction ) );
    if( name == NULL ) {
        name = REB2SAC_HSE_NO_ID_REACTION;
    }
    
    fprintf( file, REB2SAC_HSE_REACTION_COMMENT_FORMAT_START, name );
    list = GetReactantsInReactionNode( reaction );
    fprintf( file, REB2SAC_HSE_REACTION_COMMENT_FORMAT_REACTANTS );
    fprintf( file, REB2SAC_HSE_REACTION_COMMENT_FORMAT_L_BRACE );
    if( GetLinkedListSize( list ) == 0 ) {
        fprintf( file, REB2SAC_HSE_REACTION_COMMENT_FORMAT_NO_SPECIES );
    }
    else {
        ResetCurrentElement( list );
        species = (SPECIES*)GetNextFromLinkedList( list );
        fprintf( file, REB2SAC_HSE_REACTION_COMMENT_FORMAT_FIRST_SPECIES, GetLogicalSpeciesID( backend, species ) );
        
        while( ( species = (SPECIES*)GetNextFromLinkedList( list ) ) != NULL ) {
            fprintf( file, REB2SAC_HSE_REACTION_COMMENT_FORMAT_MORE_SPECIES, GetLogicalSpeciesID( backend, species ) );
        } 
    }
    fprintf( file, REB2SAC_HSE_REACTION_COMMENT_FORMAT_R_BRACE );

    list = GetProductsInReactionNode( reaction );
    fprintf( file, REB2SAC_HSE_REACTION_COMMENT_FORMAT_PRODUCTS );
    fprintf( file, REB2SAC_HSE_REACTION_COMMENT_FORMAT_L_BRACE );
    if( GetLinkedListSize( list ) == 0 ) {
        fprintf( file, REB2SAC_HSE_REACTION_COMMENT_FORMAT_NO_SPECIES );
    }
    else {
        ResetCurrentElement( list );
        species = (SPECIES*)GetNextFromLinkedList( list );
        fprintf( file, REB2SAC_HSE_REACTION_COMMENT_FORMAT_FIRST_SPECIES, GetLogicalSpeciesID( backend, species ) );
        
        while( ( species = (SPECIES*)GetNextFromLinkedList( list ) ) != NULL ) {
            fprintf( file, REB2SAC_HSE_REACTION_COMMENT_FORMAT_MORE_SPECIES, GetLogicalSpeciesID( backend, species ) );
        } 
    }
    fprintf( file, REB2SAC_HSE_REACTION_COMMENT_FORMAT_R_BRACE );
    
    list = GetModifiersInReactionNode( reaction );
    fprintf( file, REB2SAC_HSE_REACTION_COMMENT_FORMAT_MODIFIERS );
    fprintf( file, REB2SAC_HSE_REACTION_COMMENT_FORMAT_L_BRACE );
    if( GetLinkedListSize( list ) == 0 ) {
        fprintf( file, REB2SAC_HSE_REACTION_COMMENT_FORMAT_NO_SPECIES );
    }
    else {
        ResetCurrentElement( list );
        species = (SPECIES*)GetNextFromLinkedList( list );
        fprintf( file, REB2SAC_HSE_REACTION_COMMENT_FORMAT_FIRST_SPECIES, GetLogicalSpeciesID( backend, species ) );
        
        while( ( species = (SPECIES*)GetNextFromLinkedList( list ) ) != NULL ) {
            fprintf( file, REB2SAC_HSE_REACTION_COMMENT_FORMAT_MORE_SPECIES, GetLogicalSpeciesID( backend, species ) );
        } 
    }
    fprintf( file, REB2SAC_HSE_REACTION_COMMENT_FORMAT_R_BRACE );

    kineticLaw = GetKineticLawInReactionNode( reaction );
    kineticLawString = ToStringKineticLaw( kineticLaw );
    fprintf( file, REB2SAC_HSE_REACTION_COMMENT_FORMAT_KINETIC_LAW, GetCharArrayOfString( kineticLawString ) );
    FreeString( &kineticLawString );
    
    list = GetProductsInReactionNode( reaction );
    if( GetLinkedListSize( list ) == 0 ) {
        fprintf( file, REB2SAC_HSE_REACTION_COMMENT_FORMAT_DEGRADATION );
    }
    else {
        fprintf( file, REB2SAC_HSE_REACTION_COMMENT_FORMAT_PRODUCTION );
    }
    
    END_FUNCTION("_GenerateLogicalStatementComment", ret );
    return ret;
}



static RET_VAL _GenerateProductionLogicalStatements( BACK_END_PROCESSOR *backend, REB2SAC_HSE_PROCESS_INFO *process, int *index, FILE *file ) {
    RET_VAL ret = SUCCESS;
    int i = 0;
    REACTION *reaction = NULL;
    LINKED_LIST *reactionList = NULL;
    
    START_FUNCTION("_GenerateProductionLogicalStatements");
    
    reactionList = process->productions;
    i = *index;
        
    ResetCurrentElement( reactionList );
    while( ( reaction = (REACTION*)GetNextFromLinkedList( reactionList ) ) != NULL ) {        
        if( IS_FAILED( ( ret = _GenerateLogicalStatement( backend, reaction, i, file ) ) ) ) {
            if( ret == REB2SAC_RATE_ZERO_STATEMENT_RET ) {
                continue;
            }
            END_FUNCTION("_GenerateProductionLogicalStatements", ret );        
            return ret;
        }
        i++;
    }
    if( i == *index ) {
        if( i == 0 ) {
            fprintf( file, REB2SAC_HSE_REACTION_FORMAT_SPACES );
        }
        else {
            fprintf( file, REB2SAC_HSE_REACTION_FORMAT_OR );
        }
        fprintf( file, REB2SAC_HSE_REACTION_FORMAT_FALSE );
        fprintf( file, REB2SAC_HSE_REACTION_FORMAT_ARROW );
        fprintf( file, REB2SAC_HSE_REACTION_FORMAT_SPECIES, GetLogicalSpeciesID( backend, process->target ) );
        fprintf( file, REB2SAC_HSE_REACTION_FORMAT_PLUS );
        fprintf( file, NEW_LINE );
        i++;
    
    
    }
    *index = i;
    
    END_FUNCTION("_GenerateProductionLogicalStatements", SUCCESS );        
    return SUCCESS;
}


static RET_VAL _GenerateDegradationLogicalStatements( BACK_END_PROCESSOR *backend, REB2SAC_HSE_PROCESS_INFO *process, int *index, FILE *file ) {
    RET_VAL ret = SUCCESS;
    int i = 0;
    REACTION *reaction = NULL;
    LINKED_LIST *reactionList = NULL;
   
    START_FUNCTION("_GenerateDegradationLogicalStatements");
    
    reactionList = process->degradations;
    i = *index;
    
    ResetCurrentElement( reactionList );
    while( ( reaction = (REACTION*)GetNextFromLinkedList( reactionList ) ) != NULL ) {        
        if( IS_FAILED( ( ret = _GenerateLogicalStatement( backend, reaction, i, file ) ) ) ) {
            if( ret == REB2SAC_RATE_ZERO_STATEMENT_RET ) {
                continue;
            }
            END_FUNCTION("_GenerateDegradationLogicalStatements", ret );        
            return ret;
        }
        i++;
    }
    if( i == *index ) {
        if( i == 0 ) {
            fprintf( file, REB2SAC_HSE_REACTION_FORMAT_SPACES );
        }
        else {
            fprintf( file, REB2SAC_HSE_REACTION_FORMAT_OR );
        }
        fprintf( file, REB2SAC_HSE_REACTION_FORMAT_FALSE );
        fprintf( file, REB2SAC_HSE_REACTION_FORMAT_ARROW );
        fprintf( file, REB2SAC_HSE_REACTION_FORMAT_SPECIES, GetLogicalSpeciesID( backend, process->target ) );
        fprintf( file, REB2SAC_HSE_REACTION_FORMAT_MINUS );
        fprintf( file, NEW_LINE );
        i++;
    }
    *index = i;
    
    END_FUNCTION("_GenerateDegradationLogicalStatements", SUCCESS );        
    return SUCCESS;
}



static RET_VAL _GenerateLogicalStatement( BACK_END_PROCESSOR *backend, REACTION *reaction, int index, FILE *file ) {
    RET_VAL ret = SUCCESS;
    HSE_LOGICAL_STATEMENT_HANDLER *handler = NULL;

    START_FUNCTION("_GenerateLogicalStatement");
        
#if 0
    if( IS_FAILED( ( ret = _GenerateLogicalStatementComment( backend, reaction, file ) ) ) ) {
        END_FUNCTION("_GenerateLogicalStatement", ret );
        return ret;
    }
#endif    
    
    if( IS_FAILED( ( ret = HandleLogicalStatement( backend, reaction, index, file ) ) ) ) {
        END_FUNCTION("_GenerateLogicalStatement", ret );
        return ret;
    }
#if 0    
    fprintf( file, NEW_LINE );
#endif    
    END_FUNCTION("_GenerateLogicalStatement", SUCCESS );        
    return ret;
}


static RET_VAL _InitProcessInfo( BACK_END_PROCESSOR *backend, SPECIES *target, LINKED_LIST *reactions, REB2SAC_HSE_PROCESS_INFO *process ) {
    RET_VAL ret = SUCCESS;
    IR_EDGE *edge = NULL;
    REACTION *reaction = NULL;
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
        list = GetProductEdges( reaction );
        ResetCurrentElement( list );
        while( ( edge = GetNextEdge( list ) ) != NULL ) {
            species = GetSpeciesInIREdge( edge );
            if( species == target ) {
                if( IS_FAILED( ( ret = AddElementInLinkedList( reaction, productions ) ) ) ) {
                    END_FUNCTION("_InitProcessInfo", ret );        
                    return ret;
                }
                if( IS_FAILED( ( ret = RemoveElementFromLinkedList( reaction, reactions ) ) ) ) {
                    END_FUNCTION("_InitProcessInfo", ret );        
                    return ret;
                }
            }
        }
    }
        
    ResetCurrentElement( reactions );
    while( ( reaction = (REACTION*)GetNextFromLinkedList( reactions ) ) != NULL ) {
        list = GetReactantEdges( reaction );
        ResetCurrentElement( list );
        while( ( edge = GetNextEdge( list ) ) != NULL ) {
            species = GetSpeciesInIREdge( edge );
            if( species == target ) {
                if( IS_FAILED( ( ret = AddElementInLinkedList( reaction, degradations ) ) ) ) {
                    END_FUNCTION("_InitProcessInfo", ret );        
                    return ret;
                }
                if( IS_FAILED( ( ret = RemoveElementFromLinkedList( reaction, reactions ) ) ) ) {
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

static RET_VAL _CleanProcessInfo( BACK_END_PROCESSOR *backend, REB2SAC_HSE_PROCESS_INFO *process ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("_CleanProcessInfo");
    
    DeleteLinkedList( &(process->productions) );
    DeleteLinkedList( &(process->degradations) );
    
    END_FUNCTION("_CleanProcessInfo", SUCCESS );        
    return ret;
}

static double _GetInitialConcentration( SPECIES *species ) {
    return GetInitialConcentrationInSpeciesNode( species );
}

