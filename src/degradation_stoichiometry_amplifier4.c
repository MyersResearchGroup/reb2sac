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


static char * _GetDegradationStoichiometryAmplificationMethod4ID( ABSTRACTION_METHOD *method );
static RET_VAL _ApplyDegradationStoichiometryAmplificationMethod4( ABSTRACTION_METHOD *method, IR *ir );      
static BOOL _IsConditionSatisfied( ABSTRACTION_METHOD *method, SPECIES *species );
static RET_VAL _DoTransformation( ABSTRACTION_METHOD *method, IR *ir, SPECIES *species );

static RET_VAL _CombineDegradationReactions(ABSTRACTION_METHOD *method, IR *ir, SPECIES *species );
static RET_VAL _CombineProductionReactions( ABSTRACTION_METHOD *method, IR *ir, SPECIES *species );
static RET_VAL _AdjustProductionKineticLaw( ABSTRACTION_METHOD *method, SPECIES *species, REACTION *reaction, KINETIC_LAW *productionKineticLaw, KINETIC_LAW *degradationKineticLaw, double stoichiometry );
static RET_VAL _AdjustDegradationKineticLaw( ABSTRACTION_METHOD *method, SPECIES *species, REACTION *reaction, KINETIC_LAW *productionKineticLaw, KINETIC_LAW *degradationKineticLaw, double stoichiometry );
static RET_VAL _AddModifiers( ABSTRACTION_METHOD *method, IR *ir, REACTION *production, REACTION *degradation );
static KINETIC_LAW *_GenerateReplacementForProduction( ABSTRACTION_METHOD *method, SPECIES *species, double stoichiometry ); 
static KINETIC_LAW *_GenerateReplacementForDegradation( ABSTRACTION_METHOD *method, SPECIES *species, double stoichiometry ); 

static UINT32 _FindAmplifier( ABSTRACTION_METHOD_MANAGER *manager );


ABSTRACTION_METHOD *DegradationStoichiometryAmplificationMethod4Constructor(  ABSTRACTION_METHOD_MANAGER *manager ) {
    static ABSTRACTION_METHOD method;
    UINT32 amplifier = 0;
    
    START_FUNCTION("DegradationStoichiometryAmplificationMethod4Constructor");

    if( method.manager == NULL ) {
        method.manager = manager;
        method.GetID = _GetDegradationStoichiometryAmplificationMethod4ID;
        method.Apply = _ApplyDegradationStoichiometryAmplificationMethod4;
    }
    
    amplifier = _FindAmplifier( manager );
    method._internal1 = (CADDR_T)amplifier;
    
    TRACE_0( "DegradationStoichiometryAmplificationMethod4Constructor invoked" );
    
    END_FUNCTION("DegradationStoichiometryAmplificationMethod4Constructor", SUCCESS );
    return &method;
}



static char * _GetDegradationStoichiometryAmplificationMethod4ID( ABSTRACTION_METHOD *method ) {
    START_FUNCTION("_GetDegradationStoichiometryAmplificationMethod4ID");
    
    END_FUNCTION("_GetDegradationStoichiometryAmplificationMethod4ID", SUCCESS );
    return "degradation-stoichiometry-amplifier4";
}



static RET_VAL _ApplyDegradationStoichiometryAmplificationMethod4( ABSTRACTION_METHOD *method, IR *ir ) {
    RET_VAL ret = SUCCESS;
    SPECIES *species = NULL;
    LINKED_LIST *speciesList = NULL;
    
    START_FUNCTION("_ApplyDegradationStoichiometryAmplificationMethod4");
     
    speciesList = ir->GetListOfSpeciesNodes( ir );
    ResetCurrentElement( speciesList );    
    while( ( species = (SPECIES*)GetNextFromLinkedList( speciesList ) ) != NULL ) {
        if( _IsConditionSatisfied( method, species ) ) {
            if( IS_FAILED( ( ret = _DoTransformation( method, ir, species ) ) ) ) {
            }
        }       
    } 
            
    END_FUNCTION("_ApplyDegradationStoichiometryAmplificationMethod4", SUCCESS );
    return ret;
}      

