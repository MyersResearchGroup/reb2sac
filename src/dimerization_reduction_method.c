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
    REACTION *reaction;
    SPECIES *monomer;
    SPECIES *dimer;    
    KINETIC_LAW *rateRatioKineticLaw;
} DIMERIZATION_REDUCTION_INTERNAL;


static char * _GetDimerizationReductionMethodID( ABSTRACTION_METHOD *method );
static RET_VAL _ApplyDimerizationReductionMethod( ABSTRACTION_METHOD *method, IR *ir );      
static BOOL _IsConditionSatisfied( ABSTRACTION_METHOD *method, REACTION *reaction, DIMERIZATION_REDUCTION_INTERNAL *internal ); 
static RET_VAL _DoTransformation( ABSTRACTION_METHOD *method, IR *ir, DIMERIZATION_REDUCTION_INTERNAL *internal );

static BOOL _IsDimerizationFormValid( KINETIC_LAW *kineticLaw, SPECIES *monomer, SPECIES *dimer );
static BOOL _FindMultiplicationOfSpecies( KINETIC_LAW *kineticLaw, SPECIES *species, double *result );
static RET_VAL _VisitOpToFindMultiplicationOfSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitIntToFindMultiplicationOfSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitRealToFindMultiplicationOfSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSpeciesToFindMultiplicationOfSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSymbolToFindMultiplicationOfSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );

static KINETIC_LAW *_CreateReplacementWithMonomer( SPECIES *monomer );
static KINETIC_LAW *_CreateReplacementWithMonomerAndRateRatioKineticLaw( SPECIES *monomer, KINETIC_LAW *rateRatio );

static KINETIC_LAW *_CreateSquareRootPart( SPECIES *totalSpecies, KINETIC_LAW *rateRatio );
static KINETIC_LAW *_CreateReplacementForMonomer( SPECIES *totalSpecies, KINETIC_LAW *rateRatio, KINETIC_LAW *rootPart );
static KINETIC_LAW *_CreateReplacementForDimer( SPECIES *totalSpecies, KINETIC_LAW *rateRatio, KINETIC_LAW *rootPart );


ABSTRACTION_METHOD *DimerizationReductionMethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager ) {
    static ABSTRACTION_METHOD method;
    
    START_FUNCTION("DimerizationReductionMethodConstructor");

    if( method.manager == NULL ) {
        method.manager = manager;
        method.GetID = _GetDimerizationReductionMethodID;
        method.Apply = _ApplyDimerizationReductionMethod;
    }
    
    TRACE_0( "DimerizationReductionMethodConstructor invoked" );
    
    END_FUNCTION("DimerizationReductionMethodConstructor", SUCCESS );
    return &method;
}



static char * _GetDimerizationReductionMethodID( ABSTRACTION_METHOD *method ) {
    START_FUNCTION("_GetDimerizationReductionMethodID");
    
    END_FUNCTION("_GetDimerizationReductionMethodID", SUCCESS );
    return "dimerization-reduction";
}



static RET_VAL _ApplyDimerizationReductionMethod( ABSTRACTION_METHOD *method, IR *ir ) {
    RET_VAL ret = SUCCESS;
    REACTION *reaction = NULL;
    LINKED_LIST *reactionList = NULL;
    DIMERIZATION_REDUCTION_INTERNAL internal;
    
    START_FUNCTION("_ApplyDimerizationReductionMethod");
    
    reactionList = ir->GetListOfReactionNodes( ir );    
    ResetCurrentElement( reactionList );    
    while( ( reaction = (REACTION*)GetNextFromLinkedList( reactionList ) ) != NULL ) {
        if( _IsConditionSatisfied( method, reaction, &internal ) ) {
            if( IS_FAILED( ( ret = _DoTransformation( method, ir, &internal ) ) ) ) {
                END_FUNCTION("_ApplyDimerizationReductionMethod", ret );
                return ret;
            }
        }            
    }
            
    END_FUNCTION("_ApplyDimerizationReductionMethod", SUCCESS );
    return ret;
}      



