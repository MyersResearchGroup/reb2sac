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
#include "default_simulation_run_termination_decider.h"

static BOOL _IsTerminationConditionMet( DEFAULT_SIMULATION_RUN_TERMINATION_DECIDER *decider, REACTION *reaction, double time );
static RET_VAL _Destroy( DEFAULT_SIMULATION_RUN_TERMINATION_DECIDER *decider );
static RET_VAL _CreateConditions( BACK_END_PROCESSOR *backend, DEFAULT_SIMULATION_RUN_TERMINATION_DECIDER *decider );
static DEFAULT_SIMULATION_RUN_TERMINATION_CONDITION  *_CreateCondition(DEFAULT_SIMULATION_RUN_TERMINATION_DECIDER *decider, char *valueString );
static RET_VAL _Report( DEFAULT_SIMULATION_RUN_TERMINATION_DECIDER *decider, FILE *file );
    

DLLSCOPE SIMULATION_RUN_TERMINATION_DECIDER * STDCALL CreateDefaultSimulationRunTerminationDecider( 
        BACK_END_PROCESSOR *backend, SPECIES **speciesArray, int size, 
        REACTION **reactionArray, int reactionSize, double timeLimit ) {
    
    DEFAULT_SIMULATION_RUN_TERMINATION_DECIDER *decider = NULL;
    
    START_FUNCTION("CreateDefaultSimulationRunTerminationDecider");

    if( ( decider = (DEFAULT_SIMULATION_RUN_TERMINATION_DECIDER*)MALLOC( sizeof(DEFAULT_SIMULATION_RUN_TERMINATION_DECIDER) ) ) == NULL ) {
        TRACE_0("failed to allocate memory for DEFAULT_SIMULATION_RUN_TERMINATION_DECIDER");
        return NULL;
    }
    decider->speciesArray = speciesArray;
    decider->size = size;
    decider->timeLimit = timeLimit;
    
    decider->IsTerminationConditionMet = 
        (BOOL(*)(SIMULATION_RUN_TERMINATION_DECIDER *, REACTION *, double))_IsTerminationConditionMet;        
    decider->Destroy = (RET_VAL(*)(SIMULATION_RUN_TERMINATION_DECIDER *))_Destroy;
    decider->Report = (RET_VAL(*)(SIMULATION_RUN_TERMINATION_DECIDER *, FILE *))_Report;
    
    if( IS_FAILED( _CreateConditions( backend, decider ) ) ) {
        END_FUNCTION("CreateDefaultSimulationRunTerminationDecider", FAILING );
        return NULL;
    }    
         
    END_FUNCTION("CreateDefaultSimulationRunTerminationDecider", SUCCESS );
    return (SIMULATION_RUN_TERMINATION_DECIDER*)decider;
}

static BOOL _IsTerminationConditionMet( DEFAULT_SIMULATION_RUN_TERMINATION_DECIDER *decider, REACTION *reaction, double time ) {
    int i = 0;
    int size = 0;
    double quantity = 0;
    SPECIES **speciesArray = NULL;
    DEFAULT_SIMULATION_RUN_TERMINATION_CONDITION *condition = NULL;
    DEFAULT_SIMULATION_RUN_TERMINATION_CONDITION **conditions = NULL;
            
    if( time >= decider->timeLimit ) {
        return TRUE;
    }
    
    size = decider->conditionSize;
    speciesArray = decider->speciesArray;
    conditions = decider->conditions;
    
    
    for( i = 0; i < size; i++ ) {
        condition = conditions[i];
        TRACE_3("condition: (index %i), %i, %g", 
            condition->speciesIndex, condition->conditionType, condition->value );
        if( condition->inAmount ) {
            quantity = GetAmountInSpeciesNode( speciesArray[condition->speciesIndex] );
        }
        else {
            quantity = GetConcentrationInSpeciesNode( speciesArray[condition->speciesIndex] );
        }
        switch( condition->conditionType ) {
            case DEFAULT_SIMULATION_RUN_TERMINATION_CONDITION_TYPE_GE:
                if( quantity >= condition->value ) {
                    condition->count++;
                    return TRUE;
                }
            break;
            
            case DEFAULT_SIMULATION_RUN_TERMINATION_CONDITION_TYPE_GT:
                if( quantity > condition->value ) {
                    condition->count++;
                    return TRUE;
                }
            break;
            
            case DEFAULT_SIMULATION_RUN_TERMINATION_CONDITION_TYPE_EQ:
                if( IS_REAL_EQUAL( quantity, condition->value ) ) {
                    condition->count++;
                    return TRUE;
                }
            break;
            
            case DEFAULT_SIMULATION_RUN_TERMINATION_CONDITION_TYPE_LE:
                if( quantity <= condition->value ) {
                    condition->count++;
                    return TRUE;
                }
            break;
            
            case DEFAULT_SIMULATION_RUN_TERMINATION_CONDITION_TYPE_LT:
                if( quantity < condition->value ) {
                    condition->count++;
                    return TRUE;
                }
            break;
        }
    }
    return FALSE;        
}

