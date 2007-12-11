/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

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
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



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
/* Tokens.  */
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




/* Copy the first part of user declarations.  */
#line 1 "analysis_def_parser.y"

#include "sad_ast.h"

static SAD_AST_ENV *_astEnv;
static SAD_AST *ast;
static SPECIES *species;
static REACTION *reaction;
        
       
       

static SPECIES *FindSpeciesFromID( char *id ) {
    int i = 0;
    SAD_AST_ENV *env = GetSadAstEnv();
    int size = env->speciesSize;
    SPECIES **speciesArray = env->speciesArray;
    SPECIES *target = NULL;
    char *targetID = NULL;
     
    for( ; i < size; i++ ) {
        target = speciesArray[i];
        targetID = GetCharArrayOfString( GetSpeciesNodeName( target ) );
        if( strcmp( targetID, id ) == 0 ) {
            return target;
        }
    }
    return NULL;
}

static REACTION *FindReactionFromID( char *id ) {
    int i = 0;
    SAD_AST_ENV *env = GetSadAstEnv();
    int size = env->speciesSize;
    REACTION **reactionArray = env->reactionArray;
    REACTION *target = NULL;
    char *targetID = NULL;
     
    for( ; i < size; i++ ) {
        target = reactionArray[i];
        targetID = GetCharArrayOfString( GetReactionNodeName( target ) );
        if( strcmp( targetID, id ) == 0 ) {
            return target;
        }
    }
    return NULL;
}


