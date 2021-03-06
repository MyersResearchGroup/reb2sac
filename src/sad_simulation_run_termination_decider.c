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
#include "sad_simulation_run_termination_decider.h"
#include "sad_ast_creator.h"
#include "sad_ast_exp_evaluator.h"
#include "sad_ast_pretty_printer.h"


static BOOL _IsTerminationConditionMet( SAD_SIMULATION_RUN_TERMINATION_DECIDER *decider, REACTION *reaction, double time );
static RET_VAL _Destroy( SAD_SIMULATION_RUN_TERMINATION_DECIDER *decider );
static RET_VAL _Report( SAD_SIMULATION_RUN_TERMINATION_DECIDER *decider, FILE *file );
static FILE *_CreateSadFile( REB2SAC_PROPERTIES *properties );    
static BOOL _EvaluateTermConditions( SAD_AST_TERM_LIST *ast );



DLLSCOPE SIMULATION_RUN_TERMINATION_DECIDER * STDCALL CreateSadSimulationRunTerminationDecider( 
        BACK_END_PROCESSOR *backend, SPECIES **speciesArray, int size, 
        REACTION **reactionArray, int reactionSize, double timeLimit ) {
    
    SAD_SIMULATION_RUN_TERMINATION_DECIDER *decider = NULL;
    SAD_AST_ENV *env = NULL;
    REB2SAC_PROPERTIES *properties = backend->record->properties;
    FILE *sadFile = NULL;
    
    START_FUNCTION("CreateSadSimulationRunTerminationDecider");

    if( ( decider = (SAD_SIMULATION_RUN_TERMINATION_DECIDER*)MALLOC( sizeof(SAD_SIMULATION_RUN_TERMINATION_DECIDER) ) ) == NULL ) {
        TRACE_0("failed to allocate memory for SAD_SIMULATION_RUN_TERMINATION_DECIDER");
        return NULL;
    }
    decider->speciesArray = speciesArray;
    decider->size = size;
    decider->timeLimit = timeLimit;
    
    decider->IsTerminationConditionMet = 
            (BOOL(*)(SIMULATION_RUN_TERMINATION_DECIDER *, REACTION *, double))_IsTerminationConditionMet;        
    decider->Destroy = (RET_VAL(*)(SIMULATION_RUN_TERMINATION_DECIDER *))_Destroy;
    decider->Report = (RET_VAL(*)(SIMULATION_RUN_TERMINATION_DECIDER *, FILE *))_Report;
    
    
    sadFile = _CreateSadFile( properties );
    if( sadFile == NULL ) {
        env = CreateEmptySadAstEnv( size, speciesArray, reactionSize, reactionArray );
        if( env == NULL ) {
            TRACE_0("failed to create sad empty ast environment");
            return NULL;    
        }
    }
    else {
        env = CreateSadAstEnv( sadFile, size, speciesArray, reactionSize, reactionArray  );
        if( env == NULL ) {
            TRACE_0("failed to create sad ast environment");
            return NULL;    
        }
    } 
    decider->env = env;
    decider->timeLimitCount = 0;
    decider->totalCount = 0;
         
    /* testing 
    PrettyPrintSadAst( stdout, (SAD_AST*)(env->termList) );    
    exit(0);
     testing */
    
    
    END_FUNCTION("CreateSadSimulationRunTerminationDecider", SUCCESS );
    return (SIMULATION_RUN_TERMINATION_DECIDER*)decider;
}

static BOOL _IsTerminationConditionMet( SAD_SIMULATION_RUN_TERMINATION_DECIDER *decider, REACTION *reaction, double time ) {
    int i = 0;
    int size = 0;
    double quantity = 0;
    SAD_AST_ENV *env = decider->env;

    env->time = time;
    if( reaction != NULL ) {
        IncrementReactionFireCount( reaction ); 
    }
    
    if( time >= decider->timeLimit ) {
        (decider->timeLimitCount)++;
    }
    else if( !_EvaluateTermConditions( env->termList ) ) {
        return FALSE;        
    }
            
    (decider->totalCount)++;        
    return TRUE;
}

static RET_VAL _Report( SAD_SIMULATION_RUN_TERMINATION_DECIDER *decider, FILE *file ) {
    RET_VAL ret = SUCCESS;
    SAD_AST_ENV *env = decider->env;
    LINKED_LIST *terms = NULL;
    SAD_AST_TERM *term = NULL;
     
    terms = env->termList->terms;
    fprintf( file, "#total time-limit");
    
    ResetCurrentElement( terms );    
    while( ( term = (SAD_AST_TERM*)GetNextFromLinkedList( terms ) ) != NULL ) {
        fprintf( file, " %s", term->id );    
    }
    fprintf( file, NEW_LINE );
    
    fprintf( file, "%i %i", decider->totalCount, decider->timeLimitCount );
    ResetCurrentElement( terms );    
    while( ( term = (SAD_AST_TERM*)GetNextFromLinkedList( terms ) ) != NULL ) {
        fprintf( file, " %i", term->count );    
    }
    fprintf( file, NEW_LINE );
        
    return ret;
}


static RET_VAL _Destroy( SAD_SIMULATION_RUN_TERMINATION_DECIDER *decider ) {
    RET_VAL ret = SUCCESS;
    
    
    return ret;
}

 
 
static FILE *_CreateSadFile( REB2SAC_PROPERTIES *properties ) {
    FILE *file;
    char *valueString;

    if( ( valueString = properties->GetProperty( properties, COMPUTATION_ANALYSIS_SAD_PATH_KEY ) ) == NULL ) {
        return NULL;
    }    
    file = fopen( valueString, "r" );
    return file;
}

static BOOL _EvaluateTermConditions( SAD_AST_TERM_LIST *ast ) {
    BOOL condition = FALSE;
    SAD_AST_TERM *term = NULL;    
    LINKED_LIST *list = ast->terms;
    
    ResetCurrentElement( list );
    while( ( term = (SAD_AST_TERM*)GetNextFromLinkedList( list ) ) != NULL ) {
        if( EvaluateSadAstBoolExp( (SAD_AST_EXP*)(term->condition) ) ) {
            (term->count)++;
            condition = TRUE;
        } 
    }
        
    return condition;        
}

 
 
 
 
