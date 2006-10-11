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
#include "sad_ast_exp_evaluator.h"

static RET_VAL _VisitTermList( SAD_AST_VISITOR *visitor, SAD_AST_TERM_LIST *ast );
static RET_VAL _VisitTerm( SAD_AST_VISITOR *visitor, SAD_AST_TERM *ast );
static RET_VAL _VisitCompExp( SAD_AST_VISITOR *visitor, SAD_AST_BINARY_EXP *ast );
static RET_VAL _VisitBinaryNumExp( SAD_AST_VISITOR *visitor, SAD_AST_BINARY_EXP *ast );
static RET_VAL _VisitBinaryLogicalExp( SAD_AST_VISITOR *visitor, SAD_AST_BINARY_EXP *ast );
static RET_VAL _VisitUnaryNumExp( SAD_AST_VISITOR *visitor, SAD_AST_UNARY_EXP *ast );
static RET_VAL _VisitUnaryLogicalExp( SAD_AST_VISITOR *visitor, SAD_AST_UNARY_EXP *ast );
static RET_VAL _VisitFuncExp( SAD_AST_VISITOR *visitor, SAD_AST_FUNC_EXP *ast );
static RET_VAL _VisitSpeciesCon( SAD_AST_VISITOR *visitor, SAD_AST_SPECIES *ast );
static RET_VAL _VisitSpeciesCnt( SAD_AST_VISITOR *visitor, SAD_AST_SPECIES *ast );
static RET_VAL _VisitReactionCnt( SAD_AST_VISITOR *visitor, SAD_AST_REACTION *ast );
static RET_VAL _VisitConstant( SAD_AST_VISITOR *visitor, SAD_AST_CONSTANT *ast );
static RET_VAL _VisitTimeVar( SAD_AST_VISITOR *visitor, SAD_AST_TIME_VAR *ast );



double EvaluateSadAstExp( SAD_AST_EXP *ast ) {
    static SAD_AST_VISITOR visitor;
    
    if( visitor.VisitTermList == NULL ) {
        visitor.VisitTermList = _VisitTermList;
        visitor.VisitTerm = _VisitTerm;
        visitor.VisitCompExp = _VisitCompExp;
        visitor.VisitBinaryNumExp = _VisitBinaryNumExp;
        visitor.VisitBinaryLogicalExp = _VisitBinaryLogicalExp;
        visitor.VisitUnaryNumExp = _VisitUnaryNumExp;
        visitor.VisitUnaryLogicalExp = _VisitUnaryLogicalExp;
        visitor.VisitUnaryNumExp = _VisitUnaryNumExp;
        visitor.VisitFuncExp = _VisitFuncExp;
        visitor.VisitSpeciesCon = _VisitSpeciesCon;
        visitor.VisitSpeciesCnt = _VisitSpeciesCnt;
        visitor.VisitReactionCnt = _VisitReactionCnt;
        visitor.VisitConstant = _VisitConstant;
        visitor.VisitTimeVar = _VisitTimeVar;
    }
    
    if( IS_FAILED( ( ast->Accept( (SAD_AST*)ast, &visitor ) ) ) ) {
        return 0.0 / 0.0;
    }
    
    return ast->result;
}




static RET_VAL _VisitTermList( SAD_AST_VISITOR *visitor, SAD_AST_TERM_LIST *ast ) {
    return FAILING;        
}


static RET_VAL _VisitTerm( SAD_AST_VISITOR *visitor, SAD_AST_TERM *ast ) {
    return FAILING;        
}


static RET_VAL _VisitCompExp( SAD_AST_VISITOR *visitor, SAD_AST_BINARY_EXP *ast ) {
    SAD_AST_EXP *left = ast->left;
    SAD_AST_EXP *right = ast->right;
    
    left->Accept( (SAD_AST*)left, visitor );
    right->Accept( (SAD_AST*)right, visitor );
    
    switch( ast->type ) {
        case COMP_EXP_TYPE_SAD_AST_EQ:
            ast->result = ( IS_REAL_EQUAL( left->result, right->result ) ? 1.0 : 0.0 );
            break;    
        case COMP_EXP_TYPE_SAD_AST_LE:    
            ast->result = ( ( left->result <= right->result ) ? 1.0 : 0.0 );
            break;    
        case COMP_EXP_TYPE_SAD_AST_LT:    
            ast->result = ( ( left->result < right->result ) ? 1.0 : 0.0 );
            break;    
        case COMP_EXP_TYPE_SAD_AST_GE:    
            ast->result = ( ( left->result >= right->result ) ? 1.0 : 0.0 );
            break;    
        case COMP_EXP_TYPE_SAD_AST_GT:        
            ast->result = ( ( left->result > right->result ) ? 1.0 : 0.0 );
            break;
        default:        
            ast->result = 0.0 / 0.0;
            break;
    }
    
    return SUCCESS;
}


