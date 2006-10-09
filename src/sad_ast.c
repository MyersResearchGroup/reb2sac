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


static SAD_AST_TERM_LIST _instance = {
    
};


SAD_AST_TERM_LIST *GetSadAstTermListInstance( ) {
    SAD_AST_TERM_LIST *instance = &_instance;
    
    if( _instance.termList == NULL ) {
    
        if( ( _instance.termList = CreateLinkedList() ) == NULL ) {
            return NULL;
        }
    }

    return instance;
}

RET_VAL FreeSadAstTermListInstance( ) {
}

SAD_AST_TERM *CreateSadAstTerm( char *id, char *desc, SAD_AST_EXP *condition ) {
    return NULL;
}

SAD_AST_FUNC_EXP *CreateSadAstFuncExp( char *name, LINKED_LIST *expList ) {
    return NULL;
}

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

