/***************************************************************************
 *   Copyright (C) 2004 by Hiroyuki Kuwahara                               *
 *   kuwahara@cs.utah.edu                                                  *
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
#if !defined(HAVE_KINETIC_LAW)
#define HAVE_KINETIC_LAW

#include "common.h"
#include "util.h"
#include "compartment_manager.h"
#include "species_node.h"
#include "symtab.h"
#include "random_number_generator.h"

BEGIN_C_NAMESPACE

#define KINETIC_LAW_OP_PLUS '+'
#define KINETIC_LAW_OP_MINUS '-'
#define KINETIC_LAW_OP_TIMES '*'
#define KINETIC_LAW_OP_DIVIDE '/'
#define KINETIC_LAW_OP_POW '^'

#define KINETIC_LAW_OP_AND '&'
#define KINETIC_LAW_OP_OR '|'
#define KINETIC_LAW_OP_XOR 'x'
#define KINETIC_LAW_UNARY_OP_NOT '~'
#define KINETIC_LAW_OP_IMPLIES '?'
#define KINETIC_LAW_OP_MAX 'M'
#define KINETIC_LAW_OP_MIN 'm'
#define KINETIC_LAW_OP_QUOTIENT ':'

#define KINETIC_LAW_OP_EQ '='
#define KINETIC_LAW_OP_NEQ 'N'
#define KINETIC_LAW_OP_GEQ 'G'
#define KINETIC_LAW_OP_GT '>'
#define KINETIC_LAW_OP_LEQ 'L'
#define KINETIC_LAW_OP_LT '<'

#define KINETIC_LAW_OP_PW 'P'

#define KINETIC_LAW_OP_DELAY 'd'

#define KINETIC_LAW_UNARY_OP_ABS 'a'
#define KINETIC_LAW_UNARY_OP_CEILING '['
#define KINETIC_LAW_UNARY_OP_EXP 'e'
#define KINETIC_LAW_UNARY_OP_FACTORIAL '!'
#define KINETIC_LAW_UNARY_OP_FLOOR ']'
#define KINETIC_LAW_UNARY_OP_LN 'l'
#define KINETIC_LAW_OP_LOG 'g'
#define KINETIC_LAW_UNARY_OP_NEG '_'
#define KINETIC_LAW_OP_ROOT 'r'

#define KINETIC_LAW_UNARY_OP_COS 'c'
#define KINETIC_LAW_UNARY_OP_COSH 'C'
#define KINETIC_LAW_UNARY_OP_SIN 's'
#define KINETIC_LAW_UNARY_OP_SINH 'S'
#define KINETIC_LAW_UNARY_OP_TAN 't'
#define KINETIC_LAW_UNARY_OP_TANH 'T'

#define KINETIC_LAW_UNARY_OP_COT '('
#define KINETIC_LAW_UNARY_OP_COTH ')'
#define KINETIC_LAW_UNARY_OP_CSC '@'
#define KINETIC_LAW_UNARY_OP_CSCH ';'
#define KINETIC_LAW_UNARY_OP_SEC '$'
#define KINETIC_LAW_UNARY_OP_SECH '#'

#define KINETIC_LAW_UNARY_OP_ARCCOS '0'
#define KINETIC_LAW_UNARY_OP_ARCCOSH '1'
#define KINETIC_LAW_UNARY_OP_ARCSIN '2'
#define KINETIC_LAW_UNARY_OP_ARCSINH '3'
#define KINETIC_LAW_UNARY_OP_ARCTAN '4'
#define KINETIC_LAW_UNARY_OP_ARCTANH '5'

#define KINETIC_LAW_UNARY_OP_ARCCOT '6'
#define KINETIC_LAW_UNARY_OP_ARCCOTH '7'
#define KINETIC_LAW_UNARY_OP_ARCCSC '8'
#define KINETIC_LAW_UNARY_OP_ARCCSCH '9'
#define KINETIC_LAW_UNARY_OP_ARCSEC 'q'
#define KINETIC_LAW_UNARY_OP_ARCSECH 'Q'

#define KINETIC_LAW_OP_UNIFORM 'U'
#define KINETIC_LAW_OP_NORMAL 'n'
#define KINETIC_LAW_OP_GAMMA 'A'
#define KINETIC_LAW_OP_BINOMIAL 'b'
#define KINETIC_LAW_OP_LOGNORMAL 'o'
#define KINETIC_LAW_UNARY_OP_EXPRAND 'E'
#define KINETIC_LAW_UNARY_OP_POISSON 'p'
#define KINETIC_LAW_UNARY_OP_CHISQ 'H'
#define KINETIC_LAW_UNARY_OP_LAPLACE 'Z'
#define KINETIC_LAW_UNARY_OP_CAUCHY 'Y'
#define KINETIC_LAW_UNARY_OP_RAYLEIGH 'R'
#define KINETIC_LAW_UNARY_OP_BERNOULLI 'u'

#define KINETIC_LAW_UNARY_OP_RATE 'v'
#define KINETIC_LAW_UNARY_OP_BITWISE_NOT 'I'
#define KINETIC_LAW_OP_BITWISE_OR 'O'
#define KINETIC_LAW_OP_BITWISE_AND 'D'
#define KINETIC_LAW_OP_BITWISE_XOR 'X'
#define KINETIC_LAW_UNARY_OP_INT 'i'
#define KINETIC_LAW_OP_BIT 'B'
#define KINETIC_LAW_OP_MOD '%'

#define KINETIC_LAW_VALUE_TYPE_PW ((BYTE)1)
#define KINETIC_LAW_VALUE_TYPE_OP ((BYTE)2)
#define KINETIC_LAW_VALUE_TYPE_UNARY_OP ((BYTE)3)
#define KINETIC_LAW_VALUE_TYPE_INT ((BYTE)4)
#define KINETIC_LAW_VALUE_TYPE_REAL ((BYTE)5)   
#define KINETIC_LAW_VALUE_TYPE_COMPARTMENT ((BYTE)6)
#define KINETIC_LAW_VALUE_TYPE_SPECIES ((BYTE)7)
#define KINETIC_LAW_VALUE_TYPE_SYMBOL ((BYTE)8)
#define KINETIC_LAW_VALUE_TYPE_FUNCTION_SYMBOL ((BYTE)9)

#define SQR(x) ((x)*(x))
#define SQRT(x) pow((x),(.5))

struct _KINETIC_LAW_PW;
typedef struct _KINETIC_LAW_PW KINETIC_LAW_PW;

struct _KINETIC_LAW_OP;
typedef struct _KINETIC_LAW_OP KINETIC_LAW_OP;

struct _KINETIC_LAW_UNARY_OP;
typedef struct _KINETIC_LAW_UNARY_OP KINETIC_LAW_UNARY_OP;

struct _KINETIC_LAW;
typedef struct _KINETIC_LAW  KINETIC_LAW;

struct _KINETIC_LAW_VISITOR;
typedef struct _KINETIC_LAW_VISITOR KINETIC_LAW_VISITOR;

struct _KINETIC_LAW_PW {
    BYTE opType;
    LINKED_LIST *children;
};

struct _KINETIC_LAW_OP {
    BYTE opType;
    struct _KINETIC_LAW *left;
    struct _KINETIC_LAW *right;
    REB2SAC_SYMBOL *time;
    LINKED_LIST *values;
};

struct _KINETIC_LAW_UNARY_OP {
    BYTE opType;
    struct _KINETIC_LAW *child;
};

struct _KINETIC_LAW {
    union {
        KINETIC_LAW_PW pw;
        KINETIC_LAW_OP op;
        KINETIC_LAW_UNARY_OP unaryOp;
        long intValue;
        double realValue;
        COMPARTMENT *compartment;
        SPECIES *species;
        REB2SAC_SYMBOL *symbol;
        char *funcSymbol;
    } value;
    BYTE valueType;
    RET_VAL (*AcceptPostOrder)( KINETIC_LAW *law, KINETIC_LAW_VISITOR *visitor );
    RET_VAL (*AcceptPreOrder)( KINETIC_LAW *law, KINETIC_LAW_VISITOR *visitor );
    RET_VAL (*AcceptInOrder)( KINETIC_LAW *law, KINETIC_LAW_VISITOR *visitor );
    RET_VAL (*Accept)( KINETIC_LAW *law, KINETIC_LAW_VISITOR *visitor );
};

 
struct _KINETIC_LAW_VISITOR {
    CADDR_T _internal1;
    CADDR_T _internal2;
    CADDR_T _internal3;
    RET_VAL (*VisitPW)( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
    RET_VAL (*VisitOp)( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
    RET_VAL (*VisitUnaryOp)( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
    RET_VAL (*VisitInt)( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
    RET_VAL (*VisitReal)( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
    RET_VAL (*VisitCompartment)( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
    RET_VAL (*VisitSpecies)( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
    RET_VAL (*VisitSymbol)( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
    RET_VAL (*VisitFunctionSymbol)( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
};


KINETIC_LAW *CreateKineticLaw();
KINETIC_LAW *CreateIntValueKineticLaw( long value );
KINETIC_LAW *CreateRealValueKineticLaw( double value );
KINETIC_LAW *CreateCompartmentKineticLaw( COMPARTMENT *compartment );
KINETIC_LAW *CreateSpeciesKineticLaw( SPECIES *species );
KINETIC_LAW *CreateSymbolKineticLaw( REB2SAC_SYMBOL *symbol );
KINETIC_LAW *CreateFunctionKineticLaw( char * funcId, KINETIC_LAW *functionDef, LINKED_LIST *arguments, KINETIC_LAW **children, int num );
KINETIC_LAW *CreatePWKineticLaw( BYTE opType, LINKED_LIST *children );
KINETIC_LAW *CreateDelayKineticLaw( BYTE opType, KINETIC_LAW *left, KINETIC_LAW *right, REB2SAC_SYMBOL *time );
KINETIC_LAW *CreateOpKineticLaw( BYTE opType, KINETIC_LAW *left, KINETIC_LAW *right );
KINETIC_LAW *CreateUnaryOpKineticLaw( BYTE opType, KINETIC_LAW *child );
KINETIC_LAW *CreateFunctionSymbolKineticLaw( char *funcSymbol );

KINETIC_LAW *CloneKineticLaw( KINETIC_LAW *law );
LINKED_LIST *CloneChildren( LINKED_LIST *children );

RET_VAL SetIntValueKineticLaw( KINETIC_LAW *law, long value );
RET_VAL SetRealValueKineticLaw( KINETIC_LAW *law, double value );
RET_VAL SetCompartmentKineticLaw( KINETIC_LAW *law, COMPARTMENT *compartment );
RET_VAL SetSpeciesKineticLaw( KINETIC_LAW *law, SPECIES *species );
RET_VAL SetSymbolKineticLaw( KINETIC_LAW *law, REB2SAC_SYMBOL *symbol );
RET_VAL SetFunctionSymbolKineticLaw( KINETIC_LAW *law, char *funcSymbol );
RET_VAL SetPWKineticLaw( KINETIC_LAW *law, BYTE opType, LINKED_LIST *children );
RET_VAL SetOpKineticLaw( KINETIC_LAW *law, BYTE opType, KINETIC_LAW *left, KINETIC_LAW *right );
RET_VAL SetUnaryOpKineticLaw( KINETIC_LAW *law, BYTE opType, KINETIC_LAW *child );
RET_VAL SetValuesKineticLaw(KINETIC_LAW *law, LINKED_LIST *list);

BOOL IsIntValueKineticLaw(KINETIC_LAW *law);
BOOL IsRealValueKineticLaw(KINETIC_LAW *law);
BOOL IsCompartmentKineticLaw(KINETIC_LAW *law);
BOOL IsSpeciesKineticLaw(KINETIC_LAW *law);
BOOL IsSymbolKineticLaw(KINETIC_LAW *law);
BOOL IsFunctionSymbolKineticLaw(KINETIC_LAW *law);
BOOL IsPWKineticLaw(KINETIC_LAW *law);
BOOL IsOpKineticLaw(KINETIC_LAW *law);
BOOL IsUnaryOpKineticLaw(KINETIC_LAW *law);
BOOL IsConstantValueKineticLaw(KINETIC_LAW *law);

long GetIntValueFromKineticLaw(KINETIC_LAW *law);
double GetRealValueFromKineticLaw(KINETIC_LAW *law);
COMPARTMENT *GetCompartmentFromKineticLaw(KINETIC_LAW *law);
SPECIES *GetSpeciesFromKineticLaw(KINETIC_LAW *law);
REB2SAC_SYMBOL *GetSymbolFromKineticLaw(KINETIC_LAW *law);
char *GetFunctionSymbolFromKineticLaw(KINETIC_LAW *law);
BYTE GetPWTypeFromKineticLaw(KINETIC_LAW *law);
REB2SAC_SYMBOL *GetTimeFromKineticLaw(KINETIC_LAW *law);
LINKED_LIST *GetValuesFromKineticLaw(KINETIC_LAW *law);
BYTE GetOpTypeFromKineticLaw(KINETIC_LAW *law);
BYTE GetUnaryOpTypeFromKineticLaw(KINETIC_LAW *law);
LINKED_LIST *GetPWChildrenFromKineticLaw(KINETIC_LAW *law);
KINETIC_LAW *GetOpLeftFromKineticLaw(KINETIC_LAW *law);
KINETIC_LAW *GetOpRightFromKineticLaw(KINETIC_LAW *law);
KINETIC_LAW *GetUnaryOpChildFromKineticLaw(KINETIC_LAW *law);
void FreeKineticLaw(KINETIC_LAW **law);

RET_VAL ReplaceSpeciesWithIntInKineticLaw( KINETIC_LAW *law, SPECIES *from, long to );
RET_VAL ReplaceSpeciesWithRealInKineticLaw( KINETIC_LAW *law, SPECIES *from, double to );
RET_VAL ReplaceSpeciesWithKineticLawInKineticLaw( KINETIC_LAW *law, SPECIES *from, KINETIC_LAW * to );
RET_VAL ReplaceFunctionSymbolWithKineticLawInKineticLaw( KINETIC_LAW *law, char *from, KINETIC_LAW * to );

RET_VAL ReplaceConstantWithAnotherConstantInKineticLaw( KINETIC_LAW *law, double from, double to );

STRING *ToStringKineticLaw( KINETIC_LAW *law );

BOOL AreKineticLawsStructurallyEqual( KINETIC_LAW *a, KINETIC_LAW *b );

RET_VAL SimplifyInitialAssignment( KINETIC_LAW *kineticLaw );

END_C_NAMESPACE

#endif
