/* A Bison parser, made by GNU Bison 2.7.12-4996.  */

/* Bison interface for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_ANALYSIS_DEF_PARSER_TAB_H_INCLUDED
# define YY_YY_ANALYSIS_DEF_PARSER_TAB_H_INCLUDED
/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

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


#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{
/* Line 2053 of yacc.c  */
#line 49 "analysis_def_parser.y"

    char *string;	
    double value;
    SAD_AST *ast;


/* Line 2053 of yacc.c  */
#line 101 "analysis_def_parser.tab.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */

#endif /* !YY_YY_ANALYSIS_DEF_PARSER_TAB_H_INCLUDED  */
