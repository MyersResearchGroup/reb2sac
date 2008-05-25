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


static char * _GetOpSiteBindingMethod2ID( ABSTRACTION_METHOD *method );
static RET_VAL _ApplyOpSiteBindingMethod2( ABSTRACTION_METHOD *method, IR *ir );      
static BOOL _IsConditionSatisfied( ABSTRACTION_METHOD *method, SPECIES *species ); 
static RET_VAL _DoTransformation( ABSTRACTION_METHOD *method, IR *ir, SPECIES *op );
static double _GetSpeciesConcentration( SPECIES *species );


ABSTRACTION_METHOD *OpSiteBindingAbstractionMethod2Constructor(  ABSTRACTION_METHOD_MANAGER *manager ) {
    static ABSTRACTION_METHOD method;
    
    START_FUNCTION("OpSiteBindingAbstractionMethod2Constructor");

    if( method.manager == NULL ) {
        method.manager = manager;
        method.GetID = _GetOpSiteBindingMethod2ID;
        method.Apply = _ApplyOpSiteBindingMethod2;
    }
    
    TRACE_0( "OpSiteBindingAbstractionMethod2Constructor invoked" );
    
    END_FUNCTION("OpSiteBindingAbstractionMethod2Constructor", SUCCESS );
    return &method;
}



static char * _GetOpSiteBindingMethod2ID( ABSTRACTION_METHOD *method ) {
    START_FUNCTION("_GetOpSiteBindingMethod2ID");
    
    END_FUNCTION("_GetOpSiteBindingMethod2ID", SUCCESS );
    return "operator-site-forward-binding-remover2";
}



static RET_VAL _ApplyOpSiteBindingMethod2( ABSTRACTION_METHOD *method, IR *ir ) {
    RET_VAL ret = SUCCESS;
    SPECIES *species = NULL;
    LINKED_LIST *speciesList = NULL;
    
    START_FUNCTION("_ApplyOpSiteBindingMethod2");

    speciesList = ir->GetListOfSpeciesNodes( ir );
    
    ResetCurrentElement( speciesList );    
    while( ( species = (SPECIES*)GetNextFromLinkedList( speciesList ) ) != NULL ) {
        if( _IsConditionSatisfied( method, species ) ) {
            if( IS_FAILED( ( ret = _DoTransformation( method, ir, species ) ) ) ) {
                END_FUNCTION("_ApplyOpSiteBindingMethod2", ret );
                return ret;
            }
        }            
    }
            
    END_FUNCTION("_ApplyOpSiteBindingMethod2", SUCCESS );
    return ret;
}      

