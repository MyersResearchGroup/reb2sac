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


SAD_AST_ENV *GetSadAstEnv( );
RET_VAL FreeSadAstEnv( );

/* SAD_AST_TERM_LIST */
static SAD_AST_ENV _instance;

static RET_VAL _AcceptTermList( SAD_AST *ast, SAD_AST_VISITOR *visitor );

SAD_AST_ENV *GetSadAstEnv( ) {
    SAD_AST_ENV *instance = &_instance;
    
    if( _instance.termList == NULL ) {    
        if( ( _instance.termList = CreateSadAstTermList() ) == NULL ) {
            return NULL;
        }
    }

    return instance;
}

RET_VAL FreeSadAstEnv( ) {
    static SAD_AST_VISITOR visitor;
    RET_VAL ret = SUCCESS;
    SAD_AST_ENV *instance = &_instance;
    SAD_AST_TERM_LIST *termList = _instance.termList;
    
    if( visitor.VisitTermList == NULL ) {
        visitor.VisitTermList = _VisitTermListToClean;
        visitor.VisitTerm = _VisitTermToClean;
        visitor.VisitBinaryLogicalExp = _VisitBinaryExpToClean;
        visitor.VisitBinaryNumExp = _VisitBinaryExpToClean;
        visitor.VisitCompExp = _VisitBinaryExpToClean;
        visitor.VisitUnaryLogicalExp = _VisitUnaryExpToClean;
        visitor.VisitUnaryNumExp = _VisitUnaryExpToClean;
        visitor.VisitFuncExp = _VisitFuncExpToClean;
        visitor.VisitSpeciesCnt = _VisitSpeciesToClean;
        visitor.VisitSpeciesCon = _VisitSpeciesToClean;
        visitor.VisitReactionCnt = _VisitReactionToClean;
        visitor.VisitConstant = _VisitConstantToClean;
        visitor.VisitTimeVar = _VisitTimeVarToClean;        
    }
    
    ret = termList->Accept( (SAD_AST*)termList, &visitor );
    
    return ret;    
}


/* SAD_AST_TERM_LIST */
static RET_VAL _AcceptTermList( SAD_AST *ast, SAD_AST_VISITOR *visitor );

SAD_AST_TERM_LIST *CreateSadAstTermList( ) {
    SAD_AST_TERM_LIST *ast = NULL;
    
    ast = (SAD_AST_TERM_LIST*)MALLOC( sizeof(SAD_AST_TERM_LIST) );
    if( ast == NULL ) {
        return NULL;
    }
    if( ( ast->termList = CreateLinkedList() ) == NULL ) {
        return NULL;
    }
    ast->astType = TYPE_SAD_AST_TERM_LIST;
    ast->Accept = _AcceptTermList;

    return ast;
}


/* SAD_AST_TERM */
static RET_VAL _AcceptTerm( SAD_AST *ast, SAD_AST_VISITOR *visitor );

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
static RET_VAL _AcceptFuncExp( SAD_AST *ast, SAD_AST_VISITOR *visitor );

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

/* SAD_AST_BINARY_LOGICAL_EXP */
static RET_VAL _AcceptBinaryLogicalExp( SAD_AST *ast, SAD_AST_VISITOR *visitor );

SAD_AST_BINARY_EXP *CreateSadAstBinaryLogicalExp( int type, SAD_AST_EXP *left, SAD_AST_EXP *right ) {
    SAD_AST_BINARY_EXP *ast = NULL;
    
    if( !IS_LOGICAL_EXP_TYPE_SAD_AST(type) ) {
        return NULL;
    }
    
    if( ( ast = (SAD_AST_BINARY_EXP*)MALLOC( sizeof(SAD_AST_BINARY_EXP) ) ) == NULL ) {
        return NULL;
    }
    
    ast->astType = TYPE_SAD_AST_BINARY_LOGICAL_EXP;
    ast->Accept = _AcceptBinaryLogicalExp;
    ast->left = left;
    ast->right = right;
    ast->type = type;
    
    return ast;
}


/* SAD_AST_COMP_EXP */
static RET_VAL _AcceptCompExp( SAD_AST *ast, SAD_AST_VISITOR *visitor );

