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
#include "symtab.h"
#include "kinetic_law_evaluater.h"

typedef struct {
    SPECIES *substrate;
    SPECIES *complex;
    SPECIES *product;    
    IR_EDGE *substrateEdge;
    IR_EDGE *productEdge;    
    KINETIC_LAW *k1RatioKineticLaw;
    KINETIC_LAW *k2KineticLaw;
    REACTION *complexFormationReaction;
    REACTION *releaseReaction;
} ENZYME_KINETIC_RAPID_EQUILIBRIUM_2_INTERNAL_ELEMENT;
 
typedef struct {
    SPECIES *enzyme;
    LINKED_LIST *elements;
} ENZYME_KINETIC_RAPID_EQUILIBRIUM_2_INTERNAL;


static RET_VAL _FindConditionProperties( ABSTRACTION_METHOD_MANAGER *manager, ABSTRACTION_METHOD *method );

static char * _GetEnzymeKineticRapidEquilibrium2MethodID( ABSTRACTION_METHOD *method );
static RET_VAL _ApplyEnzymeKineticRapidEquilibrium2Method( ABSTRACTION_METHOD *method, IR *ir );      

static BOOL _IsEnzymeKineticRapidEquilibrium2ConditionSatisfied( ABSTRACTION_METHOD *method, SPECIES *species, ENZYME_KINETIC_RAPID_EQUILIBRIUM_2_INTERNAL *internal ); 
static RET_VAL _DoEnzymeKineticRapidEquilibrium2Transformation( ABSTRACTION_METHOD *method, IR *ir, ENZYME_KINETIC_RAPID_EQUILIBRIUM_2_INTERNAL *internal );

static RET_VAL _InitEnzymeKineticRapidEquilibrium2Internal( ENZYME_KINETIC_RAPID_EQUILIBRIUM_2_INTERNAL *internal );
static RET_VAL _CleanEnzymeKineticRapidEquilibrium2Internal( ENZYME_KINETIC_RAPID_EQUILIBRIUM_2_INTERNAL *internal );
static RET_VAL _FreeEnzymeKineticRapidEquilibrium2Internal( ENZYME_KINETIC_RAPID_EQUILIBRIUM_2_INTERNAL *internal );

static BOOL _CheckEnzymeRatioCondition( ABSTRACTION_METHOD *method,  KINETIC_LAW_EVALUATER *evaluator, SPECIES *enzyme, SPECIES *substrate, KINETIC_LAW *k1Ratio );
static BOOL _CheckReverseRateConstantRatioCondition( ABSTRACTION_METHOD *method, KINETIC_LAW_EVALUATER *evaluator, KINETIC_LAW *k1Ratio, KINETIC_LAW *k2 );

static double _GetSpeciesConcentration( SPECIES *species );


ABSTRACTION_METHOD *EnzymeKineticRapidEquilibrium2MethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager ) {
    static ABSTRACTION_METHOD method;
        
    START_FUNCTION("EnzymeKineticRapidEquilibrium2MethodConstructor");

    if( method.manager == NULL ) {
        method.manager = manager;
        method.GetID = _GetEnzymeKineticRapidEquilibrium2MethodID;
        method.Apply = _ApplyEnzymeKineticRapidEquilibrium2Method;
    }
    
    if( IS_FAILED( _FindConditionProperties( manager, &method ) ) ) {
        END_FUNCTION("EnzymeKineticRapidEquilibrium2MethodConstructor", FAILING );
        return NULL;
    }    
    
    TRACE_0( "EnzymeKineticRapidEquilibrium2MethodConstructor invoked" );
    
    END_FUNCTION("EnzymeKineticRapidEquilibrium2MethodConstructor", SUCCESS );
    return &method;
}



static char * _GetEnzymeKineticRapidEquilibrium2MethodID( ABSTRACTION_METHOD *method ) {
    START_FUNCTION("_GetEnzymeKineticRapidEquilibrium2MethodID");
    
    END_FUNCTION("_GetEnzymeKineticRapidEquilibrium2MethodID", SUCCESS );
    return "enzyme-kinetic-rapid-equilibrium-2";
}