static BOOL _IsConditionSatisfied( ABSTRACTION_METHOD *method, SPECIES *species ) {    
    double threshold = 0.0;
    double opConTotal = 0.0;
    char *valueString = NULL;
    SPECIES *product = NULL;
    IR_EDGE *edge = NULL;
    IR_EDGE *reactantEdge = NULL;
    REACTION *reaction = NULL;
    KINETIC_LAW *kineticLaw = NULL;
    LINKED_LIST *reactantEdges = NULL; 
    LINKED_LIST *productEdges = NULL; 
    LINKED_LIST *edges = NULL; 
    ABSTRACTION_METHOD_MANAGER *manager = NULL;
    COMPILER_RECORD_T *record = NULL;
    REB2SAC_PROPERTIES *properties = NULL;
   
    START_FUNCTION("_IsConditionSatisfied");
    
    
    /*
    * this species S is not one of properties of interest   
    */
    if( IsKeepFlagSetInSpeciesNode( species ) ) {
        END_FUNCTION("_IsConditionSatisfied", SUCCESS );
        return FALSE;
    }
    
    edges = GetProductEdges( (IR_NODE*)species ); 
    if( GetLinkedListSize(  edges ) != 0 ) {
        END_FUNCTION("_IsConditionSatisfied", SUCCESS );
        return FALSE;
    }
    /*
        * this species S is not used as a product in any reactions   
        */
    reactantEdges = GetReactantEdges( (IR_NODE*)species );
    ResetCurrentElement( reactantEdges );
    while( ( reactantEdge = GetNextEdge( reactantEdges ) ) != NULL ) {
        /*
            * every reaction R, where R uses S as a reactant, the stoichiometry of S is 1
            */
        if( GetStoichiometryInIREdge( reactantEdge ) != 1 ) {
            END_FUNCTION("_IsConditionSatisfied", SUCCESS );
            return FALSE;
        }
        reaction = GetReactionInIREdge( (IR_EDGE*)reactantEdge );
        /*
            * every reaction R, where R uses S as a reactant, R is a reversible reaction
            */
        if( !IsReactionReversibleInReactionNode( reaction ) ) {
            END_FUNCTION("_IsConditionSatisfied", SUCCESS );
            return FALSE;
        }            
        
        /*
            * every reaction R, where R uses S as a reactant, kinetic law of R K has a pattern of <kinetic-law> - <kinetic-law> 
            */
        kineticLaw = GetKineticLawInReactionNode( reaction );
        if( !IsOpKineticLaw( kineticLaw ) ) {
            END_FUNCTION("_IsConditionSatisfied", SUCCESS );
            return FALSE;
        }
        if( GetOpTypeFromKineticLaw( kineticLaw ) != KINETIC_LAW_OP_MINUS ) {
            END_FUNCTION("_IsConditionSatisfied", SUCCESS );
            return FALSE;
        }
        
        /*
            *every reaction R, where R uses S as a reactant, R has more than 1 reactants
            */
        edges = GetReactantEdges( (IR_NODE*)reaction );
        if( GetLinkedListSize( edges ) == 1 ) {
            END_FUNCTION("_IsConditionSatisfied", SUCCESS );
            return FALSE;
        } 
        /*
            * every reaction R, where R uses S as a reactant, R only has 1 product
            */
        edges = GetProductEdges( (IR_NODE*)reaction );
        if( GetLinkedListSize( edges ) != 1 ) {
            END_FUNCTION("_IsConditionSatisfied", SUCCESS );
            return FALSE;
        }
        /*
            * species P, where P is the product of R
            */
        edge = GetHeadEdge( edges );
        product = GetSpeciesInIREdge( edge );
        opConTotal += _GetSpeciesConcentration( product );
        
        /*
            * species P, where P is the product of R, is not property of interest
            */
        if( IsKeepFlagSetInSpeciesNode( product ) ) {
            END_FUNCTION("_IsConditionSatisfied", SUCCESS );
            return FALSE;
        }
        
        /*
            *  P is produced only by R
            */
        edges = GetProductEdges( (IR_NODE*)product );
        
        if( GetLinkedListSize( edges ) != 1 ) {
            END_FUNCTION("_IsConditionSatisfied", SUCCESS );
            return FALSE;
        }
        
        /*
            *  P is not used as a reactant in any reactions
            */
        edges = GetReactantEdges( (IR_NODE*)product );
        if( GetLinkedListSize( edges ) != 0 ) {
            END_FUNCTION("_IsConditionSatisfied", SUCCESS );
            return FALSE;
        }
    }
    
    opConTotal += _GetSpeciesConcentration( species );
    
    manager = method->manager;
    record = manager->GetCompilerRecord( manager );
    properties = record->properties;
    if( ( valueString = properties->GetProperty( properties, REB2SAC_OPERATOR_MAX_CONCENTRATION_THRESHOLD_KEY ) ) == NULL ) {
        threshold = DEFAULT_REB2SAC_OPERATOR_MAX_CONCENTRATION_THRESHOLD;
    }
    else {
        if( IS_FAILED( StrToFloat( &threshold, valueString ) ) ) {
            threshold = DEFAULT_REB2SAC_OPERATOR_MAX_CONCENTRATION_THRESHOLD;
        }
    }
    if( opConTotal > threshold ) {
        END_FUNCTION("_IsRnap", SUCCESS );
        return FALSE;
    }
    TRACE_1("species %s is an operator", GetCharArrayOfString( GetSpeciesNodeName( species ) ) );
       
    TRACE_1("species %s satisfies the condition", GetCharArrayOfString( GetSpeciesNodeName( species ) ) );
    END_FUNCTION("_IsConditionSatisfied", SUCCESS );
    return TRUE;
}

 
static RET_VAL _DoTransformation( ABSTRACTION_METHOD *method, IR *ir, SPECIES *op ) {
    RET_VAL ret = SUCCESS;
    SPECIES *boundOp = NULL;
    REACTION *transientReaction = NULL;
    REACTION *productionReaction = NULL;
    IR_EDGE *edge = NULL;
    IR_EDGE *productionEdge = NULL;
    IR_EDGE *complexEdge = NULL;
    LINKED_LIST *edges = NULL;
    LINKED_LIST *productionEdges = NULL;
    LINKED_LIST *complexEdges = NULL;
    LINKED_LIST *laws = NULL;
    KINETIC_LAW *k1Ratio = NULL;
    KINETIC_LAW *k2 = NULL;
    KINETIC_LAW *law = NULL;
    KINETIC_LAW *left = NULL;
    KINETIC_LAW *newKineticLaw = NULL;
#ifdef DEBUG
    STRING *speciesString = NULL;
    STRING *lawString = NULL;
#endif    
    
    START_FUNCTION("_DoTransformation");
        
    edges = GetReactantEdges( (IR_NODE*)op );
    ResetCurrentElement( edges );    
    while( ( edge = GetNextEdge( edges ) ) != NULL ) {
        transientReaction = GetReactionInIREdge( (IR_EDGE*)edge );
        law = GetKineticLawInReactionNode( transientReaction );
        left = GetOpLeftFromKineticLaw( law );
        complexEdges = GetProductEdges( (IR_NODE*)transientReaction );
        complexEdge = GetHeadEdge( complexEdges );
        boundOp = GetSpeciesInIREdge( complexEdge );
        productionEdges = GetReactantEdges( (IR_NODE*)boundOp );
        productionEdge = GetHeadEdge( productionEdges );
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



