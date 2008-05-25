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
    KINETIC_LAW *keKineticLaw;
    KINETIC_LAW *k2KineticLaw;
    REACTION *complexFormationReaction;
    REACTION *releaseReaction;
} PPTA_INTERNAL_ELEMENT;
 
typedef struct {
    SPECIES *enzyme;
    LINKED_LIST *elements;
} PPTA_INTERNAL;

static RET_VAL _FindConditionProperties( ABSTRACTION_METHOD_MANAGER *manager, ABSTRACTION_METHOD *method );

static char * _GetPPTAMethodID( ABSTRACTION_METHOD *method );
static RET_VAL _ApplyPPTAMethod( ABSTRACTION_METHOD *method, IR *ir );      
static BOOL _IsPPTAConditionSatisfied( ABSTRACTION_METHOD *method, SPECIES *species, PPTA_INTERNAL *internal ); 
static BOOL _CheckEnzymeRatioCondition( ABSTRACTION_METHOD *method,  KINETIC_LAW_EVALUATER *evaluator, SPECIES *enzyme, SPECIES *substrate, KINETIC_LAW *kmRatio );
static KINETIC_LAW * _CreateKe( KINETIC_LAW *keKineticLaw, KINETIC_LAW *k2KineticLaw );
static RET_VAL _DoPPTATransformation( ABSTRACTION_METHOD *method, IR *ir, PPTA_INTERNAL *internal );

static RET_VAL _InitPPTAInternal( PPTA_INTERNAL *internal );
static RET_VAL _CleanPPTAInternal( PPTA_INTERNAL *internal );
static RET_VAL _FreePPTAInternal( PPTA_INTERNAL *internal );


static double _GetSpeciesConcentration( SPECIES *species );



ABSTRACTION_METHOD *PPTAMethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager ) {
    static ABSTRACTION_METHOD method;
    
    START_FUNCTION("PPTAMethodConstructor");

    if( method.manager == NULL ) {
        method.manager = manager;
        method.GetID = _GetPPTAMethodID;
        method.Apply = _ApplyPPTAMethod;
    }
    
    TRACE_0( "PPTAMethodConstructor invoked" );
    
    END_FUNCTION("PPTAMethodConstructor", SUCCESS );
    return &method;
}



static char * _GetPPTAMethodID( ABSTRACTION_METHOD *method ) {
    START_FUNCTION("_GetPPTAMethodID");
    
    END_FUNCTION("_GetPPTAMethodID", SUCCESS );
    return "ppta";
}



static RET_VAL _ApplyPPTAMethod( ABSTRACTION_METHOD *method, IR *ir ) {
    RET_VAL ret = SUCCESS;
    SPECIES *species = NULL;
    LINKED_LIST *speciesList = NULL;
    PPTA_INTERNAL internal;
    KINETIC_LAW_EVALUATER *evaluater = NULL;
    
    START_FUNCTION("_ApplyPPTAMethod");

    if( IS_FAILED( ( ret = _InitPPTAInternal( &internal ) ) ) ) {
        END_FUNCTION("_ApplyPPTAMethod", ret );
        return ret;
    } 
    
    if( ( evaluater = CreateKineticLawEvaluater() ) == NULL ) {
        END_FUNCTION("_IsEnzymeKineticRapidEquilibrium1ConditionSatisfied", FAILING );
        return FAILING;
    }
    method->_internal2 = (CADDR_T)evaluater;       
    
    speciesList = ir->GetListOfSpeciesNodes( ir );    
    ResetCurrentElement( speciesList );    
    while( ( species = (SPECIES*)GetNextFromLinkedList( speciesList ) ) != NULL ) {
        if( _IsPPTAConditionSatisfied( method, species, &internal ) ) {
            if( IS_FAILED( ( ret = _DoPPTATransformation( method, ir, &internal ) ) ) ) {
                END_FUNCTION("_ApplyPPTAMethod", ret );
                return ret;
            }
        }            
        if( IS_FAILED( ( ret = _CleanPPTAInternal( &internal ) ) ) ) {
            END_FUNCTION("_ApplyPPTAMethod", ret );
            return ret;
        } 
    }
    if( IS_FAILED( ( ret = _FreePPTAInternal( &internal ) ) ) ) {
        END_FUNCTION("_ApplyPPTAMethod", ret );
        return ret;
    } 
    FreeKineticLawEvaluater( &evaluater );
            
    END_FUNCTION("_ApplyPPTAMethod", SUCCESS );
    return ret;
}      

