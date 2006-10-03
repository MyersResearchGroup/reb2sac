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
#if defined(DEBUG)
#undef DEBUG
#endif


#include "kinetic_law.h"

#define TO_STRING_STRING_BUF_SIZE 64

static RET_VAL _AcceptPostOrderForOpKineticLaw( KINETIC_LAW *law, KINETIC_LAW_VISITOR *visitor );
static RET_VAL _AcceptPreOrderForOpKineticLaw( KINETIC_LAW *law, KINETIC_LAW_VISITOR *visitor );
static RET_VAL _AcceptInOrderForOpKineticLaw( KINETIC_LAW *law, KINETIC_LAW_VISITOR *visitor );
static RET_VAL _AcceptForOpKineticLaw( KINETIC_LAW *law, KINETIC_LAW_VISITOR *visitor );

static RET_VAL _AcceptForIntValueKineticLaw( KINETIC_LAW *law, KINETIC_LAW_VISITOR *visitor );
static RET_VAL _AcceptForRealValueKineticLaw( KINETIC_LAW *law, KINETIC_LAW_VISITOR *visitor );
static RET_VAL _AcceptForSpeciesKineticLaw( KINETIC_LAW *law, KINETIC_LAW_VISITOR *visitor );
static RET_VAL _AcceptForSymbolKineticLaw( KINETIC_LAW *law, KINETIC_LAW_VISITOR *visitor );


static KINETIC_LAW_VISITOR speciesReplacementVisitor;

