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
#include "hse_logical_statement_handler.h"

static HSE_LOGICAL_STATEMENT_HANDLER *_FindHseLogicalStatementHandler( BACK_END_PROCESSOR *backend, REB2SAC_HSE_REACTION_RECORD *reactionRec );

static RET_VAL _InitReactionRecord( BACK_END_PROCESSOR *backend, REACTION *reaction, REB2SAC_HSE_REACTION_RECORD *reactionRec );
static RET_VAL _ReleaseReactionRecord( REB2SAC_HSE_REACTION_RECORD *reactionRec );
static RET_VAL _CheckReactionRateZero( REB2SAC_HSE_REACTION_RECORD *reactionRec );
static RET_VAL _EvaluateModifiers( REB2SAC_HSE_REACTION_RECORD *reactionRec );
static RET_VAL _EvaluateReactionRate( REB2SAC_HSE_REACTION_RECORD *reactionRec );


static HSE_LOGICAL_STATEMENT_HANDLER *_CreateSimpleProductionHandler( BACK_END_PROCESSOR *backend );
static HSE_LOGICAL_STATEMENT_HANDLER *_CreateSimpleDegradationHandler( BACK_END_PROCESSOR *backend );
static HSE_LOGICAL_STATEMENT_HANDLER *_CreateUnknownHandler( BACK_END_PROCESSOR *backend );

static HSE_LOGICAL_STATEMENT_HANDLER *_CreateXMLSimpleProductionHandler( BACK_END_PROCESSOR *backend );
static HSE_LOGICAL_STATEMENT_HANDLER *_CreateXMLSimpleDegradationHandler( BACK_END_PROCESSOR *backend );
static HSE_LOGICAL_STATEMENT_HANDLER *_CreateXMLUnknownHandler( BACK_END_PROCESSOR *backend );

static RET_VAL _HandleSimpleProduction( HSE_LOGICAL_STATEMENT_HANDLER *handler, REB2SAC_HSE_REACTION_RECORD *reactionRec, FILE *file );
static RET_VAL _HandleSimpleDegradation( HSE_LOGICAL_STATEMENT_HANDLER *handler, REB2SAC_HSE_REACTION_RECORD *reactionRec, FILE *file );
static RET_VAL _HandleUnknown( HSE_LOGICAL_STATEMENT_HANDLER *handler, REB2SAC_HSE_REACTION_RECORD *reactionRec, FILE *file );

static RET_VAL _HandleXMLSimpleProduction( HSE_LOGICAL_STATEMENT_HANDLER *handler, REB2SAC_HSE_REACTION_RECORD *reactionRec, FILE *file );
static RET_VAL _HandleXMLSimpleDegradation( HSE_LOGICAL_STATEMENT_HANDLER *handler, REB2SAC_HSE_REACTION_RECORD *reactionRec, FILE *file );
static RET_VAL _HandleXMLUnknown( HSE_LOGICAL_STATEMENT_HANDLER *handler, REB2SAC_HSE_REACTION_RECORD *reactionRec, FILE *file );
 

