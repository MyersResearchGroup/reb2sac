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
#include "abstraction_method_manager.h"
#include "law_of_mass_action_util.h"


typedef struct {
    SPECIES *substrate;
    SPECIES *boundOp;    
    KINETIC_LAW *rateRatioKineticLaw;
} RNAP_OPERATOR_BINDING_METHOD_INTERNAL_ELEMENT;

typedef struct {
    SPECIES *rnap;
    double rnapConTotal;    
} RNAP_OPERATOR_BINDING_METHOD_INTERNAL_RNAP_INFO;


typedef struct {
    SPECIES *operator;
    SPECIES *rnap;
    SPECIES *complex; 
    SPECIES *product; 
    double opConTotal;
    double rnapConTotal;    
    KINETIC_LAW *k1RatioKineticLaw;
    KINETIC_LAW *k2KineticLaw;
    REACTION *complexFormationReaction;
    REACTION *productionReaction;           
    RNAP_OPERATOR_BINDING_METHOD_INTERNAL_RNAP_INFO *rnapInfo;
    LINKED_LIST *elements;
} RNAP_OPERATOR_BINDING_METHOD_INTERNAL;



static char * _GetRnapOperatorBindingMethodID( ABSTRACTION_METHOD *method );
static RET_VAL _ApplyRnapOperatorBindingMethod( ABSTRACTION_METHOD *method, IR *ir );      
static BOOL _IsConditionSatisfied( ABSTRACTION_METHOD *method, SPECIES *species, RNAP_OPERATOR_BINDING_METHOD_INTERNAL *internal ); 
static RET_VAL _DoTransformation( ABSTRACTION_METHOD *method, IR *ir, RNAP_OPERATOR_BINDING_METHOD_INTERNAL *internal );
static double _GetSpeciesConcentration( SPECIES *species );
static RET_VAL _SubstituteSpeciesWithKineticLaw( REACTION *reaction, SPECIES *target, KINETIC_LAW *to );
static RET_VAL _CombineReactions( IR *ir, REACTION *productionReaction, SPECIES *op, SPECIES *boundOp );
static double _FindConstantSpeciesConcentrationTotal( SPECIES *species );

static BOOL _IsRnap( ABSTRACTION_METHOD *method, SPECIES *species, RNAP_OPERATOR_BINDING_METHOD_INTERNAL *internal ); 

static BOOL _IsOperator( ABSTRACTION_METHOD *method, SPECIES *species, RNAP_OPERATOR_BINDING_METHOD_INTERNAL *internal ); 


static RET_VAL _InitRnapOperatorBindingInternal( RNAP_OPERATOR_BINDING_METHOD_INTERNAL *internal );
static RET_VAL _CleanRnapOperatorBindingInternal( RNAP_OPERATOR_BINDING_METHOD_INTERNAL *internal );
static RET_VAL _FreeRnapOperatorBindingInternal( RNAP_OPERATOR_BINDING_METHOD_INTERNAL *internal );

static double _GetSpeciesConcentration( SPECIES *species );


ABSTRACTION_METHOD *RnapOperatorBindingAbstractionMethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager ) {
    static ABSTRACTION_METHOD method;
    
    START_FUNCTION("RnapOperatorBindingAbstractionMethodConstructor");

    if( method.manager == NULL ) {
        method.manager = manager;
        method.GetID = _GetRnapOperatorBindingMethodID;
        method.Apply = _ApplyRnapOperatorBindingMethod;
    }
    
    TRACE_0( "RnapOperatorBindingAbstractionMethodConstructor invoked" );
    
    END_FUNCTION("RnapOperatorBindingAbstractionMethodConstructor", SUCCESS );
    return &method;
}



static char * _GetRnapOperatorBindingMethodID( ABSTRACTION_METHOD *method ) {
    START_FUNCTION("_GetRnapOperatorBindingMethodID");
    
    END_FUNCTION("_GetRnapOperatorBindingMethodID", SUCCESS );
    return "rnap-operator-binding-remover";
}