static BOOL _IsConditionSatisfied( ABSTRACTION_METHOD *method, REACTION *reaction, DIMERIZATION_REDUCTION_INTERNAL *internal ) {
    IR_EDGE *edge = NULL;
    SPECIES *monomer = NULL;
    SPECIES *dimer = NULL;
    KINETIC_LAW *kineticLaw = NULL;
    LINKED_LIST *list = NULL;
#ifdef DEBUG
    STRING *string = NULL;
#endif
    
    START_FUNCTION("_IsConditionSatisfied");
    
    /*
    * reaction R, which is used to transform monomer to dimer, is a reversible reaction
    */
    if( !IsReactionReversibleInReactionNode( reaction ) ) {
        END_FUNCTION("_IsConditionSatisfied", SUCCESS );
        return FALSE;
    }
    
    /*
    * R only has one reactant
    */
    list = GetReactantEdges( (IR_NODE*)reaction );
    if( GetLinkedListSize( list ) != 1 ) {
        END_FUNCTION("_IsConditionSatisfied", SUCCESS );
        return FALSE;
    }
    edge = GetHeadEdge( list );
    if( GetStoichiometryInIREdge( edge ) != 2 ) {
        END_FUNCTION("_IsConditionSatisfied", SUCCESS );
        return FALSE;
    }
    monomer = GetSpeciesInIREdge( edge );
    
    /*
    * monomer M, which is the reactant of R, is not used as a modifier
    */
    list = GetModifierEdges( (IR_NODE*)monomer );
    if( GetLinkedListSize( list ) > 0 ) {
        END_FUNCTION("_IsConditionSatisfied", SUCCESS );
        return FALSE;
    }
    /*
    * R only has one product
    */
    list = GetProductEdges( (IR_NODE*)reaction );
    if( GetLinkedListSize( list ) != 1 ) {
        END_FUNCTION("_IsConditionSatisfied", SUCCESS );
        return FALSE;
    }
    edge = GetHeadEdge( list );
    if( GetStoichiometryInIREdge( edge ) != 1 ) {
        END_FUNCTION("_IsConditionSatisfied", SUCCESS );
        return FALSE;
    }
    
    dimer = GetSpeciesInIREdge( edge );
    
    /*
    * dimer D, which is the product of R, is produced only in R
    */
    list = GetProductEdges( (IR_NODE*)dimer );
    if( GetLinkedListSize( list ) != 1 ) {
        END_FUNCTION("_IsConditionSatisfied", SUCCESS );
        return FALSE;
    }
    
    kineticLaw = GetKineticLawInReactionNode( reaction );        
    if( !_IsDimerizationFormValid( kineticLaw, monomer, dimer ) ) {
        END_FUNCTION("_IsConditionSatisfied", SUCCESS );
        return FALSE;    
    }
    if( ( internal->rateRatioKineticLaw = CreateDissociationConstantRatioKineticLaw( kineticLaw ) ) == NULL ) {
        END_FUNCTION("_IsConditionSatisfied", SUCCESS );
        return FALSE;    
    } 
    
    internal->reaction = reaction;
    internal->monomer = monomer;
    internal->dimer = dimer;
    TRACE_1("reaction %s satisfies the condition", GetCharArrayOfString( GetReactionNodeName( internal->reaction ) ) );
    TRACE_1("\tmonomer is %s", GetCharArrayOfString( GetSpeciesNodeName( internal->monomer ) ) );
    TRACE_1("\tdimer is %s", GetCharArrayOfString( GetSpeciesNodeName( internal->dimer ) ) );

#ifdef DEBUG
    string = ToStringKineticLaw( internal->rateRatioKineticLaw );
    printf( "\trate ratio in kinetic law is %s" NEW_LINE, GetCharArrayOfString( string ) );
    FreeString( &string );
#endif
        
    END_FUNCTION("_IsConditionSatisfied", SUCCESS );
    return TRUE;
}


