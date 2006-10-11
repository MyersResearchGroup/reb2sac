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
#include "sad_ast_func_registry.h"


BEGIN_C_NAMESPACE


#define TYPE_SAD_AST_TERM_LIST ((int)0x0001)
#define TYPE_SAD_AST_TERM ((int)0x0002)
#define TYPE_SAD_AST_EXP ((int)0x1000)
#define TYPE_SAD_AST_BINARY_EXP ((int)0x1010)
#define TYPE_SAD_AST_UNARY_EXP ((int)0x1020)
#define TYPE_SAD_AST_FUNC_EXP ((int)0x1030)
#define TYPE_SAD_AST_BINARY_LOGICAL_EXP ((int)0x1040)
#define TYPE_SAD_AST_UNARY_LOGICAL_EXP ((int)0x1050)
#define TYPE_SAD_AST_COMP_EXP ((int)0x1060)   
#define TYPE_SAD_AST_BINARY_NUM_EXP ((int)0x1070)   
#define TYPE_SAD_AST_UNARY_NUM_EXP ((int)0x1080)   
#define TYPE_SAD_AST_SPECIES_CON ((int)0x1090)
#define TYPE_SAD_AST_SPECIES_CNT ((int)0x10a0)
#define TYPE_SAD_AST_REACTION_CNT ((int)0x10b0)
#define TYPE_SAD_AST_CONSTANT ((int)0x10c0)
#define TYPE_SAD_AST_TIME_VAR ((int)0x10d0)


#define EXP_TYPE_SAD_AST_MASK ((int)0x1000)    

#define COMP_EXP_TYPE_SAD_AST_MASK ((int)0x0100)    
#define COMP_EXP_TYPE_SAD_AST_EQ ((int)0x1101)    
#define COMP_EXP_TYPE_SAD_AST_LE ((int)0x1102)    
#define COMP_EXP_TYPE_SAD_AST_LT ((int)0x1103)    
#define COMP_EXP_TYPE_SAD_AST_GE ((int)0x1104)    
#define COMP_EXP_TYPE_SAD_AST_GT ((int)0x1105)    

#define NUM_EXP_TYPE_SAD_AST_MASK ((int)0x0200)    
#define NUM_EXP_TYPE_SAD_AST_PLUS ((int)0x1201)    
#define NUM_EXP_TYPE_SAD_AST_MINUS ((int)0x1202)    
#define NUM_EXP_TYPE_SAD_AST_TIMES ((int)0x1203)    
#define NUM_EXP_TYPE_SAD_AST_DIV ((int)0x1204)    
#define NUM_EXP_TYPE_SAD_AST_UMINUS ((int)0x1205)    

#define LOGICAL_EXP_TYPE_SAD_AST_MASK ((int)0x0400)    
#define LOGICAL_EXP_TYPE_SAD_AST_AND ((int)0x1401)    
#define LOGICAL_EXP_TYPE_SAD_AST_OR ((int)0x1402)    
#define LOGICAL_EXP_TYPE_SAD_AST_NOT ((int)0x1403)    

/*
#define SPECIES_EXP_TYPE_SAD_AST_MASK ((int)0x0800)    
#define SPECIES_EXP_TYPE_SAD_AST_CNT ((int)0x1801)    
#define SPECIES_EXP_TYPE_SAD_AST_CON ((int)0x1802)    

#define REACTION_EXP_TYPE_SAD_AST_MASK ((int)0x0810)    
#define REACTION_EXP_TYPE_SAD_AST_CNT ((int)0x1811)    
*/

#define IS_EXP_TYPE_SAD_AST(t) ((t & EXP_TYPE_SAD_AST_MASK) ? TRUE : FALSE)
#define IS_COMP_EXP_TYPE_SAD_AST(t) ((t & COMP_EXP_TYPE_SAD_AST_MASK) ? TRUE : FALSE)
#define IS_NUM_EXP_TYPE_SAD_AST(t) ((t & NUM_EXP_TYPE_SAD_AST_MASK) ? TRUE : FALSE)
#define IS_LOGICAL_EXP_TYPE_SAD_AST(t) ((t & LOGICAL_EXP_TYPE_SAD_AST_MASK) ? TRUE : FALSE)
/*
#define IS_SPECIES_EXP_TYPE_SAD_AST(t) ((t & SPECIES_EXP_TYPE_SAD_AST_MASK) ? TRUE : FALSE)
#define IS_REACTION_EXP_TYPE_SAD_AST(t) ((t & REACTION_EXP_TYPE_SAD_AST_MASK) ? TRUE : FALSE)
*/