static RET_VAL _ApplyRnapOperatorBindingMethod( ABSTRACTION_METHOD *method, IR *ir ) {
    RET_VAL ret = SUCCESS;
    BOOL rnapFound = FALSE;
    SPECIES *species = NULL;
    LINKED_LIST *speciesList = NULL;
    RNAP_OPERATOR_BINDING_METHOD_INTERNAL internal;
    RNAP_OPERATOR_BINDING_METHOD_INTERNAL_RNAP_INFO *rnapInfo = NULL;
    HASH_TABLE *table = NULL;
    LINKED_LIST *list = NULL;
    
    START_FUNCTION("_ApplyRnapOperatorBindingMethod");
    
    
    if( ( table = CreateHashTable( 32 ) ) == NULL ) {
        return ErrorReport( FAILING, "_ApplyRnapOperatorBindingMethod", "could not create a hash table" );
    }

    method->_internal1 = (CADDR_T)table;
    
    speciesList = ir->GetListOfSpeciesNodes( ir );
    ResetCurrentElement( speciesList );    
    while( ( species = (SPECIES*)GetNextFromLinkedList( speciesList ) ) != NULL ) {
        if( _IsRnap( method, species, &internal ) ) {
            rnapFound = TRUE;
        }            
    }
    
    if( !rnapFound ) {
        END_FUNCTION("_ApplyRnapOperatorBindingMethod", SUCCESS );
        return ret;
    }
    
    ResetCurrentElement( speciesList );    
    while( ( species = (SPECIES*)GetNextFromLinkedList( speciesList ) ) != NULL ) {
        if( _IsConditionSatisfied( method, species, &internal ) ) {
            if( IS_FAILED( ( ret = _DoTransformation( method, ir, &internal ) ) ) ) {
                END_FUNCTION("_ApplyRnapOperatorBindingMethod", ret );
                return ret;
            }
        }            
    }
            
    if( ( list = GenerateValueList( table ) ) == NULL ) {
        return ErrorReport( FAILING, "_ApplyRnapOperatorBindingMethod", "could not generate a value list from a hash table" );
    }  
    ResetCurrentElement( list );
    while( ( rnapInfo = (RNAP_OPERATOR_BINDING_METHOD_INTERNAL_RNAP_INFO*)GetNextFromLinkedList( list ) ) != NULL ) {
        FREE( rnapInfo );        
    }
    DeleteLinkedList( &list );
    DeleteHashTable( &table );    
    
    END_FUNCTION("_ApplyRnapOperatorBindingMethod", SUCCESS );
    return ret;
}      

static BOOL _IsConditionSatisfied( ABSTRACTION_METHOD *method, SPECIES *species, RNAP_OPERATOR_BINDING_METHOD_INTERNAL *internal ) {    
    SPECIES *reactant = NULL;
    SPECIES *substrate = NULL;
    SPECIES *boundOp = NULL;    
    SPECIES *rnap = NULL;
    KINETIC_LAW *rateRatioKineticLaw = NULL;
    
    SPECIES *product = NULL;
    REACTION *reaction = NULL;
    KINETIC_LAW *kineticLaw = NULL;
    LINKED_LIST *reactants = NULL; 
    LINKED_LIST *products = NULL; 
    LINKED_LIST *reactionsAsReactant = NULL; 
    LINKED_LIST *reactions = NULL;    
    RNAP_OPERATOR_BINDING_METHOD_INTERNAL_RNAP_INFO *rnapInfo = NULL;
    HASH_TABLE *table = NULL;
    
    START_FUNCTION("_IsConditionSatisfied");
    
    /* not fully implemented yet*/
    END_FUNCTION("_IsConditionSatisfied", SUCCESS );
    return FALSE;
    
    if( !_IsOperator( method, species, internal ) ) {
        END_FUNCTION("_IsConditionSatisfied", SUCCESS );
        return FALSE;
    }
    
    table = (HASH_TABLE*)(method->_internal1);
    internal->rnap = NULL;
    
    reactions = GetReactionsAsReactantInSpeciesNode( species );
    ResetCurrentElement( reactions );
    while( ( reaction = (REACTION*)GetNextFromLinkedList( reactions ) ) != NULL ) {
        reactants = GetReactantsInReactionNode( reaction );
        ResetCurrentElement( reactants );
        while( ( reactant = (SPECIES*)GetNextFromLinkedList( reactants ) ) != NULL ) {
            if( reactant == species ) {
                continue;
            }
            if( ( rnapInfo = (RNAP_OPERATOR_BINDING_METHOD_INTERNAL_RNAP_INFO*)GetValueFromHashTable( reactant, sizeof(SPECIES), table ) ) == NULL ) {
                continue;
            }
            if( internal->rnap != NULL ) {
                TRACE_1( "species %s binds to more than 1 RNAP", GetCharArrayOfString( GetSpeciesNodeName( species ) ) );
                END_FUNCTION("_IsConditionSatisfied", SUCCESS );
                return FALSE;
            }
            internal->rnap = rnapInfo->rnap;
            internal->rnapConTotal = rnapInfo->rnapConTotal;
        }
        products = GetProductsInReactionNode( reaction );
        product = (SPECIES*)GetHeadFromLinkedList( products );
        internal->complex = product;
        internal->complexFormationReaction = reaction;
 /* from here */       
    }
    
    TRACE_1("species %s satisfies the condition", GetCharArrayOfString( GetSpeciesNodeName( species ) ) );
    END_FUNCTION("_IsConditionSatisfied", SUCCESS );
    return TRUE;
}