static RET_VAL _DoTransformation( ABSTRACTION_METHOD *method, IR *ir, DIMERIZATION_REDUCTION_INTERNAL *internal ) {
    RET_VAL ret = SUCCESS;
    SPECIES *monomer = NULL;
    SPECIES *dimer = NULL;
    IR_EDGE *edge = NULL;
    KINETIC_LAW *monomerReplacement = NULL;
    KINETIC_LAW *dimerReplacement = NULL;
    KINETIC_LAW *rateRatio = NULL;
    KINETIC_LAW *rootPart = NULL;
    
    KINETIC_LAW *kineticLaw = NULL;
    REACTION *reaction = NULL;
    LINKED_LIST *list = NULL;
#ifdef DEBUG
    STRING *string = NULL;
#endif    
    START_FUNCTION("_DoTransformation");
    
    if( IS_FAILED( ( ret = ir->RemoveReaction( ir, internal->reaction ) ) ) ) {
        END_FUNCTION("_DoTransformation", ret );
        return ret;
    } 
   
    monomer = internal->monomer;
    dimer = internal->dimer;
    rateRatio = internal->rateRatioKineticLaw;
        
    if( ( rootPart = _CreateSquareRootPart( monomer, rateRatio ) ) == NULL ) {        
        return ErrorReport( FAILING, "_DoTransformation", "could not create a replacement with %s", GetCharArrayOfString( GetSpeciesNodeName( monomer ) ) );
    }
    if( ( monomerReplacement = _CreateReplacementForMonomer( monomer, rateRatio, rootPart ) ) == NULL ) {        
        return ErrorReport( FAILING, "_DoTransformation", "could not create a replacement with %s", GetCharArrayOfString( GetSpeciesNodeName( monomer ) ) );
    }
    if( ( dimerReplacement = _CreateReplacementForDimer( monomer, rateRatio, rootPart ) ) == NULL ) {        
        return ErrorReport( FAILING, "_DoTransformation", "could not create a replacement with %s", GetCharArrayOfString( GetSpeciesNodeName( monomer ) ) );
    }
    

    list = GetReactantEdges( (IR_NODE*)monomer );
    ResetCurrentElement( list );
    while( ( edge = GetNextEdge( list ) ) != NULL ) {
        reaction = GetReactionInIREdge( edge );
        kineticLaw = GetKineticLawInReactionNode( reaction );
        if( IS_FAILED( ( ret = ReplaceSpeciesWithKineticLawInKineticLaw( kineticLaw, monomer, monomerReplacement ) ) ) ) {
            END_FUNCTION("_DoTransformation", ret );
            return ret;
        }
    }
    list = GetProductEdges( (IR_NODE*)monomer );
    ResetCurrentElement( list );
    while( ( edge = GetNextEdge( list ) ) != NULL ) {
        reaction = GetReactionInIREdge( edge );
        kineticLaw = GetKineticLawInReactionNode( reaction );
        if( IS_FAILED( ( ret = ReplaceSpeciesWithKineticLawInKineticLaw( kineticLaw, monomer, monomerReplacement ) ) ) ) {
            END_FUNCTION("_DoTransformation", ret );
            return ret;
        }
    }
                   
    list = GetModifierEdges( (IR_NODE*)dimer );
    ResetCurrentElement( list );
    while( ( edge = GetNextEdge( list ) ) != NULL ) {
        reaction = GetReactionInIREdge( edge );
        kineticLaw = GetKineticLawInReactionNode( reaction );
        if( IS_FAILED( ( ret = ReplaceSpeciesWithKineticLawInKineticLaw( kineticLaw, dimer, dimerReplacement ) ) ) ) {
            END_FUNCTION("_DoTransformation", ret );
            return ret;
        }
        /*
        since we use an artifitial species S_tot, and substitute monomer and dimer with this species,
        the stoichiometries should be 1
        */
        if( IS_FAILED( ( ret = ir->AddModifierEdge( ir, reaction, monomer, 1 ) ) ) ) {
            END_FUNCTION("_DoTransformation", ret );
            return ret;
        }
#ifdef DEBUG
        string = ToStringKineticLaw( kineticLaw );
        printf("new kinetic law of %s is %s" NEW_LINE, GetCharArrayOfString( GetReactionNodeName( reaction ) ), GetCharArrayOfString( string ) );
        FreeString( &string );
#endif        
    }
    
    list = GetReactantEdges( (IR_NODE*)dimer );
    ResetCurrentElement( list );
    while( ( edge = GetNextEdge( list ) ) != NULL ) {
        reaction = GetReactionInIREdge( edge );
        kineticLaw = GetKineticLawInReactionNode( reaction );
        if( IS_FAILED( ( ret = ReplaceSpeciesWithKineticLawInKineticLaw( kineticLaw, dimer, dimerReplacement ) ) ) ) {
            END_FUNCTION("_DoTransformation", ret );
            return ret;
        } 
        if( IS_FAILED( ( ret = ir->AddReactantEdge( ir, reaction, monomer, 1 ) ) ) ) {
            END_FUNCTION("_DoTransformation", ret );
            return ret;
        }        
#ifdef DEBUG
        string = ToStringKineticLaw( kineticLaw );
        printf("new kinetic law of %s is %s" NEW_LINE, GetCharArrayOfString( GetReactionNodeName( reaction ) ), GetCharArrayOfString( string ) );
        FreeString( &string );
#endif        
    }
    
    if( IS_FAILED( ( ret = ir->RemoveSpecies( ir, dimer ) ) ) ) {
        END_FUNCTION("_DoTransformation", ret );
        return ret;
    }
    
    FreeKineticLaw( &(internal->rateRatioKineticLaw) );
    FreeKineticLaw( &rootPart );
    FreeKineticLaw( &monomerReplacement );
    FreeKineticLaw( &dimerReplacement );
    
    END_FUNCTION("_DoTransformation", SUCCESS );
    return ret;
}