static RET_VAL _FindConditionProperties( ABSTRACTION_METHOD_MANAGER *manager, ABSTRACTION_METHOD *method  ) {
    static double thresholds[2];
    char *valueString = NULL;
    double threshold = 0.0;
    COMPILER_RECORD_T *record = NULL;
    REB2SAC_PROPERTIES *properties = NULL;    
    
    record = manager->GetCompilerRecord( manager );
    properties = record->properties;    
    if( ( valueString = properties->GetProperty( properties, REB2SAC_RAPID_EQUILIBRIUM_CONDITION_1_KEY ) ) == NULL ) {
        threshold = DEFAULT_REB2SAC_RAPID_EQUILIBRIUM_CONDITION_1;
    }
    else {
        if( IS_FAILED( StrToFloat( &threshold, valueString ) ) ) {
            threshold = DEFAULT_REB2SAC_RAPID_EQUILIBRIUM_CONDITION_1;
        }
    }
    thresholds[0] = threshold;
    
    if( ( valueString = properties->GetProperty( properties, REB2SAC_RAPID_EQUILIBRIUM_CONDITION_2_KEY ) ) == NULL ) {
        threshold = DEFAULT_REB2SAC_RAPID_EQUILIBRIUM_CONDITION_2;
    }
    else {
        if( IS_FAILED( StrToFloat( &threshold, valueString ) ) ) {
            threshold = DEFAULT_REB2SAC_RAPID_EQUILIBRIUM_CONDITION_2;
        }
    }
    thresholds[1] = threshold;
    
    method->_internal1 = (CADDR_T)thresholds;
    
    return SUCCESS;
}



static RET_VAL _ApplyEnzymeKineticRapidEquilibrium2Method( ABSTRACTION_METHOD *method, IR *ir ) {
    RET_VAL ret = SUCCESS;
    SPECIES *species = NULL;
    LINKED_LIST *speciesList = NULL;
    ENZYME_KINETIC_RAPID_EQUILIBRIUM_2_INTERNAL internal;
    KINETIC_LAW_EVALUATER *evaluater = NULL;
        
    START_FUNCTION("_ApplyEnzymeKineticRapidEquilibrium2Method");

    if( IS_FAILED( ( ret = _InitEnzymeKineticRapidEquilibrium2Internal( &internal ) ) ) ) {
        END_FUNCTION("_ApplyEnzymeKineticRapidEquilibrium2Method", ret );
        return ret;
    } 
    
    if( ( evaluater = CreateKineticLawEvaluater() ) == NULL ) {
        END_FUNCTION("_IsEnzymeKineticRapidEquilibrium2ConditionSatisfied", FAILING );
        return FAILING;
    }
    method->_internal2 = (CADDR_T)evaluater;       
    
    speciesList = ir->GetListOfSpeciesNodes( ir );    
    ResetCurrentElement( speciesList );    
    while( ( species = (SPECIES*)GetNextFromLinkedList( speciesList ) ) != NULL ) {
        if( _IsEnzymeKineticRapidEquilibrium2ConditionSatisfied( method, species, &internal ) ) {
            if( IS_FAILED( ( ret = _DoEnzymeKineticRapidEquilibrium2Transformation( method, ir, &internal ) ) ) ) {
                END_FUNCTION("_ApplyEnzymeKineticRapidEquilibrium2Method", ret );
                return ret;
            }
        }            
        if( IS_FAILED( ( ret = _CleanEnzymeKineticRapidEquilibrium2Internal( &internal ) ) ) ) {
            END_FUNCTION("_ApplyEnzymeKineticRapidEquilibrium2Method", ret );
            return ret;
        } 
    }
    if( IS_FAILED( ( ret = _FreeEnzymeKineticRapidEquilibrium2Internal( &internal ) ) ) ) {
        END_FUNCTION("_ApplyEnzymeKineticRapidEquilibrium2Method", ret );
        return ret;
    } 
    FreeKineticLawEvaluater( &evaluater );
                
    END_FUNCTION("_ApplyEnzymeKineticRapidEquilibrium2Method", SUCCESS );
    return ret;
}      

