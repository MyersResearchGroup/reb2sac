/* A Bison parser, made by GNU Bison 2.0.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     SAD_TERM = 258,
     SAD_DESC = 259,
     SAD_COND = 260,
     SAD_LPAREN = 261,
     SAD_RPAREN = 262,
     SAD_SEMI = 263,
     SAD_LCURLY = 264,
     SAD_RCURLY = 265,
     SAD_COMMA = 266,
     SAD_AND = 267,
     SAD_NOT = 268,
     SAD_OR = 269,
     SAD_LE = 270,
     SAD_LT = 271,
     SAD_GE = 272,
     SAD_GT = 273,
     SAD_EQ = 274,
     SAD_PLUS = 275,
     SAD_MINUS = 276,
     SAD_TIMES = 277,
     SAD_DIV = 278,
     SAD_TIME_VAR = 279,
     SAD_IDENTIFIER = 280,
     SAD_SPECIES = 281,
     SAD_REACTION = 282,
     SAD_STRING = 283,
     SAD_CONSTANT = 284,
     SAD_CON_OP = 285,
     SAD_NUM_OP = 286,
     SAD_ATSIGN = 287,
     SAD_EXP = 288,
     SAD_POW = 289,
     SAD_LOG = 290,
     SAD_ERROR = 291,
     UMINUS = 292
   };
#endif
#define SAD_TERM 258
#define SAD_DESC 259
#define SAD_COND 260
#define SAD_LPAREN 261
#define SAD_RPAREN 262
#define SAD_SEMI 263
#define SAD_LCURLY 264
#define SAD_RCURLY 265
#define SAD_COMMA 266
#define SAD_AND 267
#define SAD_NOT 268
#define SAD_OR 269
#define SAD_LE 270
#define SAD_LT 271
#define SAD_GE 272
#define SAD_GT 273
#define SAD_EQ 274
#define SAD_PLUS 275
#define SAD_MINUS 276
#define SAD_TIMES 277
#define SAD_DIV 278
#define SAD_TIME_VAR 279
#define SAD_IDENTIFIER 280
#define SAD_SPECIES 281
#define SAD_REACTION 282
#define SAD_STRING 283
#define SAD_CONSTANT 284
#define SAD_CON_OP 285
#define SAD_NUM_OP 286
#define SAD_ATSIGN 287
#define SAD_EXP 288
#define SAD_POW 289
#define SAD_LOG 290
#define SAD_ERROR 291
#define UMINUS 292




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 49 "analysis_def_parser.y"
typedef union YYSTYPE {
    char *string;	
    double value;
    SAD_AST *ast;
} YYSTYPE;
/* Line 1274 of yacc.c.  */
#line 117 "analysis_def_parser.tab.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;



