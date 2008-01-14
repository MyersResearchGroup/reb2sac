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
#include "species_node.h"
#include "symtab.h"

BEGIN_C_NAMESPACE

#define KINETIC_LAW_OP_PLUS '+'
#define KINETIC_LAW_OP_MINUS '-'
#define KINETIC_LAW_OP_TIMES '*'
#define KINETIC_LAW_OP_DIVIDE '/'
#define KINETIC_LAW_OP_POW '^'

#define KINETIC_LAW_VALUE_TYPE_OP ((BYTE)1)
#define KINETIC_LAW_VALUE_TYPE_INT ((BYTE)2)
#define KINETIC_LAW_VALUE_TYPE_REAL ((BYTE)3)   
#define KINETIC_LAW_VALUE_TYPE_SPECIES ((BYTE)4)
#define KINETIC_LAW_VALUE_TYPE_SYMBOL ((BYTE)5)
#define KINETIC_LAW_VALUE_TYPE_FUNCTION_SYMBOL ((BYTE)6)

struct _KINETIC_LAW_OP;
typedef struct _KINETIC_LAW_OP KINETIC_LAW_OP;

struct _KINETIC_LAW;
typedef struct _KINETIC_LAW  KINETIC_LAW;

struct _KINETIC_LAW_VISITOR;
typedef struct _KINETIC_LAW_VISITOR KINETIC_LAW_VISITOR;

struct _KINETIC_LAW_OP {
    BYTE opType;
    struct _KINETIC_LAW *left;
    struct _KINETIC_LAW *right;
};


struct _KINETIC_LAW {
    union {
        KINETIC_LAW_OP op;
        long intValue;
        double realValue;
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
    RET_VAL (*VisitOp)( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
    RET_VAL (*VisitInt)( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
    RET_VAL (*VisitReal)( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
    RET_VAL (*VisitSpecies)( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
    RET_VAL (*VisitSymbol)( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
    RET_VAL (*VisitFunctionSymbol)( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
};


KINETIC_LAW *CreateKineticLaw();
KINETIC_LAW *CreateIntValueKineticLaw( long value );
KINETIC_LAW *CreateRealValueKineticLaw( double value );
KINETIC_LAW *CreateSpeciesKineticLaw( SPECIES *species );
KINETIC_LAW *CreateSymbolKineticLaw( REB2SAC_SYMBOL *symbol );
KINETIC_LAW *CreateFunctionKineticLaw( char * funcId, KINETIC_LAW *functionDef, LINKED_LIST *arguments, KINETIC_LAW **children, int num );
KINETIC_LAW *CreateOpKineticLaw( BYTE opType, KINETIC_LAW *left, KINETIC_LAW *right );

KINETIC_LAW *CloneKineticLaw( KINETIC_LAW *law );

RET_VAL SetIntValueKineticLaw( KINETIC_LAW *law, long value );
RET_VAL SetRealValueKineticLaw( KINETIC_LAW *law, double value );
RET_VAL SetSpeciesKineticLaw( KINETIC_LAW *law, SPECIES *species );
RET_VAL SetSymbolKineticLaw( KINETIC_LAW *law, REB2SAC_SYMBOL *symbol );
RET_VAL SetFunctionSymbolKineticLaw( KINETIC_LAW *law, char *funcSymbol );
RET_VAL SetOpKineticLaw( KINETIC_LAW *law, BYTE opType, KINETIC_LAW *left, KINETIC_LAW *right );

BOOL IsIntValueKineticLaw(KINETIC_LAW *law);
BOOL IsRealValueKineticLaw(KINETIC_LAW *law);
BOOL IsSpeciesKineticLaw(KINETIC_LAW *law);
BOOL IsSymbolKineticLaw(KINETIC_LAW *law);
BOOL IsFunctionSymbolKineticLaw(KINETIC_LAW *law);
BOOL IsOpKineticLaw(KINETIC_LAW *law);
BOOL IsConstantValueKineticLaw(KINETIC_LAW *law);

long GetIntValueFromKineticLaw(KINETIC_LAW *law);
double GetRealValueFromKineticLaw(KINETIC_LAW *law);
SPECIES *GetSpeciesFromKineticLaw(KINETIC_LAW *law);
REB2SAC_SYMBOL *GetSymbolFromKineticLaw(KINETIC_LAW *law);
char *GetFunctionSymbolFromKineticLaw(KINETIC_LAW *law);
BYTE GetOpTypeFromKineticLaw(KINETIC_LAW *law);
KINETIC_LAW *GetOpLeftFromKineticLaw(KINETIC_LAW *law);
KINETIC_LAW *GetOpRightFromKineticLaw(KINETIC_LAW *law);
void FreeKineticLaw(KINETIC_LAW **law);

RET_VAL ReplaceSpeciesWithIntInKineticLaw( KINETIC_LAW *law, SPECIES *from, long to );
RET_VAL ReplaceSpeciesWithRealInKineticLaw( KINETIC_LAW *law, SPECIES *from, double to );
RET_VAL ReplaceSpeciesWithKineticLawInKineticLaw( KINETIC_LAW *law, SPECIES *from, KINETIC_LAW * to );
RET_VAL ReplaceFunctionSymbolWithKineticLawInKineticLaw( KINETIC_LAW *law, char *from, KINETIC_LAW * to );

RET_VAL ReplaceConstantWithAnotherConstantInKineticLaw( KINETIC_LAW *law, double from, double to );

STRING *ToStringKineticLaw( KINETIC_LAW *law );

BOOL AreKineticLawsStructurallyEqual( KINETIC_LAW *a, KINETIC_LAW *b );

END_C_NAMESPACE

#endif