static RET_VAL _Report( DEFAULT_SIMULATION_RUN_TERMINATION_DECIDER *decider, FILE *file ) {
    RET_VAL ret = SUCCESS;
    int i = 0;
    int terminationCount = 0;
    int size = decider->conditionSize;
    SPECIES *species = NULL;
    SPECIES **speciesArray = decider->speciesArray;
    DEFAULT_SIMULATION_RUN_TERMINATION_CONDITION *condition = NULL;    
    DEFAULT_SIMULATION_RUN_TERMINATION_CONDITION **conditions = decider->conditions;    
    
    for( i = 0; i < size; i++ ) {
        condition = conditions[i];
        terminationCount += condition->count;        
    }

    fprintf( file, "The total termination count: %i" NEW_LINE, terminationCount );         
    for( i = 0; i < size; i++ ) {
        condition = conditions[i];
        species = speciesArray[condition->speciesIndex];
        fprintf( file, "%s %s %f: %i times, %f" NEW_LINE, 
            GetCharArrayOfString( GetSpeciesNodeName( species ) ), 
            condition->typeString, 
            condition->value,
            condition->count,
            (double)(condition->count) / (double)terminationCount );         
    }
        
    return ret;
}


static RET_VAL _Destroy( DEFAULT_SIMULATION_RUN_TERMINATION_DECIDER *decider ) {
    RET_VAL ret = SUCCESS;
    int i = 0;
    int size = decider->conditionSize;
    DEFAULT_SIMULATION_RUN_TERMINATION_CONDITION **conditions = decider->conditions;    
    
    for( i = 0; i < size; i++ ) {
        FREE( conditions[i] );
    }            
    FREE( decider->conditions );
    FREE( decider );
    
    return ret;
}

static RET_VAL _CreateConditions( BACK_END_PROCESSOR *backend, DEFAULT_SIMULATION_RUN_TERMINATION_DECIDER *decider ) {
    RET_VAL ret = SUCCESS;
    int i = 0;
    int size = 0;
    int num = 0;
    char buf[512];
    char *valueString = NULL;
    LINKED_LIST *list = NULL;
    DEFAULT_SIMULATION_RUN_TERMINATION_CONDITION *condition = NULL;    
    DEFAULT_SIMULATION_RUN_TERMINATION_CONDITION **conditions = NULL;    
    COMPILER_RECORD_T *compRec = backend->record;
    REB2SAC_PROPERTIES *properties = NULL;

    if( ( list = CreateLinkedList() ) == NULL ) {
        return ErrorReport( FAILING, "_CreateConditions", "Could not create a linked list for termination conditions" );
    }
            
    properties = compRec->properties;
    for( num = 1; ; num++ ) {        
        sprintf( buf, "%s%i", SIMULATION_RUN_TERMINATION_CONDITION_KEY_PREFIX, num );
        if( ( valueString = properties->GetProperty( properties, buf ) ) == NULL ) {
            break;
        }
        if( ( condition = _CreateCondition( decider, valueString ) ) == NULL ) {
            return ErrorReport( FAILING, "_CreateConditions", "condition could not be created" );
        }
        if( IS_FAILED( ( ret = AddElementInLinkedList( (CADDR_T)condition, list ) ) ) ) {
            return ret;
        }
    }
    
    if( ( size = GetLinkedListSize( list ) ) == 0 ) {
        DeleteLinkedList( &list );
        decider->conditions = NULL;
        decider->conditionSize = 0;
        return ret;
    }
    if( ( conditions = (DEFAULT_SIMULATION_RUN_TERMINATION_CONDITION**)MALLOC( size * sizeof(DEFAULT_SIMULATION_RUN_TERMINATION_CONDITION*) ) ) == NULL ) {
        return ErrorReport( FAILING, "_CreateConditions", "conditions array could not be created" );
    }
    i = 0;
    ResetCurrentElement( list );
    while( ( condition = (DEFAULT_SIMULATION_RUN_TERMINATION_CONDITION*)GetNextFromLinkedList( list ) ) != NULL ) {
        conditions[i] = condition;
        i++;
    }    
    DeleteLinkedList( &list );    
    decider->conditions = conditions;
    decider->conditionSize = size;
    
    return ret;
}

