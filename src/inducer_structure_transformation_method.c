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
    SPECIES *inducer;
    SPECIES *repressor;
    LINKED_LIST *transcriptionReactions;           
} INDUCER_STRUCTURE_TRANSFORMATION_METHOD_INTERNAL_ELEMENT;

typedef struct {
    SPECIES *boundInducer;
    REACTION *inductionReaction;
    INDUCER_STRUCTURE_TRANSFORMATION_METHOD_INTERNAL_ELEMENT elements[2];           
} INDUCER_STRUCTURE_TRANSFORMATION_METHOD_INTERNAL;



static char * _GetInducerStructureTransformationMethodID( ABSTRACTION_METHOD *method );
static RET_VAL _ApplyInducerStructureTransformationMethod( ABSTRACTION_METHOD *method, IR *ir );      
static BOOL _IsConditionSatisfied( ABSTRACTION_METHOD *method, REACTION *reaction, INDUCER_STRUCTURE_TRANSFORMATION_METHOD_INTERNAL *internal ); 
static RET_VAL _DoTransformation( ABSTRACTION_METHOD *method, IR *ir, INDUCER_STRUCTURE_TRANSFORMATION_METHOD_INTERNAL *internal );

static BOOL _IsInductionReaction( ABSTRACTION_METHOD *method, REACTION *reaction, INDUCER_STRUCTURE_TRANSFORMATION_METHOD_INTERNAL *internal ); 
static BOOL _IsTranscriptionReaction( ABSTRACTION_METHOD *method, REACTION *reaction, INDUCER_STRUCTURE_TRANSFORMATION_METHOD_INTERNAL *internal ); 
static KINETIC_LAW *_GenerateReplacementKineticLaw( SPECIES *inducer, SPECIES *repressor, REACTION *reaction );
static RET_VAL _TransformTranscription( IR *ir, SPECIES *inducer, SPECIES *repressor, KINETIC_LAW *replacement, REACTION *transcription );

static RET_VAL _InitInducerStructureTransformationInternal( INDUCER_STRUCTURE_TRANSFORMATION_METHOD_INTERNAL *internal );
static RET_VAL _CleanInducerStructureTransformationInternal( INDUCER_STRUCTURE_TRANSFORMATION_METHOD_INTERNAL *internal );
static RET_VAL _FreeInducerStructureTransformationInternal( INDUCER_STRUCTURE_TRANSFORMATION_METHOD_INTERNAL *internal );


ABSTRACTION_METHOD *InducerStructureTransformationAbstractionMethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager ) {
    static ABSTRACTION_METHOD method;
    
    START_FUNCTION("InducerStructureTransformationAbstractionMethodConstructor");

    if( method.manager == NULL ) {
        method.manager = manager;
        method.GetID = _GetInducerStructureTransformationMethodID;
        method.Apply = _ApplyInducerStructureTransformationMethod;
    }
    
    TRACE_0( "InducerStructureTransformationAbstractionMethodConstructor invoked" );
    
    END_FUNCTION("InducerStructureTransformationAbstractionMethodConstructor", SUCCESS );
    return &method;
}



static char * _GetInducerStructureTransformationMethodID( ABSTRACTION_METHOD *method ) {
    START_FUNCTION("_GetInducerStructureTransformationMethodID");
    
    END_FUNCTION("_GetInducerStructureTransformationMethodID", SUCCESS );
    return "inducer-structure-transformer";
}



