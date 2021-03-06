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
#include "symtab.h"
#include "kinetic_law.h"

static REB2SAC_SYMBOL *_AddRealValueSymbol( REB2SAC_SYMTAB *symtab, char *proposedID, double value, BOOL isConstant );
static REB2SAC_SYMBOL *_AddSpeciesRefSymbol( REB2SAC_SYMTAB *symtab, char *proposedID, double value, BOOL isConstant );
static REB2SAC_SYMBOL *_AddSymbol( REB2SAC_SYMTAB *symtab, char *proposedID, REB2SAC_SYMBOL *symbol );
static REB2SAC_SYMBOL *_Lookup( REB2SAC_SYMTAB *symtab, char *id );
static REB2SAC_SYMBOL *_LookupRecursively( REB2SAC_SYMTAB *symtab, char *id );
static LINKED_LIST *_GenerateListOfSymbols( REB2SAC_SYMTAB *symtab );
static STRING *_CreateID( REB2SAC_SYMTAB *symtab, char *proposedID );

STRING *GetSymbolID( REB2SAC_SYMBOL *sym ) {
    START_FUNCTION("GetSymbolID");

    if( sym == NULL ) {
        END_FUNCTION("GetSymbolID", FAILING );
        return NULL;
    }
        
    END_FUNCTION("GetSymbolID", SUCCESS );
    return sym->id;
}


double GetRealValueInSymbol( REB2SAC_SYMBOL *sym ) {
    START_FUNCTION("GetRealValueInSymbol");

    if( sym == NULL ) {
        END_FUNCTION("GetRealValueInSymbol", FAILING );
        return 0.0/0.0;
    }
    END_FUNCTION("GetRealValueInSymbol", SUCCESS );
    return sym->value;
}

UNIT_DEFINITION *GetUnitsInSymbol( REB2SAC_SYMBOL *sym ) {
    START_FUNCTION("GetUnitsInSymbol");

    if( sym == NULL ) {
        END_FUNCTION("GetUnitsInSymbol", FAILING );
        return NULL;
    }
        
    END_FUNCTION("GetUnitsInSymbol", SUCCESS );
    return sym->units;
}

double GetCurrentRealValueInSymbol( REB2SAC_SYMBOL *sym ) {
    START_FUNCTION("GetRealValueInSymbol");

    if( sym == NULL ) {
        END_FUNCTION("GetRealValueInSymbol", FAILING );
        return 0.0/0.0;
    }
    END_FUNCTION("GetRealValueInSymbol", SUCCESS );
    return sym->currentRealValue;
}

double GetCurrentRateInSymbol( REB2SAC_SYMBOL *sym ) {
    START_FUNCTION("GetRealValueInSymbol");

    if( sym == NULL ) {
        END_FUNCTION("GetRealValueInSymbol", FAILING );
        return 0.0/0.0;
    }
        
    END_FUNCTION("GetRealValueInSymbol", SUCCESS );
    return sym->currentRate;
}

struct KINETIC_LAW *GetInitialAssignmentInSymbol( REB2SAC_SYMBOL *sym ) {
    START_FUNCTION("GetInitialAssignmentInSymbol");

    if( sym == NULL ) {
        END_FUNCTION("GetInitialAssignmentInSymbol", FAILING );
        return NULL;
    }
        
    END_FUNCTION("GetInitialAssignmentInSymbol", SUCCESS );
    return sym->initialAssignment;
}

BOOL IsSymbolConstant( REB2SAC_SYMBOL *sym ) {
    START_FUNCTION("IsSymbolConstant");

    if( sym == NULL ) {
        END_FUNCTION("IsSymbolConstant", FAILING );
        return FALSE;
    }
        
    END_FUNCTION("IsSymbolConstant", SUCCESS );
    return sym->isConstant;
}

BOOL IsSymbolAlgebraic( REB2SAC_SYMBOL *sym ) {
    START_FUNCTION("IsSymbolConstant");

    if( sym == NULL ) {
        END_FUNCTION("IsSymbolConstant", FAILING );
        return FALSE;
    }
        
    END_FUNCTION("IsSymbolConstant", SUCCESS );
    return sym->algebraic;
}

BOOL PrintSymbol( REB2SAC_SYMBOL *sym ) {
    START_FUNCTION("PrintSymbol");

    if( sym == NULL ) {
        END_FUNCTION("PrintSymbol", FAILING );
        return FALSE;
    }
        
    END_FUNCTION("PrintSymbol", SUCCESS );
    return sym->print;
}

