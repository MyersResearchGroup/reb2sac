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
#if defined(DEBUG)
#undef DEBUG
#endif

#include "abstraction_method_manager.h"
#include "IR.h"
#include "hash_table.h"
#include "symtab.h"
#include "strconv.h"
#include "logical_species_node.h"
#include "kinetic_law_evaluater.h"


 
typedef struct {
    int type;
    REACTION *reaction;
    LINKED_LIST *absoluteActivators;
    LINKED_LIST *absoluteInhibitors;
} ABSOLUTE_ACTIVATION_INHIBITION_INTERNAL;


static char * _GetAbsoluteActivationInhibitionGenerationMethodID( ABSTRACTION_METHOD *method );
static RET_VAL _ApplyAbsoluteActivationInhibitionGenerationMethod( ABSTRACTION_METHOD *method, IR *ir );      
static BOOL _IsConditionSatisfied( ABSTRACTION_METHOD *method, IR *ir, REACTION *reaction, ABSOLUTE_ACTIVATION_INHIBITION_INTERNAL *internal ); 
static RET_VAL _DoTransformation( ABSTRACTION_METHOD *method, IR *ir, ABSOLUTE_ACTIVATION_INHIBITION_INTERNAL *internal );
static RET_VAL _DoAbsoluteActivation( ABSTRACTION_METHOD *method, REACTION *reaction, LINKED_LIST *absoluteActivators );
static RET_VAL _DoAbsoluteInhibition( ABSTRACTION_METHOD *method, REACTION *reaction, LINKED_LIST *absoluteInhibitors );

static RET_VAL _CreateAbsoluteSpeciesList( ABSTRACTION_METHOD *method, IR *ir );
static RET_VAL _FreeAbsoluteSpeciesList( ABSTRACTION_METHOD *method );


static RET_VAL _AddLogicalSpeciesInInternal( KINETIC_LAW_EVALUATER *evaluater, REB2SAC_SYMTAB *symtab, REACTION *reaction, LOGICAL_SPECIES *logicalSpecies, ABSOLUTE_ACTIVATION_INHIBITION_INTERNAL *internal );
static RET_VAL _InitAbsoluteActivationInhibitionInternal( ABSOLUTE_ACTIVATION_INHIBITION_INTERNAL *internal );
static RET_VAL _CleanAbsoluteActivationInhibitionInternal( ABSOLUTE_ACTIVATION_INHIBITION_INTERNAL *internal );
static RET_VAL _FreeAbsoluteActivationInhibitionInternal( ABSOLUTE_ACTIVATION_INHIBITION_INTERNAL *internal );

static int _CompareLogicalSpecies( CADDR_T species1, CADDR_T species2 );

ABSTRACTION_METHOD *AbsoluteActivationInhibitionGenerationMethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager ) {
    static ABSTRACTION_METHOD method;
    
    START_FUNCTION("AbsoluteActivationInhibitionGenerationMethodConstructor");

    if( method.manager == NULL ) {
        method.manager = manager;
        method.GetID = _GetAbsoluteActivationInhibitionGenerationMethodID;
        method.Apply = _ApplyAbsoluteActivationInhibitionGenerationMethod;
    }
    
    TRACE_0( "AbsoluteActivationInhibitionGenerationMethodConstructor invoked" );
    
    END_FUNCTION("AbsoluteActivationInhibitionGenerationMethodConstructor", SUCCESS );
    return &method;
}



static char * _GetAbsoluteActivationInhibitionGenerationMethodID( ABSTRACTION_METHOD *method ) {
    START_FUNCTION("_GetAbsoluteActivationInhibitionGenerationMethodID");
    
    END_FUNCTION("_GetAbsoluteActivationInhibitionGenerationMethodID", SUCCESS );
    return "absolute-activation/inhibition-generator";
}