static BOOL _IsConditionSatisfied( ABSTRACTION_METHOD *method, SPECIES *species ) {
    double stoichiometry = -1;
    SPECIES *reactant = NULL;
    SPECIES *product = NULL;
    IR_EDGE *edge = NULL;
    REACTION *reaction = NULL;
    LINKED_LIST *edges = NULL;
    LINKED_LIST *reactantEdges = NULL;
    LINKED_LIST *productEdges = NULL;
    
    edges = GetReactantEdges( (IR_NODE*)species );
    if( GetLinkedListSize( edges ) == 0 ) {
        return FALSE;
    }
    ResetCurrentElement( edges );
    while( ( edge = GetNextEdge( edges ) ) != NULL ) {
        reaction = GetReactionInIREdge( edge );
        reactantEdges = GetReactantEdges( (IR_NODE*)reaction );
        /*
         *  reactant in this reaction is only this species
         */
        if( GetLinkedListSize( reactantEdges ) != 1 ) {
            return FALSE;
        }
        
        productEdges = GetProductEdges( (IR_NODE*)reaction );
        /*
         *  there is no product in this reaction
         */
        if( GetLinkedListSize( productEdges ) != 0 ) {
            return FALSE;
        }                
    
        stoichiometry = GetStoichiometryInIREdge( edge );
        if( stoichiometry != 1 ) {
            return FALSE;
        }
    }
    
    stoichiometry = -1;
    edges = GetProductEdges( (IR_NODE*)species );
    if( GetLinkedListSize( edges ) == 0 ) {
        return FALSE;
    }
    ResetCurrentElement( edges );
    while( ( edge = GetNextEdge( edges ) ) != NULL ) {
        reaction = GetReactionInIREdge( edge );
        productEdges = GetProductEdges( (IR_NODE*)reaction );
        /*
         *  product in this reaction is only this species
         */
        if( GetLinkedListSize( productEdges ) != 1 ) {
            return FALSE;
        }                
        
        reactantEdges = GetReactantEdges( (IR_NODE*)reaction );
        /*
         *  there is no reactant in this reaction
         */
        if( GetLinkedListSize( reactantEdges ) != 0 ) {
            return FALSE;
        }                    
        
        if( stoichiometry == -1 ) {
            stoichiometry = GetStoichiometryInIREdge( edge );
        }                    
        else {
            if( stoichiometry != GetStoichiometryInIREdge( edge ) ) {
                return FALSE;
            }
        }
    }
    
    return TRUE;
}

static RET_VAL _DoTransformation( ABSTRACTION_METHOD *method, IR *ir, SPECIES *species ) {
    RET_VAL ret = SUCCESS;
    double stoichiometry = (long)(method->_internal1);
    IR_EDGE *productionEdge = NULL;
    IR_EDGE *degradationEdge = NULL;    
    REACTION *production = NULL;
    REACTION *degradation = NULL;
    KINETIC_LAW *productionKineticLaw = NULL;
    KINETIC_LAW *degradationKineticLaw = NULL;
    LINKED_LIST *list = NULL;    
        
    if( IS_FAILED( ( ret = _CombineProductionReactions( method, ir, species ) ) ) ) {
        return ret;
    }
    if( IS_FAILED( ( ret = _CombineDegradationReactions( method, ir, species ) ) ) ) {
        return ret;
    }
    
    list = GetProductEdges( (IR_NODE*)species );
    productionEdge = GetHeadEdge( list );
    production = GetReactionInIREdge( productionEdge );
    productionKineticLaw = GetKineticLawInReactionNode( production );
        
    list = GetReactantEdges( (IR_NODE*)species );
    degradationEdge = GetHeadEdge( list );
    degradation = GetReactionInIREdge( degradationEdge );
    degradationKineticLaw = GetKineticLawInReactionNode( degradation );
    
    if( IS_FAILED( ( ret = _AdjustProductionKineticLaw( method, species, production, productionKineticLaw, degradationKineticLaw, stoichiometry ) ) ) ) {
        return ret;
    }
    if( IS_FAILED( ( ret = _AdjustDegradationKineticLaw( method, species, degradation, productionKineticLaw, degradationKineticLaw, stoichiometry ) ) ) ) {
        return ret;
    }
    if( IS_FAILED( ( ret = SetStoichiometryInIREdge( degradationEdge, stoichiometry ) ) ) ) {
        return ret;
    }
    
    if( IS_FAILED( ( ret = _AddModifiers( method, ir, production, degradation ) ) ) ) {
        return ret;
    }
    
    FreeKineticLaw( &degradationKineticLaw );
    FreeKineticLaw( &productionKineticLaw );
    
    return SUCCESS;                    
}