static BOOL _IsDimerizationFormValid( KINETIC_LAW *kineticLaw, SPECIES *monomer, SPECIES *dimer ) {
    RET_VAL ret = SUCCESS;
    BYTE opType = 0x00;
    double multiplication = 0.0;
    KINETIC_LAW *left = NULL;
    KINETIC_LAW *right = NULL;
    
    START_FUNCTION("_IsDimerizationFormValid");
    
    if( !IsOpKineticLaw( kineticLaw ) ) {
        END_FUNCTION("_IsDimerizationFormValid", SUCCESS );
        return FALSE;
    }
    
    opType = GetOpTypeFromKineticLaw( kineticLaw );
    if( opType != KINETIC_LAW_OP_MINUS ) {
        END_FUNCTION("_IsDimerizationFormValid", SUCCESS );
        return FALSE;
    }
    
    left = GetOpLeftFromKineticLaw( kineticLaw );
    
    if( !_FindMultiplicationOfSpecies( left, monomer, &multiplication ) ) {
        END_FUNCTION("_IsDimerizationFormValid", SUCCESS );
        return FALSE;
    }
    if( !IS_REAL_EQUAL( multiplication, 2.0 ) ) {
        END_FUNCTION("_IsDimerizationFormValid", SUCCESS );
        return FALSE;
    }
    
    right = GetOpRightFromKineticLaw( kineticLaw );
    if( !_FindMultiplicationOfSpecies( right, dimer, &multiplication ) ) {
        END_FUNCTION("_IsDimerizationFormValid", SUCCESS );
        return FALSE;
    }
    if( !IS_REAL_EQUAL( multiplication, 1.0 ) ) {
        END_FUNCTION("_IsDimerizationFormValid", SUCCESS );
        return FALSE;
    }
    
    END_FUNCTION("_IsDimerizationFormValid", SUCCESS );
    return TRUE;
}


