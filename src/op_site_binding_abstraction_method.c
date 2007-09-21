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


static char * _GetOpSiteBindingMethodID( ABSTRACTION_METHOD *method );
static RET_VAL _ApplyOpSiteBindingMethod( ABSTRACTION_METHOD *method, IR *ir );      
static BOOL _IsConditionSatisfied( ABSTRACTION_METHOD *method, SPECIES *species ); 
static RET_VAL _DoTransformation( ABSTRACTION_METHOD *method, IR *ir, SPECIES *op );
static double _GetSpeciesConcentration( SPECIES *species );
static RET_VAL _SubstituteSpeciesWithKineticLaw( REACTION *reaction, SPECIES *target, KINETIC_LAW *to );
static RET_VAL _SubstituteComplexWithKineticLaw( REACTION *reaction, SPECIES *complex, 
        KINETIC_LAW *totalCon, KINETIC_LAW *numer, KINETIC_LAW *denom );
static RET_VAL _CombineReactions( IR *ir, REACTION *productionReaction, SPECIES *op, SPECIES *boundOp );


ABSTRACTION_METHOD *OpSiteBindingAbstractionMethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager ) {
    static ABSTRACTION_METHOD method;
    
    START_FUNCTION("OpSiteBindingAbstractionMethodConstructor");

    if( method.manager == NULL ) {
        method.manager = manager;
        method.GetID = _GetOpSiteBindingMethodID;
        method.Apply = _ApplyOpSiteBindingMethod;
    }
    
    TRACE_0( "OpSiteBindingAbstractionMethodConstructor invoked" );
    
    END_FUNCTION("OpSiteBindingAbstractionMethodConstructor", SUCCESS );
    return &method;
}



static char * _GetOpSiteBindingMethodID( ABSTRACTION_METHOD *method ) {
    START_FUNCTION("_GetOpSiteBindingMethodID");
    
    END_FUNCTION("_GetOpSiteBindingMethodID", SUCCESS );
    return "operator-site-forward-binding-remover";
}