static RET_VAL _CombineDegradationReactions(ABSTRACTION_METHOD *method, IR *ir, SPECIES *species ) {
    RET_VAL ret = SUCCESS;
    double stoichiometry = 0;
    REACTION *newReaction = NULL;
    KINETIC_LAW *newKineticLaw = NULL;
    REACTION *reaction = NULL;
    SPECIES *modifier = NULL;
    KINETIC_LAW *kineticLaw = NULL;    
    IR_EDGE *edge = NULL;
    LINKED_LIST *edges = NULL;
    IR_EDGE *modifierEdge = NULL;
    LINKED_LIST *modifierEdges = NULL;
    
    edges = GetReactantEdges( (IR_NODE*)species );
    ResetCurrentElement( edges );
    edge = GetNextEdge( edges );
    newReaction = GetReactionInIREdge( edge );
    newKineticLaw = GetKineticLawInReactionNode( newReaction );
    
    while( ( edge = GetNextEdge( edges ) ) != NULL ) {
        reaction = GetReactionInIREdge( edge );
        kineticLaw = GetKineticLawInReactionNode( reaction );
        newKineticLaw = CreateOpKineticLaw( KINETIC_LAW_OP_PLUS, newKineticLaw, CloneKineticLaw( kineticLaw ) );
        modifierEdges = GetModifierEdges( (IR_NODE*)reaction );
        ResetCurrentElement( modifierEdges );
        while( ( modifierEdge = GetNextEdge( modifierEdges ) ) != NULL ) {
            modifier = GetSpeciesInIREdge( modifierEdge );
            stoichiometry = GetStoichiometryInIREdge( modifierEdge );
            if( IS_FAILED( ( ret = ir->AddModifierEdge( ir, newReaction, modifier, stoichiometry ) ) ) ) {
                return ret;
            }            
        }
    }
    ResetCurrentElement( edges );
    edge = GetNextEdge( edges );
#if 1    
    while( ( edge = GetNextEdge( edges ) ) != NULL ) {
        reaction = GetReactionInIREdge( edge );
        if( IS_FAILED( ( ret = ir->RemoveReaction( ir, reaction ) ) ) ) {
            return ret;
        }        
    }
#endif    
    if( IS_FAILED( ( ret = SetKineticLawInReactionNode( newReaction, newKineticLaw ) ) ) ) {
        return ret;
    }    
    return SUCCESS;        
}

static RET_VAL _CombineProductionReactions(ABSTRACTION_METHOD *method, IR *ir, SPECIES *species ) {
    RET_VAL ret = SUCCESS;
    double stoichiometry = 0;
    REACTION *newReaction = NULL;
    KINETIC_LAW *newKineticLaw = NULL;
    REACTION *reaction = NULL;
    SPECIES *modifier = NULL;
    KINETIC_LAW *kineticLaw = NULL;    
    IR_EDGE *edge = NULL;
    LINKED_LIST *edges = NULL;
    IR_EDGE *modifierEdge = NULL;
    LINKED_LIST *modifierEdges = NULL;
    
    edges = GetProductEdges( (IR_NODE*)species );
    ResetCurrentElement( edges );
    edge = GetNextEdge( edges );
    newReaction = GetReactionInIREdge( edge );
    newKineticLaw = GetKineticLawInReactionNode( newReaction );
    
    while( ( edge = GetNextEdge( edges ) ) != NULL ) {
        reaction = GetReactionInIREdge( edge );
        kineticLaw = GetKineticLawInReactionNode( reaction );
        newKineticLaw = CreateOpKineticLaw( KINETIC_LAW_OP_PLUS, newKineticLaw, CloneKineticLaw( kineticLaw ) );
        modifierEdges = GetModifierEdges( (IR_NODE*)reaction );
        ResetCurrentElement( modifierEdges );
        while( ( modifierEdge = GetNextEdge( modifierEdges ) ) != NULL ) {
            modifier = GetSpeciesInIREdge( modifierEdge );
            stoichiometry = GetStoichiometryInIREdge( modifierEdge );
            if( IS_FAILED( ( ret = ir->AddModifierEdge( ir, newReaction, modifier, stoichiometry ) ) ) ) {
                return ret;
            }            
        }
    }    
    ResetCurrentElement( edges );
    edge = GetNextEdge( edges );
#if 1    
    while( ( edge = GetNextEdge( edges ) ) != NULL ) {
        reaction = GetReactionInIREdge( edge );
        if( IS_FAILED( ( ret = ir->RemoveReaction( ir, reaction ) ) ) ) {
            return ret;
        }        
    }
#endif    
    
    if( IS_FAILED( ( ret = SetKineticLawInReactionNode( newReaction, newKineticLaw ) ) ) ) {
        return ret;
    }    
    return SUCCESS;        
}