static RET_VAL _VisitOpToReplaceSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitIntToReplaceSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitRealToReplaceSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSymbolToReplaceSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSpeciesToReplaceSpeciesWithInt( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSpeciesToReplaceSpeciesWithReal( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSpeciesToReplaceSpeciesWithKineticLaw( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );


static RET_VAL _VisitOpToReplaceConstant( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitIntToReplaceConstant( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitRealToReplaceConstant( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSpeciesToReplaceConstant( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSymbolToReplaceConstant( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );


static KINETIC_LAW_VISITOR toStringVisitor;
typedef struct {
    LINKED_LIST *stack;
    BYTE parentOp;
} TO_STRING_VISITOR_INTERNAL;
  
static RET_VAL _VisitOpToString( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitIntToString( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitRealToString( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSpeciesToString( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSymbolToString( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static BOOL _NeedParenForLeft( KINETIC_LAW *parent, KINETIC_LAW *child );
static BOOL _NeedParenForRight( KINETIC_LAW *parent, KINETIC_LAW *child );


KINETIC_LAW *CreateKineticLaw() {
    KINETIC_LAW *law = NULL;
    
    START_FUNCTION("CreateKineticLaw");

    if( ( law = (KINETIC_LAW*)CALLOC( 1, sizeof( KINETIC_LAW ) ) ) == NULL ) {
        END_FUNCTION("CreateKineticLaw", FAILING );        
        return NULL;
    }

    END_FUNCTION("CreateKineticLaw", SUCCESS );        
    return law;
}


KINETIC_LAW *CreateIntValueKineticLaw( long value ) {
    KINETIC_LAW *law = NULL;
    
    START_FUNCTION("CreateIntValueKineticLaw");

    if( ( law = (KINETIC_LAW*)CALLOC( 1, sizeof( KINETIC_LAW ) ) ) == NULL ) {
        END_FUNCTION("CreateIntValueKineticLaw", FAILING );        
        return NULL;
    }

    law->valueType = KINETIC_LAW_VALUE_TYPE_INT;
    law->value.intValue = value;
    law->AcceptPostOrder = _AcceptForIntValueKineticLaw;
    law->AcceptPreOrder = _AcceptForIntValueKineticLaw;
    law->AcceptInOrder = _AcceptForIntValueKineticLaw;
    law->Accept = _AcceptForIntValueKineticLaw;
    END_FUNCTION("CreateIntValueKineticLaw", SUCCESS );        
    return law;
}

KINETIC_LAW *CreateRealValueKineticLaw( double value ) {
    KINETIC_LAW *law = NULL;
    
    START_FUNCTION("CreateRealValueKineticLaw");

    if( ( law = (KINETIC_LAW*)CALLOC( 1, sizeof( KINETIC_LAW ) ) ) == NULL ) {
        END_FUNCTION("CreateRealValueKineticLaw", FAILING );        
        return NULL;
    }
    
    law->valueType = KINETIC_LAW_VALUE_TYPE_REAL;
    law->value.realValue = value;
    law->AcceptPostOrder = _AcceptForRealValueKineticLaw;
    law->AcceptPreOrder = _AcceptForRealValueKineticLaw;
    law->AcceptInOrder = _AcceptForRealValueKineticLaw;
    law->Accept = _AcceptForRealValueKineticLaw;
    END_FUNCTION("CreateRealValueKineticLaw", SUCCESS );        
    return law;
}


KINETIC_LAW *CreateSpeciesKineticLaw( SPECIES *species ) {
    KINETIC_LAW *law = NULL;
    
    START_FUNCTION("CreateSpeciesKineticLaw");

    if( ( law = (KINETIC_LAW*)CALLOC( 1, sizeof( KINETIC_LAW ) ) ) == NULL ) {
        END_FUNCTION("CreateSpeciesKineticLaw", FAILING );        
        return NULL;
    }

    law->valueType = KINETIC_LAW_VALUE_TYPE_SPECIES;
    law->value.species = species;
    law->AcceptPostOrder = _AcceptForSpeciesKineticLaw;
    law->AcceptPreOrder = _AcceptForSpeciesKineticLaw;
    law->AcceptInOrder = _AcceptForSpeciesKineticLaw;
    law->Accept = _AcceptForSpeciesKineticLaw;
    
    END_FUNCTION("CreateRealValueKineticLaw", SUCCESS );        
    return law;
}

KINETIC_LAW *CreateSymbolKineticLaw( REB2SAC_SYMBOL *symbol ) {
    KINETIC_LAW *law = NULL;
    
    START_FUNCTION("CreateSymbolKineticLaw");

    if( ( law = (KINETIC_LAW*)CALLOC( 1, sizeof( KINETIC_LAW ) ) ) == NULL ) {
        END_FUNCTION("CreateSymbolKineticLaw", FAILING );        
        return NULL;
    }

    law->valueType = KINETIC_LAW_VALUE_TYPE_SYMBOL;
    law->value.symbol = symbol;
    law->AcceptPostOrder = _AcceptForSymbolKineticLaw;
    law->AcceptPreOrder = _AcceptForSymbolKineticLaw;
    law->AcceptInOrder = _AcceptForSymbolKineticLaw;
    law->Accept = _AcceptForSymbolKineticLaw;
    
    END_FUNCTION("CreateSymbolKineticLaw", SUCCESS );        
    return law;
}


KINETIC_LAW *CreateOpKineticLaw( BYTE opType, KINETIC_LAW *left, KINETIC_LAW *right ) {
    KINETIC_LAW *law = NULL;
    
    START_FUNCTION("CreateRealValueKineticLaw");

    if( ( left == NULL ) || ( right == NULL ) ) {
        TRACE_0("the input children kinetic laws are NULL" );
        return NULL;
    }
    
    if( ( law = (KINETIC_LAW*)CALLOC( 1, sizeof( KINETIC_LAW ) ) ) == NULL ) {
        END_FUNCTION("CreateRealValueKineticLaw", FAILING );        
        return NULL;
    }
    
    law->valueType = KINETIC_LAW_VALUE_TYPE_OP;
    law->value.op.opType = opType;
    law->value.op.left = left;
    law->value.op.right = right;
    law->AcceptPostOrder = _AcceptPostOrderForOpKineticLaw;
    law->AcceptPreOrder = _AcceptPreOrderForOpKineticLaw;
    law->AcceptInOrder = _AcceptInOrderForOpKineticLaw;
    law->Accept = _AcceptForOpKineticLaw;
    
    END_FUNCTION("CreateRealValueKineticLaw", SUCCESS );        
    return law;
}


KINETIC_LAW *CloneKineticLaw( KINETIC_LAW *law ) {
    KINETIC_LAW *clone = NULL;
#ifdef DEBUG
    STRING *string = NULL;
#endif        
    
    START_FUNCTION("CloneKineticLaw");
    
    if( law == NULL ) {
        TRACE_0("NULL kinetic law cannot be cloned");
        return NULL;
    }
    
    if( ( clone = (KINETIC_LAW*)MALLOC(  sizeof( KINETIC_LAW ) ) ) == NULL ) {
        END_FUNCTION("CloneKineticLaw", FAILING );        
        return NULL;
    }
    
    if( law->valueType == KINETIC_LAW_VALUE_TYPE_OP ) {
        clone->valueType = KINETIC_LAW_VALUE_TYPE_OP;
        if( ( clone->value.op.left = CloneKineticLaw( law->value.op.left ) )  == NULL ) {
#ifdef DEBUG
            string = ToStringKineticLaw( law->value.op.left );
            printf( "could not create a clone of %s", GetCharArrayOfString( string ) );
            FreeString( &string );
#endif        
            END_FUNCTION("CloneKineticLaw", FAILING );        
            return NULL;
        }
        if( ( clone->value.op.right = CloneKineticLaw( law->value.op.right ) ) == NULL ) {
#ifdef DEBUG
            string = ToStringKineticLaw( law->value.op.right );
            printf( "could not create a clone of %s", GetCharArrayOfString( string ) );
            FreeString( &string );
#endif        
            END_FUNCTION("CloneKineticLaw", FAILING );        
            return NULL;
        }                        
        clone->value.op.opType = law->value.op.opType;
        clone->AcceptPostOrder = _AcceptPostOrderForOpKineticLaw;
        clone->AcceptPreOrder = _AcceptPreOrderForOpKineticLaw;
        clone->AcceptInOrder = _AcceptInOrderForOpKineticLaw;
        clone->Accept = _AcceptForOpKineticLaw;
        END_FUNCTION("CloneKineticLaw", SUCCESS );        
        return clone;
    }
    else {
        memcpy( (CADDR_T)clone, (CADDR_T)law, sizeof( KINETIC_LAW ) );        
        END_FUNCTION("CloneKineticLaw", SUCCESS );        
        return clone;
    }
}



RET_VAL SetIntValueKineticLaw( KINETIC_LAW *law, long value ) {
    
    START_FUNCTION("SetIntValueKineticLaw");
    
    if( law == NULL ) {
        END_FUNCTION("SetIntValueKineticLaw", FAILING );        
        return FAILING;
    }
    
    if( law->valueType == KINETIC_LAW_VALUE_TYPE_OP ) {
        FreeKineticLaw( &(law->value.op.left) );
        FreeKineticLaw( &(law->value.op.right) );        
    }
    
    law->valueType = KINETIC_LAW_VALUE_TYPE_INT;
    law->value.intValue = value;
    law->AcceptPostOrder = _AcceptForIntValueKineticLaw;
    law->AcceptPreOrder = _AcceptForIntValueKineticLaw;
    law->AcceptInOrder = _AcceptForIntValueKineticLaw;
    law->Accept = _AcceptForIntValueKineticLaw;
    END_FUNCTION("SetIntValueKineticLaw", SUCCESS );        
    return SUCCESS;
}

RET_VAL SetRealValueKineticLaw( KINETIC_LAW *law, double value ) {
    START_FUNCTION("SetRealValueKineticLaw");
    
    if( law == NULL ) {
        END_FUNCTION("SetRealValueKineticLaw", FAILING );        
        return FAILING;
    }
    
    if( law->valueType == KINETIC_LAW_VALUE_TYPE_OP ) {
        FreeKineticLaw( &(law->value.op.left) );
        FreeKineticLaw( &(law->value.op.right) );        
    }
    
    law->valueType = KINETIC_LAW_VALUE_TYPE_REAL;
    law->value.realValue = value;
    law->AcceptPostOrder = _AcceptForRealValueKineticLaw;
    law->AcceptPreOrder = _AcceptForRealValueKineticLaw;
    law->AcceptInOrder = _AcceptForRealValueKineticLaw;
    law->Accept = _AcceptForRealValueKineticLaw;
    END_FUNCTION("SetRealValueKineticLaw", SUCCESS );        
    return SUCCESS;
}

RET_VAL SetSpeciesKineticLaw( KINETIC_LAW *law, SPECIES *species ) {
    START_FUNCTION("SetSpeciesKineticLaw");
    
    if( law == NULL ) {
        END_FUNCTION("SetSpeciesKineticLaw", FAILING );        
        return FAILING;
    }
    
    if( law->valueType == KINETIC_LAW_VALUE_TYPE_OP ) {
        FreeKineticLaw( &(law->value.op.left) );
        FreeKineticLaw( &(law->value.op.right) );        
    }
    
    law->valueType = KINETIC_LAW_VALUE_TYPE_SPECIES;
    law->value.species = species;
    law->AcceptPostOrder = _AcceptForSpeciesKineticLaw;
    law->AcceptPreOrder = _AcceptForSpeciesKineticLaw;
    law->AcceptInOrder = _AcceptForSpeciesKineticLaw;
    law->Accept = _AcceptForSpeciesKineticLaw;
    
    END_FUNCTION("SetSpeciesKineticLaw", SUCCESS );        
    return SUCCESS;
}


RET_VAL SetSymbolKineticLaw( KINETIC_LAW *law, REB2SAC_SYMBOL *symbol ) {
    START_FUNCTION("SetSymbolKineticLaw");
    
    if( law == NULL ) {
        END_FUNCTION("SetSymbolKineticLaw", FAILING );        
        return FAILING;
    }
    
    if( law->valueType == KINETIC_LAW_VALUE_TYPE_OP ) {
        FreeKineticLaw( &(law->value.op.left) );
        FreeKineticLaw( &(law->value.op.right) );        
    }
    
    law->valueType = KINETIC_LAW_VALUE_TYPE_SYMBOL;
    law->value.symbol = symbol;
    law->AcceptPostOrder = _AcceptForSymbolKineticLaw;
    law->AcceptPreOrder = _AcceptForSymbolKineticLaw;
    law->AcceptInOrder = _AcceptForSymbolKineticLaw;
    law->Accept = _AcceptForSymbolKineticLaw;
    
    END_FUNCTION("SetSymbolKineticLaw", SUCCESS );        
    return SUCCESS;
}


RET_VAL SetOpKineticLaw( KINETIC_LAW *law, BYTE opType, KINETIC_LAW *left, KINETIC_LAW *right ) {
    START_FUNCTION("SetOpKineticLaw");
    
    if( law == NULL ) {
        END_FUNCTION("SetOpKineticLaw", FAILING );        
        return FAILING;
    }
    
    if( ( left == NULL ) || ( right == NULL ) ) {
        return ErrorReport( FAILING, "SetOpKineticLaw", "Cannot set NULL children in Op kinetic law");
    }
    
    if( law->valueType == KINETIC_LAW_VALUE_TYPE_OP ) {
        if( left != law->value.op.left ) {
            FreeKineticLaw( &(law->value.op.left) );
        }
        if( right != law->value.op.right ) {
            FreeKineticLaw( &(law->value.op.right) );
        }       
    }
    
    law->valueType = KINETIC_LAW_VALUE_TYPE_OP;
    law->value.op.opType = opType;
    law->value.op.left = left;
    law->value.op.right = right;
    law->AcceptPostOrder = _AcceptPostOrderForOpKineticLaw;
    law->AcceptPreOrder = _AcceptPreOrderForOpKineticLaw;
    law->AcceptInOrder = _AcceptInOrderForOpKineticLaw;
    law->Accept = _AcceptForOpKineticLaw;
    
    END_FUNCTION("SetOpKineticLaw", SUCCESS );        
    return SUCCESS;
}

BOOL IsIntValueKineticLaw(KINETIC_LAW *law) {
    START_FUNCTION("IsIntValueKineticLaw");
    
    if( law == NULL ) {
        END_FUNCTION("IsIntValueKineticLaw", SUCCESS );        
        return FALSE;
    }
        
    END_FUNCTION("IsIntValueKineticLaw", SUCCESS );        
    return ( law->valueType == KINETIC_LAW_VALUE_TYPE_INT ? TRUE : FALSE );
}

BOOL IsRealValueKineticLaw(KINETIC_LAW *law) {
    START_FUNCTION("IsRealValueKineticLaw");
    
    if( law == NULL ) {
        END_FUNCTION("IsRealValueKineticLaw", SUCCESS );        
        return FALSE;
    }
        
    END_FUNCTION("IsRealValueKineticLaw", SUCCESS );        
    return ( law->valueType == KINETIC_LAW_VALUE_TYPE_REAL ? TRUE : FALSE );
}

BOOL IsConstantValueKineticLaw(KINETIC_LAW *law) {
    REB2SAC_SYMBOL *sym = NULL;


    START_FUNCTION("IsConstantValueKineticLaw");
    
    if( law == NULL ) {
        END_FUNCTION("IsConstantValueKineticLaw", FAILING );        
        return FALSE;
    }
    
    if( ( law->valueType == KINETIC_LAW_VALUE_TYPE_REAL ) || ( law->valueType == KINETIC_LAW_VALUE_TYPE_INT ) ) {
        END_FUNCTION("IsConstantValueKineticLaw", SUCCESS );        
        return TRUE;
    }
    if( law->valueType == KINETIC_LAW_VALUE_TYPE_SYMBOL ) {
        sym = law->value.symbol;
        if( IsSymbolConstant( sym ) ) {
            END_FUNCTION("IsConstantValueKineticLaw", SUCCESS );        
            return TRUE;
        }
        else {
            END_FUNCTION("IsConstantValueKineticLaw", SUCCESS );        
            return FALSE;
        }
    }
        
    END_FUNCTION("IsConstantValueKineticLaw", SUCCESS );        
    return FALSE;
}


BOOL IsSpeciesKineticLaw(KINETIC_LAW *law) {
    START_FUNCTION("IsSpeciesKineticLaw");
    
    if( law == NULL ) {
        END_FUNCTION("IsSpeciesKineticLaw", SUCCESS );        
        return FALSE;
    }
        
    END_FUNCTION("IsSpeciesKineticLaw", SUCCESS );        
    return ( law->valueType == KINETIC_LAW_VALUE_TYPE_SPECIES ? TRUE : FALSE );
}


BOOL IsSymbolKineticLaw(KINETIC_LAW *law) {
    START_FUNCTION("IsSymbolKineticLaw");
    
    if( law == NULL ) {
        END_FUNCTION("IsSymbolKineticLaw", SUCCESS );        
        return FALSE;
    }
        
    END_FUNCTION("IsSymbolKineticLaw", SUCCESS );        
    return ( law->valueType == KINETIC_LAW_VALUE_TYPE_SYMBOL ? TRUE : FALSE );
}


BOOL IsOpKineticLaw(KINETIC_LAW *law) {
    START_FUNCTION("IsOpKineticLaw");
    
    if( law == NULL ) {
        END_FUNCTION("IsOpKineticLaw", SUCCESS );        
        return FALSE;
    }
        
    END_FUNCTION("IsOpKineticLaw", SUCCESS );        
    return ( law->valueType == KINETIC_LAW_VALUE_TYPE_OP ? TRUE : FALSE );
}

long GetIntValueFromKineticLaw(KINETIC_LAW *law) {
    START_FUNCTION("GetIntValueFromKineticLaw");
    
    if( law == NULL ) {
        END_FUNCTION("GetIntValueFromKineticLaw", SUCCESS );        
        return 0L;
    }
        
    END_FUNCTION("IsRealValueKineticLaw", SUCCESS );        
    return law->value.intValue;
}

double GetRealValueFromKineticLaw(KINETIC_LAW *law ) {
    START_FUNCTION("GetRealValueFromKineticLaw");
    
    if( law == NULL ) {
        END_FUNCTION("GetRealValueFromKineticLaw", SUCCESS );        
        return 0.0;
    }
        
    END_FUNCTION("GetRealValueFromKineticLaw", SUCCESS );        
    return law->value.realValue;
}

SPECIES *GetSpeciesFromKineticLaw(KINETIC_LAW *law) {
    START_FUNCTION("GetSpeciesFromKineticLaw");
    
    if( law == NULL ) {
        END_FUNCTION("GetSpeciesFromKineticLaw", SUCCESS );        
        return NULL;
    }
        
    END_FUNCTION("GetSpeciesFromKineticLaw", SUCCESS );        
    return law->value.species;
}


REB2SAC_SYMBOL *GetSymbolFromKineticLaw(KINETIC_LAW *law) {
    START_FUNCTION("GetSymbolFromKineticLaw");
    
    if( law == NULL ) {
        END_FUNCTION("GetSymbolFromKineticLaw", SUCCESS );        
        return NULL;
    }
        
    END_FUNCTION("GetSymbolFromKineticLaw", SUCCESS );        
    return law->value.symbol;
}



BYTE GetOpTypeFromKineticLaw(KINETIC_LAW *law) {
    START_FUNCTION("GetOpTypeFromKineticLaw");
    
    if( law == NULL ) {
        END_FUNCTION("GetOpTypeFromKineticLaw", SUCCESS );        
        return 0;
    }
        
    if( law->valueType != KINETIC_LAW_VALUE_TYPE_OP ) {
        END_FUNCTION("GetOpTypeFromKineticLaw", SUCCESS );        
        return 0;
    }
    
    END_FUNCTION("GetSymValueFromKineticLaw", SUCCESS );        
    return law->value.op.opType;
}

KINETIC_LAW *GetOpLeftFromKineticLaw(KINETIC_LAW *law) {
    START_FUNCTION("GetOpLeftFromKineticLaw");
    
    if( law == NULL ) {
        END_FUNCTION("GetOpLeftFromKineticLaw", SUCCESS );        
        return NULL;
    }
        
    if( law->valueType != KINETIC_LAW_VALUE_TYPE_OP ) {
        END_FUNCTION("GetOpLeftFromKineticLaw", SUCCESS );        
        return NULL;
    }
    
    END_FUNCTION("GetOpLeftFromKineticLaw", SUCCESS );        
    return law->value.op.left;
}


KINETIC_LAW *GetOpRightFromKineticLaw(KINETIC_LAW *law) {
    START_FUNCTION("GetOpRightFromKineticLaw");
    
    if( law == NULL ) {
        END_FUNCTION("GetOpRightFromKineticLaw", SUCCESS );        
        return NULL;
    }
        
    if( law->valueType != KINETIC_LAW_VALUE_TYPE_OP ) {
        END_FUNCTION("GetOpRightFromKineticLaw", SUCCESS );        
        return NULL;
    }
    
    END_FUNCTION("GetOpRightFromKineticLaw", SUCCESS );        
    return law->value.op.right;
}

void FreeKineticLaw(KINETIC_LAW **law) {
    START_FUNCTION("FreeKineticLaw");
    
    if( *law == NULL ) {
        END_FUNCTION("FreeKineticLaw", SUCCESS );        
        return;
    }
            
    if( (*law)->valueType == KINETIC_LAW_VALUE_TYPE_OP ) {
        FreeKineticLaw( &((*law)->value.op.left) );
        FreeKineticLaw( &((*law)->value.op.right) );
    }
    FREE( *law );
        
    END_FUNCTION("FreeKineticLaw", SUCCESS );        
}


RET_VAL ReplaceSpeciesWithIntInKineticLaw( KINETIC_LAW *law, SPECIES *from, long to ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("ReplaceSpeciesWithIntInKineticLaw");
    
    if( speciesReplacementVisitor.VisitOp == NULL ) {
        speciesReplacementVisitor.VisitOp = _VisitOpToReplaceSpecies;
        speciesReplacementVisitor.VisitInt = _VisitIntToReplaceSpecies;
        speciesReplacementVisitor.VisitReal = _VisitRealToReplaceSpecies;
        speciesReplacementVisitor.VisitSymbol = _VisitSymbolToReplaceSpecies;
    }
    speciesReplacementVisitor.VisitSpecies = _VisitSpeciesToReplaceSpeciesWithInt;
    
    speciesReplacementVisitor._internal1 = (CADDR_T)(&to);
    speciesReplacementVisitor._internal2 = (CADDR_T)from;
    
    if( IS_FAILED( ( ret = law->AcceptPostOrder( law, &speciesReplacementVisitor ) ) ) ) {
        END_FUNCTION("ReplaceSpeciesWithIntInKineticLaw", ret );
        return ret;        
    }
                
    END_FUNCTION("ReplaceSpeciesWithIntInKineticLaw", SUCCESS );        
    return ret;                        
}

RET_VAL ReplaceSpeciesWithRealInKineticLaw( KINETIC_LAW *law, SPECIES *from, double to ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("ReplaceSpeciesWithRealInKineticLaw");
    
    if( speciesReplacementVisitor.VisitOp == NULL ) {
        speciesReplacementVisitor.VisitOp = _VisitOpToReplaceSpecies;
        speciesReplacementVisitor.VisitInt = _VisitIntToReplaceSpecies;
        speciesReplacementVisitor.VisitReal = _VisitRealToReplaceSpecies;
        speciesReplacementVisitor.VisitSymbol = _VisitSymbolToReplaceSpecies;
    }
    speciesReplacementVisitor.VisitSpecies = _VisitSpeciesToReplaceSpeciesWithReal;
    
    TRACE_2( "replacing %s with %f", GetCharArrayOfString( GetSpeciesNodeName( from ) ), to );
    speciesReplacementVisitor._internal1 = (CADDR_T)(&to);
    speciesReplacementVisitor._internal2 = (CADDR_T)from;
    
    if( IS_FAILED( ( ret = law->AcceptPostOrder( law, &speciesReplacementVisitor ) ) ) ) {
        END_FUNCTION("ReplaceSpeciesWithRealInKineticLaw", ret );
        return ret;        
    }
    
    END_FUNCTION("ReplaceSpeciesWithRealInKineticLaw", SUCCESS );        
    return ret;                        
}

RET_VAL ReplaceSpeciesWithKineticLawInKineticLaw( KINETIC_LAW *law, SPECIES *from, KINETIC_LAW * to ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("ReplaceSpeciesWithKineticLawInKineticLaw");
    
    if( speciesReplacementVisitor.VisitOp == NULL ) {
        speciesReplacementVisitor.VisitOp = _VisitOpToReplaceSpecies;
        speciesReplacementVisitor.VisitInt = _VisitIntToReplaceSpecies;
        speciesReplacementVisitor.VisitReal = _VisitRealToReplaceSpecies;
        speciesReplacementVisitor.VisitSymbol = _VisitSymbolToReplaceSpecies;
    }
    speciesReplacementVisitor.VisitSpecies = _VisitSpeciesToReplaceSpeciesWithKineticLaw;
    
    speciesReplacementVisitor._internal1 = (CADDR_T)(to);
    speciesReplacementVisitor._internal2 = (CADDR_T)from;
    
    if( IS_FAILED( ( ret = law->AcceptPostOrder( law, &speciesReplacementVisitor ) ) ) ) {
        END_FUNCTION("ReplaceSpeciesWithKineticLawInKineticLaw", ret );
        return ret;        
    }
    
    END_FUNCTION("ReplaceSpeciesWithKineticLawInKineticLaw", SUCCESS );        
    return ret;                        
}



RET_VAL ReplaceConstantWithAnotherConstantInKineticLaw( KINETIC_LAW *law, double from, double to ) {
    static KINETIC_LAW_VISITOR visitor;
    RET_VAL ret = SUCCESS;
        
    START_FUNCTION("ReplaceConstantWithAnotherConstantInKineticLaw");
    
    if( visitor.VisitOp == NULL ) {
        visitor.VisitOp = _VisitOpToReplaceConstant;
        visitor.VisitInt = _VisitIntToReplaceConstant;
        visitor.VisitReal = _VisitRealToReplaceConstant;
        visitor.VisitSpecies = _VisitSpeciesToReplaceConstant;
        visitor.VisitSymbol = _VisitSymbolToReplaceConstant;
    }
    
    visitor._internal1 = (CADDR_T)(&from);
    visitor._internal2 = (CADDR_T)(&to);
    
    if( IS_FAILED( ( ret = law->AcceptPostOrder( law, &visitor ) ) ) ) {
        END_FUNCTION("ReplaceConstantWithAnotherConstantInKineticLaw", ret );
        return ret;        
    }
    
    END_FUNCTION("ReplaceConstantWithAnotherConstantInKineticLaw", SUCCESS );        
    return ret;                        
}

static RET_VAL _VisitOpToReplaceConstant( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    START_FUNCTION("_VisitOpToReplaceConstant");
    END_FUNCTION("_VisitOpToReplaceConstant", SUCCESS );        
    return SUCCESS;                        
}

static RET_VAL _VisitIntToReplaceConstant( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    double value = 0.0;
    double from = 0.0;
    double to = 0.0;

    START_FUNCTION("_VisitIntToReplaceConstant");
    
    from = *((double*)(visitor->_internal1)); 
    to = *((double*)(visitor->_internal2)); 
       
    value = (double)GetIntValueFromKineticLaw( kineticLaw );
    if( IS_REAL_EQUAL( from, value ) ) {
        if( IS_FAILED( ( ret = SetIntValueKineticLaw( kineticLaw, (long)to ) ) ) ) {
            END_FUNCTION("_VisitIntToReplaceConstant", ret );        
            return ret;                        
        }
    }
    
    END_FUNCTION("_VisitIntToReplaceConstant", SUCCESS );        
    return ret;                        
}

static RET_VAL _VisitRealToReplaceConstant( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    double value = 0.0;
    double from = 0.0;
    double to = 0.0;
    
    START_FUNCTION("_VisitRealToReplaceConstant");
    
    from = *((double*)(visitor->_internal1)); 
    to = *((double*)(visitor->_internal2)); 
       
    value = GetRealValueFromKineticLaw( kineticLaw );
    if( IS_REAL_EQUAL( from, value ) ) {
        if( IS_FAILED( ( ret = SetRealValueKineticLaw( kineticLaw, to ) ) ) ) {
            END_FUNCTION("_VisitIntToReplaceConstant", ret );        
            return ret;                        
        }
    }
    
    END_FUNCTION("_VisitRealToReplaceConstant", SUCCESS );        
    return SUCCESS;                        
}

static RET_VAL _VisitSymbolToReplaceConstant( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    double value = 0.0;
    double from = 0.0;
    double to = 0.0;
    REB2SAC_SYMBOL *symbol = NULL;
    
    START_FUNCTION("_VisitSymbolToReplaceConstant");
    
    from = *((double*)(visitor->_internal1)); 
    to = *((double*)(visitor->_internal2)); 
       
    symbol = GetSymbolFromKineticLaw( kineticLaw );
    if( !IsSymbolConstant( symbol ) ) {
        END_FUNCTION("_VisitSymbolToReplaceConstant", SUCCESS );        
        return SUCCESS;                        
    }
    if( !IsRealValueSymbol( symbol ) ) {
        END_FUNCTION("_VisitSymbolToReplaceConstant", SUCCESS );        
        return SUCCESS;                        
    }
    
    value = GetRealValueInSymbol( symbol );        
    if( IS_REAL_EQUAL( from, value ) ) {
        if( IS_FAILED( ( ret = SetRealValueInSymbol( symbol, to ) ) ) ) {
            END_FUNCTION("_VisitSymbolToReplaceConstant", ret );        
            return ret;                        
        }
    }
    
    END_FUNCTION("_VisitSymbolToReplaceConstant", SUCCESS );        
    return SUCCESS;                        
}


static RET_VAL _VisitSpeciesToReplaceConstant( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    START_FUNCTION("_VisitSpeciesToReplaceConstant");
    END_FUNCTION("_VisitSpeciesToReplaceConstant", SUCCESS );        
    return SUCCESS;                        
}



static RET_VAL _VisitOpToReplaceSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    START_FUNCTION("_VisitOpToReplaceSpecies");
    END_FUNCTION("_VisitOpToReplaceSpecies", SUCCESS );        
    return SUCCESS;                        
}

static RET_VAL _VisitIntToReplaceSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    START_FUNCTION("_VisitIntToReplaceSpecies");
    END_FUNCTION("_VisitIntToReplaceSpecies", SUCCESS );        
    return SUCCESS;                        
}

static RET_VAL _VisitRealToReplaceSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    START_FUNCTION("_VisitRealToReplaceSpecies");
    END_FUNCTION("_VisitRealToReplaceSpecies", SUCCESS );        
    return SUCCESS;                        
}

static RET_VAL _VisitSymbolToReplaceSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    START_FUNCTION("_VisitSymbolToReplaceSpecies");
    END_FUNCTION("_VisitSymbolToReplaceSpecies", SUCCESS );        
    return SUCCESS;                        
}


static RET_VAL _VisitSpeciesToReplaceSpeciesWithInt( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    long value = 0L;
    SPECIES *species = NULL;
    
    START_FUNCTION("_VisitSpeciesToReplaceSpeciesWithInt");
    
    value = *((long*)(visitor->_internal1));
    species = (SPECIES*)(visitor->_internal2);
    
    if( kineticLaw->value.species == species ) {
        TRACE_2( "replacing %s with %li", GetCharArrayOfString( GetSpeciesNodeName( species ) ), value );
        if( IS_FAILED( ( ret = SetIntValueKineticLaw( kineticLaw, value ) ) ) ) {
            END_FUNCTION("_VisitSpeciesToReplaceSpeciesWithInt", ret );
            return ret;                        
        }
    }
    END_FUNCTION("_VisitSpeciesToReplaceSpeciesWithInt", SUCCESS );        
    return ret;                        
}

static RET_VAL _VisitSpeciesToReplaceSpeciesWithReal( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    double value = 0.0;
    SPECIES *species = NULL;
    
    START_FUNCTION("_VisitSpeciesToReplaceSpeciesWithReal");
    
    value = *((double*)(visitor->_internal1));
    species = (SPECIES*)(visitor->_internal2);
    
    if( kineticLaw->value.species == species ) {
        TRACE_2( "replacing %s with %f", GetCharArrayOfString( GetSpeciesNodeName( species ) ), value );
        if( IS_FAILED( ( ret = SetRealValueKineticLaw( kineticLaw, value ) ) ) ) {
            END_FUNCTION("_VisitSpeciesToReplaceSpeciesWithReal", ret );
            return ret;                        
        }
    }
    END_FUNCTION("_VisitSpeciesToReplaceSpeciesWithReal", SUCCESS );        
    return ret;                        
}

static RET_VAL _VisitSpeciesToReplaceSpeciesWithKineticLaw( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    SPECIES *species = NULL;
    KINETIC_LAW *replacement = NULL;
    STRING *string = NULL;
    
    START_FUNCTION("_VisitSpeciesToReplaceSpeciesWithKineticLaw");
    
    species = (SPECIES*)(visitor->_internal2);
    
    if( kineticLaw->value.species == species ) {
        replacement = (KINETIC_LAW*)(visitor->_internal1);
#if DEBUG
        string = ToStringKineticLaw( replacement );
        printf( "replacing %s with %s" NEW_LINE, GetCharArrayOfString( GetSpeciesNodeName( species ) ), GetCharArrayOfString( string ) );
        FreeString( &string );
#endif
        if( replacement->valueType == KINETIC_LAW_VALUE_TYPE_OP ) {
            if( IS_FAILED( ( ret = SetOpKineticLaw( kineticLaw, replacement->value.op.opType, 
                CloneKineticLaw( replacement->value.op.left ), CloneKineticLaw( replacement->value.op.right ) ) ) ) ) {
                string = ToStringKineticLaw( replacement );
                return ErrorReport( FAILING, "_VisitSpeciesToReplaceSpeciesWithKineticLaw", "failed to create clone for %s", GetCharArrayOfString( string ) );
            } 
        }
        else {
            memcpy( kineticLaw, replacement, sizeof( KINETIC_LAW ) );
        }
    }
    END_FUNCTION("_VisitSpeciesToReplaceSpeciesWithKineticLaw", SUCCESS );        
    return ret;                        
}



static RET_VAL _AcceptPostOrderForOpKineticLaw( KINETIC_LAW *law, KINETIC_LAW_VISITOR *visitor ) {
    RET_VAL ret = SUCCESS;
    
    KINETIC_LAW *child = NULL;
    
    START_FUNCTION("_AcceptPostOrderForOpKineticLaw");
    
    child = law->value.op.left;
    if( IS_FAILED( ( ret = child->AcceptPostOrder( child, visitor ) ) ) ) {
        END_FUNCTION("_AcceptPostOrderForOpKineticLaw", ret );        
        return ret;                        
    }
    
    child = law->value.op.right;
    if( IS_FAILED( ( ret = child->AcceptPostOrder( child, visitor ) ) ) ) {
        END_FUNCTION("_AcceptPostOrderForOpKineticLaw", ret );        
        return ret;                        
    }            
    
    if( IS_FAILED( ( ret = visitor->VisitOp( visitor, law ) ) ) ) {
        END_FUNCTION("_AcceptPostOrderForOpKineticLaw", ret );        
        return ret;                        
    }
    
    END_FUNCTION("_AcceptPostOrderForOpKineticLaw", SUCCESS );        
    return ret;                        
}



static RET_VAL _AcceptPreOrderForOpKineticLaw( KINETIC_LAW *law, KINETIC_LAW_VISITOR *visitor ) {
    RET_VAL ret = SUCCESS;
    
    KINETIC_LAW *child = NULL;;
    
    START_FUNCTION("_AcceptPreOrderForOpKineticLaw");
    
    if( IS_FAILED( ( ret = visitor->VisitOp( visitor, law ) ) ) ) {
        END_FUNCTION("_AcceptPreOrderForOpKineticLaw", ret );        
        return ret;                        
    }
    
    child = law->value.op.left;
    if( IS_FAILED( ( ret = child->AcceptPreOrder( child, visitor ) ) ) ) {
        END_FUNCTION("_AcceptPreOrderForOpKineticLaw", ret );        
        return ret;                        
    }
    
    child = law->value.op.right;
    if( IS_FAILED( ( ret = child->AcceptPreOrder( child, visitor ) ) ) ) {
        END_FUNCTION("_AcceptPreOrderForOpKineticLaw", ret );        
        return ret;                        
    }            
    
    END_FUNCTION("_AcceptPreOrderForOpKineticLaw", SUCCESS );        
    return ret;                        
}

static RET_VAL _AcceptForOpKineticLaw( KINETIC_LAW *law, KINETIC_LAW_VISITOR *visitor ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("_AcceptForOpKineticLaw");
    
    if( IS_FAILED( ( ret = visitor->VisitOp( visitor, law ) ) ) ) {
        END_FUNCTION("_AcceptForOpKineticLaw", ret );        
        return ret;                        
    }
    
    END_FUNCTION("_AcceptForOpKineticLaw", SUCCESS );        
    return ret;                        
}



static RET_VAL _AcceptInOrderForOpKineticLaw( KINETIC_LAW *law, KINETIC_LAW_VISITOR *visitor ) {
    RET_VAL ret = SUCCESS;
    
    KINETIC_LAW *child = NULL;;
    
    START_FUNCTION("_AcceptInOrderForOpKineticLaw");
    
    child = law->value.op.left;
    if( IS_FAILED( ( ret = child->AcceptInOrder( child, visitor ) ) ) ) {
        END_FUNCTION("_AcceptInOrderForOpKineticLaw", ret );        
        return ret;                        
    }
    
    if( IS_FAILED( ( ret = visitor->VisitOp( visitor, law ) ) ) ) {
        END_FUNCTION("_AcceptInOrderForOpKineticLaw", ret );        
        return ret;                        
    }
        
    child = law->value.op.right;
    if( IS_FAILED( ( ret = child->AcceptInOrder( child, visitor ) ) ) ) {
        END_FUNCTION("_AcceptInOrderForOpKineticLaw", ret );        
        return ret;                        
    }            
    
    END_FUNCTION("_AcceptInOrderForOpKineticLaw", SUCCESS );        
    return ret;                        
}



static RET_VAL _AcceptForIntValueKineticLaw( KINETIC_LAW *law, KINETIC_LAW_VISITOR *visitor ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("_AcceptForIntValueKineticLaw");
    
    if( IS_FAILED( ( ret = visitor->VisitInt( visitor, law ) ) ) ) {
        END_FUNCTION("_AcceptForIntValueKineticLaw", ret );        
        return ret;                        
    }
    
    END_FUNCTION("_AcceptForIntValueKineticLaw", SUCCESS );        
    return ret;                        
}

static RET_VAL _AcceptForRealValueKineticLaw( KINETIC_LAW *law, KINETIC_LAW_VISITOR *visitor ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("_AcceptForRealValueKineticLaw");
    
    if( IS_FAILED( ( ret = visitor->VisitReal( visitor, law ) ) ) ) {
        END_FUNCTION("_AcceptForRealValueKineticLaw", ret );        
        return ret;                        
    }
    
    END_FUNCTION("_AcceptForRealValueKineticLaw", SUCCESS );        
    return ret;                        
}

static RET_VAL _AcceptForSpeciesKineticLaw( KINETIC_LAW *law, KINETIC_LAW_VISITOR *visitor ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("_AcceptForSpeciesKineticLaw");
    
    if( IS_FAILED( ( ret = visitor->VisitSpecies( visitor, law ) ) ) ) {
        END_FUNCTION("_AcceptForSpeciesKineticLaw", ret );        
        return ret;                        
    }
    
    END_FUNCTION("_AcceptForSpeciesKineticLaw", SUCCESS );        
    return ret;                        
}

static RET_VAL _AcceptForSymbolKineticLaw( KINETIC_LAW *law, KINETIC_LAW_VISITOR *visitor ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("_AcceptForSymbolKineticLaw");
    
    if( IS_FAILED( ( ret = visitor->VisitSymbol( visitor, law ) ) ) ) {
        END_FUNCTION("_AcceptForSymbolKineticLaw", ret );        
        return ret;                        
    }
    
    END_FUNCTION("_AcceptForSymbolKineticLaw", SUCCESS );        
    return ret;                        
}


STRING *ToStringKineticLaw( KINETIC_LAW *law ) {
    STRING *string = NULL;
    TO_STRING_VISITOR_INTERNAL internal;
    
    START_FUNCTION("ToStringKineticLaw");
    
    if( toStringVisitor.VisitOp == NULL ) {
        toStringVisitor.VisitOp = _VisitOpToString;
        toStringVisitor.VisitInt = _VisitIntToString;
        toStringVisitor.VisitReal = _VisitRealToString;
        toStringVisitor.VisitSpecies = _VisitSpeciesToString;
        toStringVisitor.VisitSymbol = _VisitSymbolToString;
    }
    if( ( internal.stack = CreateLinkedList() ) == NULL ) {
        END_FUNCTION("ToStringKineticLaw", FAILING );
        return NULL;        
    }    
    toStringVisitor._internal1 = (CADDR_T)(&internal); 
    
    if( IS_FAILED( law->AcceptPostOrder( law, &toStringVisitor ) ) ) {
        END_FUNCTION("ToStringKineticLaw", FAILING );
        return NULL;        
    }
    
    if( ( string = (STRING*)GetHeadFromLinkedList( internal.stack ) ) == NULL ) {
        END_FUNCTION("ToStringKineticLaw", FAILING );
        return NULL;        
    }    
    DeleteLinkedList( &(internal.stack) );
     
    END_FUNCTION("ToStringKineticLaw", SUCCESS );        
    return string;
}


static RET_VAL _VisitOpToString( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    BOOL leftStrParenFlag = FALSE;    
    BOOL rightStrParenFlag = FALSE;    
    int len = 0;
    char *buf = NULL;
    STRING *str = NULL;    
    STRING *leftStr = NULL;
    STRING *rightStr = NULL;
    KINETIC_LAW *left = NULL; 
    KINETIC_LAW *right = NULL; 
    LINKED_LIST *stack = NULL;
        
    TO_STRING_VISITOR_INTERNAL *internal = NULL;    
    
    START_FUNCTION("_VisitOpToString");

    internal = (TO_STRING_VISITOR_INTERNAL*)(visitor->_internal1);
    stack = internal->stack;
    
    len = 3;
    
    rightStr = (STRING*)GetTailFromLinkedList( stack );
    RemoveTailFromLinkedList( stack );
    len += GetStringLength( rightStr );
    
    leftStr = (STRING*)GetTailFromLinkedList( stack );
    RemoveTailFromLinkedList( stack );
    len += GetStringLength( leftStr );
        
    left = kineticLaw->value.op.left;
    if( _NeedParenForLeft( kineticLaw, left ) ) {
        leftStrParenFlag = TRUE;                
        len += 4;
    }
        
    right = kineticLaw->value.op.right;
    if( _NeedParenForRight( kineticLaw, right ) ) {
        rightStrParenFlag = TRUE;
        len += 4;
    }
    
    if( ( buf = (char*)MALLOC( len + 1) ) == NULL ) {
        return ErrorReport( FAILING, "_VisitOpToString", "failed to create %s %c %s", 
            GetCharArrayOfString( leftStr ),  kineticLaw->value.op.opType, GetCharArrayOfString( rightStr ) );
    }
    if( leftStrParenFlag && rightStrParenFlag ) {
        sprintf( buf, "( %s ) %c ( %s )", 
            GetCharArrayOfString( leftStr ),  kineticLaw->value.op.opType, GetCharArrayOfString( rightStr ) );
    }
    else if( leftStrParenFlag ) {
        sprintf( buf, "( %s ) %c %s", 
            GetCharArrayOfString( leftStr ),  kineticLaw->value.op.opType, GetCharArrayOfString( rightStr ) );
    }
    else if( rightStrParenFlag ) {
        sprintf( buf, "%s %c ( %s )", 
            GetCharArrayOfString( leftStr ),  kineticLaw->value.op.opType, GetCharArrayOfString( rightStr ) );
    }
    else {
        sprintf( buf, "%s %c %s", 
            GetCharArrayOfString( leftStr ),  kineticLaw->value.op.opType, GetCharArrayOfString( rightStr ) );
    }
#if 0
    if( leftStrParenFlag && rightStrParenFlag ) {
        sprintf( buf, "( %.*s ) %c ( %.*s )", 
            GetStringLength( leftStr ), GetCharArrayOfString( leftStr ),  kineticLaw->value.op.opType, GetStringLength( rightStr ), GetCharArrayOfString( rightStr ) );
    }
    else if( leftStrParenFlag ) {
        sprintf( buf, "( %.*s ) %c %.*s", 
            GetStringLength( leftStr ), GetCharArrayOfString( leftStr ),  kineticLaw->value.op.opType, GetStringLength( rightStr ), GetCharArrayOfString( rightStr ) );
    }
    else if( rightStrParenFlag ) {
        sprintf( buf, "%.*s %c ( %.*s )", 
            GetStringLength( leftStr ), GetCharArrayOfString( leftStr ),  kineticLaw->value.op.opType, GetStringLength( rightStr ), GetCharArrayOfString( rightStr ) );
    }
    else {
        sprintf( buf, "%.*s %c %.*s", 
            GetStringLength( leftStr ), GetCharArrayOfString( leftStr ),  kineticLaw->value.op.opType, GetStringLength( rightStr ), GetCharArrayOfString( rightStr ) );
    }
#endif
    
    FreeString( &leftStr );
    FreeString( &rightStr );
    
    if( ( str = CreateString( buf ) ) == NULL ) {
        return ErrorReport( FAILING, "_VisitIntToString", "could not create a string for %s", buf );
    }
    if( IS_FAILED( ( ret = AddElementInLinkedList( str, stack ) ) ) ) {
        return ErrorReport( FAILING, "_VisitIntToString", "could not add %s", buf );
    }
    FREE( buf );                           
    END_FUNCTION("_VisitOpToString", SUCCESS );        
    return ret;                        
}

static RET_VAL _VisitIntToString( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    char buf[TO_STRING_STRING_BUF_SIZE];
    STRING *str = NULL;    
    TO_STRING_VISITOR_INTERNAL *internal = NULL;    
    
    START_FUNCTION("_VisitIntToString");
    
    sprintf( buf, "%li", kineticLaw->value.intValue );
    if( ( str = CreateString( buf ) ) == NULL ) {
        return ErrorReport( FAILING, "_VisitIntToString", "could not create a string for %s", buf );
    }
    internal = (TO_STRING_VISITOR_INTERNAL*)(visitor->_internal1);
    if( IS_FAILED( ( ret = AddElementInLinkedList( str, internal->stack ) ) ) ) {
        return ErrorReport( FAILING, "_VisitIntToString", "could not add %s", buf );
    }
        
    END_FUNCTION("_VisitIntToString", SUCCESS );        
    return ret;                        
}

static RET_VAL _VisitRealToString( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    char buf[TO_STRING_STRING_BUF_SIZE];
    STRING *str = NULL;    
    TO_STRING_VISITOR_INTERNAL *internal = NULL;    
    
    START_FUNCTION("_VisitRealToString");
    
    sprintf( buf, "%g", kineticLaw->value.realValue );
    if( ( str = CreateString( buf ) ) == NULL ) {
        return ErrorReport( FAILING, "_VisitRealToString", "could not create a string for %s", buf );
    }
    internal = (TO_STRING_VISITOR_INTERNAL*)(visitor->_internal1);
    if( IS_FAILED( ( ret = AddElementInLinkedList( str, internal->stack ) ) ) ) {
        return ErrorReport( FAILING, "_VisitRealToString", "could not add %s", buf );
    }
    
    END_FUNCTION("_VisitRealToString", SUCCESS );        
    return ret;                        
}

static RET_VAL _VisitSpeciesToString( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    STRING *from = NULL;
    STRING *str = NULL;    
    TO_STRING_VISITOR_INTERNAL *internal = NULL;    
    
    START_FUNCTION("_VisitSpeciesToString");
    from = GetSpeciesNodeName( kineticLaw->value.species );
    if( ( str = CloneString( from ) ) == NULL ) {
        return ErrorReport( FAILING, "_VisitSpeciesToString", "could not create a string for %s", GetCharArrayOfString( from ) );
    }
    internal = (TO_STRING_VISITOR_INTERNAL*)(visitor->_internal1);
    if( IS_FAILED( ( ret = AddElementInLinkedList( str, internal->stack ) ) ) ) {
        return ErrorReport( FAILING, "_VisitSpeciesToString", "could not add %s", GetCharArrayOfString( from ) );
    }
    
    END_FUNCTION("_VisitSpeciesToString", SUCCESS );        
    return ret;                        
}

static RET_VAL _VisitSymbolToString( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    STRING *from = NULL;
    STRING *str = NULL;    
    TO_STRING_VISITOR_INTERNAL *internal = NULL;    
    
    START_FUNCTION("_VisitSymbolToString");
    from = GetSymbolID( kineticLaw->value.symbol );
    if( ( str = CloneString( from ) ) == NULL ) {
        return ErrorReport( FAILING, "_VisitSymbolToString", "could not create a string for %s", GetCharArrayOfString( from ) );
    }
    internal = (TO_STRING_VISITOR_INTERNAL*)(visitor->_internal1);
    if( IS_FAILED( ( ret = AddElementInLinkedList( str, internal->stack ) ) ) ) {
        return ErrorReport( FAILING, "_VisitSymbolToString", "could not add %s", GetCharArrayOfString( from ) );
    }
    
    END_FUNCTION("_VisitSymbolToString", SUCCESS );        
    return ret;                        
}


static BOOL _NeedParenForLeft( KINETIC_LAW *parent, KINETIC_LAW *child ) {
    BYTE parentOpType = 0;
    BYTE childOpType = 0;
    
    START_FUNCTION("_NeedParenForLeft");
    
    parentOpType = parent->value.op.opType;
    if( parentOpType == KINETIC_LAW_OP_POW ) {
        if( IsOpKineticLaw( child ) ) {
            END_FUNCTION("_NeedParenForLeft", SUCCESS );        
            return TRUE;
        }
        else {
            END_FUNCTION("_NeedParenForLeft", SUCCESS );        
            return FALSE;
        }
    } 
    if( IsOpKineticLaw( child ) ) {        
        childOpType = child->value.op.opType;
        if( ( parentOpType == KINETIC_LAW_OP_PLUS ) || ( parentOpType == KINETIC_LAW_OP_MINUS ) ) {
            END_FUNCTION("_NeedParenForLeft", SUCCESS );        
            return FALSE;
        }
        else {
            if( ( childOpType == KINETIC_LAW_OP_PLUS ) || ( childOpType == KINETIC_LAW_OP_MINUS ) ) {
                END_FUNCTION("_NeedParenForLeft", SUCCESS );        
                return TRUE;
            }
            else {
                END_FUNCTION("_NeedParenForLeft", SUCCESS );        
                return FALSE;
            }
        }                
    }
        
    END_FUNCTION("_NeedParenForLeft", SUCCESS );
    return FALSE;        
}


static BOOL _NeedParenForRight( KINETIC_LAW *parent, KINETIC_LAW *child ) {
    BYTE parentOpType = 0;
    BYTE childOpType = 0;
    
    START_FUNCTION("_NeedParenForRight");
    
    parentOpType = parent->value.op.opType;
    if( parentOpType == KINETIC_LAW_OP_POW ) {
        if( IsOpKineticLaw( child ) ) {
            END_FUNCTION("_NeedParenForRight", SUCCESS );        
            return TRUE;
        }
        else {
            END_FUNCTION("_NeedParenForRight", SUCCESS );        
            return FALSE;
        }
    } 
    if( IsOpKineticLaw( child ) ) {        
        childOpType = child->value.op.opType;
        if( childOpType == KINETIC_LAW_OP_POW ) {
            END_FUNCTION("_NeedParenForRight", SUCCESS );        
            return FALSE;
        }
        switch( parentOpType ) {
            case KINETIC_LAW_OP_TIMES:
                if( ( childOpType == KINETIC_LAW_OP_TIMES ) || ( childOpType == KINETIC_LAW_OP_DIVIDE ) ) {
                    END_FUNCTION("_NeedParenForRight", SUCCESS );        
                    return FALSE;
                }
                else {
                    END_FUNCTION("_NeedParenForRight", SUCCESS );        
                    return TRUE;
                }
            break;    
            
            case KINETIC_LAW_OP_DIVIDE:
                if( childOpType == KINETIC_LAW_OP_DIVIDE ) {
                    END_FUNCTION("_NeedParenForRight", SUCCESS );        
                    return FALSE;
                }
                else {
                    END_FUNCTION("_NeedParenForRight", SUCCESS );        
                    return TRUE;
                }
            break;    
            
            case KINETIC_LAW_OP_PLUS:
                END_FUNCTION("_NeedParenForRight", SUCCESS );        
            return FALSE;
            
            case KINETIC_LAW_OP_MINUS:
                if( ( childOpType == KINETIC_LAW_OP_TIMES ) || ( childOpType == KINETIC_LAW_OP_DIVIDE ) ) {
                    END_FUNCTION("_NeedParenForRight", SUCCESS );        
                    return FALSE;
                }
                else {
                    END_FUNCTION("_NeedParenForRight", SUCCESS );        
                    return TRUE;
                }
            break;                            
        }
        END_FUNCTION("_NeedParenForRight", SUCCESS );
        return TRUE;        
    }
    else {
        END_FUNCTION("_NeedParenForRight", SUCCESS );
        return FALSE;        
    }
}

BOOL AreKineticLawsStructurallyEqual( KINETIC_LAW *a, KINETIC_LAW *b ) {
    if( a->valueType != b->valueType ) {
        return FALSE;
    }
    switch( a->valueType ) {
        case KINETIC_LAW_VALUE_TYPE_OP:
            if( a->value.op.opType != b->value.op.opType ) {
                return FALSE;
            }
            return ( AreKineticLawsStructurallyEqual( a->value.op.left, b->value.op.left ) &&
                     AreKineticLawsStructurallyEqual( a->value.op.right, b->value.op.right ) ) ?
                    TRUE : FALSE;                     
        
        case KINETIC_LAW_VALUE_TYPE_INT:
            return ( a->value.intValue == b->value.intValue ) ? TRUE : FALSE; 
        
        case KINETIC_LAW_VALUE_TYPE_REAL:
            return ( a->value.realValue == b->value.realValue ) ? TRUE : FALSE; 
        
        case KINETIC_LAW_VALUE_TYPE_SPECIES:
            return ( a->value.species == b->value.species ) ? TRUE : FALSE; 
        
        case KINETIC_LAW_VALUE_TYPE_SYMBOL:
            return ( a->value.symbol == b->value.symbol ) ? TRUE : FALSE;             
    }    
}