static RET_VAL _ApplyInducerStructureTransformationMethod( ABSTRACTION_METHOD *method, IR *ir ) {
    RET_VAL ret = SUCCESS;
    BOOL rnapFound = FALSE;
    REACTION *reaction = NULL;
    LINKED_LIST *reactionList = NULL;
    INDUCER_STRUCTURE_TRANSFORMATION_METHOD_INTERNAL internal;
    LINKED_LIST *list = NULL;
    
    START_FUNCTION("_ApplyInducerStructureTransformationMethod");
    
    reactionList = ir->GetListOfReactionNodes( ir );
    
    if( IS_FAILED( ( ret = _InitInducerStructureTransformationInternal( &internal ) ) ) ) {
        END_FUNCTION("_ApplyInducerStructureTransformationMethod", ret );
        return ret;
    }
    
    ResetCurrentElement( reactionList );        
    while( ( reaction = (REACTION*)GetNextFromLinkedList( reactionList ) ) != NULL ) {
        if( _IsConditionSatisfied( method, reaction, &internal ) ) {
            if( IS_FAILED( ( ret = _DoTransformation( method, ir, &internal ) ) ) ) {
                END_FUNCTION("_ApplyInducerStructureTransformationMethod", ret );
                return ret;
            }
        }            
        if( IS_FAILED( ( ret = _CleanInducerStructureTransformationInternal( &internal ) ) ) ) {
            END_FUNCTION("_ApplyInducerStructureTransformationMethod", ret );
            return ret;
        }
    }
    
    if( IS_FAILED( ( ret = _FreeInducerStructureTransformationInternal( &internal ) ) ) ) {
        END_FUNCTION("_ApplyInducerStructureTransformationMethod", ret );
        return ret;
    }
    END_FUNCTION("_ApplyInducerStructureTransformationMethod", SUCCESS );
    return ret;
}      


static BOOL _IsConditionSatisfied( ABSTRACTION_METHOD *method, REACTION *reaction, INDUCER_STRUCTURE_TRANSFORMATION_METHOD_INTERNAL *internal ) {
    BOOL transcriptionExist = FALSE;
    IR_EDGE *edge = NULL;
    SPECIES *inducer = NULL;
    SPECIES *repressor = NULL;
    REACTION *transcription = NULL;
    LINKED_LIST *list = NULL;
    LINKED_LIST *transcriptionReactions = NULL;
    
    START_FUNCTION("_IsConditionSatisfied");
    
    if( !_IsInductionReaction( method, reaction, internal ) ) {
        END_FUNCTION("_IsConditionSatisfied", SUCCESS );
        return FALSE;
    }
    
    repressor = internal->elements[0].repressor;
    transcriptionReactions = internal->elements[0].transcriptionReactions;
    list = GetModifierEdges( (IR_NODE*)repressor );
    ResetCurrentElement( list );
    while( ( edge = GetNextEdge( list ) ) != NULL ) {
        transcription = GetReactionInIREdge( edge );
        if( _IsTranscriptionReaction( method, transcription, internal ) ) {
            transcriptionExist = TRUE;
            if( IS_FAILED( AddElementInLinkedList( (CADDR_T)transcription, transcriptionReactions ) ) ) {
                END_FUNCTION("_IsConditionSatisfied", FAILING );
                return FALSE;
            } 
        }
    }
    
    repressor = internal->elements[1].repressor;
    transcriptionReactions = internal->elements[1].transcriptionReactions;
    list = GetModifierEdges( (IR_NODE*)repressor );
    ResetCurrentElement( list );
    while( ( edge = GetNextEdge( list ) ) != NULL ) {
        transcription = GetReactionInIREdge( edge );
        if( _IsTranscriptionReaction( method, transcription, internal ) ) {
            transcriptionExist = TRUE;
            if( IS_FAILED( AddElementInLinkedList( (CADDR_T)transcription, transcriptionReactions ) ) ) {
                END_FUNCTION("_IsConditionSatisfied", FAILING );
                return FALSE;
            } 
        }
    }
    
    if( !transcriptionExist ) {
        END_FUNCTION("_IsConditionSatisfied", SUCCESS );
        return FALSE;
    }

    END_FUNCTION("_IsConditionSatisfied", SUCCESS );
    return TRUE;
}