BOOL IsRealValueSymbol( REB2SAC_SYMBOL *sym ) {
    START_FUNCTION("IsSymbolConstant");

    if( sym == NULL ) {
        END_FUNCTION("IsSymbolConstant", FAILING );
        return FALSE;
    }
        
    END_FUNCTION("IsSymbolConstant", SUCCESS );
    //return ( sym->type == REB2SAC_SYMBOL_TYPE_REAL ? TRUE : FALSE );
    return TRUE;
}

RET_VAL SetRealValueInSymbol( REB2SAC_SYMBOL *sym, double value ) {
    
    START_FUNCTION("SetRealValueInSymbol");

    if( sym == NULL ) {
        END_FUNCTION("SetRealValueInSymbol", FAILING );
        return FAILING;
    }
    
    //sym->type = REB2SAC_SYMBOL_TYPE_REAL;    
    sym->value = value;
    sym->currentRealValue = value;
    
    END_FUNCTION("SetRealValueInSymbol", SUCCESS );
    return SUCCESS;
}

RET_VAL SetUnitsInSymbol( REB2SAC_SYMBOL *sym, UNIT_DEFINITION *units ) {
    
    START_FUNCTION("SetUnitsInSymbol");

    if( sym == NULL ) {
        END_FUNCTION("SetUnitsInSymbol", FAILING );
        return FAILING;
    }
    
    sym->units = units;
    
    END_FUNCTION("SetUnitsInSymbol", SUCCESS );
    return SUCCESS;
}

RET_VAL SetSymbolAlgebraic( REB2SAC_SYMBOL *sym, BOOL algebraic ) {
    
    START_FUNCTION("SetSymbolAlgebraic");

    if( sym == NULL ) {
        END_FUNCTION("SetSymbolAlgebraic", FAILING );
        return FAILING;
    }
    
    sym->algebraic = algebraic;
    
    END_FUNCTION("SetSymbolAlgebraic", SUCCESS );
    return SUCCESS;
}

RET_VAL SetPrintSymbol( REB2SAC_SYMBOL *sym, BOOL print ) {
    
    START_FUNCTION("SetPrintSymbol");

    if( sym == NULL ) {
        END_FUNCTION("SetPrintSymbol", FAILING );
        return FAILING;
    }
    
    sym->print = print;
    
    END_FUNCTION("SetPrintSymbol", SUCCESS );
    return SUCCESS;
}

RET_VAL SetCurrentRealValueInSymbol( REB2SAC_SYMBOL *sym, double value ) {
    
    START_FUNCTION("SetRealValueInSymbol");

    if( sym == NULL ) {
        END_FUNCTION("SetRealValueInSymbol", FAILING );
        return FAILING;
    }
    
    //sym->type = REB2SAC_SYMBOL_TYPE_REAL;    
    sym->currentRealValue = value;
    
    END_FUNCTION("SetRealValueInSymbol", SUCCESS );
    return SUCCESS;
}

RET_VAL SetCurrentRateInSymbol( REB2SAC_SYMBOL *sym, double rate ) {
    
    START_FUNCTION("SetRateInSymbol");

    if( sym == NULL ) {
        END_FUNCTION("SetRateInSymbol", FAILING );
        return FAILING;
    }
    sym->currentRate = rate;
    
    END_FUNCTION("SetRateInSymbol", SUCCESS );
    return SUCCESS;
}

RET_VAL SetInitialAssignmentInSymbol( REB2SAC_SYMBOL *sym, struct KINETIC_LAW *law ) {
    
    START_FUNCTION("SetInitialAssignmentInSymbol");

    if( sym == NULL ) {
        END_FUNCTION("SetInitialAssignmentInSymbol", FAILING );
        return FAILING;
    }
    
    sym->initialAssignment = law;
    
    END_FUNCTION("SetInitialAssignment", SUCCESS );
    return SUCCESS;
}

static REB2SAC_SYMBOL *_CloneSymbol( REB2SAC_SYMBOL *sym ) {
    REB2SAC_SYMBOL *clone = NULL;

    START_FUNCTION("_CloneSymbol");

    if( sym == NULL ) {
        END_FUNCTION("_CloneSymbol", FAILING );
        return NULL;
    }
    
    if( ( clone = (REB2SAC_SYMBOL*)MALLOC( sizeof(REB2SAC_SYMBOL) ) ) == NULL ) {
        END_FUNCTION("_CloneSymbol", FAILING );
        return NULL;
    }
    
    if( ( clone->id = CloneString( sym->id ) ) == NULL ) {
        END_FUNCTION("_CloneSymbol", FAILING );
        return NULL;
    }
    memcpy( &(clone->value), &(sym->value), sizeof(clone) - sizeof(clone->id) ); 
    clone->type = sym->type;
    clone->isConstant = sym->isConstant;
    clone->units = sym->units;
    clone->initialAssignment = sym->initialAssignment;
    clone->algebraic = sym->algebraic;
   
    END_FUNCTION("_CloneSymbol", SUCCESS );
    return clone;
}


