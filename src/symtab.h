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
#if !defined(HAVE_SYMTAB)
#define HAVE_SYMTAB

#include "common.h"
#include "hash_table.h"
#include "util.h"
#include "unit_manager.h"

BEGIN_C_NAMESPACE


struct _REB2SAC_SYMTAB;
typedef struct _REB2SAC_SYMTAB REB2SAC_SYMTAB;

#define REB2SAC_SYMBOL_TYPE_REAL ((BYTE)0)
#define REB2SAC_SYMBOL_TYPE_SPECIES_REF ((BYTE)1)

struct KINETIC_LAW;

typedef struct {
    STRING *id;
    double value;        
    double currentRealValue;
    double currentRate;
    BYTE type;
    BOOL isConstant;
    BOOL print;
    BOOL algebraic;
    UNIT_DEFINITION *units;
    struct KINETIC_LAW *initialAssignment;    
} REB2SAC_SYMBOL;

STRING *GetSymbolID( REB2SAC_SYMBOL *sym );
BOOL IsRealValueSymbol( REB2SAC_SYMBOL *sym );
double GetRealValueInSymbol( REB2SAC_SYMBOL *sym );
RET_VAL SetRealValueInSymbol( REB2SAC_SYMBOL *sym, double value );
double GetCurrentRealValueInSymbol( REB2SAC_SYMBOL *sym );
RET_VAL SetCurrentRealValueInSymbol( REB2SAC_SYMBOL *sym, double value );
double GetCurrentRateInSymbol( REB2SAC_SYMBOL *sym );
RET_VAL SetCurrentRateInSymbol( REB2SAC_SYMBOL *sym, double rate );
UNIT_DEFINITION *GetUnitsInSymbol( REB2SAC_SYMBOL *sym );
RET_VAL SetUnitsInSymbol( REB2SAC_SYMBOL *sym, UNIT_DEFINITION *units );
struct KINETIC_LAW *GetInitialAssignmentInSymbol( REB2SAC_SYMBOL *sym );
RET_VAL SetInitialAssignmentInSymbol( REB2SAC_SYMBOL *sym, struct KINETIC_LAW *law );
BOOL IsSymbolConstant( REB2SAC_SYMBOL *sym );
BOOL IsSymbolAlgebraic( REB2SAC_SYMBOL *sym );
RET_VAL SetSymbolAlgebraic( REB2SAC_SYMBOL *sym, BOOL algebraic );
BOOL PrintSymbol( REB2SAC_SYMBOL *sym );
RET_VAL SetPrintSymbol( REB2SAC_SYMBOL *sym, BOOL print );

struct _REB2SAC_SYMTAB {
    REB2SAC_SYMTAB *parent;
    HASH_TABLE *table;
    REB2SAC_SYMBOL *(*AddRealValueSymbol)( REB2SAC_SYMTAB *symtab, char *proposedID, double value, BOOL isConstant );
    REB2SAC_SYMBOL *(*AddSpeciesRefSymbol)( REB2SAC_SYMTAB *symtab, char *proposedID, double value, BOOL isConstant );
    REB2SAC_SYMBOL *(*AddSymbol)( REB2SAC_SYMTAB *symtab, char *proposedID, REB2SAC_SYMBOL *symbol );
    REB2SAC_SYMBOL *(*Lookup)( REB2SAC_SYMTAB *symtab, char *id );
    REB2SAC_SYMBOL *(*LookupRecursively)( REB2SAC_SYMTAB *symtab, char *id );
    LINKED_LIST *(*GenerateListOfSymbols)( REB2SAC_SYMTAB *symtab );
};

REB2SAC_SYMTAB *CreateSymtab( REB2SAC_SYMTAB *parent );
REB2SAC_SYMTAB *CloneSymtab( REB2SAC_SYMTAB *symtab );
RET_VAL FreeSymtab( REB2SAC_SYMTAB **symtab );

END_C_NAMESPACE

#endif