static BOOL _IsEnzymeKineticRapidEquilibrium2ConditionSatisfied( ABSTRACTION_METHOD *method, SPECIES *species, ENZYME_KINETIC_RAPID_EQUILIBRIUM_2_INTERNAL *internal ) {  
    int num = 0;
    IR_EDGE *edge = NULL;    
    KINETIC_LAW *k1RatioKineticLaw = NULL;
    KINETIC_LAW *k2KineticLaw = NULL;
    STRING *string = NULL;
    SPECIES *substrate = NULL;
    SPECIES *complex = NULL;
    SPECIES *product = NULL;    
    SPECIES *release = NULL;    
    IR_EDGE *substrateEdge = NULL;
    IR_EDGE *productEdge = NULL;
    IR_EDGE *releaseEdge = NULL;    
    REACTION *reaction = NULL;
    REACTION *reaction1 = NULL;
    REACTION *complexFormationReaction = NULL;
    
    KINETIC_LAW *kineticLaw = NULL;
    LINKED_LIST *reactions = NULL; 
    LINKED_LIST *reactants = NULL; 
    LINKED_LIST *list = NULL; 
    LINKED_LIST *products = NULL; 
    LINKED_LIST *edges = NULL;
    
    ENZYME_KINETIC_RAPID_EQUILIBRIUM_2_INTERNAL_ELEMENT *element = NULL;
    KINETIC_LAW_EVALUATER *evaluator = (KINETIC_LAW_EVALUATER*)(method->_internal2);
        
    START_FUNCTION("_IsEnzymeKineticRapidEquilibrium2ConditionSatisfied");
    
    
    /*
    * this species S is not one of properties of interest   
    */
    if( IsKeepFlagSetInSpeciesNode( species ) ) {
        END_FUNCTION("_IsEnzymeKineticRapidEquilibrium2ConditionSatisfied", SUCCESS );
        return FALSE;
    }

    edges = GetReactantEdges( (IR_NODE*)species );   
    if( GetLinkedListSize( edges ) == 0 ) {
        END_FUNCTION("_IsEnzymeKineticRapidEquilibrium2ConditionSatisfied", SUCCESS );
        return FALSE;
    }

    ResetCurrentElement( edges );
    while( ( edge = GetNextEdge( edges ) ) != NULL ) {
        /*
        * every reaction R1, where R1 uses S as a reactant, the stoichiometry of S is 1.
        */
        if( GetStoichiometryInIREdge( edge ) != 1.0 ) {
            END_FUNCTION("_IsEnzymeKineticRapidEquilibrium2ConditionSatisfied", SUCCESS );
            return FALSE;
        }
        
        reaction = GetReactionInIREdge( edge );
        /*
        * every reaction R1, where R1 uses S as a reactant, R1 is a reversible reaction
        */
        if( !IsReactionReversibleInReactionNode( reaction ) ) {
            END_FUNCTION("_IsEnzymeKineticRapidEquilibrium2ConditionSatisfied", SUCCESS );
            return FALSE;
        }
        
        /*
        * k1 Ratio is found from kinetic law of R1  
        */
        kineticLaw = GetKineticLawInReactionNode( reaction );
        if( ( k1RatioKineticLaw = CreateRateConstantRatioKineticLaw( kineticLaw ) ) == NULL ) {
            END_FUNCTION("_IsEnzymeKineticRapidEquilibrium2ConditionSatisfied", SUCCESS );
            return FALSE;
        }
        
        
        /*
        * R1 has 2 reactants
        */
        list = GetReactantEdges( (IR_NODE*)reaction );
        if( GetLinkedListSize( list ) != 2 ) {
            END_FUNCTION("_IsEnzymeKineticRapidEquilibrium2ConditionSatisfied", SUCCESS );
            return FALSE;
        }

        ResetCurrentElement( list );
        while( ( substrateEdge = GetNextEdge( list ) ) != NULL ) {
            substrate = GetSpeciesInIREdge( substrateEdge ); 
            if( substrate  != species ) {
                break;
            }
        }

        /*
        * E0 / ( S0 + kr /kf)
        */
        if( !_CheckEnzymeRatioCondition( method, evaluator, species, substrate, k1RatioKineticLaw ) ) {
            END_FUNCTION("_IsEnzymeKineticRapidEquilibrium2ConditionSatisfied", SUCCESS );
            return FALSE;
        }        
        /*
        * R1 has no modifier
        */
        list = GetModifierEdges( (IR_NODE*)reaction );
        if( GetLinkedListSize( list ) > 0 ) {
            END_FUNCTION("_IsEnzymeKineticRapidEquilibrium2ConditionSatisfied", SUCCESS );
            return FALSE;
        }
        
        /*
        * R1 has 1 product
        */
        list = GetProductEdges( (IR_NODE*)reaction );
        if( GetLinkedListSize( list ) != 1 ) {
            END_FUNCTION("_IsEnzymeKineticRapidEquilibrium2ConditionSatisfied", SUCCESS );
            return FALSE;
        }
        
        complexFormationReaction = reaction;
        
        /*
        * The product of R1, C is produced only by R1
        */
        productEdge = GetHeadEdge( list );
        product = GetSpeciesInIREdge( productEdge );
        list = GetProductEdges( (IR_NODE*)product );
        if( GetLinkedListSize( list ) != 1 ) {
            END_FUNCTION("_IsEnzymeKineticRapidEquilibrium2ConditionSatisfied", SUCCESS );
            return FALSE;
        }
        
        complex = product;
        /*
        * C is not an intersting species
        */
        if( IsKeepFlagSetInSpeciesNode( complex ) ) {
            END_FUNCTION("_IsEnzymeKineticRapidEquilibrium2ConditionSatisfied", SUCCESS );
            return FALSE;
        }
        
        
        /*
        * C is not used as a modifier
        */
        list = GetModifierEdges( (IR_NODE*)complex );
        if( GetLinkedListSize( list ) > 0 ) {
            END_FUNCTION("_IsEnzymeKineticRapidEquilibrium2ConditionSatisfied", SUCCESS );
            return FALSE;
        }
        
        /*
        * C is consumed only by one reaction
        */
        list = GetReactantEdges( (IR_NODE*)complex );
        if( GetLinkedListSize( list ) != 1 ) {
            END_FUNCTION("_IsEnzymeKineticRapidEquilibrium2ConditionSatisfied", SUCCESS );
            return FALSE;
        }        
        edge = GetHeadEdge( list );
        reaction = GetReactionInIREdge( edge );
        list = GetReactantEdges( (IR_NODE*)reaction );
        if( GetLinkedListSize( list ) != 1 ) {
            END_FUNCTION("_IsEnzymeKineticRapidEquilibrium2ConditionSatisfied", SUCCESS );
            return FALSE;
        }
                
        
        /*
        * The reaction R2, which uses C as a reactant, does not have modifier                   
        */
        if( IsReactionReversibleInReactionNode( reaction ) ) {
            END_FUNCTION("_IsEnzymeKineticRapidEquilibrium2ConditionSatisfied", SUCCESS );
            return FALSE;
        }
        
        /*
        * k2 is found from kinetic law of R2  
        */
        kineticLaw = GetKineticLawInReactionNode( reaction );
        if( ( k2KineticLaw = CreateRateConstantKineticLaw( kineticLaw ) ) == NULL ) {
            END_FUNCTION("_IsEnzymeKineticRapidEquilibrium2ConditionSatisfied", SUCCESS );
            return FALSE;
        }

        if( !_CheckReverseRateConstantRatioCondition( method, evaluator, k1RatioKineticLaw, k2KineticLaw ) ) {
            END_FUNCTION("_IsEnzymeKineticRapidEquilibrium2ConditionSatisfied", SUCCESS );
            return FALSE;
        }
        
        /*
        * R2 does not have modifier                   
        */
        list = GetModifierEdges( (IR_NODE*)reaction );
        if( GetLinkedListSize( list ) > 0 ) {
            END_FUNCTION("_IsEnzymeKineticRapidEquilibrium2ConditionSatisfied", SUCCESS );
            return FALSE;
        }
        
        list = GetProductEdges( (IR_NODE*)reaction );
        num = GetLinkedListSize( list );
        /*
        * The number of products in R2 is either 1 or 2                   
        * One of the products of R2 is S                   
        */
        switch( num ) {
            case 1:
                productEdge = GetHeadEdge( list );
                product = GetSpeciesInIREdge( productEdge );
                if( species != product  ) {
                    END_FUNCTION("_IsEnzymeKineticRapidEquilibrium2ConditionSatisfied", SUCCESS );
                    return FALSE;
                }
                productEdge = NULL;
                product = NULL;
            break;                
            
            case 2:            
                productEdge = GetHeadEdge( list );
                product = GetSpeciesInIREdge( productEdge );
                releaseEdge = GetTailEdge( list );
                release = GetSpeciesInIREdge( releaseEdge );
                if( species == product  ) {
                    productEdge = releaseEdge;
                    product = release;
                }
                else {
                    if( species != release ) {
                        END_FUNCTION("_IsEnzymeKineticRapidEquilibrium2ConditionSatisfied", SUCCESS );
                        return FALSE;
                    }
                }
            break;                            
            
            default:
                END_FUNCTION("_IsEnzymeKineticRapidEquilibrium2ConditionSatisfied", SUCCESS );
            return FALSE;            
        }
                                        
        if( ( element = (ENZYME_KINETIC_RAPID_EQUILIBRIUM_2_INTERNAL_ELEMENT*)MALLOC( sizeof(ENZYME_KINETIC_RAPID_EQUILIBRIUM_2_INTERNAL_ELEMENT) ) ) == NULL ) {
            TRACE_1("could not create element in %s", GetCharArrayOfString( GetSpeciesNodeName( species ) ) );
            END_FUNCTION("_IsEnzymeKineticRapidEquilibrium2ConditionSatisfied", FAILING );
            return FALSE;
        }
        
        element->substrate = substrate;
        element->complex = complex;
        element->product = product;
        element->substrateEdge = substrateEdge;
        element->productEdge = productEdge;
        element->k1RatioKineticLaw = k1RatioKineticLaw;
        element->k2KineticLaw = k2KineticLaw;
        element->complexFormationReaction = complexFormationReaction;
        element->releaseReaction = reaction;
        
        TRACE_1("enzyme is: %s", GetCharArrayOfString( GetSpeciesNodeName( species ) ) );
        TRACE_1("\tsubtrate is: %s", GetCharArrayOfString( GetSpeciesNodeName( element->substrate ) ) );
        TRACE_1("\tcomplex is: %s", GetCharArrayOfString( GetSpeciesNodeName( element->complex ) ) );
        TRACE_1("\tproduct is: %s", ( element->product == NULL ? "null" : GetCharArrayOfString( GetSpeciesNodeName( element->product ) ) ) );
        TRACE_1("\tcomplex formation reaction is: %s", GetCharArrayOfString( GetSpeciesNodeName( element->complexFormationReaction ) ) );
        TRACE_1("\trelease reaction is: %s", GetCharArrayOfString( GetSpeciesNodeName( element->releaseReaction ) ) );
#ifdef DEBUG
        string = ToStringKineticLaw( element->k1RatioKineticLaw );
        printf( "\tk1 ratio kinetic law is: %s" NEW_LINE, GetCharArrayOfString( string ) );
        FreeString( &string ); 
        string = ToStringKineticLaw( element->k2KineticLaw );
        printf( "\tk2 kinetic law is: %s" NEW_LINE, GetCharArrayOfString( string ) );
        FreeString( &string ); 
#endif        
        if( IS_FAILED( AddElementInLinkedList( (CADDR_T)element, internal->elements ) ) ) {
            TRACE_1("could not add element in %s", GetCharArrayOfString( GetSpeciesNodeName( species ) ) );
            END_FUNCTION("_IsEnzymeKineticRapidEquilibrium2ConditionSatisfied", FAILING );
            return FALSE;
        }                       
                    
    }
    internal->enzyme = species;
        
    END_FUNCTION("_IsEnzymeKineticRapidEquilibrium2ConditionSatisfied", SUCCESS );
    return TRUE;
}

