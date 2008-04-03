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
static RET_VAL _PrintCompartmentListInXHTML( LINKED_LIST *list, FILE *file ); 
static RET_VAL _PrintSpeciesListInXHTML( LINKED_LIST *list, FILE *file ); 
static RET_VAL _PrintKineticLawInXHTML( KINETIC_LAW *kineticLaw, FILE *file ); 

static void _PrintTab( FILE *file, int tabCount );

static RET_VAL _VisitOpToPrintInXHTML( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitIntToPrintInXHTML( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitRealToPrintInXHTML( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
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
    
    DeleteLinkedList( &list );
    END_FUNCTION("_PrintConstantsInXHTML", SUCCESS );        
    return ret;
}

static RET_VAL _PrintConstantInXHTML( REB2SAC_SYMBOL *sym, FILE *file ) {
    RET_VAL ret = SUCCESS;
    char *id = NULL;
    double value = 0.0;

    START_FUNCTION("_PrintConstantInXHTML");
    
    if( !IsSymbolConstant( sym ) ) {
        END_FUNCTION("_PrintConstantInXHTML", SUCCESS );        
        return ret;
    }
    if( !IsRealValueSymbol( sym ) ) {
        END_FUNCTION("_PrintConstantInXHTML", SUCCESS );        
        return ret;
    }
    
    id = GetCharArrayOfString( GetSymbolID( sym ) );
    value = GetRealValueInSymbol( sym );
    
    fprintf( file, REB2SAC_XHTML_CONSTANT_ENTRY_FORMAT, id, value );
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

    ResetCurrentElement( list );
    while( ( reaction = (REACTION*)GetNextFromLinkedList( list ) )  != NULL ) {
        if( IS_FAILED( ( ret = _PrintReactionInXHTML( reaction, file ) ) ) ) {
            END_FUNCTION("_PrintListOfReactionsInXHTML", ret );
            return ret;
        }
        fprintf( file, REB2SAC_XHTML_LINE_BREAK );
    }
    
    END_FUNCTION("_PrintListOfReactionsInXHTML", SUCCESS );
    return ret;
}

static RET_VAL _PrintReactionInXHTML( REACTION *reaction, FILE *file ) {
    RET_VAL ret = SUCCESS;
    KINETIC_LAW *kineticLaw = NULL;
    LINKED_LIST *list = NULL;
    
    START_FUNCTION("_PrintReactionInXHTML");

    fprintf( file, REB2SAC_XHTML_START_REACTION_FORMAT, GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
    
    list = GetReactantsInReactionNode( reaction );
    if( IS_FAILED( ( ret = _PrintListOfReactantsInXHTML( list, file ) ) ) ) {
        END_FUNCTION("_PrintReactionInXHTML", ret );
        return ret;
    }    
    
    list = GetProductsInReactionNode( reaction );
    if( IS_FAILED( ( ret = _PrintListOfProductsInXHTML( list, file ) ) ) ) {
        END_FUNCTION("_PrintReactionInXHTML", ret );
        return ret;
    }    

    list = GetModifiersInReactionNode( reaction );
    if( IS_FAILED( ( ret = _PrintListOfModifiersInXHTML( list, file ) ) ) ) {
        END_FUNCTION("_PrintReactionInXHTML", ret );
        return ret;
    }    
        
    kineticLaw = GetKineticLawInReactionNode( reaction );
    if( IS_FAILED( ( ret = _PrintKineticLawInXHTML( kineticLaw, file ) ) ) ) {
        END_FUNCTION("_PrintReactionInXHTML", ret );
        return ret;
    }    
    fprintf( file, REB2SAC_XHTML_END_REACTION_FORMAT );
    fprintf( file, NEW_LINE );
    
    END_FUNCTION("_PrintReactionInXHTML", SUCCESS );
    return ret;
}


static RET_VAL _PrintListOfReactantsInXHTML( LINKED_LIST *list, FILE *file ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("_PrintListOfReactantsInXHTML");
    
    fprintf( file, REB2SAC_XHTML_START_REACTANTS_FORMAT );
    
    if( IS_FAILED( ( ret = _PrintListOfSpeciesInXHTML( list, file ) ) ) ) {
        END_FUNCTION("_PrintListOfReactantsInXHTML", ret );
        return ret;
    }    
    
    fprintf( file, REB2SAC_XHTML_END_REACTANTS_FORMAT );
    fprintf( file, NEW_LINE );
    
    END_FUNCTION("_PrintListOfReactantsInXHTML", SUCCESS );
    return ret;
}

static RET_VAL _PrintListOfProductsInXHTML( LINKED_LIST *list, FILE *file ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("_PrintListOfProductsInXHTML");
    
    fprintf( file, REB2SAC_XHTML_START_PRODUCTS_FORMAT );
    
    if( IS_FAILED( ( ret = _PrintListOfSpeciesInXHTML( list, file ) ) ) ) {
        END_FUNCTION("_PrintListOfReactantsInXHTML", ret );
        return ret;
    }    
    
    fprintf( file, REB2SAC_XHTML_END_PRODUCTS_FORMAT );
    fprintf( file, NEW_LINE );
    
    END_FUNCTION("_PrintListOfProductsInXHTML", SUCCESS );
    return ret;
}

static RET_VAL _PrintListOfModifiersInXHTML( LINKED_LIST *list, FILE *file ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("_PrintListOfModifiersInXHTML");
    
    fprintf( file, REB2SAC_XHTML_START_MODIFIERS_FORMAT );
    
    if( IS_FAILED( ( ret = _PrintListOfSpeciesInXHTML( list, file ) ) ) ) {
        END_FUNCTION("_PrintListOfReactantsInXHTML", ret );
        return ret;
    }    
    
    fprintf( file, REB2SAC_XHTML_END_MODIFIERS_FORMAT );
    fprintf( file, NEW_LINE );
    
    END_FUNCTION("_PrintListOfModifiersInXHTML", SUCCESS );
    return ret;
}

static RET_VAL _PrintListOfSpeciesInXHTML( LINKED_LIST *list, FILE *file ) {
    RET_VAL ret = SUCCESS;
    int stoichiometry = 0;
    SPECIES *species = NULL;
    IR_EDGE *edge = NULL;
    
    START_FUNCTION("_PrintListOfSpeciesInXHTML");
    
    ResetCurrentElement( list );
    if( ( edge = (IR_EDGE*)GetNextFromLinkedList( list ) ) == NULL ) {
        END_FUNCTION("_PrintListOfSpeciesInXHTML", SUCCESS );
        return ret;
    } 
    
    species = GetSpeciesInIREdge( edge );
    stoichiometry = GetStoichiometryInIREdge( edge );
    if( stoichiometry == 1 ) {    
        fprintf( file, REB2SAC_XHTML_SPECIES_FORMAT, GetCharArrayOfString( GetSpeciesNodeName( species ) ) );
    }
    else {
        fprintf( file, REB2SAC_XHTML_SPECIES_FORMAT2, 
            stoichiometry, GetCharArrayOfString( GetSpeciesNodeName( species ) ) );
    }
    while( ( edge = (IR_EDGE*)GetNextFromLinkedList( list ) ) != NULL ) {
        species = GetSpeciesInIREdge( edge );
        stoichiometry = GetStoichiometryInIREdge( edge );
        if( stoichiometry == 1 ) {
            fprintf( file, REB2SAC_XHTML_MORE_SPECIES_FORMAT, GetCharArrayOfString( GetSpeciesNodeName( species ) ) );
        }
        else {
            fprintf( file, REB2SAC_XHTML_MORE_SPECIES_FORMAT2, 
                stoichiometry, GetCharArrayOfString( GetSpeciesNodeName( species ) ) );
        }
    }
    
    END_FUNCTION("_PrintListOfSpeciesInXHTML", SUCCESS );
    return ret;
}

static RET_VAL _PrintCompartmentForXHTML( COMPARTMENT *compartment, FILE *file ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("_PrintCompartmentForXHTML");

    fprintf( file, REB2SAC_XHTML_COMPARTMENT_ENTRY_FORMAT,
	     GetCharArrayOfString( GetCompartmentID( compartment ) ),
	     GetSizeInCompartment( compartment ) );

    fprintf( file, NEW_LINE );
            
    END_FUNCTION("_PrintCompartmentForXHTML", SUCCESS );
    
    return ret;
}

static RET_VAL _PrintCompartmentListInXHTML( LINKED_LIST *list, FILE *file ) {
    RET_VAL ret = SUCCESS;
    COMPARTMENT *compartment = NULL;

    fprintf( file, REB2SAC_XHTML_START_COMPARTMENT_FORMAT );
    
    START_FUNCTION("_PrintCompartmentListInXHTML");
    if( ( list == NULL ) || ( GetLinkedListSize( list ) == 0 ) ) {
        END_FUNCTION("_PrintListOfCompartmentsForSBML", SUCCESS );    
        return ret;
    }    
    ResetCurrentElement( list );
    while( ( compartment = (COMPARTMENT*)GetNextFromLinkedList( list ) ) != NULL ) {
        if( IS_FAILED( ( ret = _PrintCompartmentForXHTML( compartment, file ) ) ) ) {
            END_FUNCTION("_PrintCompartmentListInXHTML", ret );    
            return ret;
        }
    }
    
    fprintf( file, REB2SAC_XHTML_END_COMPARTMENT_FORMAT );
    fprintf( file, NEW_LINE );
    
    END_FUNCTION("_PrintCompartmentListInXHTML", SUCCESS );
    return ret;
}

static RET_VAL _PrintSpeciesForXHTML( SPECIES *species, FILE *file ) {
    RET_VAL ret = SUCCESS;
    double initialQuantity = 0.0;
    COMPARTMENT *compartment = NULL;
    
    START_FUNCTION("_PrintSpeciesForXHTML");
        
    compartment = GetCompartmentInSpeciesNode( species );
    
     if( IsInitialQuantityInAmountInSpeciesNode( species ) ) {
         initialQuantity = GetInitialAmountInSpeciesNode( species );
     } else {
         initialQuantity = GetInitialConcentrationInSpeciesNode( species );
     }

    fprintf( file, REB2SAC_XHTML_SPECIES_ENTRY_FORMAT,
	     GetCharArrayOfString( GetSpeciesNodeID( species ) ),
	     GetCharArrayOfString( GetCompartmentID( compartment ) ),
	     initialQuantity );

    fprintf( file, NEW_LINE );
            
    END_FUNCTION("_PrintSpeciesForXHTML", SUCCESS );
    
    return ret;
}

static RET_VAL _PrintSpeciesListInXHTML( LINKED_LIST *list, FILE *file ) {
    RET_VAL ret = SUCCESS;
    SPECIES *species = NULL;

    fprintf( file, REB2SAC_XHTML_START_SPECIES_FORMAT );
    
    START_FUNCTION("_PrintSpeciesListInXHTML");
    if( ( list == NULL ) || ( GetLinkedListSize( list ) == 0 ) ) {
        END_FUNCTION("_PrintListOfSpeciesForSBML", SUCCESS );    
        return ret;
    }    
    ResetCurrentElement( list );
    while( ( species = (SPECIES*)GetNextFromLinkedList( list ) ) != NULL ) {
        if( IS_FAILED( ( ret = _PrintSpeciesForXHTML( species, file ) ) ) ) {
            END_FUNCTION("_PrintSpeciesListInXHTML", ret );    
            return ret;
        }
    }
    
    fprintf( file, REB2SAC_XHTML_END_SPECIES_FORMAT );
    fprintf( file, NEW_LINE );
    
    END_FUNCTION("_PrintSpeciesListInXHTML", SUCCESS );
    return ret;
}

static RET_VAL _PrintKineticLawInXHTML( KINETIC_LAW *kineticLaw, FILE *file ) {
    static KINETIC_LAW_VISITOR visitor;
    RET_VAL ret = SUCCESS;
    int tabCount = 1;
    
    START_FUNCTION("_PrintKineticLawInXHTML");

    fprintf( file, REB2SAC_XHTML_START_KINETIC_LAW_FORMAT );
    fprintf( file, NEW_LINE );
    
    if( visitor.VisitOp == NULL ) {
        visitor.VisitOp = _VisitOpToPrintInXHTML;
        visitor.VisitInt = _VisitIntToPrintInXHTML;
        visitor.VisitReal = _VisitRealToPrintInXHTML;
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
    
    fprintf( file, REB2SAC_XHTML_END_KINETIC_LAW_FORMAT );
    fprintf( file, NEW_LINE );
    
    END_FUNCTION("_PrintKineticLawInXHTML", SUCCESS );
    return ret;
}

static void _PrintTab( FILE *file, int tabCount ) {
    int i = 0;
    
    for( i = 0; i < tabCount; i++ ) {
        fprintf( file, __IR2XHTML_TAB );
    }
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
                
            if( _NeedParenForRight( kineticLaw, right ) ) {
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
    if( parentOpType == KINETIC_LAW_OP_POW ) {
        END_FUNCTION("_NeedParenForRight", SUCCESS );        
        return FALSE;
    } 
    if( IsOpKineticLaw( child ) ) {        
        childOpType = GetOpTypeFromKineticLaw( child );
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
















