/* A Bison parser, made by GNU Bison 3.5.3.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
   Inc.

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.5.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
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

#line 118 "analysis_def_parser.tab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Use api.header.include to #include this header
   instead of duplicating it here.  */
#ifndef YY_YY_ANALYSIS_DEF_PARSER_TAB_H_INCLUDED
# define YY_YY_ANALYSIS_DEF_PARSER_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
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

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 49 "analysis_def_parser.y"

    char *string;	
    double value;
    SAD_AST *ast;

#line 214 "analysis_def_parser.tab.c"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_ANALYSIS_DEF_PARSER_TAB_H_INCLUDED  */



#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))

/* Stored state numbers (used for stacks). */
typedef yytype_int8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && ! defined __ICC && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                            \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
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
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
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
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

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
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  80

#define YYUNDEFTOK  2
#define YYMAXUTOK   292


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
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
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,    85,    85,    85,    92,    94,   104,   115,   121,   127,
     130,   136,   144,   152,   160,   166,   174,   182,   190,   198,
     206,   212,   220,   228,   236,   244,   252,   255,   263,   271,
     279,   287,   295,   309,   323,   337
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
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
  "$accept", "program", "$@1", "opt_term_list", "term_dec",
  "desc_statement", "cond_statement", "bool_exp", "logical_exp",
  "comp_exp", "num_exp", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_int16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292
};
# endif

#define YYPACT_NINF (-16)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
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

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_int8 yydefact[] =
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

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -16,   -16,   -16,   -16,   -16,   -16,   -16,    -4,    88,    89,
     -15
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     1,     2,     4,     6,    10,    13,    27,    28,    29,
      30
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int8 yytable[] =
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
static const yytype_int8 yystos[] =
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

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_int8 yyr1[] =
{
       0,    38,    40,    39,    41,    41,    42,    43,    44,    45,
      45,    46,    46,    46,    46,    47,    47,    47,    47,    47,
      47,    48,    48,    48,    48,    48,    48,    48,    48,    48,
      48,    48,    48,    48,    48,    48
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     0,     2,     0,     2,     6,     3,     3,     1,
       1,     3,     3,     2,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     2,     3,     4,     6,     4,
       1,     1,     2,     2,     1,     2
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YYUSE (yyoutput);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyo, yytoknum[yytype], *yyvaluep);
# endif
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyo, yytype, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[+yyssp[yyi + 1 - yynrhs]],
                       &yyvsp[(yyi + 1) - (yynrhs)]
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

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
#ifndef YYINITDEPTH
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
#   define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
#  else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
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
static char *
yystpcpy (char *yydest, const char *yysrc)
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
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
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
            else
              goto append;

          append:
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

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                yy_state_t *yyssp, int yytoken)
{
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Actual size of YYARG. */
  int yycount = 0;
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[+*yyssp];
      YYPTRDIFF_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
      yysize = yysize0;
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYPTRDIFF_T yysize1
                    = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
                    yysize = yysize1;
                  else
                    return 2;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    /* Don't count the "%s"s in the final size, but reserve room for
       the terminator.  */
    YYPTRDIFF_T yysize1 = yysize + (yystrlen (yyformat) - 2 * yycount) + 1;
    if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
      yysize = yysize1;
    else
      return 2;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss;
    yy_state_t *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYPTRDIFF_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    goto yyexhaustedlab;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
# undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
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
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
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
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

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
    
    }
#line 1448 "analysis_def_parser.tab.c"
    break;

  case 5:
#line 94 "analysis_def_parser.y"
                             {        
        _astEnv = GetSadAstEnv();
        if( IS_FAILED( AddElementInLinkedList( (CADDR_T)(yyvsp[0].ast), _astEnv->termList->terms ) ) ) {
            PrintSadAstErrorMessage( "Error adding term %s in the list", ((SAD_AST_TERM*)(yyvsp[0].ast))->id );
            return 1;
        }
    }
#line 1460 "analysis_def_parser.tab.c"
    break;

  case 6:
#line 104 "analysis_def_parser.y"
                                                                                {
      ast = (SAD_AST*)CreateSadAstTerm( (yyvsp[-4].string), (yyvsp[-2].string), (SAD_AST_EXP*)(yyvsp[-1].ast) );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating term %s", (yyvsp[-4].string) );
            yyerror(NULL); return 1;
        } 
        (yyval.ast) = ast; 
    }
