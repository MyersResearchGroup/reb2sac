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




SAD_AST_TERM_LIST *GetSadAstTermListInstance( ) {
    static SAD_AST_TERM_LIST instance = {
    };
    
    if( instance.termList == NULL ) {
    
        if( ( instance.termList = CreateLinkedList() ) == NULL ) {
            return NULL;
        }
    }

    return &instance;
}

SAD_AST_TERM *CreateSadAstTerm( char *id, char *desc, SAD_AST_EXP *condition ) {
}

SAD_AST_FUNC_EXP *CreateSadAstFuncExp( char *name, LINKED_LIST *expList ) {
}

SAD_AST_BINARY_LOGICAL_EXP *CreateSadAstBinaryLogicalExp( int type, SAD_AST_EXP *left, SAD_AST_EXP *right ) {
}

SAD_AST_UNARY_LOGICAL_EXP *CreateSadAstBinaryLogicalExp( int type, SAD_AST_EXP *exp ) {
}

SAD_AST_COMP_EXP *CreateSadAstCompExp( int type, SAD_AST_EXP *left, SAD_AST_EXP *right ) {
}

SAD_AST_BINARY_NUM_EXP *CreateSadAstBinaryNumExp( int type, SAD_AST_EXP *left, SAD_AST_EXP *right ) {
}

SAD_AST_UNARY_NUM_EXP *CreateSadAstBinaryNumExp( int type, SAD_AST_EXP *exp ) {
}

SAD_AST_SPECIES_CON *CreateSadAstSpeciesCon( SPECIES *species ) {
}

SAD_AST_SPECIES_CNT *CreateSadAstSpeciesCnt( SPECIES *species ) {
}

SAD_AST_REACTION_CNT *CreateSadAstReactionCnt( REACTION *reaction ) {
}

SAD_AST_CONSTANT *CreateSadAstConstant( double value ) {
}

SAD_AST_TIME_VAR *CreateSadAstTimeVar( ) {
}
