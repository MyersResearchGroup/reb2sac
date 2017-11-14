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

static RET_VAL _AcceptPostOrderForPWKineticLaw( KINETIC_LAW *law, KINETIC_LAW_VISITOR *visitor );
static RET_VAL _AcceptPreOrderForPWKineticLaw( KINETIC_LAW *law, KINETIC_LAW_VISITOR *visitor );
static RET_VAL _AcceptInOrderForPWKineticLaw( KINETIC_LAW *law, KINETIC_LAW_VISITOR *visitor );
static RET_VAL _AcceptForPWKineticLaw( KINETIC_LAW *law, KINETIC_LAW_VISITOR *visitor );

static RET_VAL _AcceptPostOrderForOpKineticLaw( KINETIC_LAW *law, KINETIC_LAW_VISITOR *visitor );
static RET_VAL _AcceptPreOrderForOpKineticLaw( KINETIC_LAW *law, KINETIC_LAW_VISITOR *visitor );
static RET_VAL _AcceptInOrderForOpKineticLaw( KINETIC_LAW *law, KINETIC_LAW_VISITOR *visitor );
static RET_VAL _AcceptForOpKineticLaw( KINETIC_LAW *law, KINETIC_LAW_VISITOR *visitor );

static RET_VAL _AcceptPostOrderForUnaryOpKineticLaw( KINETIC_LAW *law, KINETIC_LAW_VISITOR *visitor );
static RET_VAL _AcceptPreOrderForUnaryOpKineticLaw( KINETIC_LAW *law, KINETIC_LAW_VISITOR *visitor );
static RET_VAL _AcceptInOrderForUnaryOpKineticLaw( KINETIC_LAW *law, KINETIC_LAW_VISITOR *visitor );
static RET_VAL _AcceptForUnaryOpKineticLaw( KINETIC_LAW *law, KINETIC_LAW_VISITOR *visitor );

static RET_VAL _AcceptForIntValueKineticLaw( KINETIC_LAW *law, KINETIC_LAW_VISITOR *visitor );
static RET_VAL _AcceptForRealValueKineticLaw( KINETIC_LAW *law, KINETIC_LAW_VISITOR *visitor );
static RET_VAL _AcceptForCompartmentKineticLaw( KINETIC_LAW *law, KINETIC_LAW_VISITOR *visitor );
static RET_VAL _AcceptForSpeciesKineticLaw( KINETIC_LAW *law, KINETIC_LAW_VISITOR *visitor );
static RET_VAL _AcceptForSymbolKineticLaw( KINETIC_LAW *law, KINETIC_LAW_VISITOR *visitor );
static RET_VAL _AcceptForFunctionSymbolKineticLaw( KINETIC_LAW *law, KINETIC_LAW_VISITOR *visitor );


static KINETIC_LAW_VISITOR speciesReplacementVisitor;

static KINETIC_LAW_VISITOR compartmentReplacementVisitor;

