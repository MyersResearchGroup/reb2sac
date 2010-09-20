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
#include "ir2xhtml_transformer.h"
#include "symtab.h"

static RET_VAL _PrintConstantsInXHTML( IR *ir, FILE *file );
static RET_VAL _PrintConstantInXHTML( REB2SAC_SYMBOL *sym, FILE *file );

static RET_VAL _PrintListOfReactionsInXHTML( LINKED_LIST *list, FILE *file ); 
static RET_VAL _PrintReactionInXHTML( REACTION *reaction, FILE *file ); 
static RET_VAL _PrintListOfReactantsInXHTML( LINKED_LIST *list, FILE *file ); 
static RET_VAL _PrintListOfProductsInXHTML( LINKED_LIST *list, FILE *file ); 
static RET_VAL _PrintListOfModifiersInXHTML( LINKED_LIST *list, FILE *file ); 
static RET_VAL _PrintListOfSpeciesInXHTML( LINKED_LIST *list, FILE *file ); 
static RET_VAL _PrintFunctionListInXHTML( LINKED_LIST *list, FILE *file ); 
static RET_VAL _PrintUnitListInXHTML( LINKED_LIST *list, FILE *file ); 
static RET_VAL _PrintRuleListInXHTML( LINKED_LIST *list, FILE *file ); 
static RET_VAL _PrintConstraintListInXHTML( LINKED_LIST *list, FILE *file ); 
static RET_VAL _PrintEventListInXHTML( LINKED_LIST *list, FILE *file ); 
static RET_VAL _PrintEventAssignmentInXHTML( LINKED_LIST *assignments, FILE *file ); 
static RET_VAL _PrintCompartmentListInXHTML( LINKED_LIST *list, FILE *file ); 
static RET_VAL _PrintSpeciesListInXHTML( LINKED_LIST *list, FILE *file ); 
static RET_VAL _PrintKineticLawInXHTML( KINETIC_LAW *kineticLaw, FILE *file ); 
static RET_VAL _PrintMathInXHTML( KINETIC_LAW *kineticLaw, char *LHS, FILE *file ); 

static void _PrintTab( FILE *file, int tabCount );

