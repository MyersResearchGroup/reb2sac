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
#if 0
#if defined(DEBUG)
#undef DEBUG
#endif
#endif

#include "abstraction_method_manager.h"
#include "law_of_mass_action_util.h"
#include "strconv.h"
#include "nary_order_transformation_method.h"
#include "nary_order_decider.h"
#include "logical_species_node.h"
#include "critical_concentration_finder.h"

static char * _GetNaryOrderUnaryTransformationMethodID( ABSTRACTION_METHOD *method );
static RET_VAL _ApplyNaryOrderUnaryTransformationMethod( ABSTRACTION_METHOD *method, IR *ir );      
static RET_VAL _FreeNaryOrderUnaryTransformationMethod( ABSTRACTION_METHOD *method );
static BOOL _IsConditionSatisfied( ABSTRACTION_METHOD *method, SPECIES *species, CRITICAL_CONCENTRATION_INFO *info );

static RET_VAL _DoTransformation( ABSTRACTION_METHOD *method, IR *ir, CRITICAL_CONCENTRATION_INFO *info );      
static RET_VAL _CreateLogicalSpecies( ABSTRACTION_METHOD *method, IR *ir, CRITICAL_CONCENTRATION_INFO *info );
static RET_VAL _CreateLogicalConcentrationLevels( ABSTRACTION_METHOD *method, IR *ir, CRITICAL_CONCENTRATION_INFO *info );

static RET_VAL _HandleReactionM( ABSTRACTION_METHOD *method, IR *ir, IR_EDGE *edge, CRITICAL_CONCENTRATION_INFO *info );
static RET_VAL _HandleReactionP( ABSTRACTION_METHOD *method, IR *ir, IR_EDGE *edge, CRITICAL_CONCENTRATION_INFO *info );
static RET_VAL _HandleReactionR( ABSTRACTION_METHOD *method, IR *ir, IR_EDGE *edge, CRITICAL_CONCENTRATION_INFO *info );

static RET_VAL _AddInhibitorInKineticLaw( REACTION *reaction, SPECIES *inhibitor ); 
static RET_VAL _AddActivatorInKineticLaw( REACTION *reaction, SPECIES *activator ); 
static RET_VAL _AddNormalizationInKineticLaw( REACTION *reaction, double criticalConDelta ); 
static RET_VAL _AddNormalizationInSymbolKineticLaw( REACTION *reaction, KINETIC_LAW *value2, KINETIC_LAW *value1, int stoichiometry );

static RET_VAL _FindCriticalConcentrationLevelsFromPropertiesFile( ABSTRACTION_METHOD *method, SPECIES *species, LINKED_LIST *list );
static int _GetCriticalConcentrationLevelsSpecificationType( ABSTRACTION_METHOD *method, SPECIES *species );

static STRING *_CreateNewReactionID( REACTION *newReaction );
static STRING *_CreateLogicalSpeciesID( SPECIES *originalSpecies, int order );
        
static RET_VAL _CleanCriticalConInfo( CRITICAL_CONCENTRATION_INFO *info );

ABSTRACTION_METHOD *NaryOrderUnaryTransformationMethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager ) {
    static ABSTRACTION_METHOD method;
    HASH_TABLE *table = NULL;
    NARY_ORDER_DECIDER *decider = NULL;
    static CRITICAL_CONCENTRATION_FINDER finder;
    
    START_FUNCTION("NaryOrderUnaryTransformationMethodConstructor");

    if( method.manager == NULL ) {
        method.manager = manager;
        method.GetID = _GetNaryOrderUnaryTransformationMethodID;
        method.Apply = _ApplyNaryOrderUnaryTransformationMethod;
        method.Free = _FreeNaryOrderUnaryTransformationMethod;

        if( ( table = CreateHashTable( 64 ) ) == NULL ) {
            END_FUNCTION("NaryOrderUnaryTransformationMethodConstructor", FAILING );
            return NULL;
        }
        method._internal1 = (CADDR_T)table;
        
        if( ( decider = GetNaryOrderDeciderInstance( manager ) ) == NULL ) {
            END_FUNCTION("NaryOrderUnaryTransformationMethodConstructor", FAILING );
            return NULL;
        }        
        method._internal2 = (CADDR_T)decider;        
    
        if( IS_FAILED( InitCriticalConcentrationFinder( &finder, manager->GetCompilerRecord( manager ) ) ) ) {
            END_FUNCTION("NaryOrderUnaryTransformationMethodConstructor", FAILING );
            return NULL;
        }
        method._internal3 = (CADDR_T)(&finder);  
    }
    
    TRACE_0( "NaryOrderUnaryTransformationMethodConstructor invoked" );
    
    END_FUNCTION("NaryOrderUnaryTransformationMethodConstructor", SUCCESS );
    return &method;
}



static char * _GetNaryOrderUnaryTransformationMethodID( ABSTRACTION_METHOD *method ) {
    START_FUNCTION("_GetNaryOrderUnaryTransformationMethodID");
    
    END_FUNCTION("_GetNaryOrderUnaryTransformationMethodID", SUCCESS );
    return "nary-order-unary-transformer";
}



static RET_VAL _ApplyNaryOrderUnaryTransformationMethod( ABSTRACTION_METHOD *method, IR *ir ) {
    static BOOL firstTime = TRUE;
    RET_VAL ret = SUCCESS;
    SPECIES *species = NULL;
    LINKED_LIST *speciesList = NULL;
    LINKED_LIST *list = NULL;    
    IR_EDGE *edge = NULL;
    CRITICAL_CONCENTRATION_INFO info;
    
    START_FUNCTION("_ApplyNaryOrderUnaryTransformationMethod");

    if( !firstTime ) {
        END_FUNCTION("_ApplyNaryOrderUnaryTransformationMethod", SUCCESS );
        return SUCCESS;
    }
    firstTime = FALSE;
    
    speciesList = ir->GetListOfSpeciesNodes( ir );
    
    ResetCurrentElement( speciesList );    
    while( ( species = (SPECIES*)GetNextFromLinkedList( speciesList ) ) ) {
        if( _IsConditionSatisfied( method, species, &info ) ) {
            TRACE_1("%s satisfied the conditions", GetCharArrayOfString( GetSpeciesNodeName( species ) ) );
#if 1
            if( IS_FAILED( ( ret = _DoTransformation( method, ir, &info ) ) ) ) {
                END_FUNCTION("_ApplyNaryOrderUnaryTransformationMethod", ret );
                return ret;
            }
#endif
        }   
    }
#if 1
    ResetCurrentElement( speciesList );    
    while( ( species = (SPECIES*)GetNextFromLinkedList( speciesList ) ) ) {
        list =  GetReactantEdges( species );
        ResetCurrentElement( list );    
        while( ( edge = GetNextEdge( list ) ) != NULL ) {
            edge->stoichiometry = 1;
        }
        list = GetProductEdges( species );
        ResetCurrentElement( list );    
        while( ( edge = GetNextEdge( list ) ) != NULL ) {
            edge->stoichiometry = 1;
        }
    }
#endif        
    
    
    END_FUNCTION("_ApplyNaryOrderUnaryTransformationMethod", SUCCESS );
    return ret;
}      