static BOOL _IsRnap( ABSTRACTION_METHOD *method, SPECIES *species, RNAP_OPERATOR_BINDING_METHOD_INTERNAL *internal ) {
    char *valueString = NULL;
    double threshold = 0.0;
    double rnapConTotal = 0.0;
    SPECIES *rnap = NULL;
    SPECIES *product = NULL;
    REACTION *reaction = NULL;
    LINKED_LIST *reactions = NULL;    
    SPECIES *products = NULL;
    RNAP_OPERATOR_BINDING_METHOD_INTERNAL_RNAP_INFO *rnapInfo = NULL;
    HASH_TABLE *table = NULL;
    ABSTRACTION_METHOD_MANAGER *manager = NULL;
    COMPILER_RECORD_T *record = NULL;
    REB2SAC_PROPERTIES *properties = NULL;

    START_FUNCTION("_IsRnap");
    
    table = (HASH_TABLE*)(method->_internal1);
    
    reactions = GetReactionsAsProductInSpeciesNode( species ); 
    if( GetLinkedListSize( reactions ) != 0 )  {
        END_FUNCTION("_IsRnap", SUCCESS );
        return FALSE;
    }    
    
    reactions = GetReactionsAsReactantInSpeciesNode( species );
    ResetCurrentElement( reactions );
    while( ( reaction = (REACTION*)GetNextFromLinkedList( reactions ) ) != NULL ) {
        if( !IsReactionReversibleInReactionNode( reaction ) ) {
            END_FUNCTION("_IsRnap", SUCCESS );
            return FALSE;
        }
        products = GetProductsInReactionNode( reaction );
        if( GetLinkedListSize( products ) != 1 ) {
            END_FUNCTION("_IsRnap", SUCCESS );
            return FALSE;
        }
        product = (SPECIES*)GetHeadFromLinkedList( products );        
        rnapConTotal += _GetSpeciesConcentration( product );
    }
    
    rnapConTotal += _GetSpeciesConcentration( species );

    manager = method->manager;
    record = manager->GetCompilerRecord( manager );
    properties = record->properties;
    if( ( valueString = properties->GetProperty( properties, REB2SAC_RNAP_MIN_CONCENTRATION_THRESHOLD_KEY ) ) == NULL ) {
        threshold = DEFAULT_REB2SAC_RNAP_MIN_CONCENTRATION_THRESHOLD;
    }
    else {
        if( IS_FAILED( StrToFloat( &threshold, valueString ) ) ) {
            threshold = DEFAULT_REB2SAC_RNAP_MIN_CONCENTRATION_THRESHOLD;
        }
    }
    if( rnapConTotal < threshold ) {
        END_FUNCTION("_IsRnap", SUCCESS );
        return FALSE;
    }
    
    if( ( rnapInfo = (RNAP_OPERATOR_BINDING_METHOD_INTERNAL_RNAP_INFO*)MALLOC( sizeof(RNAP_OPERATOR_BINDING_METHOD_INTERNAL_RNAP_INFO) ) ) == NULL ) {
        END_FUNCTION("_IsRnap", FAILING );
        return FALSE;
    }
    
    rnapInfo->rnap = species;
    rnapInfo->rnapConTotal = rnapConTotal;
    
    if( IS_FAILED( PutInHashTable( rnapInfo->rnap, sizeof(SPECIES), rnapInfo, table ) ) ) {
        END_FUNCTION("_IsRnap", FAILING );
        return FALSE;
    }     

    TRACE_1("species %s is RNAP", GetCharArrayOfString( GetSpeciesNodeName( species ) ) );
            
    END_FUNCTION("_IsRnap", SUCCESS );
    return TRUE;
}