static RET_VAL _FreeSymbol( REB2SAC_SYMBOL **sym ) {
    REB2SAC_SYMBOL *target = NULL;

    START_FUNCTION("_FreeSymbol");

    target = *sym;    
    if( target == NULL ) {
        END_FUNCTION("_FreeSymbol", SUCCESS );
        return SUCCESS;
    }
    
    FreeString( (&target->id) );
    FREE( *sym );
        
    END_FUNCTION("_FreeSymbol", SUCCESS );
    return SUCCESS;
}


REB2SAC_SYMTAB *CreateSymtab( REB2SAC_SYMTAB *parent ) {
    REB2SAC_SYMTAB *symtab = NULL;

    START_FUNCTION("CreateSymtab");
    
    if( ( symtab = (REB2SAC_SYMTAB*)MALLOC( sizeof(REB2SAC_SYMTAB) ) ) == NULL ) {
        END_FUNCTION("CreateSymtab", FAILING );    
        return NULL;
    }
    
    if( ( symtab->table = CreateHashTable( 16 ) ) == NULL ) {
        END_FUNCTION("CreateSymtab", FAILING );    
        return NULL;
    } 

    symtab->parent = parent;
    symtab->AddRealValueSymbol = _AddRealValueSymbol;
    symtab->AddSpeciesRefSymbol = _AddSpeciesRefSymbol;
    symtab->AddSymbol = _AddSymbol;
    symtab->Lookup = _Lookup;
    symtab->LookupRecursively = _LookupRecursively;
    symtab->GenerateListOfSymbols = _GenerateListOfSymbols;

    END_FUNCTION("CreateSymtab", SUCCESS );    
    return symtab;
}

REB2SAC_SYMTAB *CloneSymtab( REB2SAC_SYMTAB *symtab ) {
    STRING *id = NULL;
    REB2SAC_SYMBOL *symbol = NULL;
    REB2SAC_SYMTAB *clone = NULL;
    LINKED_LIST *list = NULL;

    START_FUNCTION("CloneSymtab");
    
    if( ( clone = (REB2SAC_SYMTAB*)MALLOC( sizeof(REB2SAC_SYMTAB) ) ) == NULL ) {
        END_FUNCTION("CloneSymtab", FAILING );    
        return NULL;
    }
    
    if( ( clone->table = CreateHashTable( 16 ) ) == NULL ) {
        END_FUNCTION("CloneSymtab", FAILING );    
        return NULL;
    } 
    
    clone->parent = symtab->parent;
    clone->AddRealValueSymbol = symtab->AddRealValueSymbol;
    clone->AddSpeciesRefSymbol = symtab->AddSpeciesRefSymbol;
    clone->AddSymbol = symtab->AddSymbol;
    clone->Lookup = symtab->Lookup;
    clone->LookupRecursively = symtab->LookupRecursively;
    clone->GenerateListOfSymbols = symtab->GenerateListOfSymbols;

    if( ( list = symtab->GenerateListOfSymbols( symtab ) ) == NULL ) {
        END_FUNCTION("CloneSymtab", FAILING );    
        return NULL;
    } 
    
    ResetCurrentElement( list );
    while( ( symbol = (REB2SAC_SYMBOL*)GetNextFromLinkedList( list ) ) != NULL ) {
        id = GetSymbolID( symbol );
        if( clone->AddSymbol( clone, GetCharArrayOfString( id ), _CloneSymbol( symbol ) ) == NULL ) {
            END_FUNCTION("CloneSymtab", FAILING );    
            return NULL;
        }        
    }        
    
    DeleteLinkedList( &list );
        
    END_FUNCTION("CloneSymtab", SUCCESS );    
    return clone;
}