static RET_VAL _VisitBinaryNumExp( SAD_AST_VISITOR *visitor, SAD_AST_BINARY_EXP *ast ) {
    SAD_AST_EXP *left = ast->left;
    SAD_AST_EXP *right = ast->right;
    
    left->Accept( (SAD_AST*)left, visitor );
    right->Accept( (SAD_AST*)right, visitor );
    
    switch( ast->type ) {
        case NUM_EXP_TYPE_SAD_AST_PLUS:
            ast->result = left->result + right->result;
            break;    
        case NUM_EXP_TYPE_SAD_AST_MINUS:    
            ast->result = left->result - right->result;
            break;    
        case NUM_EXP_TYPE_SAD_AST_TIMES:    
            ast->result = left->result * right->result;
            break;    
        case NUM_EXP_TYPE_SAD_AST_DIV:    
            ast->result = left->result / right->result;
            break;    
        default:        
            ast->result = 0.0 / 0.0;
            break;
    }
        
    return SUCCESS;
}


static RET_VAL _VisitBinaryLogicalExp( SAD_AST_VISITOR *visitor, SAD_AST_BINARY_EXP *ast ) {
    SAD_AST_EXP *left = ast->left;
    SAD_AST_EXP *right = ast->right;
    
    left->Accept( (SAD_AST*)left, visitor );
    right->Accept( (SAD_AST*)right, visitor );
    
    switch( ast->type ) {
        case LOGICAL_EXP_TYPE_SAD_AST_AND:
            ast->result = ( (left->result * right->result > 0.5) ? 1.0 : 0.0 );
            break;    
        case LOGICAL_EXP_TYPE_SAD_AST_OR:    
            ast->result = ( (left->result + right->result > 0.5) ? 1.0 : 0.0 );
            break;    
        default:        
            ast->result = 0.0 / 0.0;
            break;
    }
    
    return SUCCESS;
}


static RET_VAL _VisitUnaryNumExp( SAD_AST_VISITOR *visitor, SAD_AST_UNARY_EXP *ast ) {
    FILE *file = (FILE*)(visitor->_internal1);
    char *op = NULL;
    SAD_AST_EXP *exp = ast->exp;
    
    exp->Accept( (SAD_AST*)exp, visitor );    
    switch( ast->type ) {
        case NUM_EXP_TYPE_SAD_AST_UMINUS:
            ast->result = -(exp->result);
            break;    
        default:        
            ast->result = 0.0 / 0.0;
            break;
    }
    
    return SUCCESS;
}


static RET_VAL _VisitUnaryLogicalExp( SAD_AST_VISITOR *visitor, SAD_AST_UNARY_EXP *ast ) {
    SAD_AST_EXP *exp = ast->exp;
    
    exp->Accept( (SAD_AST*)exp, visitor );
    switch( ast->type ) {
        case LOGICAL_EXP_TYPE_SAD_AST_NOT:
            ast->result = ( (exp->result > 0.5) ? 0.0 : 1.0 );
            break;    
        default:        
            ast->result = 0.0 / 0.0;
            break;
    }
    
    return SUCCESS;
}


static RET_VAL _VisitFuncExp( SAD_AST_VISITOR *visitor, SAD_AST_FUNC_EXP *ast ) {
    int i = 0;
    SAD_AST_FUNC_REGISTRY_ENTRY *entry = ast->entry;
    double **pValues = ast->values;
    int argc = ast->entry->argc;    
    SAD_AST_EXP *exp = NULL;
    SAD_AST_EXP **exps = ast->argExps;
    
    for( i = 0; i < argc; i++ ) {
        exp = exps[i];
        exp->Accept( (SAD_AST*)exp, visitor );
    }
    ast->result = entry->func( pValues );
    
    return SUCCESS;
}


static RET_VAL _VisitSpeciesCon( SAD_AST_VISITOR *visitor, SAD_AST_SPECIES *ast ) {
    
    ast->result = GetConcentrationInSpeciesNode( ast->species );    

    return SUCCESS;
}


static RET_VAL _VisitSpeciesCnt( SAD_AST_VISITOR *visitor, SAD_AST_SPECIES *ast ) {
    
    ast->result = GetAmountInSpeciesNode( ast->species );    

    return SUCCESS;
}


static RET_VAL _VisitReactionCnt( SAD_AST_VISITOR *visitor, SAD_AST_REACTION *ast ) {
    
    ast->result = GetReactionFireCount( ast->reaction );    

    return SUCCESS;
}


static RET_VAL _VisitConstant( SAD_AST_VISITOR *visitor, SAD_AST_CONSTANT *ast ) {

    return SUCCESS;
}


static RET_VAL _VisitTimeVar( SAD_AST_VISITOR *visitor, SAD_AST_TIME_VAR *ast ) {

    ast->result = *(ast->pTime);
    
    return SUCCESS;
}



