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
#include "common.h"
#include "linked_list.h"
#include "abstraction_method_manager.h"
#include "IR.h"
#include "species_node.h"
#include "reaction_node.h"
#include "kinetic_law.h"

typedef struct {
    LINKED_LIST *irrelevantSpeciesList;
    LINKED_LIST *irrelevantReactionList;
} IRRELEVANT_SPECIES_ELIMINATION_INTERNAL;

static char * _GetIrrelevantSpeciesEliminationMethodID( ABSTRACTION_METHOD *method );
static RET_VAL _ApplyIrrelevantSpeciesEliminationMethod( ABSTRACTION_METHOD *method, IR *ir );      

static LINKED_LIST *_CreateListOfInterestingSpecies( ABSTRACTION_METHOD *method, IR *ir );

static RET_VAL _InitInternal( IR *ir, IRRELEVANT_SPECIES_ELIMINATION_INTERNAL *internal );
static RET_VAL _CleanInternal( IRRELEVANT_SPECIES_ELIMINATION_INTERNAL *internal );
static RET_VAL _FindIrrelevant( IR *ir, IRRELEVANT_SPECIES_ELIMINATION_INTERNAL *internal );
static RET_VAL _RemoveIrrelevant( IR *ir, IRRELEVANT_SPECIES_ELIMINATION_INTERNAL *internal );

static RET_VAL _SetNodeFlagsToWhite( IR *ir );

static RET_VAL _WalkSpecies( ABSTRACTION_METHOD *method, SPECIES *species );      
static RET_VAL _WalkReaction( ABSTRACTION_METHOD *method, REACTION *reaction );      


ABSTRACTION_METHOD *IrrelevantSpeciesEliminationMethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager ) {
    static ABSTRACTION_METHOD method;
    
    START_FUNCTION("IrrelevantSpeciesEliminationMethodConstructor");

    if( method.manager == NULL ) {
        method.manager = manager;
        method.GetID = _GetIrrelevantSpeciesEliminationMethodID;
        method.Apply = _ApplyIrrelevantSpeciesEliminationMethod;       
    }
    
    TRACE_0( "IrrelevantSpeciesEliminationMethodConstructor invoked" );
    
    END_FUNCTION("IrrelevantSpeciesEliminationMethodConstructor", SUCCESS );
    return &method;
}



static char * _GetIrrelevantSpeciesEliminationMethodID( ABSTRACTION_METHOD *method ) {
    START_FUNCTION("_GetIrrelevantSpeciesEliminationMethodID");
    
    END_FUNCTION("_GetIrrelevantSpeciesEliminationMethodID", SUCCESS );
    return "irrelevant-species-remover";
}



static RET_VAL _ApplyIrrelevantSpeciesEliminationMethod( ABSTRACTION_METHOD *method, IR *ir ) {
    RET_VAL ret = SUCCESS;
    SPECIES *species = NULL;
    LINKED_LIST *speciesList = NULL;
    IRRELEVANT_SPECIES_ELIMINATION_INTERNAL internal;
    
    START_FUNCTION("_ApplyIrrelevantSpeciesEliminationMethod");
        
    if( ( speciesList = _CreateListOfInterestingSpecies( method, ir ) ) == NULL ) {
        END_FUNCTION("_ApplyIrrelevantSpeciesEliminationMethod", ret );
        return ret;
    }
    
    if( GetLinkedListSize( speciesList ) == 0 ) {
        DeleteLinkedList( &speciesList );    
        END_FUNCTION("_ApplyIrrelevantSpeciesEliminationMethod", SUCCESS );
        return ret;
    }
    
    if( IS_FAILED( ( ret = _InitInternal( ir, &internal ) ) ) ) {
        END_FUNCTION("_ApplyIrrelevantSpeciesEliminationMethod", ret );
        return ret;
    }
    
    ResetCurrentElement( speciesList );
    while( ( species = (SPECIES*)GetNextFromLinkedList( speciesList ) ) != NULL ) {
        if( IS_FAILED( ( ret = _SetNodeFlagsToWhite( ir ) ) ) ) {
            END_FUNCTION("_ApplyIrrelevantSpeciesEliminationMethod", ret );
            return ret;
        }
        if( IS_FAILED( ( ret = _WalkSpecies( method, species ) ) ) ) {
            END_FUNCTION("_ApplyIrrelevantSpeciesEliminationMethod", ret );
            return ret;
        }
        if( IS_FAILED( ( ret = _FindIrrelevant( ir, &internal ) ) ) ) {
            END_FUNCTION("_ApplyIrrelevantSpeciesEliminationMethod", ret );
            return ret;
        }
    }

    if( IS_FAILED( ( ret = _RemoveIrrelevant( ir, &internal ) ) ) ) {
        END_FUNCTION("_ApplyIrrelevantSpeciesEliminationMethod", ret );
        return ret;
    }
    
    if( IS_FAILED( ( ret = _SetNodeFlagsToWhite( ir ) ) ) ) {
        END_FUNCTION("_ApplyIrrelevantSpeciesEliminationMethod", ret );
        return ret;
    }
    
    DeleteLinkedList( &speciesList );    
    if( IS_FAILED( ( ret = _CleanInternal( &internal ) ) ) ) {
        END_FUNCTION("_ApplyIrrelevantSpeciesEliminationMethod", ret );
        return ret;
    }
         
    END_FUNCTION("_ApplyIrrelevantSpeciesEliminationMethod", SUCCESS );
    return ret;
}      