static RET_VAL _FreeNaryOrderUnaryTransformationMethod( ABSTRACTION_METHOD *method ) {
    RET_VAL ret = SUCCESS;
    HASH_TABLE *table = NULL;
    
    START_FUNCTION("_FreeNaryOrderUnaryTransformationMethod");

    table = (HASH_TABLE*)(method->_internal1);
    if( IS_FAILED( ( ret = DeleteHashTable( &table ) ) ) ) {
        END_FUNCTION("_FreeNaryOrderUnaryTransformationMethod", ret );
        return ret;
    }
        
    END_FUNCTION("_FreeNaryOrderUnaryTransformationMethod", SUCCESS );
    return ret;
}


static BOOL _IsConditionSatisfied( ABSTRACTION_METHOD *method, SPECIES *species, CRITICAL_CONCENTRATION_INFO *info ) {
    int specType = REB2SAC_CRITICAL_CONCENTRATION_LEVEL_SPECIFICATION_TYPE_DEFAULT;
    int i = 0;
    int len = 0;
    double criticalConcentration = 0.0;
    double *concentrationListElement = NULL;
    double *criticalCons = NULL;
    IR_EDGE *edge = NULL;
    REACTION *reaction = NULL;
    KINETIC_LAW *kineticLaw = NULL;
    LINKED_LIST *edges = NULL;
    LINKED_LIST *list = NULL;
    LINKED_LIST *concentrationList = NULL;
    HASH_TABLE *table = NULL;
    NARY_ORDER_DECIDER *decider = NULL;
    CRITICAL_CONCENTRATION_FINDER *finder = NULL;    
    
    START_FUNCTION("_IsConditionSatisfied");

    /*
     * species S cannot be a logical species 
     */  
    table = (HASH_TABLE*)(method->_internal1);
    if( GetValueFromHashTable( species, sizeof(SPECIES), table ) != NULL ) {
        END_FUNCTION("_IsConditionSatisfied", SUCCESS );
        return FALSE;
    }
    
    finder = (CRITICAL_CONCENTRATION_FINDER *)method->_internal3;
    
    /*
     * species S must be either produced or consumed 
     */  
    
    edges = GetReactantEdges( species );
    if( GetLinkedListSize( edges ) == 0 ) {
        edges = GetProductEdges( species );
        if( GetLinkedListSize( edges ) == 0 ) {
            END_FUNCTION("_IsConditionSatisfied", SUCCESS );
            return FALSE;
        }
    }
    
    edges = GetReactantEdges( species );
    ResetCurrentElement( edges );
    while( ( edge = GetNextEdge( edges ) ) != NULL ) {
        /*
        * for each r in { reactions which use S as a reactant }
        *  S is the only reactant in r
        */  
        reaction = GetReactionInIREdge( edge );
        list = GetReactantEdges( reaction );
        if( GetLinkedListSize( list ) > 1 ) {
            END_FUNCTION("_IsConditionSatisfied", SUCCESS );
            return FALSE;
        }
        /*
        * for each r in { reactions which use S as a reactant }
        *  there is no product in r
        */  
        list = GetProductEdges( reaction );
        if( GetLinkedListSize( list ) > 0 ) {
            END_FUNCTION("_IsConditionSatisfied", SUCCESS );
            return FALSE;
        }    
    }        
    
    edges = GetProductEdges( species );
    ResetCurrentElement( edges );
    while( ( edge = GetNextEdge( edges ) ) != NULL ) {
        /*
        * for each r in { reactions which use S as a product }
        *  there is no reactant in r
        */  
        reaction = GetReactionInIREdge( edge );
        list = GetReactantEdges( reaction );
        if( GetLinkedListSize( list ) > 0 ) {
            END_FUNCTION("_IsConditionSatisfied", SUCCESS );
            return FALSE;
        }
        /*
        * for each r in { reactions which use S as a product }
        *  S is the only product in r
        */  
        list = GetProductEdges( reaction );
        if( GetLinkedListSize( list ) > 1 ) {
            END_FUNCTION("_IsConditionSatisfied", SUCCESS );
            return FALSE;
        }    
    }
    
    if( ( concentrationList = CreateLinkedList() ) == NULL ) {
        TRACE_1( "failed to create critical concentration list %s", GetCharArrayOfString( GetSpeciesNodeName( species ) ) ); 
        END_FUNCTION("_IsConditionSatisfied", FAILING );
        return FALSE;
    } 
    
    specType = _GetCriticalConcentrationLevelsSpecificationType( method, species );    

    if( specType != REB2SAC_CRITICAL_CONCENTRATION_LEVEL_SPECIFICATION_TYPE_ONLY_PROPERTIES ) {
        list = GetModifierEdges( species );
        ResetCurrentElement( list );
        while( ( edge = GetNextEdge( list ) ) != NULL ) {
            reaction = GetReactionInIREdge( edge );    
            if( IS_FAILED( finder->AddListOfCriticalConcentrationLevels( finder, reaction, species, concentrationList ) ) ) {
                END_FUNCTION("_IsConditionSatisfied", FAILING );
                return FALSE;
            }
        }
    }
    
    if( specType != REB2SAC_CRITICAL_CONCENTRATION_LEVEL_SPECIFICATION_TYPE_ONLY_CALCULATION ) {
        if( IS_FAILED( _FindCriticalConcentrationLevelsFromPropertiesFile( method, species, concentrationList ) ) ) {
            TRACE_1( "failed to create critical concentrations for %s", GetCharArrayOfString( GetSpeciesNodeName( species ) ) ); 
            END_FUNCTION("_IsConditionSatisfied", FAILING );
            return FALSE;
        }
    }
        
    len = GetLinkedListSize( concentrationList );
    if( len == 0 ) {
        END_FUNCTION("_IsConditionSatisfied", SUCCESS );
        return FALSE;
    } 
    
    if( ( criticalCons = (double*)MALLOC( sizeof(double) * len ) ) == NULL ) {
        TRACE_1( "failed to create critical concentrations for %s", GetCharArrayOfString( GetSpeciesNodeName( species ) ) ); 
        END_FUNCTION("_IsConditionSatisfied", FAILING );
        return FALSE;
    }

    ResetCurrentElement( concentrationList );
    i = 0;
    while( ( concentrationListElement = (double*)GetNextFromLinkedList( concentrationList ) ) != NULL ) {
        criticalCons[i] = *concentrationListElement;
        FREE( concentrationListElement );
        i++;
    }    
    DeleteLinkedList( &concentrationList );

    info->species = species;        
    decider = (NARY_ORDER_DECIDER*)(method->_internal2);
    
    if( IS_FAILED( decider->Decide( method, info, criticalCons, len ) ) ) {
        FREE( criticalCons );
        END_FUNCTION("_IsConditionSatisfied", FAILING );
        return FALSE;
    } 
            
    FREE( criticalCons );
    END_FUNCTION("_IsConditionSatisfied", SUCCESS );
    return TRUE;
}