static BOOL _IsInductionReaction( ABSTRACTION_METHOD *method, REACTION *reaction, INDUCER_STRUCTURE_TRANSFORMATION_METHOD_INTERNAL *internal ) {
    SPECIES *species = NULL;
    IR_EDGE *edge = NULL;
    LINKED_LIST *list = NULL;
    
    START_FUNCTION("_IsInductionReaction");        

    if( !IsReactionReversibleInReactionNode( reaction ) ) {
        END_FUNCTION("_IsInductionReaction", SUCCESS );
        return FALSE;
    }

    list = GetProductEdges( (IR_NODE*)reaction );
    if( GetLinkedListSize( list ) != 1 ) {
        END_FUNCTION("_IsInductionReaction", SUCCESS );
        return FALSE;
    }
    
    edge = GetHeadEdge( list );
    species = GetSpeciesInIREdge( edge );
    list = GetProductEdges( (IR_NODE*)species );
    if( GetLinkedListSize( list ) != 1 ) {
        END_FUNCTION("_IsInductionReaction", SUCCESS );
        return FALSE;
    }
    
    list = GetReactantEdges( (IR_NODE*)species );
    if( GetLinkedListSize( list ) != 0 ) {
        END_FUNCTION("_IsInductionReaction", SUCCESS );
        return FALSE;
    }
    
    list = GetModifierEdges( (IR_NODE*)species );
    if( GetLinkedListSize( list ) != 0 ) {
        END_FUNCTION("_IsInductionReaction", SUCCESS );
        return FALSE;
    }
    
    internal->boundInducer = species;        
        
    list = GetReactantEdges( (IR_NODE*)reaction );
    if( GetLinkedListSize( list ) != 2 ) {
        END_FUNCTION("_IsInductionReaction", SUCCESS );
        return FALSE;
    }
    edge = GetHeadEdge( list );
    species = GetSpeciesInIREdge( edge );
    internal->elements[0].inducer = species;
    internal->elements[1].repressor = species;
    
    edge = GetTailEdge( list );
    species = GetSpeciesInIREdge( edge );
    internal->elements[1].inducer = species;
    internal->elements[0].repressor = species;
    
    internal->inductionReaction = reaction;
        
    END_FUNCTION("_IsInductionReaction", SUCCESS );
    return TRUE;
}

static BOOL _IsTranscriptionReaction( ABSTRACTION_METHOD *method, REACTION *reaction, INDUCER_STRUCTURE_TRANSFORMATION_METHOD_INTERNAL *internal ) {
    IR_EDGE *edge = NULL;
    SPECIES *species = NULL;
    LINKED_LIST *list = NULL;
    
    START_FUNCTION("_IsTranscriptionReaction");
    
    if( IsReactionReversibleInReactionNode( reaction ) ) {
        END_FUNCTION("_IsTranscriptionReaction", SUCCESS );
        return FALSE;
    }

    list = GetReactantEdges( (IR_NODE*)reaction );
    if( GetLinkedListSize( list ) != 0 ) {
        END_FUNCTION("_IsTranscriptionReaction", SUCCESS );
        return FALSE;
    }
    
    list = GetProductEdges( (IR_NODE*)reaction );
    if( GetLinkedListSize( list ) != 1 ) {
        END_FUNCTION("_IsTranscriptionReaction", SUCCESS );
        return FALSE;
    }
    
    END_FUNCTION("_IsTranscriptionReaction", SUCCESS );
    return TRUE;
}