static DEFAULT_SIMULATION_RUN_TERMINATION_CONDITION  *_CreateCondition(DEFAULT_SIMULATION_RUN_TERMINATION_DECIDER *decider, char *valueString ) {
    char buf[1024];
    char *speciesName = NULL;
    char *typeString = NULL;
    char *quantityString = NULL;
    char *tmp = NULL;
    BOOL inAmount = TRUE;
    int size = decider->size;
    int conditionType = 0;
    int i = 0;
    int len = 0;
    double value = 0.0;
    SPECIES *species = NULL;
    SPECIES **speciesArray = decider->speciesArray;
    DEFAULT_SIMULATION_RUN_TERMINATION_CONDITION *condition = NULL;
    
    strcpy( buf, valueString );
    len = strlen( buf ) - 1;
    for( i = 0; i < len; i++ ) {
        if( buf[i] == '.' ) {
            buf[i] = '\0';
            if( speciesName == NULL ) {
                speciesName = buf;
                quantityString = buf + (i+1);
            }
            else if( typeString == NULL ) {            
                typeString = buf + (i+1);
            }
            else {
                tmp = buf + (i+1);
                break;
            }
        }
    }
    if( ( speciesName == NULL ) || 
        ( quantityString == NULL ) || 
        ( typeString == NULL ) || 
        ( tmp == NULL ) ) {
        TRACE_1("invalid condition %s", valueString );
        return NULL;
    }    
    
    for( i = 0; i < size; i++ ) {
        species = speciesArray[i];
        if( strcmp( GetCharArrayOfString( GetSpeciesNodeName( species ) ), speciesName ) == 0 ) {
            break;
        }    
    }
    TRACE_1("species used in condition is %s", speciesName );
    if( i >= size ) {
        TRACE_1("invalid species name for a condition %s", valueString );
        return NULL;
    }
      
    if( strcmp( quantityString, SIMULATION_RUN_TERMINATION_CONDITION_IN_AMOUNT ) == 0 ) {
        inAmount = TRUE;
    }
    else if( strcmp( quantityString, SIMULATION_RUN_TERMINATION_CONDITION_IN_CONCENTRATION ) == 0 ) {
        inAmount = FALSE;
    }
    else {
        TRACE_1("invalid condition quantity %s", quantityString );
        return NULL;
    }  
    
    if( strlen( typeString ) != 2 ) {
        TRACE_1("invalid condition type %s", typeString );
        return NULL;
    }        
    switch( typeString[0] ) {
        case 'g':
            if( strcmp( typeString, DEFAULT_SIMULATION_RUN_TERMINATION_CONDITION_TYPE_GE_STRING ) == 0 ) {
                conditionType = DEFAULT_SIMULATION_RUN_TERMINATION_CONDITION_TYPE_GE;
                typeString = ">=";
            }
            else if( strcmp( typeString, DEFAULT_SIMULATION_RUN_TERMINATION_CONDITION_TYPE_GT_STRING ) == 0 ) {
                conditionType = DEFAULT_SIMULATION_RUN_TERMINATION_CONDITION_TYPE_GT;
                typeString = ">";
            }
            else {
                TRACE_1("invalid condition type %s", typeString );
                return NULL;
            }
        break;
        
        case 'e':
            if( strcmp( typeString, DEFAULT_SIMULATION_RUN_TERMINATION_CONDITION_TYPE_EQ_STRING ) == 0 ) {
                conditionType = DEFAULT_SIMULATION_RUN_TERMINATION_CONDITION_TYPE_EQ;
                typeString = "=";
            }
            else {
                TRACE_1("invalid condition type %s", typeString );
                return NULL;
            }
        break;
        
        case 'l':
            if( strcmp( typeString, DEFAULT_SIMULATION_RUN_TERMINATION_CONDITION_TYPE_LE_STRING ) == 0 ) {
                conditionType = DEFAULT_SIMULATION_RUN_TERMINATION_CONDITION_TYPE_LE;
                typeString = "<=";
            }
            else if( strcmp( typeString, DEFAULT_SIMULATION_RUN_TERMINATION_CONDITION_TYPE_LT_STRING ) == 0 ) {
                conditionType = DEFAULT_SIMULATION_RUN_TERMINATION_CONDITION_TYPE_LT;
                typeString = "<";
            }
            else {
                TRACE_1("invalid condition type %s", typeString );
                return NULL;
            }
        break;
        
        default:
            TRACE_1("invalid condition type %s", typeString );
        return NULL;
    }
    if( IS_FAILED( StrToFloat( &value, tmp ) ) ) {
        TRACE_1("invalid number %s", tmp );
        return NULL;
    }
    
    if( ( condition = (DEFAULT_SIMULATION_RUN_TERMINATION_CONDITION*)MALLOC( sizeof(DEFAULT_SIMULATION_RUN_TERMINATION_CONDITION) ) ) == NULL ) {
        TRACE_0("failed to allocate space for DEFAULT_SIMULATION_RUN_TERMINATION_CONDITION" );
        return NULL;
    }
    condition->conditionType = conditionType;
    condition->speciesIndex = i;
    condition->value = value;
    condition->inAmount = inAmount;
    condition->count = 0;
    condition->typeString = typeString;
    TRACE_4("condition: %s (index %i), %s, %g is created", speciesName, i, typeString, value );
    return condition;
}
