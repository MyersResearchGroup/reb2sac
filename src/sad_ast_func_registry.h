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
#if !defined(HAVE_SAD_AST_FUNC_REGISTRY)
#define HAVE_SAD_AST_FUNC_REGISTRY

#include "common.h"
#include "hash_table.h"


BEGIN_C_NAMESPACE

typedef struct {
    char *funcID;
    int argc;
    double (*func)( double *argv[] );
} SAD_AST_FUNC_REGISTRY_ENTRY;


struct _SAD_AST_FUNC_REGISTRY;
typedef struct _SAD_AST_FUNC_REGISTRY SAD_AST_FUNC_REGISTRY;

struct _SAD_AST_FUNC_REGISTRY {
    HASH_TABLE *table;
    
    SAD_AST_FUNC_REGISTRY_ENTRY *(*GetEntry)( SAD_AST_FUNC_REGISTRY *registry, char *id );
    RET_VAL (*AddEntry)( SAD_AST_FUNC_REGISTRY *registry, char *id, int argc, double (*func)( double *argv[] ) );    
};


SAD_AST_FUNC_REGISTRY *GetSadAstFuncRegistryInstance();
RET_VAL FreeSadAstFuncRegistryInstance();
        
        
END_C_NAMESPACE
        
#endif