static RET_VAL _ApplyOpSiteBindingMethod( ABSTRACTION_METHOD *method, IR *ir ) {
    RET_VAL ret = SUCCESS;
    SPECIES *species = NULL;
    LINKED_LIST *speciesList = NULL;
    
    START_FUNCTION("_ApplyOpSiteBindingMethod");

    speciesList = ir->GetListOfSpeciesNodes( ir );
    
    ResetCurrentElement( speciesList );    
    while( ( species = (SPECIES*)GetNextFromLinkedList( speciesList ) ) != NULL ) {
        if( _IsConditionSatisfied( method, species ) ) {
            if( IS_FAILED( ( ret = _DoTransformation( method, ir, species ) ) ) ) {
                END_FUNCTION("_ApplyOpSiteBindingMethod", ret );
                return ret;
            }
        }            
    }
            
    END_FUNCTION("_ApplyOpSiteBindingMethod", SUCCESS );
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
    
    edges = GetProductEdges( species ); 
    if( GetLinkedListSize(  edges ) != 0 ) {
        END_FUNCTION("_IsConditionSatisfied", SUCCESS );
        return FALSE;
    }
    /*
        * this species S is not used as a product in any reactions   
        */
    reactantEdges = GetReactantEdges( species );
    ResetCurrentElement( reactantEdges );
    while( ( reactantEdge = GetNextEdge( reactantEdges ) ) != NULL ) {
        /*
            * every reaction R, where R uses S as a reactant, the stoichiometry of S is 1
            */
        if( GetStoichiometryInIREdge( reactantEdge ) != 1 ) {
            END_FUNCTION("_IsConditionSatisfied", SUCCESS );
            return FALSE;
        }
        reaction = GetReactionInIREdge( reactantEdge );
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
        edges = GetReactantEdges( reaction );
        if( GetLinkedListSize( edges ) == 1 ) {
            END_FUNCTION("_IsConditionSatisfied", SUCCESS );
            return FALSE;
        } 
        /*
            * every reaction R, where R uses S as a reactant, R only has 1 product
            */
        edges = GetProductEdges( reaction );
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
        edges = GetProductEdges( product );
        
        if( GetLinkedListSize( edges ) != 1 ) {
            END_FUNCTION("_IsConditionSatisfied", SUCCESS );
            return FALSE;
        }
        
        /*
            *  P is not used as a reactant in any reactions
            */
        edges = GetReactantEdges( product );
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
    double totalConcentration = 0.0;
    SPECIES *boundOp = NULL;
    REACTION *reaction = NULL;
    REACTION *productionReaction = NULL;
    IR_EDGE *edge = NULL;
    IR_EDGE *complexEdge = NULL;
    IR_EDGE *modifierEdge = NULL;
    LINKED_LIST *edges = NULL;
    LINKED_LIST *modifierEdges = NULL;
    LINKED_LIST *complexEdges = NULL;
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
    
    totalConcentration += _GetSpeciesConcentration( op );
    if( ( massActionTable = CreateHashTable( 32 ) ) == NULL ) {
        return ErrorReport( FAILING, "_DoTransformation", "could not create a table for mass action ratio for %s", GetCharArrayOfString( GetSpeciesNodeName( op ) ) );
    } 
    if( ( denom = CreateRealValueKineticLaw( 1.0 ) ) == NULL ) {
        return ErrorReport( FAILING, "_DoTransformation", "could not create a kinetic law value 1.0" );
    }
    
    edges = GetReactantEdges( op );
    ResetCurrentElement( edges );    
    while( ( edge = GetNextEdge( edges ) ) != NULL ) {
        reaction = GetReactionInIREdge( edge );
        complexEdges = GetProductEdges( reaction );
        complexEdge = GetHeadEdge( complexEdges );
        boundOp = GetSpeciesInIREdge( complexEdge );
        totalConcentration += _GetSpeciesConcentration( boundOp );
        if( ( law = CreateMassActionRatioKineticLawWithRateConstantInKineticLaw( reaction, op ) ) == NULL ) {
            return ErrorReport( FAILING, "_DoTransformation", "could not create mass action ratio for %s in %s", 
                GetCharArrayOfString( GetSpeciesNodeName( op ) ), GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
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
    
    modifierEdges = GetModifierEdges( op );
    if( GetLinkedListSize( modifierEdges ) > 0 ) {
        if( ( law = CreateOpKineticLaw( KINETIC_LAW_OP_DIVIDE, CloneKineticLaw( totalConKineticLaw ), CloneKineticLaw( denom ) ) ) == NULL ) {
            return ErrorReport( FAILING, "_DoTransformation", "could not create a new kinetic law for %s", GetCharArrayOfString( GetSpeciesNodeName( op ) ) );
        }
        ResetCurrentElement( modifierEdges );
        while( ( modifierEdge = GetNextEdge( modifierEdges ) ) != NULL ) {
            reaction = GetReactionInIREdge( modifierEdge );
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
            printf( "new kinetic law for reaction %s is %s" NEW_LINE, GetCharArrayOfString( GetReactionNodeName( reaction ) ), GetCharArrayOfString( lawString ) ); 
            FreeString( &lawString );            
#endif                
        }
        
        FreeKineticLaw( &law );
    }
    
    
    ResetCurrentElement( edges );    
    while( ( edge = GetNextEdge( edges ) ) != NULL ) {
        reaction = GetReactionInIREdge( edge );        
        complexEdges = GetProductEdges( reaction );
        complexEdge = GetHeadEdge( complexEdges );        
        boundOp = GetSpeciesInIREdge( complexEdge );
        modifierEdges = GetModifierEdges( boundOp );
        if( GetLinkedListSize( modifierEdges ) > 0 ) {
            TRACE_1("transforming reaction %s", GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
            if( ( numer = (KINETIC_LAW*)GetValueFromHashTable( boundOp, sizeof( SPECIES ), massActionTable ) ) == NULL ) {
                return ErrorReport( FAILING, "_DoTransformation", "could not find mass action ratio for %s", GetCharArrayOfString( GetSpeciesNodeName( boundOp ) ) );
            }   
/*            
            if( ( numer = CreateOpKineticLaw( KINETIC_LAW_OP_TIMES, numer, CloneKineticLaw( totalConKineticLaw ) ) ) == NULL ) {
                return ErrorReport( FAILING, "_DoTransformation", "could not create a new kinetic law" );
            }
            
            if( ( law = CreateOpKineticLaw( KINETIC_LAW_OP_DIVIDE, numer, CloneKineticLaw( denom ) ) ) == NULL ) {
                return ErrorReport( FAILING, "_DoTransformation", "could not create a new kinetic law for %s", GetCharArrayOfString( GetSpeciesNodeName( boundOp ) ) );
            }
*/
            ResetCurrentElement( modifierEdges );       
            while( ( modifierEdge = GetNextEdge( modifierEdges ) ) != NULL ) {     
                productionReaction = GetReactionInIREdge( modifierEdge );
                TRACE_1("\ttransforming production reaction %s", GetCharArrayOfString( GetReactionNodeName( productionReaction ) ) );
#ifdef DEBUG
                lawString = ToStringKineticLaw( GetKineticLawInReactionNode( productionReaction ) );
                printf( "kinetic law for reaction %s is %s" NEW_LINE, GetCharArrayOfString( GetReactionNodeName( productionReaction ) ), GetCharArrayOfString( lawString ) ); 
                FreeString( &lawString );            
#endif                
                if( IS_FAILED( ( ret = _SubstituteComplexWithKineticLaw( productionReaction, boundOp, 
                        totalConKineticLaw, numer, denom ) ) ) ) {
                    END_FUNCTION("_DoTransformation", ret );
                    return ret;
                }
                /*                
                if( IS_FAILED( ( ret = _SubstituteSpeciesWithKineticLaw( productionReaction, boundOp, law ) ) ) ) {
                    END_FUNCTION("_DoTransformation", ret );
                    return ret;
                }
                */
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
/*
            FreeKineticLaw( &law );
*/                
        }
    }
    
    ResetCurrentElement( edges );    
    while( ( edge = GetNextEdge( edges ) ) != NULL ) {
        reaction = GetReactionInIREdge( edge );        
        complexEdges = GetProductEdges( reaction );
        complexEdge = GetHeadEdge( complexEdges );        
        boundOp = GetSpeciesInIREdge( complexEdge );
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

static RET_VAL _SubstituteComplexWithKineticLaw( REACTION *reaction, SPECIES *complex, 
        KINETIC_LAW *totalCon, KINETIC_LAW *numer, KINETIC_LAW *denom ) {
    RET_VAL ret = SUCCESS;    
    KINETIC_LAW *law = NULL;

    START_FUNCTION("_SubstituteComplexWithKineticLaw");

    law = GetKineticLawInReactionNode( reaction );
    TRACE_2("substitution of species %s in reaction %s", GetCharArrayOfString( GetSpeciesNodeName( complex ) ), GetCharArrayOfString( GetReactionNodeName( reaction ) ) );

    if( IS_FAILED( ( ret = ReplaceSpeciesWithKineticLawInKineticLaw( law, complex, numer ) ) ) ) {
        END_FUNCTION("_SubstituteComplexWithKineticLaw", FAILING );
        return ret;
    }
    if( ( law = CreateOpKineticLaw( KINETIC_LAW_OP_TIMES, CloneKineticLaw( totalCon ), law ) ) == NULL ) {
        return ErrorReport( FAILING, "_DoTransformation", "could not create a new kinetic law" );
    }
            
    if( ( law = CreateOpKineticLaw( KINETIC_LAW_OP_DIVIDE, law, CloneKineticLaw( denom ) ) ) == NULL ) {
        return ErrorReport( FAILING, "_DoTransformation", "could not create a new kinetic law" );
    }

    SetKineticLawInReactionNode( reaction, law );
    END_FUNCTION("_SubstituteComplexWithKineticLaw", SUCCESS );
    return ret;
}


static RET_VAL _CombineReactions( IR *ir, REACTION *productionReaction, SPECIES *op, SPECIES *boundOp ) {
    RET_VAL ret = SUCCESS;    
    char *buf = NULL;
    IR_EDGE *edge = NULL;
    IR_EDGE *productionEdge = NULL;
    IR_EDGE *modifierEdge = NULL;
    LINKED_LIST *edges = NULL;
    LINKED_LIST *productionEdges = NULL;
    LINKED_LIST *modifierEdges = NULL;
    LINKED_LIST *reactions = NULL;
    SPECIES *modifier = NULL;
    REACTION *reaction = NULL;
        
    START_FUNCTION("_CombineReactions");
    if( boundOp != NULL ) {
        modifierEdges = GetModifierEdges( boundOp );
        modifierEdge = GetHeadEdge( modifierEdges );
        if( IS_FAILED( ( ret = ir->RemoveModifierEdge( ir, &modifierEdge ) ) ) ) {
            END_FUNCTION("_CombineReactions", ret );
            return ret;
        } 
    }   
    
    edges = GetReactantEdges( op );
    ResetCurrentElement( edges );
    while( ( edge = GetNextEdge( edges ) ) != NULL ) {
        reaction = GetReactionInIREdge( edge );
        modifierEdges = GetReactantEdges( reaction );
        ResetCurrentElement( modifierEdges );
        while( ( modifierEdge = GetNextEdge( modifierEdges ) ) != NULL ) {
            modifier = GetSpeciesInIREdge( modifierEdge );
            if( modifier == op ) {
                continue;
            }
            TRACE_2( "adding modifier %s in the combined reaction %s", 
                GetCharArrayOfString( GetSpeciesNodeName( modifier ) ), GetCharArrayOfString( GetReactionNodeName( productionReaction ) ) );
            if( IS_FAILED( ( ret = ir->AddModifierEdge( ir, productionReaction, modifier, GetStoichiometryInIREdge( modifierEdge ) ) ) ) ) {
                END_FUNCTION("_CombineReactions", ret );
                return ret;
            }    
        }    
    }
    
    edges = GetReactantEdges( op );
    ResetCurrentElement( edges );
    while( ( edge = GetNextEdge( edges ) ) != NULL ) {
        reaction = GetReactionInIREdge( edge );
        modifierEdges = GetModifierEdges( reaction );
        ResetCurrentElement( modifierEdges );
        while( ( modifierEdge = GetNextEdge( modifierEdges ) ) != NULL ) {
            modifier = GetSpeciesInIREdge( modifierEdge );
            TRACE_2( "adding modifier %s in the combined reaction %s", 
                GetCharArrayOfString( GetSpeciesNodeName( modifier ) ), GetCharArrayOfString( GetReactionNodeName( productionReaction ) ) );
            if( IS_FAILED( ( ret = ir->AddModifierEdge( ir, productionReaction, modifier, GetStoichiometryInIREdge( modifierEdge ) ) ) ) ) {
                END_FUNCTION("_CombineReactions", ret );
                return ret;
            }    
        }    
    }
    
    END_FUNCTION("_CombineReactions", SUCCESS );
    return ret;
}


