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
#if !defined(HAVE_SAD_AST)
#define HAVE_SAD_AST


#include "common.h"
#include "linked_list.h"
#include "species_node.h"
#include "reaction_node.h"

BEGIN_C_NAMESPACE



struct _SAD_AST_VISITOR;
typedef struct _SAD_AST_VISITOR SAD_AST_VISITOR;

struct _SAD_AST;
typedef struct _SAD_AST SAD_AST;

#define TYPE_SAD_AST_TERM_LIST ((int)0x0001)
struct _SAD_AST_TERM_LIST;
typedef struct _SAD_AST_TERM_LIST SAD_AST_TERM_LIST;

#define TYPE_SAD_AST_TERM ((int)0x0002)
struct _SAD_AST_TERM;
typedef struct _SAD_AST_TERM SAD_AST_TERM;

#define TYPE_SAD_AST_EXP ((int)0x1000)
struct _SAD_AST_EXP;
typedef struct _SAD_AST_EXP SAD_AST_EXP;

#define TYPE_SAD_AST_BINARY_EXP ((int)0x1010)
struct _SAD_AST_BINARY_EXP;
typedef struct _SAD_AST_BINARY_EXP SAD_AST_BINARY_EXP;

#define TYPE_SAD_AST_UNARY_EXP ((int)0x1020)
struct _SAD_AST_UNARY_EXP;
typedef struct _SAD_AST_UNARY_EXP SAD_AST_UNARY_EXP;

#define TYPE_SAD_AST_FUNC_EXP ((int)0x1030)
struct _SAD_AST_FUNC_EXP;
typedef struct _SAD_AST_FUNC_EXP SAD_AST_FUNC_EXP;


#define TYPE_SAD_AST_BINARY_LOGICAL_EXP ((int)0x1040)
struct _SAD_AST_BINARY_LOGICAL_EXP;
typedef struct _SAD_AST_BINARY_LOGICAL_EXP SAD_AST_BINARY_LOGICAL_EXP;

#define TYPE_SAD_AST_UNARY_LOGICAL_EXP ((int)0x1050)
struct _SAD_AST_UNARY_LOGICAL_EXP;
typedef struct _SAD_AST_UNARY_LOGICAL_EXP SAD_AST_UNARY_LOGICAL_EXP;

#define TYPE_SAD_AST_COMP_EXP ((int)0x1060)   
struct _SAD_AST_COMP_EXP;
typedef struct _SAD_AST_COMP_EXP SAD_AST_COMP_EXP;

#define TYPE_SAD_AST_BINARY_NUM_EXP ((int)0x1070)   
struct _SAD_AST_BINARY_NUM_EXP;
typedef struct _SAD_AST_BINARY_NUM_EXP SAD_AST_BINARY_NUM_EXP;

#define TYPE_SAD_AST_UNARY_NUM_EXP ((int)0x1080)   
struct _SAD_AST_UNARY_NUM_EXP;
typedef struct _SAD_AST_UNARY_NUM_EXP SAD_AST_UNARY_NUM_EXP;

#define TYPE_SAD_AST_SPECIES_CON ((int)0x1090)
struct _SAD_AST_SPECIES_CON;
typedef struct _SAD_AST_SPECIES_CON SAD_AST_SPECIES_CON;

#define TYPE_SAD_AST_SPECIES_CNT ((int)0x10a0)
struct _SAD_AST_SPECIES_CNT;
typedef struct _SAD_AST_SPECIES_CNT SAD_AST_SPECIES_CNT;

#define TYPE_SAD_AST_REACTION_CNT ((int)0x10b0)
struct _SAD_AST_REACTION_CNT;
typedef struct _SAD_AST_REACTION_CNT SAD_AST_REACTION_CNT;

#define TYPE_SAD_AST_CONSTANT ((int)0x10c0)
struct _SAD_AST_CONSTANT;
typedef struct _SAD_AST_CONSTANT SAD_AST_CONSTANT;

#define TYPE_SAD_AST_TIME_VAR ((int)0x10d0)
struct _SAD_AST_TIME_VAR;
typedef struct _SAD_AST_TIME_VAR SAD_AST_TIME_VAR;




struct _SAD_AST {
    int astType;
    RET_VAL (*AcceptPostOrder)( SAD_AST *ast, SAD_AST_VISITOR *visitor );
    RET_VAL (*AcceptPreOrder)( SAD_AST *ast, SAD_AST_VISITOR *visitor );
    RET_VAL (*AcceptInOrder)( SAD_AST *ast, SAD_AST_VISITOR *visitor );
    RET_VAL (*Accept)( SAD_AST *ast, SAD_AST_VISITOR *visitor );
};