static RET_VAL _DoTransformation( ABSTRACTION_METHOD *method, IR *ir, INDUCER_STRUCTURE_TRANSFORMATION_METHOD_INTERNAL *internal ) {
    RET_VAL ret = SUCCESS;
    SPECIES *inducer = NULL;
    SPECIES *repressor = NULL;
    SPECIES *boundInducer = NULL;
    KINETIC_LAW *replacement = NULL;
    REACTION *inductionReaction = NULL;
    REACTION *transcription = NULL;
    LINKED_LIST *transcriptionReactions = NULL;           
    
    START_FUNCTION("_DoTransformation");
    
    boundInducer = internal->boundInducer;
    inductionReaction = internal->inductionReaction;
    
    
    inducer = internal->elements[0].inducer;
    repressor = internal->elements[0].repressor;
    transcriptionReactions = internal->elements[0].transcriptionReactions;
    if( ( replacement = _GenerateReplacementKineticLaw( inducer, repressor, inductionReaction ) ) == NULL ) {
        return ErrorReport( FAILING, "_DoTransformation", "could not create a replacement for repressor %s", GetCharArrayOfString( GetSpeciesNodeName( repressor ) ) );
    }
    
    ResetCurrentElement( transcriptionReactions );    
    while( ( transcription = (REACTION*)GetNextFromLinkedList( transcriptionReactions ) ) != NULL ) {
        if( IS_FAILED( ( ret = _TransformTranscription( ir, inducer, repressor, replacement, transcription ) ) ) ) {
            END_FUNCTION("_DoTransformation", ret );
            return ret;
        }
    }
    FreeKineticLaw( &replacement );
    
    inducer = internal->elements[1].inducer;
    repressor = internal->elements[1].repressor;
    transcriptionReactions = internal->elements[1].transcriptionReactions;
    if( ( replacement = _GenerateReplacementKineticLaw( inducer, repressor, inductionReaction ) ) == NULL ) {
        return ErrorReport( FAILING, "_DoTransformation", "could not create a replacement for repressor %s", GetCharArrayOfString( GetSpeciesNodeName( repressor ) ) );
    }
    
    ResetCurrentElement( transcriptionReactions );    
    while( ( transcription = (REACTION*)GetNextFromLinkedList( transcriptionReactions ) ) != NULL ) {
        if( IS_FAILED( ( ret = _TransformTranscription( ir, inducer, repressor, replacement, transcription ) ) ) ) {
            END_FUNCTION("_DoTransformation", ret );
            return ret;
        }
    }
    FreeKineticLaw( &replacement );
    
    if( IS_FAILED( ( ret = ir->RemoveSpecies( ir, boundInducer ) ) ) ) {
        END_FUNCTION("_DoTransformation", ret );
        return ret;
    }
    
    if( IS_FAILED( ( ret = ir->RemoveReaction( ir, inductionReaction ) ) ) ) {
        END_FUNCTION("_DoTransformation", ret );
        return ret;
    }
    
        
    END_FUNCTION("_DoTransformation", SUCCESS );
    return ret;
}

static KINETIC_LAW *_GenerateReplacementKineticLaw( SPECIES *inducer, SPECIES *repressor, REACTION *reaction ) {
    KINETIC_LAW *replacement = NULL;
    KINETIC_LAW *equilibriumConstant = NULL;
    KINETIC_LAW *kineticLaw = NULL;
    
    START_FUNCTION("_TransformTranscription");
    
    kineticLaw = GetKineticLawInReactionNode( reaction );
    if( ( equilibriumConstant = CreateRateConstantRatioKineticLaw( kineticLaw ) ) == NULL ) {
        TRACE_0("error creating kinetic law" );        
        END_FUNCTION("_TransformTranscription", FAILING );
        return NULL;
    }
    
    if( ( replacement = CreateSpeciesKineticLaw( inducer ) ) == NULL ) {
        TRACE_0("error creating kinetic law" );        
        END_FUNCTION("_TransformTranscription", FAILING );
        return NULL;
    }
    if( ( replacement = CreateOpKineticLaw( KINETIC_LAW_OP_TIMES, equilibriumConstant, replacement ) ) == NULL ) {
        TRACE_0("error creating kinetic law" );        
        END_FUNCTION("_TransformTranscription", FAILING );
        return NULL;
    }
    if( ( replacement = CreateOpKineticLaw( KINETIC_LAW_OP_PLUS, CreateRealValueKineticLaw( 1.0 ), replacement ) ) == NULL ) {
        TRACE_0("error creating kinetic law" );        
        END_FUNCTION("_TransformTranscription", FAILING );
        return NULL;
    }
    if( ( replacement = CreateOpKineticLaw( KINETIC_LAW_OP_DIVIDE, CreateRealValueKineticLaw( 1.0 ), replacement ) ) == NULL ) {
        TRACE_0("error creating kinetic law" );        
        END_FUNCTION("_TransformTranscription", FAILING );
        return NULL;
    }
    if( ( replacement = CreateOpKineticLaw( KINETIC_LAW_OP_TIMES, replacement, CreateSpeciesKineticLaw( repressor ) ) ) == NULL ) {
        TRACE_0("error creating kinetic law" );        
        END_FUNCTION("_TransformTranscription", FAILING );
        return NULL;
    }
    
    END_FUNCTION("_TransformTranscription", SUCCESS );
    return replacement;
}