static RET_VAL _VisitPWToReplaceSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitOpToReplaceSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitUnaryOpToReplaceSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitIntToReplaceSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitRealToReplaceSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSymbolToReplaceSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitCompartmentToReplaceSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitFunctionSymbolToReplaceSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSpeciesToReplaceSpeciesWithInt( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSpeciesToReplaceSpeciesWithReal( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSpeciesToReplaceSpeciesWithKineticLaw( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );

static RET_VAL _VisitPWToReplaceCompartment( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitOpToReplaceCompartment( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitUnaryOpToReplaceCompartment( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitIntToReplaceCompartment( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitRealToReplaceCompartment( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSymbolToReplaceCompartment( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitFunctionSymbolToReplaceCompartment( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitCompartmentToReplaceCompartmentWithInt( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitCompartmentToReplaceCompartmentWithReal( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitCompartmentToReplaceCompartmentWithKineticLaw( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );

static RET_VAL _VisitFunctionSymbolToReplaceWithKineticLaw( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );

static RET_VAL _VisitPWToReplaceConstant( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitOpToReplaceConstant( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitUnaryOpToReplaceConstant( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitIntToReplaceConstant( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitRealToReplaceConstant( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSpeciesToReplaceConstant( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitCompartmentToReplaceConstant( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSymbolToReplaceConstant( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitFunctionSymbolToReplaceConstant( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );


static KINETIC_LAW_VISITOR toStringVisitor;
typedef struct {
    LINKED_LIST *stack;
    BYTE parentOp;
} TO_STRING_VISITOR_INTERNAL;
  
static RET_VAL _VisitPWToString( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitOpToString( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitUnaryOpToString( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitIntToString( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitRealToString( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSpeciesToString( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitCompartmentToString( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSymbolToString( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitFunctionSymbolToString( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
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
    
    END_FUNCTION("CreateSpeciesKineticLaw", SUCCESS );        
    return law;
}

KINETIC_LAW *CreateCompartmentKineticLaw( COMPARTMENT *compartment ) {
    KINETIC_LAW *law = NULL;
    
    START_FUNCTION("CreateCompartmentKineticLaw");

    if( ( law = (KINETIC_LAW*)CALLOC( 1, sizeof( KINETIC_LAW ) ) ) == NULL ) {
        END_FUNCTION("CreateCompartmentKineticLaw", FAILING );        
        return NULL;
    }

    law->valueType = KINETIC_LAW_VALUE_TYPE_COMPARTMENT;
    law->value.compartment = compartment;
    law->AcceptPostOrder = _AcceptForCompartmentKineticLaw;
    law->AcceptPreOrder = _AcceptForCompartmentKineticLaw;
    law->AcceptInOrder = _AcceptForCompartmentKineticLaw;
    law->Accept = _AcceptForCompartmentKineticLaw;
    
    END_FUNCTION("CreateCompartmentKineticLaw", SUCCESS );        
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

KINETIC_LAW *CreateFunctionSymbolKineticLaw( char *funcSymbol ) {
    KINETIC_LAW *law = NULL;
    
    START_FUNCTION("CreateFunctionSymbolKineticLaw");

    if( ( law = (KINETIC_LAW*)CALLOC( 1, sizeof( KINETIC_LAW ) ) ) == NULL ) {
        END_FUNCTION("CreateFunctionSymbolKineticLaw", FAILING );        
        return NULL;
    }

    law->valueType = KINETIC_LAW_VALUE_TYPE_FUNCTION_SYMBOL;
    law->value.funcSymbol = funcSymbol;
    law->AcceptPostOrder = _AcceptForFunctionSymbolKineticLaw;
    law->AcceptPreOrder = _AcceptForFunctionSymbolKineticLaw;
    law->AcceptInOrder = _AcceptForFunctionSymbolKineticLaw;
    law->Accept = _AcceptForFunctionSymbolKineticLaw;
    
    END_FUNCTION("CreateFunctionSymbolKineticLaw", SUCCESS );        
    return law;
}

KINETIC_LAW *CreateFunctionKineticLaw( char *funcId, KINETIC_LAW *function, LINKED_LIST *arguments, KINETIC_LAW **children, int num ) {
    KINETIC_LAW *law = NULL;
    int i;
    char *argument = NULL;

    START_FUNCTION("CreateFunctionKineticLaw");

    for (i = 0; i < num; i++) {
      if (children[i] == NULL) {
        TRACE_0("the input children kinetic laws are NULL" );
        return NULL;
      }
    }

    if (GetLinkedListSize(arguments) != num) {
      printf( "number of arguments to function %s do not match", funcId );
      END_FUNCTION("CreateFunctionKineticLaw", FAILING );        
      return NULL;
    }

    if( ( law = (KINETIC_LAW*)CALLOC( 1, sizeof( KINETIC_LAW ) ) ) == NULL ) {
        END_FUNCTION("CreateFunctionKineticLaw", FAILING );        
        return NULL;
    }

    law = CloneKineticLaw(function);
    for (i = 0; i < num; i++) {
      argument = (char*)GetElementByIndex(i,arguments);
      if (ReplaceFunctionSymbolWithKineticLawInKineticLaw( law, argument, children[i])!=SUCCESS) {
	printf("problem during function parameter replacement");
        END_FUNCTION("CreateFunctionKineticLaw", FAILING );        
	return NULL;
      }
    }
    
    END_FUNCTION("CreateFunctionKineticLaw", SUCCESS );        
    return law;
}

KINETIC_LAW *CreatePWKineticLaw( BYTE opType, LINKED_LIST *children ) {
    KINETIC_LAW *law = NULL;
    
    START_FUNCTION("CreatePWKineticLaw");

    if( ( children == NULL ) ) {
        TRACE_0("the input children kinetic laws are NULL" );
        return NULL;
    }
    
    if( ( law = (KINETIC_LAW*)CALLOC( 1, sizeof( KINETIC_LAW ) ) ) == NULL ) {
        END_FUNCTION("CreatPWKineticLaw", FAILING );        
        return NULL;
    }
    law->valueType = KINETIC_LAW_VALUE_TYPE_PW;
    law->value.pw.opType = opType;
    law->value.pw.children = children;
    law->AcceptPostOrder = _AcceptPostOrderForPWKineticLaw;
    law->AcceptPreOrder = _AcceptPreOrderForPWKineticLaw;
    law->AcceptInOrder = _AcceptInOrderForPWKineticLaw;
    law->Accept = _AcceptForPWKineticLaw;
    
    END_FUNCTION("CreatePWKineticLaw", SUCCESS );        
    return law;
}

KINETIC_LAW *CreateDelayKineticLaw( BYTE opType, KINETIC_LAW *left, KINETIC_LAW *right, REB2SAC_SYMBOL *time ) {
    KINETIC_LAW *law = NULL;
    
    START_FUNCTION("CreateOpKineticLaw");

    if( ( left == NULL ) || ( right == NULL ) ) {
        TRACE_0("the input children kinetic laws are NULL" );
        return NULL;
    }
    
    if( ( law = (KINETIC_LAW*)CALLOC( 1, sizeof( KINETIC_LAW ) ) ) == NULL ) {
        END_FUNCTION("CreatOpKineticLaw", FAILING );        
        return NULL;
    }
    
    law->valueType = KINETIC_LAW_VALUE_TYPE_OP;
    law->value.op.opType = opType;
    law->value.op.left = left;
    law->value.op.right = right;
    law->value.op.time = time;
    law->value.op.values = CreateLinkedList();
    law->AcceptPostOrder = _AcceptPostOrderForOpKineticLaw;
    law->AcceptPreOrder = _AcceptPreOrderForOpKineticLaw;
    law->AcceptInOrder = _AcceptInOrderForOpKineticLaw;
    law->Accept = _AcceptForOpKineticLaw;
    
    END_FUNCTION("CreateOpKineticLaw", SUCCESS );        
    return law;
}

KINETIC_LAW *CreateOpKineticLaw( BYTE opType, KINETIC_LAW *left, KINETIC_LAW *right ) {
    KINETIC_LAW *law = NULL;
    
    START_FUNCTION("CreateOpKineticLaw");

    if( ( left == NULL ) || ( right == NULL ) ) {
        TRACE_0("the input children kinetic laws are NULL" );
        return NULL;
    }
    
    if( ( law = (KINETIC_LAW*)CALLOC( 1, sizeof( KINETIC_LAW ) ) ) == NULL ) {
        END_FUNCTION("CreatOpKineticLaw", FAILING );        
        return NULL;
    }
    
    law->valueType = KINETIC_LAW_VALUE_TYPE_OP;
    law->value.op.opType = opType;
    law->value.op.left = left;
    law->value.op.right = right;
    law->value.op.time = NULL;
    law->value.op.values = NULL;
    law->AcceptPostOrder = _AcceptPostOrderForOpKineticLaw;
    law->AcceptPreOrder = _AcceptPreOrderForOpKineticLaw;
    law->AcceptInOrder = _AcceptInOrderForOpKineticLaw;
    law->Accept = _AcceptForOpKineticLaw;
    
    END_FUNCTION("CreateOpKineticLaw", SUCCESS );        
    return law;
}

KINETIC_LAW *CreateUnaryOpKineticLaw( BYTE opType, KINETIC_LAW *child ) {
    KINETIC_LAW *law = NULL;
    
    START_FUNCTION("CreateUnaryOpKineticLaw");

    if( child == NULL ) {
        TRACE_0("the input children kinetic laws are NULL" );
        return NULL;
    }
    
    if( ( law = (KINETIC_LAW*)CALLOC( 1, sizeof( KINETIC_LAW ) ) ) == NULL ) {
        END_FUNCTION("CreatOpKineticLaw", FAILING );        
        return NULL;
    }
    
    law->valueType = KINETIC_LAW_VALUE_TYPE_UNARY_OP;
    law->value.unaryOp.opType = opType;
    law->value.unaryOp.child = child;
    law->AcceptPostOrder = _AcceptPostOrderForUnaryOpKineticLaw;
    law->AcceptPreOrder = _AcceptPreOrderForUnaryOpKineticLaw;
    law->AcceptInOrder = _AcceptInOrderForUnaryOpKineticLaw;
    law->Accept = _AcceptForUnaryOpKineticLaw;
    
    END_FUNCTION("CreateUnaryOpKineticLaw", SUCCESS );        
    return law;
}


LINKED_LIST *CloneChildren( LINKED_LIST *children ) {
    LINKED_LIST *clones = NULL;
    KINETIC_LAW *child = NULL;
    KINETIC_LAW *clone = NULL;

    START_FUNCTION("CloneChildren");

    if( ( clones = CreateLinkedList() ) == NULL ) {
      END_FUNCTION("CloneChildren", FAILING );
      return NULL;        
    }    
    ResetCurrentElement( children );
    while ( child = (KINETIC_LAW*)GetNextFromLinkedList( children )) {
      if( ( clone = CloneKineticLaw( child ) )  == NULL ) {
	END_FUNCTION("CloneChildren", FAILING );        
	return NULL;
      }
      if( IS_FAILED( ( AddElementInLinkedList( (CADDR_T)clone, clones ) ) ) ) {
	END_FUNCTION("CloneChildren", FAILING );        
	return NULL;
      }
    }  
    return clones;
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
	clone->value.op.time = law->value.op.time;
	clone->value.op.values = law->value.op.values;
        clone->AcceptPostOrder = _AcceptPostOrderForOpKineticLaw;
        clone->AcceptPreOrder = _AcceptPreOrderForOpKineticLaw;
        clone->AcceptInOrder = _AcceptInOrderForOpKineticLaw;
        clone->Accept = _AcceptForOpKineticLaw;
        END_FUNCTION("CloneKineticLaw", SUCCESS );        
        return clone;
    }
    else if( law->valueType == KINETIC_LAW_VALUE_TYPE_UNARY_OP ) {
      clone->valueType = KINETIC_LAW_VALUE_TYPE_UNARY_OP;
      if( ( clone->value.unaryOp.child = CloneKineticLaw( law->value.unaryOp.child ) )  == NULL ) {
#ifdef DEBUG
	string = ToStringKineticLaw( law->value.unaryOp.child );
	printf( "could not create a clone of %s", GetCharArrayOfString( string ) );
	FreeString( &string );
#endif        
	END_FUNCTION("CloneKineticLaw", FAILING );        
	return NULL;
      }
      clone->value.unaryOp.opType = law->value.unaryOp.opType;
      clone->AcceptPostOrder = _AcceptPostOrderForUnaryOpKineticLaw;
      clone->AcceptPreOrder = _AcceptPreOrderForUnaryOpKineticLaw;
      clone->AcceptInOrder = _AcceptInOrderForUnaryOpKineticLaw;
      clone->Accept = _AcceptForUnaryOpKineticLaw;
      END_FUNCTION("CloneKineticLaw", SUCCESS );        
      return clone;
    }
    else if( law->valueType == KINETIC_LAW_VALUE_TYPE_PW ) {
      clone->valueType = KINETIC_LAW_VALUE_TYPE_PW;
      if( ( clone->value.pw.children = CloneChildren( law->value.pw.children ) )  == NULL ) {
#ifdef DEBUG
	string = ToStringKineticLaw( law->value.pw.children );
	printf( "could not create a clone of %s", GetCharArrayOfString( string ) );
	FreeString( &string );
#endif        
	END_FUNCTION("CloneKineticLaw", FAILING );        
	return NULL;
      }
      clone->value.pw.opType = law->value.pw.opType;
      clone->AcceptPostOrder = _AcceptPostOrderForPWKineticLaw;
      clone->AcceptPreOrder = _AcceptPreOrderForPWKineticLaw;
      clone->AcceptInOrder = _AcceptInOrderForPWKineticLaw;
      clone->Accept = _AcceptForPWKineticLaw;
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
    } else if( law->valueType == KINETIC_LAW_VALUE_TYPE_UNARY_OP ) {
        FreeKineticLaw( &(law->value.unaryOp.child) );
    } else if( law->valueType == KINETIC_LAW_VALUE_TYPE_PW ) {
        DeleteLinkedList( &(law->value.pw.children) );        
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
    } else if( law->valueType == KINETIC_LAW_VALUE_TYPE_UNARY_OP ) {
        FreeKineticLaw( &(law->value.unaryOp.child) );
    } else if( law->valueType == KINETIC_LAW_VALUE_TYPE_PW ) {
        DeleteLinkedList( &(law->value.pw.children) );        
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
    } else if( law->valueType == KINETIC_LAW_VALUE_TYPE_UNARY_OP ) {
        FreeKineticLaw( &(law->value.unaryOp.child) );
    } else if( law->valueType == KINETIC_LAW_VALUE_TYPE_PW ) {
        DeleteLinkedList( &(law->value.pw.children) );        
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

RET_VAL SetCompartmentKineticLaw( KINETIC_LAW *law, COMPARTMENT *compartment ) {
    START_FUNCTION("SetCompartmentKineticLaw");
    
    if( law == NULL ) {
        END_FUNCTION("SetCompartmentKineticLaw", FAILING );        
        return FAILING;
    }
    
    if( law->valueType == KINETIC_LAW_VALUE_TYPE_OP ) {
        FreeKineticLaw( &(law->value.op.left) );
        FreeKineticLaw( &(law->value.op.right) );        
    } else if( law->valueType == KINETIC_LAW_VALUE_TYPE_UNARY_OP ) {
        FreeKineticLaw( &(law->value.unaryOp.child) );
    } else if( law->valueType == KINETIC_LAW_VALUE_TYPE_PW ) {
        DeleteLinkedList( &(law->value.pw.children) );        
    }
    
    law->valueType = KINETIC_LAW_VALUE_TYPE_COMPARTMENT;
    law->value.compartment = compartment;
    law->AcceptPostOrder = _AcceptForCompartmentKineticLaw;
    law->AcceptPreOrder = _AcceptForCompartmentKineticLaw;
    law->AcceptInOrder = _AcceptForCompartmentKineticLaw;
    law->Accept = _AcceptForCompartmentKineticLaw;
    
    END_FUNCTION("SetCompartmentKineticLaw", SUCCESS );        
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
    } else if( law->valueType == KINETIC_LAW_VALUE_TYPE_UNARY_OP ) {
        FreeKineticLaw( &(law->value.unaryOp.child) );
    } else if( law->valueType == KINETIC_LAW_VALUE_TYPE_PW ) {
        DeleteLinkedList( &(law->value.pw.children) );        
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

RET_VAL SetFunctionSymbolKineticLaw( KINETIC_LAW *law, char *funcSymbol ) {
    START_FUNCTION("SetFunctionSymbolKineticLaw");
    
    if( law == NULL ) {
        END_FUNCTION("SetFunctionSymbolKineticLaw", FAILING );        
        return FAILING;
    }
    
    if( law->valueType == KINETIC_LAW_VALUE_TYPE_OP ) {
        FreeKineticLaw( &(law->value.op.left) );
        FreeKineticLaw( &(law->value.op.right) );        
    } else if( law->valueType == KINETIC_LAW_VALUE_TYPE_UNARY_OP ) {
        FreeKineticLaw( &(law->value.unaryOp.child) );
    } else if( law->valueType == KINETIC_LAW_VALUE_TYPE_PW ) {
        DeleteLinkedList( &(law->value.pw.children) );        
    }
    
    law->valueType = KINETIC_LAW_VALUE_TYPE_FUNCTION_SYMBOL;
    law->value.funcSymbol = funcSymbol;
    law->AcceptPostOrder = _AcceptForFunctionSymbolKineticLaw;
    law->AcceptPreOrder = _AcceptForFunctionSymbolKineticLaw;
    law->AcceptInOrder = _AcceptForFunctionSymbolKineticLaw;
    law->Accept = _AcceptForFunctionSymbolKineticLaw;
    
    END_FUNCTION("SetFunctionSymbolKineticLaw", SUCCESS );        
    return SUCCESS;
}


RET_VAL SetPWKineticLaw( KINETIC_LAW *law, BYTE opType, LINKED_LIST *children ) {
    START_FUNCTION("SetPWKineticLaw");
    
    if( law == NULL ) {
        END_FUNCTION("SetPWKineticLaw", FAILING );        
        return FAILING;
    }
    
    if( ( children == NULL ) ) {
        return ErrorReport( FAILING, "SetPWKineticLaw", "Cannot set NULL children in PW kinetic law");
    }
    
    if( law->valueType == KINETIC_LAW_VALUE_TYPE_PW ) {
        if( children != law->value.pw.children ) {
            DeleteLinkedList( &(law->value.pw.children) );
        }
    }
    
    law->valueType = KINETIC_LAW_VALUE_TYPE_PW;
    law->value.pw.opType = opType;
    law->value.pw.children = children;
    law->AcceptPostOrder = _AcceptPostOrderForPWKineticLaw;
    law->AcceptPreOrder = _AcceptPreOrderForPWKineticLaw;
    law->AcceptInOrder = _AcceptInOrderForPWKineticLaw;
    law->Accept = _AcceptForPWKineticLaw;
    
    END_FUNCTION("SetPWKineticLaw", SUCCESS );        
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

RET_VAL SetUnaryOpKineticLaw( KINETIC_LAW *law, BYTE opType, KINETIC_LAW *child ) {
    START_FUNCTION("SetUnaryOpKineticLaw");
    
    if( law == NULL ) {
        END_FUNCTION("SetUnaryOpKineticLaw", FAILING );        
        return FAILING;
    }
    
    if( child == NULL ) {
        return ErrorReport( FAILING, "SetUnaryOpKineticLaw", "Cannot set NULL children in Unary Op kinetic law");
    }
    
    if( law->valueType == KINETIC_LAW_VALUE_TYPE_UNARY_OP ) {
        if( child != law->value.unaryOp.child ) {
            FreeKineticLaw( &(law->value.unaryOp.child) );
        }
    }
    
    law->valueType = KINETIC_LAW_VALUE_TYPE_UNARY_OP;
    law->value.unaryOp.opType = opType;
    law->value.unaryOp.child = child;
    law->AcceptPostOrder = _AcceptPostOrderForUnaryOpKineticLaw;
    law->AcceptPreOrder = _AcceptPreOrderForUnaryOpKineticLaw;
    law->AcceptInOrder = _AcceptInOrderForUnaryOpKineticLaw;
    law->Accept = _AcceptForUnaryOpKineticLaw;
    
    END_FUNCTION("SetUnaryOpKineticLaw", SUCCESS );        
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
        if( IsSymbolConstant( sym ) && GetInitialAssignmentInSymbol( sym ) == NULL) {
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

BOOL IsCompartmentKineticLaw(KINETIC_LAW *law) {
    START_FUNCTION("IsCompartmentKineticLaw");
    
    if( law == NULL ) {
        END_FUNCTION("IsCompartmentKineticLaw", SUCCESS );        
        return FALSE;
    }
        
    END_FUNCTION("IsCompartmentKineticLaw", SUCCESS );        
    return ( law->valueType == KINETIC_LAW_VALUE_TYPE_COMPARTMENT ? TRUE : FALSE );
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


BOOL IsFunctionSymbolKineticLaw(KINETIC_LAW *law) {
    START_FUNCTION("IsFunctionSymbolKineticLaw");
    
    if( law == NULL ) {
        END_FUNCTION("IsFunctionSymbolKineticLaw", SUCCESS );        
        return FALSE;
    }
        
    END_FUNCTION("IsFunctionSymbolKineticLaw", SUCCESS );        
    return ( law->valueType == KINETIC_LAW_VALUE_TYPE_FUNCTION_SYMBOL ? TRUE : FALSE );
}


BOOL IsPWKineticLaw(KINETIC_LAW *law) {
    START_FUNCTION("IsPWKineticLaw");
    
    if( law == NULL ) {
        END_FUNCTION("IsPWKineticLaw", SUCCESS );        
        return FALSE;
    }
        
    END_FUNCTION("IsPWKineticLaw", SUCCESS );        
    return ( law->valueType == KINETIC_LAW_VALUE_TYPE_PW ? TRUE : FALSE );
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

BOOL IsUnaryOpKineticLaw(KINETIC_LAW *law) {
    START_FUNCTION("IsUnaryOpKineticLaw");
    
    if( law == NULL ) {
        END_FUNCTION("IsUnaryOpKineticLaw", SUCCESS );        
        return FALSE;
    }
        
    END_FUNCTION("IsUnaryOpKineticLaw", SUCCESS );        
    return ( law->valueType == KINETIC_LAW_VALUE_TYPE_UNARY_OP ? TRUE : FALSE );
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

COMPARTMENT *GetCompartmentFromKineticLaw(KINETIC_LAW *law) {
    START_FUNCTION("GetCompartmentFromKineticLaw");
    
    if( law == NULL ) {
        END_FUNCTION("GetCompartmentFromKineticLaw", SUCCESS );        
        return NULL;
    }
        
    END_FUNCTION("GetCompartmentFromKineticLaw", SUCCESS );        
    return law->value.compartment;
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


char *GetFunctionSymbolFromKineticLaw(KINETIC_LAW *law) {
    START_FUNCTION("GetFunctionSymbolFromKineticLaw");
    
    if( law == NULL ) {
        END_FUNCTION("GetFunctionSymbolFromKineticLaw", SUCCESS );        
        return NULL;
    }
        
    END_FUNCTION("GetFunctionSymbolFromKineticLaw", SUCCESS );        
    return law->value.funcSymbol;
}


BYTE GetPWTypeFromKineticLaw(KINETIC_LAW *law) {
    START_FUNCTION("GetPWTypeFromKineticLaw");
    
    if( law == NULL ) {
        END_FUNCTION("GetPWTypeFromKineticLaw", SUCCESS );        
        return 0;
    }
        
    if( law->valueType != KINETIC_LAW_VALUE_TYPE_PW ) {
        END_FUNCTION("GetPWTypeFromKineticLaw", SUCCESS );        
        return 0;
    }
    
    END_FUNCTION("GetPWTypeFromKineticLaw", SUCCESS );        
    return law->value.pw.opType;
}

REB2SAC_SYMBOL *GetTimeFromKineticLaw(KINETIC_LAW *law) {
    START_FUNCTION("GetOpTypeFromKineticLaw");
    
    if( law == NULL ) {
        END_FUNCTION("GetOpTypeFromKineticLaw", SUCCESS );        
        return 0;
    }
        
    if( law->valueType != KINETIC_LAW_VALUE_TYPE_OP ) {
        END_FUNCTION("GetOpTypeFromKineticLaw", SUCCESS );        
        return 0;
    }
    
    END_FUNCTION("GetOpTypeFromKineticLaw", SUCCESS );        
    return law->value.op.time;
}

LINKED_LIST *GetValuesFromKineticLaw(KINETIC_LAW *law) {
    START_FUNCTION("GetOpTypeFromKineticLaw");
    
    if( law == NULL ) {
        END_FUNCTION("GetOpTypeFromKineticLaw", SUCCESS );        
        return 0;
    }
        
    if( law->valueType != KINETIC_LAW_VALUE_TYPE_OP ) {
        END_FUNCTION("GetOpTypeFromKineticLaw", SUCCESS );        
        return 0;
    }
    
    END_FUNCTION("GetOpTypeFromKineticLaw", SUCCESS );        
    return law->value.op.values;
}

RET_VAL SetValuesKineticLaw(KINETIC_LAW *law, LINKED_LIST *list) {
    START_FUNCTION("SetValuesKineticLaw");

    law->value.op.values = list;
    
    END_FUNCTION("SetValuesKineticLaw", SUCCESS );        
    return SUCCESS;
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
    
    END_FUNCTION("GetOpTypeFromKineticLaw", SUCCESS );        
    return law->value.op.opType;
}

BYTE GetUnaryOpTypeFromKineticLaw(KINETIC_LAW *law) {
    START_FUNCTION("GetUnaryOpTypeFromKineticLaw");
    
    if( law == NULL ) {
        END_FUNCTION("GetUnaryOpTypeFromKineticLaw", SUCCESS );        
        return 0;
    }
        
    if( law->valueType != KINETIC_LAW_VALUE_TYPE_UNARY_OP ) {
        END_FUNCTION("GetUnaryOpTypeFromKineticLaw", SUCCESS );        
        return 0;
    }
    
    END_FUNCTION("GetUnaryOpTypeFromKineticLaw", SUCCESS );        
    return law->value.unaryOp.opType;
}

LINKED_LIST *GetPWChildrenFromKineticLaw(KINETIC_LAW *law) {
    START_FUNCTION("GetPWChildrenFromKineticLaw");
    
    if( law == NULL ) {
        END_FUNCTION("GetPWChildrenFromKineticLaw", SUCCESS );        
        return NULL;
    }
        
    if( law->valueType != KINETIC_LAW_VALUE_TYPE_PW ) {
        END_FUNCTION("GetPWChildrenFromKineticLaw", SUCCESS );        
        return NULL;
    }
    
    END_FUNCTION("GetPWChildrenFromKineticLaw", SUCCESS );        
    return law->value.pw.children;
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

KINETIC_LAW *GetUnaryOpChildFromKineticLaw(KINETIC_LAW *law) {
    START_FUNCTION("GetUnaryOpChildFromKineticLaw");
    
    if( law == NULL ) {
        END_FUNCTION("GetUnaryOpChildFromKineticLaw", SUCCESS );        
        return NULL;
    }
        
    if( law->valueType != KINETIC_LAW_VALUE_TYPE_UNARY_OP ) {
        END_FUNCTION("GetUnaryOpChildFromKineticLaw", SUCCESS );        
        return NULL;
    }
    
    END_FUNCTION("GetUnaryOpChildFromKineticLaw", SUCCESS );        
    return law->value.unaryOp.child;
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
    } else if( (*law)->valueType == KINETIC_LAW_VALUE_TYPE_UNARY_OP ) {
        FreeKineticLaw( &((*law)->value.unaryOp.child) );
    } else if( (*law)->valueType == KINETIC_LAW_VALUE_TYPE_PW ) {
        DeleteLinkedList( &((*law)->value.pw.children) );
    }
    FREE( *law );
        
    END_FUNCTION("FreeKineticLaw", SUCCESS );        
}


RET_VAL ReplaceSpeciesWithIntInKineticLaw( KINETIC_LAW *law, SPECIES *from, long to ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("ReplaceSpeciesWithIntInKineticLaw");
    
    if( speciesReplacementVisitor.VisitOp == NULL ) {
        speciesReplacementVisitor.VisitPW = _VisitPWToReplaceSpecies;
        speciesReplacementVisitor.VisitOp = _VisitOpToReplaceSpecies;
        speciesReplacementVisitor.VisitUnaryOp = _VisitUnaryOpToReplaceSpecies;
        speciesReplacementVisitor.VisitInt = _VisitIntToReplaceSpecies;
        speciesReplacementVisitor.VisitReal = _VisitRealToReplaceSpecies;
        speciesReplacementVisitor.VisitSymbol = _VisitSymbolToReplaceSpecies;
        speciesReplacementVisitor.VisitCompartment = _VisitCompartmentToReplaceSpecies;
        speciesReplacementVisitor.VisitFunctionSymbol = _VisitFunctionSymbolToReplaceSpecies;
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
        speciesReplacementVisitor.VisitPW = _VisitPWToReplaceSpecies;
        speciesReplacementVisitor.VisitOp = _VisitOpToReplaceSpecies;
        speciesReplacementVisitor.VisitUnaryOp = _VisitUnaryOpToReplaceSpecies;
        speciesReplacementVisitor.VisitInt = _VisitIntToReplaceSpecies;
        speciesReplacementVisitor.VisitReal = _VisitRealToReplaceSpecies;
        speciesReplacementVisitor.VisitSymbol = _VisitSymbolToReplaceSpecies;
        speciesReplacementVisitor.VisitCompartment = _VisitCompartmentToReplaceSpecies;
        speciesReplacementVisitor.VisitFunctionSymbol = _VisitFunctionSymbolToReplaceSpecies;
    }
    speciesReplacementVisitor.VisitSpecies = _VisitSpeciesToReplaceSpeciesWithReal;
    
    TRACE_2( "replacing %s with %f", GetCharArrayOfString( GetSpeciesNodeID( from ) ), to );
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
        speciesReplacementVisitor.VisitPW = _VisitPWToReplaceSpecies;
        speciesReplacementVisitor.VisitOp = _VisitOpToReplaceSpecies;
        speciesReplacementVisitor.VisitUnaryOp = _VisitUnaryOpToReplaceSpecies;
        speciesReplacementVisitor.VisitInt = _VisitIntToReplaceSpecies;
        speciesReplacementVisitor.VisitReal = _VisitRealToReplaceSpecies;
        speciesReplacementVisitor.VisitSymbol = _VisitSymbolToReplaceSpecies;
        speciesReplacementVisitor.VisitCompartment = _VisitCompartmentToReplaceSpecies;
        speciesReplacementVisitor.VisitFunctionSymbol = _VisitFunctionSymbolToReplaceSpecies;
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

RET_VAL ReplaceFunctionSymbolWithKineticLawInKineticLaw( KINETIC_LAW *law, char *from, KINETIC_LAW * to ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("ReplaceFunctionSymbolWithKineticLawInKineticLaw");
    
    if( speciesReplacementVisitor.VisitOp == NULL ) {
        speciesReplacementVisitor.VisitPW = _VisitPWToReplaceSpecies;
        speciesReplacementVisitor.VisitOp = _VisitOpToReplaceSpecies;
        speciesReplacementVisitor.VisitUnaryOp = _VisitUnaryOpToReplaceSpecies;
        speciesReplacementVisitor.VisitInt = _VisitIntToReplaceSpecies;
        speciesReplacementVisitor.VisitReal = _VisitRealToReplaceSpecies;
        speciesReplacementVisitor.VisitSymbol = _VisitSymbolToReplaceSpecies;
        speciesReplacementVisitor.VisitCompartment = _VisitCompartmentToReplaceSpecies;
        speciesReplacementVisitor.VisitSpecies = _VisitSpeciesToReplaceConstant;
    }
    speciesReplacementVisitor.VisitFunctionSymbol = _VisitFunctionSymbolToReplaceWithKineticLaw;
    
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
        visitor.VisitPW = _VisitPWToReplaceConstant;
        visitor.VisitOp = _VisitOpToReplaceConstant;
        visitor.VisitUnaryOp = _VisitUnaryOpToReplaceConstant;
        visitor.VisitInt = _VisitIntToReplaceConstant;
        visitor.VisitReal = _VisitRealToReplaceConstant;
        visitor.VisitSpecies = _VisitSpeciesToReplaceConstant;
        visitor.VisitCompartment = _VisitCompartmentToReplaceConstant;
        visitor.VisitSymbol = _VisitSymbolToReplaceConstant;
        speciesReplacementVisitor.VisitCompartment = _VisitCompartmentToReplaceSpecies;
        visitor.VisitFunctionSymbol = _VisitFunctionSymbolToReplaceConstant;
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

static RET_VAL _VisitPWToReplaceConstant( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    START_FUNCTION("_VisitPWToReplaceConstant");
    END_FUNCTION("_VisitPWToReplaceConstant", SUCCESS );        
    return SUCCESS;                        
}

static RET_VAL _VisitOpToReplaceConstant( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    START_FUNCTION("_VisitOpToReplaceConstant");
    END_FUNCTION("_VisitOpToReplaceConstant", SUCCESS );        
    return SUCCESS;                        
}

static RET_VAL _VisitUnaryOpToReplaceConstant( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
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

static RET_VAL _VisitCompartmentToReplaceConstant( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    double value = 0.0;
    double from = 0.0;
    double to = 0.0;
    COMPARTMENT *compartment = NULL;
    
    START_FUNCTION("_VisitCompartmentToReplaceConstant");
    
    from = *((double*)(visitor->_internal1)); 
    to = *((double*)(visitor->_internal2)); 
       
    compartment = GetCompartmentFromKineticLaw( kineticLaw );
    if( !IsCompartmentConstant( compartment ) ) {
        END_FUNCTION("_VisitSymbolToReplaceConstant", SUCCESS );        
        return SUCCESS;                        
    }
    
    value = GetSizeInCompartment( compartment );        
    if( IS_REAL_EQUAL( from, value ) ) {
        if( IS_FAILED( ( ret = SetCurrentSizeInCompartment( compartment, to ) ) ) ) {
            END_FUNCTION("_VisitCompartmentToReplaceConstant", ret );        
            return ret;                        
        }
    }
    
    END_FUNCTION("_VisitCompartmentToReplaceConstant", SUCCESS );        
    return SUCCESS;                        
}

static RET_VAL _VisitFunctionSymbolToReplaceConstant( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    double value = 0.0;
    double from = 0.0;
    double to = 0.0;
    char *funcSymbol = NULL;
    
    START_FUNCTION("_VisitFunctionSymbolToReplaceConstant");
    
    from = *((double*)(visitor->_internal1)); 
    to = *((double*)(visitor->_internal2)); 

    /* TODO: What do I do here? */       
//     funcSymbol = GetFunctionSymbolFromKineticLaw( kineticLaw );
//     if( !IsSymbolConstant( symbol ) ) {
//         END_FUNCTION("_VisitSymbolToReplaceConstant", SUCCESS );        
//         return SUCCESS;                        
//     }
//     if( !IsRealValueSymbol( symbol ) ) {
//         END_FUNCTION("_VisitSymbolToReplaceConstant", SUCCESS );        
//         return SUCCESS;                        
//     }
    
//     value = GetRealValueInSymbol( symbol );        
//     if( IS_REAL_EQUAL( from, value ) ) {
//         if( IS_FAILED( ( ret = SetRealValueInSymbol( symbol, to ) ) ) ) {
//             END_FUNCTION("_VisitSymbolToReplaceConstant", ret );        
//             return ret;                        
//         }
//     }
    
    END_FUNCTION("_VisitFunctionSymbolToReplaceConstant", SUCCESS );        
    return SUCCESS;                        
}


static RET_VAL _VisitSpeciesToReplaceConstant( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    START_FUNCTION("_VisitSpeciesToReplaceConstant");
    END_FUNCTION("_VisitSpeciesToReplaceConstant", SUCCESS );        
    return SUCCESS;                        
}



static RET_VAL _VisitPWToReplaceSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    START_FUNCTION("_VisitPWToReplaceSpecies");
    END_FUNCTION("_VisitPWToReplaceSpecies", SUCCESS );        
    return SUCCESS;                        
}

static RET_VAL _VisitOpToReplaceSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    START_FUNCTION("_VisitOpToReplaceSpecies");
    END_FUNCTION("_VisitOpToReplaceSpecies", SUCCESS );        
    return SUCCESS;                        
}

static RET_VAL _VisitUnaryOpToReplaceSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    START_FUNCTION("_VisitUnaryOpToReplaceSpecies");
    END_FUNCTION("_VisitUnaryOpToReplaceSpecies", SUCCESS );        
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

static RET_VAL _VisitCompartmentToReplaceSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    START_FUNCTION("_VisitCompartmentToReplaceSpecies");
    END_FUNCTION("_VisitCompartmentToReplaceSpecies", SUCCESS );        
    return SUCCESS;                        
}

static RET_VAL _VisitFunctionSymbolToReplaceSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    START_FUNCTION("_VisitFunctionSymbolToReplaceSpecies");
    END_FUNCTION("_VisitFunctionSymbolToReplaceSpecies", SUCCESS );        
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
        TRACE_2( "replacing %s with %li", GetCharArrayOfString( GetSpeciesNodeID( species ) ), value );
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
        TRACE_2( "replacing %s with %f", GetCharArrayOfString( GetSpeciesNodeID( species ) ), value );
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
        printf( "replacing %s with %s" NEW_LINE, GetCharArrayOfString( GetSpeciesNodeID( species ) ), GetCharArrayOfString( string ) );
        FreeString( &string );
#endif
        if( replacement->valueType == KINETIC_LAW_VALUE_TYPE_OP ) {
            if( IS_FAILED( ( ret = SetOpKineticLaw( kineticLaw, replacement->value.op.opType, 
                CloneKineticLaw( replacement->value.op.left ), CloneKineticLaw( replacement->value.op.right ) ) ) ) ) {
                string = ToStringKineticLaw( replacement );
                return ErrorReport( FAILING, "_VisitSpeciesToReplaceSpeciesWithKineticLaw", "failed to create clone for %s", GetCharArrayOfString( string ) );
            } 
        } else if( replacement->valueType == KINETIC_LAW_VALUE_TYPE_UNARY_OP ) {
            if( IS_FAILED( ( ret = SetUnaryOpKineticLaw( kineticLaw, replacement->value.unaryOp.opType, 
                CloneKineticLaw( replacement->value.unaryOp.child ) ) ) ) ) {
                string = ToStringKineticLaw( replacement );
                return ErrorReport( FAILING, "_VisitSpeciesToReplaceSpeciesWithKineticLaw", "failed to create clone for %s", GetCharArrayOfString( string ) );
            } 
        } else if( replacement->valueType == KINETIC_LAW_VALUE_TYPE_PW ) {
            if( IS_FAILED( ( ret = SetPWKineticLaw( kineticLaw, replacement->value.pw.opType, 
                CloneChildren( replacement->value.pw.children ) ) ) ) ) {
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

static RET_VAL _VisitFunctionSymbolToReplaceWithKineticLaw( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    char *funcSymbol = NULL;
    KINETIC_LAW *replacement = NULL;
    STRING *string = NULL;
    
    START_FUNCTION("_VisitFunctionSymbolToReplaceWithKineticLaw");
    
    funcSymbol = (char*)(visitor->_internal2);

    if( strcmp(kineticLaw->value.funcSymbol,funcSymbol)==0 ) {
        replacement = (KINETIC_LAW*)(visitor->_internal1);
#if DEBUG
        string = ToStringKineticLaw( replacement );
        printf( "replacing %s with %s" NEW_LINE, funcSymbol, GetCharArrayOfString( string ) );
        FreeString( &string );
#endif
        if( replacement->valueType == KINETIC_LAW_VALUE_TYPE_OP ) {
            if( IS_FAILED( ( ret = SetOpKineticLaw( kineticLaw, replacement->value.op.opType, 
                CloneKineticLaw( replacement->value.op.left ), CloneKineticLaw( replacement->value.op.right ) ) ) ) ) {
                string = ToStringKineticLaw( replacement );
                return ErrorReport( FAILING, "_VisitFunctionSymbolToReplaceWithKineticLaw", "failed to create clone for %s", GetCharArrayOfString( string ) );
            } 
        } else if( replacement->valueType == KINETIC_LAW_VALUE_TYPE_UNARY_OP ) {
            if( IS_FAILED( ( ret = SetUnaryOpKineticLaw( kineticLaw, replacement->value.unaryOp.opType, 
                CloneKineticLaw( replacement->value.unaryOp.child ) ) ) ) ) {
                string = ToStringKineticLaw( replacement );
                return ErrorReport( FAILING, "_VisitFunctionSymbolToReplaceWithKineticLaw", "failed to create clone for %s", GetCharArrayOfString( string ) );
            } 
        } else if( replacement->valueType == KINETIC_LAW_VALUE_TYPE_PW ) {
            if( IS_FAILED( ( ret = SetPWKineticLaw( kineticLaw, replacement->value.pw.opType, 
                CloneChildren( replacement->value.pw.children ) ) ) ) ) {
                string = ToStringKineticLaw( replacement );
                return ErrorReport( FAILING, "_VisitSpeciesToReplaceSpeciesWithKineticLaw", "failed to create clone for %s", GetCharArrayOfString( string ) );
            } 
        }
        else {
            memcpy( kineticLaw, replacement, sizeof( KINETIC_LAW ) );
        }
    }
    END_FUNCTION("_VisitFunctionSymbolToReplaceWithKineticLaw", SUCCESS );        
    return ret;                        
}



static RET_VAL _AcceptPostOrderForPWKineticLaw( KINETIC_LAW *law, KINETIC_LAW_VISITOR *visitor ) {
    RET_VAL ret = SUCCESS;
    
    LINKED_LIST *children = NULL;
    KINETIC_LAW *child = NULL;

    START_FUNCTION("_AcceptPostOrderForPWKineticLaw");
    
    children = law->value.pw.children;
    
    ResetCurrentElement( children );
    while ( child = (KINETIC_LAW*)GetNextFromLinkedList( children )) {
      if( IS_FAILED( ( ret = child->AcceptPostOrder( child, visitor ) ) ) ) {
        END_FUNCTION("_AcceptPostOrderForPWKineticLaw", ret );        
        return ret;                        
      }
    }
    
    if( IS_FAILED( ( ret = visitor->VisitPW( visitor, law ) ) ) ) {
        END_FUNCTION("_AcceptPostOrderForPWKineticLaw", ret );        
        return ret;                        
    }
    
    END_FUNCTION("_AcceptPostOrderForPWKineticLaw", SUCCESS );        
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


static RET_VAL _AcceptPostOrderForUnaryOpKineticLaw( KINETIC_LAW *law, KINETIC_LAW_VISITOR *visitor ) {
    RET_VAL ret = SUCCESS;
    
    KINETIC_LAW *child = NULL;
    
    START_FUNCTION("_AcceptPostOrderForUnaryOpKineticLaw");
    
    child = law->value.unaryOp.child;
    if( IS_FAILED( ( ret = child->AcceptPostOrder( child, visitor ) ) ) ) {
        END_FUNCTION("_AcceptPostOrderForOpKineticLaw", ret );        
        return ret;                        
    }
    
    if( IS_FAILED( ( ret = visitor->VisitUnaryOp( visitor, law ) ) ) ) {
        END_FUNCTION("_AcceptPostOrderForUnaryOpKineticLaw", ret );        
        return ret;                        
    }
    
    END_FUNCTION("_AcceptPostOrderForUnaryOpKineticLaw", SUCCESS );        
    return ret;                        
}



static RET_VAL _AcceptPreOrderForPWKineticLaw( KINETIC_LAW *law, KINETIC_LAW_VISITOR *visitor ) {
    RET_VAL ret = SUCCESS;
    
    LINKED_LIST *children = NULL;
    KINETIC_LAW *child = NULL;

    START_FUNCTION("_AcceptPreOrderForPWKineticLaw");
    
    if( IS_FAILED( ( ret = visitor->VisitPW( visitor, law ) ) ) ) {
        END_FUNCTION("_AcceptPreOrderForPWKineticLaw", ret );        
        return ret;                        
    }
    
    children = law->value.pw.children;
    ResetCurrentElement( children );
    while ( child = (KINETIC_LAW*)GetNextFromLinkedList( children )) {
      if( IS_FAILED( ( ret = child->AcceptPostOrder( child, visitor ) ) ) ) {
        END_FUNCTION("_AcceptPostOrderForPWKineticLaw", ret );        
        return ret;                        
      }
    }
    
    END_FUNCTION("_AcceptPreOrderForPWKineticLaw", SUCCESS );        
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

static RET_VAL _AcceptPreOrderForUnaryOpKineticLaw( KINETIC_LAW *law, KINETIC_LAW_VISITOR *visitor ) {
    RET_VAL ret = SUCCESS;
    
    KINETIC_LAW *child = NULL;;
    
    START_FUNCTION("_AcceptPreOrderForUnaryOpKineticLaw");
    
    if( IS_FAILED( ( ret = visitor->VisitUnaryOp( visitor, law ) ) ) ) {
        END_FUNCTION("_AcceptPreOrderForUnaryOpKineticLaw", ret );        
        return ret;                        
    }
    
    child = law->value.unaryOp.child;
    if( IS_FAILED( ( ret = child->AcceptPreOrder( child, visitor ) ) ) ) {
        END_FUNCTION("_AcceptPreOrderForUnaryOpKineticLaw", ret );        
        return ret;                        
    }
    
    END_FUNCTION("_AcceptPreOrderForUnaryOpKineticLaw", SUCCESS );        
    return ret;                        
}

static RET_VAL _AcceptForPWKineticLaw( KINETIC_LAW *law, KINETIC_LAW_VISITOR *visitor ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("_AcceptForPWKineticLaw");
    
    if( IS_FAILED( ( ret = visitor->VisitPW( visitor, law ) ) ) ) {
        END_FUNCTION("_AcceptForPWKineticLaw", ret );        
        return ret;                        
    }
    
    END_FUNCTION("_AcceptForPWKineticLaw", SUCCESS );        
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

static RET_VAL _AcceptForUnaryOpKineticLaw( KINETIC_LAW *law, KINETIC_LAW_VISITOR *visitor ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("_AcceptForUnaryOpKineticLaw");
    
    if( IS_FAILED( ( ret = visitor->VisitUnaryOp( visitor, law ) ) ) ) {
        END_FUNCTION("_AcceptForUnaryOpKineticLaw", ret );        
        return ret;                        
    }
    
    END_FUNCTION("_AcceptForUnaryOpKineticLaw", SUCCESS );        
    return ret;                        
}



static RET_VAL _AcceptInOrderForPWKineticLaw( KINETIC_LAW *law, KINETIC_LAW_VISITOR *visitor ) {
    RET_VAL ret = SUCCESS;
    
    LINKED_LIST *children = NULL;
    KINETIC_LAW *child = NULL;;
    
    START_FUNCTION("_AcceptInOrderForPWKineticLaw");
    
    children = law->value.pw.children;
    ResetCurrentElement( children );
    while ( child = (KINETIC_LAW*)GetNextFromLinkedList( children )) {
      if( IS_FAILED( ( ret = child->AcceptPostOrder( child, visitor ) ) ) ) {
        END_FUNCTION("_AcceptPostOrderForPWKineticLaw", ret );        
        return ret;                        
      }
    }
    
    if( IS_FAILED( ( ret = visitor->VisitPW( visitor, law ) ) ) ) {
        END_FUNCTION("_AcceptInOrderForPWKineticLaw", ret );        
        return ret;                        
    }
        
    END_FUNCTION("_AcceptInOrderForPWKineticLaw", SUCCESS );        
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


static RET_VAL _AcceptInOrderForUnaryOpKineticLaw( KINETIC_LAW *law, KINETIC_LAW_VISITOR *visitor ) {
    RET_VAL ret = SUCCESS;
    
    KINETIC_LAW *child = NULL;;
    
    START_FUNCTION("_AcceptInOrderForUnaryOpKineticLaw");
    
    child = law->value.unaryOp.child;
    if( IS_FAILED( ( ret = child->AcceptInOrder( child, visitor ) ) ) ) {
        END_FUNCTION("_AcceptInOrderForUnaryOpKineticLaw", ret );        
        return ret;                        
    }
    
    if( IS_FAILED( ( ret = visitor->VisitUnaryOp( visitor, law ) ) ) ) {
        END_FUNCTION("_AcceptInOrderForUnaryOpKineticLaw", ret );        
        return ret;                        
    }
    
    END_FUNCTION("_AcceptInOrderForUnaryOpKineticLaw", SUCCESS );        
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

static RET_VAL _AcceptForCompartmentKineticLaw( KINETIC_LAW *law, KINETIC_LAW_VISITOR *visitor ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("_AcceptForCompartmentKineticLaw");
    
    if( IS_FAILED( ( ret = visitor->VisitCompartment( visitor, law ) ) ) ) {
        END_FUNCTION("_AcceptForCompartmentKineticLaw", ret );        
        return ret;                        
    }
    
    END_FUNCTION("_AcceptForCompartmentKineticLaw", SUCCESS );        
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

static RET_VAL _AcceptForFunctionSymbolKineticLaw( KINETIC_LAW *law, KINETIC_LAW_VISITOR *visitor ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("_AcceptForFunctionSymbolKineticLaw");
    
    if( IS_FAILED( ( ret = visitor->VisitFunctionSymbol( visitor, law ) ) ) ) {
        END_FUNCTION("_AcceptForFunctionSymbolKineticLaw", ret );        
        return ret;                        
    }
    
    END_FUNCTION("_AcceptForFunctionSymbolKineticLaw", SUCCESS );        
    return ret;                        
}


STRING *ToStringKineticLaw( KINETIC_LAW *law ) {
    STRING *string = NULL;
    TO_STRING_VISITOR_INTERNAL internal;
    
    START_FUNCTION("ToStringKineticLaw");
    
    if( toStringVisitor.VisitOp == NULL ) {
        toStringVisitor.VisitPW = _VisitPWToString;
        toStringVisitor.VisitOp = _VisitOpToString;
        toStringVisitor.VisitUnaryOp = _VisitUnaryOpToString;
        toStringVisitor.VisitInt = _VisitIntToString;
        toStringVisitor.VisitReal = _VisitRealToString;
        toStringVisitor.VisitCompartment = _VisitCompartmentToString;
        toStringVisitor.VisitSpecies = _VisitSpeciesToString;
        toStringVisitor.VisitSymbol = _VisitSymbolToString;
        toStringVisitor.VisitFunctionSymbol = _VisitFunctionSymbolToString;
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


static RET_VAL _VisitPWToString( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    int len = 0;
    char *buf = NULL;
    STRING *str = NULL;    
    STRING *childStr = NULL;
    LINKED_LIST *children = NULL;
    KINETIC_LAW *child = NULL; 
    LINKED_LIST *stack = NULL;
        
    TO_STRING_VISITOR_INTERNAL *internal = NULL;    
    
    START_FUNCTION("_VisitPWToString");

    internal = (TO_STRING_VISITOR_INTERNAL*)(visitor->_internal1);
    stack = internal->stack;
    
    len = 3;
    
    childStr = (STRING*)GetTailFromLinkedList( stack );
    RemoveTailFromLinkedList( stack );
    len += GetStringLength( childStr );
    
    if( ( buf = (char*)MALLOC( len + 1) ) == NULL ) {
        return ErrorReport( FAILING, "_VisitPWToString", "failed to create %s %c", 
            GetCharArrayOfString( childStr ),  kineticLaw->value.pw.opType );
    }
    else {
        sprintf( buf, "%s %c", 
            GetCharArrayOfString( childStr ),  kineticLaw->value.pw.opType );
    }
    
    FreeString( &childStr );
    
    if( ( str = CreateString( buf ) ) == NULL ) {
        return ErrorReport( FAILING, "_VisitPWToString", "could not create a string for %s", buf );
    }
    if( IS_FAILED( ( ret = AddElementInLinkedList( (CADDR_T)str, stack ) ) ) ) {
        return ErrorReport( FAILING, "_VisitPWToString", "could not add %s", buf );
    }
    FREE( buf );                           
    END_FUNCTION("_VisitPWToString", SUCCESS );        
    return ret;                        
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
    if( IS_FAILED( ( ret = AddElementInLinkedList( (CADDR_T)str, stack ) ) ) ) {
        return ErrorReport( FAILING, "_VisitIntToString", "could not add %s", buf );
    }
    FREE( buf );                           
    END_FUNCTION("_VisitOpToString", SUCCESS );        
    return ret;                        
}

static RET_VAL _VisitUnaryOpToString( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    int len = 0;
    char *buf = NULL;
    STRING *str = NULL;    
    STRING *childStr = NULL;
    KINETIC_LAW *child = NULL; 
    LINKED_LIST *stack = NULL;
        
    TO_STRING_VISITOR_INTERNAL *internal = NULL;    
    
    START_FUNCTION("_VisitUnaryOpToString");

    internal = (TO_STRING_VISITOR_INTERNAL*)(visitor->_internal1);
    stack = internal->stack;
    
    len = 3;
    
    childStr = (STRING*)GetTailFromLinkedList( stack );
    RemoveTailFromLinkedList( stack );
    len += GetStringLength( childStr );
        
    child = kineticLaw->value.unaryOp.child;
    
    if( ( buf = (char*)MALLOC( len + 1) ) == NULL ) {
        return ErrorReport( FAILING, "_VisitUnaryOpToString", "failed to create %c %s", 
            kineticLaw->value.unaryOp.opType, GetCharArrayOfString( childStr ) );
    }
    else {
        sprintf( buf, "%c %s", 
            kineticLaw->value.unaryOp.opType, GetCharArrayOfString( childStr ) );
    }
    
    FreeString( &childStr );
    
    if( ( str = CreateString( buf ) ) == NULL ) {
        return ErrorReport( FAILING, "_VisitIntToString", "could not create a string for %s", buf );
    }
    if( IS_FAILED( ( ret = AddElementInLinkedList( (CADDR_T)str, stack ) ) ) ) {
        return ErrorReport( FAILING, "_VisitIntToString", "could not add %s", buf );
    }
    FREE( buf );                           
    END_FUNCTION("_VisitUnaryOpToString", SUCCESS );        
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
    if( IS_FAILED( ( ret = AddElementInLinkedList( (CADDR_T)str, internal->stack ) ) ) ) {
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
    if( IS_FAILED( ( ret = AddElementInLinkedList( (CADDR_T)str, internal->stack ) ) ) ) {
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
    from = GetSpeciesNodeID( kineticLaw->value.species );
    if( ( str = CloneString( from ) ) == NULL ) {
        return ErrorReport( FAILING, "_VisitSpeciesToString", "could not create a string for %s", GetCharArrayOfString( from ) );
    }
    internal = (TO_STRING_VISITOR_INTERNAL*)(visitor->_internal1);
    if( IS_FAILED( ( ret = AddElementInLinkedList( (CADDR_T)str, internal->stack ) ) ) ) {
        return ErrorReport( FAILING, "_VisitSpeciesToString", "could not add %s", GetCharArrayOfString( from ) );
    }
    
    END_FUNCTION("_VisitSpeciesToString", SUCCESS );        
    return ret;                        
}

static RET_VAL _VisitCompartmentToString( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    STRING *from = NULL;
    STRING *str = NULL;    
    TO_STRING_VISITOR_INTERNAL *internal = NULL;    
    
    START_FUNCTION("_VisitCompartmentToString");
    from = GetCompartmentID( kineticLaw->value.compartment );
    if( ( str = CloneString( from ) ) == NULL ) {
        return ErrorReport( FAILING, "_VisitCompartmentToString", "could not create a string for %s", GetCharArrayOfString( from ) );
    }
    internal = (TO_STRING_VISITOR_INTERNAL*)(visitor->_internal1);
    if( IS_FAILED( ( ret = AddElementInLinkedList( (CADDR_T)str, internal->stack ) ) ) ) {
        return ErrorReport( FAILING, "_VisitCompartmentToString", "could not add %s", GetCharArrayOfString( from ) );
    }
    
    END_FUNCTION("_VisitCompartmentToString", SUCCESS );        
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
    if( IS_FAILED( ( ret = AddElementInLinkedList( (CADDR_T)str, internal->stack ) ) ) ) {
        return ErrorReport( FAILING, "_VisitSymbolToString", "could not add %s", GetCharArrayOfString( from ) );
    }
    
    END_FUNCTION("_VisitSymbolToString", SUCCESS );        
    return ret;                        
}

static RET_VAL _VisitFunctionSymbolToString( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    STRING *from = NULL;
    STRING *str = NULL;    
    TO_STRING_VISITOR_INTERNAL *internal = NULL;    
    
    START_FUNCTION("_VisitFunctionSymbolToString");
    from = CreateString(kineticLaw->value.funcSymbol);
    if( ( str = CloneString( from ) ) == NULL ) {
        return ErrorReport( FAILING, "_VisitFunctionSymbolToString", "could not create a string for %s", GetCharArrayOfString( from ) );
    }
    internal = (TO_STRING_VISITOR_INTERNAL*)(visitor->_internal1);
    if( IS_FAILED( ( ret = AddElementInLinkedList( (CADDR_T)str, internal->stack ) ) ) ) {
        return ErrorReport( FAILING, "_VisitFunctionSymbolToString", "could not add %s", GetCharArrayOfString( from ) );
    }
    
    END_FUNCTION("_VisitFunctionSymbolToString", SUCCESS );        
    return ret;                        
}


static BOOL _NeedParenForLeft( KINETIC_LAW *parent, KINETIC_LAW *child ) {
    BYTE parentOpType = 0;
    BYTE childOpType = 0;
    
    START_FUNCTION("_NeedParenForLeft");
    
    parentOpType = parent->value.op.opType;
    if( ( parentOpType == KINETIC_LAW_OP_POW ) ||
	( parentOpType == KINETIC_LAW_OP_LOG ) ||
	( parentOpType == KINETIC_LAW_OP_ROOT ) ||
	( parentOpType == KINETIC_LAW_OP_DELAY ) ||
	( parentOpType == KINETIC_LAW_OP_EQ ) ||
	( parentOpType == KINETIC_LAW_OP_NEQ ) ||
	( parentOpType == KINETIC_LAW_OP_GEQ ) ||
	( parentOpType == KINETIC_LAW_OP_GT ) ||
	( parentOpType == KINETIC_LAW_OP_LEQ ) ||
	( parentOpType == KINETIC_LAW_OP_LT ) ||
	( parentOpType == KINETIC_LAW_OP_MOD ) ||
	( parentOpType == KINETIC_LAW_OP_UNIFORM ) ||
	( parentOpType == KINETIC_LAW_OP_GAMMA ) ||
	( parentOpType == KINETIC_LAW_OP_NORMAL ) ||
	( parentOpType == KINETIC_LAW_OP_LOGNORMAL ) ||
	( parentOpType == KINETIC_LAW_OP_BINOMIAL ) ||
	( parentOpType == KINETIC_LAW_OP_BITWISE_AND ) ||
	( parentOpType == KINETIC_LAW_OP_BITWISE_OR ) ||
	( parentOpType == KINETIC_LAW_OP_BITWISE_XOR ) ||
	( parentOpType == KINETIC_LAW_OP_BIT ) ||
	( parentOpType == KINETIC_LAW_UNARY_OP_RATE ) ||
	( parentOpType == KINETIC_LAW_OP_IMPLIES ) ||
	( parentOpType == KINETIC_LAW_OP_MIN ) ||
	( parentOpType == KINETIC_LAW_OP_MAX ) ||
	( parentOpType == KINETIC_LAW_OP_QUOTIENT ) ||
	( parentOpType == KINETIC_LAW_OP_AND ) ||
	( parentOpType == KINETIC_LAW_OP_XOR ) ||
	( parentOpType == KINETIC_LAW_OP_OR ) ) {
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
    if( ( parentOpType == KINETIC_LAW_OP_POW ) ||
	( parentOpType == KINETIC_LAW_OP_LOG ) ||
	( parentOpType == KINETIC_LAW_OP_ROOT ) ||
	( parentOpType == KINETIC_LAW_OP_DELAY ) ||
	( parentOpType == KINETIC_LAW_OP_EQ ) ||
	( parentOpType == KINETIC_LAW_OP_NEQ ) ||
	( parentOpType == KINETIC_LAW_OP_GEQ ) ||
	( parentOpType == KINETIC_LAW_OP_GT ) ||
	( parentOpType == KINETIC_LAW_OP_LEQ ) ||
	( parentOpType == KINETIC_LAW_OP_LT ) ||
	( parentOpType == KINETIC_LAW_OP_MOD ) ||
	( parentOpType == KINETIC_LAW_OP_UNIFORM ) ||
	( parentOpType == KINETIC_LAW_OP_GAMMA ) ||
	( parentOpType == KINETIC_LAW_OP_NORMAL ) ||
	( parentOpType == KINETIC_LAW_OP_LOGNORMAL ) ||
	( parentOpType == KINETIC_LAW_OP_BINOMIAL ) ||
	( parentOpType == KINETIC_LAW_OP_BITWISE_AND ) ||
	( parentOpType == KINETIC_LAW_OP_BITWISE_OR ) ||
	( parentOpType == KINETIC_LAW_OP_BITWISE_XOR ) ||
	( parentOpType == KINETIC_LAW_OP_BIT ) ||
	( parentOpType == KINETIC_LAW_UNARY_OP_RATE ) ||
	( parentOpType == KINETIC_LAW_OP_IMPLIES ) ||
	( parentOpType == KINETIC_LAW_OP_MIN ) ||
	( parentOpType == KINETIC_LAW_OP_MAX ) ||
	( parentOpType == KINETIC_LAW_OP_QUOTIENT ) ||
	( parentOpType == KINETIC_LAW_OP_AND ) ||
	( parentOpType == KINETIC_LAW_OP_XOR ) ||
	( parentOpType == KINETIC_LAW_OP_OR ) ) {
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
        if( ( childOpType == KINETIC_LAW_OP_POW ) ||
	    ( childOpType == KINETIC_LAW_OP_LOG ) ||
	    ( childOpType == KINETIC_LAW_OP_ROOT ) ||
	    ( childOpType == KINETIC_LAW_OP_DELAY ) ||
	    ( childOpType == KINETIC_LAW_OP_EQ ) ||
	    ( childOpType == KINETIC_LAW_OP_NEQ ) ||
	    ( childOpType == KINETIC_LAW_OP_GEQ ) ||
	    ( childOpType == KINETIC_LAW_OP_GT ) ||
	    ( childOpType == KINETIC_LAW_OP_LEQ ) ||
	    ( childOpType == KINETIC_LAW_OP_LT ) ||
	    ( childOpType == KINETIC_LAW_OP_MOD ) ||
	    ( childOpType == KINETIC_LAW_OP_UNIFORM ) ||
	    ( childOpType == KINETIC_LAW_OP_GAMMA ) ||
	    ( childOpType == KINETIC_LAW_OP_NORMAL ) ||
	    ( childOpType == KINETIC_LAW_OP_LOGNORMAL ) ||
	    ( childOpType == KINETIC_LAW_OP_BINOMIAL ) ||
	    ( childOpType == KINETIC_LAW_OP_BITWISE_AND ) ||
	    ( childOpType == KINETIC_LAW_OP_BITWISE_OR ) ||
	    ( childOpType == KINETIC_LAW_OP_BITWISE_XOR ) ||
	    ( childOpType == KINETIC_LAW_OP_BIT ) ||
	    ( childOpType == KINETIC_LAW_UNARY_OP_RATE ) ||
	    ( childOpType == KINETIC_LAW_OP_IMPLIES ) ||
	    ( childOpType == KINETIC_LAW_OP_MIN ) ||
	    ( childOpType == KINETIC_LAW_OP_MAX ) ||
	    ( parentOpType == KINETIC_LAW_OP_QUOTIENT ) ||
	    ( childOpType == KINETIC_LAW_OP_AND ) ||
	    ( childOpType == KINETIC_LAW_OP_XOR ) ||
	    ( childOpType == KINETIC_LAW_OP_OR ) ) {
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
    KINETIC_LAW *aChild;
    KINETIC_LAW *bChild;
    if( a->valueType != b->valueType ) {
        return FALSE;
    }
    switch( a->valueType ) {
        case KINETIC_LAW_VALUE_TYPE_PW:
            if( a->value.pw.opType != b->value.pw.opType ) {
                return FALSE;
            }
	    if (GetLinkedListSize(a->value.pw.children) != GetLinkedListSize(b->value.pw.children)) {
	      return FALSE;
	    }
	    ResetCurrentElement( a->value.pw.children );
	    while ( aChild = (KINETIC_LAW*)GetNextFromLinkedList( a->value.pw.children )) {
	      aChild = (KINETIC_LAW*)GetNextFromLinkedList( a->value.pw.children );
	      if ( !AreKineticLawsStructurallyEqual( aChild, bChild ))
		return FALSE;
	    }
	    return TRUE;
        case KINETIC_LAW_VALUE_TYPE_OP:
            if( a->value.op.opType != b->value.op.opType ) {
                return FALSE;
            }
            return ( AreKineticLawsStructurallyEqual( a->value.op.left, b->value.op.left ) &&
                     AreKineticLawsStructurallyEqual( a->value.op.right, b->value.op.right ) ) ?
                    TRUE : FALSE;                     

        case KINETIC_LAW_VALUE_TYPE_UNARY_OP:
            if( a->value.unaryOp.opType != b->value.unaryOp.opType ) {
                return FALSE;
            }
            return ( AreKineticLawsStructurallyEqual( a->value.unaryOp.child, b->value.unaryOp.child ) ) ?
                    TRUE : FALSE;                     
        
        case KINETIC_LAW_VALUE_TYPE_INT:
            return ( a->value.intValue == b->value.intValue ) ? TRUE : FALSE; 
        
        case KINETIC_LAW_VALUE_TYPE_REAL:
            return ( a->value.realValue == b->value.realValue ) ? TRUE : FALSE; 
        
        case KINETIC_LAW_VALUE_TYPE_SPECIES:
            return ( a->value.species == b->value.species ) ? TRUE : FALSE; 
        
        case KINETIC_LAW_VALUE_TYPE_COMPARTMENT:
            return ( a->value.compartment == b->value.compartment ) ? TRUE : FALSE; 
        
        case KINETIC_LAW_VALUE_TYPE_SYMBOL:
            return ( a->value.symbol == b->value.symbol ) ? TRUE : FALSE;             

        case KINETIC_LAW_VALUE_TYPE_FUNCTION_SYMBOL:
            return ( a->value.funcSymbol == b->value.funcSymbol ) ? TRUE : FALSE;             
    }    
}