static BOOL _IsPPTAConditionSatisfied( ABSTRACTION_METHOD *method, SPECIES *species, PPTA_INTERNAL *internal ) {    
    int num = 0;
    IR_EDGE *edge = NULL;    
    KINETIC_LAW *k1Ratio = NULL;
    KINETIC_LAW *keKineticLaw = NULL;
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
    
    PPTA_INTERNAL_ELEMENT *element = NULL;
    KINETIC_LAW_EVALUATER *evaluater = (KINETIC_LAW_EVALUATER*)(method->_internal2);
        
    START_FUNCTION("_IsPPTAConditionSatisfied");
    
    
    /*
    * this species S is not one of properties of interest   
    */
    if( IsKeepFlagSetInSpeciesNode( species ) ) {
        END_FUNCTION("_IsPPTAConditionSatisfied", SUCCESS );
        return FALSE;
    }

    edges = GetReactantEdges( (IR_NODE*)species );   
    if( GetLinkedListSize( edges ) == 0 ) {
        END_FUNCTION("_IsPPTAConditionSatisfied", SUCCESS );
        return FALSE;
    }
    
    ResetCurrentElement( edges );
    while( ( edge = GetNextEdge( edges ) ) != NULL ) {
        /*
        * every reaction R1, where R1 uses S as a reactant, the stoichiometry of S is 1.
        */
        if( GetStoichiometryInIREdge( edge ) != 1 ) {
            END_FUNCTION("_IsPPTAConditionSatisfied", SUCCESS );
            return FALSE;
        }
        
        reaction = GetReactionInIREdge( edge );
        /*
        * every reaction R1, where R1 uses S as a reactant, R1 is a reversible reaction
        */
        if( !IsReactionReversibleInReactionNode( reaction ) ) {
            END_FUNCTION("_IsPPTAConditionSatisfied", SUCCESS );
            return FALSE;
        }
        
        /*
        * k1 Ratio is found from kinetic law of R1  
        */
        kineticLaw = GetKineticLawInReactionNode( reaction );
        if( ( k1Ratio = CreateRateConstantRatioKineticLaw( kineticLaw ) ) == NULL ) {
            END_FUNCTION("_IsPPTAConditionSatisfied", SUCCESS );
            return FALSE;
        }
                
                
        /*
        * R1 has 2 reactants
        */
        list = GetReactantEdges( (IR_NODE*)reaction );
        if( GetLinkedListSize( list ) != 2 ) {
            END_FUNCTION("_IsPPTAConditionSatisfied", SUCCESS );
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
        * R1 has no modifier
        */
        list = GetModifierEdges( (IR_NODE*)reaction );
        if( GetLinkedListSize( list ) > 0 ) {
            END_FUNCTION("_IsPPTAConditionSatisfied", SUCCESS );
            return FALSE;
        }
        
        /*
        * R1 has 1 product
        */
        list = GetProductEdges( (IR_NODE*)reaction );
        if( GetLinkedListSize( list ) != 1 ) {
            END_FUNCTION("_IsPPTAConditionSatisfied", SUCCESS );
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
            END_FUNCTION("_IsPPTAConditionSatisfied", SUCCESS );
            return FALSE;
        }
        
        complex = product;
        /*
        * C is not an intersting species
        */
        if( IsKeepFlagSetInSpeciesNode( product ) ) {
            END_FUNCTION("_IsPPTAConditionSatisfied", SUCCESS );
            return FALSE;
        }
        
        
        /*
        * C is not used as a modifier
        */
        list = GetModifierEdges((IR_NODE*) product );
        if( GetLinkedListSize( list ) > 0 ) {
            END_FUNCTION("_IsPPTAConditionSatisfied", SUCCESS );
            return FALSE;
        }
        
        /*
        * C is consumed only by one reaction
        */
        list = GetReactantEdges( (IR_NODE*)product );
        if( GetLinkedListSize( list ) != 1 ) {
            END_FUNCTION("_IsPPTAConditionSatisfied", SUCCESS );
            return FALSE;
        }        
        edge = GetHeadEdge( list );
        reaction = GetReactionInIREdge( edge );
        list = GetReactantEdges( (IR_NODE*)reaction );
        if( GetLinkedListSize( list ) != 1 ) {
            END_FUNCTION("_IsPPTAConditionSatisfied", SUCCESS );
            return FALSE;
        }
                
        
        /*
        * The reaction R2, which uses C as a reactant, does not have modifier                   
        */
        if( IsReactionReversibleInReactionNode( reaction ) ) {
            END_FUNCTION("_IsPPTAConditionSatisfied", SUCCESS );
            return FALSE;
        }
        
        /*
        * k2 is found from kinetic law of R2  
        */
        kineticLaw = GetKineticLawInReactionNode( reaction );
        if( ( k2KineticLaw = CreateRateConstantKineticLaw( kineticLaw ) ) == NULL ) {
            END_FUNCTION("_IsPPTAConditionSatisfied", SUCCESS );
            return FALSE;
        }

        keKineticLaw =  _CreateKe( k1Ratio, k2KineticLaw );
        FreeKineticLaw( &k1Ratio );
        
        /*
        * R2 does not have modifier                   
        */
        list = GetModifierEdges((IR_NODE*) reaction );
        if( GetLinkedListSize( list ) > 0 ) {
            END_FUNCTION("_IsPPTAConditionSatisfied", SUCCESS );
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
                    END_FUNCTION("_IsPPTAConditionSatisfied", SUCCESS );
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
                        END_FUNCTION("_IsPPTAConditionSatisfied", SUCCESS );
                        return FALSE;
                    }
                }
            break;                            
            
            default:
                END_FUNCTION("_IsPPTAConditionSatisfied", SUCCESS );
            return FALSE;            
        }
        
                                        
        if( ( element = (PPTA_INTERNAL_ELEMENT*)MALLOC( sizeof(PPTA_INTERNAL_ELEMENT) ) ) == NULL ) {
            TRACE_1("could not create element in %s", GetCharArrayOfString( GetSpeciesNodeName( species ) ) );
            END_FUNCTION("_IsPPTAConditionSatisfied", FAILING );
            return FALSE;
        }
                
        element->substrate = substrate;
        element->complex = complex;
        element->product = product;
        element->substrateEdge = substrateEdge;
        element->productEdge = productEdge;
        element->keKineticLaw = keKineticLaw;
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
        string = ToStringKineticLaw( element->keKineticLaw );
        printf( "\tkm ratio kinetic law is: %s" NEW_LINE, GetCharArrayOfString( string ) );
        FreeString( &string ); 
        string = ToStringKineticLaw( element->k2KineticLaw );
        printf( "\tk2 kinetic law is: %s" NEW_LINE, GetCharArrayOfString( string ) );
        FreeString( &string ); 
#endif        
        if( IS_FAILED( AddElementInLinkedList( (CADDR_T)element, internal->elements ) ) ) {
            TRACE_1("could not add element in %s", GetCharArrayOfString( GetSpeciesNodeName( species ) ) );
            END_FUNCTION("_IsPPTAConditionSatisfied", FAILING );
            return FALSE;
        }                       
                    
    }
    internal->enzyme = species;
    
    END_FUNCTION("_IsPPTAConditionSatisfied", SUCCESS );
    return TRUE;
}