struct _SAD_AST_VISITOR;
typedef struct _SAD_AST_VISITOR SAD_AST_VISITOR;

struct _SAD_AST;
typedef struct _SAD_AST SAD_AST;



struct _SAD_AST_TERM_LIST;
typedef struct _SAD_AST_TERM_LIST SAD_AST_TERM_LIST;


struct _SAD_AST_TERM;
typedef struct _SAD_AST_TERM SAD_AST_TERM;


struct _SAD_AST_EXP;
typedef struct _SAD_AST_EXP SAD_AST_EXP;

struct _SAD_AST_BINARY_EXP;
typedef struct _SAD_AST_BINARY_EXP SAD_AST_BINARY_EXP;

struct _SAD_AST_UNARY_EXP;
typedef struct _SAD_AST_UNARY_EXP SAD_AST_UNARY_EXP;

struct _SAD_AST_FUNC_EXP;
typedef struct _SAD_AST_FUNC_EXP SAD_AST_FUNC_EXP;

struct _SAD_AST_SPECIES;
typedef struct _SAD_AST_SPECIES SAD_AST_SPECIES;

struct _SAD_AST_REACTION;
typedef struct _SAD_AST_REACTION SAD_AST_REACTION;

struct _SAD_AST_CONSTANT;
typedef struct _SAD_AST_CONSTANT SAD_AST_CONSTANT;

struct _SAD_AST_TIME_VAR;
typedef struct _SAD_AST_TIME_VAR SAD_AST_TIME_VAR;


typedef struct {
    double time;
    SPECIES **speciesArray;
    int speciesSize;
    REACTION **reactionArray; 
    int reactionSize;
    SAD_AST_TERM_LIST *termList;
} SAD_AST_ENV;


struct _SAD_AST {
    int astType;
    RET_VAL (*Accept)( SAD_AST *ast, SAD_AST_VISITOR *visitor );
};


struct _SAD_AST_TERM_LIST {
    int astType;
    RET_VAL (*Accept)( SAD_AST *ast, SAD_AST_VISITOR *visitor );
    
    LINKED_LIST *terms;
};

struct _SAD_AST_TERM {
    int astType;
    RET_VAL (*Accept)( SAD_AST *ast, SAD_AST_VISITOR *visitor );
    
    char *id;
    char *desc;
    SAD_AST_EXP *condition;            
};

struct _SAD_AST_EXP {
    int astType;
    RET_VAL (*Accept)( SAD_AST *ast, SAD_AST_VISITOR *visitor );
    
    double result;
};

struct _SAD_AST_BINARY_EXP {
    int astType;
    RET_VAL (*Accept)( SAD_AST *ast, SAD_AST_VISITOR *visitor );
    
    double result;
    
    int type;
    SAD_AST_EXP *left;    
    SAD_AST_EXP *right;    
};


struct _SAD_AST_UNARY_EXP {
    int astType;
    RET_VAL (*Accept)( SAD_AST *ast, SAD_AST_VISITOR *visitor );
    
    double result;
    
    int type;
    SAD_AST_EXP *exp;    
};


struct _SAD_AST_FUNC_EXP {
    int astType;
    RET_VAL (*Accept)( SAD_AST *ast, SAD_AST_VISITOR *visitor );
    
    double result;
    
    double **values;
    SAD_AST_FUNC_REGISTRY_ENTRY *entry;    
    SAD_AST_EXP **argExps;    
};


struct _SAD_AST_SPECIES {
    int astType;
    RET_VAL (*Accept)( SAD_AST *ast, SAD_AST_VISITOR *visitor );
    
    double result;
    
    SPECIES *species;    
};


struct _SAD_AST_REACTION {
    int astType;
    RET_VAL (*Accept)( SAD_AST *ast, SAD_AST_VISITOR *visitor );
    