static RET_VAL _VisitPWToPrintInXHTML( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitUnaryOpToPrintInXHTML( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitOpToPrintInXHTML( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitIntToPrintInXHTML( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitRealToPrintInXHTML( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitFunctionSymbolToPrintInXHTML( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitCompartmentToPrintInXHTML( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSpeciesToPrintInXHTML( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSymbolToPrintInXHTML( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );

static BOOL _NeedParenForLeft( KINETIC_LAW *parent, KINETIC_LAW *child );
static BOOL _NeedParenForRight( KINETIC_LAW *parent, KINETIC_LAW *child );


RET_VAL GenerateXHTMLFromIR( IR *ir, FILE *file ) {
    RET_VAL ret = SUCCESS;
    LINKED_LIST *list = NULL;
    
    START_FUNCTION("GenerateXHTMLFromIR");

    fprintf( file, REB2SAC_XHTML_START_FORMAT_ON_LINE );
    fprintf( file, NEW_LINE );
    fprintf( file, NEW_LINE );
        
    if( IS_FAILED( ( ret = _PrintConstantsInXHTML( ir, file ) ) ) ) {
        END_FUNCTION("GenerateXHTMLFromIR", ret );
        return ret;
    }
    fprintf( file, REB2SAC_XHTML_LINE_BREAK );
    fprintf( file, REB2SAC_XHTML_LINE_BREAK );

    list = ir->GetListOfReactionNodes( ir );
    if( IS_FAILED( ( ret = _PrintListOfReactionsInXHTML( list, file ) ) ) ) {
        END_FUNCTION("GenerateXHTMLFromIR", ret );
        return ret;
    }
    fprintf( file, NEW_LINE );
    
    fprintf( file, REB2SAC_XHTML_END_FORMAT );
    fprintf( file, NEW_LINE );
    
    END_FUNCTION("GenerateXHTMLFromIR", SUCCESS );
    return ret;
}

RET_VAL PrintConstantsInXHTML( IR *ir, FILE *file ) {
    START_FUNCTION("_PrintConstantsInXHTML");
    END_FUNCTION("_PrintConstantsInXHTML", SUCCESS );
    return _PrintConstantsInXHTML( ir, file );
}

static RET_VAL _PrintConstantsInXHTML( IR *ir, FILE *file ) {
    RET_VAL ret = SUCCESS;
    REB2SAC_SYMBOL *symbol = NULL;
    REB2SAC_SYMTAB *symtab = NULL;
    LINKED_LIST *list = NULL;
        
    START_FUNCTION("_PrintConstantsInXHTML");
    
    fprintf( file, REB2SAC_XHTML_START_CONSTANTS_FORMAT );
    
    symtab = ir->GetGlobalSymtab( ir );
    if( ( list = symtab->GenerateListOfSymbols( symtab ) ) == NULL ) {
        return ErrorReport( FAILING, "_PrintConstantsInXHTML", "could not generate a list of symbols" );
    }
    ResetCurrentElement( list );
    while( ( symbol = (REB2SAC_SYMBOL*)GetNextFromLinkedList( list ) ) != NULL ) {
        if( IS_FAILED( ( ret = _PrintConstantInXHTML( symbol, file ) ) ) ) {
            END_FUNCTION("_PrintConstantsInXHTML", ret );        
            return ret;
        }
    }
    fprintf( file, REB2SAC_XHTML_END_CONSTANTS_FORMAT );
    fprintf( file, NEW_LINE );
    fprintf( file, REB2SAC_XHTML_LINE_BREAK );
    fprintf( file, REB2SAC_XHTML_LINE_BREAK );
    
    DeleteLinkedList( &list );
    END_FUNCTION("_PrintConstantsInXHTML", SUCCESS );        
    return ret;
}

static RET_VAL _PrintConstantInXHTML( REB2SAC_SYMBOL *sym, FILE *file ) {
    RET_VAL ret = SUCCESS;
    char *id = NULL;
    double value = 0.0;
    char *units;
    char empty[5];
    KINETIC_LAW *law;

    START_FUNCTION("_PrintConstantInXHTML");
    
    /*
    if( !IsSymbolConstant( sym ) ) {
        END_FUNCTION("_PrintConstantInXHTML", SUCCESS );        
        return ret;
    }
    if( !IsRealValueSymbol( sym ) ) {
        END_FUNCTION("_PrintConstantInXHTML", SUCCESS );        
        return ret;
    }
    */
    if (strcmp(GetCharArrayOfString( GetSymbolID( sym ) ),"t")==0 ||
	strcmp(GetCharArrayOfString( GetSymbolID( sym ) ),"time")==0) {
        END_FUNCTION("_PrintConstantInXHTML", SUCCESS );        
        return ret;
    }

    id = GetCharArrayOfString( GetSymbolID( sym ) );
    value = GetRealValueInSymbol( sym );
    strcpy(empty,"none");
    if (GetUnitsInSymbol ( sym ) != NULL) {
      units = GetCharArrayOfString( GetUnitDefinitionID( GetUnitsInSymbol( sym ) ) );
    } else {
      units = empty;
    }
    
    fprintf( file, REB2SAC_XHTML_START_CONSTANT_ENTRY_FORMAT, id );
    if (GetInitialAssignmentInSymbol( sym ) != NULL) {
      law = (KINETIC_LAW*)GetInitialAssignmentInSymbol( sym );
      if( IS_FAILED( ( ret = _PrintMathInXHTML( law, NULL, file ) ) ) ) {
	END_FUNCTION("_PrintRuleForXHTML", ret );
	return ret;
      }
    } else {
      fprintf( file, "%g", value );
    }
    fprintf( file, REB2SAC_XHTML_END_CONSTANT_ENTRY_FORMAT, units,
	     IsSymbolConstant( sym ) ? "True" : "False");
    fprintf( file, NEW_LINE );
    
    END_FUNCTION("_PrintConstantInXHTML", SUCCESS );        
    return ret;
}

RET_VAL PrintListOfReactionsInXHTML( LINKED_LIST *list, FILE *file ) {
    START_FUNCTION("PrintListOfReactionsInXHTML");
    END_FUNCTION("PrintListOfReactionsInXHTML", SUCCESS );
    return _PrintListOfReactionsInXHTML( list, file );
}

RET_VAL PrintReactionInXHTML( REACTION *reaction, FILE *file ) {
    START_FUNCTION("PrintReactionInXHTML");
    END_FUNCTION("PrintReactionInXHTML", SUCCESS );
    return _PrintReactionInXHTML( reaction, file );
}

RET_VAL PrintListOfReactantsInXHTML( LINKED_LIST *list, FILE *file ) {
    START_FUNCTION("PrintListOfReactantsInXHTML");
    END_FUNCTION("PrintListOfReactantsInXHTML", SUCCESS );
    return _PrintListOfReactantsInXHTML( list, file );
}

RET_VAL PrintListOfProductsInXHTML( LINKED_LIST *list, FILE *file ) {
    START_FUNCTION("PrintListOfProductsInXHTML");
    END_FUNCTION("PrintListOfProductsInXHTML", SUCCESS );
    return _PrintListOfProductsInXHTML( list, file );
}

RET_VAL PrintListOfModifiersInXHTML( LINKED_LIST *list, FILE *file ) {
    START_FUNCTION("PrintListOfModifiersInXHTML");
    END_FUNCTION("PrintListOfModifiersInXHTML", SUCCESS );
    return _PrintListOfModifiersInXHTML( list, file );
}

RET_VAL PrintListOfSpeciesInXHTML( LINKED_LIST *list, FILE *file ) {
    START_FUNCTION("PrintListOfSpeciesInXHTML");
    END_FUNCTION("PrintListOfSpeciesInXHTML", SUCCESS );
    return _PrintListOfSpeciesInXHTML( list, file );
}

RET_VAL PrintFunctionListInXHTML( LINKED_LIST *list, FILE *file ) {
    START_FUNCTION("PrintFunctionListInXHTML");
    END_FUNCTION("PrintFunctionListInXHTML", SUCCESS );
    return _PrintFunctionListInXHTML( list, file );
}

RET_VAL PrintUnitListInXHTML( LINKED_LIST *list, FILE *file ) {
    START_FUNCTION("PrintUnitListInXHTML");
    END_FUNCTION("PrintUnitListInXHTML", SUCCESS );
    return _PrintUnitListInXHTML( list, file );
}

RET_VAL PrintRuleListInXHTML( LINKED_LIST *list, FILE *file ) {
    START_FUNCTION("PrintRuleListInXHTML");
    END_FUNCTION("PrintRuleListInXHTML", SUCCESS );
    return _PrintRuleListInXHTML( list, file );
}

RET_VAL PrintConstraintListInXHTML( LINKED_LIST *list, FILE *file ) {
    START_FUNCTION("PrintConstraintListInXHTML");
    END_FUNCTION("PrintConstraintListInXHTML", SUCCESS );
    return _PrintConstraintListInXHTML( list, file );
}

RET_VAL PrintEventListInXHTML( LINKED_LIST *list, FILE *file ) {
    START_FUNCTION("PrintEventListInXHTML");
    END_FUNCTION("PrintEventListInXHTML", SUCCESS );
    return _PrintEventListInXHTML( list, file );
}

RET_VAL PrintCompartmentListInXHTML( LINKED_LIST *list, FILE *file ) {
    START_FUNCTION("PrintCompartmentListInXHTML");
    END_FUNCTION("PrintCompartmentListInXHTML", SUCCESS );
    return _PrintCompartmentListInXHTML( list, file );
}

RET_VAL PrintSpeciesListInXHTML( LINKED_LIST *list, FILE *file ) {
    START_FUNCTION("PrintSpeciesListInXHTML");
    END_FUNCTION("PrintSpeciesListInXHTML", SUCCESS );
    return _PrintSpeciesListInXHTML( list, file );
}

RET_VAL PrintKineticLawInXHTML( KINETIC_LAW *kineticLaw, FILE *file ) {
    START_FUNCTION("PrintKineticLawInXHTML");
    END_FUNCTION("PrintKineticLawInXHTML", SUCCESS );
    return _PrintKineticLawInXHTML( kineticLaw, file );
}


static RET_VAL _PrintListOfReactionsInXHTML( LINKED_LIST *list, FILE *file ) {
    RET_VAL ret = SUCCESS;
    REACTION *reaction = NULL;
    
    START_FUNCTION("_PrintListOfReactionsInXHTML");

    fprintf( file, REB2SAC_XHTML_START_REACTION_FORMAT );
    ResetCurrentElement( list );
    while( ( reaction = (REACTION*)GetNextFromLinkedList( list ) )  != NULL ) {
        if( IS_FAILED( ( ret = _PrintReactionInXHTML( reaction, file ) ) ) ) {
            END_FUNCTION("_PrintListOfReactionsInXHTML", ret );
            return ret;
        }
        fprintf( file, REB2SAC_XHTML_LINE_BREAK );
    }
    fprintf( file, REB2SAC_XHTML_END_REACTION_FORMAT );
    
    END_FUNCTION("_PrintListOfReactionsInXHTML", SUCCESS );
    return ret;
}

static RET_VAL _PrintReactionInXHTML( REACTION *reaction, FILE *file ) {
    RET_VAL ret = SUCCESS;
    KINETIC_LAW *kineticLaw = NULL;
    LINKED_LIST *list = NULL;
    
    START_FUNCTION("_PrintReactionInXHTML");

    fprintf( file, REB2SAC_XHTML_START_REACTION_ENTRY_FORMAT, GetCharArrayOfString( GetReactionNodeName( reaction ) ),
	     IsReactionReversibleInReactionNode( reaction ) ? "True" : "False",
	     IsReactionFastInReactionNode( reaction ) ? "True" : "False");
    
    list = GetReactantsInReactionNode( reaction );
    if( IS_FAILED( ( ret = _PrintListOfReactantsInXHTML( list, file ) ) ) ) {
        END_FUNCTION("_PrintReactionInXHTML", ret );
        return ret;
    }    
    fprintf( file, REB2SAC_XHTML_SEPARATOR_FORMAT );
    
    list = GetProductsInReactionNode( reaction );
    if( IS_FAILED( ( ret = _PrintListOfProductsInXHTML( list, file ) ) ) ) {
        END_FUNCTION("_PrintReactionInXHTML", ret );
        return ret;
    }    
    fprintf( file, REB2SAC_XHTML_SEPARATOR_FORMAT );

    list = GetModifiersInReactionNode( reaction );
    if( IS_FAILED( ( ret = _PrintListOfModifiersInXHTML( list, file ) ) ) ) {
        END_FUNCTION("_PrintReactionInXHTML", ret );
        return ret;
    }    
    fprintf( file, REB2SAC_XHTML_SEPARATOR_FORMAT );
        
    kineticLaw = GetKineticLawInReactionNode( reaction );
    if( IS_FAILED( ( ret = _PrintKineticLawInXHTML( kineticLaw, file ) ) ) ) {
        END_FUNCTION("_PrintReactionInXHTML", ret );
        return ret;
    }    
    fprintf( file, REB2SAC_XHTML_END_REACTION_ENTRY_FORMAT );
    fprintf( file, NEW_LINE );
    
    END_FUNCTION("_PrintReactionInXHTML", SUCCESS );
    return ret;
}


static RET_VAL _PrintListOfReactantsInXHTML( LINKED_LIST *list, FILE *file ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("_PrintListOfReactantsInXHTML");
    
    //fprintf( file, REB2SAC_XHTML_START_REACTANTS_FORMAT );
    
    if( IS_FAILED( ( ret = _PrintListOfSpeciesInXHTML( list, file ) ) ) ) {
        END_FUNCTION("_PrintListOfReactantsInXHTML", ret );
        return ret;
    }    
    
    //fprintf( file, REB2SAC_XHTML_END_REACTANTS_FORMAT );
    //fprintf( file, NEW_LINE );
    
    END_FUNCTION("_PrintListOfReactantsInXHTML", SUCCESS );
    return ret;
}

static RET_VAL _PrintListOfProductsInXHTML( LINKED_LIST *list, FILE *file ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("_PrintListOfProductsInXHTML");
    
    //    fprintf( file, REB2SAC_XHTML_START_PRODUCTS_FORMAT );
    
    if( IS_FAILED( ( ret = _PrintListOfSpeciesInXHTML( list, file ) ) ) ) {
        END_FUNCTION("_PrintListOfProductsInXHTML", ret );
        return ret;
    }    
    
    //fprintf( file, REB2SAC_XHTML_END_PRODUCTS_FORMAT );
    //fprintf( file, NEW_LINE );
    
    END_FUNCTION("_PrintListOfProductsInXHTML", SUCCESS );
    return ret;
}

static RET_VAL _PrintListOfModifiersInXHTML( LINKED_LIST *list, FILE *file ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("_PrintListOfModifiersInXHTML");
    
    //fprintf( file, REB2SAC_XHTML_START_MODIFIERS_FORMAT );
    
    if( IS_FAILED( ( ret = _PrintListOfSpeciesInXHTML( list, file ) ) ) ) {
        END_FUNCTION("_PrintListOfModifiersInXHTML", ret );
        return ret;
    }    
    
    //fprintf( file, REB2SAC_XHTML_END_MODIFIERS_FORMAT );
    //fprintf( file, NEW_LINE );
    
    END_FUNCTION("_PrintListOfModifiersInXHTML", SUCCESS );
    return ret;
}

static RET_VAL _PrintListOfSpeciesInXHTML( LINKED_LIST *list, FILE *file ) {
    RET_VAL ret = SUCCESS;
    double stoichiometry = 0.0;
    SPECIES *species = NULL;
    IR_EDGE *edge = NULL;
    REB2SAC_SYMBOL *speciesRef = NULL;
    
    START_FUNCTION("_PrintListOfSpeciesInXHTML");
    
    ResetCurrentElement( list );
    if( ( edge = (IR_EDGE*)GetNextFromLinkedList( list ) ) == NULL ) {
        END_FUNCTION("_PrintListOfSpeciesInXHTML", SUCCESS );
        return ret;
    } 
    
    species = GetSpeciesInIREdge( edge );
    if ( ( speciesRef = GetSpeciesRefInIREdge( edge ) ) != NULL ) {
      fprintf( file, REB2SAC_XHTML_SPECIES_FORMAT3, 
	       GetCharArrayOfString( GetSymbolID( speciesRef ) ),
	       GetCharArrayOfString( GetSpeciesNodeName( species ) ) );
    } else {
      stoichiometry = GetStoichiometryInIREdge( edge );
      if( stoichiometry == 1.0 ) {    
        fprintf( file, REB2SAC_XHTML_SPECIES_FORMAT, GetCharArrayOfString( GetSpeciesNodeName( species ) ) );
      }
      else {
        fprintf( file, REB2SAC_XHTML_SPECIES_FORMAT2, 
            stoichiometry, GetCharArrayOfString( GetSpeciesNodeName( species ) ) );
      }
    }
    while( ( edge = (IR_EDGE*)GetNextFromLinkedList( list ) ) != NULL ) {
        species = GetSpeciesInIREdge( edge );
	if ( ( speciesRef = GetSpeciesRefInIREdge( edge ) ) != NULL ) {
            fprintf( file, REB2SAC_XHTML_MORE_SPECIES_FORMAT3, 
		     GetCharArrayOfString( GetSymbolID( speciesRef ) ),
		     GetCharArrayOfString( GetSpeciesNodeName( species ) ) );
	} else {
	  stoichiometry = GetStoichiometryInIREdge( edge );
	  if( stoichiometry == 1.0 ) {
            fprintf( file, REB2SAC_XHTML_MORE_SPECIES_FORMAT, GetCharArrayOfString( GetSpeciesNodeName( species ) ) );
	  }
	  else {
            fprintf( file, REB2SAC_XHTML_MORE_SPECIES_FORMAT2, 
		     stoichiometry, GetCharArrayOfString( GetSpeciesNodeName( species ) ) );
	  }
	}
    }
    
    END_FUNCTION("_PrintListOfSpeciesInXHTML", SUCCESS );
    return ret;
}

static RET_VAL _PrintFunctionForXHTML( FUNCTION_DEFINITION *function, FILE *file ) {
    RET_VAL ret = SUCCESS;
    KINETIC_LAW *kineticLaw;
    LINKED_LIST *arguments;
    char *argument;
    int num;
    int i;
    char funcLabel[256];

    START_FUNCTION("_PrintFunctionForXHTML");

    fprintf( file, REB2SAC_XHTML_START_FUNCTION_ENTRY_FORMAT );
    sprintf( funcLabel, " %s ( ", GetCharArrayOfString( GetFunctionDefinitionID( function ) ) );

    arguments = GetArgumentsInFunctionDefinition( function );
    num = GetLinkedListSize(arguments);
    for (i = 0; i < num; i++) {
      argument = (char*)GetElementByIndex(i,arguments);
      if (i > 0) {
	strcat( funcLabel, ", ");
      }
      strcat( funcLabel, argument );
    }
    strcat( funcLabel, " )" );

    kineticLaw = GetFunctionInFunctionDefinition( function );
    if( IS_FAILED( ( ret = _PrintMathInXHTML( kineticLaw, funcLabel, file ) ) ) ) {
        END_FUNCTION("_PrintFunctionForXHTML", ret );
        return ret;
    }    
    fprintf( file, REB2SAC_XHTML_END_FUNCTION_ENTRY_FORMAT );

    fprintf( file, NEW_LINE );
            
    END_FUNCTION("_PrintFunctionForXHTML", SUCCESS );
    
    return ret;
}

static RET_VAL _PrintFunctionListInXHTML( LINKED_LIST *list, FILE *file ) {
    RET_VAL ret = SUCCESS;
    FUNCTION_DEFINITION *function = NULL;

    START_FUNCTION("_PrintFunctionListInXHTML");
    if( ( list == NULL ) || ( GetLinkedListSize( list ) == 0 ) ) {
        END_FUNCTION("_PrintListOfFunctionsForSBML", SUCCESS );    
        return ret;
    }    
    fprintf( file, REB2SAC_XHTML_START_FUNCTION_FORMAT );
    ResetCurrentElement( list );
    while( ( function = (FUNCTION_DEFINITION*)GetNextFromLinkedList( list ) ) != NULL ) {
        if( IS_FAILED( ( ret = _PrintFunctionForXHTML( function, file ) ) ) ) {
            END_FUNCTION("_PrintFunctionListInXHTML", ret );    
            return ret;
        }
    }
    
    fprintf( file, REB2SAC_XHTML_END_FUNCTION_FORMAT );
    fprintf( file, NEW_LINE );
    fprintf( file, REB2SAC_XHTML_LINE_BREAK );
    fprintf( file, REB2SAC_XHTML_LINE_BREAK );
    
    END_FUNCTION("_PrintFunctionListInXHTML", SUCCESS );
    return ret;
}
static RET_VAL _PrintUnitForXHTML( UNIT_DEFINITION *unitDef, FILE *file ) {
    RET_VAL ret = SUCCESS;
    LINKED_LIST *units;
    UNIT *unit;
    int num;
    int i;
    int scale;
    double exp;
    double mult;
    
    START_FUNCTION("_PrintUnitForXHTML");

    fprintf( file, REB2SAC_XHTML_START_UNITDEF_ENTRY_FORMAT,
	     GetCharArrayOfString( GetUnitDefinitionID( unitDef ) ) );

    units = GetUnitsInUnitDefinition( unitDef );
    num = GetLinkedListSize( units );
    for (i = 0; i < num; i++) {
      unit = (UNIT*)GetElementByIndex( i, units );
      scale = GetScaleInUnit( unit );
      exp = GetExponentInUnit( unit );
      mult = GetMultiplierInUnit( unit );
      if (i != 0) {
	fprintf( file, REB2SAC_XHTML_MATHML_BULLET_FORMAT );
      }
      if (exp != 1) {
	fprintf( file, REB2SAC_XHTML_MATHML_L_PAREN_FORMAT );
      }
      if (mult != 1.0) {
	fprintf( file, "%g", mult );
	fprintf( file, REB2SAC_XHTML_MATHML_BULLET_FORMAT );
      } 
      if (scale != 0) {
	fprintf( file, "10" );
	fprintf( file, REB2SAC_XHTML_MATHML_SUP_INT_FORMAT, scale );
	fprintf( file, REB2SAC_XHTML_MATHML_BULLET_FORMAT );
      }
      fprintf( file, "%s ", GetCharArrayOfString( GetKindInUnit( unit ) ) );
      if (exp != 1) {
	fprintf( file, REB2SAC_XHTML_MATHML_R_PAREN_FORMAT );
	fprintf( file, REB2SAC_XHTML_MATHML_SUP_REAL_FORMAT, exp );
      }
    }

    fprintf( file, REB2SAC_XHTML_END_UNITDEF_ENTRY_FORMAT );
    fprintf( file, NEW_LINE );
            
    END_FUNCTION("_PrintUnitForXHTML", SUCCESS );
    
    return ret;
}

static RET_VAL _PrintUnitListInXHTML( LINKED_LIST *list, FILE *file ) {
    RET_VAL ret = SUCCESS;
    UNIT_DEFINITION *unitDef = NULL;
    int count = 0;

    START_FUNCTION("_PrintUnitListInXHTML");
    if( ( list == NULL ) || ( GetLinkedListSize( list ) == 0 ) ) {
        END_FUNCTION("_PrintListOfUnitsForSBML", SUCCESS );    
        return ret;
    }    
    ResetCurrentElement( list );
    while( ( unitDef = (UNIT_DEFINITION*)GetNextFromLinkedList( list ) ) != NULL ) {
      if ( !IsBuiltInUnitDefinition( unitDef ) ) {
	count++;
      }
    }
    if (count==0) return ret;
    fprintf( file, REB2SAC_XHTML_START_UNITDEF_FORMAT );
    ResetCurrentElement( list );
    while( ( unitDef = (UNIT_DEFINITION*)GetNextFromLinkedList( list ) ) != NULL ) {
      if ( !IsBuiltInUnitDefinition( unitDef ) ) {
        if( IS_FAILED( ( ret = _PrintUnitForXHTML( unitDef, file ) ) ) ) {
            END_FUNCTION("_PrintUnitListInXHTML", ret );    
            return ret;
        }
      }
    }
    fprintf( file, REB2SAC_XHTML_END_UNITDEF_FORMAT );
    fprintf( file, NEW_LINE );
    fprintf( file, REB2SAC_XHTML_LINE_BREAK );
    fprintf( file, REB2SAC_XHTML_LINE_BREAK );
    
    END_FUNCTION("_PrintUnitListInXHTML", SUCCESS );
    return ret;
}

static RET_VAL _PrintRuleForXHTML( RULE *rule, FILE *file ) {
    RET_VAL ret = SUCCESS;
    KINETIC_LAW *kineticLaw;
    BYTE type;
    char ruleLHS[256];

    START_FUNCTION("_PrintRuleForXHTML");
    
    type = GetRuleType( rule );
    
    if (type == RULE_TYPE_ALGEBRAIC ) {
      strcpy(ruleLHS,"0");
    } else if (type == RULE_TYPE_ASSIGNMENT ) {
      strcpy(ruleLHS,GetCharArrayOfString( GetRuleVar( rule ) ) );
    } else {
      sprintf(ruleLHS, "d(%s)/dt",
	      GetCharArrayOfString( GetRuleVar( rule ) ) );
    }
    fprintf( file, REB2SAC_XHTML_START_RULE_ENTRY_FORMAT );
    kineticLaw = GetMathInRule( rule );
    if( IS_FAILED( ( ret = _PrintMathInXHTML( kineticLaw, ruleLHS, file ) ) ) ) {
        END_FUNCTION("_PrintRuleForXHTML", ret );
        return ret;
    }    
    fprintf( file, REB2SAC_XHTML_END_RULE_ENTRY_FORMAT );

    fprintf( file, NEW_LINE );
            
    END_FUNCTION("_PrintRuleForXHTML", SUCCESS );
    
    return ret;
}

static RET_VAL _PrintRuleListInXHTML( LINKED_LIST *list, FILE *file ) {
    RET_VAL ret = SUCCESS;
    RULE *rule = NULL;

    START_FUNCTION("_PrintRuleListInXHTML");
    if( ( list == NULL ) || ( GetLinkedListSize( list ) == 0 ) ) {
        END_FUNCTION("_PrintListOfRulesForSBML", SUCCESS );    
        return ret;
    }    
    fprintf( file, REB2SAC_XHTML_START_RULE_FORMAT );
    ResetCurrentElement( list );
    while( ( rule = (RULE*)GetNextFromLinkedList( list ) ) != NULL ) {
        if( IS_FAILED( ( ret = _PrintRuleForXHTML( rule, file ) ) ) ) {
            END_FUNCTION("_PrintRuleListInXHTML", ret );    
            return ret;
        }
    }
    
    fprintf( file, REB2SAC_XHTML_END_RULE_FORMAT );
    fprintf( file, NEW_LINE );
    fprintf( file, REB2SAC_XHTML_LINE_BREAK );
    fprintf( file, REB2SAC_XHTML_LINE_BREAK );
    
    END_FUNCTION("_PrintRuleListInXHTML", SUCCESS );
    return ret;
}

static RET_VAL _PrintConstraintForXHTML( CONSTRAINT *constraint, FILE *file ) {
    RET_VAL ret = SUCCESS;
    KINETIC_LAW *kineticLaw;

    START_FUNCTION("_PrintConstraintForXHTML");

    fprintf( file, REB2SAC_XHTML_START_CONSTRAINT_ENTRY_FORMAT,
	     GetCharArrayOfString( GetConstraintId( constraint ) ) );

    kineticLaw = GetMathInConstraint( constraint );
    if( IS_FAILED( ( ret = _PrintMathInXHTML( kineticLaw, NULL, file ) ) ) ) {
        END_FUNCTION("_PrintRuleForXHTML", ret );
        return ret;
    }    
    if (GetConstraintMessage( constraint )==NULL) {
      fprintf( file, REB2SAC_XHTML_END_CONSTRAINT_ENTRY_FORMAT, "" ); 
    } else {
      fprintf( file, REB2SAC_XHTML_END_CONSTRAINT_ENTRY_FORMAT, 
	       GetCharArrayOfString( GetConstraintMessage( constraint ) ) );
    }

    fprintf( file, NEW_LINE );
            
    END_FUNCTION("_PrintConstraintForXHTML", SUCCESS );
    
    return ret;
}

static RET_VAL _PrintConstraintListInXHTML( LINKED_LIST *list, FILE *file ) {
    RET_VAL ret = SUCCESS;
    CONSTRAINT *constraint = NULL;

    START_FUNCTION("_PrintConstraintListInXHTML");
    if( ( list == NULL ) || ( GetLinkedListSize( list ) == 0 ) ) {
        END_FUNCTION("_PrintListOfConstraintsForSBML", SUCCESS );    
        return ret;
    }    
    fprintf( file, REB2SAC_XHTML_START_CONSTRAINT_FORMAT );
    ResetCurrentElement( list );
    while( ( constraint = (CONSTRAINT*)GetNextFromLinkedList( list ) ) != NULL ) {
        if( IS_FAILED( ( ret = _PrintConstraintForXHTML( constraint, file ) ) ) ) {
            END_FUNCTION("_PrintRuleListInXHTML", ret );    
            return ret;
        }
    }
    
    fprintf( file, REB2SAC_XHTML_END_CONSTRAINT_FORMAT );
    fprintf( file, NEW_LINE );
    fprintf( file, REB2SAC_XHTML_LINE_BREAK );
    fprintf( file, REB2SAC_XHTML_LINE_BREAK );
    
    END_FUNCTION("_PrintConstraintListInXHTML", SUCCESS );
    return ret;
}

static RET_VAL _PrintEventAssignmentInXHTML( LINKED_LIST *assignments, FILE *file ) {
    static KINETIC_LAW_VISITOR visitor;
    RET_VAL ret = SUCCESS;
    int tabCount = 1;
    EVENT_ASSIGNMENT *assignment;
    BOOL first = TRUE;
    KINETIC_LAW *kineticLaw;

    START_FUNCTION("_PrintMathInXHTML");
    
    if( visitor.VisitOp == NULL ) {
        visitor.VisitPW = _VisitPWToPrintInXHTML;
        visitor.VisitUnaryOp = _VisitUnaryOpToPrintInXHTML;
        visitor.VisitOp = _VisitOpToPrintInXHTML;
        visitor.VisitInt = _VisitIntToPrintInXHTML;
        visitor.VisitReal = _VisitRealToPrintInXHTML;
        visitor.VisitFunctionSymbol = _VisitFunctionSymbolToPrintInXHTML;
        visitor.VisitCompartment = _VisitCompartmentToPrintInXHTML;
        visitor.VisitSpecies = _VisitSpeciesToPrintInXHTML;
        visitor.VisitSymbol = _VisitSymbolToPrintInXHTML;
    }
    
    visitor._internal1 = (CADDR_T)file;
    visitor._internal2 = (CADDR_T)(&tabCount);
    
    _PrintTab( file, tabCount );
    fprintf( file, REB2SAC_XHTML_MATHML_START_MATH_FORMAT );
    fprintf( file, NEW_LINE );
    tabCount++;
    
    _PrintTab( file, tabCount );
    fprintf( file, REB2SAC_XHTML_MATHML_START_SUBGROUP_FORMAT );
    fprintf( file, NEW_LINE );
    tabCount++;

    ResetCurrentElement( assignments );
    while( ( assignment = (EVENT_ASSIGNMENT*)GetNextFromLinkedList( assignments ) ) != NULL ) {
      if (first) {
	first = FALSE;
      } else {
	fprintf( file, REB2SAC_XHTML_MATHML_COMMA_FORMAT );
      }
      fprintf( file, REB2SAC_XHTML_START_ASSIGN_FORMAT,
	       GetCharArrayOfString( assignment->var ) );
      kineticLaw = assignment->assignment;
      if (kineticLaw) {
	if( IS_FAILED( ( ret = kineticLaw->Accept( kineticLaw, &visitor ) ) ) ) {
	  END_FUNCTION("_PrintKineticLawInXHTML", ret );
	  return ret;
	}
      }
      fprintf( file, REB2SAC_XHTML_END_ASSIGN_FORMAT );
    }
    
    tabCount--;
    _PrintTab( file, tabCount );
    fprintf( file, REB2SAC_XHTML_MATHML_END_SUBGROUP_FORMAT );
    fprintf( file, NEW_LINE );
    
    tabCount--;
    _PrintTab( file, tabCount );
    fprintf( file, REB2SAC_XHTML_MATHML_END_MATH_FORMAT );
    fprintf( file, NEW_LINE );
    
    END_FUNCTION("_PrintMathInXHTML", SUCCESS );
    return ret;
}

static RET_VAL _PrintEventForXHTML( EVENT *event, FILE *file ) {
    RET_VAL ret = SUCCESS;
    KINETIC_LAW *kineticLaw;
    LINKED_LIST *assignments;

    START_FUNCTION("_PrintEventForXHTML");

    fprintf( file, REB2SAC_XHTML_START_EVENT_ENTRY_FORMAT,
	     GetCharArrayOfString( GetEventId( event ) ) );

    kineticLaw = GetTriggerInEvent( event );
    if (kineticLaw) {
      if( IS_FAILED( ( ret = _PrintMathInXHTML( kineticLaw, NULL, file ) ) ) ) {
        END_FUNCTION("_PrintRuleForXHTML", ret );
        return ret;
      }
    }
    fprintf( file, REB2SAC_XHTML_SEPARATOR_FORMAT );

    kineticLaw = GetDelayInEvent( event );
    if (kineticLaw) {
      if( IS_FAILED( ( ret = _PrintMathInXHTML( kineticLaw, NULL, file ) ) ) ) {
        END_FUNCTION("_PrintRuleForXHTML", ret );
        return ret;
      }   
    } 
    fprintf( file, REB2SAC_XHTML_SEPARATOR_FORMAT );

    kineticLaw = GetPriorityInEvent( event );
    if (kineticLaw) {
      if( IS_FAILED( ( ret = _PrintMathInXHTML( kineticLaw, NULL, file ) ) ) ) {
        END_FUNCTION("_PrintRuleForXHTML", ret );
        return ret;
      }   
    } 
    fprintf( file, REB2SAC_XHTML_SEPARATOR_FORMAT );

    if (GetUseValuesFromTriggerTime( event )) {
      fprintf( file, "True" );
    } else {
      fprintf( file, "False" );
    }
    fprintf( file, REB2SAC_XHTML_SEPARATOR_FORMAT );

    if (GetTriggerCanBeDisabled( event )) {
      fprintf( file, "True" );
    } else {
      fprintf( file, "False" );
    }
    fprintf( file, REB2SAC_XHTML_SEPARATOR_FORMAT );

    if (GetTriggerInitialValue( event )) {
      fprintf( file, "True" );
    } else {
      fprintf( file, "False" );
    }
    fprintf( file, REB2SAC_XHTML_SEPARATOR_FORMAT );

    assignments = GetEventAssignments( event );
    if( IS_FAILED( ( ret = _PrintEventAssignmentInXHTML( assignments, file ) ) ) ) {
      END_FUNCTION("_PrintRuleForXHTML", ret );
      return ret;
    }   

    fprintf( file, REB2SAC_XHTML_END_EVENT_ENTRY_FORMAT );
            
    END_FUNCTION("_PrintEventForXHTML", SUCCESS );
    
    return ret;
}

static RET_VAL _PrintEventListInXHTML( LINKED_LIST *list, FILE *file ) {
    RET_VAL ret = SUCCESS;
    EVENT *event = NULL;

    START_FUNCTION("_PrintEventListInXHTML");

    if( ( list == NULL ) || ( GetLinkedListSize( list ) == 0 ) ) {
        END_FUNCTION("_PrintListOfEventsForSBML", SUCCESS );    
        return ret;
    }    

    fprintf( file, REB2SAC_XHTML_START_EVENT_FORMAT );

    ResetCurrentElement( list );
    while( ( event = (EVENT*)GetNextFromLinkedList( list ) ) != NULL ) {
        if( IS_FAILED( ( ret = _PrintEventForXHTML( event, file ) ) ) ) {
            END_FUNCTION("_PrintEventListInXHTML", ret );    
            return ret;
        }
    }

    fprintf( file, REB2SAC_XHTML_END_EVENT_FORMAT );
    fprintf( file, NEW_LINE );
    fprintf( file, REB2SAC_XHTML_LINE_BREAK );
    fprintf( file, REB2SAC_XHTML_LINE_BREAK );
    
    END_FUNCTION("_PrintEventListInXHTML", SUCCESS );
    return ret;
}

static RET_VAL _PrintCompartmentForXHTML( COMPARTMENT *compartment, FILE *file ) {
    RET_VAL ret = SUCCESS;
    char *outside;
    char *type;
    char empty[5];
    char *units;
    KINETIC_LAW *law;

    START_FUNCTION("_PrintCompartmentForXHTML");

    strcpy(empty,"none");
    if (GetOutsideInCompartment ( compartment ) != NULL) {
      outside = GetCharArrayOfString( GetOutsideInCompartment( compartment ) );
    } else {
      outside = empty;
    }
    if (GetTypeInCompartment ( compartment ) != NULL) {
      type = GetCharArrayOfString( GetTypeInCompartment( compartment ) );
    } else {
      type = empty;
    }
    if (GetUnitInCompartment ( compartment ) != NULL) {
      units = GetCharArrayOfString( GetUnitDefinitionID( GetUnitInCompartment( compartment ) ) );
    } else {
      units = empty;
    }
    fprintf( file, REB2SAC_XHTML_START_COMPARTMENT_ENTRY_FORMAT,
	     GetCharArrayOfString( GetCompartmentID( compartment ) ), 
	     GetSpatialDimensionsInCompartment( compartment ) );
    /*
    fprintf( file, REB2SAC_XHTML_START_COMPARTMENT_ENTRY_FORMAT,
	     GetCharArrayOfString( GetCompartmentID( compartment ) ), type,
	     GetSpatialDimensionsInCompartment( compartment ) );*/
    if (GetInitialAssignmentInCompartment( compartment ) != NULL) {
      law = (KINETIC_LAW*)GetInitialAssignmentInCompartment( compartment );
      if( IS_FAILED( ( ret = _PrintMathInXHTML( law, NULL, file ) ) ) ) {
	END_FUNCTION("_PrintRuleForXHTML", ret );
	return ret;
      }
    } else {
      fprintf( file, "%g",GetSizeInCompartment( compartment ) );
    }
    fprintf( file, REB2SAC_XHTML_END_COMPARTMENT_ENTRY_FORMAT,
	     empty, IsCompartmentConstant( compartment ) ? "True" : "False");
    /*fprintf( file, REB2SAC_XHTML_END_COMPARTMENT_ENTRY_FORMAT,
      empty, outside, IsCompartmentConstant( compartment ) ? "True" : "False");*/

    fprintf( file, NEW_LINE );
            
    END_FUNCTION("_PrintCompartmentForXHTML", SUCCESS );
    
    return ret;
}

static RET_VAL _PrintCompartmentListInXHTML( LINKED_LIST *list, FILE *file ) {
    RET_VAL ret = SUCCESS;
    COMPARTMENT *compartment = NULL;
    
    START_FUNCTION("_PrintCompartmentListInXHTML");
    if( ( list == NULL ) || ( GetLinkedListSize( list ) == 0 ) ) {
        END_FUNCTION("_PrintListOfCompartmentsForSBML", SUCCESS );    
        return ret;
    }    

    fprintf( file, REB2SAC_XHTML_START_COMPARTMENT_FORMAT );

    ResetCurrentElement( list );
    while( ( compartment = (COMPARTMENT*)GetNextFromLinkedList( list ) ) != NULL ) {
        if( IS_FAILED( ( ret = _PrintCompartmentForXHTML( compartment, file ) ) ) ) {
            END_FUNCTION("_PrintCompartmentListInXHTML", ret );    
            return ret;
        }
    }
    
    fprintf( file, REB2SAC_XHTML_END_COMPARTMENT_FORMAT );
    fprintf( file, NEW_LINE );
    fprintf( file, REB2SAC_XHTML_LINE_BREAK );
    fprintf( file, REB2SAC_XHTML_LINE_BREAK );
    
    END_FUNCTION("_PrintCompartmentListInXHTML", SUCCESS );
    return ret;
}

static RET_VAL _PrintSpeciesForXHTML( SPECIES *species, FILE *file ) {
    RET_VAL ret = SUCCESS;
    double initialQuantity = 0.0;
    COMPARTMENT *compartment = NULL;
    char *units;
    char *type;
    char *convFactor;
    char empty[5];
    char none[5];
    KINETIC_LAW *law;
    
    START_FUNCTION("_PrintSpeciesForXHTML");
        
    compartment = GetCompartmentInSpeciesNode( species );
    
    strcpy(none,"none");
    if (GetSubstanceUnitsInSpeciesNode ( species ) != NULL) {
      units = GetCharArrayOfString( GetUnitDefinitionID( GetSubstanceUnitsInSpeciesNode( species ) ) );
    } else {
      units = none;
    }
    if (GetConversionFactorInSpeciesNode ( species ) != NULL) {
      convFactor = GetCharArrayOfString( GetSymbolID( GetConversionFactorInSpeciesNode( species )));
    } else {
      convFactor = none;
    }
    strcpy(empty,"");
    if (GetTypeInSpeciesNode ( species ) != NULL) {
      type = GetCharArrayOfString( GetTypeInSpeciesNode( species ) );
    } else {
      type = empty;
    }
    fprintf( file, REB2SAC_XHTML_START_SPECIES_ENTRY_FORMAT,
	     GetCharArrayOfString( GetSpeciesNodeID( species ) ), 
	     GetCharArrayOfString( GetCompartmentID( compartment ) ) );
    /*
    fprintf( file, REB2SAC_XHTML_START_SPECIES_ENTRY_FORMAT,
	     GetCharArrayOfString( GetSpeciesNodeID( species ) ), type,
	     GetCharArrayOfString( GetCompartmentID( compartment ) ) );
    */
    if (GetInitialAssignmentInSpeciesNode( species ) != NULL) {
      law = (KINETIC_LAW*)GetInitialAssignmentInSpeciesNode( species );
      if( IS_FAILED( ( ret = _PrintMathInXHTML( law, NULL, file ) ) ) ) {
	END_FUNCTION("_PrintRuleForXHTML", ret );
	return ret;
      }    
    } else {
      if( IsInitialQuantityInAmountInSpeciesNode( species ) ) {
	initialQuantity = GetInitialAmountInSpeciesNode( species );
	fprintf( file, "%g (amount)",initialQuantity );
      } else {
	initialQuantity = GetInitialConcentrationInSpeciesNode( species );
	fprintf( file, "%g (conc.)",initialQuantity );
      }
    }
    fprintf( file, REB2SAC_XHTML_END_SPECIES_ENTRY_FORMAT,
	     units, convFactor,
	     HasBoundaryConditionInSpeciesNode( species ) ? "True" : "False",
	     IsSpeciesNodeConstant( species ) ? "True" : "False",
	     HasOnlySubstanceUnitsInSpeciesNode( species ) ? "True" : "False");
    /*
    fprintf( file, REB2SAC_XHTML_END_SPECIES_ENTRY_FORMAT,
	     units, 
	     HasBoundaryConditionInSpeciesNode( species ) ? "True" : "False",
	     IsSpeciesNodeConstant( species ) ? "True" : "False",
	     HasOnlySubstanceUnitsInSpeciesNode( species ) ? "True" : "False");
    */
    fprintf( file, NEW_LINE );
            
    END_FUNCTION("_PrintSpeciesForXHTML", SUCCESS );
    
    return ret;
}

static RET_VAL _PrintSpeciesListInXHTML( LINKED_LIST *list, FILE *file ) {
    RET_VAL ret = SUCCESS;
    SPECIES *species = NULL;
    
    START_FUNCTION("_PrintSpeciesListInXHTML");
    if( ( list == NULL ) || ( GetLinkedListSize( list ) == 0 ) ) {
        END_FUNCTION("_PrintListOfSpeciesForSBML", SUCCESS );    
        return ret;
    }    

    fprintf( file, REB2SAC_XHTML_START_SPECIES_FORMAT );

    ResetCurrentElement( list );
    while( ( species = (SPECIES*)GetNextFromLinkedList( list ) ) != NULL ) {
        if( IS_FAILED( ( ret = _PrintSpeciesForXHTML( species, file ) ) ) ) {
            END_FUNCTION("_PrintSpeciesListInXHTML", ret );    
            return ret;
        }
    }
    
    fprintf( file, REB2SAC_XHTML_END_SPECIES_FORMAT );
    fprintf( file, NEW_LINE );
    fprintf( file, REB2SAC_XHTML_LINE_BREAK );
    fprintf( file, REB2SAC_XHTML_LINE_BREAK );
    
    END_FUNCTION("_PrintSpeciesListInXHTML", SUCCESS );
    return ret;
}

static RET_VAL _PrintMathInXHTML( KINETIC_LAW *kineticLaw, char * LHS, FILE *file ) {
    static KINETIC_LAW_VISITOR visitor;
    RET_VAL ret = SUCCESS;
    int tabCount = 1;
    
    START_FUNCTION("_PrintMathInXHTML");
    
    if( visitor.VisitOp == NULL ) {
        visitor.VisitPW = _VisitPWToPrintInXHTML;
        visitor.VisitUnaryOp = _VisitUnaryOpToPrintInXHTML;
        visitor.VisitOp = _VisitOpToPrintInXHTML;
        visitor.VisitInt = _VisitIntToPrintInXHTML;
        visitor.VisitReal = _VisitRealToPrintInXHTML;
        visitor.VisitFunctionSymbol = _VisitFunctionSymbolToPrintInXHTML;
        visitor.VisitCompartment = _VisitCompartmentToPrintInXHTML;
        visitor.VisitSpecies = _VisitSpeciesToPrintInXHTML;
        visitor.VisitSymbol = _VisitSymbolToPrintInXHTML;
    }
    
    visitor._internal1 = (CADDR_T)file;
    visitor._internal2 = (CADDR_T)(&tabCount);
    
    _PrintTab( file, tabCount );
    fprintf( file, REB2SAC_XHTML_MATHML_START_MATH_FORMAT );
    fprintf( file, NEW_LINE );
    tabCount++;
    
    _PrintTab( file, tabCount );
    fprintf( file, REB2SAC_XHTML_MATHML_START_SUBGROUP_FORMAT );
    fprintf( file, NEW_LINE );
    tabCount++;
        
    if (LHS != NULL) {
      fprintf( file, "%s = ",LHS);
    }
    if( IS_FAILED( ( ret = kineticLaw->Accept( kineticLaw, &visitor ) ) ) ) {
        END_FUNCTION("_PrintKineticLawInXHTML", ret );
        return ret;
    }
    
    tabCount--;
    _PrintTab( file, tabCount );
    fprintf( file, REB2SAC_XHTML_MATHML_END_SUBGROUP_FORMAT );
    fprintf( file, NEW_LINE );
    
    tabCount--;
    _PrintTab( file, tabCount );
    fprintf( file, REB2SAC_XHTML_MATHML_END_MATH_FORMAT );
    fprintf( file, NEW_LINE );
    
    END_FUNCTION("_PrintMathInXHTML", SUCCESS );
    return ret;
}

static RET_VAL _PrintKineticLawInXHTML( KINETIC_LAW *kineticLaw, FILE *file ) {
    static KINETIC_LAW_VISITOR visitor;
    RET_VAL ret = SUCCESS;
    int tabCount = 1;
    
    START_FUNCTION("_PrintKineticLawInXHTML");

    //fprintf( file, REB2SAC_XHTML_START_KINETIC_LAW_FORMAT );
    //fprintf( file, NEW_LINE );
    
    _PrintMathInXHTML( kineticLaw, NULL, file );
    
    //fprintf( file, REB2SAC_XHTML_END_KINETIC_LAW_FORMAT );
    //fprintf( file, NEW_LINE );
    
    END_FUNCTION("_PrintKineticLawInXHTML", SUCCESS );
    return ret;
}

static void _PrintTab( FILE *file, int tabCount ) {
    int i = 0;
    
    for( i = 0; i < tabCount; i++ ) {
        fprintf( file, __IR2XHTML_TAB );
    }
}

static RET_VAL _VisitPWToPrintInXHTML( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    int *tabCount = NULL;
    FILE *file = NULL;
    LINKED_LIST *children = NULL;
    KINETIC_LAW *child = NULL;
    BOOL first = TRUE;

    START_FUNCTION("_VisitPWToPrintInXHTML");

    file = (FILE*)(visitor->_internal1);
    tabCount = (int*)(visitor->_internal2);
    
    children = GetPWChildrenFromKineticLaw( kineticLaw );
    ResetCurrentElement( children );
    _PrintTab( file, *tabCount );
    switch( GetPWTypeFromKineticLaw( kineticLaw ) ) {
    case KINETIC_LAW_OP_PW:
      fprintf( file, REB2SAC_XHTML_MATHML_OP_PW_FORMAT );
      break;
    case KINETIC_LAW_OP_AND:
      fprintf( file, REB2SAC_XHTML_MATHML_OP_AND_FORMAT );
      break;
    case KINETIC_LAW_OP_OR:
      fprintf( file, REB2SAC_XHTML_MATHML_OP_OR_FORMAT );
      break;
    case KINETIC_LAW_OP_XOR:
      fprintf( file, REB2SAC_XHTML_MATHML_OP_XOR_FORMAT );
      break;
    }

    (*tabCount)++;
    _PrintTab( file, *tabCount );
    fprintf( file, REB2SAC_XHTML_MATHML_START_SUBGROUP_FORMAT );             
    fprintf( file, NEW_LINE );             

    (*tabCount)++;
    _PrintTab( file, *tabCount );
    fprintf( file, REB2SAC_XHTML_MATHML_L_PAREN_FORMAT );
    fprintf( file, NEW_LINE );
    (*tabCount)++;

    while( ( child = (KINETIC_LAW*)GetNextFromLinkedList( children ) ) != NULL ) {
      if (first) {
	first = FALSE;
      } else {
	fprintf( file, REB2SAC_XHTML_MATHML_COMMA_FORMAT );
      }
      if( IS_FAILED( ( ret = child->Accept( child, visitor ) ) ) ) {
	END_FUNCTION("_VisitPWToPrintInXHTML", ret );
	return ret;
      }      
    }
    
    (*tabCount)--;
    _PrintTab( file, *tabCount );
    fprintf( file, REB2SAC_XHTML_MATHML_R_PAREN_FORMAT );
    fprintf( file, NEW_LINE );
	    
    (*tabCount)--;
    _PrintTab( file, *tabCount );
    fprintf( file, REB2SAC_XHTML_MATHML_END_SUBGROUP_FORMAT );
    fprintf( file, NEW_LINE );
    (*tabCount)--;
}

static RET_VAL _VisitUnaryOpToPrintInXHTML( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    BYTE unaryOpType = 0;
    int *tabCount = NULL;
    FILE *file = NULL;
    KINETIC_LAW *child = NULL;
    
    START_FUNCTION("_VisitUnaryOpToPrintInXHTML");

    file = (FILE*)(visitor->_internal1);
    tabCount = (int*)(visitor->_internal2);
    
    unaryOpType = GetUnaryOpTypeFromKineticLaw( kineticLaw );
    child = GetUnaryOpChildFromKineticLaw( kineticLaw );
    
    switch( unaryOpType ) {
    case KINETIC_LAW_UNARY_OP_COS:
    case KINETIC_LAW_UNARY_OP_COSH:
    case KINETIC_LAW_UNARY_OP_SIN:
    case KINETIC_LAW_UNARY_OP_SINH:
    case KINETIC_LAW_UNARY_OP_TAN:
    case KINETIC_LAW_UNARY_OP_TANH:
    case KINETIC_LAW_UNARY_OP_COT:
    case KINETIC_LAW_UNARY_OP_COTH:
    case KINETIC_LAW_UNARY_OP_CSC:
    case KINETIC_LAW_UNARY_OP_CSCH:
    case KINETIC_LAW_UNARY_OP_SEC:
    case KINETIC_LAW_UNARY_OP_SECH:
    case KINETIC_LAW_UNARY_OP_ARCCOS:
    case KINETIC_LAW_UNARY_OP_ARCCOSH:
    case KINETIC_LAW_UNARY_OP_ARCSIN:
    case KINETIC_LAW_UNARY_OP_ARCSINH:
    case KINETIC_LAW_UNARY_OP_ARCTAN:
    case KINETIC_LAW_UNARY_OP_ARCTANH:
    case KINETIC_LAW_UNARY_OP_ARCCOT:
    case KINETIC_LAW_UNARY_OP_ARCCOTH:
    case KINETIC_LAW_UNARY_OP_ARCCSC:
    case KINETIC_LAW_UNARY_OP_ARCCSCH:
    case KINETIC_LAW_UNARY_OP_ARCSEC:
    case KINETIC_LAW_UNARY_OP_ARCSECH:
    case KINETIC_LAW_UNARY_OP_LN:
    case KINETIC_LAW_UNARY_OP_LOG:
    case KINETIC_LAW_UNARY_OP_FACTORIAL:
    case KINETIC_LAW_UNARY_OP_ABS:
    case KINETIC_LAW_UNARY_OP_FLOOR:
    case KINETIC_LAW_UNARY_OP_CEILING:
    case KINETIC_LAW_UNARY_OP_NOT:
    case KINETIC_LAW_UNARY_OP_NEG:
    case KINETIC_LAW_UNARY_OP_EXP:
    case KINETIC_LAW_UNARY_OP_EXPRAND:
    case KINETIC_LAW_UNARY_OP_POISSON:
    case KINETIC_LAW_UNARY_OP_CHISQ:
    case KINETIC_LAW_UNARY_OP_LAPLACE:
    case KINETIC_LAW_UNARY_OP_CAUCHY:
    case KINETIC_LAW_UNARY_OP_RAYLEIGH:
    case KINETIC_LAW_UNARY_OP_BERNOULLI:
    case KINETIC_LAW_UNARY_OP_BITWISE_NOT:
    case KINETIC_LAW_UNARY_OP_INT:
	    if ( unaryOpType == KINETIC_LAW_UNARY_OP_EXP ) { 
	      _PrintTab( file, *tabCount );
	      fprintf( file, REB2SAC_XHTML_MATHML_START_POWER_FORMAT );            
	      fprintf( file, NEW_LINE );
	      (*tabCount)++;
	      fprintf( file, REB2SAC_XHTML_MATHML_OP_EXP_FORMAT );
	    }
            _PrintTab( file, *tabCount );
            fprintf( file, REB2SAC_XHTML_MATHML_START_SUBGROUP_FORMAT );             
            fprintf( file, NEW_LINE );
	    if ( unaryOpType == KINETIC_LAW_UNARY_OP_COS ) { 
	      fprintf( file, REB2SAC_XHTML_MATHML_OP_COS_FORMAT );
	    } else if ( unaryOpType == KINETIC_LAW_UNARY_OP_COSH ) { 
	      fprintf( file, REB2SAC_XHTML_MATHML_OP_COSH_FORMAT );
	    } else if ( unaryOpType == KINETIC_LAW_UNARY_OP_SIN ) { 
	      fprintf( file, REB2SAC_XHTML_MATHML_OP_SIN_FORMAT );
	    } else if ( unaryOpType == KINETIC_LAW_UNARY_OP_SINH ) { 
	      fprintf( file, REB2SAC_XHTML_MATHML_OP_SIN_FORMAT );
	    } else if ( unaryOpType == KINETIC_LAW_UNARY_OP_TAN ) { 
	      fprintf( file, REB2SAC_XHTML_MATHML_OP_TAN_FORMAT );
	    } else if ( unaryOpType == KINETIC_LAW_UNARY_OP_TANH ) { 
	      fprintf( file, REB2SAC_XHTML_MATHML_OP_TANH_FORMAT );
	    } else if ( unaryOpType == KINETIC_LAW_UNARY_OP_COT ) { 
	      fprintf( file, REB2SAC_XHTML_MATHML_OP_COT_FORMAT );
	    } else if ( unaryOpType == KINETIC_LAW_UNARY_OP_COTH ) { 
	      fprintf( file, REB2SAC_XHTML_MATHML_OP_COTH_FORMAT );
	    } else if ( unaryOpType == KINETIC_LAW_UNARY_OP_CSC ) { 
	      fprintf( file, REB2SAC_XHTML_MATHML_OP_CSC_FORMAT );
	    } else if ( unaryOpType == KINETIC_LAW_UNARY_OP_CSCH ) { 
	      fprintf( file, REB2SAC_XHTML_MATHML_OP_CSCH_FORMAT );
	    } else if ( unaryOpType == KINETIC_LAW_UNARY_OP_SEC ) { 
	      fprintf( file, REB2SAC_XHTML_MATHML_OP_SEC_FORMAT );
	    } else if ( unaryOpType == KINETIC_LAW_UNARY_OP_SECH ) { 
	      fprintf( file, REB2SAC_XHTML_MATHML_OP_SECH_FORMAT );
	    } else if ( unaryOpType == KINETIC_LAW_UNARY_OP_ARCCOS ) { 
	      fprintf( file, REB2SAC_XHTML_MATHML_OP_ARCCOS_FORMAT );
	    } else if ( unaryOpType == KINETIC_LAW_UNARY_OP_ARCCOSH ) { 
	      fprintf( file, REB2SAC_XHTML_MATHML_OP_COSH_FORMAT );
	    } else if ( unaryOpType == KINETIC_LAW_UNARY_OP_ARCSIN ) { 
	      fprintf( file, REB2SAC_XHTML_MATHML_OP_ARCSIN_FORMAT );
	    } else if ( unaryOpType == KINETIC_LAW_UNARY_OP_ARCSINH ) { 
	      fprintf( file, REB2SAC_XHTML_MATHML_OP_ARCSIN_FORMAT );
	    } else if ( unaryOpType == KINETIC_LAW_UNARY_OP_ARCTAN ) { 
	      fprintf( file, REB2SAC_XHTML_MATHML_OP_ARCTAN_FORMAT );
	    } else if ( unaryOpType == KINETIC_LAW_UNARY_OP_ARCTANH ) { 
	      fprintf( file, REB2SAC_XHTML_MATHML_OP_ARCTANH_FORMAT );
	    } else if ( unaryOpType == KINETIC_LAW_UNARY_OP_ARCCOT ) { 
	      fprintf( file, REB2SAC_XHTML_MATHML_OP_ARCCOT_FORMAT );
	    } else if ( unaryOpType == KINETIC_LAW_UNARY_OP_ARCCOTH ) { 
	      fprintf( file, REB2SAC_XHTML_MATHML_OP_ARCCOTH_FORMAT );
	    } else if ( unaryOpType == KINETIC_LAW_UNARY_OP_ARCCSC ) { 
	      fprintf( file, REB2SAC_XHTML_MATHML_OP_ARCCSC_FORMAT );
	    } else if ( unaryOpType == KINETIC_LAW_UNARY_OP_ARCCSCH ) { 
	      fprintf( file, REB2SAC_XHTML_MATHML_OP_ARCCSCH_FORMAT );
	    } else if ( unaryOpType == KINETIC_LAW_UNARY_OP_ARCSEC ) { 
	      fprintf( file, REB2SAC_XHTML_MATHML_OP_ARCSEC_FORMAT );
	    } else if ( unaryOpType == KINETIC_LAW_UNARY_OP_ARCSECH ) { 
	      fprintf( file, REB2SAC_XHTML_MATHML_OP_ARCSECH_FORMAT );
	    } else if ( unaryOpType == KINETIC_LAW_UNARY_OP_LOG ) { 
	      fprintf( file, REB2SAC_XHTML_MATHML_OP_LOG_FORMAT );
	    } else if ( unaryOpType == KINETIC_LAW_UNARY_OP_LN ) { 
	      fprintf( file, REB2SAC_XHTML_MATHML_OP_LN_FORMAT );
	    } else if ( unaryOpType == KINETIC_LAW_UNARY_OP_NOT ) { 
	      fprintf( file, REB2SAC_XHTML_MATHML_OP_NOT_FORMAT );
	    } else if ( unaryOpType == KINETIC_LAW_UNARY_OP_NEG ) { 
	      fprintf( file, REB2SAC_XHTML_MATHML_OP_NEG_FORMAT );
	    } else if ( unaryOpType == KINETIC_LAW_UNARY_OP_EXPRAND ) { 
	      fprintf( file, REB2SAC_XHTML_MATHML_OP_EXPRAND_FORMAT );
	    } else if ( unaryOpType == KINETIC_LAW_UNARY_OP_POISSON ) { 
	      fprintf( file, REB2SAC_XHTML_MATHML_OP_POISSON_FORMAT );
	    } else if ( unaryOpType == KINETIC_LAW_UNARY_OP_CHISQ ) { 
	      fprintf( file, REB2SAC_XHTML_MATHML_OP_CHISQ_FORMAT );
	    } else if ( unaryOpType == KINETIC_LAW_UNARY_OP_LAPLACE ) { 
	      fprintf( file, REB2SAC_XHTML_MATHML_OP_LAPLACE_FORMAT );
	    } else if ( unaryOpType == KINETIC_LAW_UNARY_OP_CAUCHY ) { 
	      fprintf( file, REB2SAC_XHTML_MATHML_OP_CAUCHY_FORMAT );
	    } else if ( unaryOpType == KINETIC_LAW_UNARY_OP_RAYLEIGH ) { 
	      fprintf( file, REB2SAC_XHTML_MATHML_OP_RAYLEIGH_FORMAT );
	    } else if ( unaryOpType == KINETIC_LAW_UNARY_OP_BERNOULLI ) { 
	      fprintf( file, REB2SAC_XHTML_MATHML_OP_BERNOULLI_FORMAT );
	    } else if ( unaryOpType == KINETIC_LAW_UNARY_OP_BITWISE_NOT ) { 
	      fprintf( file, REB2SAC_XHTML_MATHML_OP_BITWISE_NOT_FORMAT );
	    } else if ( unaryOpType == KINETIC_LAW_UNARY_OP_INT ) { 
	      fprintf( file, REB2SAC_XHTML_MATHML_OP_INT_FORMAT );
	    } 

            (*tabCount)++;
	    _PrintTab( file, *tabCount );
	    fprintf( file, REB2SAC_XHTML_MATHML_START_SUBGROUP_FORMAT );             
	    fprintf( file, NEW_LINE );             

	    (*tabCount)++;
	    _PrintTab( file, *tabCount );
	    if ( unaryOpType == KINETIC_LAW_UNARY_OP_ABS ) { 
	      fprintf( file, REB2SAC_XHTML_MATHML_ABS_FORMAT );
	    } else if ( unaryOpType == KINETIC_LAW_UNARY_OP_FLOOR ) { 
	      fprintf( file, REB2SAC_XHTML_MATHML_L_FLOOR_FORMAT );
	    } else if ( unaryOpType == KINETIC_LAW_UNARY_OP_CEILING ) { 
	      fprintf( file, REB2SAC_XHTML_MATHML_L_CEILING_FORMAT );
	    } else {
	      fprintf( file, REB2SAC_XHTML_MATHML_L_PAREN_FORMAT );
	    } 
	    fprintf( file, NEW_LINE );
	    (*tabCount)++;
                
	    if( IS_FAILED( ( ret = child->Accept( child, visitor ) ) ) ) {
	      END_FUNCTION("_VisitOpToPrintInXHTML", ret );
	      return ret;
	    }             
            
	    (*tabCount)--;
	    _PrintTab( file, *tabCount );
	    if ( unaryOpType == KINETIC_LAW_UNARY_OP_ABS ) { 
	      fprintf( file, REB2SAC_XHTML_MATHML_ABS_FORMAT );
	    } else if ( unaryOpType == KINETIC_LAW_UNARY_OP_FLOOR ) { 
	      fprintf( file, REB2SAC_XHTML_MATHML_R_FLOOR_FORMAT );
	    } else if ( unaryOpType == KINETIC_LAW_UNARY_OP_CEILING ) { 
	      fprintf( file, REB2SAC_XHTML_MATHML_R_CEILING_FORMAT );
	    } else {
	      fprintf( file, REB2SAC_XHTML_MATHML_R_PAREN_FORMAT );
	    } 
	    fprintf( file, NEW_LINE );
	    
	    (*tabCount)--;
	    _PrintTab( file, *tabCount );
	    fprintf( file, REB2SAC_XHTML_MATHML_END_SUBGROUP_FORMAT );
	    fprintf( file, NEW_LINE );
	    if ( unaryOpType == KINETIC_LAW_UNARY_OP_FACTORIAL ) { 
	      fprintf( file, REB2SAC_XHTML_MATHML_OP_FACT_FORMAT );
	    }
            (*tabCount)--;
            _PrintTab( file, *tabCount );
            fprintf( file, REB2SAC_XHTML_MATHML_END_SUBGROUP_FORMAT );
            fprintf( file, NEW_LINE );
	    if ( unaryOpType == KINETIC_LAW_UNARY_OP_EXP ) { 
	      (*tabCount)--;
	      _PrintTab( file, *tabCount );
	      fprintf( file, REB2SAC_XHTML_MATHML_END_POWER_FORMAT );
	      fprintf( file, NEW_LINE );
	    }
        break;
        
        default:
        return ErrorReport( FAILING, "_PrintUnaryOpKineticLawForSBML", "unary op type %c is not valid", unaryOpType );
    }
    
    
    END_FUNCTION("_VisitUnaryOpToPrintInXHTML", SUCCESS );
    return ret;
}

static RET_VAL _VisitOpToPrintInXHTML( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    BYTE opType = 0;
    int *tabCount = NULL;
    FILE *file = NULL;
    KINETIC_LAW *left = NULL;
    KINETIC_LAW *right = NULL;
    
    START_FUNCTION("_VisitOpToPrintInXHTML");

    file = (FILE*)(visitor->_internal1);
    tabCount = (int*)(visitor->_internal2);
    
    opType = GetOpTypeFromKineticLaw( kineticLaw );
    left = GetOpLeftFromKineticLaw( kineticLaw );
    right = GetOpRightFromKineticLaw( kineticLaw );
    
    switch( opType ) {
        case KINETIC_LAW_OP_MOD:
        case KINETIC_LAW_OP_UNIFORM:
        case KINETIC_LAW_OP_GAMMA:
        case KINETIC_LAW_OP_NORMAL:
        case KINETIC_LAW_OP_BINOMIAL:
        case KINETIC_LAW_OP_LOGNORMAL:
        case KINETIC_LAW_OP_BITWISE_AND:
        case KINETIC_LAW_OP_BITWISE_OR:
        case KINETIC_LAW_OP_BITWISE_XOR:
        case KINETIC_LAW_OP_BIT:
        case KINETIC_LAW_OP_DELAY:
	  _PrintTab( file, *tabCount );
	  if (opType == KINETIC_LAW_OP_MOD) {
	    fprintf( file, REB2SAC_XHTML_MATHML_OP_MOD_FORMAT );
	  } else if (opType == KINETIC_LAW_OP_UNIFORM) {
	    fprintf( file, REB2SAC_XHTML_MATHML_OP_UNIFORM_FORMAT );
	  } else if (opType == KINETIC_LAW_OP_GAMMA) {
	    fprintf( file, REB2SAC_XHTML_MATHML_OP_GAMMA_FORMAT );
	  } else if (opType == KINETIC_LAW_OP_BINOMIAL) {
	    fprintf( file, REB2SAC_XHTML_MATHML_OP_BINOMIAL_FORMAT );
	  } else if (opType == KINETIC_LAW_OP_LOGNORMAL) {
	    fprintf( file, REB2SAC_XHTML_MATHML_OP_LOGNORMAL_FORMAT );
	  } else if (opType == KINETIC_LAW_OP_NORMAL) {
	    fprintf( file, REB2SAC_XHTML_MATHML_OP_NORMAL_FORMAT );
	  } else if (opType == KINETIC_LAW_OP_BITWISE_AND) {
	    fprintf( file, REB2SAC_XHTML_MATHML_OP_BITWISE_AND_FORMAT );
	  } else if (opType == KINETIC_LAW_OP_BITWISE_OR) {
	    fprintf( file, REB2SAC_XHTML_MATHML_OP_BITWISE_OR_FORMAT );
	  } else if (opType == KINETIC_LAW_OP_BITWISE_XOR) {
	    fprintf( file, REB2SAC_XHTML_MATHML_OP_BITWISE_XOR_FORMAT );
	  } else if (opType == KINETIC_LAW_OP_BIT) {
	    fprintf( file, REB2SAC_XHTML_MATHML_OP_BIT_FORMAT );
	  } else {
	    fprintf( file, REB2SAC_XHTML_MATHML_OP_DELAY_FORMAT );
	  }
	  (*tabCount)++;
	  _PrintTab( file, *tabCount );
	  fprintf( file, REB2SAC_XHTML_MATHML_START_SUBGROUP_FORMAT );             
	  fprintf( file, NEW_LINE );             

	  (*tabCount)++;
	  _PrintTab( file, *tabCount );
	  fprintf( file, REB2SAC_XHTML_MATHML_L_PAREN_FORMAT );
	  fprintf( file, NEW_LINE );
	  (*tabCount)++;

	  if( IS_FAILED( ( ret = left->Accept( left, visitor ) ) ) ) {
	    END_FUNCTION("_VisitOpToPrintInXHTML", ret );
	    return ret;
	  }             
	  fprintf( file, REB2SAC_XHTML_MATHML_COMMA_FORMAT );
	  if( IS_FAILED( ( ret = right->Accept( right, visitor ) ) ) ) {
	    END_FUNCTION("_VisitOpToPrintInXHTML", ret );
	    return ret;
	  }             
    
	  (*tabCount)--;
	  _PrintTab( file, *tabCount );
	  fprintf( file, REB2SAC_XHTML_MATHML_R_PAREN_FORMAT );
	  fprintf( file, NEW_LINE );
	  
	  (*tabCount)--;
	  _PrintTab( file, *tabCount );
	  fprintf( file, REB2SAC_XHTML_MATHML_END_SUBGROUP_FORMAT );
	  fprintf( file, NEW_LINE );
	  (*tabCount)--;
	break;
        case KINETIC_LAW_OP_GT:
        case KINETIC_LAW_OP_LT:
        case KINETIC_LAW_OP_GEQ:
        case KINETIC_LAW_OP_LEQ:
        case KINETIC_LAW_OP_EQ:
        case KINETIC_LAW_OP_NEQ:
            _PrintTab( file, *tabCount );
            fprintf( file, REB2SAC_XHTML_MATHML_START_SUBGROUP_FORMAT );             
            fprintf( file, NEW_LINE );             
            (*tabCount)++;
            if( _NeedParenForLeft( kineticLaw, left ) ) {
                _PrintTab( file, *tabCount );
                fprintf( file, REB2SAC_XHTML_MATHML_START_SUBGROUP_FORMAT );             
                fprintf( file, NEW_LINE );             
                (*tabCount)++;
                
                _PrintTab( file, *tabCount );
                fprintf( file, REB2SAC_XHTML_MATHML_L_PAREN_FORMAT );
                fprintf( file, NEW_LINE );
                (*tabCount)++;
                
                if( IS_FAILED( ( ret = left->Accept( left, visitor ) ) ) ) {
                    END_FUNCTION("_VisitOpToPrintInXHTML", ret );
                    return ret;
                }             
                
                (*tabCount)--;
                _PrintTab( file, *tabCount );
                fprintf( file, REB2SAC_XHTML_MATHML_R_PAREN_FORMAT );
                fprintf( file, NEW_LINE );

                (*tabCount)--;
                _PrintTab( file, *tabCount );
                fprintf( file, REB2SAC_XHTML_MATHML_END_SUBGROUP_FORMAT );
                fprintf( file, NEW_LINE );
            }
            else {
                if( IS_FAILED( ( ret = left->Accept( left, visitor ) ) ) ) {
                    END_FUNCTION("_VisitOpToPrintInXHTML", ret );
                    return ret;
                }             
            }
            (*tabCount)--;
            _PrintTab( file, *tabCount );
            fprintf( file, REB2SAC_XHTML_MATHML_END_SUBGROUP_FORMAT );
            fprintf( file, NEW_LINE );

            _PrintTab( file, *tabCount );
	    if ( opType == KINETIC_LAW_OP_GT ) { 
	      fprintf( file, REB2SAC_XHTML_MATHML_OP_GT_FORMAT );
	    } else if ( opType == KINETIC_LAW_OP_LT ) { 
	      fprintf( file, REB2SAC_XHTML_MATHML_OP_LT_FORMAT );
	    } else if ( opType == KINETIC_LAW_OP_GEQ ) { 
	      fprintf( file, REB2SAC_XHTML_MATHML_OP_GEQ_FORMAT );
	    } else if ( opType == KINETIC_LAW_OP_LEQ ) {
	      fprintf( file, REB2SAC_XHTML_MATHML_OP_LEQ_FORMAT );
	    } else if ( opType == KINETIC_LAW_OP_EQ ) {
	      fprintf( file, REB2SAC_XHTML_MATHML_OP_EQ_FORMAT );
	    } else if ( opType == KINETIC_LAW_OP_NEQ ) {
	      fprintf( file, REB2SAC_XHTML_MATHML_OP_NEQ_FORMAT );
	    } 
            fprintf( file, NEW_LINE );

            _PrintTab( file, *tabCount );
            fprintf( file, REB2SAC_XHTML_MATHML_START_SUBGROUP_FORMAT );             
            fprintf( file, NEW_LINE );             
            (*tabCount)++;
            if( _NeedParenForRight( kineticLaw, right ) ) {
                _PrintTab( file, *tabCount );
                fprintf( file, REB2SAC_XHTML_MATHML_START_SUBGROUP_FORMAT );             
                fprintf( file, NEW_LINE );             
                (*tabCount)++;
                
                _PrintTab( file, *tabCount );
                fprintf( file, REB2SAC_XHTML_MATHML_L_PAREN_FORMAT );
                fprintf( file, NEW_LINE );
                (*tabCount)++;
                
                if( IS_FAILED( ( ret = right->Accept( right, visitor ) ) ) ) {
                    END_FUNCTION("_VisitOpToPrintInXHTML", ret );
                    return ret;
                }             
                
                (*tabCount)--;
                _PrintTab( file, *tabCount );
                fprintf( file, REB2SAC_XHTML_MATHML_R_PAREN_FORMAT );
                fprintf( file, NEW_LINE );

                (*tabCount)--;
                _PrintTab( file, *tabCount );
                fprintf( file, REB2SAC_XHTML_MATHML_END_SUBGROUP_FORMAT );
                fprintf( file, NEW_LINE );
            }
            else {
                if( IS_FAILED( ( ret = right->Accept( right, visitor ) ) ) ) {
                    END_FUNCTION("_VisitOpToPrintInXHTML", ret );
                    return ret;
                }             
            }
            (*tabCount)--;
            _PrintTab( file, *tabCount );
            fprintf( file, REB2SAC_XHTML_MATHML_END_SUBGROUP_FORMAT );
            fprintf( file, NEW_LINE );
        break;

        case KINETIC_LAW_OP_PLUS:
            _PrintTab( file, *tabCount );
            fprintf( file, REB2SAC_XHTML_MATHML_START_SUBGROUP_FORMAT );             
            fprintf( file, NEW_LINE );             
            (*tabCount)++;
            if( _NeedParenForLeft( kineticLaw, left ) ) {
                _PrintTab( file, *tabCount );
                fprintf( file, REB2SAC_XHTML_MATHML_START_SUBGROUP_FORMAT );             
                fprintf( file, NEW_LINE );             
                (*tabCount)++;
                
                _PrintTab( file, *tabCount );
                fprintf( file, REB2SAC_XHTML_MATHML_L_PAREN_FORMAT );
                fprintf( file, NEW_LINE );
                (*tabCount)++;
                
                if( IS_FAILED( ( ret = left->Accept( left, visitor ) ) ) ) {
                    END_FUNCTION("_VisitOpToPrintInXHTML", ret );
                    return ret;
                }             
                
                (*tabCount)--;
                _PrintTab( file, *tabCount );
                fprintf( file, REB2SAC_XHTML_MATHML_R_PAREN_FORMAT );
                fprintf( file, NEW_LINE );

                (*tabCount)--;
                _PrintTab( file, *tabCount );
                fprintf( file, REB2SAC_XHTML_MATHML_END_SUBGROUP_FORMAT );
                fprintf( file, NEW_LINE );
            }
            else {
                if( IS_FAILED( ( ret = left->Accept( left, visitor ) ) ) ) {
                    END_FUNCTION("_VisitOpToPrintInXHTML", ret );
                    return ret;
                }             
            }
            (*tabCount)--;
            _PrintTab( file, *tabCount );
            fprintf( file, REB2SAC_XHTML_MATHML_END_SUBGROUP_FORMAT );
            fprintf( file, NEW_LINE );

            _PrintTab( file, *tabCount );
            fprintf( file, REB2SAC_XHTML_MATHML_OP_PLUS_FORMAT );
            fprintf( file, NEW_LINE );

            _PrintTab( file, *tabCount );
            fprintf( file, REB2SAC_XHTML_MATHML_START_SUBGROUP_FORMAT );             
            fprintf( file, NEW_LINE );             
            (*tabCount)++;
            if( _NeedParenForRight( kineticLaw, right ) ) {
                _PrintTab( file, *tabCount );
                fprintf( file, REB2SAC_XHTML_MATHML_START_SUBGROUP_FORMAT );             
                fprintf( file, NEW_LINE );             
                (*tabCount)++;
                
                _PrintTab( file, *tabCount );
                fprintf( file, REB2SAC_XHTML_MATHML_L_PAREN_FORMAT );
                fprintf( file, NEW_LINE );
                (*tabCount)++;
                
                if( IS_FAILED( ( ret = right->Accept( right, visitor ) ) ) ) {
                    END_FUNCTION("_VisitOpToPrintInXHTML", ret );
                    return ret;
                }             
                
                (*tabCount)--;
                _PrintTab( file, *tabCount );
                fprintf( file, REB2SAC_XHTML_MATHML_R_PAREN_FORMAT );
                fprintf( file, NEW_LINE );

                (*tabCount)--;
                _PrintTab( file, *tabCount );
                fprintf( file, REB2SAC_XHTML_MATHML_END_SUBGROUP_FORMAT );
                fprintf( file, NEW_LINE );
            }
            else {
                if( IS_FAILED( ( ret = right->Accept( right, visitor ) ) ) ) {
                    END_FUNCTION("_VisitOpToPrintInXHTML", ret );
                    return ret;
                }             
            }
            (*tabCount)--;
            _PrintTab( file, *tabCount );
            fprintf( file, REB2SAC_XHTML_MATHML_END_SUBGROUP_FORMAT );
            fprintf( file, NEW_LINE );
        break;
        
        case KINETIC_LAW_OP_MINUS:
            _PrintTab( file, *tabCount );
            fprintf( file, REB2SAC_XHTML_MATHML_START_SUBGROUP_FORMAT );             
            fprintf( file, NEW_LINE );             
            (*tabCount)++;
            if( _NeedParenForLeft( kineticLaw, left ) ) {
                _PrintTab( file, *tabCount );
                fprintf( file, REB2SAC_XHTML_MATHML_START_SUBGROUP_FORMAT );             
                fprintf( file, NEW_LINE );             
                (*tabCount)++;
                
                _PrintTab( file, *tabCount );
                fprintf( file, REB2SAC_XHTML_MATHML_L_PAREN_FORMAT );
                fprintf( file, NEW_LINE );
                (*tabCount)++;
                
                if( IS_FAILED( ( ret = left->Accept( left, visitor ) ) ) ) {
                    END_FUNCTION("_VisitOpToPrintInXHTML", ret );
                    return ret;
                }             
                
                (*tabCount)--;
                _PrintTab( file, *tabCount );
                fprintf( file, REB2SAC_XHTML_MATHML_R_PAREN_FORMAT );
                fprintf( file, NEW_LINE );

                (*tabCount)--;
                _PrintTab( file, *tabCount );
                fprintf( file, REB2SAC_XHTML_MATHML_END_SUBGROUP_FORMAT );
                fprintf( file, NEW_LINE );
            }
            else {
                if( IS_FAILED( ( ret = left->Accept( left, visitor ) ) ) ) {
                    END_FUNCTION("_VisitOpToPrintInXHTML", ret );
                    return ret;
                }             
            }
            (*tabCount)--;
            _PrintTab( file, *tabCount );
            fprintf( file, REB2SAC_XHTML_MATHML_END_SUBGROUP_FORMAT );
            fprintf( file, NEW_LINE );

            _PrintTab( file, *tabCount );
            fprintf( file, REB2SAC_XHTML_MATHML_OP_MINUS_FORMAT );            
            fprintf( file, NEW_LINE );

            _PrintTab( file, *tabCount );
            fprintf( file, REB2SAC_XHTML_MATHML_START_SUBGROUP_FORMAT );             
            fprintf( file, NEW_LINE );             
            (*tabCount)++;
            if( _NeedParenForRight( kineticLaw, right ) ) {
                _PrintTab( file, *tabCount );
                fprintf( file, REB2SAC_XHTML_MATHML_START_SUBGROUP_FORMAT );             
                fprintf( file, NEW_LINE );             
                (*tabCount)++;
                
                _PrintTab( file, *tabCount );
                fprintf( file, REB2SAC_XHTML_MATHML_L_PAREN_FORMAT );
                fprintf( file, NEW_LINE );
                (*tabCount)++;
                
                if( IS_FAILED( ( ret = right->Accept( right, visitor ) ) ) ) {
                    END_FUNCTION("_VisitOpToPrintInXHTML", ret );
                    return ret;
                }             
                
                (*tabCount)--;
                _PrintTab( file, *tabCount );
                fprintf( file, REB2SAC_XHTML_MATHML_R_PAREN_FORMAT );
                fprintf( file, NEW_LINE );

                (*tabCount)--;
                _PrintTab( file, *tabCount );
                fprintf( file, REB2SAC_XHTML_MATHML_END_SUBGROUP_FORMAT );
                fprintf( file, NEW_LINE );
            }
            else {
                if( IS_FAILED( ( ret = right->Accept( right, visitor ) ) ) ) {
                    END_FUNCTION("_VisitOpToPrintInXHTML", ret );
                    return ret;
                }             
            }
            (*tabCount)--;
            _PrintTab( file, *tabCount );
            fprintf( file, REB2SAC_XHTML_MATHML_END_SUBGROUP_FORMAT );
            fprintf( file, NEW_LINE );
        break;
        
        case KINETIC_LAW_OP_TIMES:
            if( _NeedParenForLeft( kineticLaw, left ) ) {
                _PrintTab( file, *tabCount );
                fprintf( file, REB2SAC_XHTML_MATHML_START_SUBGROUP_FORMAT );             
                fprintf( file, NEW_LINE );             
                (*tabCount)++;
                
                _PrintTab( file, *tabCount );
                fprintf( file, REB2SAC_XHTML_MATHML_L_PAREN_FORMAT );
                fprintf( file, NEW_LINE );
                (*tabCount)++;
                
                if( IS_FAILED( ( ret = left->Accept( left, visitor ) ) ) ) {
                    END_FUNCTION("_VisitOpToPrintInXHTML", ret );
                    return ret;
                }             
                
                (*tabCount)--;
                _PrintTab( file, *tabCount );
                fprintf( file, REB2SAC_XHTML_MATHML_R_PAREN_FORMAT );
                fprintf( file, NEW_LINE );

                (*tabCount)--;
                _PrintTab( file, *tabCount );
                fprintf( file, REB2SAC_XHTML_MATHML_END_SUBGROUP_FORMAT );
                fprintf( file, NEW_LINE );
            }
            else {
                if( IS_FAILED( ( ret = left->Accept( left, visitor ) ) ) ) {
                    END_FUNCTION("_VisitOpToPrintInXHTML", ret );
                    return ret;
                }             
            }

            _PrintTab( file, *tabCount );
            fprintf( file, REB2SAC_XHTML_MATHML_OP_TIMES_FORMAT );            
            fprintf( file, NEW_LINE );

            if( _NeedParenForRight( kineticLaw, right ) ) {
                _PrintTab( file, *tabCount );
                fprintf( file, REB2SAC_XHTML_MATHML_START_SUBGROUP_FORMAT );             
                fprintf( file, NEW_LINE );             
                (*tabCount)++;
                
                _PrintTab( file, *tabCount );
                fprintf( file, REB2SAC_XHTML_MATHML_L_PAREN_FORMAT );
                fprintf( file, NEW_LINE );
                (*tabCount)++;
                
                if( IS_FAILED( ( ret = right->Accept( right, visitor ) ) ) ) {
                    END_FUNCTION("_VisitOpToPrintInXHTML", ret );
                    return ret;
                }             
                
                (*tabCount)--;
                _PrintTab( file, *tabCount );
                fprintf( file, REB2SAC_XHTML_MATHML_R_PAREN_FORMAT );
                fprintf( file, NEW_LINE );

                (*tabCount)--;
                _PrintTab( file, *tabCount );
                fprintf( file, REB2SAC_XHTML_MATHML_END_SUBGROUP_FORMAT );
                fprintf( file, NEW_LINE );
            }
            else {
                if( IS_FAILED( ( ret = right->Accept( right, visitor ) ) ) ) {
                    END_FUNCTION("_VisitOpToPrintInXHTML", ret );
                    return ret;
                }             
            }
        break;
        
        case KINETIC_LAW_OP_DIVIDE:
            _PrintTab( file, *tabCount );
            fprintf( file, REB2SAC_XHTML_MATHML_START_FRAC_FORMAT );            
            fprintf( file, NEW_LINE );
            (*tabCount)++;

            _PrintTab( file, *tabCount );
            fprintf( file, REB2SAC_XHTML_MATHML_START_SUBGROUP_FORMAT );             
            fprintf( file, NEW_LINE );             
            (*tabCount)++;
                            
            if( IS_FAILED( ( ret = left->Accept( left, visitor ) ) ) ) {
                END_FUNCTION("_VisitOpToPrintInXHTML", ret );
                return ret;
            }             
            
            (*tabCount)--;
            _PrintTab( file, *tabCount );
            fprintf( file, REB2SAC_XHTML_MATHML_END_SUBGROUP_FORMAT );
            fprintf( file, NEW_LINE );
            
            _PrintTab( file, *tabCount );
            fprintf( file, REB2SAC_XHTML_MATHML_START_SUBGROUP_FORMAT );             
            fprintf( file, NEW_LINE );             
            (*tabCount)++;
                
            if( IS_FAILED( ( ret = right->Accept( right, visitor ) ) ) ) {
                END_FUNCTION("_VisitOpToPrintInXHTML", ret );
                return ret;
            }             
            
            (*tabCount)--;
            _PrintTab( file, *tabCount );
            fprintf( file, REB2SAC_XHTML_MATHML_END_SUBGROUP_FORMAT );
            fprintf( file, NEW_LINE );
            
            (*tabCount)--;
            _PrintTab( file, *tabCount );
            fprintf( file, REB2SAC_XHTML_MATHML_END_FRAC_FORMAT );
            fprintf( file, NEW_LINE );
        break;
        
        case KINETIC_LAW_OP_POW:            
        case KINETIC_LAW_OP_ROOT:            
            _PrintTab( file, *tabCount );
            fprintf( file, REB2SAC_XHTML_MATHML_START_POWER_FORMAT );            
            fprintf( file, NEW_LINE );
            (*tabCount)++;

            _PrintTab( file, *tabCount );
            fprintf( file, REB2SAC_XHTML_MATHML_START_SUBGROUP_FORMAT );             
            fprintf( file, NEW_LINE );             
            (*tabCount)++;
            if( _NeedParenForLeft( kineticLaw, left ) ) {
                _PrintTab( file, *tabCount );
                fprintf( file, REB2SAC_XHTML_MATHML_L_PAREN_FORMAT );
                fprintf( file, NEW_LINE );
                (*tabCount)++;
                
                if( IS_FAILED( ( ret = left->Accept( left, visitor ) ) ) ) {
                    END_FUNCTION("_VisitOpToPrintInXHTML", ret );
                    return ret;
                }             
                
                (*tabCount)--;
                _PrintTab( file, *tabCount );
                fprintf( file, REB2SAC_XHTML_MATHML_R_PAREN_FORMAT );
                fprintf( file, NEW_LINE );
            }
            else {                            
                if( IS_FAILED( ( ret = left->Accept( left, visitor ) ) ) ) {
                    END_FUNCTION("_VisitOpToPrintInXHTML", ret );
                    return ret;
                }             
            }
            (*tabCount)--;
            _PrintTab( file, *tabCount );
            fprintf( file, REB2SAC_XHTML_MATHML_END_SUBGROUP_FORMAT );
            fprintf( file, NEW_LINE );
            
            _PrintTab( file, *tabCount );
            fprintf( file, REB2SAC_XHTML_MATHML_START_SUBGROUP_FORMAT );             
            fprintf( file, NEW_LINE );             
            (*tabCount)++;

	    if ( opType == KINETIC_LAW_OP_ROOT ) { 
	      fprintf( file, REB2SAC_XHTML_MATHML_ROOT_FORMAT );
	    }
                
	    if ( ( opType == KINETIC_LAW_OP_ROOT ) ||
		 ( _NeedParenForRight( kineticLaw, right ) ) ) {
                _PrintTab( file, *tabCount );
                fprintf( file, REB2SAC_XHTML_MATHML_L_PAREN_FORMAT );
                fprintf( file, NEW_LINE );
                (*tabCount)++;
                
                if( IS_FAILED( ( ret = right->Accept( right, visitor ) ) ) ) {
                    END_FUNCTION("_VisitOpToPrintInXHTML", ret );
                    return ret;
                }             
                
                (*tabCount)--;
                _PrintTab( file, *tabCount );
                fprintf( file, REB2SAC_XHTML_MATHML_R_PAREN_FORMAT );
                fprintf( file, NEW_LINE );
            }
            else {            
                if( IS_FAILED( ( ret = right->Accept( right, visitor ) ) ) ) {
                    END_FUNCTION("_VisitOpToPrintInXHTML", ret );
                    return ret;
                }             
            }
            (*tabCount)--;
            _PrintTab( file, *tabCount );
            fprintf( file, REB2SAC_XHTML_MATHML_END_SUBGROUP_FORMAT );
            fprintf( file, NEW_LINE );
            
            (*tabCount)--;
            _PrintTab( file, *tabCount );
            fprintf( file, REB2SAC_XHTML_MATHML_END_POWER_FORMAT );
            fprintf( file, NEW_LINE );
        break;
        
        default:
        return ErrorReport( FAILING, "_PrintOpKineticLawForSBML", "op type %c is not valid", opType );
    }
    
    
    END_FUNCTION("_VisitOpToPrintInXHTML", SUCCESS );
    return ret;
}

static RET_VAL _VisitIntToPrintInXHTML( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    int *tabCount = NULL;
    FILE *file = NULL;
    
    START_FUNCTION("_VisitIntToPrintInXHTML");

    file = (FILE*)(visitor->_internal1);
    tabCount = (int*)(visitor->_internal2);
    
    _PrintTab( file, *tabCount );
    fprintf( file, REB2SAC_XHTML_MATHML_INT_VALUE_FORMAT, GetIntValueFromKineticLaw( kineticLaw ) );
    fprintf( file, NEW_LINE );
        
    END_FUNCTION("_VisitIntToPrintInXHTML", SUCCESS );
    return ret;
}

static RET_VAL _VisitRealToPrintInXHTML( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    int *tabCount = NULL;
    FILE *file = NULL;
    
    START_FUNCTION("_VisitRealToPrintInXHTML");

    file = (FILE*)(visitor->_internal1);
    tabCount = (int*)(visitor->_internal2);
    
    _PrintTab( file, *tabCount );
    fprintf( file, REB2SAC_XHTML_MATHML_REAL_VALUE_FORMAT, GetRealValueFromKineticLaw(kineticLaw ) );
    fprintf( file, NEW_LINE );
    
    END_FUNCTION("_VisitRealToPrintInXHTML", SUCCESS );
    return ret;
}

static RET_VAL _VisitFunctionSymbolToPrintInXHTML( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    int *tabCount = NULL;
    char *functionSymbol = NULL;
    FILE *file = NULL;
    
    START_FUNCTION("_VisitFunctionSymbolToPrintInXHTML");

    file = (FILE*)(visitor->_internal1);
    tabCount = (int*)(visitor->_internal2);
    functionSymbol = GetFunctionSymbolFromKineticLaw( kineticLaw );
    
    _PrintTab( file, *tabCount );
    fprintf( file, REB2SAC_XHTML_MATHML_FUNCTION_SYMBOL_FORMAT, functionSymbol );
    fprintf( file, NEW_LINE );
    
    END_FUNCTION("_VisitFunctionSymbolToPrintInXHTML", SUCCESS );
    return ret;
}

static RET_VAL _VisitCompartmentToPrintInXHTML( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    int *tabCount = NULL;
    COMPARTMENT *compartment = NULL;
    FILE *file = NULL;
    
    START_FUNCTION("_VisitCompartmentToPrintInXHTML");

    file = (FILE*)(visitor->_internal1);
    tabCount = (int*)(visitor->_internal2);
    compartment = GetCompartmentFromKineticLaw( kineticLaw );
    
    _PrintTab( file, *tabCount );
    fprintf( file, REB2SAC_XHTML_MATHML_COMPARTMENT_FORMAT, GetCharArrayOfString( GetCompartmentID( compartment ) ) );
    fprintf( file, NEW_LINE );
    
    END_FUNCTION("_VisitCompartmentToPrintInXHTML", SUCCESS );
    return ret;
}

static RET_VAL _VisitSpeciesToPrintInXHTML( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    int *tabCount = NULL;
    SPECIES *species = NULL;
    FILE *file = NULL;
    
    START_FUNCTION("_VisitSpeciesToPrintInXHTML");

    file = (FILE*)(visitor->_internal1);
    tabCount = (int*)(visitor->_internal2);
    species = GetSpeciesFromKineticLaw( kineticLaw );
    
    _PrintTab( file, *tabCount );
    fprintf( file, REB2SAC_XHTML_MATHML_SPECIES_FORMAT, GetCharArrayOfString( GetSpeciesNodeName( species ) ) );
    fprintf( file, NEW_LINE );
    
    END_FUNCTION("_VisitSpeciesToPrintInXHTML", SUCCESS );
    return ret;
}

static RET_VAL _VisitSymbolToPrintInXHTML( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    int *tabCount = NULL;
    REB2SAC_SYMBOL *sym = NULL;
    FILE *file = NULL;
    
    START_FUNCTION("_VisitSymbolToPrintInXHTML");

    file = (FILE*)(visitor->_internal1);
    tabCount = (int*)(visitor->_internal2);
    sym = GetSymbolFromKineticLaw( kineticLaw );
    
    _PrintTab( file, *tabCount );
    fprintf( file, REB2SAC_XHTML_MATHML_SYMBOL_FORMAT, GetCharArrayOfString( GetSymbolID( sym ) ) );
    fprintf( file, NEW_LINE );
    
    END_FUNCTION("_VisitSymbolToPrintInXHTML", SUCCESS );
    return ret;
}



static BOOL _NeedParenForLeft( KINETIC_LAW *parent, KINETIC_LAW *child ) {
    BYTE parentOpType = 0;
    BYTE childOpType = 0;
    
    START_FUNCTION("_NeedParenForLeft");
    
    parentOpType = GetOpTypeFromKineticLaw( parent );
    if( ( parentOpType == KINETIC_LAW_OP_POW ) ||
	( parentOpType == KINETIC_LAW_OP_ROOT ) ||
	( parentOpType == KINETIC_LAW_OP_EQ ) ||
	( parentOpType == KINETIC_LAW_OP_NEQ ) ||
	( parentOpType == KINETIC_LAW_OP_GEQ ) ||
	( parentOpType == KINETIC_LAW_OP_GT ) ||
	( parentOpType == KINETIC_LAW_OP_LEQ ) ||
	( parentOpType == KINETIC_LAW_OP_LT ) ||
	( parentOpType == KINETIC_LAW_OP_MOD ) ||
	( parentOpType == KINETIC_LAW_OP_UNIFORM ) ||
	( parentOpType == KINETIC_LAW_OP_LOGNORMAL ) ||
	( parentOpType == KINETIC_LAW_OP_NORMAL ) ||
	( parentOpType == KINETIC_LAW_OP_BINOMIAL ) ||
	( parentOpType == KINETIC_LAW_OP_GAMMA ) ||
	( parentOpType == KINETIC_LAW_OP_BITWISE_AND ) ||
	( parentOpType == KINETIC_LAW_OP_BITWISE_OR ) ||
	( parentOpType == KINETIC_LAW_OP_BITWISE_XOR ) ||
	( parentOpType == KINETIC_LAW_OP_BIT ) ||
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
        childOpType = GetOpTypeFromKineticLaw( child );
        if( ( parentOpType == KINETIC_LAW_OP_PLUS ) || ( parentOpType == KINETIC_LAW_OP_MINUS ) || ( parentOpType == KINETIC_LAW_OP_DIVIDE ) ) {
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
    
    parentOpType = GetOpTypeFromKineticLaw( parent );
    if( ( parentOpType == KINETIC_LAW_OP_POW ) ||
	( parentOpType == KINETIC_LAW_OP_ROOT ) ||
	( parentOpType == KINETIC_LAW_OP_EQ ) ||
	( parentOpType == KINETIC_LAW_OP_NEQ ) ||
	( parentOpType == KINETIC_LAW_OP_GEQ ) ||
	( parentOpType == KINETIC_LAW_OP_GT ) ||
	( parentOpType == KINETIC_LAW_OP_LEQ ) ||
	( parentOpType == KINETIC_LAW_OP_LT ) ||
	( parentOpType == KINETIC_LAW_OP_MOD ) ||
	( parentOpType == KINETIC_LAW_OP_UNIFORM ) ||
	( parentOpType == KINETIC_LAW_OP_LOGNORMAL ) ||
	( parentOpType == KINETIC_LAW_OP_GAMMA ) ||
	( parentOpType == KINETIC_LAW_OP_NORMAL ) ||
	( parentOpType == KINETIC_LAW_OP_BINOMIAL ) ||
	( parentOpType == KINETIC_LAW_OP_BITWISE_AND ) ||
	( parentOpType == KINETIC_LAW_OP_BITWISE_OR ) ||
	( parentOpType == KINETIC_LAW_OP_BITWISE_XOR ) ||
	( parentOpType == KINETIC_LAW_OP_BIT ) ||
	( parentOpType == KINETIC_LAW_OP_AND ) ||
	( parentOpType == KINETIC_LAW_OP_XOR ) ||
	( parentOpType == KINETIC_LAW_OP_OR ) ) {
        END_FUNCTION("_NeedParenForRight", SUCCESS );        
        return FALSE;
    } 
    if( IsOpKineticLaw( child ) ) {        
        childOpType = GetOpTypeFromKineticLaw( child );
        if( ( childOpType == KINETIC_LAW_OP_POW ) ||
	    ( childOpType == KINETIC_LAW_OP_ROOT ) ||
	    ( childOpType == KINETIC_LAW_OP_EQ ) ||
	    ( childOpType == KINETIC_LAW_OP_NEQ ) ||
	    ( childOpType == KINETIC_LAW_OP_GEQ ) ||
	    ( childOpType == KINETIC_LAW_OP_GT ) ||
	    ( childOpType == KINETIC_LAW_OP_LEQ ) ||
	    ( childOpType == KINETIC_LAW_OP_LT ) ||
	    ( childOpType == KINETIC_LAW_OP_MOD ) ||
	    ( childOpType == KINETIC_LAW_OP_UNIFORM ) ||
	    ( childOpType == KINETIC_LAW_OP_LOGNORMAL ) ||
	    ( childOpType == KINETIC_LAW_OP_GAMMA ) ||
	    ( childOpType == KINETIC_LAW_OP_NORMAL ) ||
	    ( childOpType == KINETIC_LAW_OP_BINOMIAL ) ||
	    ( childOpType == KINETIC_LAW_OP_BITWISE_AND ) ||
	    ( childOpType == KINETIC_LAW_OP_BITWISE_OR ) ||
	    ( childOpType == KINETIC_LAW_OP_BITWISE_XOR ) ||
	    ( childOpType == KINETIC_LAW_OP_BIT ) ||
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
                END_FUNCTION("_NeedParenForRight", SUCCESS );        
            return FALSE;
            
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
















