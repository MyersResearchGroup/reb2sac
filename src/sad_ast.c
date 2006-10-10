/***************************************************************************
 *   Copyright (C) 2006 by Hiroyuki Kuwahara   *
 *   kuwahara@cs.utah.edu   *
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
#include "sad_ast.h"

static RET_VAL _VisitTermListToClean( SAD_AST_VISITOR *visitor, SAD_AST_TERM_LIST *ast );
static RET_VAL _VisitTermToClean( SAD_AST_VISITOR *visitor, SAD_AST_TERM *ast );
static RET_VAL _VisitBinaryExpToClean( SAD_AST_VISITOR *visitor, SAD_AST_BINARY_EXP *ast );
static RET_VAL _VisitUnaryExpToClean( SAD_AST_VISITOR *visitor, SAD_AST_UNARY_EXP *ast );
static RET_VAL _VisitFuncExpToClean( SAD_AST_VISITOR *visitor, SAD_AST_FUNC_EXP *ast );
static RET_VAL _VisitSpeciesToClean( SAD_AST_VISITOR *visitor, SAD_AST_SPECIES *ast );
static RET_VAL _VisitReactionToClean( SAD_AST_VISITOR *visitor, SAD_AST_REACTION *ast );
static RET_VAL _VisitConstantToClean( SAD_AST_VISITOR *visitor, SAD_AST_CONSTANT *ast );
static RET_VAL _VisitTimeVarToClean( SAD_AST_VISITOR *visitor, SAD_AST_TIME_VAR *ast );



/* SAD_AST_TERM_LIST */
static SAD_AST_TERM_LIST _instance;

static RET_VAL _AcceptTermList( SAD_AST *ast, SAD_AST_VISITOR *visitor ) {
    return visitor->VisitTermList( visitor, (SAD_AST_TERM_LIST*)ast );
}

static RET_VAL _VisitTermListToClean( SAD_AST_VISITOR *visitor, SAD_AST_TERM_LIST *ast ) {
    SAD_AST_TERM *term = NULL;
    LINKED_LIST *list = ast->termList;
    
    ResetCurrentElement( list );
    while( ( term = (SAD_AST_TERM*)GetCurrentFromLinkedList( list ) ) != NULL ) {
        term->Accept( term, visitor );
    }
        
    DeleteLinkedList( &(_instance.termList) );
    
    return SUCCESS;    
}

SAD_AST_TERM_LIST *GetSadAstTermListInstance( ) {
    SAD_AST_TERM_LIST *instance = &_instance;
    
    if( _instance.termList == NULL ) {    
        if( ( _instance.termList = CreateLinkedList() ) == NULL ) {
            return NULL;
        }
        _instance.astType = TYPE_SAD_AST_TERM_LIST;
        _instance.Accept = _AcceptTermList;
    }

    return instance;
}

RET_VAL FreeSadAstTermListInstance( ) {
    static SAD_AST_VISITOR visitor;
    RET_VAL ret = SUCCESS;
    SAD_AST_TERM_LIST *instance = &_instance;
    
    
    if( visitor.VisitTermList == NULL ) {
        visitor.VisitTermList = _VisitTermListToClean;
        visitor.VisitTerm = _VisitTermToClean;
        visitor.VisitBinaryExp = _VisitBinaryExpToClean;
        visitor.VisitUnaryExp = _VisitUnaryExpToClean;
        visitor.VisitFuncExp = _VisitFuncExpToClean;
        visitor.VisitSpecies = _VisitSpeciesToClean;
        visitor.VisitReaction = _VisitReactionToClean;
        visitor.VisitConstant = _VisitConstantToClean;
        visitor.VisitTimeVar = _VisitTimeVarToClean;        
    }
    
    ret = instance->Accept( instance, &visitor );
    
    return ret;    
}





/* SAD_AST_TERM */
static RET_VAL _AcceptTerm( SAD_AST *ast, SAD_AST_VISITOR *visitor ) {
    return visitor->VisitTerm( visitor, (SAD_AST_TERM*)ast );
}

static RET_VAL _VisitTermToClean( SAD_AST_VISITOR *visitor, SAD_AST_TERM *ast ) {
    SAD_AST_EXP *condition = ast->condition;
    
    condition->Accept( condition, visitor );
    FREE( ast->id );
    FREE( ast->desc );
    FREE( ast );
    
    return SUCCESS;
}

