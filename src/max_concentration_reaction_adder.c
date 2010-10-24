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

typedef struct {
    int maxAmount;
    int minAmount;
    SPECIES *species;
} MAX_CON_REACTION_ADDER_RECORD;

static char * _GetMaxConcentrationReactionAdditionMethodID( ABSTRACTION_METHOD *method );
static RET_VAL _ApplyMaxConcentrationReactionAdditionMethod( ABSTRACTION_METHOD *method, IR *ir );      
static BOOL _IsConditionSatisfied( ABSTRACTION_METHOD *method, SPECIES *species, MAX_CON_REACTION_ADDER_RECORD *rec ); 
static RET_VAL _DoTransformation( ABSTRACTION_METHOD *method, IR *ir, MAX_CON_REACTION_ADDER_RECORD *rec, double reactionRate );
static double _GetOscillationReactionRate( ABSTRACTION_METHOD_MANAGER *manager );

ABSTRACTION_METHOD *MaxConcentrationReactionAdditionAbstractionMethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager ) {
    static ABSTRACTION_METHOD method;
    static double reactionRate = 0.0;
    
    START_FUNCTION("MaxConcentrationReactionAdditionAbstractionMethodConstructor");

    if( method.manager == NULL ) {
        method.manager = manager;
        method.GetID = _GetMaxConcentrationReactionAdditionMethodID;
        method.Apply = _ApplyMaxConcentrationReactionAdditionMethod;
    }

    reactionRate = _GetOscillationReactionRate( manager );
    method._internal1 = (CADDR_T)(&reactionRate);
    
    TRACE_0( "MaxConcentrationReactionAdditionAbstractionMethodConstructor invoked" );

    END_FUNCTION("MaxConcentrationReactionAdditionAbstractionMethodConstructor", SUCCESS );
    return &method;
}



static char * _GetMaxConcentrationReactionAdditionMethodID( ABSTRACTION_METHOD *method ) {
    START_FUNCTION("_GetMaxConcentrationReactionAdditionMethodID");
    
    END_FUNCTION("_GetMaxConcentrationReactionAdditionMethodID", SUCCESS );
    return "max-concentration-reaction-adder";
}



static RET_VAL _ApplyMaxConcentrationReactionAdditionMethod( ABSTRACTION_METHOD *method, IR *ir ) {
    RET_VAL ret = SUCCESS;
    SPECIES *species = NULL;
    LINKED_LIST *speciesList = NULL;
    double reactionRate = (*(double*)(method->_internal1));
    static MAX_CON_REACTION_ADDER_RECORD rec;
    
    START_FUNCTION("_ApplyMaxConcentrationReactionAdditionMethod");

    speciesList = ir->GetListOfSpeciesNodes( ir );
    
    ResetCurrentElement( speciesList );    
    while( ( species = (SPECIES*)GetNextFromLinkedList( speciesList ) ) != NULL ) {
        if( _IsConditionSatisfied( method, species, &rec ) ) {
            if( IS_FAILED( ( ret = _DoTransformation( method, ir, &rec, reactionRate ) ) ) ) {
                END_FUNCTION("_ApplyMaxConcentrationReactionAdditionMethod", ret );
                return ret;
            }
        }            
    }
            
    END_FUNCTION("_ApplyMaxConcentrationReactionAdditionMethod", SUCCESS );
    return ret;
}      