#line 1473 "analysis_def_parser.tab.c"
    break;

  case 7:
#line 115 "analysis_def_parser.y"
                                  {
        (yyval.string) = (yyvsp[-1].string);     
    }
#line 1481 "analysis_def_parser.tab.c"
    break;

  case 8:
#line 121 "analysis_def_parser.y"
                               {
        (yyval.ast) = (yyvsp[-1].ast);     
    }
#line 1489 "analysis_def_parser.tab.c"
    break;

  case 9:
#line 127 "analysis_def_parser.y"
                {
        (yyval.ast) = (yyvsp[0].ast);     
    }
#line 1497 "analysis_def_parser.tab.c"
    break;

  case 10:
#line 130 "analysis_def_parser.y"
               {
        (yyval.ast) = (yyvsp[0].ast);     
    }
#line 1505 "analysis_def_parser.tab.c"
    break;

  case 11:
#line 136 "analysis_def_parser.y"
                                   {
        ast = (SAD_AST*)CreateSadAstBinaryLogicalExp( LOGICAL_EXP_TYPE_SAD_AST_AND, (SAD_AST_EXP*)(yyvsp[-2].ast), (SAD_AST_EXP*)(yyvsp[0].ast) );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating and expression" );
            yyerror(NULL); return 1;
        } 
        (yyval.ast) = ast; 
    }
#line 1518 "analysis_def_parser.tab.c"
    break;

  case 12:
#line 144 "analysis_def_parser.y"
                               {
        ast = (SAD_AST*)CreateSadAstBinaryLogicalExp( LOGICAL_EXP_TYPE_SAD_AST_OR, (SAD_AST_EXP*)(yyvsp[-2].ast), (SAD_AST_EXP*)(yyvsp[0].ast) );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating or expression" );
            yyerror(NULL); return 1;
        } 
        (yyval.ast) = ast; 
    }
#line 1531 "analysis_def_parser.tab.c"
    break;

  case 13:
#line 152 "analysis_def_parser.y"
                       {
        ast = (SAD_AST*)CreateSadAstUnaryLogicalExp( LOGICAL_EXP_TYPE_SAD_AST_NOT, (SAD_AST_EXP*)(yyvsp[0].ast) );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating not expression" );
            yyerror(NULL); return 1;
        } 
        (yyval.ast) = ast; 
    }
#line 1544 "analysis_def_parser.tab.c"
    break;

  case 14:
#line 160 "analysis_def_parser.y"
                                        {
        (yyval.ast) = (yyvsp[-1].ast);     
    }
#line 1552 "analysis_def_parser.tab.c"
    break;

  case 15:
#line 166 "analysis_def_parser.y"
                           {
        ast = (SAD_AST*)CreateSadAstCompExp( COMP_EXP_TYPE_SAD_AST_LE, (SAD_AST_EXP*)(yyvsp[-2].ast), (SAD_AST_EXP*)(yyvsp[0].ast) );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating less than or equal to expression" );
            yyerror(NULL); return 1;
        } 
        (yyval.ast) = ast; 
    }
#line 1565 "analysis_def_parser.tab.c"
    break;

  case 16:
#line 174 "analysis_def_parser.y"
                             {
        ast = (SAD_AST*)CreateSadAstCompExp( COMP_EXP_TYPE_SAD_AST_LT, (SAD_AST_EXP*)(yyvsp[-2].ast), (SAD_AST_EXP*)(yyvsp[0].ast) );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating less than expression" );
            yyerror(NULL); return 1;
        } 
        (yyval.ast) = ast; 
    }
#line 1578 "analysis_def_parser.tab.c"
    break;

  case 17:
#line 182 "analysis_def_parser.y"
                             {
        ast = (SAD_AST*)CreateSadAstCompExp( COMP_EXP_TYPE_SAD_AST_GE, (SAD_AST_EXP*)(yyvsp[-2].ast), (SAD_AST_EXP*)(yyvsp[0].ast) );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating greater than or equal to expression" );
            yyerror(NULL); return 1;
        } 
        (yyval.ast) = ast; 
    }
#line 1591 "analysis_def_parser.tab.c"
    break;

  case 18:
#line 190 "analysis_def_parser.y"
                             {
        ast = (SAD_AST*)CreateSadAstCompExp( COMP_EXP_TYPE_SAD_AST_GT, (SAD_AST_EXP*)(yyvsp[-2].ast), (SAD_AST_EXP*)(yyvsp[0].ast) );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating greater than expression" );
            yyerror(NULL); return 1;
        } 
        (yyval.ast) = ast; 
    }
#line 1604 "analysis_def_parser.tab.c"
    break;

  case 19:
#line 198 "analysis_def_parser.y"
                             {
        ast = (SAD_AST*)CreateSadAstCompExp( COMP_EXP_TYPE_SAD_AST_EQ, (SAD_AST_EXP*)(yyvsp[-2].ast), (SAD_AST_EXP*)(yyvsp[0].ast) );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating equal expression" );
            yyerror(NULL); return 1;
        } 
        (yyval.ast) = ast; 
    }
#line 1617 "analysis_def_parser.tab.c"
    break;

  case 20:
#line 206 "analysis_def_parser.y"
                                      {
        (yyval.ast) = (yyvsp[-1].ast);     
    }
#line 1625 "analysis_def_parser.tab.c"
    break;

  case 21:
#line 212 "analysis_def_parser.y"
                             {
        ast = (SAD_AST*)CreateSadAstBinaryNumExp( NUM_EXP_TYPE_SAD_AST_PLUS, (SAD_AST_EXP*)(yyvsp[-2].ast), (SAD_AST_EXP*)(yyvsp[0].ast) );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating plus expression" );
            yyerror(NULL); return 1;
        } 
        (yyval.ast) = ast; 
    }
#line 1638 "analysis_def_parser.tab.c"
    break;

  case 22:
#line 220 "analysis_def_parser.y"
                                {
        ast = (SAD_AST*)CreateSadAstBinaryNumExp( NUM_EXP_TYPE_SAD_AST_MINUS, (SAD_AST_EXP*)(yyvsp[-2].ast), (SAD_AST_EXP*)(yyvsp[0].ast) );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating minus expression" );
            yyerror(NULL); return 1;
        } 
        (yyval.ast) = ast; 
    }
#line 1651 "analysis_def_parser.tab.c"
    break;

  case 23:
#line 228 "analysis_def_parser.y"
                                {
        ast = (SAD_AST*)CreateSadAstBinaryNumExp( NUM_EXP_TYPE_SAD_AST_TIMES, (SAD_AST_EXP*)(yyvsp[-2].ast), (SAD_AST_EXP*)(yyvsp[0].ast) );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating times expression" );
            yyerror(NULL); return 1;
        } 
        (yyval.ast) = ast; 
    }
#line 1664 "analysis_def_parser.tab.c"
    break;

  case 24:
#line 236 "analysis_def_parser.y"
                              {
        ast = (SAD_AST*)CreateSadAstBinaryNumExp( NUM_EXP_TYPE_SAD_AST_DIV, (SAD_AST_EXP*)(yyvsp[-2].ast), (SAD_AST_EXP*)(yyvsp[0].ast) );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating div expression" );
            yyerror(NULL); return 1;
        } 
        (yyval.ast) = ast; 
    }
#line 1677 "analysis_def_parser.tab.c"
    break;

  case 25:
#line 244 "analysis_def_parser.y"
                                     {
        ast = (SAD_AST*)CreateSadAstUnaryNumExp( NUM_EXP_TYPE_SAD_AST_UMINUS, (SAD_AST_EXP*)(yyvsp[0].ast) );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating unary minus expression" );
            yyerror(NULL); return 1;
        } 
        (yyval.ast) = ast; 
    }
#line 1690 "analysis_def_parser.tab.c"
    break;

  case 26:
#line 252 "analysis_def_parser.y"
                                    {
        (yyval.ast) = (yyvsp[-1].ast);     
    }
#line 1698 "analysis_def_parser.tab.c"
    break;

  case 27:
#line 255 "analysis_def_parser.y"
                                            {
        ast = (SAD_AST*)CreateSadAstFuncExp( "exp", 1, (SAD_AST_EXP*)(yyvsp[-1].ast) );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating exp function" );
            yyerror(NULL); return 1;
        } 
        (yyval.ast) = ast; 
    }
#line 1711 "analysis_def_parser.tab.c"
    break;

  case 28:
#line 263 "analysis_def_parser.y"
                                                              {
        ast = (SAD_AST*)CreateSadAstFuncExp( "pow", 2, (SAD_AST_EXP*)(yyvsp[-3].ast), (SAD_AST_EXP*)(yyvsp[-1].ast) );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating pow function" );
            yyerror(NULL); return 1;
        } 
        (yyval.ast) = ast; 
    }
#line 1724 "analysis_def_parser.tab.c"
    break;

  case 29:
#line 271 "analysis_def_parser.y"
                                            {
        ast = (SAD_AST*)CreateSadAstFuncExp( "log", 1, (SAD_AST_EXP*)(yyvsp[-1].ast) );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating log function" );
            yyerror(NULL); return 1;
        } 
        (yyval.ast) = ast; 
    }
#line 1737 "analysis_def_parser.tab.c"
    break;

  case 30:
#line 279 "analysis_def_parser.y"
                   { 
        ast = (SAD_AST*)CreateSadAstConstant( (yyvsp[0].value) );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating time variable" );
            yyerror(NULL); return 1;
        } 
        (yyval.ast) = ast; 
    }
#line 1750 "analysis_def_parser.tab.c"
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
    }
#line 1763 "analysis_def_parser.tab.c"
    break;

  case 32:
#line 295 "analysis_def_parser.y"
                                { 
        species = FindSpeciesFromID( (yyvsp[0].string) );
        if( species == NULL ) {
            PrintSadAstErrorMessage( "%s is not a valid species ID", (yyvsp[0].string) );
            yyerror(NULL); return 1;
        } 
        ast = (SAD_AST*)CreateSadAstSpeciesCon( species );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating species node for %s", (yyvsp[0].string) );
            yyerror(NULL); return 1;
        } 
        (yyval.ast) = ast; 
        FREE((yyvsp[0].string));
    }
#line 1782 "analysis_def_parser.tab.c"
    break;

  case 33:
#line 309 "analysis_def_parser.y"
                             { 
        species = FindSpeciesFromID( (yyvsp[0].string) );
        if( species == NULL ) {
            PrintSadAstErrorMessage( "%s is not a valid species ID", (yyvsp[0].string) );
            yyerror(NULL); return 1;
        } 
        ast = (SAD_AST*)CreateSadAstSpeciesCnt( species );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating species node for %s", (yyvsp[0].string) );
            yyerror(NULL); return 1;
        } 
        (yyval.ast) = ast; 
        FREE((yyvsp[0].string));
    }
#line 1801 "analysis_def_parser.tab.c"
    break;

  case 34:
#line 323 "analysis_def_parser.y"
                     { 
        species = FindSpeciesFromID( (yyvsp[0].string) );
        if( species == NULL ) {
            PrintSadAstErrorMessage( "%s is not a valid species ID", (yyvsp[0].string) );
            yyerror(NULL); return 1;
        } 
        ast = (SAD_AST*)CreateSadAstSpeciesCnt( species );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating species node for %s", (yyvsp[0].string) );
            yyerror(NULL); return 1;
        } 
        (yyval.ast) = ast; 
        FREE((yyvsp[0].string));
    }
#line 1820 "analysis_def_parser.tab.c"
    break;

  case 35:
#line 337 "analysis_def_parser.y"
                              { 
        reaction = FindReactionFromID( (yyvsp[0].string) );
        if( reaction == NULL ) {
            PrintSadAstErrorMessage( "%s is not a valid reaction ID", (yyvsp[0].string) );
            yyerror(NULL); return 1;
        } 
        ast = (SAD_AST*)CreateSadAstReactionCnt( reaction );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating reaction node for %s", (yyvsp[0].string) );
            yyerror(NULL); return 1;
        }
        (yyval.ast) = ast; 
        FREE((yyvsp[0].string));
    }
#line 1839 "analysis_def_parser.tab.c"
    break;


#line 1843 "analysis_def_parser.tab.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *, YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
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

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;

  /* Do not reclaim the symbols of the rule whose action triggered
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
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
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

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


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


#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif


/*-----------------------------------------------------.
| yyreturn -- parsing is finished, return the result.  |
`-----------------------------------------------------*/
yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[+*yyssp], yyvsp);
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
  return yyresult;
}
#line 355 "analysis_def_parser.y"