    double result;
    
    REACTION *reaction;    
};


struct _SAD_AST_CONSTANT {
    int astType;
    RET_VAL (*Accept)( SAD_AST *ast, SAD_AST_VISITOR *visitor );
    
    double result;
};


struct _SAD_AST_TIME_VAR {
    int astType;
    RET_VAL (*Accept)( SAD_AST *ast, SAD_AST_VISITOR *visitor );
    
    double result;
    
    double *pTime;
};

SAD_AST_ENV *GetSadAstEnv( );
RET_VAL FreeSadAstEnv( );

SAD_AST_TERM_LIST *CreateSadAstTermList( );
SAD_AST_TERM *CreateSadAstTerm( char *id, char *desc, SAD_AST_EXP *condition );
SAD_AST_FUNC_EXP *CreateSadAstFuncExp( char *name, int argc, ... );
SAD_AST_BINARY_EXP *CreateSadAstBinaryLogicalExp( int type, SAD_AST_EXP *left, SAD_AST_EXP *right );
SAD_AST_BINARY_EXP *CreateSadAstCompExp( int type, SAD_AST_EXP *left, SAD_AST_EXP *right );
SAD_AST_BINARY_EXP *CreateSadAstBinaryNumExp( int type, SAD_AST_EXP *left, SAD_AST_EXP *right );
SAD_AST_UNARY_EXP *CreateSadAstUnaryNumExp( int type, SAD_AST_EXP *exp );
SAD_AST_UNARY_EXP *CreateSadAstUnaryLogicalExp( int type, SAD_AST_EXP *exp );
SAD_AST_SPECIES *CreateSadAstSpeciesCon( SPECIES *species );
SAD_AST_SPECIES *CreateSadAstSpeciesCnt( SPECIES *species );
SAD_AST_REACTION *CreateSadAstReactionCnt( REACTION *reaction );
SAD_AST_CONSTANT *CreateSadAstConstant( double value );
SAD_AST_TIME_VAR *CreateSadAstTimeVar( );


struct _SAD_AST_VISITOR {
    CADDR_T _internal1;
    CADDR_T _internal2;
    CADDR_T _internal3;
    RET_VAL (*VisitTermList)( SAD_AST_VISITOR *visitor, SAD_AST_TERM_LIST *ast );
    RET_VAL (*VisitTerm)( SAD_AST_VISITOR *visitor, SAD_AST_TERM *ast );
    RET_VAL (*VisitCompExp)( SAD_AST_VISITOR *visitor, SAD_AST_BINARY_EXP *ast );
    RET_VAL (*VisitBinaryNumExp)( SAD_AST_VISITOR *visitor, SAD_AST_BINARY_EXP *ast );
    RET_VAL (*VisitBinaryLogicalExp)( SAD_AST_VISITOR *visitor, SAD_AST_BINARY_EXP *ast );
    RET_VAL (*VisitUnaryNumExp)( SAD_AST_VISITOR *visitor, SAD_AST_UNARY_EXP *ast );
    RET_VAL (*VisitUnaryLogicalExp)( SAD_AST_VISITOR *visitor, SAD_AST_UNARY_EXP *ast );
    RET_VAL (*VisitFuncExp)( SAD_AST_VISITOR *visitor, SAD_AST_FUNC_EXP *ast );
    RET_VAL (*VisitSpeciesCon)( SAD_AST_VISITOR *visitor, SAD_AST_SPECIES *ast );
    RET_VAL (*VisitSpeciesCnt)( SAD_AST_VISITOR *visitor, SAD_AST_SPECIES *ast );
    RET_VAL (*VisitReactionCnt)( SAD_AST_VISITOR *visitor, SAD_AST_REACTION *ast );
    RET_VAL (*VisitConstant)( SAD_AST_VISITOR *visitor, SAD_AST_CONSTANT *ast );
    RET_VAL (*VisitTimeVar)( SAD_AST_VISITOR *visitor, SAD_AST_TIME_VAR *ast );
};


void PrintSadAstErrorMessage( char *format, ... );

END_C_NAMESPACE


#endif
        
        