RET_VAL HandleLogicalStatement( BACK_END_PROCESSOR *backend, REACTION *reaction,  int index, FILE *file ) {
    RET_VAL ret = SUCCESS;
    char *name = NULL;
    REB2SAC_HSE_REACTION_RECORD reactionRec;
    HSE_LOGICAL_STATEMENT_HANDLER *handler = NULL;

    START_FUNCTION("HandleLogicalStatement");

    reactionRec.type = REB2SAC_LOGICAL_STATEMENT_OUTPUT_TYPE_DEFAULT;
        
    if( IS_FAILED( ( ret = _InitReactionRecord( backend, reaction, &reactionRec ) ) ) ) {
        END_FUNCTION("HandleLogicalStatement", ret );
        return ret;
    }
        
    if( IS_FAILED( ( ret = _CheckReactionRateZero( &reactionRec ) ) ) ) {
        END_FUNCTION("HandleLogicalStatement", ret );
        return ret;
    } 
    
    if( reactionRec.isRateZero ) {
        if( IS_FAILED( ( ret = _ReleaseReactionRecord( &reactionRec ) ) ) ) {
            END_FUNCTION("HandleLogicalStatement", ret );
            return ret;
        } 
        ret = REB2SAC_RATE_ZERO_STATEMENT_RET;
        END_FUNCTION("HandleLogicalStatement", ret );
        return ret;
    }
    
    name = GetCharArrayOfString( GetReactionNodeName( reaction ) );
    if( name == NULL ) {
        name = REB2SAC_HSE_NO_ID_REACTION;
    }
    
    fprintf( file, REB2SAC_HSE_REACTION_COMMENT_FORMAT_START, name );
    if( index > 0 ) {
        fprintf( file, REB2SAC_HSE_REACTION_FORMAT_OR );
    }
    else {
        fprintf( file, REB2SAC_HSE_REACTION_FORMAT_SPACES );
    }

    
    if( IS_FAILED( ( ret = _EvaluateModifiers( &reactionRec ) ) ) ) {
        END_FUNCTION("HandleLogicalStatement", ret );
        return ret;
    } 
    
    if( IS_FAILED( ( ret = _EvaluateReactionRate( &reactionRec ) ) ) ) {
        END_FUNCTION("HandleLogicalStatement", ret );
        return ret;
    } 
    
    if( ( handler = _FindHseLogicalStatementHandler( backend, &reactionRec ) ) == NULL ) {
        return ErrorReport( FAILING, "HandleLogicalStatement", "could not find a handler for %s", GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
    }
    
    if( IS_FAILED( ( ret = handler->Handle( handler, &reactionRec, file ) ) ) ) {
        END_FUNCTION("HandleLogicalStatement", ret );
        return ret;
    } 
    
    if( IS_FAILED( ( ret = _ReleaseReactionRecord( &reactionRec ) ) ) ) {
        END_FUNCTION("HandleLogicalStatement", ret );
        return ret;
    } 
    fprintf( file, NEW_LINE );
    
    END_FUNCTION("HandleLogicalStatement", SUCCESS );        
    return ret;
}


RET_VAL HandleLogicalStatementInXML( BACK_END_PROCESSOR *backend, REACTION *reaction, FILE *file ) {
    RET_VAL ret = SUCCESS;
    REB2SAC_HSE_REACTION_RECORD reactionRec;
    HSE_LOGICAL_STATEMENT_HANDLER *handler = NULL;

    START_FUNCTION("HandleLogicalStatementInXML");
    
    reactionRec.type = REB2SAC_LOGICAL_STATEMENT_OUTPUT_TYPE_XML;
    
    if( IS_FAILED( ( ret = _InitReactionRecord( backend, reaction, &reactionRec ) ) ) ) {
        END_FUNCTION("HandleLogicalStatementInXML", ret );
        return ret;
    }
        
    if( IS_FAILED( ( ret = _CheckReactionRateZero( &reactionRec ) ) ) ) {
        END_FUNCTION("HandleLogicalStatementInXML", ret );
        return ret;
    } 
    
    if( !reactionRec.isRateZero ) {
        if( IS_FAILED( ( ret = _EvaluateModifiers( &reactionRec ) ) ) ) {
            END_FUNCTION("HandleLogicalStatementInXML", ret );
            return ret;
        } 
    }    
    
    if( IS_FAILED( ( ret = _EvaluateReactionRate( &reactionRec ) ) ) ) {
        END_FUNCTION("HandleLogicalStatementInXML", ret );
        return ret;
    } 
    
    if( ( handler = _FindHseLogicalStatementHandler( backend, &reactionRec ) ) == NULL ) {
        return ErrorReport( FAILING, "HandleLogicalStatementInXML", "could not find a handler for %s", GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
    }
    
    if( IS_FAILED( ( ret = handler->Handle( handler, &reactionRec, file ) ) ) ) {
        END_FUNCTION("HandleLogicalStatementInXML", ret );
        return ret;
    } 
    
    if( IS_FAILED( ( ret = _ReleaseReactionRecord( &reactionRec ) ) ) ) {
        END_FUNCTION("HandleLogicalStatementInXML", ret );
        return ret;
    } 
    
#if 0    
    fprintf( file, NEW_LINE );
#endif    
    
    END_FUNCTION("HandleLogicalStatementInXML", SUCCESS );        
    return ret;
}



static RET_VAL _InitReactionRecord( BACK_END_PROCESSOR *backend, REACTION *reaction, REB2SAC_HSE_REACTION_RECORD *reactionRec ) {
    START_FUNCTION("_InitReactionRecord");
    
    reactionRec->reaction = reaction;
    if( ( reactionRec->inhibitors = CreateLinkedList() ) == NULL ) {
        return ErrorReport( FAILING, "_InitReactionRecord", "could not allocate an inhibitor list for %s", GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
    }
    if( ( reactionRec->catalysts = CreateLinkedList() ) == NULL ) {
        return ErrorReport( FAILING, "_InitReactionRecord", "could not allocate an activator list for %s", GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
    }
    
    if( ( reactionRec->evaluater = CreateKineticLawEvaluater() ) == NULL ) {
        return ErrorReport( FAILING, "_InitReactionRecord", "could not create an evaluater for %s", GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
    }
    
    reactionRec->isRateZero = FALSE;
    
    END_FUNCTION("_CreateReactionRecord", SUCCESS );
    return SUCCESS;
}

static RET_VAL _ReleaseReactionRecord( REB2SAC_HSE_REACTION_RECORD *reactionRec ) {
    RET_VAL ret = SUCCESS;

    START_FUNCTION("_ReleaseReactionRecord");
    
    if( IS_FAILED( ( ret = DeleteLinkedList( &(reactionRec->inhibitors) ) ) ) ) {
        END_FUNCTION("_ReleaseReactionRecord", ret );        
        return ret;
    }
    if( IS_FAILED( ( ret = DeleteLinkedList( &(reactionRec->catalysts) ) ) ) ) {
        END_FUNCTION("_ReleaseReactionRecord", ret );        
        return ret;
    }
    if( IS_FAILED( ( ret = FreeKineticLawEvaluater( &(reactionRec->evaluater) ) ) ) ) {
        END_FUNCTION("_ReleaseReactionRecord", ret );        
        return ret;
    }
    
    END_FUNCTION("_ReleaseReactionRecord", SUCCESS );        
    return ret;
}


static RET_VAL _CheckReactionRateZero( REB2SAC_HSE_REACTION_RECORD *reactionRec ) {
    RET_VAL ret = SUCCESS;
    double result = 0.0;
    SPECIES *species = NULL;
    LINKED_LIST *list = NULL;
    KINETIC_LAW *kineticLaw = NULL;
    STRING *string = NULL;
    REACTION *reaction = NULL;
    KINETIC_LAW_EVALUATER *evaluater = NULL;

    START_FUNCTION("_CheckReactionRateZero");
        
    evaluater = reactionRec->evaluater;
    reaction = reactionRec->reaction;
    kineticLaw = GetKineticLawInReactionNode( reaction );    
        
    if( IS_FAILED( ( ret = evaluater->SetDefaultSpeciesValue( evaluater, REB2SAC_HSE_DEFAULT_VALUE_TO_EVALUATE_MODIFIERS ) ) ) ) {
        END_FUNCTION("_CheckReactionRateZero", ret );        
        return ret;
    }
    
    result = evaluater->Evaluate( evaluater, kineticLaw );
    if( IS_REAL_EQUAL( result, 0.0 ) ) {
#ifdef DEBUG
        string = ToStringKineticLaw( kineticLaw );
        printf("the rate from %s is 0" NEW_LINE, GetCharArrayOfString( string ) );
        FreeString( &string );
#endif
        TRACE_1("rate of %s is 0", GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
#if 0
        if( IS_FAILED( ( ret = ReplaceConstantWithAnotherConstantInKineticLaw( kineticLaw, 0.0, 1.0 ) ) ) ) {
            END_FUNCTION("_CheckReactionRateZero", ret );        
            return ret;
        }
#endif
        reactionRec->isRateZero = TRUE;
#if 0
        result = evaluater->Evaluate( evaluater, kineticLaw );
        if( IS_REAL_EQUAL( result, 0.0 ) ) {
            string = ToStringKineticLaw( kineticLaw );
            return ErrorReport( FAILING, "_CheckReactionRateZero", "error on making kinetic law %s of %s non-zero", GetCharArrayOfString( string ), GetCharArrayOfString( GetReactionNodeName( reaction ) ) ); 
        }            
#endif
    }
            
    END_FUNCTION("_EvaluateReactionRate", SUCCESS );        
    return ret;
}


static RET_VAL _EvaluateModifiers( REB2SAC_HSE_REACTION_RECORD *reactionRec ) {
    RET_VAL ret = SUCCESS;
    double value1 = 0.0;
    double value2 = 0.0;
    IR_EDGE *edge = NULL;
    SPECIES *modifier = NULL;
    KINETIC_LAW *kineticLaw = NULL;
    REACTION *reaction = NULL;
    KINETIC_LAW_EVALUATER *evaluater = NULL;
    LINKED_LIST *edges = NULL;
    
        
    START_FUNCTION("_EvaluateModifiers");
    
    evaluater = reactionRec->evaluater;
    reaction = reactionRec->reaction;
    kineticLaw = GetKineticLawInReactionNode( reaction );    
    
    if( IS_FAILED( ( ret = evaluater->SetDefaultSpeciesValue( evaluater, REB2SAC_HSE_DEFAULT_VALUE_TO_EVALUATE_MODIFIERS ) ) ) ) {
        END_FUNCTION("_EvaluateModifiers", ret );        
        return ret;
    }
    
    edges = GetModifierEdges( reaction );
    ResetCurrentElement( edges );
    while( ( edge = GetNextEdge( edges ) ) != NULL ) {
        modifier = GetSpeciesInIREdge( edge );
        if( IS_FAILED( ( ret = evaluater->SetSpeciesValue( evaluater, modifier, REB2SAC_HSE_VALUE1_TO_EVALUATE_MODIFIERS ) ) ) ) {
            END_FUNCTION("_EvaluateModifiers", ret );        
            return ret;
        }
        value1 = evaluater->Evaluate( evaluater, kineticLaw );

        if( IS_FAILED( ( ret = evaluater->SetSpeciesValue( evaluater, modifier, REB2SAC_HSE_VALUE2_TO_EVALUATE_MODIFIERS ) ) ) ) {
            END_FUNCTION("_EvaluateModifiers", ret );        
            return ret;
        }
        value2 = evaluater->Evaluate( evaluater, kineticLaw );
        
        if( IS_REAL_EQUAL( value2, value1 ) ) {
            /*value2 and value1 are equal.  most likely they are both zero. i.e., rate 0 */            
        }
        else if( value2 > value1 ) {
            TRACE_2("%s is an activator in %s", GetCharArrayOfString( GetSpeciesNodeName( modifier ) ), GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
            if( IS_FAILED( ( ret = AddElementInLinkedList( modifier, reactionRec->catalysts ) ) ) ) {
                END_FUNCTION("_EvaluateModifiers", ret );        
                return ret;
            }
        }
        else {
            TRACE_2("%s is an inhibitor in %s", GetCharArrayOfString( GetSpeciesNodeName( modifier ) ), GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
            if( IS_FAILED( ( ret = AddElementInLinkedList( modifier, reactionRec->inhibitors ) ) ) ) {
                END_FUNCTION("_EvaluateModifiers", ret );        
                return ret;
            }
        }
        if( IS_FAILED( ( ret = evaluater->RemoveSpeciesValue( evaluater, modifier ) ) ) ) {
            END_FUNCTION("_EvaluateModifiers", ret );        
            return ret;
        }
    }
           
    END_FUNCTION("_EvaluateModifiers", SUCCESS );        
    return ret;
}

static RET_VAL _EvaluateReactionRate( REB2SAC_HSE_REACTION_RECORD *reactionRec ) {
    RET_VAL ret = SUCCESS;
    double result = 0.0;
    SPECIES *species = NULL;
    IR_EDGE *edge = NULL;
    LINKED_LIST *list = NULL;
    KINETIC_LAW *kineticLaw = NULL;
#ifdef DEBUG
    STRING *string = NULL;
#endif
    REACTION *reaction = NULL;
    KINETIC_LAW_EVALUATER *evaluater = NULL;

    START_FUNCTION("_EvaluateReactionRate");
        
    evaluater = reactionRec->evaluater;
    reaction = reactionRec->reaction;
    kineticLaw = GetKineticLawInReactionNode( reaction );    
    
    if( IS_FAILED( ( ret = evaluater->SetDefaultSpeciesValue( evaluater, REB2SAC_HSE_DEFAULT_VALUE_TO_EVALUATE ) ) ) ) {
        END_FUNCTION("_EvaluateModifiers", ret );        
        return ret;
    }
    
    list = reactionRec->inhibitors;
    ResetCurrentElement( list );
    while( ( species = (SPECIES*)GetNextFromLinkedList( list ) ) != NULL ) {
        if( IS_FAILED( ( ret = evaluater->SetSpeciesValue( evaluater, species, REB2SAC_HSE_INHIBITOR_VALUE ) ) ) ) {
            END_FUNCTION("_EvaluateModifiers", ret );        
            return ret;
        }
    }
        
    list = reactionRec->catalysts;
    ResetCurrentElement( list );
    while( ( species = (SPECIES*)GetNextFromLinkedList( list ) ) != NULL ) {
        if( IS_FAILED( ( ret = evaluater->SetSpeciesValue( evaluater, species, REB2SAC_HSE_ACTIVATOR_VALUE ) ) ) ) {
            END_FUNCTION("_EvaluateModifiers", ret );        
            return ret;
        }
    }    
    
    list = GetReactantEdges( reaction );
    while( ( edge = GetNextEdge( list ) ) != NULL ) {
        species = GetSpeciesInIREdge( edge );
        if( IS_FAILED( ( ret = evaluater->SetSpeciesValue( evaluater, species, REB2SAC_HSE_ACTIVATOR_VALUE ) ) ) ) {
            END_FUNCTION("_EvaluateModifiers", ret );        
            return ret;
        }
    }    
    list = GetProductEdges( reaction );
    while( ( edge = GetNextEdge( list ) ) != NULL ) {
        species = GetSpeciesInIREdge( edge );
        if( IS_FAILED( ( ret = evaluater->SetSpeciesValue( evaluater, species, REB2SAC_HSE_INHIBITOR_VALUE ) ) ) ) {
            END_FUNCTION("_EvaluateModifiers", ret );        
            return ret;
        }
    }    
    
    result = evaluater->Evaluate( evaluater, kineticLaw );
#ifdef DEBUG
    string = ToStringKineticLaw( kineticLaw );
    printf("the rate from %s is %g" NEW_LINE, GetCharArrayOfString( string ), result );
    FreeString( &string );
#endif

    reactionRec->rate = result;
    
    END_FUNCTION("_EvaluateReactionRate", SUCCESS );        
    return ret;
}





HSE_LOGICAL_STATEMENT_HANDLER *_FindHseLogicalStatementHandler( BACK_END_PROCESSOR *backend, REB2SAC_HSE_REACTION_RECORD *reactionRec ) {
    REACTION *reaction = NULL;
    UINT32 reactantsNum = 0;
    UINT32 productsNum = 0;
    LINKED_LIST *list = NULL;
    HSE_LOGICAL_STATEMENT_HANDLER *handler = NULL;
    
    START_FUNCTION("__FindHseLogicalStatementHandler");

    reaction = reactionRec->reaction;
    list = GetReactantsInReactionNode( reaction );
    reactantsNum = GetLinkedListSize( list );
    
    list = GetProductsInReactionNode( reaction );
    productsNum = GetLinkedListSize( list );
    switch( reactionRec->type ) {
        case REB2SAC_LOGICAL_STATEMENT_OUTPUT_TYPE_XML:
            if( ( productsNum > 0 ) && ( reactantsNum == 0 ) ) {
                handler = _CreateXMLSimpleProductionHandler( backend );
            }
            else if( ( productsNum == 0 ) && ( reactantsNum > 0 ) ) {
                handler = _CreateXMLSimpleDegradationHandler( backend );
            }
            else {
                handler = _CreateXMLUnknownHandler( backend );
            }
                        
            END_FUNCTION("__FindHseLogicalStatementHandler", SUCCESS );
        return handler;
        
        default:
            if( ( productsNum > 0 ) && ( reactantsNum == 0 ) ) {
                handler = _CreateSimpleProductionHandler( backend );
            }
            else if( ( productsNum == 0 ) && ( reactantsNum > 0 ) ) {
                handler = _CreateSimpleDegradationHandler( backend );
            }
            else {
                handler = _CreateUnknownHandler( backend );
            }
                        
            END_FUNCTION("__FindHseLogicalStatementHandler", SUCCESS );
        return handler;
    }
    END_FUNCTION("__FindHseLogicalStatementHandler", FAILING );
    return NULL;
}



static HSE_LOGICAL_STATEMENT_HANDLER *_CreateSimpleProductionHandler( BACK_END_PROCESSOR *backend ) {
    static HSE_LOGICAL_STATEMENT_HANDLER handler;
    
    START_FUNCTION("_CreateSimpleProductionHandler");

    if( handler.Handle == NULL ) {
        handler.Handle = _HandleSimpleProduction;
    }    
    handler.backend = backend;
            
    END_FUNCTION("_CreateSimpleProductionHandler", SUCCESS );
    return &handler;
}

static HSE_LOGICAL_STATEMENT_HANDLER *_CreateSimpleDegradationHandler( BACK_END_PROCESSOR *backend ) {
    static HSE_LOGICAL_STATEMENT_HANDLER handler;
    
    START_FUNCTION("_CreateSimpleDegradationHandler");

    if( handler.Handle == NULL ) {
        handler.Handle = _HandleSimpleDegradation;
    }    
    handler.backend = backend;
            
    END_FUNCTION("_CreateSimpleDegradationHandler", SUCCESS );
    return &handler;
}

static HSE_LOGICAL_STATEMENT_HANDLER *_CreateUnknownHandler( BACK_END_PROCESSOR *backend  ) {
    static HSE_LOGICAL_STATEMENT_HANDLER handler;
    
    START_FUNCTION("_CreateUnknownHandler");

    if( handler.Handle == NULL ) {
        handler.Handle = _HandleUnknown;
    }    
    handler.backend = backend;
            
    END_FUNCTION("_CreateUnknownHandler", SUCCESS );
    return &handler;
}

static HSE_LOGICAL_STATEMENT_HANDLER *_CreateXMLSimpleProductionHandler( BACK_END_PROCESSOR *backend ) {
    static HSE_LOGICAL_STATEMENT_HANDLER handler;
    
    START_FUNCTION("_CreateXMLSimpleProductionHandler");

    if( handler.Handle == NULL ) {
        handler.Handle = _HandleXMLSimpleProduction;
    }    
    handler.backend = backend;
            
    END_FUNCTION("_CreateXMLSimpleProductionHandler", SUCCESS );
    return &handler;
}

static HSE_LOGICAL_STATEMENT_HANDLER *_CreateXMLSimpleDegradationHandler( BACK_END_PROCESSOR *backend ) {
    static HSE_LOGICAL_STATEMENT_HANDLER handler;
    
    START_FUNCTION("_CreateXMLSimpleDegradationHandler");

    if( handler.Handle == NULL ) {
        handler.Handle = _HandleXMLSimpleDegradation;
    }    
    handler.backend = backend;
            
    END_FUNCTION("_CreateXMLSimpleDegradationHandler", SUCCESS );
    return &handler;
}

static HSE_LOGICAL_STATEMENT_HANDLER *_CreateXMLUnknownHandler( BACK_END_PROCESSOR *backend  ) {
    static HSE_LOGICAL_STATEMENT_HANDLER handler;
    
    START_FUNCTION("_CreateXMLUnknownHandler");

    if( handler.Handle == NULL ) {
        handler.Handle = _HandleXMLUnknown;
    }    
    handler.backend = backend;
            
    END_FUNCTION("_CreateXMLUnknownHandler", SUCCESS );
    return &handler;
}




static RET_VAL _HandleSimpleProduction( HSE_LOGICAL_STATEMENT_HANDLER *handler, REB2SAC_HSE_REACTION_RECORD *reactionRec, FILE *file ) {
    RET_VAL ret = SUCCESS;
    BOOL first = TRUE;
    int type = 0;
    IR_EDGE *edge = NULL;
    SPECIES *species = NULL;
    REACTION *reaction = NULL;
    LINKED_LIST *list = NULL;
    BACK_END_PROCESSOR *backend;

    START_FUNCTION("_HandleSimpleProduction");
    
    reaction = reactionRec->reaction;
    type = reactionRec->type;
    
    if( !(reactionRec->isRateZero) ) {
        list = reactionRec->inhibitors;        
        if( GetLinkedListSize( list ) > 0 ) {
            ResetCurrentElement( list );
            species = (SPECIES*)GetNextFromLinkedList( list );
            fprintf( file, REB2SAC_HSE_REACTION_FORMAT_NOT );
            fprintf( file, REB2SAC_HSE_REACTION_FORMAT_SPECIES, GetLogicalSpeciesID( backend, species ) );
            first = FALSE;
        }
        while( ( species = (SPECIES*)GetNextFromLinkedList( list ) ) != NULL ) {
            fprintf( file, REB2SAC_HSE_REACTION_FORMAT_AND );
            fprintf( file, REB2SAC_HSE_REACTION_FORMAT_NOT );
            fprintf( file, REB2SAC_HSE_REACTION_FORMAT_SPECIES, GetLogicalSpeciesID( backend, species ) );
        }

        list = reactionRec->catalysts;
        if( GetLinkedListSize( list ) > 0 ) {
            if( first ) {
                first = FALSE;
            }
            else {
                fprintf( file, REB2SAC_HSE_REACTION_FORMAT_AND );
            }
            ResetCurrentElement( list );
            species = (SPECIES*)GetNextFromLinkedList( list );
            fprintf( file, REB2SAC_HSE_REACTION_FORMAT_SPECIES, GetLogicalSpeciesID( backend, species ) );
        }
        while( ( species = (SPECIES*)GetNextFromLinkedList( list ) ) != NULL ) {
            fprintf( file, REB2SAC_HSE_REACTION_FORMAT_AND );
            fprintf( file, REB2SAC_HSE_REACTION_FORMAT_SPECIES, GetLogicalSpeciesID( backend, species ) );
        }
        
        list = GetProductEdges( reaction );
        
        if( first ) {
            first = FALSE;
        }
        else {
            fprintf( file, REB2SAC_HSE_REACTION_FORMAT_AND );
        }
        
        /* simple production has at least one product */
        ResetCurrentElement( list );
        edge = GetNextEdge( list );
        species = GetSpeciesInIREdge( edge );
        fprintf( file, REB2SAC_HSE_REACTION_FORMAT_NOT );
        fprintf( file, REB2SAC_HSE_REACTION_FORMAT_SPECIES, GetLogicalSpeciesID( backend, species ) );
        while( ( edge = GetNextEdge( list ) ) != NULL ) {
            species = GetSpeciesInIREdge( edge );
            fprintf( file, REB2SAC_HSE_REACTION_FORMAT_AND );
            fprintf( file, REB2SAC_HSE_REACTION_FORMAT_NOT );
            fprintf( file, REB2SAC_HSE_REACTION_FORMAT_SPECIES, GetLogicalSpeciesID( backend, species ) );
        }

        fprintf( file, REB2SAC_HSE_REACTION_FORMAT_RATE, GenerateRealNumStringForHse( reactionRec->rate ) );
    }
    else {
        fprintf( file, REB2SAC_HSE_REACTION_FORMAT_FALSE );
    }
    fprintf( file, REB2SAC_HSE_REACTION_FORMAT_ARROW );

    /* simple production has at least one product */
    list = GetProductEdges( reaction );
    ResetCurrentElement( list );
    edge = GetNextEdge( list );
    species = GetSpeciesInIREdge( edge );
    fprintf( file, REB2SAC_HSE_REACTION_FORMAT_SPECIES, GetLogicalSpeciesID( backend, species ) );
    fprintf( file, REB2SAC_HSE_REACTION_FORMAT_PLUS );
    while( ( edge = GetNextEdge( list ) ) != NULL ) {
        species = GetSpeciesInIREdge( edge );
        fprintf( file, REB2SAC_HSE_REACTION_FORMAT_SEMICOLON );
        fprintf( file, REB2SAC_HSE_REACTION_FORMAT_SPECIES, GetLogicalSpeciesID( backend, species ) );
        fprintf( file, REB2SAC_HSE_REACTION_FORMAT_PLUS );
    }

    END_FUNCTION("_HandleSimpleProduction", SUCCESS );        
    return ret;
}

static RET_VAL _HandleSimpleDegradation( HSE_LOGICAL_STATEMENT_HANDLER *handler, REB2SAC_HSE_REACTION_RECORD *reactionRec, FILE *file ) {
    RET_VAL ret = SUCCESS;
    BOOL first = TRUE;
    IR_EDGE *edge = NULL;
    SPECIES *species = NULL;
    REACTION *reaction = NULL;
    LINKED_LIST *list = NULL;
    BACK_END_PROCESSOR *backend;

    START_FUNCTION("_HandleSimpleDegradation");

    reaction = reactionRec->reaction;
    
    if( !(reactionRec->isRateZero) ) {
        list = reactionRec->inhibitors;
        if( GetLinkedListSize( list ) > 0 ) {
            ResetCurrentElement( list );
            species = (SPECIES*)GetNextFromLinkedList( list );
            fprintf( file, REB2SAC_HSE_REACTION_FORMAT_NOT );
            fprintf( file, REB2SAC_HSE_REACTION_FORMAT_SPECIES, GetLogicalSpeciesID( backend, species ) );
            first = FALSE;
        }
        while( ( species = (SPECIES*)GetNextFromLinkedList( list ) ) != NULL ) {
            fprintf( file, REB2SAC_HSE_REACTION_FORMAT_AND );
            fprintf( file, REB2SAC_HSE_REACTION_FORMAT_NOT );
            fprintf( file, REB2SAC_HSE_REACTION_FORMAT_SPECIES, GetLogicalSpeciesID( backend, species ) );
        }

        list = reactionRec->catalysts;
        if( GetLinkedListSize( list ) > 0 ) {
            if( first ) {
                first = FALSE;            
            }
            else {
                fprintf( file, REB2SAC_HSE_REACTION_FORMAT_AND );
            }
            ResetCurrentElement( list );
            species = (SPECIES*)GetNextFromLinkedList( list );
            fprintf( file, REB2SAC_HSE_REACTION_FORMAT_SPECIES, GetLogicalSpeciesID( backend, species ) );
        }
        while( ( species = (SPECIES*)GetNextFromLinkedList( list ) ) != NULL ) {
            fprintf( file, REB2SAC_HSE_REACTION_FORMAT_AND );
            fprintf( file, REB2SAC_HSE_REACTION_FORMAT_SPECIES, GetLogicalSpeciesID( backend, species ) );
        }

        /* simple degradation has at least one reactant */
        list = GetReactantEdges( reaction );
        if( first ) {
            first = FALSE;
        }
        else {
            fprintf( file, REB2SAC_HSE_REACTION_FORMAT_AND );
        }
        ResetCurrentElement( list );        
        edge = GetNextEdge( list );
        species = GetSpeciesInIREdge( edge );
        fprintf( file, REB2SAC_HSE_REACTION_FORMAT_SPECIES, GetLogicalSpeciesID( backend, species ) );
        while( ( edge = GetNextEdge( list ) ) != NULL ) {
            species = GetSpeciesInIREdge( edge );
            fprintf( file, REB2SAC_HSE_REACTION_FORMAT_AND );
            fprintf( file, REB2SAC_HSE_REACTION_FORMAT_SPECIES, GetLogicalSpeciesID( backend, species ) );
        }    
        fprintf( file, REB2SAC_HSE_REACTION_FORMAT_RATE, GenerateRealNumStringForHse( reactionRec->rate ) );
    }
    else {
        fprintf( file, REB2SAC_HSE_REACTION_FORMAT_FALSE );
    }
    
    list = GetReactantEdges( reaction );
    fprintf( file, REB2SAC_HSE_REACTION_FORMAT_ARROW );

    /* simple degradation has at least one reactant */
    ResetCurrentElement( list );
    edge = GetNextEdge( list );
    species = GetSpeciesInIREdge( edge );
    fprintf( file, REB2SAC_HSE_REACTION_FORMAT_SPECIES, GetLogicalSpeciesID( backend, species ) );
    fprintf( file, REB2SAC_HSE_REACTION_FORMAT_MINUS );
    while( ( edge = GetNextEdge( list ) ) != NULL ) {
        species = GetSpeciesInIREdge( edge );
        fprintf( file, REB2SAC_HSE_REACTION_FORMAT_SEMICOLON );
        fprintf( file, REB2SAC_HSE_REACTION_FORMAT_SPECIES, GetLogicalSpeciesID( backend, species ) );
        fprintf( file, REB2SAC_HSE_REACTION_FORMAT_MINUS );
    }

    END_FUNCTION("_HandleSimpleDegradation", SUCCESS );        
    return ret;
}

static RET_VAL _HandleUnknown( HSE_LOGICAL_STATEMENT_HANDLER *handler, REB2SAC_HSE_REACTION_RECORD *reactionRec, FILE *file ) {
    RET_VAL ret = SUCCESS;

    START_FUNCTION("_HandleUnknown");

    fprintf( file, REB2SAC_HSE_REACTION_FORMAT_FALSE );
    fprintf( file, REB2SAC_HSE_REACTION_FORMAT_ARROW );
    fprintf( file, REB2SAC_HSE_REACTION_FORMAT_FALSE );

    END_FUNCTION("_HandleUnknown", SUCCESS );        
    return ret;
}





static RET_VAL _HandleXMLSimpleProduction( HSE_LOGICAL_STATEMENT_HANDLER *handler, REB2SAC_HSE_REACTION_RECORD *reactionRec, FILE *file ) {
    RET_VAL ret = SUCCESS;
    BOOL first = TRUE;
    int type = 0;
    IR_EDGE *edge = NULL;
    SPECIES *species = NULL;
    REACTION *reaction = NULL;
    LINKED_LIST *list = NULL;
    BACK_END_PROCESSOR *backend;

    START_FUNCTION("_HandleXMLSimpleProduction");
    
    reaction = reactionRec->reaction;
    type = reactionRec->type;
    
    if( !(reactionRec->isRateZero) ) {
        list = reactionRec->inhibitors;        
        if( GetLinkedListSize( list ) > 0 ) {
            ResetCurrentElement( list );
            species = (SPECIES*)GetNextFromLinkedList( list );
            fprintf( file, REB2SAC_HSE_REACTION_FORMAT_NOT_XML );
            fprintf( file, REB2SAC_HSE_REACTION_FORMAT_SPECIES, GetLogicalSpeciesID( backend, species ) );
            first = FALSE;
        }
        while( ( species = (SPECIES*)GetNextFromLinkedList( list ) ) != NULL ) {
            fprintf( file, REB2SAC_HSE_REACTION_FORMAT_AND_XML );
            fprintf( file, REB2SAC_HSE_REACTION_FORMAT_NOT_XML );
            fprintf( file, REB2SAC_HSE_REACTION_FORMAT_SPECIES, GetLogicalSpeciesID( backend, species ) );
        }

        list = reactionRec->catalysts;
        if( GetLinkedListSize( list ) > 0 ) {
            if( first ) {
                first = FALSE;
            }
            else {
                fprintf( file, REB2SAC_HSE_REACTION_FORMAT_AND_XML );
            }
            ResetCurrentElement( list );
            species = (SPECIES*)GetNextFromLinkedList( list );
            fprintf( file, REB2SAC_HSE_REACTION_FORMAT_SPECIES, GetLogicalSpeciesID( backend, species ) );
        }
        while( ( species = (SPECIES*)GetNextFromLinkedList( list ) ) != NULL ) {
            fprintf( file, REB2SAC_HSE_REACTION_FORMAT_AND_XML );
            fprintf( file, REB2SAC_HSE_REACTION_FORMAT_SPECIES, GetLogicalSpeciesID( backend, species ) );
        }
        
        list = GetProductEdges( reaction );
        
        if( first ) {
            first = FALSE;
        }
        else {
            fprintf( file, REB2SAC_HSE_REACTION_FORMAT_AND_XML );
        }
        
        /* simple production has at least one product */
        ResetCurrentElement( list );
        edge = GetNextEdge( list );
        species = GetSpeciesInIREdge( edge );
        fprintf( file, REB2SAC_HSE_REACTION_FORMAT_NOT_XML );
        fprintf( file, REB2SAC_HSE_REACTION_FORMAT_SPECIES, GetLogicalSpeciesID( backend, species ) );
        while( ( edge = GetNextEdge( list ) ) != NULL ) {
            species = GetSpeciesInIREdge( edge );
            fprintf( file, REB2SAC_HSE_REACTION_FORMAT_AND_XML );
            fprintf( file, REB2SAC_HSE_REACTION_FORMAT_NOT_XML );
            fprintf( file, REB2SAC_HSE_REACTION_FORMAT_SPECIES, GetLogicalSpeciesID( backend, species ) );
        }

        fprintf( file, REB2SAC_HSE_REACTION_FORMAT_RATE, GenerateRealNumStringForHse( reactionRec->rate ) );
    }
    else {
        fprintf( file, REB2SAC_HSE_REACTION_FORMAT_FALSE );
    }
    fprintf( file, REB2SAC_HSE_REACTION_FORMAT_ARROW );

    /* simple production has at least one product */
    list = GetProductEdges( reaction );
    ResetCurrentElement( list );
    edge = GetNextEdge( list );
    species = GetSpeciesInIREdge( edge );
    fprintf( file, REB2SAC_HSE_REACTION_FORMAT_SPECIES, GetLogicalSpeciesID( backend, species ) );
    fprintf( file, REB2SAC_HSE_REACTION_FORMAT_PLUS );
    while( ( edge = GetNextEdge( list ) ) != NULL ) {
        species = GetSpeciesInIREdge( edge );
        fprintf( file, REB2SAC_HSE_REACTION_FORMAT_SEMICOLON );
        fprintf( file, REB2SAC_HSE_REACTION_FORMAT_SPECIES, GetLogicalSpeciesID( backend, species ) );
        fprintf( file, REB2SAC_HSE_REACTION_FORMAT_PLUS );
    }

    END_FUNCTION("_HandleXMLSimpleProduction", SUCCESS );        
    return ret;
}

static RET_VAL _HandleXMLSimpleDegradation( HSE_LOGICAL_STATEMENT_HANDLER *handler, REB2SAC_HSE_REACTION_RECORD *reactionRec, FILE *file ) {
    RET_VAL ret = SUCCESS;
    BOOL first = TRUE;
    IR_EDGE *edge = NULL;
    SPECIES *species = NULL;
    REACTION *reaction = NULL;
    LINKED_LIST *list = NULL;
    BACK_END_PROCESSOR *backend;

    START_FUNCTION("_HandleXMLSimpleDegradation");

    reaction = reactionRec->reaction;
    
    if( !(reactionRec->isRateZero) ) {
        list = reactionRec->inhibitors;
        if( GetLinkedListSize( list ) > 0 ) {
            ResetCurrentElement( list );
            species = (SPECIES*)GetNextFromLinkedList( list );
            fprintf( file, REB2SAC_HSE_REACTION_FORMAT_NOT_XML );
            fprintf( file, REB2SAC_HSE_REACTION_FORMAT_SPECIES, GetLogicalSpeciesID( backend, species ) );
            first = FALSE;
        }
        while( ( species = (SPECIES*)GetNextFromLinkedList( list ) ) != NULL ) {
            fprintf( file, REB2SAC_HSE_REACTION_FORMAT_AND_XML );
            fprintf( file, REB2SAC_HSE_REACTION_FORMAT_NOT_XML );
            fprintf( file, REB2SAC_HSE_REACTION_FORMAT_SPECIES, GetLogicalSpeciesID( backend, species ) );
        }

        list = reactionRec->catalysts;
        if( GetLinkedListSize( list ) > 0 ) {
            if( first ) {
                first = FALSE;            
            }
            else {
                fprintf( file, REB2SAC_HSE_REACTION_FORMAT_AND_XML );
            }
            ResetCurrentElement( list );
            species = (SPECIES*)GetNextFromLinkedList( list );
            fprintf( file, REB2SAC_HSE_REACTION_FORMAT_SPECIES, GetLogicalSpeciesID( backend, species ) );
        }
        while( ( species = (SPECIES*)GetNextFromLinkedList( list ) ) != NULL ) {
            fprintf( file, REB2SAC_HSE_REACTION_FORMAT_AND_XML );
            fprintf( file, REB2SAC_HSE_REACTION_FORMAT_SPECIES, GetLogicalSpeciesID( backend, species ) );
        }

        /* simple degradation has at least one reactant */
        list = GetReactantEdges( reaction );
        if( first ) {
            first = FALSE;
        }
        else {
            fprintf( file, REB2SAC_HSE_REACTION_FORMAT_AND_XML );
        }
        ResetCurrentElement( list );
        edge = GetNextEdge( list );
        species = GetSpeciesInIREdge( edge );
        fprintf( file, REB2SAC_HSE_REACTION_FORMAT_SPECIES, GetLogicalSpeciesID( backend, species ) );
        while( ( edge = GetNextEdge( list ) ) != NULL ) {
            species = GetSpeciesInIREdge( edge );
            fprintf( file, REB2SAC_HSE_REACTION_FORMAT_AND_XML );
            fprintf( file, REB2SAC_HSE_REACTION_FORMAT_SPECIES, GetLogicalSpeciesID( backend, species ) );
        }    
        fprintf( file, REB2SAC_HSE_REACTION_FORMAT_RATE, GenerateRealNumStringForHse( reactionRec->rate ) );
    }
    else {
        fprintf( file, REB2SAC_HSE_REACTION_FORMAT_FALSE );    
    }
    
    list = GetReactantEdges( reaction );
    fprintf( file, REB2SAC_HSE_REACTION_FORMAT_ARROW );

    /* simple degradation has at least one reactant */
    ResetCurrentElement( list );
    edge = GetNextEdge( list );
    species = GetSpeciesInIREdge( edge );
    fprintf( file, REB2SAC_HSE_REACTION_FORMAT_SPECIES, GetLogicalSpeciesID( backend, species ) );
    fprintf( file, REB2SAC_HSE_REACTION_FORMAT_MINUS );
    while( ( edge = GetNextEdge( list ) ) != NULL ) {
        species = GetSpeciesInIREdge( edge );
        fprintf( file, REB2SAC_HSE_REACTION_FORMAT_SEMICOLON );
        fprintf( file, REB2SAC_HSE_REACTION_FORMAT_SPECIES, GetLogicalSpeciesID( backend, species ) );
        fprintf( file, REB2SAC_HSE_REACTION_FORMAT_MINUS );
    }

    END_FUNCTION("_HandleXMLSimpleDegradation", SUCCESS );        
    return ret;
}

static RET_VAL _HandleXMLUnknown( HSE_LOGICAL_STATEMENT_HANDLER *handler, REB2SAC_HSE_REACTION_RECORD *reactionRec, FILE *file ) {
    RET_VAL ret = SUCCESS;

    START_FUNCTION("_HandleXMLUnknown");

    fprintf( file, REB2SAC_HSE_REACTION_FORMAT_FALSE );
    fprintf( file, REB2SAC_HSE_REACTION_FORMAT_ARROW );
    fprintf( file, REB2SAC_HSE_REACTION_FORMAT_FALSE );

    END_FUNCTION("_HandleXMLUnknown", SUCCESS );        
    return ret;
}