static BOOL _FindMultiplicationOfSpecies( KINETIC_LAW *kineticLaw, SPECIES *species, double *result ) {
    static KINETIC_LAW_VISITOR visitor;
#ifdef DEBUG    
    STRING *string = NULL;
#endif
    
    START_FUNCTION("_FindMultiplicationOfSpecies");
    
    if( visitor.VisitOp == NULL ) {
        visitor.VisitOp = _VisitOpToFindMultiplicationOfSpecies;
        visitor.VisitInt = _VisitIntToFindMultiplicationOfSpecies;
        visitor.VisitReal = _VisitRealToFindMultiplicationOfSpecies;
        visitor.VisitSpecies = _VisitSpeciesToFindMultiplicationOfSpecies;
        visitor.VisitSymbol = _VisitSymbolToFindMultiplicationOfSpecies;
    }
    *result = 0.0;
    
    visitor._internal1 = (CADDR_T)species;
    visitor._internal2 = (CADDR_T)result;
    
    if( IS_FAILED( kineticLaw->Accept( kineticLaw, &visitor ) ) ) {
        END_FUNCTION("_FindSpecies", SUCCESS );
        return FALSE;
    } 
#ifdef DEBUG    
    string = ToStringKineticLaw( kineticLaw );
    printf( "the multiplication of %s found in %s is %g" NEW_LINE, GetCharArrayOfString( GetSpeciesNodeName( species ) ), GetCharArrayOfString( string ), *result ); 
    FreeString( &string );
#endif
    
    END_FUNCTION("_FindMultiplicationOfSpecies", SUCCESS );
    return TRUE;
}

static RET_VAL _VisitOpToFindMultiplicationOfSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    BYTE opType = 0x00;
    double *resultSave = NULL;
    double temp = 0.0;
    KINETIC_LAW *left = NULL;
    KINETIC_LAW *right = NULL;
    
    START_FUNCTION("_VisitOpToFindMultiplicationOfSpecies");
    
    opType = GetOpTypeFromKineticLaw( kineticLaw );
    left = GetOpLeftFromKineticLaw( kineticLaw );
    right = GetOpRightFromKineticLaw( kineticLaw );
    
    if( opType == KINETIC_LAW_OP_TIMES ) {
        if( IS_FAILED( ( ret = left->Accept( left, visitor ) ) ) ) {
            END_FUNCTION("_VisitOpToFindMultiplicationOfSpecies", ret );
            return ret;
        }
        if( IS_FAILED( ( ret = right->Accept( right, visitor ) ) ) ) {
            END_FUNCTION("_VisitOpToFindMultiplicationOfSpecies", ret );
            return ret;
        }
    }
    else if( opType == KINETIC_LAW_OP_POW ) {
        resultSave = (double*)(visitor->_internal2);
        temp = 0.0;
        visitor->_internal2 = (CADDR_T)(&temp);
        if( IS_FAILED( ( ret = left->Accept( left, visitor ) ) ) ) {
            END_FUNCTION("_VisitOpToFindMultiplicationOfSpecies", ret );
            return ret;
        }
        if( IsRealValueKineticLaw( right ) ) {
            temp *= GetRealValueFromKineticLaw( right );
        }
        else if( IsIntValueKineticLaw( right ) ) {
            temp *= (double)GetIntValueFromKineticLaw( right );
        }
        else {
            END_FUNCTION("_VisitOpToFindMultiplicationOfSpecies", E_WRONGDATA );
            return E_WRONGDATA;
        }
        *resultSave = (*resultSave) + temp;
        visitor->_internal2 = (CADDR_T)resultSave;
    }
    else {
        END_FUNCTION("_VisitOpToFindMultiplicationOfSpecies", E_WRONGDATA );
        return E_WRONGDATA;
    }
    
    END_FUNCTION("_VisitOpToFindMultiplicationOfSpecies", SUCCESS );
    return ret;
}

static RET_VAL _VisitIntToFindMultiplicationOfSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    START_FUNCTION("_VisitIntToFindMultiplicationOfSpecies");
    
    END_FUNCTION("_VisitIntToFindMultiplicationOfSpecies", SUCCESS );
    return ret;
}