SAD_AST_BINARY_EXP *CreateSadAstCompExp( int type, SAD_AST_EXP *left, SAD_AST_EXP *right ) {
    SAD_AST_BINARY_EXP *ast = NULL;
    
    if( !IS_COMP_EXP_TYPE_SAD_AST(type) ) {
        return NULL;
    }
    
    if( ( ast = (SAD_AST_BINARY_EXP*)MALLOC( sizeof(SAD_AST_BINARY_EXP) ) ) == NULL ) {
        return NULL;
    }
    
    ast->astType = TYPE_SAD_AST_COMP_EXP;
    ast->Accept = _AcceptCompExp;
    ast->left = left;
    ast->right = right;
    ast->type = type;
    
    return ast;
}


/* SAD_AST_BINARY_NUM_EXP */
static RET_VAL _AcceptBinaryNumExp( SAD_AST *ast, SAD_AST_VISITOR *visitor );

SAD_AST_BINARY_EXP *CreateSadAstBinaryNumExp( int type, SAD_AST_EXP *left, SAD_AST_EXP *right ) {
    SAD_AST_BINARY_EXP *ast = NULL;
    
    if( !IS_NUM_EXP_TYPE_SAD_AST(type) ) {
        return NULL;
    }
    
    if( ( ast = (SAD_AST_BINARY_EXP*)MALLOC( sizeof(SAD_AST_BINARY_EXP) ) ) == NULL ) {
        return NULL;
    }
    
    ast->astType = TYPE_SAD_AST_BINARY_NUM_EXP;
    ast->Accept = _AcceptBinaryNumExp;
    ast->left = left;
    ast->right = right;
    ast->type = type;
    
    return ast;
}


/* SAD_AST_UNARY_NUM_EXP */
static RET_VAL _AcceptUnaryNumExp( SAD_AST *ast, SAD_AST_VISITOR *visitor );

SAD_AST_UNARY_EXP *CreateSadAstUnaryNumExp( int type, SAD_AST_EXP *exp ) {
    SAD_AST_UNARY_EXP *ast = NULL;
    
    if( !IS_NUM_EXP_TYPE_SAD_AST(type) ) {
        return NULL;
    }
    
    if( ( ast = (SAD_AST_UNARY_EXP*)MALLOC( sizeof(SAD_AST_UNARY_EXP) ) ) == NULL ) {
        return NULL;
    }
    
    ast->astType = TYPE_SAD_AST_UNARY_NUM_EXP;
    ast->Accept = _AcceptUnaryNumExp;
    ast->exp = exp;
    ast->type = type;
    
    return ast;
}

/* SAD_AST_UNARY_LOGICAL_EXP */
static RET_VAL _AcceptUnaryLogicalExp( SAD_AST *ast, SAD_AST_VISITOR *visitor );

SAD_AST_UNARY_EXP *CreateSadAstUnaryLogicalExp( int type, SAD_AST_EXP *exp ) {
    SAD_AST_UNARY_EXP *ast = NULL;
    
    if( !IS_LOGICAL_EXP_TYPE_SAD_AST(type) ) {
        return NULL;
    }
    
    if( ( ast = (SAD_AST_UNARY_EXP*)MALLOC( sizeof(SAD_AST_UNARY_EXP) ) ) == NULL ) {
        return NULL;
    }
    
    ast->astType = TYPE_SAD_AST_UNARY_LOGICAL_EXP;
    ast->Accept = _AcceptUnaryLogicalExp;
    ast->exp = exp;
    ast->type = type;
    
    return ast;
}


/* SAD_AST_SPECIES_CON */
static RET_VAL _AcceptSpeciesCon( SAD_AST *ast, SAD_AST_VISITOR *visitor );

SAD_AST_SPECIES *CreateSadAstSpeciesCon( SPECIES *species ) {
    SAD_AST_SPECIES *ast = NULL;
    
    
    if( ( ast = (SAD_AST_SPECIES*)MALLOC( sizeof(SAD_AST_SPECIES) ) ) == NULL ) {
        return NULL;
    }
    
    ast->astType = TYPE_SAD_AST_SPECIES_CON;
    ast->Accept = _AcceptSpeciesCon;
    ast->species = species;
        
    return ast;
}


/* SAD_AST_SPECIES_CNT */
static RET_VAL _AcceptSpeciesCnt( SAD_AST *ast, SAD_AST_VISITOR *visitor );