static RET_VAL _AdjustProductionKineticLaw( 
ABSTRACTION_METHOD *method, SPECIES *species, REACTION *reaction, KINETIC_LAW *productionKineticLaw, 
KINETIC_LAW *degradationKineticLaw, double stoichiometry ) {
    RET_VAL ret = SUCCESS;
    double n = (double)(stoichiometry);
    KINETIC_LAW *fpgn = NULL;
    KINETIC_LAW *gn = NULL;
    KINETIC_LAW *temp1 = NULL;
    KINETIC_LAW *temp2 = NULL;
    KINETIC_LAW *temp3 = NULL;
    KINETIC_LAW *newKineticLaw = NULL;
    KINETIC_LAW *replacement = NULL;
    
    
    fpgn = CreateOpKineticLaw( KINETIC_LAW_OP_PLUS, CloneKineticLaw( productionKineticLaw ), CloneKineticLaw( degradationKineticLaw ) );
    fpgn = CreateOpKineticLaw( KINETIC_LAW_OP_POW, fpgn, CreateRealValueKineticLaw( n ) ); 
    
    gn = CreateOpKineticLaw( KINETIC_LAW_OP_POW, CloneKineticLaw( degradationKineticLaw ), CreateRealValueKineticLaw( n ) ); 
    
    temp1 = CreateOpKineticLaw( KINETIC_LAW_OP_MINUS, CloneKineticLaw( fpgn ), CloneKineticLaw( gn ) );
    temp1 = CreateOpKineticLaw( KINETIC_LAW_OP_TIMES, CloneKineticLaw( productionKineticLaw ), temp1 );
    
    temp2 = CreateOpKineticLaw( KINETIC_LAW_OP_MINUS, fpgn, gn );
    temp2 = CreateOpKineticLaw( KINETIC_LAW_OP_TIMES, CreateRealValueKineticLaw( 2.0 ), temp2 );
    temp3 = CreateOpKineticLaw( KINETIC_LAW_OP_POW, CloneKineticLaw( degradationKineticLaw ), CreateRealValueKineticLaw( n - 1.0 ) ); 
    temp3 = CreateOpKineticLaw( KINETIC_LAW_OP_TIMES, CloneKineticLaw( productionKineticLaw ), temp3 );
    temp3 = CreateOpKineticLaw( KINETIC_LAW_OP_TIMES, CreateRealValueKineticLaw( n - 2.0 ), temp3 );
    temp2 = CreateOpKineticLaw( KINETIC_LAW_OP_PLUS, temp2, temp3 );
    
    newKineticLaw = CreateOpKineticLaw( KINETIC_LAW_OP_DIVIDE, temp1, temp2 ); 
    replacement = _GenerateReplacementForProduction( method, species, stoichiometry );
    if( IS_FAILED( ( ret = ReplaceSpeciesWithKineticLawInKineticLaw( newKineticLaw, species, replacement ) ) ) ) {
        return ret;
    }
    
    FreeKineticLaw( &replacement );
    
    SetKineticLawInReactionNode( reaction, newKineticLaw );
            
    return SUCCESS;
}

static RET_VAL _AdjustDegradationKineticLaw( ABSTRACTION_METHOD *method, SPECIES *species, REACTION *reaction, KINETIC_LAW *productionKineticLaw, KINETIC_LAW *degradationKineticLaw, double stoichiometry ) {
    RET_VAL ret = SUCCESS;
    KINETIC_LAW *newKineticLaw = NULL;
    KINETIC_LAW *replacement = NULL;
    
    newKineticLaw = CreateOpKineticLaw( KINETIC_LAW_OP_DIVIDE, CloneKineticLaw( degradationKineticLaw ), CreateRealValueKineticLaw( (double)stoichiometry ) );
    
    replacement = _GenerateReplacementForDegradation( method, species, stoichiometry );
    if( IS_FAILED( ( ret = ReplaceSpeciesWithKineticLawInKineticLaw( newKineticLaw, species, replacement ) ) ) ) {
        return ret;
    }
    
    FreeKineticLaw( &replacement );
    
    SetKineticLawInReactionNode( reaction, newKineticLaw );
                     
    return SUCCESS;
}

