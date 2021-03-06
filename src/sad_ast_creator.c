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
#include "sad_ast_creator.h"

extern int yyparse();
extern FILE *yyin;

SAD_AST_ENV *CreateSadAstEnv( FILE *file, int speciesSize, SPECIES **speciesArray, int reactionSize, REACTION **reactionArray ) {
    SAD_AST_ENV *env = NULL;
    
    if( ( env = GetSadAstEnv() ) == NULL ) {
        return NULL;
    }
     
    env->speciesSize = speciesSize;
    env->speciesArray = speciesArray;
    env->reactionSize = reactionSize;
    env->reactionArray = reactionArray;
    
    yyin = file;
    if( yyparse() != 0 ) {
        return NULL;
    }             
    
    return env;
}

SAD_AST_ENV *CreateEmptySadAstEnv( int speciesSize, SPECIES **speciesArray, int reactionSize, REACTION **reactionArray ) {
    SAD_AST_ENV *env = NULL;
    
    if( ( env = GetSadAstEnv() ) == NULL ) {
        return NULL;
    }
     
    env->speciesSize = speciesSize;
    env->speciesArray = speciesArray;
    env->reactionSize = reactionSize;
    env->reactionArray = reactionArray;
    
    return env;
}