static int _GetCriticalConcentrationLevelsSpecificationType( ABSTRACTION_METHOD *method, SPECIES *species ) {
    int type = REB2SAC_CRITICAL_CONCENTRATION_LEVEL_SPECIFICATION_TYPE_DEFAULT;
    char buf[2048];
    char *valueString = NULL;
    ABSTRACTION_METHOD_MANAGER *manager = NULL;
    COMPILER_RECORD_T *record = NULL;
    REB2SAC_PROPERTIES *properties = NULL;

    START_FUNCTION("_FindCriticalConcentrationLevelsFromPropertiesFile");

    manager = method->manager;
    record = manager->GetCompilerRecord( manager );
    properties = record->properties;
    
    sprintf( buf, "%s%s", REB2SAC_CRITICAL_CONCENTRATION_LEVEL_SPECIFICATION_TYPE_KEY_PREFIX, GetCharArrayOfString( GetSpeciesNodeName( species ) ) );
    if( ( valueString = properties->GetProperty( properties, buf ) ) == NULL ) {
        END_FUNCTION("_FindCriticalConcentrationLevelsFromPropertiesFile", SUCCESS );
        return type;
    }
    if( strcmp( valueString, REB2SAC_CRITICAL_CONCENTRATION_LEVEL_SPECIFICATION_TYPE_ONLY_PROPERTIES_STRING ) == 0 ) {
        type = REB2SAC_CRITICAL_CONCENTRATION_LEVEL_SPECIFICATION_TYPE_ONLY_PROPERTIES;
        END_FUNCTION("_FindCriticalConcentrationLevelsFromPropertiesFile", SUCCESS );
        return type;
    }
    else if( strcmp( valueString, REB2SAC_CRITICAL_CONCENTRATION_LEVEL_SPECIFICATION_TYPE_ONLY_CALCULATION_STRING ) == 0 ) {
        type = REB2SAC_CRITICAL_CONCENTRATION_LEVEL_SPECIFICATION_TYPE_ONLY_CALCULATION;
        END_FUNCTION("_FindCriticalConcentrationLevelsFromPropertiesFile", SUCCESS );
        return type;
    }
/*    
    else {
        type = REB2SAC_CRITICAL_CONCENTRATION_LEVEL_SPECIFICATION_TYPE_DEFAULT;
    }
*/
    END_FUNCTION("_FindCriticalConcentrationLevelsFromPropertiesFile", SUCCESS );
    return type;
}


static RET_VAL _FindCriticalConcentrationLevelsFromPropertiesFile( ABSTRACTION_METHOD *method, SPECIES *species, LINKED_LIST *list ) {
    RET_VAL ret = SUCCESS;
    int i = 0;
    char buf[2048];
    char *valueString = NULL;
    double *conListElement = NULL;
    ABSTRACTION_METHOD_MANAGER *manager = NULL;
    COMPILER_RECORD_T *record = NULL;
    REB2SAC_PROPERTIES *properties = NULL;

        
    START_FUNCTION("_FindCriticalConcentrationLevelsFromPropertiesFile");

    manager = method->manager;
    record = manager->GetCompilerRecord( manager );
    properties = record->properties;
    
    for( i = 1; ; i++ ) {
        sprintf( buf, "%s%s.%i", REB2SAC_CRITICAL_CONCENTRATION_LEVEL_KEY_PREFIX, GetCharArrayOfString( GetSpeciesNodeName( species ) ), i );
        if( ( valueString = properties->GetProperty( properties, buf ) ) == NULL ) {
            break;
        }
        if( ( conListElement = (double*)MALLOC( sizeof(double) ) ) == NULL ) {
            return ErrorReport( FAILING, "_FindCriticalConcentrationLevelsFromPropertiesFile", "could not allocate double for %s", valueString );
        }
        if( IS_FAILED( ( ret = StrToFloat( conListElement, valueString ) ) ) ) {
            END_FUNCTION("_FindCriticalConcentrationLevelsFromPropertiesFile", ret );
            return ret;
        }
        if( IS_FAILED( ( ret = AddElementInLinkedList( conListElement, list ) ) ) ) {
            END_FUNCTION("_FindCriticalConcentrationLevelsFromPropertiesFile", ret );
            return ret;
        } 
    }
        
    END_FUNCTION("_FindCriticalConcentrationLevelsFromPropertiesFile", SUCCESS );
    return ret;
}



static RET_VAL _DoTransformation( ABSTRACTION_METHOD *method, IR *ir, CRITICAL_CONCENTRATION_INFO *info ) {
    RET_VAL ret = SUCCESS;
    SPECIES *species = NULL;
    IR_EDGE *edge = NULL;
    REACTION *reaction = NULL;
    REACTION *reactionAsModifier = NULL;
    LINKED_LIST *list = NULL;
    LINKED_LIST *reactionsAsModifier = NULL;
    
    START_FUNCTION("_DoTransformation");

    if( IS_FAILED( ( ret = _CreateLogicalSpecies( method, ir, info ) ) ) ) {
        END_FUNCTION("_DoTransformation", ret );
        return ret;
    }
    
    if( IS_FAILED( ( ret = _CreateLogicalConcentrationLevels( method, ir, info ) ) ) ) {
        END_FUNCTION("_DoTransformation", ret );
        return ret;
    }
    
    species = info->species;
    
    list = GetProductEdges( species );
    if( GetLinkedListSize( list ) > 0 ) {
        ResetCurrentElement( list );        
        while( ( edge = GetNextEdge( list ) ) != NULL ) {
            if( IS_FAILED( ( ret = _HandleReactionP( method, ir, edge, info ) ) ) ) {
                END_FUNCTION("_DoTransformation", ret );
                return ret;
            }
        }
    }
    
    list = GetReactantEdges( species );
    if( GetLinkedListSize( list ) > 0 ) {
        ResetCurrentElement( list );        
        while( ( edge = GetNextEdge( list ) ) != NULL ) {
            if( IS_FAILED( ( ret = _HandleReactionR( method, ir, edge, info ) ) ) ) {
                END_FUNCTION("_DoTransformation", ret );
                return ret;
            }
        }
    }
    
    list = GetModifierEdges( species );
    ResetCurrentElement( list );
    while( ( edge = GetNextEdge( list ) ) != NULL ) {
        if( IS_FAILED( ( ret = _HandleReactionM( method, ir, edge, info ) ) ) ) {
            END_FUNCTION("_DoTransformation", ret );
            return ret;
        }                
    }
    
    if( IS_FAILED( ( ret = _CleanCriticalConInfo( info ) ) ) ) {
        END_FUNCTION("_DoTransformation", ret );
        return ret;
    }
    
    if( IS_FAILED( ( ret = ir->RemoveSpecies( ir, species ) ) ) ) {
        END_FUNCTION("_DoTransformation", ret );
        return ret;
    }
        
    END_FUNCTION("_DoTransformation", SUCCESS );
    return ret;
}