static RET_VAL _AddModifiers( ABSTRACTION_METHOD *method, IR *ir, REACTION *production, REACTION *degradation ) {    
    RET_VAL ret = SUCCESS;
    int size = 0;
    int stoichiometry = 0;
    SPECIES *modifier = NULL;
    IR_EDGE *edge = NULL;
    LINKED_LIST *productionEdges = NULL;        
    LINKED_LIST *degradationEdges = NULL;        

    productionEdges = GetModifierEdges( (IR_NODE*)production );
    degradationEdges = GetModifierEdges( (IR_NODE*)degradation );
    
    size = GetLinkedListSize( degradationEdges );
    ResetCurrentElement( productionEdges );
    
    while( ( edge = GetNextEdge( productionEdges ) ) != NULL ) {
        modifier = GetSpeciesInIREdge( edge );
        stoichiometry = GetStoichiometryInIREdge( edge );
        if( IS_FAILED( ( ret = ir->AddModifierEdge( ir, degradation, modifier, stoichiometry ) ) ) ) {
            return ret;
        }     
    }     
        
    ResetCurrentElement( degradationEdges );
    while( ( edge = GetNextEdge( degradationEdges ) ) != NULL ) {
        modifier = GetSpeciesInIREdge( edge );
        stoichiometry = GetStoichiometryInIREdge( edge );
        if( IS_FAILED( ( ret = ir->AddModifierEdge( ir, production, modifier, stoichiometry ) ) ) ) {
            return ret;
        }     
        if( size == 1 ) {
            break;
        }
        size--;
    }     
    return SUCCESS;    
}


static KINETIC_LAW *_GenerateReplacementForProduction( 
ABSTRACTION_METHOD *method, SPECIES *species, double stoichiometry ) {
    KINETIC_LAW *replacement = NULL;
    
    replacement = CreateSpeciesKineticLaw( species );
#if 0    
    replacement = CreateOpKineticLaw( KINETIC_LAW_OP_PLUS, replacement, 
        CreateRealValueKineticLaw( (double)(stoichiometry>>1) ) );
#endif
    replacement = CreateOpKineticLaw( KINETIC_LAW_OP_MINUS, replacement, 
				      CreateRealValueKineticLaw( (double)( ((long)stoichiometry>>1) ) ) );
    return replacement;                      
    
}


static KINETIC_LAW *_GenerateReplacementForDegradation( 
ABSTRACTION_METHOD *method, SPECIES *species, double stoichiometry ) {
    KINETIC_LAW *replacement = NULL;
    
    replacement = CreateSpeciesKineticLaw( species );
    replacement = CreateOpKineticLaw( KINETIC_LAW_OP_MINUS, replacement, 
				      CreateRealValueKineticLaw( (double)( ((long)stoichiometry>>1) ) ) );
    return replacement;                          
}


static UINT32 _FindAmplifier( ABSTRACTION_METHOD_MANAGER *manager ) {
    char *valueString = NULL;
    UINT32 amplifier = DEFAULT_REB2SAC_STOICHIOMETRY_AMPLIFIER;
    COMPILER_RECORD_T *record = NULL;
    REB2SAC_PROPERTIES *properties = NULL;
    
    record = manager->GetCompilerRecord( manager );
    properties = record->properties;
    
    if( ( valueString = properties->GetProperty( properties, REB2SAC_DEGRADATION_STOICHIOMETRY_AMPLIFIER_KEY ) ) != NULL ) {
        if( IS_FAILED( StrToUINT32( &amplifier, valueString ) ) ) {
            return DEFAULT_REB2SAC_DEGRADATION_STOICHIOMETRY_AMPLIFIER;         
        }
        return amplifier;
    }
    
    return DEFAULT_REB2SAC_DEGRADATION_STOICHIOMETRY_AMPLIFIER;
}