static BOOL _CheckEnzymeRatioCondition( ABSTRACTION_METHOD *method, KINETIC_LAW_EVALUATER *evaluator,  SPECIES *enzyme, SPECIES *substrate, KINETIC_LAW *k1Ratio ) {
    double ratio = 0.0;
    double threshold = 0.0;
    
    threshold = ((double*)method->_internal1)[0];        
    ratio =  GetInitialConcentrationInSpeciesNode( enzyme ) / (  (1.0 / evaluator->Evaluate( evaluator, k1Ratio )) + GetInitialConcentrationInSpeciesNode( substrate ) );    
    TRACE_2("the enzyme %s = %f", GetCharArrayOfString( GetSpeciesNodeName( enzyme ) ), GetInitialConcentrationInSpeciesNode( enzyme ) );
    TRACE_1("kr/kf = %f", 1.0 / evaluator->Evaluate( evaluator, k1Ratio ) );
    TRACE_1( "the ratio: E0 / (S0 + kr/kf) = %f", ratio );
    TRACE_1( "the threshold = %f", threshold );
    
    return ( ratio < threshold ) ? TRUE : FALSE;         
}


static BOOL _CheckReverseRateConstantRatioCondition( ABSTRACTION_METHOD *method, KINETIC_LAW_EVALUATER *evaluator, KINETIC_LAW *k1Ratio, KINETIC_LAW *k2 ) {
    double ratio = 0.0;
    double threshold = 0.0;
    KINETIC_LAW *k1r = NULL;
    
    k1r = GetOpRightFromKineticLaw( k1Ratio ); 
    threshold = ((double*)method->_internal1)[1];        
    
    ratio =  evaluator->Evaluate( evaluator, k2 ) / evaluator->Evaluate( evaluator, k1r );    
    TRACE_1( "the ratio: k2 / k1r = %f", ratio );
    TRACE_1( "the threshold = %f", threshold );
    
    return ( ratio < threshold ) ? TRUE : FALSE;         
}


 
static RET_VAL _DoEnzymeKineticRapidEquilibrium2Transformation( ABSTRACTION_METHOD *method, IR *ir, ENZYME_KINETIC_RAPID_EQUILIBRIUM_2_INTERNAL *internal ) {
    RET_VAL ret = SUCCESS;
    SPECIES *enzyme = NULL;
    SPECIES *substrate = NULL;
    REACTION *complexFormationReaction = NULL;
    IR_EDGE *edge = NULL;
    KINETIC_LAW *law = NULL;
    KINETIC_LAW *temp =  NULL;
#ifdef DEBUG
    STRING *lawString = NULL;
#endif    
    ENZYME_KINETIC_RAPID_EQUILIBRIUM_2_INTERNAL_ELEMENT *element = NULL;
    LINKED_LIST *elements = NULL;
    
    START_FUNCTION("_DoEnzymeKineticRapidEquilibrium2Transformation");
    
    enzyme = internal->enzyme;
    elements = internal->elements;
    
    ResetCurrentElement( elements );
    while( ( element = (ENZYME_KINETIC_RAPID_EQUILIBRIUM_2_INTERNAL_ELEMENT*)GetNextFromLinkedList( elements ) ) != NULL ) {
        substrate = element->substrate;
        
        if( ( law = CreateOpKineticLaw( KINETIC_LAW_OP_TIMES, 
              CloneKineticLaw( element->k2KineticLaw ), CloneKineticLaw( element->k1RatioKineticLaw ) ) ) == NULL ) {
            return ErrorReport( FAILING, "_DoEnzymeKineticRapidEquilibrium2Transformation", 
                                "error creating kinetic law for %s * %s", 
                                GetCharArrayOfString( ToStringKineticLaw( element->k2KineticLaw ) ), 
                                GetCharArrayOfString( ToStringKineticLaw( element->k1RatioKineticLaw ) ) );
        } 
        if( ( law = CreateOpKineticLaw( KINETIC_LAW_OP_TIMES,  law, CreateSpeciesKineticLaw( enzyme ) ) ) == NULL ) {
            return ErrorReport( FAILING, "_DoEnzymeKineticRapidEquilibrium2Transformation", 
                "error creating kinetic law" );
        } 
        if( ( law = CreateOpKineticLaw( KINETIC_LAW_OP_TIMES, law, CreateSpeciesKineticLaw( substrate ) ) ) == NULL ) {
            return ErrorReport( FAILING, "_DoEnzymeKineticRapidEquilibrium2Transformation", 
                                "error creating kinetic law" );
        } 
        complexFormationReaction = element->complexFormationReaction;
        temp = GetKineticLawInReactionNode( complexFormationReaction );
        FreeKineticLaw( &temp );
        if( IS_FAILED( ( ret = SetKineticLawInReactionNode( complexFormationReaction, law ) ) ) ) {
            END_FUNCTION("_DoEnzymeKineticRapidEquilibrium2Transformation", ret );
            return ret;
        }
#ifdef DEBUG
        lawString = ToStringKineticLaw( law );
        printf( "new kinetic law for %s is %s" NEW_LINE, GetCharArrayOfString( GetReactionNodeName( complexFormationReaction ) ), GetCharArrayOfString( lawString ) );
        FreeString( &lawString );
#endif
        
        if( IS_FAILED( ( ret = SetReactionReversibleInReactionNode( complexFormationReaction, FALSE ) ) ) ) {
            END_FUNCTION("_DoEnzymeKineticRapidEquilibrium2Transformation", ret );
            return ret;
        }
    }
    END_FUNCTION("_DoEnzymeKineticRapidEquilibrium2Transformation", SUCCESS );
    return ret;
}