SAD_AST_SPECIES *CreateSadAstSpeciesCnt( SPECIES *species ) {
    SAD_AST_SPECIES *ast = NULL;
    
    
    if( ( ast = (SAD_AST_SPECIES*)MALLOC( sizeof(SAD_AST_SPECIES) ) ) == NULL ) {
        return NULL;
    }
    
    ast->astType = TYPE_SAD_AST_SPECIES_CNT;
    ast->Accept = _AcceptSpeciesCnt;
    ast->species = species;
        
    return ast;
}

/* SAD_AST_REACTION_CNT */
static RET_VAL _AcceptReactionCnt( SAD_AST *ast, SAD_AST_VISITOR *visitor );

SAD_AST_REACTION *CreateSadAstReactionCnt( REACTION *reaction ) {
    SAD_AST_REACTION *ast = NULL;    
    
    if( ( ast = (SAD_AST_REACTION*)MALLOC( sizeof(SAD_AST_REACTION) ) ) == NULL ) {
        return NULL;
    }
    
    ast->astType = TYPE_SAD_AST_REACTION_CNT;
    ast->Accept = _AcceptReactionCnt;
    ast->reaction = reaction;
        
    return ast;
}

/* SAD_AST_CONSTANT */
static RET_VAL _AcceptConstant( SAD_AST *ast, SAD_AST_VISITOR *visitor );

SAD_AST_CONSTANT *CreateSadAstConstant( double value ) {
    SAD_AST_CONSTANT *ast = NULL;
        
    if( ( ast = (SAD_AST_CONSTANT*)MALLOC( sizeof(SAD_AST_CONSTANT) ) ) == NULL ) {
        return NULL;
    }
    
    ast->astType = TYPE_SAD_AST_CONSTANT;
    ast->Accept = _AcceptConstant;
    ast->result = value;
        
    return ast;
}

/* SAD_AST_TIME_VAR */
static RET_VAL _AcceptTimeVar( SAD_AST *ast, SAD_AST_VISITOR *visitor );

SAD_AST_TIME_VAR *CreateSadAstTimeVar( ) {
    SAD_AST_TIME_VAR *ast = NULL;
    SAD_AST_ENV *env = NULL;
        
        
    if( ( ast = (SAD_AST_TIME_VAR*)MALLOC( sizeof(SAD_AST_TIME_VAR) ) ) == NULL ) {
        return NULL;
    }
    env = GetSadAstEnv();
    
    ast->astType = TYPE_SAD_AST_TIME_VAR;
    ast->Accept = _AcceptTimeVar;
    ast->pTime = &(env->time);
        
    return ast;
}


/** Accept methods */
static RET_VAL _AcceptTermList( SAD_AST *ast, SAD_AST_VISITOR *visitor ) {
    return visitor->VisitTermList( visitor, (SAD_AST_TERM_LIST*)ast );
}

static RET_VAL _AcceptTerm( SAD_AST *ast, SAD_AST_VISITOR *visitor ) {
    return visitor->VisitTerm( visitor, (SAD_AST_TERM*)ast );
}

static RET_VAL _AcceptFuncExp( SAD_AST *ast, SAD_AST_VISITOR *visitor ) {
    return visitor->VisitFuncExp( visitor, (SAD_AST_FUNC_EXP*)ast );
}

static RET_VAL _AcceptBinaryLogicalExp( SAD_AST *ast, SAD_AST_VISITOR *visitor ) {
    return visitor->VisitBinaryLogicalExp( visitor, (SAD_AST_BINARY_EXP*)ast );
}

static RET_VAL _AcceptCompExp( SAD_AST *ast, SAD_AST_VISITOR *visitor ) {
    return visitor->VisitCompExp( visitor, (SAD_AST_BINARY_EXP*)ast );
}

static RET_VAL _AcceptBinaryNumExp( SAD_AST *ast, SAD_AST_VISITOR *visitor ) {
    return visitor->VisitBinaryNumExp( visitor, (SAD_AST_BINARY_EXP*)ast );
}

static RET_VAL _AcceptUnaryNumExp( SAD_AST *ast, SAD_AST_VISITOR *visitor ) {
    return visitor->VisitUnaryNumExp( visitor, (SAD_AST_UNARY_EXP*)ast );
}

static RET_VAL _AcceptUnaryLogicalExp( SAD_AST *ast, SAD_AST_VISITOR *visitor ) {
    return visitor->VisitUnaryLogicalExp( visitor, (SAD_AST_UNARY_EXP*)ast );
}