static RET_VAL _CreateLogicalSpecies( ABSTRACTION_METHOD *method, IR *ir, CRITICAL_CONCENTRATION_INFO *info ) {
    RET_VAL ret = SUCCESS;
    char buf[REB2SAC_LOGICAL_SPECIES_NAME_SIZE];
    int i = 0;
    int len = 0;
    int suffixPos = 0;
    double concentration = 0.0;
    STRING *newName = NULL;
    SPECIES *species = NULL;
    LOGICAL_SPECIES *logicalSpecies = NULL;
    CRITICAL_CONCENTRATION_ELEMENT *elements = NULL;
    HASH_TABLE *table = NULL;
    
    START_FUNCTION("_CreateLogicalSpecies");
    
    table = (HASH_TABLE*)(method->_internal1);
    
    len = info->len;
    elements = info->elements;
    species = info->species;
    
    strcpy( buf, GetCharArrayOfString( GetSpeciesNodeName( species ) ) );
    suffixPos = strlen( buf );
    buf[suffixPos] = '_';
    suffixPos++;
    buf[suffixPos + len] = '\0';
    
    for( i = 1; i <= len; i++ ) {
        concentration = elements[i-1].concentration;
#if 0        
        memset( buf + suffixPos, '0', len - i );
        memset( buf + suffixPos + len - i, '1', i );
        TRACE_3( "%ith logical species of %s is %s", i, GetCharArrayOfString( GetSpeciesNodeName( species ) ), buf );
        if( ( newName = CreateString( buf ) ) == NULL ) {
            return ErrorReport( FAILING, "_CreateLogicalSpecies",  "name of %ith logical species of %s %s could not be created", i, GetCharArrayOfString( GetSpeciesNodeName( species ) ), buf );
        }
#endif        
        if( ( newName = _CreateLogicalSpeciesID( species, i ) ) == NULL ) {
            return ErrorReport( FAILING, "_CreateLogicalSpecies",  "name of %ith logical species of %s could not be created", i, GetCharArrayOfString( GetSpeciesNodeName( species ) ) );
        }
        if( ( logicalSpecies = CreateLogicalSpeciesFromSpecies( species, newName, i, len, concentration ) ) == NULL ) {
            return ErrorReport( FAILING, "_CreateLogicalSpecies",  "%ith logical species of %s %s could not be created", i, GetCharArrayOfString( GetSpeciesNodeName( species ) ), buf );
        } 
        
        if( IS_FAILED( ( ret = ir->AddSpecies( ir, (SPECIES*)logicalSpecies ) ) ) ) {
            END_FUNCTION("_CreateLogicalSpecies", ret );
            return ret;
        }
                
        elements[i-1].logicalSpecies = (SPECIES*)logicalSpecies;        
        if( IS_FAILED( ( ret = PutInHashTable( logicalSpecies, sizeof(SPECIES), logicalSpecies, table ) ) ) ) {
            END_FUNCTION("_CreateLogicalSpecies", ret );
            return ret;
        }
    }    
    
    END_FUNCTION("_CreateLogicalSpecies", SUCCESS );
    return ret;
}

static RET_VAL _CreateLogicalConcentrationLevels( ABSTRACTION_METHOD *method, IR *ir, CRITICAL_CONCENTRATION_INFO *info ) {
    RET_VAL ret = SUCCESS;
    int i = 0;
    int len = 0;
    double concentration = 0.0;
    SPECIES *logicalSpecies = NULL;
    KINETIC_LAW *conKineticLaw = NULL;
    REB2SAC_SYMTAB *symtab = NULL;
    CRITICAL_CONCENTRATION_ELEMENT *elements = NULL;
    
    START_FUNCTION("_CreateLogicalConcentrationLevels");
    
    len = info->len;
    elements = info->elements;
        
    symtab = ir->GetGlobalSymtab( ir );
    for( i = 0; i < len; i++ ) {
        logicalSpecies = elements[i].logicalSpecies;
        concentration = elements[i].concentration;
        if( ( conKineticLaw = CreateConcentrationKineticLaw( logicalSpecies, symtab, concentration ) ) == NULL ) {
            return ErrorReport( FAILING, "_CreateLogicalConcentrationLevels",  "could not create a kinetic law for the concentration level of %s", GetCharArrayOfString( GetSpeciesNodeName( logicalSpecies ) ) );
        }
        elements[i].conKineticLaw = conKineticLaw;
    }    
    
    END_FUNCTION("_CreateLogicalConcentrationLevels", SUCCESS );
    return ret;
}