static RET_VAL _InitEnzymeKineticRapidEquilibrium2Internal( ENZYME_KINETIC_RAPID_EQUILIBRIUM_2_INTERNAL *internal ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("_InitEnzymeKineticRapidEquilibrium2Internal");
    
    internal->enzyme = NULL;
    
    if( ( internal->elements = CreateLinkedList() ) == NULL ) {
        return ErrorReport( FAILING, "_InitEnzymeKineticRapidEquilibrium2Internal", "could not create an elements list" );
    }
        
    END_FUNCTION("_InitEnzymeKineticRapidEquilibrium2Internal", SUCCESS );
    return ret;
}

static RET_VAL _CleanEnzymeKineticRapidEquilibrium2Internal( ENZYME_KINETIC_RAPID_EQUILIBRIUM_2_INTERNAL *internal ) {
    RET_VAL ret = SUCCESS;
    ENZYME_KINETIC_RAPID_EQUILIBRIUM_2_INTERNAL_ELEMENT *element = NULL;
    LINKED_LIST *elements = NULL;
    
    START_FUNCTION("_CleanEnzymeKineticRapidEquilibrium2Internal");
    
    internal->enzyme = NULL;
    
    elements = internal->elements;
    if( GetLinkedListSize( elements ) == 0 ) {
        END_FUNCTION("_CleanEnzymeKineticRapidEquilibrium2Internal", SUCCESS );
        return ret;
    }
    
    ResetCurrentElement( elements );
    while( ( element = (ENZYME_KINETIC_RAPID_EQUILIBRIUM_2_INTERNAL_ELEMENT*)GetNextFromLinkedList( elements ) ) != NULL ) {
        FreeKineticLaw( &(element->k1RatioKineticLaw) );
        FreeKineticLaw( &(element->k2KineticLaw) );
        FREE( element );
    }
    
    if( IS_FAILED( ( ret = DeleteLinkedList( &(elements) ) ) ) ) {
        END_FUNCTION("_CleanEnzymeKineticRapidEquilibrium2Internal", ret );
        return ret;
    }    
    if( ( internal->elements = CreateLinkedList() ) == NULL ) {
        return ErrorReport( FAILING, "_CleanEnzymeKineticRapidEquilibrium2Internal", "could not create an elements list" );
    }
    
    END_FUNCTION("_CleanEnzymeKineticRapidEquilibrium2Internal", SUCCESS );
    return ret;
}