static KINETIC_LAW *_CreateKe( KINETIC_LAW *keKineticLaw, KINETIC_LAW *k2KineticLaw ) {
    KINETIC_LAW *km = NULL;    
    KINETIC_LAW *k1f = NULL;    
    KINETIC_LAW *k1b = NULL;    

    START_FUNCTION("_CreateKe");
    
    k1f = CloneKineticLaw( GetOpLeftFromKineticLaw( keKineticLaw ) );
    k1b = CloneKineticLaw( GetOpRightFromKineticLaw( keKineticLaw ) );
    km = CreateOpKineticLaw( KINETIC_LAW_OP_PLUS, k1b, CloneKineticLaw( k2KineticLaw ) );
    km = CreateOpKineticLaw( KINETIC_LAW_OP_DIVIDE, k1f, km );
    
    END_FUNCTION("_CreateKe", SUCCESS );
    return km;
}

 
static RET_VAL _DoPPTATransformation( ABSTRACTION_METHOD *method, IR *ir, PPTA_INTERNAL *internal ) {
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
    PPTA_INTERNAL_ELEMENT *element = NULL;
    LINKED_LIST *elements = NULL;
    
    START_FUNCTION("_DoPPTATransformation");
    
    enzyme = internal->enzyme;
    elements = internal->elements;
    
    
    ResetCurrentElement( elements );    
    while( ( element = (PPTA_INTERNAL_ELEMENT*)GetNextFromLinkedList( elements ) ) != NULL ) {
        substrate = element->substrate;
        
        if( ( law = CreateOpKineticLaw( KINETIC_LAW_OP_TIMES, CloneKineticLaw( element->k2KineticLaw ), CloneKineticLaw( element->keKineticLaw ) ) ) == NULL ) {
            return ErrorReport( FAILING, "_DoPPTATransformation", 
                "error creating kinetic law for %s * %s", GetCharArrayOfString( ToStringKineticLaw( element->k2KineticLaw ) ), GetCharArrayOfString( ToStringKineticLaw( element->keKineticLaw ) ) );
        } 
        if( ( temp = CreateOpKineticLaw( KINETIC_LAW_OP_TIMES,CreateSpeciesKineticLaw( enzyme ),
	CreateSpeciesKineticLaw( substrate ) ) ) == NULL ) {
            return ErrorReport( FAILING, "_DoPPTATransformation", 
                "error creating kinetic law for %s * %s", 
                GetCharArrayOfString( GetSpeciesNodeName( enzyme ) ), GetCharArrayOfString( GetSpeciesNodeName( substrate ) ) );
        } 
        
        if( ( law = CreateOpKineticLaw( KINETIC_LAW_OP_TIMES, law, temp ) ) == NULL ) {
            return ErrorReport( FAILING, "_DoPPTATransformation", 
                "error creating kinetic law k1' * E * S");
        } 
        complexFormationReaction = element->complexFormationReaction;
        temp = GetKineticLawInReactionNode( complexFormationReaction );
        FreeKineticLaw( &temp );
        if( IS_FAILED( ( ret = SetKineticLawInReactionNode( complexFormationReaction, law ) ) ) ) {
            END_FUNCTION("_DoPPTATransformation", ret );
            return ret;
        }
#ifdef DEBUG
        lawString = ToStringKineticLaw( law );
        printf( "new kinetic law for %s is %s" NEW_LINE, GetCharArrayOfString( GetReactionNodeName( complexFormationReaction ) ), GetCharArrayOfString( lawString ) );
        FreeString( &lawString );
#endif        
        if( IS_FAILED( ( ret = SetReactionReversibleInReactionNode( complexFormationReaction, FALSE ) ) ) ) {
            END_FUNCTION("_DoPPTATransformation", ret );
            return ret;
        }                
    }
    
    END_FUNCTION("_DoPPTATransformation", SUCCESS );
    return ret;
}