static RET_VAL _VisitRealToFindMultiplicationOfSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    START_FUNCTION("_VisitRealToFindMultiplicationOfSpecies");
    
    END_FUNCTION("_VisitRealToFindMultiplicationOfSpecies", SUCCESS );
    return ret;
}


static RET_VAL _VisitSymbolToFindMultiplicationOfSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    START_FUNCTION("_VisitSymbolToFindMultiplicationOfSpecies");
    
    END_FUNCTION("_VisitSymbolToFindMultiplicationOfSpecies", SUCCESS );
    return ret;
}

static RET_VAL _VisitSpeciesToFindMultiplicationOfSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    double *result = NULL;
    SPECIES *species = NULL;
    SPECIES *target = NULL;
    
    START_FUNCTION("_VisitSpeciesToFindMultiplicationOfSpecies");
    
    species = GetSpeciesFromKineticLaw( kineticLaw );
    target = (SPECIES*)(visitor->_internal1);
    
    if( species == target ) {
        result = (double*)(visitor->_internal2);
        *result = *result + 1.0;
    }
    
    END_FUNCTION("_VisitSpeciesToFindMultiplicationOfSpecies", SUCCESS );
    return ret;
}



static KINETIC_LAW *_CreateReplacementWithMonomer( SPECIES *monomer ) {
    KINETIC_LAW *kineticLaw = NULL;
    
    START_FUNCTION("_CreateReplacementWithMonomer");
    
    if( ( kineticLaw = CreateOpKineticLaw( KINETIC_LAW_OP_TIMES, 
            CreateRealValueKineticLaw( 0.5 ), CreateSpeciesKineticLaw( monomer ) ) ) == NULL ) {
        END_FUNCTION("_CreateReplacementWithMonomer", FAILING );
        return NULL;
    }
    
    END_FUNCTION("_CreateReplacementWithMonomer", SUCCESS );
    return kineticLaw;
}

static KINETIC_LAW *_CreateReplacementWithMonomerAndRateRatioKineticLaw( SPECIES *monomer, KINETIC_LAW *rateRatio ) {
    KINETIC_LAW *kineticLaw = NULL;
    
    START_FUNCTION("_CreateReplacementWithMonomerAndRateRatioKineticLaw");

    if( ( kineticLaw = CreateOpKineticLaw( KINETIC_LAW_OP_POW, CreateSpeciesKineticLaw( monomer ), CreateRealValueKineticLaw( 2.0 ) ) ) == NULL ) {
        END_FUNCTION("_CreateReplacementWithMonomerAndRateRatioKineticLaw", FAILING );
        return NULL;
    }
    if( ( kineticLaw = CreateOpKineticLaw( KINETIC_LAW_OP_TIMES, CloneKineticLaw( rateRatio ), kineticLaw ) ) == NULL ) {
        END_FUNCTION("_CreateReplacementWithMonomerAndRateRatioKineticLaw", FAILING );
        return NULL;
    }
    
    END_FUNCTION("_CreateReplacementWithMonomerAndRateRatioKineticLaw", SUCCESS );
    return kineticLaw;
}


static KINETIC_LAW *_CreateSquareRootPart( SPECIES *totalSpecies, KINETIC_LAW *rateRatio ) {
    KINETIC_LAW *kineticLaw = NULL;
    KINETIC_LAW *temp1 = NULL;
    KINETIC_LAW *temp2 = NULL;
        
    if( ( temp1 = CreateOpKineticLaw( KINETIC_LAW_OP_TIMES, CreateRealValueKineticLaw( 8.0  ), CloneKineticLaw( rateRatio ) ) ) == NULL ) {
        return NULL;
    }        
    if( ( temp1 = CreateOpKineticLaw( KINETIC_LAW_OP_TIMES, temp1, CreateSpeciesKineticLaw( totalSpecies ) ) ) == NULL ) {
        return NULL;
    }        
    if( ( temp2 = CreateOpKineticLaw( KINETIC_LAW_OP_POW, CloneKineticLaw( rateRatio ), CreateRealValueKineticLaw( 2.0 ) ) ) == NULL ) {
        return NULL;
    }
    if( ( kineticLaw = CreateOpKineticLaw( KINETIC_LAW_OP_PLUS, temp1, temp2 ) ) == NULL ) {
        return NULL;
    }
    if( ( kineticLaw = CreateOpKineticLaw( KINETIC_LAW_OP_POW, kineticLaw, CreateRealValueKineticLaw( 0.5 )  ) ) == NULL ) {
        return NULL;
    }    
    return kineticLaw;    
}