static RET_VAL _TransformTranscription( IR *ir, SPECIES *inducer, SPECIES *repressor, KINETIC_LAW *replacement, REACTION *transcription ) {
    RET_VAL ret = SUCCESS;
    KINETIC_LAW *kineticLaw = NULL;

    START_FUNCTION("_TransformTranscription");
    
    kineticLaw = GetKineticLawInReactionNode( transcription );
    if( IS_FAILED( ( ret = ReplaceSpeciesWithKineticLawInKineticLaw( kineticLaw, repressor, replacement ) ) ) ) {
        END_FUNCTION("_TransformTranscription", ret );
        return ret;
    }          
    
    if( IS_FAILED( ( ir->AddModifierEdge( ir, transcription, inducer, 1 ) ) ) ) {
        END_FUNCTION("_TransformTranscription", ret );
        return ret;
    }
    
    END_FUNCTION("_TransformTranscription", SUCCESS );
    return ret;
}


static RET_VAL _InitInducerStructureTransformationInternal( INDUCER_STRUCTURE_TRANSFORMATION_METHOD_INTERNAL *internal ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("_InitInducerStructureTransformationInternal");
        
    if( ( internal->elements[0].transcriptionReactions = CreateLinkedList() ) == NULL ) {
        return ErrorReport( FAILING, "_InitInducerStructureTransformationInternal", "could not create a reaction list" );
    }
        
    if( ( internal->elements[1].transcriptionReactions = CreateLinkedList() ) == NULL ) {
        return ErrorReport( FAILING, "_InitInducerStructureTransformationInternal", "could not create a reaction list" );
    }
    
    END_FUNCTION("_InitInducerStructureTransformationInternal", SUCCESS );
    return ret;
}

static RET_VAL _CleanInducerStructureTransformationInternal( INDUCER_STRUCTURE_TRANSFORMATION_METHOD_INTERNAL *internal ) {
    RET_VAL ret = SUCCESS;
    LINKED_LIST *list = NULL;
    START_FUNCTION("_CleanInducerStructureTransformationInternal");
    
    list = internal->elements[0].transcriptionReactions;
    if( GetLinkedListSize( list ) > 0 ) {
        DeleteLinkedList( &(list) );
        if( ( internal->elements[0].transcriptionReactions = CreateLinkedList() ) == NULL ) {
            return ErrorReport( FAILING, "_CleanInducerStructureTransformationInternal", "could not create a reaction list" );
        }
    }
    
    list = internal->elements[1].transcriptionReactions;
    if( GetLinkedListSize( list ) > 0 ) {
        DeleteLinkedList( &(list) );
        if( ( internal->elements[1].transcriptionReactions = CreateLinkedList() ) == NULL ) {
            return ErrorReport( FAILING, "_CleanInducerStructureTransformationInternal", "could not create a reaction list" );
        }
    }
    
    END_FUNCTION("_CleanInducerStructureTransformationInternal", SUCCESS );
    return ret;
}

static RET_VAL _FreeInducerStructureTransformationInternal( INDUCER_STRUCTURE_TRANSFORMATION_METHOD_INTERNAL *internal ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("_FreeInducerStructureTransformationInternal");
    
    DeleteLinkedList( &(internal->elements[0].transcriptionReactions) );
    DeleteLinkedList( &(internal->elements[1].transcriptionReactions) );
    
    END_FUNCTION("_FreeInducerStructureTransformationInternal", SUCCESS );
    return ret;
}