static RET_VAL _InitPPTAInternal( PPTA_INTERNAL *internal ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("_InitPPTAInternal");
    
    internal->enzyme = NULL;
    
    if( ( internal->elements = CreateLinkedList() ) == NULL ) {
        return ErrorReport( FAILING, "_InitPPTAInternal", "could not create an elements list" );
    }
        
    END_FUNCTION("_InitPPTAInternal", SUCCESS );
    return ret;
}

static RET_VAL _CleanPPTAInternal( PPTA_INTERNAL *internal ) {
    RET_VAL ret = SUCCESS;
    PPTA_INTERNAL_ELEMENT *element = NULL;
    LINKED_LIST *elements = NULL;
    
    START_FUNCTION("_CleanPPTAInternal");
    
    internal->enzyme = NULL;
    
    elements = internal->elements;
    if( GetLinkedListSize( elements ) == 0 ) {
        END_FUNCTION("_CleanPPTAInternal", SUCCESS );
        return ret;
    }
    
    ResetCurrentElement( elements );
    while( ( element = (PPTA_INTERNAL_ELEMENT*)GetNextFromLinkedList( elements ) ) != NULL ) {
        FreeKineticLaw( &(element->keKineticLaw) );
        FreeKineticLaw( &(element->k2KineticLaw) );
        FREE( element );
    }
    
    if( IS_FAILED( ( ret = DeleteLinkedList( &(elements) ) ) ) ) {
        END_FUNCTION("_CleanPPTAInternal", ret );
        return ret;
    }    
    if( ( internal->elements = CreateLinkedList() ) == NULL ) {
        return ErrorReport( FAILING, "_CleanPPTAInternal", "could not create an elements list" );
    }
    
    END_FUNCTION("_CleanPPTAInternal", SUCCESS );
    return ret;
}

static RET_VAL _FreePPTAInternal( PPTA_INTERNAL *internal ) {
    RET_VAL ret = SUCCESS;
    PPTA_INTERNAL_ELEMENT *element = NULL;
    LINKED_LIST *elements = NULL;
    
    START_FUNCTION("_FreePPTAInternal");
    
    internal->enzyme = NULL;
    
    elements = internal->elements;
    ResetCurrentElement( elements );
    while( ( element = (PPTA_INTERNAL_ELEMENT*)GetNextFromLinkedList( elements ) ) != NULL ) {
        FreeKineticLaw( &(element->keKineticLaw) );
        FreeKineticLaw( &(element->k2KineticLaw) );
        FREE( element );
    }
    
    if( IS_FAILED( ( ret = DeleteLinkedList( &(elements) ) ) ) ) {
        END_FUNCTION("_CleanPPTAInternal", ret );
        return ret;
    }    
    
    END_FUNCTION("_FreePPTAInternal", SUCCESS );
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