static RET_VAL _ApplyAbsoluteActivationInhibitionGenerationMethod( ABSTRACTION_METHOD *method, IR *ir ) {
    static BOOL firstTime = TRUE;
    RET_VAL ret = SUCCESS;
    REACTION *reaction = NULL;
    LINKED_LIST *reactionList = NULL;
    ABSOLUTE_ACTIVATION_INHIBITION_INTERNAL internal;
    
    START_FUNCTION("_ApplyAbsoluteActivationInhibitionGenerationMethod");

    if( !firstTime ) {
        END_FUNCTION("_ApplyAbsoluteActivationInhibitionGenerationMethod", SUCCESS );
        return SUCCESS;
    }
    firstTime = FALSE;
    
    if( IS_FAILED( ( ret = _CreateAbsoluteSpeciesList( method, ir ) ) ) ) {
        END_FUNCTION("_ApplyAbsoluteActivationInhibitionGenerationMethod", ret );
        return ret;
    } 
    
    if( IS_FAILED( ( ret = _InitAbsoluteActivationInhibitionInternal( &internal ) ) ) ) {
        END_FUNCTION("_ApplyAbsoluteActivationInhibitionGenerationMethod", ret );
        return ret;
    } 
    
    reactionList = ir->GetListOfReactionNodes( ir );    
    ResetCurrentElement( reactionList );    
    while( ( reaction = (REACTION*)GetNextFromLinkedList( reactionList ) ) != NULL ) {
        if( _IsConditionSatisfied( method, ir, reaction, &internal ) ) {
            if( IS_FAILED( ( ret = _DoTransformation( method, ir, &internal ) ) ) ) {
                END_FUNCTION("_ApplyAbsoluteActivationInhibitionGenerationMethod", ret );
                return ret;
            }
        }            
        if( IS_FAILED( ( ret = _CleanAbsoluteActivationInhibitionInternal( &internal ) ) ) ) {
            END_FUNCTION("_ApplyAbsoluteActivationInhibitionGenerationMethod", ret );
            return ret;
        } 
    }
    if( IS_FAILED( ( ret = _FreeAbsoluteActivationInhibitionInternal( &internal ) ) ) ) {
        END_FUNCTION("_ApplyAbsoluteActivationInhibitionGenerationMethod", ret );
        return ret;
    } 
            
    if( IS_FAILED( ( ret = _FreeAbsoluteSpeciesList( method ) ) ) ) {
        END_FUNCTION("_ApplyAbsoluteActivationInhibitionGenerationMethod", ret );
        return ret;
    } 
    
    END_FUNCTION("_ApplyAbsoluteActivationInhibitionGenerationMethod", SUCCESS );
    return ret;
}      