SAD_AST_TERM *CreateSadAstTerm( char *id, char *desc, SAD_AST_EXP *condition ) {
    SAD_AST_TERM *ast = NULL;
    
    ast = (SAD_AST_TERM*)MALLOC( sizeof(SAD_AST_TERM) );
    if( ast == NULL ) {
        return NULL;
    }
    
    if( ( ast->id = (char*)MALLOC( strlen(id) + 1 ) ) == NULL ) {
        return NULL;
    }
    strcpy( ast->id, id );
    
    if( ( ast->desc = (char*)MALLOC( strlen(desc) + 1 ) ) == NULL ) {
        return NULL;
    }
    strcpy( ast->desc, desc );
    
    ast->condition = condition;
    ast->Accept = _AcceptTerm;
    ast->astType = TYPE_SAD_AST_TERM;
    
    return ast;
}


/* SAD_AST_FUNC_EXP */
static RET_VAL _AcceptFuncExp( SAD_AST *ast, SAD_AST_VISITOR *visitor ) {
    return visitor->VisitFuncExp( visitor, (SAD_AST_FUNC_EXP*)ast );
}

static RET_VAL _VisitFuncExpToClean( SAD_AST_VISITOR *visitor, SAD_AST_FUNC_EXP *ast ) {
    int i = 0;
    int argc = ast->entry->argc;    
    SAD_AST_EXP *exp = NULL;
    SAD_AST_EXP **exps = ast->argExps;
    
    for( i = 0; i < argc; i++ ) {
        exp = exps[i];
        exp->Accept( exp, visitor );
    }
    
    FREE( exps );
    FREE( ast->values );
    FREE( ast );
    
    return SUCCESS;
}

SAD_AST_FUNC_EXP *CreateSadAstFuncExp( char *name, LINKED_LIST *expList ) {
    SAD_AST_FUNC_EXP *ast = NULL;
    int i = 0;
    int argc = 0;
    double **values = NULL;
    SAD_AST_EXP *exp = NULL;    
    SAD_AST_EXP **argExps = NULL;    
    SAD_AST_FUNC_REGISTRY_ENTRY *entry = NULL;    
    SAD_AST_FUNC_REGISTRY *registry = NULL;    
    
    registry = GetSadAstFuncRegistryInstance();
    entry = registry->GetEntry( registry, name );
    if( entry == NULL ) {
        return NULL;
    } 
    argc = entry->argc;
    if( argc != GetLinkedListSize( expList ) ) {
        return NULL;
    }
    
    if( ( values = (double**)MALLOC( argc * sizeof(double*) ) ) == NULL ) {
        return NULL;    
    }
    if( ( argExps = (SAD_AST_EXP**)MALLOC( argc * sizeof(SAD_AST_EXP*) ) ) == NULL ) {
        return NULL;    
    } 
    if( ( ast = (SAD_AST_FUNC_EXP*)MALLOC( sizeof(SAD_AST_FUNC_EXP) ) ) == NULL ) {
        return NULL;    
    } 
    
    ast->argExps = argExps;
    ast->values = values;
    ast->entry = entry;
    ast->astType = TYPE_SAD_AST_FUNC_EXP;
    ast->Accept = _AcceptFuncExp;
    
    ResetCurrentElement( expList );
    for( i = 0; i < argc; i++ ) {
        exp = (SAD_AST_EXP*)GetCurrentFromLinkedList( expList );
        values[i] = &(exp->result);
        argExps[i] = exp;
    }
    
    return ast;
}

/** from here */
/* SAD_AST_BINARY_EXP */
SAD_AST_BINARY_EXP *CreateSadAstBinaryLogicalExp( int type, SAD_AST_EXP *left, SAD_AST_EXP *right ) {
    return NULL;
}

SAD_AST_UNARY_EXP *CreateSadAstUnaryLogicalExp( int type, SAD_AST_EXP *exp ) {
    return NULL;
}

SAD_AST_BINARY_EXP *CreateSadAstCompExp( int type, SAD_AST_EXP *left, SAD_AST_EXP *right ) {
    return NULL;
}

SAD_AST_BINARY_EXP *CreateSadAstBinaryNumExp( int type, SAD_AST_EXP *left, SAD_AST_EXP *right ) {
    return NULL;
}

SAD_AST_UNARY_EXP *CreateSadAstUnaryNumExp( int type, SAD_AST_EXP *exp ) {
    return NULL;
}

SAD_AST_SPECIES *CreateSadAstSpeciesCon( SPECIES *species ) {
    return NULL;
}

SAD_AST_SPECIES *CreateSadAstSpeciesCnt( SPECIES *species ) {
    return NULL;
}

SAD_AST_REACTION *CreateSadAstReactionCnt( REACTION *reaction ) {
    return NULL;
}

SAD_AST_CONSTANT *CreateSadAstConstant( double value ) {
    return NULL;
}

SAD_AST_TIME_VAR *CreateSadAstTimeVar( ) {
    return NULL;
}

double EvaluateSadAstExp( SAD_AST_EXP *exp ) {
    return 0.0;
}

RET_VAL PrettyPrintSadAst( SAD_AST *exp ) {

    return SUCCESS;
}