static KINETIC_LAW *_CreateReplacementForMonomer( SPECIES *totalSpecies, KINETIC_LAW *rateRatio, KINETIC_LAW *rootPart ) {
    KINETIC_LAW *kineticLaw = NULL;
/*    
    if( ( kineticLaw = CreateOpKineticLaw( KINETIC_LAW_OP_MINUS, CloneKineticLaw( rateRatio ), CloneKineticLaw( rootPart ) ) ) == NULL ) {
        return NULL;
    } 
*/           
    if( ( kineticLaw = CreateOpKineticLaw( KINETIC_LAW_OP_MINUS, CloneKineticLaw( rootPart ), CloneKineticLaw( rateRatio )  ) ) == NULL ) {
        return NULL;
    }        
    if( ( kineticLaw = CreateOpKineticLaw( KINETIC_LAW_OP_DIVIDE, kineticLaw, CreateRealValueKineticLaw( 4.0 ) ) ) == NULL ) {
        return NULL;
    }        
    
#if 0
/*add this to make it non-negative*/    
    if( ( kineticLaw = CreateOpKineticLaw( KINETIC_LAW_OP_POW, kineticLaw, CreateRealValueKineticLaw( 2.0 )  ) ) == NULL ) {
        return NULL;
    }    
    if( ( kineticLaw = CreateOpKineticLaw( KINETIC_LAW_OP_POW, kineticLaw, CreateRealValueKineticLaw( 0.5 )  ) ) == NULL ) {
        return NULL;
    }    
#endif
    
    return kineticLaw;    
}

static KINETIC_LAW *_CreateReplacementForDimer( SPECIES *totalSpecies, KINETIC_LAW *rateRatio, KINETIC_LAW *rootPart ) {
    KINETIC_LAW *kineticLaw = NULL;
    KINETIC_LAW *temp1 = NULL;
    KINETIC_LAW *temp2 = NULL;

    if( ( temp1 = CreateOpKineticLaw( KINETIC_LAW_OP_DIVIDE, CreateSpeciesKineticLaw( totalSpecies ), CreateRealValueKineticLaw( 2.0 ) ) ) == NULL ) {
        return NULL;
    }        
    if( ( temp2 = CreateOpKineticLaw( KINETIC_LAW_OP_MINUS, CloneKineticLaw( rateRatio ), CloneKineticLaw( rootPart ) ) ) == NULL ) {
        return NULL;
    }        
    if( ( temp2 = CreateOpKineticLaw( KINETIC_LAW_OP_DIVIDE, temp2, CreateRealValueKineticLaw( 8.0 ) ) ) == NULL ) {
        return NULL;
    }        
    if( ( kineticLaw = CreateOpKineticLaw( KINETIC_LAW_OP_PLUS, temp1, temp2 ) ) == NULL ) {
        return NULL;
    }    
    
#if 0
/*add this to make it non-negative*/    
    if( ( kineticLaw = CreateOpKineticLaw( KINETIC_LAW_OP_POW, kineticLaw, CreateRealValueKineticLaw( 2.0 )  ) ) == NULL ) {
        return NULL;
    }    
    if( ( kineticLaw = CreateOpKineticLaw( KINETIC_LAW_OP_POW, kineticLaw, CreateRealValueKineticLaw( 0.5 )  ) ) == NULL ) {
        return NULL;
    }    
#endif
    
    return kineticLaw;    
}




 
 