struct _SAD_AST_TERM_LIST {
    int astType;
    RET_VAL (*AcceptPostOrder)( SAD_AST *ast, SAD_AST_VISITOR *visitor );
    RET_VAL (*AcceptPreOrder)( SAD_AST *ast, SAD_AST_VISITOR *visitor );
    RET_VAL (*AcceptInOrder)( SAD_AST *ast, SAD_AST_VISITOR *visitor );
    RET_VAL (*Accept)( SAD_AST *ast, SAD_AST_VISITOR *visitor );
    
    LINKED_LIST *termList;
};

struct _SAD_AST_TERM {
    int astType;
    RET_VAL (*AcceptPostOrder)( SAD_AST *ast, SAD_AST_VISITOR *visitor );
    RET_VAL (*AcceptPreOrder)( SAD_AST *ast, SAD_AST_VISITOR *visitor );
    RET_VAL (*AcceptInOrder)( SAD_AST *ast, SAD_AST_VISITOR *visitor );
    RET_VAL (*Accept)( SAD_AST *ast, SAD_AST_VISITOR *visitor );
    
    char *id;
    char *desc;
    SAD_AST_EXP *condition;            
};

struct _SAD_AST_EXP {
    int astType;
    RET_VAL (*AcceptPostOrder)( SAD_AST *ast, SAD_AST_VISITOR *visitor );
    RET_VAL (*AcceptPreOrder)( SAD_AST *ast, SAD_AST_VISITOR *visitor );
    RET_VAL (*AcceptInOrder)( SAD_AST *ast, SAD_AST_VISITOR *visitor );
    RET_VAL (*Accept)( SAD_AST *ast, SAD_AST_VISITOR *visitor );
    
    double value;
};


struct _SAD_AST_BINARY_EXP {
    int astType;
    RET_VAL (*AcceptPostOrder)( SAD_AST *ast, SAD_AST_VISITOR *visitor );
    RET_VAL (*AcceptPreOrder)( SAD_AST *ast, SAD_AST_VISITOR *visitor );
    RET_VAL (*AcceptInOrder)( SAD_AST *ast, SAD_AST_VISITOR *visitor );
    RET_VAL (*Accept)( SAD_AST *ast, SAD_AST_VISITOR *visitor );        

    double value;
    int type;
    SAD_AST_EXP *left;    
    SAD_AST_EXP *right;    
};


struct _SAD_AST_UNARY_EXP {
    int astType;
    RET_VAL (*AcceptPostOrder)( SAD_AST *ast, SAD_AST_VISITOR *visitor );
    RET_VAL (*AcceptPreOrder)( SAD_AST *ast, SAD_AST_VISITOR *visitor );
    RET_VAL (*AcceptInOrder)( SAD_AST *ast, SAD_AST_VISITOR *visitor );
    RET_VAL (*Accept)( SAD_AST *ast, SAD_AST_VISITOR *visitor );        

    double value;
    int type;
    SAD_AST_EXP *exp;    
};

struct _SAD_AST_UNARY_EXP {
    int astType;
    RET_VAL (*AcceptPostOrder)( SAD_AST *ast, SAD_AST_VISITOR *visitor );
    RET_VAL (*AcceptPreOrder)( SAD_AST *ast, SAD_AST_VISITOR *visitor );
    RET_VAL (*AcceptInOrder)( SAD_AST *ast, SAD_AST_VISITOR *visitor );
    RET_VAL (*Accept)( SAD_AST *ast, SAD_AST_VISITOR *visitor );        

    double value;
    LINKED_LIST *argExps;
    double (*func)( LINKED_LIST *argList );
};


struct _SAD_AST_BINARY_LOGICAL_EXP {
    int astType;
    RET_VAL (*AcceptPostOrder)( SAD_AST *ast, SAD_AST_VISITOR *visitor );
    RET_VAL (*AcceptPreOrder)( SAD_AST *ast, SAD_AST_VISITOR *visitor );
    RET_VAL (*AcceptInOrder)( SAD_AST *ast, SAD_AST_VISITOR *visitor );
    RET_VAL (*Accept)( SAD_AST *ast, SAD_AST_VISITOR *visitor );        

    double value;
    int type;
    SAD_AST_EXP *left;    
    SAD_AST_EXP *right;    
};


struct _SAD_AST_UNARY_LOGICAL_EXP {
    int astType;
    RET_VAL (*AcceptPostOrder)( SAD_AST *ast, SAD_AST_VISITOR *visitor );
    RET_VAL (*AcceptPreOrder)( SAD_AST *ast, SAD_AST_VISITOR *visitor );
    RET_VAL (*AcceptInOrder)( SAD_AST *ast, SAD_AST_VISITOR *visitor );
    RET_VAL (*Accept)( SAD_AST *ast, SAD_AST_VISITOR *visitor );        

    double value;
    int type;
    SAD_AST_EXP *exp;    
};

struct _SAD_AST_COMP_EXP;



END_C_NAMESPACE


#endif