/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 49 "analysis_def_parser.y"
{
    char *string;	
    double value;
    SAD_AST *ast;
}
/* Line 187 of yacc.c.  */
#line 224 "analysis_def_parser.tab.c"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 237 "analysis_def_parser.tab.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   119

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  38
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  11
/* YYNRULES -- Number of rules.  */
#define YYNRULES  35
/* YYNRULES -- Number of states.  */
#define YYNSTATES  80

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   292

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint8 yyprhs[] =
{
       0,     0,     3,     4,     7,     8,    11,    18,    22,    26,
      28,    30,    34,    38,    41,    45,    49,    53,    57,    61,
      65,    69,    73,    77,    81,    85,    88,    92,    97,   104,
     109,   111,   113,   116,   119,   121
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      39,     0,    -1,    -1,    40,    41,    -1,    -1,    41,    42,
      -1,     3,    25,     9,    43,    44,    10,    -1,     4,    28,
       8,    -1,     5,    45,     8,    -1,    46,    -1,    47,    -1,
      45,    12,    45,    -1,    45,    14,    45,    -1,    13,    45,
      -1,     6,    46,     7,    -1,    48,    15,    48,    -1,    48,
      16,    48,    -1,    48,    17,    48,    -1,    48,    18,    48,
      -1,    48,    19,    48,    -1,     6,    47,     7,    -1,    48,
      20,    48,    -1,    48,    21,    48,    -1,    48,    22,    48,
      -1,    48,    23,    48,    -1,    21,    48,    -1,     6,    48,
       7,    -1,    33,     6,    48,     7,    -1,    34,     6,    48,
      11,    48,     7,    -1,    35,     6,    48,     7,    -1,    29,
      -1,    24,    -1,    30,    25,    -1,    31,    26,    -1,    25,
      -1,    32,    27,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,    85,    85,    85,    92,    94,   104,   115,   121,   127,
     130,   136,   144,   152,   160,   166,   174,   182,   190,   198,
     206,   212,   220,   228,   236,   244,   252,   255,   263,   271,
     279,   287,   295,   309,   323,   337
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "SAD_TERM", "SAD_DESC", "SAD_COND",
  "SAD_LPAREN", "SAD_RPAREN", "SAD_SEMI", "SAD_LCURLY", "SAD_RCURLY",
  "SAD_COMMA", "SAD_AND", "SAD_NOT", "SAD_OR", "SAD_LE", "SAD_LT",
  "SAD_GE", "SAD_GT", "SAD_EQ", "SAD_PLUS", "SAD_MINUS", "SAD_TIMES",
  "SAD_DIV", "SAD_TIME_VAR", "SAD_IDENTIFIER", "SAD_SPECIES",
  "SAD_REACTION", "SAD_STRING", "SAD_CONSTANT", "SAD_CON_OP", "SAD_NUM_OP",
  "SAD_ATSIGN", "SAD_EXP", "SAD_POW", "SAD_LOG", "SAD_ERROR", "UMINUS",
  "$accept", "program", "@1", "opt_term_list", "term_dec",
  "desc_statement", "cond_statement", "bool_exp", "logical_exp",
  "comp_exp", "num_exp", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    38,    40,    39,    41,    41,    42,    43,    44,    45,
      45,    46,    46,    46,    46,    47,    47,    47,    47,    47,
      47,    48,    48,    48,    48,    48,    48,    48,    48,    48,
      48,    48,    48,    48,    48,    48
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     0,     2,     6,     3,     3,     1,
       1,     3,     3,     2,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     2,     3,     4,     6,     4,
       1,     1,     2,     2,     1,     2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       2,     0,     4,     1,     3,     0,     5,     0,     0,     0,
       0,     0,     0,     0,     7,     0,     0,     0,    31,    34,
      30,     0,     0,     0,     0,     0,     0,     0,     9,    10,
       0,     6,     0,     9,    10,     0,    13,     0,    25,    32,
      33,    35,     0,     0,     0,     8,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,    20,    26,
       0,     0,     0,     0,    11,    12,    15,    16,    17,    18,
      19,    21,    22,    23,    24,    27,     0,    29,     0,    28
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     1,     2,     4,     6,    10,    13,    27,    28,    29,
      30
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -16
static const yytype_int8 yypact[] =
{
     -16,     1,   -16,   -16,     5,    24,   -16,    41,    52,    29,
      53,    56,    38,    55,   -16,    38,    38,    54,   -16,   -16,
     -16,    49,    40,    72,    76,    94,    95,    -5,   -16,   -16,
      96,   -16,    -8,    69,    73,    -2,   -16,    54,   -16,   -16,
     -16,   -16,    54,    54,    54,   -16,    38,    38,    54,    54,
      54,    54,    54,    54,    54,    54,    54,   -16,   -16,   -16,
       3,    25,    87,    70,   -16,   -16,    32,    32,    32,    32,
      32,     8,     8,   -16,   -16,   -16,    54,   -16,    74,   -16
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -16,   -16,   -16,   -16,   -16,   -16,   -16,    -4,    88,    89,
     -15
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint8 yytable[] =
{
      35,     3,    38,    45,    46,    59,    47,    46,     5,    47,
      59,    32,    36,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    60,    53,    54,    55,    56,    61,    62,    63,
      55,    56,    75,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    64,    65,    15,    53,    54,    55,    56,     7,
       8,    16,    53,    54,    55,    56,     9,    11,    12,    17,
      37,    78,    18,    19,    14,    31,    40,    20,    21,    22,
      23,    24,    25,    26,    39,    17,    57,    77,    18,    19,
      58,    79,    42,    20,    21,    22,    23,    24,    25,    26,
      53,    54,    55,    56,    53,    54,    55,    56,    76,    41,
      43,    44,     0,    33,    34,     0,     0,    53,    54,    55,
      56,    48,    49,    50,    51,    52,    53,    54,    55,    56
};

static const yytype_int8 yycheck[] =
{
      15,     0,    17,     8,    12,     7,    14,    12,     3,    14,
       7,    15,    16,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    37,    20,    21,    22,    23,    42,    43,    44,
      22,    23,     7,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    46,    47,     6,    20,    21,    22,    23,    25,
       9,    13,    20,    21,    22,    23,     4,    28,     5,    21,
       6,    76,    24,    25,     8,    10,    26,    29,    30,    31,
      32,    33,    34,    35,    25,    21,     7,     7,    24,    25,
       7,     7,     6,    29,    30,    31,    32,    33,    34,    35,
      20,    21,    22,    23,    20,    21,    22,    23,    11,    27,
       6,     6,    -1,    15,    15,    -1,    -1,    20,    21,    22,
      23,    15,    16,    17,    18,    19,    20,    21,    22,    23
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    39,    40,     0,    41,     3,    42,    25,     9,     4,
      43,    28,     5,    44,     8,     6,    13,    21,    24,    25,
      29,    30,    31,    32,    33,    34,    35,    45,    46,    47,
      48,    10,    45,    46,    47,    48,    45,     6,    48,    25,
      26,    27,     6,     6,     6,     8,    12,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,     7,     7,     7,
      48,    48,    48,    48,    45,    45,    48,    48,    48,    48,
      48,    48,    48,    48,    48,     7,    11,     7,    48,     7
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

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



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 85 "analysis_def_parser.y"
    { 
        _astEnv = GetSadAstEnv();
    
    ;}
    break;

  case 5:
#line 94 "analysis_def_parser.y"
    {        
        _astEnv = GetSadAstEnv();
        if( IS_FAILED( AddElementInLinkedList( (yyvsp[(2) - (2)].ast), _astEnv->termList->terms ) ) ) {
            PrintSadAstErrorMessage( "Error adding term %s in the list", ((SAD_AST_TERM*)(yyvsp[(2) - (2)].ast))->id );
            return 1;
        }
    ;}
    break;

  case 6:
#line 104 "analysis_def_parser.y"
    {
        ast = (SAD_AST*)CreateSadAstTerm( (yyvsp[(2) - (6)].string), (yyvsp[(4) - (6)].string), (yyvsp[(5) - (6)].ast) );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating term %s", (yyvsp[(2) - (6)].string) );
            yyerror(NULL); return 1;
        } 
        (yyval.ast) = ast; 
    ;}
    break;

  case 7:
#line 115 "analysis_def_parser.y"
    {
        (yyval.string) = (yyvsp[(2) - (3)].string);     
    ;}
    break;

  case 8:
#line 121 "analysis_def_parser.y"
    {
        (yyval.ast) = (yyvsp[(2) - (3)].ast);     
    ;}
    break;

  case 9:
#line 127 "analysis_def_parser.y"
    {
        (yyval.ast) = (yyvsp[(1) - (1)].ast);     
    ;}
    break;

  case 10:
#line 130 "analysis_def_parser.y"
    {
        (yyval.ast) = (yyvsp[(1) - (1)].ast);     
    ;}
    break;

  case 11:
#line 136 "analysis_def_parser.y"
    {
        ast = (SAD_AST*)CreateSadAstBinaryLogicalExp( LOGICAL_EXP_TYPE_SAD_AST_AND, (SAD_AST_EXP*)(yyvsp[(1) - (3)].ast), (SAD_AST_EXP*)(yyvsp[(3) - (3)].ast) );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating and expression" );
            yyerror(NULL); return 1;
        } 
        (yyval.ast) = ast; 
    ;}
    break;

  case 12:
#line 144 "analysis_def_parser.y"
    {
        ast = (SAD_AST*)CreateSadAstBinaryLogicalExp( LOGICAL_EXP_TYPE_SAD_AST_OR, (SAD_AST_EXP*)(yyvsp[(1) - (3)].ast), (SAD_AST_EXP*)(yyvsp[(3) - (3)].ast) );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating or expression" );
            yyerror(NULL); return 1;
        } 
        (yyval.ast) = ast; 
    ;}
    break;

  case 13:
#line 152 "analysis_def_parser.y"
    {
        ast = (SAD_AST*)CreateSadAstUnaryLogicalExp( LOGICAL_EXP_TYPE_SAD_AST_NOT, (SAD_AST_EXP*)(yyvsp[(2) - (2)].ast) );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating not expression" );
            yyerror(NULL); return 1;
        } 
        (yyval.ast) = ast; 
    ;}
    break;

  case 14:
#line 160 "analysis_def_parser.y"
    {
        (yyval.ast) = (yyvsp[(2) - (3)].ast);     
    ;}
    break;

  case 15:
#line 166 "analysis_def_parser.y"
    {
        ast = (SAD_AST*)CreateSadAstCompExp( COMP_EXP_TYPE_SAD_AST_LE, (SAD_AST_EXP*)(yyvsp[(1) - (3)].ast), (SAD_AST_EXP*)(yyvsp[(3) - (3)].ast) );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating less than or equal to expression" );
            yyerror(NULL); return 1;
        } 
        (yyval.ast) = ast; 
    ;}
    break;

  case 16:
#line 174 "analysis_def_parser.y"
    {
        ast = (SAD_AST*)CreateSadAstCompExp( COMP_EXP_TYPE_SAD_AST_LT, (SAD_AST_EXP*)(yyvsp[(1) - (3)].ast), (SAD_AST_EXP*)(yyvsp[(3) - (3)].ast) );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating less than expression" );
            yyerror(NULL); return 1;
        } 
        (yyval.ast) = ast; 
    ;}
    break;

  case 17:
#line 182 "analysis_def_parser.y"
    {
        ast = (SAD_AST*)CreateSadAstCompExp( COMP_EXP_TYPE_SAD_AST_GE, (SAD_AST_EXP*)(yyvsp[(1) - (3)].ast), (SAD_AST_EXP*)(yyvsp[(3) - (3)].ast) );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating greater than or equal to expression" );
            yyerror(NULL); return 1;
        } 
        (yyval.ast) = ast; 
    ;}
    break;

  case 18:
#line 190 "analysis_def_parser.y"
    {
        ast = (SAD_AST*)CreateSadAstCompExp( COMP_EXP_TYPE_SAD_AST_GT, (SAD_AST_EXP*)(yyvsp[(1) - (3)].ast), (SAD_AST_EXP*)(yyvsp[(3) - (3)].ast) );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating greater than expression" );
            yyerror(NULL); return 1;
        } 
        (yyval.ast) = ast; 
    ;}
    break;

  case 19:
#line 198 "analysis_def_parser.y"
    {
        ast = (SAD_AST*)CreateSadAstCompExp( COMP_EXP_TYPE_SAD_AST_EQ, (SAD_AST_EXP*)(yyvsp[(1) - (3)].ast), (SAD_AST_EXP*)(yyvsp[(3) - (3)].ast) );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating equal expression" );
            yyerror(NULL); return 1;
        } 
        (yyval.ast) = ast; 
    ;}
    break;

  case 20:
#line 206 "analysis_def_parser.y"
    {
        (yyval.ast) = (yyvsp[(2) - (3)].ast);     
    ;}
    break;

  case 21:
#line 212 "analysis_def_parser.y"
    {
        ast = (SAD_AST*)CreateSadAstBinaryNumExp( NUM_EXP_TYPE_SAD_AST_PLUS, (SAD_AST_EXP*)(yyvsp[(1) - (3)].ast), (SAD_AST_EXP*)(yyvsp[(3) - (3)].ast) );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating plus expression" );
            yyerror(NULL); return 1;
        } 
        (yyval.ast) = ast; 
    ;}
    break;

  case 22:
#line 220 "analysis_def_parser.y"
    {
        ast = (SAD_AST*)CreateSadAstBinaryNumExp( NUM_EXP_TYPE_SAD_AST_MINUS, (SAD_AST_EXP*)(yyvsp[(1) - (3)].ast), (SAD_AST_EXP*)(yyvsp[(3) - (3)].ast) );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating minus expression" );
            yyerror(NULL); return 1;
        } 
        (yyval.ast) = ast; 
    ;}
    break;

  case 23:
#line 228 "analysis_def_parser.y"
    {
        ast = (SAD_AST*)CreateSadAstBinaryNumExp( NUM_EXP_TYPE_SAD_AST_TIMES, (SAD_AST_EXP*)(yyvsp[(1) - (3)].ast), (SAD_AST_EXP*)(yyvsp[(3) - (3)].ast) );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating times expression" );
            yyerror(NULL); return 1;
        } 
        (yyval.ast) = ast; 
    ;}
    break;

  case 24:
#line 236 "analysis_def_parser.y"
    {
        ast = (SAD_AST*)CreateSadAstBinaryNumExp( NUM_EXP_TYPE_SAD_AST_DIV, (SAD_AST_EXP*)(yyvsp[(1) - (3)].ast), (SAD_AST_EXP*)(yyvsp[(3) - (3)].ast) );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating div expression" );
            yyerror(NULL); return 1;
        } 
        (yyval.ast) = ast; 
    ;}
    break;

  case 25:
#line 244 "analysis_def_parser.y"
    {
        ast = (SAD_AST*)CreateSadAstUnaryNumExp( NUM_EXP_TYPE_SAD_AST_UMINUS, (SAD_AST_EXP*)(yyvsp[(2) - (2)].ast) );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating unary minus expression" );
            yyerror(NULL); return 1;
        } 
        (yyval.ast) = ast; 
    ;}
    break;

  case 26:
#line 252 "analysis_def_parser.y"
    {
        (yyval.ast) = (yyvsp[(2) - (3)].ast);     
    ;}
    break;

  case 27:
#line 255 "analysis_def_parser.y"
    {
        ast = (SAD_AST*)CreateSadAstFuncExp( "exp", 1, (SAD_AST_EXP*)(yyvsp[(3) - (4)].ast) );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating exp function" );
            yyerror(NULL); return 1;
        } 
        (yyval.ast) = ast; 
    ;}
    break;

  case 28:
#line 263 "analysis_def_parser.y"
    {
        ast = (SAD_AST*)CreateSadAstFuncExp( "pow", 2, (SAD_AST_EXP*)(yyvsp[(3) - (6)].ast), (SAD_AST_EXP*)(yyvsp[(5) - (6)].ast) );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating pow function" );
            yyerror(NULL); return 1;
        } 
        (yyval.ast) = ast; 
    ;}
    break;

  case 29:
#line 271 "analysis_def_parser.y"
    {
        ast = (SAD_AST*)CreateSadAstFuncExp( "log", 1, (SAD_AST_EXP*)(yyvsp[(3) - (4)].ast) );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating log function" );
            yyerror(NULL); return 1;
        } 
        (yyval.ast) = ast; 
    ;}
    break;

  case 30:
#line 279 "analysis_def_parser.y"
    { 
        ast = (SAD_AST*)CreateSadAstConstant( (yyvsp[(1) - (1)].value) );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating time variable" );
            yyerror(NULL); return 1;
        } 
        (yyval.ast) = ast; 
    ;}
    break;

  case 31:
#line 287 "analysis_def_parser.y"
    { 
        ast = (SAD_AST*)CreateSadAstTimeVar( );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating time variable" );
            yyerror(NULL); return 1;
        } 
        (yyval.ast) = ast; 
    ;}
    break;

  case 32:
#line 295 "analysis_def_parser.y"
    { 
        species = FindSpeciesFromID( (yyvsp[(2) - (2)].string) );
        if( species == NULL ) {
            PrintSadAstErrorMessage( "%s is not a valid species ID", (yyvsp[(2) - (2)].string) );
            yyerror(NULL); return 1;
        } 
        ast = (SAD_AST*)CreateSadAstSpeciesCon( species );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating species node for %s", (yyvsp[(2) - (2)].string) );
            yyerror(NULL); return 1;
        } 
        (yyval.ast) = ast; 
        FREE((yyvsp[(2) - (2)].string));
    ;}
    break;

  case 33:
#line 309 "analysis_def_parser.y"
    { 
        species = FindSpeciesFromID( (yyvsp[(2) - (2)].string) );
        if( species == NULL ) {
            PrintSadAstErrorMessage( "%s is not a valid species ID", (yyvsp[(2) - (2)].string) );
            yyerror(NULL); return 1;
        } 
        ast = (SAD_AST*)CreateSadAstSpeciesCnt( species );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating species node for %s", (yyvsp[(2) - (2)].string) );
            yyerror(NULL); return 1;
        } 
        (yyval.ast) = ast; 
        FREE((yyvsp[(2) - (2)].string));
    ;}
    break;

  case 34:
#line 323 "analysis_def_parser.y"
    { 
        species = FindSpeciesFromID( (yyvsp[(1) - (1)].string) );
        if( species == NULL ) {
            PrintSadAstErrorMessage( "%s is not a valid species ID", (yyvsp[(1) - (1)].string) );
            yyerror(NULL); return 1;
        } 
        ast = (SAD_AST*)CreateSadAstSpeciesCnt( species );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating species node for %s", (yyvsp[(1) - (1)].string) );
            yyerror(NULL); return 1;
        } 
        (yyval.ast) = ast; 
        FREE((yyvsp[(1) - (1)].string));
    ;}
    break;

  case 35:
#line 337 "analysis_def_parser.y"
    { 
        reaction = FindReactionFromID( (yyvsp[(2) - (2)].string) );
        if( reaction == NULL ) {
            PrintSadAstErrorMessage( "%s is not a valid reaction ID", (yyvsp[(2) - (2)].string) );
            yyerror(NULL); return 1;
        } 
        ast = (SAD_AST*)CreateSadAstReactionCnt( reaction );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating reaction node for %s", (yyvsp[(2) - (2)].string) );
            yyerror(NULL); return 1;
        }
        (yyval.ast) = ast; 
        FREE((yyvsp[(2) - (2)].string));
    ;}
    break;


/* Line 1267 of yacc.c.  */
#line 1868 "analysis_def_parser.tab.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


#line 355 "analysis_def_parser.y"