static BOOL _IsConditionSatisfied( ABSTRACTION_METHOD *method, SPECIES *species, MAX_CON_REACTION_ADDER_RECORD *rec ) {    
    char buf[4096];
    char *valueString = NULL;
    int maxAmount = 0;
    int minAmount = 0;
    ABSTRACTION_METHOD_MANAGER *manager = NULL;
    COMPILER_RECORD_T *record = NULL;
    REB2SAC_PROPERTIES *properties = NULL;

    manager = method->manager;
    record = manager->GetCompilerRecord( manager );
    properties = record->properties;
    
    sprintf( buf, "%s%s", 
        REB2SAC_MAX_SPECIES_OSCILLATION_AMOUNT_KEY_PREFIX,
        GetCharArrayOfString( GetSpeciesNodeName( species ) ) );
    
    if( ( valueString = properties->GetProperty( properties, buf ) ) == NULL ) {
        return FALSE;
    }
    
    if( IS_FAILED( StrToUINT32( &maxAmount, valueString ) ) ) {
        return FALSE;
    } 
    
    sprintf( buf, "%s%s", 
             REB2SAC_MIN_SPECIES_OSCILLATION_AMOUNT_KEY_PREFIX,
             GetCharArrayOfString( GetSpeciesNodeName( species ) ) );
    
    if( ( valueString = properties->GetProperty( properties, buf ) ) == NULL ) {
        return FALSE;
    }
    
    if( IS_FAILED( StrToUINT32( &minAmount, valueString ) ) ) {
        return FALSE;
    } 
    
    TRACE_3("species %s oscillates  from %i to %i", 
            GetCharArrayOfString( GetSpeciesNodeName( species ) ),
            minAmount,
            maxAmount );
    rec->species = species;
    rec->maxAmount = maxAmount;
    rec->minAmount = minAmount;
    
    return TRUE;
}

 
static RET_VAL _DoTransformation( ABSTRACTION_METHOD *method, IR *ir, MAX_CON_REACTION_ADDER_RECORD *rec, double reactionRate ) {
    RET_VAL ret = SUCCESS;
    int maxAmount = rec->maxAmount;
    int minAmount = rec->minAmount;
    SPECIES *species = rec->species;
    KINETIC_LAW *kineticLaw = NULL;
    char buf[4096];
    STRING *newName = NULL;
    REACTION *newReaction = NULL;
    
    sprintf( buf, "%s_max_oscillation", 
             GetCharArrayOfString( GetSpeciesNodeName( species ) ) 
           );    
    if( ( newReaction = ir->CreateReaction( ir, buf ) ) == NULL ) {
        return ErrorReport( FAILING, "_DoTransformation", "could not create a new oscillation reaction for %s", GetCharArrayOfString( GetSpeciesNodeName( species ) ) );
    }
    
    if( ( kineticLaw = CreateRealValueKineticLaw( reactionRate ) ) == NULL ) {
        return ErrorReport( FAILING, "_DoTransformation", "could not create a kinetic law of new oscillation reaction for %s", GetCharArrayOfString( GetSpeciesNodeName( species ) ) );
    }
    if( IS_FAILED( ( ret = SetKineticLawInReactionNode( newReaction, kineticLaw ) ) ) ) {
        return ret;
    }
    if( IS_FAILED( ( ret = SetReactionReversibleInReactionNode( newReaction, FALSE ) ) ) ) {
        return ret;
    }
    
    if( IS_FAILED( ( ret = ir->AddReactantEdge(  ir, newReaction, species, maxAmount, NULL, TRUE ) ) ) ) {
        return ret;
    } 
    if( IS_FAILED( ( ret = ir->AddProductEdge( ir, newReaction, species, minAmount, NULL, TRUE ) ) ) ) {
        return ret;
    } 
    
    return SUCCESS;
}

static double _GetOscillationReactionRate( ABSTRACTION_METHOD_MANAGER *manager ) {
    char *valueString = NULL;
    double reactionRate = 0.0;
    COMPILER_RECORD_T *record = NULL;
    REB2SAC_PROPERTIES *properties = NULL;

    record = manager->GetCompilerRecord( manager );
    properties = record->properties;

    if( ( valueString = properties->GetProperty( properties, REB2SAC_MAX_SPECIES_OSCILLATION_REACTION_RATE_KEY ) ) == NULL ) {
        return DEFAULT_REB2SAC_MAX_SPECIES_OSCILLATION_REACTION_RATE;
    }
    if( IS_FAILED( StrToFloat( &reactionRate, valueString ) ) ) {
        return DEFAULT_REB2SAC_MAX_SPECIES_OSCILLATION_REACTION_RATE;
    }
    return reactionRate;
}