static RET_VAL _FreeEnzymeKineticRapidEquilibrium2Internal( ENZYME_KINETIC_RAPID_EQUILIBRIUM_2_INTERNAL *internal ) {
    RET_VAL ret = SUCCESS;
    ENZYME_KINETIC_RAPID_EQUILIBRIUM_2_INTERNAL_ELEMENT *element = NULL;
    LINKED_LIST *elements = NULL;
    
    START_FUNCTION("_FreeEnzymeKineticRapidEquilibrium2Internal");
    
    internal->enzyme = NULL;
    
    elements = internal->elements;
    ResetCurrentElement( elements );
    while( ( element = (ENZYME_KINETIC_RAPID_EQUILIBRIUM_2_INTERNAL_ELEMENT*)GetNextFromLinkedList( elements ) ) != NULL ) {
        FreeKineticLaw( &(element->k1RatioKineticLaw) );
        FreeKineticLaw( &(element->k2KineticLaw) );
        FREE( element );
    }
    
    if( IS_FAILED( ( ret = DeleteLinkedList( &(elements) ) ) ) ) {
        END_FUNCTION("_CleanEnzymeKineticRapidEquilibrium2Internal", ret );
        return ret;
    }    
    
    END_FUNCTION("_FreeEnzymeKineticRapidEquilibrium2Internal", SUCCESS );
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








