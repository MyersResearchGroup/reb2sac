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
#include "sad_ast_pretty_printer.h"

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



RET_VAL PrettyPrintSadAst( FILE *file, SAD_AST *ast ) {
    static SAD_AST_VISITOR visitor;
    
    if( visitor.VisitTermList == NULL ) {
        visitor._internal1 = (CADDR_T)file;
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
    
    ast->Accept( ast, &visitor );
    
    return SUCCESS;
}




static RET_VAL _VisitTermList( SAD_AST_VISITOR *visitor, SAD_AST_TERM_LIST *ast ) {
    SAD_AST_TERM *term = NULL;
    LINKED_LIST *list = ast->terms;
    
    ResetCurrentElement( list );
    while( ( term = (SAD_AST_TERM*)GetNextFromLinkedList( list ) ) != NULL ) {
        term->Accept( (SAD_AST*)term, visitor );
    }
        
    return SUCCESS;    
    
}


static RET_VAL _VisitTerm( SAD_AST_VISITOR *visitor, SAD_AST_TERM *ast ) {
    FILE *file = (FILE*)(visitor->_internal1);
    SAD_AST_EXP *condition = ast->condition;
    
    fprintf( file, NEW_LINE );
    fprintf( file, "term %s {" NEW_LINE, ast->id );
    fprintf( file, "\tdesc \"%s\";" NEW_LINE, ast->desc );
    fprintf( file, "\tcond ");
    condition->Accept( (SAD_AST*)condition, visitor );
    fprintf( file, ";" NEW_LINE );
    fprintf( file, "}" NEW_LINE );
    fprintf( file, NEW_LINE );
    
    return SUCCESS;
}


static RET_VAL _VisitCompExp( SAD_AST_VISITOR *visitor, SAD_AST_BINARY_EXP *ast ) {
    FILE *file = (FILE*)(visitor->_internal1);
    char *op = NULL;
    SAD_AST_EXP *left = ast->left;
    SAD_AST_EXP *right = ast->right;
    
    fprintf( file, "( ");
    left->Accept( (SAD_AST*)left, visitor );
    
    switch( ast->type ) {
        case COMP_EXP_TYPE_SAD_AST_EQ:
            op = "=";
        break;    
        case COMP_EXP_TYPE_SAD_AST_LE:    
            op = "<=";
        break;    
        case COMP_EXP_TYPE_SAD_AST_LT:    
            op = "<";
        break;    
        case COMP_EXP_TYPE_SAD_AST_GE:    
            op = ">=";
        break;    
        case COMP_EXP_TYPE_SAD_AST_GT:        
            op = ">";
        break;
        default:        
            op = "?";
        break;
    }
    fprintf( file, " %s ", op );
    
    right->Accept( (SAD_AST*)right, visitor );
    fprintf( file, " )");
    
    return SUCCESS;
}


static RET_VAL _VisitBinaryNumExp( SAD_AST_VISITOR *visitor, SAD_AST_BINARY_EXP *ast ) {
    FILE *file = (FILE*)(visitor->_internal1);
    char *op = NULL;
    SAD_AST_EXP *left = ast->left;
    SAD_AST_EXP *right = ast->right;
    
    fprintf( file, "( ");
    left->Accept( (SAD_AST*)left, visitor );
    
    switch( ast->type ) {
        case NUM_EXP_TYPE_SAD_AST_PLUS:
            op = "+";
            break;    
        case NUM_EXP_TYPE_SAD_AST_MINUS:    
            op = "-";
            break;    
        case NUM_EXP_TYPE_SAD_AST_TIMES:    
            op = "*";
            break;    
        case NUM_EXP_TYPE_SAD_AST_DIV:    
            op = "/";
            break;    
        default:        
            op = "?";
            break;
    }
    fprintf( file, " %s ", op );
    
    right->Accept( (SAD_AST*)right, visitor );
    fprintf( file, " )");
    
    return SUCCESS;
}


static RET_VAL _VisitBinaryLogicalExp( SAD_AST_VISITOR *visitor, SAD_AST_BINARY_EXP *ast ) {
    FILE *file = (FILE*)(visitor->_internal1);
    char *op = NULL;
    SAD_AST_EXP *left = ast->left;
    SAD_AST_EXP *right = ast->right;
    
    fprintf( file, "( ");
    left->Accept( (SAD_AST*)left, visitor );
    
    switch( ast->type ) {
        case LOGICAL_EXP_TYPE_SAD_AST_AND:
            op = "&&";
            break;    
        case LOGICAL_EXP_TYPE_SAD_AST_OR:    
            op = "||";
            break;    
        default:        
            op = "?";
            break;
    }
    fprintf( file, " %s ", op );
    
    right->Accept( (SAD_AST*)right, visitor );
    fprintf( file, " )");
    
    return SUCCESS;
}


static RET_VAL _VisitUnaryNumExp( SAD_AST_VISITOR *visitor, SAD_AST_UNARY_EXP *ast ) {
    FILE *file = (FILE*)(visitor->_internal1);
    char *op = NULL;
    SAD_AST_EXP *exp = ast->exp;
    
    switch( ast->type ) {
        case NUM_EXP_TYPE_SAD_AST_UMINUS:
            op = "-";
            break;    
        default:        
            op = "?";
            break;
    }
    fprintf( file, " %s( ", op );
    
    exp->Accept( (SAD_AST*)exp, visitor );
    fprintf( file, " )" );
    
    return SUCCESS;
}


static RET_VAL _VisitUnaryLogicalExp( SAD_AST_VISITOR *visitor, SAD_AST_UNARY_EXP *ast ) {
    FILE *file = (FILE*)(visitor->_internal1);
    char *op = NULL;
    SAD_AST_EXP *exp = ast->exp;
    
    switch( ast->type ) {
        case LOGICAL_EXP_TYPE_SAD_AST_NOT:
            op = "!";
            break;    
        default:        
            op = "?";
            break;
    }
    fprintf( file, " %s( ", op );
    
    exp->Accept( (SAD_AST*)exp, visitor );
    fprintf( file, " )" );
    
    return SUCCESS;
}


static RET_VAL _VisitFuncExp( SAD_AST_VISITOR *visitor, SAD_AST_FUNC_EXP *ast ) {
    FILE *file = (FILE*)(visitor->_internal1);
    int i = 0;
    SAD_AST_FUNC_REGISTRY_ENTRY *entry = ast->entry;
    int argc = ast->entry->argc;    
    SAD_AST_EXP *exp = NULL;
    SAD_AST_EXP **exps = ast->argExps;
    
    fprintf( file, " %s( ", entry->funcID );    
    for( i = 0; i < argc; i++ ) {
        exp = exps[i];
        exp->Accept( (SAD_AST*)exp, visitor );
    }
    fprintf( file, " )" );
    
    return SUCCESS;
}


static RET_VAL _VisitSpeciesCon( SAD_AST_VISITOR *visitor, SAD_AST_SPECIES *ast ) {
    FILE *file = (FILE*)(visitor->_internal1);
    
    fprintf( file, "\%$%s", GetCharArrayOfString( GetSpeciesNodeName( ast->species ) ) );    

    return SUCCESS;
}


static RET_VAL _VisitSpeciesCnt( SAD_AST_VISITOR *visitor, SAD_AST_SPECIES *ast ) {
    FILE *file = (FILE*)(visitor->_internal1);
    
    fprintf( file, "#$%s", GetCharArrayOfString( GetSpeciesNodeName( ast->species ) ) );    

    return SUCCESS;
}


static RET_VAL _VisitReactionCnt( SAD_AST_VISITOR *visitor, SAD_AST_REACTION *ast ) {
    FILE *file = (FILE*)(visitor->_internal1);
    
    fprintf( file, "#$%s", GetCharArrayOfString( GetReactionNodeName( ast->reaction ) ) );    

    return SUCCESS;
}


static RET_VAL _VisitConstant( SAD_AST_VISITOR *visitor, SAD_AST_CONSTANT *ast ) {
    FILE *file = (FILE*)(visitor->_internal1);
    
    fprintf( file, "%g", ast->result );    

    return SUCCESS;
}


static RET_VAL _VisitTimeVar( SAD_AST_VISITOR *visitor, SAD_AST_TIME_VAR *ast ) {
    FILE *file = (FILE*)(visitor->_internal1);
    
    fprintf( file, "t" );    

    return SUCCESS;
}