RET_VAL FreeSymtab( REB2SAC_SYMTAB **symtab ) {
    RET_VAL ret = SUCCESS;
    REB2SAC_SYMBOL *symbol = NULL;
    REB2SAC_SYMTAB *target = NULL;
    LINKED_LIST *list = NULL;
    
    START_FUNCTION("CloneSymtab");
    
    target = *symtab;
    if( ( list = target->GenerateListOfSymbols( target ) ) == NULL ) {
        END_FUNCTION("CloneSymtab", FAILING );    
        return FAILING;
    } 
    
    ResetCurrentElement( list );
    while( ( symbol = (REB2SAC_SYMBOL*)GetNextFromLinkedList( list ) ) != NULL ) {
        if( IS_FAILED( ( ret = _FreeSymbol( &symbol ) ) ) ) {
            END_FUNCTION("CloneSymtab", ret );    
            return ret;
        }
    }        
    
    DeleteLinkedList( &list );
    DeleteHashTable( &(target->table) );
    FREE( *symtab );
    
    END_FUNCTION("CloneSymtab", SUCCESS );    
    return ret;
}

static STRING *_CreateID( REB2SAC_SYMTAB *symtab, char *proposedID ) {
    int i = 0;
    char buf[256];
    char *suffixHead = NULL;
    STRING *id = NULL;
    REB2SAC_SYMBOL *symbol = NULL;
    HASH_TABLE *table = NULL;
    
    START_FUNCTION("_CreateID");
    
    table = symtab->table;    
    if( ( symbol = (REB2SAC_SYMBOL*)GetValueFromHashTable( proposedID, strlen( proposedID ), table ) ) == NULL ) {
        if( ( id = CreateString( proposedID ) ) == NULL ) {
            END_FUNCTION("_CreateID", FAILING );    
            return NULL;
        }
        TRACE_2("symbol id %s is created from the proposed id %s", proposedID, proposedID ); 
        END_FUNCTION("_CreateID", SUCCESS );    
        return id;
    }
    
    strcpy( buf, proposedID );
    suffixHead = buf + strlen( buf );
    
    for( i = 1; i < 1000; i++ ) {
        sprintf( suffixHead, "_%03i", i );
        if( ( symbol = (REB2SAC_SYMBOL*)GetValueFromHashTable( buf, strlen( buf ), table ) ) == NULL ) {
            if( ( id = CreateString( buf ) ) == NULL ) {
                END_FUNCTION("_CreateID", FAILING );    
                return NULL;
            }
            TRACE_2("symbol id %s is created from the proposed id %s", buf, proposedID ); 
            END_FUNCTION("_CreateID", SUCCESS );    
            return id;
        }
    }
        
    END_FUNCTION("_CreateID", FAILING );    
    return NULL;
}


static REB2SAC_SYMBOL *_AddRealValueSymbol( REB2SAC_SYMTAB *symtab, char *proposedID, double value, BOOL isConstant ) {
    STRING *id = NULL;
    REB2SAC_SYMBOL *symbol = NULL;
    HASH_TABLE *table = NULL;
    
    START_FUNCTION("_AddRealValueSymbol");
    
    if( ( id = _CreateID( symtab, proposedID ) ) == NULL ) {
        END_FUNCTION("_AddRealValueSymbol", FAILING );    
        return NULL;
    }
    
    if( ( symbol = (REB2SAC_SYMBOL*)MALLOC( sizeof(REB2SAC_SYMBOL) ) ) == NULL ) {
        END_FUNCTION("_AddRealValueSymbol", FAILING );    
        return NULL;
    }

    symbol->id = id;
    symbol->type = REB2SAC_SYMBOL_TYPE_REAL;
    symbol->initialAssignment = NULL;
    symbol->units = NULL;
    if( IS_FAILED( SetRealValueInSymbol( symbol, value ) ) ) {
        END_FUNCTION("_AddRealValueSymbol", FAILING );    
        return NULL;
    }
    if( IS_FAILED( SetCurrentRateInSymbol( symbol, 0.0 ) ) ) {
        END_FUNCTION("_AddRealValueSymbol", FAILING );    
        return NULL;
    }
    
    symbol->isConstant = isConstant;
    //if ((strcmp(proposedID,"t")==0) || (strcmp(proposedID,"time")==0) || isConstant) { 
    symbol->algebraic = FALSE;
    //} else {
    //  symbol->algebraic = TRUE;
    //}

    table = symtab->table;
    if( IS_FAILED( PutInHashTable( GetCharArrayOfString( id ), GetStringLength( id ), (CADDR_T)symbol, table ) ) ) {
        END_FUNCTION("_AddRealValueSymbol", FAILING );    
        return NULL;
    }
        
    END_FUNCTION("_AddRealValueSymbol", SUCCESS );    
    return symbol;
}