static RET_VAL _HandleReactionM( ABSTRACTION_METHOD *method, IR *ir, IR_EDGE *edge, CRITICAL_CONCENTRATION_INFO *info ) {
    RET_VAL ret = SUCCESS;
    int i = 0;
    int j = 0;
    int len = 0;   
    char buf[REB2SAC_LOGICAL_REACTION_NAME_SIZE];
    double logicalConcentration = 0.0;
    STRING *newName = NULL;
    SPECIES *species;
    SPECIES *logicalSpecies = NULL;
    REACTION *reaction = NULL;
    REACTION *logicalReaction = NULL;
    CRITICAL_CONCENTRATION_ELEMENT *elements = NULL;
    KINETIC_LAW *kineticLaw = NULL;
    KINETIC_LAW *conKineticLaw = NULL;
#ifdef DEBUG
    STRING *kineticLawString = NULL;
#endif
    
    START_FUNCTION("_HandleReactionM");

    reaction = GetReactionInIREdge( edge );
        
    len = info->len;
    elements = info->elements;
    species = info->species;
    
    
    for( i = 0; i < len; i++ ) {
        logicalSpecies = elements[i].logicalSpecies;
        if( ( logicalReaction = ir->CloneReaction( ir, reaction ) ) == NULL ) {
            return ErrorReport( FAILING, "_HandleReactionM", "failed to create reaction %s", buf );
        }
#if 0        
        sprintf( buf, "%s_%s", GetCharArrayOfString( GetReactionNodeName( reaction ) ), GetCharArrayOfString( GetSpeciesNodeName( logicalSpecies ) ) );
        if( ( newName = CreateString( buf ) ) == NULL ) {
            return ErrorReport( FAILING, "_HandleReactionM", "failed to create reaction name string %s", buf );
        }
#endif        
        if( ( newName = _CreateNewReactionID( logicalReaction ) ) == NULL ) {
            return ErrorReport( FAILING, "_HandleReactionM", "failed to create reaction name string" );
        }
        if( IS_FAILED( ( ret = SetReactionNodeName( logicalReaction, newName ) ) ) ) {
            END_FUNCTION("_HandleReactionM", ret );
            return ret;
        }
        TRACE_1("logical reaction %s is created", buf );
        
        logicalConcentration = elements[i].concentration;
        kineticLaw = GetKineticLawInReactionNode( logicalReaction );
        
        conKineticLaw = elements[i].conKineticLaw;
        if( IS_FAILED( ( ret = ReplaceSpeciesWithKineticLawInKineticLaw( kineticLaw, species, conKineticLaw ) ) ) ) {
            END_FUNCTION("_HandleReactionM", ret );
            return ret;
        }  
        for( j = len - 1; j >= 0; j-- ) {
            logicalSpecies = elements[j].logicalSpecies;
            if( i < j ) {
                if( IS_FAILED( ( ret = _AddInhibitorInKineticLaw( logicalReaction, logicalSpecies ) ) ) ) {
                    END_FUNCTION("_HandleReactionM", ret );
                    return ret;
                }
            }
            else {
                if( IS_FAILED( ( ret = _AddActivatorInKineticLaw( logicalReaction, logicalSpecies ) ) ) ) {
                    END_FUNCTION("_HandleReactionM", ret );
                    return ret;
                }
            }
        }
#ifdef DEBUG
        kineticLaw = GetKineticLawInReactionNode( logicalReaction );
        kineticLawString = ToStringKineticLaw( kineticLaw );
        TRACE_2( "kinetic law of logical reaction %s is %s", GetCharArrayOfString( GetReactionNodeName( logicalReaction ) ), GetCharArrayOfString( kineticLawString ) );
        FreeString( &kineticLawString );
#endif
        if( IS_FAILED( ( ret = ir->RemoveModifierInReaction( ir, logicalReaction, species ) ) ) ) {
            END_FUNCTION("_HandleReactionM", ret );
            return ret;
        }
    }
        
    j = sprintf( buf, "%s_no", GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
    for( i = 0; i < len; i++ ) {
        j += sprintf( buf + j, "_%s", GetCharArrayOfString( GetSpeciesNodeName( elements[i].logicalSpecies ) ) );
    }
    if( ( logicalReaction = ir->CloneReaction( ir, reaction ) ) == NULL ) {
        return ErrorReport( FAILING, "_HandleReactionM", "failed to create reaction %s", buf );
    }
    if( ( newName = CreateString( buf ) ) == NULL ) {
        return ErrorReport( FAILING, "_HandleReactionM", "failed to create reaction name string %s", buf );
    }
    if( IS_FAILED( ( ret = SetReactionNodeName( logicalReaction, newName ) ) ) ) {
        END_FUNCTION("_HandleReactionM", ret );
        return ret;
    }
    kineticLaw = GetKineticLawInReactionNode( logicalReaction );
    TRACE_1("logical reaction %s is created", buf );
    if( IS_FAILED( ( ret = ReplaceSpeciesWithRealInKineticLaw( kineticLaw, species, 0.0 ) ) ) ) {
        END_FUNCTION("_HandleReactionM", ret );
        return ret;
    }  
    
    for( j = len - 1; j >= 0; j-- ) {
        logicalSpecies = elements[j].logicalSpecies;
        if( IS_FAILED( ( ret = _AddInhibitorInKineticLaw( logicalReaction, logicalSpecies ) ) ) ) {
            END_FUNCTION("_HandleReactionM", ret );
            return ret;
        }
    }
#ifdef DEBUG
    kineticLaw = GetKineticLawInReactionNode( logicalReaction );
    kineticLawString = ToStringKineticLaw( kineticLaw );
    TRACE_2( "kinetic law of logical reaction %s is %s", GetCharArrayOfString( GetReactionNodeName( logicalReaction ) ), GetCharArrayOfString( kineticLawString ) );
    FreeString( &kineticLawString );
#endif
    if( IS_FAILED( ( ret = ir->RemoveModifierInReaction( ir, logicalReaction, species ) ) ) ) {
        END_FUNCTION("_HandleReactionM", ret );
        return ret;
    }
              
    if( IS_FAILED( ( ret = ir->RemoveReaction( ir, reaction ) ) ) ) {
        END_FUNCTION("_HandleReactionM", ret );
        return ret;
    }

    END_FUNCTION("_HandleReactionM", SUCCESS );
    return ret;
}

static RET_VAL _HandleReactionP( ABSTRACTION_METHOD *method, IR *ir, IR_EDGE *edge, CRITICAL_CONCENTRATION_INFO *info ) {
    RET_VAL ret = SUCCESS;
    int i = 0;
    int j = 0;
    int len = 0;   
    int stoichiometry = 0;
    char buf[REB2SAC_LOGICAL_REACTION_NAME_SIZE];
    double logicalConcentration = 0.0;
    STRING *newName = NULL;
    SPECIES *species;
    SPECIES *logicalSpecies = NULL;
    REACTION *reaction = NULL;
    REACTION *logicalReaction = NULL;
    CRITICAL_CONCENTRATION_ELEMENT *elements = NULL;
    KINETIC_LAW *kineticLaw = NULL;
    KINETIC_LAW *conKineticLaw = NULL;
#ifdef DEBUG
    STRING *kineticLawString = NULL;
#endif
    
    START_FUNCTION("_HandleReactionP");
    
    reaction = GetReactionInIREdge( edge );
    stoichiometry = GetStoichiometryInIREdge( edge );
    
    len = info->len;
    elements = info->elements;
    species = info->species;

    for( i = 0; i < len; i++ ) {
        logicalSpecies = elements[i].logicalSpecies;
        if( ( logicalReaction = ir->CloneReaction( ir, reaction ) ) == NULL ) {
            return ErrorReport( FAILING, "_HandleReactionP", "failed to create reaction %s", buf );
        }
#if 0        
        sprintf( buf, "%s_p_%s", GetCharArrayOfString( GetReactionNodeName( reaction ) ), GetCharArrayOfString( GetSpeciesNodeName( logicalSpecies ) ) );
        if( ( newName = CreateString( buf ) ) == NULL ) {
            return ErrorReport( FAILING, "_HandleReactionP", "failed to create reaction name string %s", buf );
        }
#endif        
        if( ( newName = _CreateNewReactionID( logicalReaction ) ) == NULL ) {
            return ErrorReport( FAILING, "_HandleReactionP", "failed to create reaction name string" );
        }
        if( IS_FAILED( ( ret = SetReactionNodeName( logicalReaction, newName ) ) ) ) {
            END_FUNCTION("_HandleReactionP", ret );
            return ret;
        }
        TRACE_1("logical reaction %s is created", buf );
        
        kineticLaw = GetKineticLawInReactionNode( logicalReaction );
        if( i == 0 ) {
            logicalConcentration = 0.0;
            if( IS_FAILED( ( ret = ReplaceSpeciesWithRealInKineticLaw( kineticLaw, species, 0.0 ) ) ) ) {
                END_FUNCTION("_HandleReactionP", ret );
                return ret;
            }  
            if( IS_FAILED( ( ret = _AddNormalizationInSymbolKineticLaw( logicalReaction, elements[i].conKineticLaw, NULL, stoichiometry ) ) ) ) {
                END_FUNCTION("_HandleReactionP", ret );
                return ret;
            }
        }
        else {
            logicalConcentration = elements[i-1].concentration;
            conKineticLaw = elements[i-1].conKineticLaw;
            if( IS_FAILED( ( ret = ReplaceSpeciesWithKineticLawInKineticLaw( kineticLaw, species, conKineticLaw ) ) ) ) {
                END_FUNCTION("_HandleReactionP", ret );
                return ret;
            }  
            if( IS_FAILED( ( ret = _AddNormalizationInSymbolKineticLaw( logicalReaction, elements[i].conKineticLaw, conKineticLaw, stoichiometry ) ) ) ) {
                END_FUNCTION("_HandleReactionP", ret );
                return ret;
            }
        }
        for( j = len - 1; j >= 0; j-- ) {
            logicalSpecies = elements[j].logicalSpecies;
            if( i != j ) {
                if( IS_FAILED( ( ret = ir->AddModifierEdge( ir, logicalReaction, logicalSpecies, stoichiometry ) ) ) ) {
                    END_FUNCTION("_HandleReactionP", ret );
                    return ret;
                } 
                if( IS_FAILED( ( ret = ir->RemoveProductInReaction( ir, logicalReaction, logicalSpecies ) ) ) ) {
                    END_FUNCTION("_HandleReactionP", ret );
                    return ret;
                }
            }
            else {
                if( IS_FAILED( ( ret = ir->RemoveModifierInReaction( ir, logicalReaction, logicalSpecies ) ) ) ) {
                    END_FUNCTION("_HandleReactionP", ret );
                    return ret;
                }
#if 1                
                if( IS_FAILED( ( ret = _AddInhibitorInKineticLaw( logicalReaction, logicalSpecies ) ) ) ) {
                    END_FUNCTION("_HandleReactionP", ret );
                    return ret;
                }
#endif                
            }
            if( i > j ) {
                if( IS_FAILED( ( ret = _AddActivatorInKineticLaw( logicalReaction, logicalSpecies ) ) ) ) {
                    END_FUNCTION("_HandleReactionP", ret );
                    return ret;
                }
            }
            else if( i < j ) {
                if( IS_FAILED( ( ret = _AddInhibitorInKineticLaw( logicalReaction, logicalSpecies ) ) ) ) {
                    END_FUNCTION("_HandleReactionP", ret );
                    return ret;
                }
            }
        }
        
#ifdef DEBUG
        kineticLaw = GetKineticLawInReactionNode( logicalReaction );
        kineticLawString = ToStringKineticLaw( kineticLaw );
        TRACE_2( "kinetic law of logical reaction %s is %s", GetCharArrayOfString( GetReactionNodeName( logicalReaction ) ), GetCharArrayOfString( kineticLawString ) );
        FreeString( &kineticLawString );
#endif
        if( IS_FAILED( ( ret = ir->RemoveProductInReaction( ir, logicalReaction, species ) ) ) ) {
            END_FUNCTION("_HandleReactionP", ret );
            return ret;
        }
        if( IS_FAILED( ( ret = ir->RemoveModifierInReaction( ir, logicalReaction, species ) ) ) ) {
            END_FUNCTION("_HandleReactionP", ret );
            return ret;
        }
    }
    if( IS_FAILED( ( ret = ir->RemoveReaction( ir, reaction ) ) ) ) {
        END_FUNCTION("_HandleReactionP", ret );
        return ret;
    }

    END_FUNCTION("_HandleReactionP", SUCCESS );
    return ret;
}

static RET_VAL _HandleReactionR( ABSTRACTION_METHOD *method, IR *ir, IR_EDGE *edge, CRITICAL_CONCENTRATION_INFO *info ) {
    RET_VAL ret = SUCCESS;
    int i = 0;
    int j = 0;
    int len = 0;   
    int stoichiometry = 0;
    char buf[REB2SAC_LOGICAL_REACTION_NAME_SIZE];
    double logicalConcentration = 0.0;
    STRING *newName = NULL;
    SPECIES *species;
    SPECIES *logicalSpecies = NULL;
    REACTION *reaction = NULL;
    REACTION *logicalReaction = NULL;
    CRITICAL_CONCENTRATION_ELEMENT *elements = NULL;
    KINETIC_LAW *kineticLaw = NULL;
    KINETIC_LAW *conKineticLaw = NULL;
#ifdef DEBUG
    STRING *kineticLawString = NULL;
#endif
    
    START_FUNCTION("_HandleReactionR");

    reaction = GetReactionInIREdge( edge );
    stoichiometry = GetStoichiometryInIREdge( edge );
    len = info->len;
    elements = info->elements;
    species = info->species;
    
    for( i = 0; i < len; i++ ) {
        logicalSpecies = elements[i].logicalSpecies;
        if( ( logicalReaction = ir->CloneReaction( ir, reaction ) ) == NULL ) {
            return ErrorReport( FAILING, "_HandleReactionR", "failed to create reaction %s", buf );
        }
#if 0        
        sprintf( buf, "%s_r_%s", GetCharArrayOfString( GetReactionNodeName( reaction ) ), GetCharArrayOfString( GetSpeciesNodeName( logicalSpecies ) ) );        
        if( ( newName = CreateString( buf ) ) == NULL ) {
            return ErrorReport( FAILING, "_HandleReactionR", "failed to create reaction name string %s", buf );
        }
#endif        
        if( ( newName = _CreateNewReactionID( logicalReaction ) ) == NULL ) {
            return ErrorReport( FAILING, "_HandleReactionR", "failed to create reaction name string" );
        }
        
        if( IS_FAILED( ( ret = SetReactionNodeName( logicalReaction, newName ) ) ) ) {
            END_FUNCTION("_HandleReactionR", ret );
            return ret;
        }
        TRACE_1("logical reaction %s is created", buf );
        
        logicalConcentration = elements[i].concentration;
        kineticLaw = GetKineticLawInReactionNode( logicalReaction );
        conKineticLaw = elements[i].conKineticLaw;
        if( IS_FAILED( ( ret = ReplaceSpeciesWithKineticLawInKineticLaw( kineticLaw, species, conKineticLaw ) ) ) ) {
            END_FUNCTION("_HandleReactionR", ret );
            return ret;
        }  
        if( i == 0 ) {
            if( IS_FAILED( ( ret = _AddNormalizationInSymbolKineticLaw( logicalReaction, conKineticLaw, NULL, stoichiometry  ) ) ) ) {
                END_FUNCTION("_HandleReactionR", ret );
                return ret;
            }
        }
        else {
            if( IS_FAILED( ( ret = _AddNormalizationInSymbolKineticLaw( logicalReaction, conKineticLaw, elements[i-1].conKineticLaw, stoichiometry  ) ) ) ) {
                END_FUNCTION("_HandleReactionR", ret );
                return ret;
            }
        }
        for( j = len - 1; j >= 0; j-- ) {
            logicalSpecies = elements[j].logicalSpecies;
            if( i != j ) {
                if( IS_FAILED( ( ret = ir->AddModifierEdge( ir, logicalReaction, logicalSpecies, 1 ) ) ) ) {
                    END_FUNCTION("_HandleReactionR", ret );
                    return ret;
                } 
                if( IS_FAILED( ( ret = ir->RemoveReactantInReaction( ir, logicalReaction, logicalSpecies ) ) ) ) {
                    END_FUNCTION("_HandleReactionR", ret );
                    return ret;
                }
            }
            else {
                if( IS_FAILED( ( ret = ir->RemoveModifierInReaction( ir, logicalReaction, logicalSpecies ) ) ) ) {
                    END_FUNCTION("_HandleReactionR", ret );
                    return ret;
                }
#if 1
                if( IS_FAILED( ( ret = _AddActivatorInKineticLaw( logicalReaction, logicalSpecies ) ) ) ) {
                    END_FUNCTION("_HandleReactionR", ret );
                    return ret;
                }
#endif            
            }
            if( i < j ) {
                if( IS_FAILED( ( ret = _AddInhibitorInKineticLaw( logicalReaction, logicalSpecies ) ) ) ) {
                    END_FUNCTION("_HandleReactionR", ret );
                    return ret;
                }
            }
            else if( i > j ) {
                if( IS_FAILED( ( ret = _AddActivatorInKineticLaw( logicalReaction, logicalSpecies ) ) ) ) {
                    END_FUNCTION("_HandleReactionR", ret );
                    return ret;
                }
            }
        }
#ifdef DEBUG
        kineticLaw = GetKineticLawInReactionNode( logicalReaction );
        kineticLawString = ToStringKineticLaw( kineticLaw );
        TRACE_2( "kinetic law of logical reaction %s is %s", GetCharArrayOfString( GetReactionNodeName( logicalReaction ) ), GetCharArrayOfString( kineticLawString ) );
        FreeString( &kineticLawString );
#endif
        if( IS_FAILED( ( ret = ir->RemoveReactantInReaction( ir, logicalReaction, species ) ) ) ) {
            END_FUNCTION("_HandleReactionR", ret );
            return ret;
        }
        if( IS_FAILED( ( ret = ir->RemoveModifierInReaction( ir, logicalReaction, species ) ) ) ) {
            END_FUNCTION("_HandleReactionR", ret );
            return ret;
        }
    }
    if( IS_FAILED( ( ret = ir->RemoveReaction( ir, reaction ) ) ) ) {
        END_FUNCTION("_HandleReactionR", ret );
        return ret;
    }

    END_FUNCTION("_HandleReactionR", SUCCESS );
    return ret;
}


static RET_VAL _AddInhibitorInKineticLaw( REACTION *reaction, SPECIES *inhibitor ) {
    RET_VAL ret = SUCCESS;
    KINETIC_LAW *kineticLaw = NULL;
    KINETIC_LAW *inhibition = NULL;
    
    START_FUNCTION("_AddInhibitorInKineticLaw");
    
    kineticLaw = GetKineticLawInReactionNode( reaction );
#if 0    
    if( ( inhibition = CreateOpKineticLaw( KINETIC_LAW_OP_PLUS, CreateRealValueKineticLaw( 1.0 ), CreateSpeciesKineticLaw( inhibitor ) ) ) == NULL ) {
        return ErrorReport( FAILING, "_AddInhibitorInKineticLaw", 
            "could not create 1 + %s in %s", GetCharArrayOfString( GetSpeciesNodeName( inhibitor ) ), GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
    }
    
    if( ( inhibition = CreateOpKineticLaw( KINETIC_LAW_OP_DIVIDE, CreateRealValueKineticLaw( 1.0 ), inhibition ) ) == NULL ) {
        return ErrorReport( FAILING, "_AddInhibitorInKineticLaw", 
            "could not create 1 / (1 + %s) in %s", GetCharArrayOfString( GetSpeciesNodeName( inhibitor ) ), GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
    }
    
    if( ( kineticLaw = CreateOpKineticLaw( KINETIC_LAW_OP_TIMES, inhibition, kineticLaw ) ) == NULL ) {
        return ErrorReport( FAILING, "_AddInhibitorInKineticLaw", 
            "could not add 1 / (1 + %s) in %s", GetCharArrayOfString( GetSpeciesNodeName( inhibitor ) ), GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
    }
#else 
    if( ( inhibition = CreateOpKineticLaw( KINETIC_LAW_OP_MINUS, CreateRealValueKineticLaw( 1.0 ), CreateSpeciesKineticLaw( inhibitor ) ) ) == NULL ) {
        return ErrorReport( FAILING, "_AddInhibitorInKineticLaw", 
            "could not create 1 - %s in %s", GetCharArrayOfString( GetSpeciesNodeName( inhibitor ) ), GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
    }
    if( ( kineticLaw = CreateOpKineticLaw( KINETIC_LAW_OP_TIMES, inhibition, kineticLaw ) ) == NULL ) {
        return ErrorReport( FAILING, "_AddInhibitorInKineticLaw", 
            "could not add (1 - %s) in %s", GetCharArrayOfString( GetSpeciesNodeName( inhibitor ) ), GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
    }
#endif
    
    if( IS_FAILED( ( ret = SetKineticLawInReactionNode( reaction, kineticLaw ) ) ) ) {
        END_FUNCTION("_AddInhibitorInKineticLaw", ret );
        return ret;
    }
    
    END_FUNCTION("_AddInhibitorInKineticLaw", SUCCESS );
    return ret;
}

static RET_VAL _AddActivatorInKineticLaw( REACTION *reaction, SPECIES *activator ) {
    RET_VAL ret = SUCCESS;
    KINETIC_LAW *kineticLaw = NULL;
    KINETIC_LAW *activation = NULL;
    
    START_FUNCTION("_AddActivatorInKineticLaw");
    
    kineticLaw = GetKineticLawInReactionNode( reaction );
    
#if 0    
    if( ( activation = CreateOpKineticLaw( KINETIC_LAW_OP_PLUS, CreateRealValueKineticLaw( 1.0 ), CreateSpeciesKineticLaw( activator ) ) ) == NULL ) {
        return ErrorReport( FAILING, "_AddActivatorInKineticLaw", 
            "could not create 1 + %s in %s", GetCharArrayOfString( GetSpeciesNodeName( activator ) ), GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
    }
    
    if( ( activation = CreateOpKineticLaw( KINETIC_LAW_OP_DIVIDE, CreateSpeciesKineticLaw( activator ), activation ) ) == NULL ) {
        return ErrorReport( FAILING, "_AddActivatorInKineticLaw", 
            "could not create %s / (1 + %s) in %s", 
            GetCharArrayOfString( GetSpeciesNodeName( activator ) ), GetCharArrayOfString( GetSpeciesNodeName( activator ) ), GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
    }
    
    if( ( kineticLaw = CreateOpKineticLaw( KINETIC_LAW_OP_TIMES, activation, kineticLaw ) ) == NULL ) {
        return ErrorReport( FAILING, "_AddActivatorInKineticLaw", 
            "could not add %s / (1 + %s) in %s", 
            GetCharArrayOfString( GetSpeciesNodeName( activator ) ), GetCharArrayOfString( GetSpeciesNodeName( activator ) ), GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
    }
#else 
    if( ( activation = CreateSpeciesKineticLaw( activator ) ) == NULL ) {
        return ErrorReport( FAILING, "_AddActivatorInKineticLaw", 
            "could not create %s in %s", GetCharArrayOfString( GetSpeciesNodeName( activator ) ), GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
    }
    if( ( kineticLaw = CreateOpKineticLaw( KINETIC_LAW_OP_TIMES, activation, kineticLaw ) ) == NULL ) {
        return ErrorReport( FAILING, "_AddActivatorInKineticLaw", 
            "could not add (%s) in %s", 
            GetCharArrayOfString( GetSpeciesNodeName( activator ) ), GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
    }
#endif    
    if( IS_FAILED( ( ret = SetKineticLawInReactionNode( reaction, kineticLaw ) ) ) ) {
        END_FUNCTION("_AddActivatorInKineticLaw", ret );
        return ret;
    }
    
    END_FUNCTION("_AddActivatorInKineticLaw", SUCCESS );
    return ret;
}

static RET_VAL _AddNormalizationInKineticLaw( REACTION *reaction, double criticalConDelta ) {
    RET_VAL ret = SUCCESS;
    KINETIC_LAW *kineticLaw = NULL;   
    KINETIC_LAW *normalization = NULL;
    
    START_FUNCTION("_AddNormalizationInKineticLaw");
    
    kineticLaw = GetKineticLawInReactionNode( reaction );
    
    if( ( normalization = CreateRealValueKineticLaw( 1.0 / criticalConDelta ) ) == NULL ) {
        return ErrorReport( FAILING, "_AddNormalizationInKineticLaw", "could not create %f in %s", 1.0 / criticalConDelta, GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
    }
    
    TRACE_2( "adding normalization term %f in %s", 1.0 / criticalConDelta, GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
    if( ( kineticLaw = CreateOpKineticLaw( KINETIC_LAW_OP_TIMES, normalization, kineticLaw ) ) == NULL ) {
        return ErrorReport( FAILING, "_AddNormalizationInKineticLaw", "could not add %f in %s", 1.0 / criticalConDelta, GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
    }
    
    if( IS_FAILED( ( ret = SetKineticLawInReactionNode( reaction, kineticLaw ) ) ) ) {
        END_FUNCTION("_AddNormalizationInKineticLaw", ret );
        return ret;
    }
    
    END_FUNCTION("_AddNormalizationInKineticLaw", SUCCESS );
    return ret;
}

static RET_VAL _AddNormalizationInSymbolKineticLaw( REACTION *reaction, KINETIC_LAW *value2, KINETIC_LAW *value1, int stoichiometry ) {
    RET_VAL ret = SUCCESS;
    KINETIC_LAW *kineticLaw = NULL;   
    KINETIC_LAW *normalization = NULL;
    STRING *string = NULL;
    
    START_FUNCTION("_AddNormalizationInKineticLaw");
    
    kineticLaw = GetKineticLawInReactionNode( reaction );
    if( value1 == NULL ) {
        if( ( normalization = CloneKineticLaw( value2 ) ) == NULL ) {
            return ErrorReport( FAILING, "_AddNormalizationInKineticLaw", "could not create normalization" );
        }
    }
    else {
        if( ( normalization = CreateOpKineticLaw( KINETIC_LAW_OP_MINUS, CloneKineticLaw( value2 ), CloneKineticLaw( value1 ) ) ) == NULL ) {
            return ErrorReport( FAILING, "_AddNormalizationInKineticLaw", "could not create normalization" );
        }
    }
    if( ( normalization = CreateOpKineticLaw( KINETIC_LAW_OP_DIVIDE, CreateRealValueKineticLaw( (double)stoichiometry ), normalization ) ) == NULL ) {
        return ErrorReport( FAILING, "_AddNormalizationInKineticLaw", "could not create normalization" );
    }
#ifdef DEBUG
    string = ToStringKineticLaw( normalization );
    printf( "adding normalization term %s in %s", GetCharArrayOfString( string ), GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
    FreeString( &string );
#endif
    if( ( kineticLaw = CreateOpKineticLaw( KINETIC_LAW_OP_TIMES, normalization, kineticLaw ) ) == NULL ) {
        return ErrorReport( FAILING, "_AddNormalizationInKineticLaw", "could not create normalization" );
    }
    
    if( IS_FAILED( ( ret = SetKineticLawInReactionNode( reaction, kineticLaw ) ) ) ) {
        END_FUNCTION("_AddNormalizationInKineticLaw", ret );
        return ret;
    }
    
    END_FUNCTION("_AddNormalizationInKineticLaw", SUCCESS );
    return ret;
}


static RET_VAL _CleanCriticalConInfo( CRITICAL_CONCENTRATION_INFO *info ) {
    RET_VAL ret = SUCCESS;
    int i = 0;
    int len = 0;
    CRITICAL_CONCENTRATION_ELEMENT *elements = NULL;
    
    START_FUNCTION("_CleanCriticalConInfo");
    
    len = info->len;
    elements = info->elements;
    for( i = 0; i < len; i++ ) {
        FreeKineticLaw( &(elements[i].conKineticLaw) );
    }
    FREE( info->elements );
    
    END_FUNCTION("_CleanCriticalConInfo", SUCCESS );
    return ret;
}

static STRING *_CreateNewReactionID( REACTION *newReaction ) {
    IR_EDGE *edge = NULL;
    SPECIES *species = NULL;
    LOGICAL_SPECIES *logicalSpecies;
    LINKED_LIST *edges = NULL;
    char buf[REB2SAC_LOGICAL_REACTION_NAME_SIZE];
    STRING *newID = NULL;
            
    edges = GetReactantEdges( newReaction );
    if( GetLinkedListSize( edges ) > 0 ) {
        edge = GetHeadEdge( edges );
        species = GetSpeciesInIREdge( edge );
        if( IsLogicalSpecies( species ) ) {
            sprintf( buf, "Deg_%s__%X", 
                     GetCharArrayOfString( GetOriginalSpeciesName( (LOGICAL_SPECIES*)species ) ), 
                     (int)newReaction );
        }
        else {
            sprintf( buf, "Deg_%s__%X", 
                     GetCharArrayOfString( GetSpeciesNodeName( species ) ), 
                     (int)newReaction );
        }
        newID = CreateString( buf );
        return newID;
    }
    
    edges = GetProductEdges( newReaction );
    if( GetLinkedListSize( edges ) > 0 ) {
        edge = GetHeadEdge( edges );
        species = GetSpeciesInIREdge( edge );
        if( IsLogicalSpecies( species ) ) {
            sprintf( buf, "Prod_%s__%X", 
                     GetCharArrayOfString( GetOriginalSpeciesName( (LOGICAL_SPECIES*)species ) ), 
                     (int)newReaction );
        }
        else {
            sprintf( buf, "Prod_%s__%X", 
                     GetCharArrayOfString( GetSpeciesNodeName( species ) ), 
                     (int)newReaction );
        }
        newID = CreateString( buf );
        return newID;
    }

    return NULL;
}

static STRING *_CreateLogicalSpeciesID( SPECIES *originalSpecies, int order ) {
    char buf[REB2SAC_LOGICAL_SPECIES_NAME_SIZE];
    STRING *newID = NULL;
    
    sprintf( buf, "%s__%i", GetCharArrayOfString( GetSpeciesNodeName( originalSpecies ) ), order );
    newID = CreateString( buf );
    return newID;    
}