static BOOL _IsConditionSatisfied( ABSTRACTION_METHOD *method, IR *ir, REACTION *reaction, ABSOLUTE_ACTIVATION_INHIBITION_INTERNAL *internal ) {
    SPECIES *modifier = NULL;    
    LINKED_LIST *list = NULL;
    LINKED_LIST *absoluteSpeciesList = NULL;
    KINETIC_LAW_EVALUATER *evaluater = NULL;
    REB2SAC_SYMTAB *symtab;
    
    START_FUNCTION("_IsConditionSatisfied");
    
    absoluteSpeciesList = (LINKED_LIST*)(method->_internal1);
    symtab = ir->GetGlobalSymtab( ir );
    
    list = GetProductsInReactionNode( reaction );
    if( GetLinkedListSize( list ) == 0 ) {
        END_FUNCTION("_IsConditionSatisfied", SUCCESS );
        return FALSE;
    }
    
    if( ( evaluater = CreateKineticLawEvaluater() ) == NULL ) {
        TRACE_0("could not create a kinetic law evaluator" );
        END_FUNCTION("_IsConditionSatisfied", SUCCESS );
        return FALSE;
    }

    TRACE_1("finding absolute species for reaction %s", GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
        
    list = GetModifiersInReactionNode( reaction );
    ResetCurrentElement( list );
    while( ( modifier = (SPECIES*)GetNextFromLinkedList( list ) ) != NULL ) {
        if( !IsLogicalSpecies( modifier ) ) {
            END_FUNCTION("_IsConditionSatisfied", SUCCESS );
            return FALSE;
        } 
        if( FindElementInLinkedList( modifier, _CompareLogicalSpecies, absoluteSpeciesList ) < 0 ) {
            continue;        
        }
        
        TRACE_1("logical species %s is in the absolute species list", GetCharArrayOfString( GetSpeciesNodeName( modifier ) ) );
        if( IS_FAILED( _AddLogicalSpeciesInInternal( evaluater, symtab, reaction, (LOGICAL_SPECIES*)modifier, internal ) ) ) {
            END_FUNCTION("_IsConditionSatisfied", SUCCESS );
            return FALSE;
        }                        
    }
    
    
    FreeKineticLawEvaluater( &evaluater );
    internal->reaction = reaction;
    
    if( GetLinkedListSize( internal->absoluteActivators ) > 0 ) {
        if( GetLinkedListSize( internal->absoluteInhibitors ) > 0 ) {
            fprintf( stderr, "reaction %s has both absolute activator and absolute inhibitor" NEW_LINE, 
GetCharArrayOfString( 
GetReactionNodeName( reaction ) ) );
            END_FUNCTION("_IsConditionSatisfied", SUCCESS );
            return FALSE;
        }
        else {
            internal->type = REB2SAC_TYPE_ABSOLUTE_ACTIVATION;
            TRACE_1("%s satisfies the condition", GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
            END_FUNCTION("_IsConditionSatisfied", SUCCESS );
            return TRUE;
        }
    }
    else {
        if( GetLinkedListSize( internal->absoluteInhibitors ) > 0 ) {
            internal->type = REB2SAC_TYPE_ABSOLUTE_INHIBITION;
            TRACE_1("%s satisfies the condition", GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
            END_FUNCTION("_IsConditionSatisfied", SUCCESS );
            return TRUE;
        }
        else {
            END_FUNCTION("_IsConditionSatisfied", SUCCESS );
            return FALSE;
        }
    }
}

static RET_VAL _DoTransformation( ABSTRACTION_METHOD *method, IR *ir, ABSOLUTE_ACTIVATION_INHIBITION_INTERNAL *internal ) {
    RET_VAL ret = SUCCESS;
    REACTION *reaction = NULL;
    int type = 0;
    
    START_FUNCTION("_DoTransformation");
    
    type = internal->type;    
    reaction = internal->reaction;
    
    if( type == REB2SAC_TYPE_ABSOLUTE_ACTIVATION ) {
        if( IS_FAILED( ( ret = _DoAbsoluteActivation( method, reaction, internal->absoluteActivators ) ) ) ) {
            END_FUNCTION("_DoTransformation", ret );
            return ret;
        }
    }
    else if( type == REB2SAC_TYPE_ABSOLUTE_INHIBITION ) {
        if( IS_FAILED( ( ret = _DoAbsoluteInhibition( method, reaction, internal->absoluteInhibitors ) ) ) ) {
            END_FUNCTION("_DoTransformation", ret );
            return ret;
        }
    }
    else {
        return ErrorReport( FAILING, "_DoTransformation", "reaction %s has invalid absolute activation/inhibition type", 
        GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
    }
    
    END_FUNCTION("_DoTransformation", SUCCESS );
    return ret;
}


static RET_VAL _CreateAbsoluteSpeciesList( ABSTRACTION_METHOD *method, IR *ir ) {
    RET_VAL ret = SUCCESS;
    char *valueString = NULL;
    char buf[REB2SAC_LOGICAL_SPECIES_NAME_SIZE];
    double threshold = 0.0;
    double conLevel = 0.0;
    LINKED_LIST *list = NULL;
    LINKED_LIST *absoluteSpeciesList = NULL;
    STRING *originalSpeciesName = NULL;
    SPECIES *species = NULL;    
    LOGICAL_SPECIES *logicalSpecies = NULL;    
    ABSTRACTION_METHOD_MANAGER *manager = NULL;
    COMPILER_RECORD_T *record = NULL;
    REB2SAC_PROPERTIES *properties = NULL;
    
    START_FUNCTION("_CreateAbsoluteSpeciesList");
    
    if( ( absoluteSpeciesList = CreateLinkedList() ) == NULL ) {
        return ErrorReport( FAILING, "_CreateAbsoluteSpeciesList", "could not create a list for absolute species" );
    }
    
    manager = method->manager;
    record = manager->GetCompilerRecord( manager );
    properties = record->properties;
    
    list = ir->GetListOfSpeciesNodes( ir );
    ResetCurrentElement( list );
    while( ( species = (SPECIES*)GetNextFromLinkedList( list ) ) != NULL ) {
        if( !IsLogicalSpecies( species ) ) {
            continue;
        } 
        logicalSpecies = (LOGICAL_SPECIES*)species;
        originalSpeciesName = GetOriginalSpeciesName( logicalSpecies );
        sprintf( buf, "%s%s",  REB2SAC_ABSOLUTE_ACTIVATION_INHIBITION_THRESHOLD_KEY_PREFIX, GetCharArrayOfString( originalSpeciesName ) );
        if( ( valueString = properties->GetProperty( properties, buf ) ) == NULL ) {
            continue;
        }
        if( IS_FAILED( ( ret = StrToFloat( &threshold, valueString ) ) ) ) {
            continue;
        }
        TRACE_2( "threshold of %s is %f", GetCharArrayOfString( originalSpeciesName ), threshold );
        conLevel = GetCriticalConcentrationInLogicalSpecies( logicalSpecies );
        TRACE_2("the concentration level of %s is %f", GetCharArrayOfString( GetSpeciesNodeName( logicalSpecies ) ), conLevel );  
        if( conLevel < threshold ) {
            continue;
        }        
        
        TRACE_1("%s is added in the absolute list", GetCharArrayOfString( GetSpeciesNodeName( logicalSpecies ) ) );  
        if( IS_FAILED( ( ret = AddElementInLinkedList( logicalSpecies, absoluteSpeciesList ) ) ) ) {
            END_FUNCTION("_CreateAbsoluteSpeciesList", ret );
            return ret;
        }                        
    }
    
    method->_internal1 = (CADDR_T)absoluteSpeciesList;
    END_FUNCTION("_CreateAbsoluteSpeciesList", SUCCESS );
    return SUCCESS;
}


static RET_VAL _FreeAbsoluteSpeciesList( ABSTRACTION_METHOD *method ) {
    RET_VAL ret = SUCCESS;
    LINKED_LIST *list = NULL;
    
    START_FUNCTION("_FreeAbsoluteSpeciesList");
    
    list = (LINKED_LIST*)(method->_internal1);
    if( list != NULL ) {
        if( IS_FAILED( ( ret = DeleteLinkedList( &list ) ) ) ) {
            END_FUNCTION("_FreeAbsoluteSpeciesList", ret );
            return ret;
        }
    }
    
    END_FUNCTION("_FreeAbsoluteSpeciesList", SUCCESS );
    return SUCCESS;
}


static RET_VAL _AddLogicalSpeciesInInternal( KINETIC_LAW_EVALUATER *evaluater, REB2SAC_SYMTAB *symtab, REACTION *reaction, LOGICAL_SPECIES *logicalSpecies, ABSOLUTE_ACTIVATION_INHIBITION_INTERNAL *internal ) {
    RET_VAL ret = SUCCESS;
    char buf[REB2SAC_LOGICAL_SPECIES_NAME_SIZE];
    double value = 0.0;
    double value1 = 0.0;
    double value2 = 0.0;
    KINETIC_LAW *kineticLaw = NULL;
    LINKED_LIST *modifiers = NULL;
    REB2SAC_SYMBOL *sym = NULL;
    
    START_FUNCTION("_AddLogicalSpeciesInInternal");
    
    kineticLaw = GetKineticLawInReactionNode( reaction );
    sprintf( buf, "%s_con", GetCharArrayOfString( GetSpeciesNodeName( logicalSpecies ) ) );    
    if( ( sym = symtab->Lookup( symtab, buf ) ) == NULL ) {
        return ErrorReport( FAILING, "_AddLogicalSpeciesInInternal", "%s is not found in the symtab", buf );
    }

    value = GetRealValueInSymbol( sym );
        
    if( IS_FAILED( ( ret = evaluater->SetDefaultSpeciesValue( evaluater,
        REB2SAC_ABSOLUTE_ACTIVATION_INHIBITION_DEFAULT_VALUE_TO_EVALUATE_MODIFIERS ) ) ) ) {
        END_FUNCTION("_AddLogicalSpeciesInInternal", ret );        
        return ret;
    }
    
    if( IS_FAILED( ( ret = SetRealValueInSymbol( sym, REB2SAC_ABSOLUTE_ACTIVATION_INHIBITION_VALUE1_TO_EVALUATE_MODIFIERS ) ) ) ) {
        END_FUNCTION("_AddLogicalSpeciesInInternal", ret );        
        return ret;
    }
    value1 = evaluater->Evaluate( evaluater, kineticLaw );

    if( IS_FAILED( ( ret = SetRealValueInSymbol( sym, REB2SAC_ABSOLUTE_ACTIVATION_INHIBITION_VALUE2_TO_EVALUATE_MODIFIERS ) ) ) ) {
        END_FUNCTION("_AddLogicalSpeciesInInternal", ret );        
        return ret;
    }
    value2 = evaluater->Evaluate( evaluater, kineticLaw );
    
    if( IS_FAILED( ( ret = SetRealValueInSymbol( sym, value ) ) ) ) {
        END_FUNCTION("_AddLogicalSpeciesInInternal", ret );        
        return ret;
    }
    
    TRACE_3( "value1 is %f, and value2 is %f for %s", value1, value2, GetCharArrayOfString( GetSpeciesNodeName( (SPECIES*)logicalSpecies ) ) );
    
    if( IS_REAL_EQUAL( value2, value1 ) ) {        
        END_FUNCTION("_AddLogicalSpeciesInInternal", ret );        
        return ret;
    }
    else if( value2 > value1 ) {
        TRACE_1( "%s is an absolute activator", GetCharArrayOfString( GetSpeciesNodeName( (SPECIES*)logicalSpecies ) ) );
        if( IS_FAILED( ( ret = AddElementInLinkedList( logicalSpecies, internal->absoluteActivators ) ) ) ) {
            END_FUNCTION("_AddLogicalSpeciesInInternal", ret );        
            return ret;
        }
    }
    else {
        TRACE_1( "%s is an absolute inhibitor", GetCharArrayOfString( GetSpeciesNodeName( (SPECIES*)logicalSpecies ) ) );
        if( IS_FAILED( ( ret = AddElementInLinkedList( logicalSpecies, internal->absoluteInhibitors ) ) ) ) {
            END_FUNCTION("_AddLogicalSpeciesInInternal", ret );        
            return ret;
        }
    }
           
    END_FUNCTION("_AddLogicalSpeciesInInternal", SUCCESS );        
    return ret;
}




static RET_VAL _DoAbsoluteActivation( ABSTRACTION_METHOD *method, REACTION *reaction, LINKED_LIST *absoluteActivators ) {
    RET_VAL ret = SUCCESS;
    SPECIES *activator = NULL;
    KINETIC_LAW *activation = NULL;
    KINETIC_LAW *kineticLaw = NULL;
    
    START_FUNCTION("_DoAbsoluteActivation");
    
    kineticLaw = GetKineticLawInReactionNode( reaction );
    FreeKineticLaw( &kineticLaw );
    
        
    ResetCurrentElement( absoluteActivators );
    activator = (SPECIES*)GetNextFromLinkedList( absoluteActivators );
    if( ( kineticLaw = CreateOpKineticLaw( KINETIC_LAW_OP_TIMES, CreateSpeciesKineticLaw( activator ), CreateRealValueKineticLaw( 1.0 ) ) ) == NULL ) {
        return ErrorReport( FAILING, "_DoAbsoluteActivation",  "could not create %s in %s", 
        GetCharArrayOfString( GetSpeciesNodeName( activator ) ), GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
    }    
    
    while( ( activator = (SPECIES*)GetNextFromLinkedList( absoluteActivators ) ) != NULL ) {
        if( ( activation = CreateSpeciesKineticLaw( activator ) ) == NULL ) {
            return ErrorReport( FAILING, "_DoAbsoluteActivation",  "could not create %s in %s", 
            GetCharArrayOfString( GetSpeciesNodeName( activator ) ), GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
        }
        if( ( kineticLaw = CreateOpKineticLaw( KINETIC_LAW_OP_TIMES, activation, kineticLaw ) ) == NULL ) {
            return ErrorReport( FAILING, "_DoAbsoluteActivation", "could not add (%s) in %s", 
                GetCharArrayOfString( GetSpeciesNodeName( activator ) ), GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
        }
    }
    
    if( IS_FAILED( ( ret = SetKineticLawInReactionNode( reaction, kineticLaw ) ) ) ) {
        END_FUNCTION("_DoAbsoluteActivation", ret );
        return ret;
    } 
    
    END_FUNCTION("_DoAbsoluteActivation", SUCCESS );
    return ret;
}

static RET_VAL _DoAbsoluteInhibition( ABSTRACTION_METHOD *method, REACTION *reaction, LINKED_LIST *absoluteInhibitors ) {
    RET_VAL ret = SUCCESS;
    SPECIES *inhibitor = NULL;
    KINETIC_LAW *inhibition = NULL;
    KINETIC_LAW *kineticLaw = NULL;
    
    START_FUNCTION("_DoAbsoluteInhibition");
    
    kineticLaw = GetKineticLawInReactionNode( reaction );
    FreeKineticLaw( &kineticLaw );
        
    ResetCurrentElement( absoluteInhibitors );
    inhibitor = (SPECIES*)GetNextFromLinkedList( absoluteInhibitors );
        
    if( ( kineticLaw = CreateOpKineticLaw( KINETIC_LAW_OP_MINUS, CreateRealValueKineticLaw( 1.0 ), CreateSpeciesKineticLaw( inhibitor ) ) ) == NULL ) {
        return ErrorReport( FAILING, "_DoAbsoluteInhibition",  "could not create 1 - %s in %s", 
        GetCharArrayOfString( GetSpeciesNodeName( inhibitor ) ), GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
    }
    if( ( kineticLaw = CreateOpKineticLaw( KINETIC_LAW_OP_TIMES, kineticLaw, CreateRealValueKineticLaw( 0.0 ) ) ) == NULL ) {
        return ErrorReport( FAILING, "_DoAbsoluteInhibition",  "could not create kinetic law in %s", 
        GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
    }    
    
    while( ( inhibitor = (SPECIES*)GetNextFromLinkedList( absoluteInhibitors ) ) != NULL ) {
        if( ( inhibition = CreateOpKineticLaw( KINETIC_LAW_OP_MINUS, CreateRealValueKineticLaw( 1.0 ), CreateSpeciesKineticLaw( inhibitor ) ) ) == NULL ) {
            return ErrorReport( FAILING, "_DoAbsoluteInhibition",  "could not create 1 - %s in %s", 
            GetCharArrayOfString( GetSpeciesNodeName( inhibitor ) ), GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
        }
        if( ( kineticLaw = CreateOpKineticLaw( KINETIC_LAW_OP_TIMES, inhibition, kineticLaw ) ) == NULL ) {
            return ErrorReport( FAILING, "_DoAbsoluteInhibition",  "could not add (1 - %s) in %s", 
            GetCharArrayOfString( GetSpeciesNodeName( inhibitor ) ), GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
        }
    }
    
    if( IS_FAILED( ( ret = SetKineticLawInReactionNode( reaction, kineticLaw ) ) ) ) {
        END_FUNCTION("_DoAbsoluteActivation", ret );
        return ret;
    } 
    
    END_FUNCTION("_DoAbsoluteInhibition", SUCCESS );
    return ret;
}




static RET_VAL _InitAbsoluteActivationInhibitionInternal( ABSOLUTE_ACTIVATION_INHIBITION_INTERNAL *internal ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("_InitAbsoluteActivationInhibitionInternal");
    
    if( ( internal->absoluteActivators = CreateLinkedList() ) == NULL ) {
        return ErrorReport( FAILING, "_InitAbsoluteActivationInhibitionInternal", "failed to create a list for absolute activators" );
    }
    if( ( internal->absoluteInhibitors = CreateLinkedList() ) == NULL ) {
        return ErrorReport( FAILING, "_InitAbsoluteActivationInhibitionInternal", "failed to create a list for absolute inhibitors" );
    }
    
    END_FUNCTION("_InitAbsoluteActivationInhibitionInternal", SUCCESS );
    return ret;
}

static RET_VAL _CleanAbsoluteActivationInhibitionInternal( ABSOLUTE_ACTIVATION_INHIBITION_INTERNAL *internal ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("_CleanAbsoluteActivationInhibitionInternal");
    
    if( IS_FAILED( ( ret = _FreeAbsoluteActivationInhibitionInternal( internal ) ) ) ) {
        END_FUNCTION("_CleanAbsoluteActivationInhibitionInternal", ret );
        return ret;
    }
    
    if( IS_FAILED( ( ret = _InitAbsoluteActivationInhibitionInternal( internal ) ) ) ) {
        END_FUNCTION("_CleanAbsoluteActivationInhibitionInternal", ret );
        return ret;
    }
    
    END_FUNCTION("_CleanAbsoluteActivationInhibitionInternal", SUCCESS );
    return ret;
}

static RET_VAL _FreeAbsoluteActivationInhibitionInternal( ABSOLUTE_ACTIVATION_INHIBITION_INTERNAL *internal ) {
    RET_VAL ret = SUCCESS;
    
    START_FUNCTION("_FreeAbsoluteActivationInhibitionInternal");
    
    DeleteLinkedList( &(internal->absoluteActivators) );
    DeleteLinkedList( &(internal->absoluteInhibitors) );
    
    END_FUNCTION("_FreeAbsoluteActivationInhibitionInternal", SUCCESS );
    return ret;
}

static int _CompareLogicalSpecies( CADDR_T species1, CADDR_T species2 ) {
    return (int)species1 - (int)species2;
}