static RET_VAL _AcceptSpeciesCon( SAD_AST *ast, SAD_AST_VISITOR *visitor ) {
    return visitor->VisitSpeciesCon( visitor, (SAD_AST_SPECIES*)ast );
}

static RET_VAL _AcceptSpeciesCnt( SAD_AST *ast, SAD_AST_VISITOR *visitor ) {
    return visitor->VisitSpeciesCnt( visitor, (SAD_AST_SPECIES*)ast );
}

static RET_VAL _AcceptReactionCnt( SAD_AST *ast, SAD_AST_VISITOR *visitor ) {
    return visitor->VisitReactionCnt( visitor, (SAD_AST_REACTION*)ast );
}

static RET_VAL _AcceptConstant( SAD_AST *ast, SAD_AST_VISITOR *visitor ) {
    return visitor->VisitConstant( visitor, (SAD_AST_CONSTANT*)ast );
}

static RET_VAL _AcceptTimeVar( SAD_AST *ast, SAD_AST_VISITOR *visitor ) {
    return visitor->VisitTimeVar( visitor, (SAD_AST_TIME_VAR*)ast );
}


/** clean visitor */
static RET_VAL _VisitTermListToClean( SAD_AST_VISITOR *visitor, SAD_AST_TERM_LIST *ast ) {
    SAD_AST_TERM *term = NULL;
    LINKED_LIST *list = ast->termList;
    
    ResetCurrentElement( list );
    while( ( term = (SAD_AST_TERM*)GetCurrentFromLinkedList( list ) ) != NULL ) {
        term->Accept( (SAD_AST*)term, visitor );
    }
        
    DeleteLinkedList( &(ast->termList) );
    FREE( ast );
        
    return SUCCESS;    
}

static RET_VAL _VisitTermToClean( SAD_AST_VISITOR *visitor, SAD_AST_TERM *ast ) {
    SAD_AST_EXP *condition = ast->condition;
    
    condition->Accept( (SAD_AST*)condition, visitor );
    FREE( ast->id );
    FREE( ast->desc );
    FREE( ast );
    
    return SUCCESS;
}

static RET_VAL _VisitFuncExpToClean( SAD_AST_VISITOR *visitor, SAD_AST_FUNC_EXP *ast ) {
    int i = 0;
    int argc = ast->entry->argc;    
    SAD_AST_EXP *exp = NULL;
    SAD_AST_EXP **exps = ast->argExps;
    
    for( i = 0; i < argc; i++ ) {
        exp = exps[i];
        exp->Accept( (SAD_AST*)exp, visitor );
    }
    
    FREE( exps );
    FREE( ast->values );
    FREE( ast );
    
    return SUCCESS;
}

static RET_VAL _VisitBinaryExpToClean( SAD_AST_VISITOR *visitor, SAD_AST_BINARY_EXP *ast ) {
    SAD_AST_EXP *left = ast->left;
    SAD_AST_EXP *right = ast->right;
    
    left->Accept( (SAD_AST*)left, visitor );
    right->Accept( (SAD_AST*)right, visitor );
    
    FREE( ast );
    
    return SUCCESS;
}

static RET_VAL _VisitUnaryExpToClean( SAD_AST_VISITOR *visitor, SAD_AST_UNARY_EXP *ast ) {
    SAD_AST_EXP *exp = ast->exp;

    exp->Accept( (SAD_AST*)exp, visitor );
    
    FREE( ast );
    
    return SUCCESS;
}


static RET_VAL _VisitSpeciesToClean( SAD_AST_VISITOR *visitor, SAD_AST_SPECIES *ast ) {
    
    FREE( ast );
    
    return SUCCESS;
}

static RET_VAL _VisitReactionToClean( SAD_AST_VISITOR *visitor, SAD_AST_REACTION *ast ) {
    
    FREE( ast );
    
    return SUCCESS;
}

static RET_VAL _VisitConstantToClean( SAD_AST_VISITOR *visitor, SAD_AST_CONSTANT *ast ) {
    
    FREE( ast );
    
    return SUCCESS;
}

static RET_VAL _VisitTimeVarToClean( SAD_AST_VISITOR *visitor, SAD_AST_TIME_VAR *ast ) {
    
    FREE( ast );
    
    return SUCCESS;
}