static REB2SAC_SYMBOL *_AddSpeciesRefSymbol( REB2SAC_SYMTAB *symtab, char *proposedID, double value, BOOL isConstant ) {
    STRING *id = NULL;
    REB2SAC_SYMBOL *symbol = NULL;
    HASH_TABLE *table = NULL;
    
    START_FUNCTION("_AddRealValueSymbol");
    
    if( ( id = _CreateID( symtab, proposedID ) ) == NULL ) {
        END_FUNCTION("_AddRealValueSymbol", FAILING );    
        return NULL;
    }
    
    if( ( symbol = (REB2SAC_SYMBOL*)MALLOC( sizeof(REB2SAC_SYMBOL) ) ) == NULL ) {
        END_FUNCTION("_AddRealValueSymbol", FAILING );    
        return NULL;
    }

    symbol->id = id;
    symbol->type = REB2SAC_SYMBOL_TYPE_SPECIES_REF;
    symbol->initialAssignment = NULL;
    symbol->units = NULL;
    if( IS_FAILED( SetRealValueInSymbol( symbol, value ) ) ) {
        END_FUNCTION("_AddRealValueSymbol", FAILING );    
        return NULL;
    }
    
    symbol->isConstant = isConstant;
    //if ((strcmp(proposedID,"t")==0) || (strcmp(proposedID,"time")==0) || isConstant) { 
    symbol->algebraic = FALSE;
    //} else {
    //  symbol->algebraic = TRUE;
    //}

    table = symtab->table;
    if( IS_FAILED( PutInHashTable( GetCharArrayOfString( id ), GetStringLength( id ), (CADDR_T)symbol, table ) ) ) {
        END_FUNCTION("_AddRealValueSymbol", FAILING );    
        return NULL;
    }
        
    END_FUNCTION("_AddRealValueSymbol", SUCCESS );    
    return symbol;
}

static REB2SAC_SYMBOL *_AddSymbol( REB2SAC_SYMTAB *symtab, char *proposedID, REB2SAC_SYMBOL *symbol ) {
    STRING *id = NULL;
    HASH_TABLE *table = NULL;
    
    START_FUNCTION("_AddSymbol");
    
    if( ( id = _CreateID( symtab, proposedID ) ) == NULL ) {
        END_FUNCTION("_AddRealValueSymbol", FAILING );    
        return NULL;
    }
    symbol->id = id;
    symbol->initialAssignment = NULL;
    symbol->units = NULL;
    symbol->algebraic = FALSE;
    table = symtab->table;
    if( IS_FAILED( PutInHashTable( GetCharArrayOfString( id ), GetStringLength( id ), (CADDR_T)symbol, table ) ) ) {
        END_FUNCTION("_AddRealValueSymbol", FAILING );    
        return NULL;
    }
        
    END_FUNCTION("_AddRealValueSymbol", SUCCESS );    
    return symbol;
}

static REB2SAC_SYMBOL *_Lookup( REB2SAC_SYMTAB *symtab, char *id ) {
    REB2SAC_SYMBOL *symbol = NULL;
    HASH_TABLE *table = NULL;
    
    START_FUNCTION("_Lookup");
    
    table = symtab->table;
    symbol = (REB2SAC_SYMBOL*)GetValueFromHashTable( id, strlen(id), table );        
    
    END_FUNCTION("_Lookup", SUCCESS );    
    return symbol;
}

static REB2SAC_SYMBOL *_LookupRecursively( REB2SAC_SYMTAB *symtab, char *id ) {
    REB2SAC_SYMBOL *symbol = NULL;
    REB2SAC_SYMTAB *target = NULL;
    
    START_FUNCTION("_LookupRecursively");
    
    target = symtab;    
    while( target != NULL ) {
        if( ( symbol = target->Lookup( target, id ) ) != NULL ) {
            END_FUNCTION("_LookupRecursively", SUCCESS );    
            return symbol;
        }
        target = target->parent;         
    }
    
    END_FUNCTION("_LookupRecursively", SUCCESS );    
    return NULL;
}

static LINKED_LIST *_GenerateListOfSymbols( REB2SAC_SYMTAB *symtab ) {
    LINKED_LIST *list = NULL;
    HASH_TABLE *table = NULL;
    
    START_FUNCTION("_GenerateListOfSymbols");
    
    table = symtab->table;
    if( ( list = GenerateValueList( table ) ) == NULL ) {
        END_FUNCTION("_GenerateListOfSymbols", FAILING );    
        return NULL;
    }
    
    END_FUNCTION("_GenerateListOfSymbols", SUCCESS );    
    return list;
}