static BOOL _IsOperator( ABSTRACTION_METHOD *method, SPECIES *species, RNAP_OPERATOR_BINDING_METHOD_INTERNAL *internal ) {
    SPECIES *reactant = NULL;
    SPECIES *substrate = NULL;
    SPECIES *boundOp = NULL;    
    KINETIC_LAW *rateRatioKineticLaw = NULL;
    
    SPECIES *product = NULL;
    REACTION *reaction = NULL;
    KINETIC_LAW *kineticLaw = NULL;
    LINKED_LIST *reactants = NULL; 
    LINKED_LIST *products = NULL; 
    LINKED_LIST *reactionsAsReactant = NULL; 
    LINKED_LIST *reactions = NULL;    
    RNAP_OPERATOR_BINDING_METHOD_INTERNAL_ELEMENT *element = NULL;
    
    START_FUNCTION("_IsOperator");
    
    
    /*
    * this species S is not one of properties of interest   
    */
    if( IsKeepFlagSetInSpeciesNode( species ) ) {
        END_FUNCTION("_IsOperator", SUCCESS );
        return FALSE;
    }
    
    reactions = GetReactionsAsProductInSpeciesNode( species ); 
    if( GetLinkedListSize( reactions ) != 0 ) {
        END_FUNCTION("_IsOperator", SUCCESS );
        return FALSE;
    }
    /*
        * this species S is not used as a product in any reactions   
        */
    reactionsAsReactant = GetReactionsAsReactantInSpeciesNode( species );
    ResetCurrentElement( reactionsAsReactant );
    while( ( reaction = (REACTION*)GetNextFromLinkedList( reactionsAsReactant ) ) != NULL ) {
        /*
            * every reaction R, where R uses S as a reactant, R is a reversible reaction
            */
        if( !IsReactionReversibleInReactionNode( reaction ) ) {
            END_FUNCTION("_IsOperator", SUCCESS );
            return FALSE;
        }            
        
        /*
            * every reaction R, where R uses S as a reactant, kinetic law of R K has a pattern of <kinetic-law> - <kinetic-law> 
            */
        kineticLaw = GetKineticLawInReactionNode( reaction );
        if( !IsOpKineticLaw( kineticLaw ) ) {
            END_FUNCTION("_IsOperator", SUCCESS );
            return FALSE;
        }            
        if( GetOpTypeFromKineticLaw( kineticLaw ) != KINETIC_LAW_OP_MINUS ) {
            END_FUNCTION("_IsOperator", SUCCESS );
            return FALSE;
        }
        if( ( rateRatioKineticLaw = CreateRateConstantRatioKineticLaw( kineticLaw ) ) == NULL ) {
            END_FUNCTION("_IsOperator", FAILING );
            return FALSE;
        }
        
        
        /*
            *every reaction R, where R uses S as a reactant, R has 2 reactants
            */
        reactants = GetReactantsInReactionNode( reaction );
        if( GetLinkedListSize( reactants ) != 2 ) {
            END_FUNCTION("_IsOperator", SUCCESS );
            return FALSE;
        } 
        ResetCurrentElement( reactants );
        while( ( reactant = (SPECIES*)GetNextFromLinkedList( reactants ) ) != NULL ) {
            if( reactant == species ) {
                continue;
            }
            else {
                substrate = reactant;
            }
        }
        
        /*
            * every reaction R, where R uses S as a reactant, R only has 1 product
            */
        products = GetProductsInReactionNode( reaction );
        if( GetLinkedListSize( products ) != 1 ) {
            END_FUNCTION("_IsOperator", SUCCESS );
            return FALSE;
        }
        /*
            * species P, where P is the product of R
            */
        product = (SPECIES*)GetHeadFromLinkedList( products );
        
        /*
            * species P, where P is the product of R, is not property of interest
            */
        if( IsKeepFlagSetInSpeciesNode( product ) ) {
            END_FUNCTION("_IsOperator", SUCCESS );
            return FALSE;
        }
        
        /*
            *  P is produced only by R
            */
        reactions = GetReactionsAsProductInSpeciesNode( product );
        if( GetLinkedListSize( reactions ) != 1 ) {
            END_FUNCTION("_IsOperator", SUCCESS );
            return FALSE;
        }
        /*
            *  P is not used as a reactant in any reactions
            */
        reactions = GetReactionsAsReactantInSpeciesNode( product );
        if( GetLinkedListSize( reactions ) != 0 ) {
            END_FUNCTION("_IsOperator", SUCCESS );
            return FALSE;
        }
        
        if( ( element = (RNAP_OPERATOR_BINDING_METHOD_INTERNAL_ELEMENT*)MALLOC( sizeof(RNAP_OPERATOR_BINDING_METHOD_INTERNAL_ELEMENT) ) ) == NULL ) {
            END_FUNCTION("_IsOperator", FAILING );
            return FALSE;
        }
        
        if( IS_FAILED( AddElementInLinkedList( element, internal->elements ) ) ) {
            END_FUNCTION("_IsOperator", FAILING );
            return FALSE;
        } 
    }            
    TRACE_1("species %s satisfies the condition", GetCharArrayOfString( GetSpeciesNodeName( species ) ) );
    END_FUNCTION("_IsOperator", SUCCESS );
    return TRUE;
}




 
static RET_VAL _DoTransformation( ABSTRACTION_METHOD *method, IR *ir, RNAP_OPERATOR_BINDING_METHOD_INTERNAL *internal ) {
    RET_VAL ret = SUCCESS;
    double totalConcentration = 0.0;
    SPECIES *boundOp = NULL;
    SPECIES *op = NULL;
    REACTION *reaction = NULL;
    REACTION *productionReaction = NULL;
    LINKED_LIST *reactions = NULL;
    LINKED_LIST *reactionsAsModifier = NULL;
    LINKED_LIST *speciesList = NULL;
    LINKED_LIST *laws = NULL;
    KINETIC_LAW *law = NULL;
    KINETIC_LAW *totalConKineticLaw = NULL;
    KINETIC_LAW *numer = NULL;
    KINETIC_LAW *denom =  NULL;
    HASH_TABLE *massActionTable = NULL;
    REB2SAC_SYMTAB *symtab = NULL;    
#ifdef DEBUG
    STRING *speciesString = NULL;
    STRING *lawString = NULL;
#endif    
    
    START_FUNCTION("_DoTransformation");
    
    op = internal->operator;
    
    reactions = GetReactionsAsReactantInSpeciesNode( op );    
    totalConcentration += _GetSpeciesConcentration( op );
    if( ( massActionTable = CreateHashTable( 32 ) ) == NULL ) {
        return ErrorReport( FAILING, "_DoTransformation", "could not create a table for mass action ratio for %s", GetCharArrayOfString( GetSpeciesNodeName( op ) ) );
    } 
    if( ( denom = CreateRealValueKineticLaw( 1.0 ) ) == NULL ) {
        return ErrorReport( FAILING, "_DoTransformation", "could not create a kinetic law value 1.0" );
    }
    
    ResetCurrentElement( reactions );    
    while( ( reaction = (REACTION*)GetNextFromLinkedList( reactions ) ) != NULL ) {
        speciesList = GetProductsInReactionNode( reaction );
        boundOp = (SPECIES*)GetHeadFromLinkedList( speciesList );
        totalConcentration += _GetSpeciesConcentration( boundOp );
        if( ( law = CreateMassActionRatioKineticLawWithRateConstantInKineticLaw( reaction, op ) ) == NULL ) {
            return ErrorReport( FAILING, "_DoTransformation", "could not create mass action ratio for %s", GetCharArrayOfString( GetSpeciesNodeName( op ) ) );
        }
        
        if( IS_FAILED( ( ret = PutInHashTable( boundOp, sizeof( SPECIES ), law, massActionTable ) ) ) ) {
            END_FUNCTION("_DoTransformation", ret );
            return ret;
        }
        
        if( ( denom = CreateOpKineticLaw( KINETIC_LAW_OP_PLUS, denom, CloneKineticLaw( law ) ) ) == NULL ) {
            return ErrorReport( FAILING, "_DoTransformation", "could not create denominator kinetic law" );
        }
    }
    
    TRACE_1( "total concentration is %g", totalConcentration );
    symtab = ir->GetGlobalSymtab( ir );
    if( ( totalConKineticLaw = CreateTotalConcentrationKineticLaw( op, symtab, totalConcentration ) ) == NULL ) {
        return ErrorReport( FAILING, "_DoTransformation", "could not create kinetic law for total concentation of %s", GetCharArrayOfString( GetSpeciesNodeName( op ) ) );
    }

    reactionsAsModifier = GetReactionsAsModifierInSpeciesNode( op );
    if( GetLinkedListSize( reactionsAsModifier ) > 0 ) {
        if( ( law = CreateOpKineticLaw( KINETIC_LAW_OP_DIVIDE, CloneKineticLaw( totalConKineticLaw ), CloneKineticLaw( denom ) ) ) == NULL ) {
            return ErrorReport( FAILING, "_DoTransformation", "could not create a new kinetic law for %s", GetCharArrayOfString( GetSpeciesNodeName( boundOp ) ) );
        }
        ResetCurrentElement( reactionsAsModifier );
        while( ( reaction = (REACTION*)GetNextFromLinkedList( reactionsAsModifier ) ) != NULL ) {
#ifdef DEBUG
            lawString = ToStringKineticLaw( GetKineticLawInReactionNode( reaction ) );
            printf( "kinetic law for reaction %s is %s" NEW_LINE, GetCharArrayOfString( GetReactionNodeName( reaction ) ), GetCharArrayOfString( lawString ) ); 
            FreeString( &lawString );            
#endif                
            if( IS_FAILED( ( ret = _SubstituteSpeciesWithKineticLaw( reaction, op, law ) ) ) ) {
                END_FUNCTION("_DoTransformation", ret );
                return ret;
            }
            if( IS_FAILED( ( ret = _CombineReactions( ir, reaction, op, NULL ) ) ) ) {
                END_FUNCTION("_DoTransformation", ret );
                return ret;
            }
#ifdef DEBUG
            lawString = ToStringKineticLaw( GetKineticLawInReactionNode( reaction ) );
            printf( "new kinetic law for reaction %s is %s%s", GetCharArrayOfString( GetReactionNodeName( reaction ) ), GetCharArrayOfString( lawString ), NEW_LINE ); 
            FreeString( &lawString );            
#endif                
        }
        
        FreeKineticLaw( &law );
    }
    
    
    ResetCurrentElement( reactions );    
    while( ( reaction = (REACTION*)GetNextFromLinkedList( reactions ) ) != NULL ) {
        speciesList = GetProductsInReactionNode( reaction );
        boundOp = (SPECIES*)GetHeadFromLinkedList( speciesList );
        reactionsAsModifier = GetReactionsAsModifierInSpeciesNode( boundOp );
        if( GetLinkedListSize( reactionsAsModifier ) > 0 ) {
            TRACE_1("transforming reaction %s", GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
            if( ( numer = (KINETIC_LAW*)GetValueFromHashTable( boundOp, sizeof( SPECIES ), massActionTable ) ) == NULL ) {
                return ErrorReport( FAILING, "_DoTransformation", "could not find mass action ratio for %s", GetCharArrayOfString( GetSpeciesNodeName( boundOp ) ) );
            }   
            
            if( ( numer = CreateOpKineticLaw( KINETIC_LAW_OP_TIMES, numer, CloneKineticLaw( totalConKineticLaw ) ) ) == NULL ) {
                return ErrorReport( FAILING, "_DoTransformation", "could not create a new kinetic law" );
            }
            
            if( ( law = CreateOpKineticLaw( KINETIC_LAW_OP_DIVIDE, numer, CloneKineticLaw( denom ) ) ) == NULL ) {
                return ErrorReport( FAILING, "_DoTransformation", "could not create a new kinetic law for %s", GetCharArrayOfString( GetSpeciesNodeName( boundOp ) ) );
            }
            ResetCurrentElement( reactionsAsModifier );
            while( ( productionReaction = (REACTION*)GetNextFromLinkedList( reactionsAsModifier ) ) != NULL ) {
                TRACE_1("\ttransforming production reaction %s", GetCharArrayOfString( GetReactionNodeName( productionReaction ) ) );
#ifdef DEBUG
                lawString = ToStringKineticLaw( GetKineticLawInReactionNode( productionReaction ) );
                printf( "kinetic law for reaction %s is %s%s", GetCharArrayOfString( GetReactionNodeName( productionReaction ) ), GetCharArrayOfString( lawString ), NEW_LINE ); 
                FreeString( &lawString );            
#endif                
                if( IS_FAILED( ( ret = _SubstituteSpeciesWithKineticLaw( productionReaction, boundOp, law ) ) ) ) {
                    END_FUNCTION("_DoTransformation", ret );
                    return ret;
                }
                if( IS_FAILED( ( ret = _CombineReactions( ir, productionReaction, op, boundOp ) ) ) ) {
                    END_FUNCTION("_DoTransformation", ret );
                    return ret;
                }
#ifdef DEBUG
                lawString = ToStringKineticLaw( GetKineticLawInReactionNode( productionReaction ) );
                printf( "new kinetic law for reaction %s is %s%s", GetCharArrayOfString( GetReactionNodeName( productionReaction ) ), GetCharArrayOfString( lawString ), NEW_LINE ); 
                FreeString( &lawString );            
#endif                
            }  
            FreeKineticLaw( &law );                
        }
    }
    
    ResetCurrentElement( reactions );    
    while( ( reaction = (REACTION*)GetNextFromLinkedList( reactions ) ) != NULL ) {
        speciesList = GetProductsInReactionNode( reaction );
        boundOp = (SPECIES*)GetHeadFromLinkedList( speciesList );
        TRACE_1("removing reaction %s", GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
        if( IS_FAILED( ( ret = ir->RemoveReaction( ir, reaction ) ) ) ) {
            END_FUNCTION("_DoTransformation", ret );
            return ret;
        }   
        if( IS_FAILED( ( ret = ir->RemoveSpecies( ir, boundOp ) ) ) ) {
            END_FUNCTION("_DoTransformation", ret );
            return ret;
        } 
    }
    
    FreeKineticLaw( &totalConKineticLaw );
    FreeKineticLaw( &denom );    
    DeleteHashTable( &massActionTable );    
    if( IS_FAILED( ( ret = ir->RemoveSpecies( ir, op ) ) ) ) {
        END_FUNCTION("_DoTransformation", ret );
        return ret;
    }   
    END_FUNCTION("_DoTransformation", SUCCESS );
    return ret;
}

static double _GetSpeciesConcentration( SPECIES *species ) {
    double concentration = 0.0;
    
    START_FUNCTION("_GetSpeciesConcentration");
    
    /*
        assuming concentration is used for its quantity for now. 
    */
    concentration = GetInitialConcentrationInSpeciesNode( species );
    
    END_FUNCTION("_GetSpeciesConcentration", SUCCESS );
    return concentration;
}



static RET_VAL _SubstituteSpeciesWithKineticLaw( REACTION *reaction, SPECIES *target, KINETIC_LAW *to ) {
    RET_VAL ret = SUCCESS;    
    KINETIC_LAW *law = NULL;
    
    START_FUNCTION("_SubstituteSpeciesWithKineticLaw");
    
    law = GetKineticLawInReactionNode( reaction );
    TRACE_2("substitution of species %s in reaction %s", GetCharArrayOfString( GetSpeciesNodeName( target ) ), GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
    
    if( IS_FAILED( ( ret = ReplaceSpeciesWithKineticLawInKineticLaw( law, target, to ) ) ) ) {
        END_FUNCTION("_SubstituteSpeciesWithKineticLaw", FAILING );
        return ret;
    }
    
    END_FUNCTION("_SubstituteSpeciesWithKineticLaw", SUCCESS );
    return ret;
}

static RET_VAL _CombineReactions( IR *ir, REACTION *productionReaction, SPECIES *op, SPECIES *boundOp ) {
    RET_VAL ret = SUCCESS;    
    char *buf = NULL;
    LINKED_LIST *modifiers = NULL;
    LINKED_LIST *reactions = NULL;
    SPECIES *modifier = NULL;
    REACTION *reaction = NULL;
        
    START_FUNCTION("_CombineReactions");
    if( boundOp != NULL ) {
        if( IS_FAILED( ( ret = ir->RemoveModifierInReaction( ir, productionReaction, boundOp ) ) ) ) {
            END_FUNCTION("_CombineReactions", ret );
            return ret;
        } 
    }   
    
    reactions = GetReactionsAsReactantInSpeciesNode( op );
    ResetCurrentElement( reactions );
    while( ( reaction = (REACTION*)GetNextFromLinkedList( reactions ) ) != NULL ) {
        modifiers = GetReactantsInReactionNode( reaction );
        ResetCurrentElement( modifiers );
        while( ( modifier = (SPECIES*)GetNextFromLinkedList( modifiers ) ) != NULL ) {
            if( modifier == op ) {
                continue;
            }
            TRACE_2( "adding modifier %s in the combined reaction %s", 
                GetCharArrayOfString( GetSpeciesNodeName( modifier ) ), GetCharArrayOfString( GetReactionNodeName( productionReaction ) ) );
            if( IS_FAILED( ( ret = ir->AddModifierInReaction( ir, productionReaction, modifier ) ) ) ) {
                END_FUNCTION("_CombineReactions", ret );
                return ret;
            }    
        }    
    }
    
    reactions = GetReactionsAsReactantInSpeciesNode( op );
    ResetCurrentElement( reactions );
    while( ( reaction = (REACTION*)GetNextFromLinkedList( reactions ) ) != NULL ) {
        modifiers = GetModifiersInReactionNode( reaction );
        ResetCurrentElement( modifiers );
        while( ( modifier = (SPECIES*)GetNextFromLinkedList( modifiers ) ) != NULL ) {
            TRACE_2( "adding modifier %s in the combined reaction %s", 
                GetCharArrayOfString( GetSpeciesNodeName( modifier ) ), GetCharArrayOfString( GetReactionNodeName( productionReaction ) ) );
            if( IS_FAILED( ( ret = ir->AddModifierInReaction( ir, productionReaction, modifier ) ) ) ) {
                END_FUNCTION("_CombineReactions", ret );
                return ret;
            }    
        }
    }
    
    END_FUNCTION("_CombineReactions", SUCCESS );
    return ret;
}



static double _FindConstantSpeciesConcentrationTotal( SPECIES *species ) {
    double total = 0.0;
    SPECIES *product = NULL;
    REACTION *reaction = NULL;
    LINKED_LIST *products = NULL;
    LINKED_LIST *reactions = NULL;
        
    START_FUNCTION("_FindConstantSpeciesConcentrationTotal");
    
    reactions = GetReactionsAsReactantInSpeciesNode( species );
    ResetCurrentElement( reactions );
    while( ( reaction = (REACTION*)GetNextFromLinkedList( reactions ) ) != NULL ) {
        products = GetProductsInReactionNode( reaction );
        ResetCurrentElement( products );
        while( ( product = (SPECIES*)GetNextFromLinkedList( products ) ) != NULL ) {
            total += _GetSpeciesConcentration( product ); 
        }
    }
    
    total += _GetSpeciesConcentration( species );
    
    END_FUNCTION("_FindConstantSpeciesConcentrationTotal", SUCCESS );
    return total;
}



static RET_VAL _InitRnapOperatorBindingInternal( RNAP_OPERATOR_BINDING_METHOD_INTERNAL *internal ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("_InitRnapOperatorBindingInternal");
        
    if( ( internal->elements = CreateLinkedList() ) == NULL ) {
        return ErrorReport( FAILING, "_InitRnapOperatorBindingInternal", "could not create an elements list" );
    }
        
    END_FUNCTION("_InitRnapOperatorBindingInternal", SUCCESS );
    return ret;
}

static RET_VAL _CleanRnapOperatorBindingInternal( RNAP_OPERATOR_BINDING_METHOD_INTERNAL *internal ) {
    RET_VAL ret = SUCCESS;
    RNAP_OPERATOR_BINDING_METHOD_INTERNAL_ELEMENT *element = NULL;
    LINKED_LIST *elements = NULL;
    
    START_FUNCTION("_CleanRnapOperatorBindingInternal");
    
    elements = internal->elements;
    if( GetLinkedListSize( elements ) == 0 ) {
        END_FUNCTION("_CleanRnapOperatorBindingInternal", SUCCESS );
        return ret;
    }
    
    ResetCurrentElement( elements );
    while( ( element = (RNAP_OPERATOR_BINDING_METHOD_INTERNAL_ELEMENT*)GetNextFromLinkedList( elements ) ) != NULL ) {
        FreeKineticLaw( &(element->rateRatioKineticLaw) );
        FREE( element );
    }
    
    FreeKineticLaw( &(internal->k1RatioKineticLaw) );
    FreeKineticLaw( &(internal->k2KineticLaw) );
    
    if( IS_FAILED( ( ret = DeleteLinkedList( &(elements) ) ) ) ) {
        END_FUNCTION("_CleanRnapOperatorBindingInternal", ret );
        return ret;
    }    
    
    if( ( internal->elements = CreateLinkedList() ) == NULL ) {
        return ErrorReport( FAILING, "_CleanRnapOperatorBindingInternal", "could not create an elements list" );
    }
    
    END_FUNCTION("_CleanRnapOperatorBindingInternal", SUCCESS );
    return ret;
}

static RET_VAL _FreeRnapOperatorBindingInternal( RNAP_OPERATOR_BINDING_METHOD_INTERNAL *internal ) {
    RET_VAL ret = SUCCESS;
    RNAP_OPERATOR_BINDING_METHOD_INTERNAL_ELEMENT *element = NULL;
    LINKED_LIST *elements = NULL;
    
    START_FUNCTION("_FreeRnapOperatorBindingInternal");
    
    elements = internal->elements;
    ResetCurrentElement( elements );
    while( ( element = (RNAP_OPERATOR_BINDING_METHOD_INTERNAL_ELEMENT*)GetNextFromLinkedList( elements ) ) != NULL ) {
        FreeKineticLaw( &(element->rateRatioKineticLaw) );
        FREE( element );
    }
    
    FreeKineticLaw( &(internal->k1RatioKineticLaw) );
    FreeKineticLaw( &(internal->k2KineticLaw) );
    
    if( IS_FAILED( ( ret = DeleteLinkedList( &(elements) ) ) ) ) {
        END_FUNCTION("_CleanRnapOperatorBindingInternal", ret );
        return ret;
    }    
    
    END_FUNCTION("_FreeRnapOperatorBindingInternal", SUCCESS );
    return ret;
}