static LINKED_LIST *_CreateListOfInterestingSpecies( ABSTRACTION_METHOD *method, IR *ir ) {
    SPECIES *species = NULL;
    LINKED_LIST *speciesList = NULL;
    LINKED_LIST *list = NULL;
    
    START_FUNCTION("_CreateListOfInterestingSpecies");
    
    if( ( list = CreateLinkedList() ) == NULL ) {
        END_FUNCTION("_CreateListOfInterestingSpecies", FAILING );
        return NULL;
    }
    
    speciesList = ir->GetListOfSpeciesNodes( ir );
    
    ResetCurrentElement( speciesList );    
    while( ( species = (SPECIES*)GetNextFromLinkedList( speciesList ) ) != NULL ) {
        if( IsKeepFlagSetInSpeciesNode( species ) ) {
	  if( IS_FAILED( AddElementInLinkedList( (CADDR_T)species, list ) ) ) {
                END_FUNCTION("_CreateListOfInterestingSpecies", FAILING );
                return NULL;
            } 
        }
    }
         
    END_FUNCTION("_CreateListOfInterestingSpecies", SUCCESS );
    return list;
}


static RET_VAL _InitInternal( IR *ir, IRRELEVANT_SPECIES_ELIMINATION_INTERNAL *internal ) {
    RET_VAL ret = SUCCESS;
    LINKED_LIST *list = NULL;
    
    START_FUNCTION("_InitInternal");
    
    list = ir->GetListOfSpeciesNodes( ir );
    if( ( internal->irrelevantSpeciesList = CloneLinkedList( list ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitInternal", "could not create a list for irrelevant species" );
    }
    
    list = ir->GetListOfReactionNodes( ir );
    if( ( internal->irrelevantReactionList = CloneLinkedList( list ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitInternal", "could not create a list for irrelevant reactions" );
    }
    
    END_FUNCTION("_InitInternal", SUCCESS );
    return ret;
}


static RET_VAL _CleanInternal( IRRELEVANT_SPECIES_ELIMINATION_INTERNAL *internal ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("_CleanInternal");
    
    DeleteLinkedList( &(internal->irrelevantSpeciesList) );
    DeleteLinkedList( &(internal->irrelevantReactionList) );
    
    END_FUNCTION("_CleanInternal", SUCCESS );
    return ret;
}

static RET_VAL _FindIrrelevant( IR *ir, IRRELEVANT_SPECIES_ELIMINATION_INTERNAL *internal ) {
    RET_VAL ret = SUCCESS;
    IR_NODE *irrelevantNode = NULL;
    LINKED_LIST *irrelevantList = NULL;
    
    START_FUNCTION("_FindIrrelevant");
    
    irrelevantList = internal->irrelevantSpeciesList;
    ResetCurrentElement( irrelevantList );
    while( ( irrelevantNode = (IR_NODE*)GetNextFromLinkedList( irrelevantList ) ) != NULL ) {
        if( IsGraphFlagBlackInIRNode( (IR_NODE*)irrelevantNode ) ) {
            if( IS_FAILED( ( ret = RemoveElementFromLinkedList( (CADDR_T)irrelevantNode, irrelevantList ) ) ) ) {
                END_FUNCTION("_FindIrrelevant", ret );
                return ret;
            }        
        }
    }
    
    irrelevantList = internal->irrelevantReactionList;
    ResetCurrentElement( irrelevantList );
    while( ( irrelevantNode = (IR_NODE*)GetNextFromLinkedList( irrelevantList ) ) != NULL ) {
        if( IsGraphFlagBlackInIRNode( (IR_NODE*)irrelevantNode ) ) {
            if( IS_FAILED( ( ret = RemoveElementFromLinkedList( (CADDR_T)irrelevantNode, irrelevantList ) ) ) ) {
                END_FUNCTION("_FindIrrelevant", ret );
                return ret;
            }        
        }
    }
    
    END_FUNCTION("_FindIrrelevant", SUCCESS );
    return ret;
}


static RET_VAL _RemoveIrrelevant( IR *ir, IRRELEVANT_SPECIES_ELIMINATION_INTERNAL *internal ) {
    RET_VAL ret = SUCCESS;
    SPECIES *species = NULL;
    REACTION *reaction = NULL;
    LINKED_LIST *list = NULL;
    
    START_FUNCTION("_RemoveIrrelevant");
    
    list = internal->irrelevantSpeciesList;
    ResetCurrentElement( list );
    while( ( species = (SPECIES*)GetNextFromLinkedList( list ) ) != NULL ) {
        TRACE_1("removing species %s", GetCharArrayOfString( GetSpeciesNodeName( species ) ) );
        if( IS_FAILED( ( ir->RemoveSpecies( ir, species ) ) ) ) {
            END_FUNCTION("_RemoveIrrelevant", SUCCESS );
            return ret;
        }
    }
    
    list = internal->irrelevantReactionList;
    ResetCurrentElement( list );
    while( ( reaction = (REACTION*)GetNextFromLinkedList( list ) ) != NULL ) {
        TRACE_1("removing species %s", GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
        if( IS_FAILED( ( ir->RemoveReaction( ir, reaction ) ) ) ) {
            END_FUNCTION("_RemoveIrrelevant", SUCCESS );
            return ret;
        }
    }
    
    END_FUNCTION("_RemoveIrrelevant", SUCCESS );
    return ret;
}


static RET_VAL _SetNodeFlagsToWhite( IR *ir ) {
    RET_VAL ret = SUCCESS;
    LINKED_LIST *list = NULL;
    IR_NODE *node = NULL;
    
    START_FUNCTION("_SetNodeFlagsToWhite");
    
    list = ir->GetListOfSpeciesNodes( ir );
    ResetCurrentElement( list );
    while( ( node = (IR_NODE*)GetNextFromLinkedList( list ) ) != NULL ) {
        if( IS_FAILED( ( ret = SetGraphFlagToWhiteInIRNode( node ) ) ) ) {
            END_FUNCTION("_SetNodeFlagsToWhite", ret );
            return ret;
        } 
    }
    
    list = ir->GetListOfReactionNodes( ir );
    ResetCurrentElement( list );
    while( ( node = (IR_NODE*)GetNextFromLinkedList( list ) ) != NULL ) {
        if( IS_FAILED( ( ret = SetGraphFlagToWhiteInIRNode( node ) ) ) ) {
            END_FUNCTION("_SetNodeFlagsToWhite", ret );
            return ret;
        } 
    }
    
    END_FUNCTION("_SetNodeFlagsToWhite", SUCCESS );
    return ret;
}




static RET_VAL _WalkSpecies( ABSTRACTION_METHOD *method, SPECIES *species ) {
    RET_VAL ret = SUCCESS;
    IR_EDGE *edge = NULL;
    LINKED_LIST *edges = NULL;
    REACTION *reaction = NULL;
    
    START_FUNCTION("_WalkSpecies");
    
    if( !IsGraphFlagWhiteInIRNode( (IR_NODE*)species ) ) {
        END_FUNCTION("_WalkSpecies", SUCCESS );
        return ret;
    }
    
    TRACE_1( "visiting species %s", GetCharArrayOfString( GetSpeciesNodeName( species )  ) );
    
    if( IS_FAILED( ( ret = SetGraphFlagToGreyInIRNode( (IR_NODE*)species ) ) ) ) {
        END_FUNCTION("_WalkSpecies", ret );
        return ret;
    }
    
    edges = GetProductEdges( (IR_NODE*)species );
    ResetCurrentElement( edges );
    while( ( edge = GetNextEdge( edges ) ) != NULL ) {
        reaction = GetReactionInIREdge( edge );
        if( IS_FAILED( ( ret = _WalkReaction( method, reaction ) ) ) ) {
            END_FUNCTION("_WalkSpecies", ret );
            return ret;
        }
    }

    edges = GetReactantEdges( (IR_NODE*)species );
    ResetCurrentElement( edges );
    while( ( edge = GetNextEdge( edges ) ) != NULL ) {
        reaction = GetReactionInIREdge( edge );
        if( IS_FAILED( ( ret = _WalkReaction( method, reaction ) ) ) ) {
            END_FUNCTION("_WalkSpecies", ret );
            return ret;
        }
    }
             
    if( IS_FAILED( ( ret = SetGraphFlagToBlackInIRNode( (IR_NODE*)species ) ) ) ) {
        END_FUNCTION("_WalkSpecies", ret );
        return ret;
    }
    
    END_FUNCTION("_WalkSpecies", SUCCESS );
    return ret;
}
     
static RET_VAL _WalkReaction( ABSTRACTION_METHOD *method, REACTION *reaction ) {
    RET_VAL ret = SUCCESS;
    SPECIES *species = NULL;
    IR_EDGE *edge = NULL;
    LINKED_LIST *edges = NULL;
    
    START_FUNCTION("_WalkReaction");
    
    if( !IsGraphFlagWhiteInIRNode( (IR_NODE*)reaction ) ) {
        END_FUNCTION("_WalkSpecies", SUCCESS );
        return ret;
    }
    
    TRACE_1( "visiting reaction %s", GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
    
    if( IS_FAILED( ( ret = SetGraphFlagToGreyInIRNode( (IR_NODE*)reaction ) ) ) ) {
        END_FUNCTION("_WalkSpecies", ret );
        return ret;
    }

    edges = GetProductEdges( (IR_NODE*)reaction );
    ResetCurrentElement( edges );
    while( ( edge = GetNextEdge( edges ) ) != NULL ) {
        species = GetSpeciesInIREdge( edge );
        if( IS_FAILED( ( ret = _WalkSpecies( method, species ) ) ) ) {
            END_FUNCTION("_WalkSpecies", ret );
            return ret;
        }
    }

    edges = GetReactantEdges( (IR_NODE*)reaction );
    ResetCurrentElement( edges );
    while( ( edge = GetNextEdge( edges ) ) != NULL ) {
        species = GetSpeciesInIREdge( edge );
        if( IS_FAILED( ( ret = _WalkSpecies( method, species ) ) ) ) {
            END_FUNCTION("_WalkSpecies", ret );
            return ret;
        }
    }

    edges = GetModifierEdges( (IR_NODE*)reaction );
    ResetCurrentElement( edges );
    while( ( edge = GetNextEdge( edges ) ) != NULL ) {
        species = GetSpeciesInIREdge( edge );
        if( IS_FAILED( ( ret = _WalkSpecies( method, species ) ) ) ) {
            END_FUNCTION("_WalkSpecies", ret );
            return ret;
        }
    }
            
    if( IS_FAILED( ( ret = SetGraphFlagToBlackInIRNode( (IR_NODE*)reaction ) ) ) ) {
        END_FUNCTION("_WalkSpecies", ret );
        return ret;
    }
                 
    END_FUNCTION("_WalkReaction", SUCCESS );
    return ret;
}
     


