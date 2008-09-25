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
#include "ir2ctmc_transformer.h"
#include "ctmc_transformation_checker.h"
#include "species_critical_level_generator.h"


static CTMC *_Generate( IR2CTMC_TRANSFORMER *transformer );
static int _GetCriticalLevelArraySize( IR2CTMC_TRANSFORMER *transformer );
static SPECIES_CRITICAL_LEVEL **_GetCriticalLevelArray( IR2CTMC_TRANSFORMER *transformer );
static RET_VAL _ResetCurrentLevelArray( IR2CTMC_TRANSFORMER *transformer );
static RET_VAL _IncrementCurrentLevelArray( IR2CTMC_TRANSFORMER *transformer );
static double *_GetCurrentLevelArray( IR2CTMC_TRANSFORMER *transformer );
static RET_VAL _ResetCurrentArray( int speciesSize, int *currentArray );
static RET_VAL _IncrementCurrentArray( int speciesSize, int *currentArray, SPECIES_CRITICAL_LEVEL **criticalLevelArray );
static RET_VAL _UpdateCurrenLevelArray( 
                int speciesSize, 
                int *currentArray, 
                double *currentLevelArray, 
                SPECIES_CRITICAL_LEVEL **criticalLevelArray ); 
static RET_VAL _Print( IR2CTMC_TRANSFORMER *transformer, FILE *file );
static RET_VAL _PrintState( FILE *file, int state, int size, double *currentLevelArray );

static RET_VAL _ReleaseResource( IR2CTMC_TRANSFORMER *transformer );



static SPECIES_CRITICAL_LEVEL **_CreateCriticalLevelArray( LINKED_LIST *speciesList, REB2SAC_PROPERTIES *properties );
static RET_VAL _InitTransformer( IR2CTMC_TRANSFORMER *transformer, IR *ir, REB2SAC_PROPERTIES *properties );
static int _FindNumberOfStates( SPECIES_CRITICAL_LEVEL **criticalLevelArray, int speciesSize );
static RET_VAL _AddTransitions( IR2CTMC_TRANSFORMER *transformer, CTMC_GENERATOR *gen, int state );
static RET_VAL _AddProductionTransition( IR2CTMC_TRANSFORMER *transformer, CTMC_GENERATOR *gen, int state, int index );
static RET_VAL _AddDegradationTransition( IR2CTMC_TRANSFORMER *transformer, CTMC_GENERATOR *gen, int state, int index );
static RET_VAL _Update( IR2CTMC_TRANSFORMER *transformer, int state );
static RET_VAL _UpdateLevels( IR2CTMC_TRANSFORMER *transformer, int state );
static RET_VAL _UpdateReactionRates( IR2CTMC_TRANSFORMER *transformer, int state );
static RET_VAL _GenerateCurrentRate( KINETIC_LAW_EVALUATER *evaluator, REACTION *reaction, int state );
static RET_VAL _SetLevelInSpecies( SPECIES *species, double level );
static int _FindInitialState( IR2CTMC_TRANSFORMER *transformer );
static double _GetLevelInSpecies( SPECIES *species );

IR2CTMC_TRANSFORMER *CreateIR2CTMCTransformer( IR *ir, REB2SAC_PROPERTIES *properties ) {
    IR2CTMC_TRANSFORMER *transformer = NULL;
    
    START_FUNCTION("CreateIR2CTMCTransformer");
    
    if( !IsTransformableToCTMC( ir ) ) {
        END_FUNCTION("CreateIR2CTMCTransformer", FAILING );
        return NULL;
    }    
    
    if( ( transformer = (IR2CTMC_TRANSFORMER*)MALLOC( sizeof(IR2CTMC_TRANSFORMER) ) ) == NULL ) {
        END_FUNCTION("CreateIR2CTMCTransformer", FAILING );
        return NULL;
    }
    
    if( IS_FAILED( ( _InitTransformer( transformer, ir, properties ) ) ) ) {
        END_FUNCTION("CreateIR2CTMCTransformer", FAILING );
        return NULL;
    }     
        
    END_FUNCTION("CreateIR2CTMCTransformer", SUCCESS );
    
    return transformer; 
}

RET_VAL FreeIR2CTMCTransformer( IR2CTMC_TRANSFORMER **pTransformer ) {
    IR2CTMC_TRANSFORMER *transformer = *pTransformer;
    
    transformer->ReleaseResource( transformer );
    FREE( transformer );
    
    return SUCCESS;
}


static RET_VAL _InitTransformer( IR2CTMC_TRANSFORMER *transformer, IR *ir, REB2SAC_PROPERTIES *properties ) {
    RET_VAL ret = SUCCESS;
    int i = 0;
    int multiply = 1;
    int reactionSize = 0;
    int speciesSize = 0;
    int *currentArray = NULL;
    int *multiplyArray = NULL;
    double *currentLevelArray = NULL;
    SPECIES *species = NULL;
    SPECIES **updatedSpeciesArray = NULL;
    REACTION *reaction = NULL;
    REACTION **reactions = NULL;
    LINKED_LIST *list = NULL;    
    SPECIES_CRITICAL_LEVEL **criticalLevelArray = NULL;
    SPECIES_CRITICAL_LEVEL *criticalLevel = NULL;
    KINETIC_LAW_EVALUATER *evaluator = NULL;
    
    list = ir->GetListOfReactionNodes( ir );
    reactionSize = GetLinkedListSize( list );
    if( ( reactions = (REACTION**)MALLOC( reactionSize * sizeof(REACTION*) ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitArrays", "failed to create reaction array" );
    }
    ResetCurrentElement( list );
    for( i = 0; i < reactionSize; i++ ) {
        reaction = (REACTION*)GetNextFromLinkedList(list);
        reactions[i] = reaction;
    }
    
    list = ir->GetListOfSpeciesNodes( ir );
    if( ( criticalLevelArray = _CreateCriticalLevelArray( list, properties ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitArrays", "failed to create critical level array" );
    }
    
    speciesSize = GetLinkedListSize( list );
    
    if( ( currentArray = (int*)CALLOC( speciesSize, sizeof(int) ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitArrays", "failed to allocate memory for currentArray" );
    } 
    if( ( currentLevelArray = (double*)CALLOC( speciesSize, sizeof(double) ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitArrays", "failed to allocate memory for currentLevelArray" );
    } 
    if( ( multiplyArray = (int*)MALLOC( speciesSize * sizeof(int) ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitArrays", "failed to allocate memory for multiplyArray" );
    } 
    if( ( updatedSpeciesArray = (SPECIES**)MALLOC( speciesSize * sizeof(SPECIES*) ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitArrays", "failed to allocate memory for updatedSpeciesArray" );
    } 
    
    for( i = 0; i < speciesSize; i++ ) {
        criticalLevel = criticalLevelArray[i];
        species = criticalLevel->species;
        if( IS_FAILED(  ( ret = _SetLevelInSpecies( species, criticalLevel->levels[0] ) ) ) ) {
            return ret;
        }
        multiplyArray[i] = multiply;
        multiply *= criticalLevel->size;
    }
    
    if( ( evaluator = CreateKineticLawEvaluater() ) == NULL ) {
        return ErrorReport( FAILING, "_InitArrays", "failed to create a kinetic law evaluator" );
    }
    
    for( i = 0; i < reactionSize; i++ ) {
        reaction = reactions[i];
        if( IS_FAILED( ( ret = SetReactionRateUpdatedTime( reaction, -1.0 ) ) ) ) {
            return ret;
        }
        if( IS_FAILED( ( ret = _GenerateCurrentRate( evaluator, reaction, 0 ) ) ) ) {
            return ErrorReport( FAILING, "_InitArrays", "failed to create a kinetic law evaluator" );        
        }
    }
    
    transformer->stateSize = _FindNumberOfStates( criticalLevelArray, speciesSize );
    transformer->reactionSize = reactionSize;
    transformer->reactionArray = reactions;
    transformer->speciesSize = speciesSize;
    transformer->criticalLevelArray = criticalLevelArray;
    transformer->currentArray = currentArray;
    transformer->currentLevelArray = currentLevelArray;
    transformer->multiplyArray = multiplyArray; 
    transformer->evaluator = evaluator;
    transformer->updatedSpeciesArray = updatedSpeciesArray;

    transformer->Generate = _Generate;
    transformer->ReleaseResource = _ReleaseResource;
    transformer->GetCriticalLevelArraySize = _GetCriticalLevelArraySize;
    transformer->GetCriticalLevelArray = _GetCriticalLevelArray;
    transformer->GetCurrentLevelArray = _GetCurrentLevelArray;
    transformer->ResetCurrentLevelArray = _ResetCurrentLevelArray;
    transformer->IncrementCurrentLevelArray = _IncrementCurrentLevelArray;
    transformer->Print = _Print;
    
    return ret;
}


static CTMC *_Generate( IR2CTMC_TRANSFORMER *transformer ) {
    int i = 0;
    int stateSize = transformer->stateSize;
    int speciesSize = transformer->speciesSize;
    int *currentArray = transformer->currentArray; 
    int initialState = 0;
    SPECIES_CRITICAL_LEVEL **criticalLevelArray = transformer->criticalLevelArray;
    CTMC *ctmc = NULL;
    CTMC_STATE *states = NULL;
    CTMC_GENERATOR *gen = NULL;
    KINETIC_LAW_EVALUATER *evaluator = transformer->evaluator;
    
    if( ( gen = CreateCTMCGenerator() ) == NULL ) {
        return NULL;
    }
    
    if( ( states = gen->CreateStates( gen, stateSize ) ) == NULL ) {
        return NULL;
    }
    
    if( IS_FAILED( ( _AddTransitions( transformer, gen, 0 ) ) ) ) {
        return NULL;
    }    
    for( i = 1; i < stateSize; i++ ) {        
        if( IS_FAILED( ( _Update( transformer, i ) ) ) ) {
            return NULL;
        }
        
        if( IS_FAILED( ( _AddTransitions( transformer, gen, i ) ) ) ) {
            return NULL;
        }        
    }
    
    initialState = _FindInitialState( transformer );
    if( IS_FAILED( ( gen->SetInitialState( gen, initialState ) ) ) ) {
        return NULL;    
    }        
    
    if( ( ctmc = gen->Generate( gen ) ) == NULL ) {
        return NULL;
    }
    
    if( IS_FAILED( ( FreeCTMCGenerator( &gen ) ) ) ) {
        return NULL;
    }
    
    return ctmc;
}

static int _GetCriticalLevelArraySize( IR2CTMC_TRANSFORMER *transformer ) {
    return transformer->speciesSize;
}

static SPECIES_CRITICAL_LEVEL **_GetCriticalLevelArray( IR2CTMC_TRANSFORMER *transformer ) {
    return transformer->criticalLevelArray;
}

static RET_VAL _ResetCurrentLevelArray( IR2CTMC_TRANSFORMER *transformer ) {
    RET_VAL ret = SUCCESS;
    int speciesSize = transformer->speciesSize;
    int *currentArray = transformer->currentArray;
    double *currentLevelArray = transformer->currentLevelArray;
    SPECIES_CRITICAL_LEVEL **criticalLevelArray = transformer->criticalLevelArray;
    
    if( IS_FAILED( ( ret = _ResetCurrentArray( speciesSize, currentArray ) ) ) ) {
        return ret;
    }
    
    if( IS_FAILED( ( ret = _UpdateCurrenLevelArray( speciesSize, currentArray, currentLevelArray, criticalLevelArray ) ) ) ) {
        return ret;
    }
    
    return SUCCESS;
}

static RET_VAL _IncrementCurrentLevelArray( IR2CTMC_TRANSFORMER *transformer ) {
    RET_VAL ret = SUCCESS;
    int speciesSize = transformer->speciesSize;
    int *currentArray = transformer->currentArray;
    double *currentLevelArray = transformer->currentLevelArray;
    SPECIES_CRITICAL_LEVEL **criticalLevelArray = transformer->criticalLevelArray;
    
    if( IS_FAILED( ( ret = _IncrementCurrentArray( speciesSize, currentArray, criticalLevelArray ) ) ) ) {
        return ret;
    }
    
    if( IS_FAILED( ( ret = _UpdateCurrenLevelArray( speciesSize, currentArray, currentLevelArray, criticalLevelArray ) ) ) ) {
        return ret;
    }
    
    return SUCCESS;
}

static double *_GetCurrentLevelArray( IR2CTMC_TRANSFORMER *transformer ) {
    return transformer->currentLevelArray;
}


static RET_VAL _ResetCurrentArray( int speciesSize, int *currentArray ) {
    RET_VAL ret = SUCCESS;
    int i = 0;

    for( ; i < speciesSize; i++ ){
        currentArray[i] = 0;
    }
    
    return ret;
}

static RET_VAL _IncrementCurrentArray( int speciesSize, int *currentArray, SPECIES_CRITICAL_LEVEL **criticalLevelArray ){
    int i = 0;
    int current = 0;
    int delta = 0;
    SPECIES_CRITICAL_LEVEL *criticalLevel = NULL;
    
    for( i = 0; i < speciesSize; i++ ) {
        criticalLevel = criticalLevelArray[i];
        current = currentArray[i];
        delta = criticalLevel->size - current;
        if( ( delta == 1 ) && ( current == 0 ) ) {
            continue;
        }
        else if( delta == 1 ) {
            currentArray[i] = 0;
        }
        else {
            current++;
            currentArray[i] = current;
            return SUCCESS;
        }        
    }
    return SUCCESS;    
}

static RET_VAL _UpdateCurrenLevelArray( 
                int speciesSize, 
                int *currentArray, 
                double *currentLevelArray, 
                SPECIES_CRITICAL_LEVEL **criticalLevelArray ) 
{
    int i = 0;
    int current = 0;
    SPECIES_CRITICAL_LEVEL *criticalLevel = NULL;
    
    for( i = 0; i < speciesSize; i++ ) {
        criticalLevel = criticalLevelArray[i];
        current = currentArray[i];
        currentLevelArray[i] = criticalLevel->levels[current];
    }
    return SUCCESS;    
}



static RET_VAL _ReleaseResource( IR2CTMC_TRANSFORMER *transformer ) {
    int i = 0;
    int speciesSize = transformer->speciesSize;
    SPECIES_CRITICAL_LEVEL *criticalLevel = NULL;
    SPECIES_CRITICAL_LEVEL **criticalLevelArray = transformer->criticalLevelArray;
    
    for( ; i < speciesSize; i++ ) {
        criticalLevel = criticalLevelArray[i];
        FreeSpeciesCriticalLevel( &criticalLevel );
    }
    
    FREE( criticalLevelArray );
    FREE( transformer->reactionArray );
    FREE( transformer->multiplyArray );
    FREE( transformer->currentArray );
    FREE( transformer->currentLevelArray );
    FREE( transformer->updatedSpeciesArray );

    return SUCCESS;
}



static SPECIES_CRITICAL_LEVEL **_CreateCriticalLevelArray( LINKED_LIST *speciesList, REB2SAC_PROPERTIES *properties ) {
    int i = 0;
    int speciesSize = 0;
    SPECIES *species = NULL;
    SPECIES_CRITICAL_LEVEL **criticalLevels = NULL;
    SPECIES_CRITICAL_LEVEL *criticalLevel = NULL;
    SPECIES_CRITICAL_LEVEL_GENERATOR *gen = NULL;    
    
    speciesSize = GetLinkedListSize( speciesList );
    criticalLevels = (SPECIES_CRITICAL_LEVEL**)MALLOC( speciesSize *sizeof(SPECIES_CRITICAL_LEVEL*) );
    if( criticalLevels == NULL ) {
        return NULL;
    }
    
    if( ( gen = CreateSpeciesCriticalLevelGenerator( properties ) ) == NULL ) {
        return NULL;
    }
    
    ResetCurrentElement( speciesList );
    for( i = 0; i < speciesSize; i++ ) {
        species = (SPECIES*)GetNextFromLinkedList( speciesList );
        criticalLevel = gen->Generate( gen, species );
        criticalLevels[i] = criticalLevel;
    }
    
    FreeSpeciesCriticalLevelGenerator( &gen );
    
    return criticalLevels;        
}



static int _FindNumberOfStates( SPECIES_CRITICAL_LEVEL **criticalLevelArray, int speciesSize ) {
    int i = 0;
    int num = 1;
    SPECIES_CRITICAL_LEVEL *criticalLevel = 0;
    
    for( i = 0; i < speciesSize; i++ ) {
        criticalLevel = criticalLevelArray[i];
        num *= criticalLevel->size;        
    }
    return num;
}

static RET_VAL _AddTransitions( IR2CTMC_TRANSFORMER *transformer, CTMC_GENERATOR *gen, int state ) {
    RET_VAL ret = SUCCESS;
    int i = 0;
    int speciesSize = transformer->speciesSize;
    
    for( ; i < speciesSize; i++ ){
        if( IS_FAILED( ( ret = _AddProductionTransition( transformer, gen, state, i ) ) ) ) {
            return ret;
        } 
        if( IS_FAILED( ( ret = _AddDegradationTransition( transformer, gen, state, i ) ) ) ) {
            return ret;
        } 
    }
    
    return SUCCESS;
}

static RET_VAL _AddProductionTransition( IR2CTMC_TRANSFORMER *transformer, CTMC_GENERATOR *gen, int state, int index ) {
    RET_VAL ret = SUCCESS;
    int *multiplyArray = transformer->multiplyArray;
    int levelIndex = 0;
    int targetState = state;    
    int current = 0;
    int *currentArray = transformer->currentArray;
    double stoichiometry = 0.0;
    double rate = 0.0;
    double deltaLevel = 0.0;
    double reactionRate = 0.0;
    SPECIES *species = NULL;
    IR_EDGE *edge = NULL;
    LINKED_LIST *list = NULL;
    REACTION *reaction = NULL;
    SPECIES_CRITICAL_LEVEL *criticalLevel = NULL;
    SPECIES_CRITICAL_LEVEL **criticalLevelArray = transformer->criticalLevelArray;
    
    criticalLevel = criticalLevelArray[index];
    current = currentArray[index];
    
    if( ( criticalLevel->size - current ) == 1 ) {
        return SUCCESS;        
    }
    
    species = criticalLevel->species;        
    list = GetProductEdges( (IR_NODE*)species );
    ResetCurrentElement( list );
    while( (edge = GetNextEdge( list )) != NULL ){
        stoichiometry = GetStoichiometryInIREdge( edge );
        reaction = GetReactionInIREdge( edge );
        reactionRate = GetReactionRate( reaction );
        rate += (stoichiometry * reactionRate);
    }
    
    if( IS_REAL_EQUAL( rate, 0.0 ) ) {
        return SUCCESS;        
    }
    
    deltaLevel = criticalLevel->levels[current + 1] - criticalLevel->levels[current];
    targetState += multiplyArray[index];
    
    TRACE_3( "Adding transition from %i to %i at rate %g", state, targetState, rate / deltaLevel );
    if( IS_FAILED( ( ret = gen->AddTransition( gen, state, targetState, rate / deltaLevel ) ) ) ) {
        return ret;
    }
    
    return SUCCESS;
}

static RET_VAL _AddDegradationTransition( IR2CTMC_TRANSFORMER *transformer, CTMC_GENERATOR *gen, int state, int index ) {
    RET_VAL ret = SUCCESS;
    int *multiplyArray = transformer->multiplyArray;
    int levelIndex = 0;
    int targetState = state;    
    int current = 0;
    int *currentArray = transformer->currentArray;
    double stoichiometry = 0.0;
    double rate = 0.0;
    double deltaLevel = 0.0;
    double reactionRate = 0.0;
    SPECIES *species = NULL;
    IR_EDGE *edge = NULL;
    LINKED_LIST *list = NULL;
    REACTION *reaction = NULL;
    SPECIES_CRITICAL_LEVEL *criticalLevel = NULL;
    SPECIES_CRITICAL_LEVEL **criticalLevelArray = transformer->criticalLevelArray;
    
    criticalLevel = criticalLevelArray[index];
    current = currentArray[index];
    
    if( current == 0 ) {
        return SUCCESS;        
    }
    
    species = criticalLevel->species;        
    list = GetReactantEdges( (IR_NODE*)species );
    ResetCurrentElement( list );
    while( (edge = GetNextEdge( list )) != NULL ){
        stoichiometry = GetStoichiometryInIREdge( edge );
        reaction = GetReactionInIREdge( edge );
        reactionRate = GetReactionRate( reaction );
        rate += (stoichiometry * reactionRate);
    }
    
    if( IS_REAL_EQUAL( rate, 0.0 ) ) {
        return SUCCESS;        
    }
    
    deltaLevel = criticalLevel->levels[current] - criticalLevel->levels[current - 1];
    targetState -= multiplyArray[index];
    
    TRACE_3( "Adding transition from %i to %i at rate %g", state, targetState, rate / deltaLevel );
    if( IS_FAILED( ( ret = gen->AddTransition( gen, state, targetState, rate / deltaLevel ) ) ) ) {
        return ret;
    }
    
    return SUCCESS;
}



static RET_VAL _Update( IR2CTMC_TRANSFORMER *transformer, int state ) {
    RET_VAL ret = SUCCESS;
    
    if( IS_FAILED( ( ret = _UpdateLevels( transformer, state ) ) ) ) {
        return ret;
    }
    if( IS_FAILED( ( ret = _UpdateReactionRates( transformer, state ) ) ) ) {
        return ret;
    }
    
    return SUCCESS;
}

static RET_VAL _UpdateLevels( IR2CTMC_TRANSFORMER *transformer, int state ) {
    RET_VAL ret = SUCCESS;
    int i = 0;
    int current = 0;
    int delta = 0;
    int updatedSpeciesIndex = 0;    
    int speciesSize = transformer->speciesSize;
    int *currentArray = transformer->currentArray;
    SPECIES *species = NULL;
    SPECIES **updatedSpeciesArray = transformer->updatedSpeciesArray;
    SPECIES_CRITICAL_LEVEL *criticalLevel = NULL;
    SPECIES_CRITICAL_LEVEL **criticalLevelArray = transformer->criticalLevelArray;
    
    for( i = 0; i < speciesSize; i++ ) {
        criticalLevel = criticalLevelArray[i];
        current = currentArray[i];
        delta = criticalLevel->size - current;
        species = criticalLevel->species;
        if( ( delta == 1 ) && ( current == 0 ) ) {
            continue;
        }
        else if( delta == 1 ) {
            currentArray[i] = 0;
            if( IS_FAILED(  ( ret = _SetLevelInSpecies( species, criticalLevel->levels[0] ) ) ) ) {
                return ret;
            }
            updatedSpeciesArray[updatedSpeciesIndex] = species;
            updatedSpeciesIndex++;
        }
        else {
            current++;
            if( IS_FAILED(  ( ret = _SetLevelInSpecies( species, criticalLevel->levels[current] ) ) ) ) {
                return ret;
            }
            updatedSpeciesArray[updatedSpeciesIndex] = species;
            updatedSpeciesIndex++;
            transformer->updatedSpeciesSize = updatedSpeciesIndex;
            currentArray[i] = current;
            return SUCCESS;
        }
    }
    return FAILING;
}

static RET_VAL _UpdateReactionRates( IR2CTMC_TRANSFORMER *transformer, int state ) {
    RET_VAL ret = SUCCESS;
    int i = 0;
    int updatedSpeciesSize = transformer->updatedSpeciesSize;
    SPECIES *species = NULL;
    SPECIES **updatedSpeciesArray = transformer->updatedSpeciesArray;
    REACTION *reaction = NULL;
    IR_EDGE *edge = NULL;
    LINKED_LIST *list = NULL;
    KINETIC_LAW_EVALUATER *evaluator = transformer->evaluator;
    
    for( i = 0; i < updatedSpeciesSize; i++ ) {
        species = updatedSpeciesArray[i];
        
        list = GetReactantEdges( (IR_NODE*)species );
        ResetCurrentElement( list );
        while( (edge = GetNextEdge( list )) != NULL ){
            reaction = GetReactionInIREdge( edge );
            if( IS_FAILED( ( ret = _GenerateCurrentRate( evaluator, reaction, state ) ) ) ) {
                return ret;
            }
        }
        
        list = GetModifierEdges( (IR_NODE*)species );
        ResetCurrentElement( list );
        while( (edge = GetNextEdge( list )) != NULL ){
            reaction = GetReactionInIREdge( edge );
            if( IS_FAILED( ( ret = _GenerateCurrentRate( evaluator, reaction, state ) ) ) ) {
                return ret;
            }
        }
        
        list = GetProductEdges( (IR_NODE*)species );
        ResetCurrentElement( list );
        while( (edge = GetNextEdge( list )) != NULL ){
            reaction = GetReactionInIREdge( edge );
            if( IS_FAILED( ( ret = _GenerateCurrentRate( evaluator, reaction, state ) ) ) ) {
                return ret;
            }
        }
    }
    
    return SUCCESS;
}



static RET_VAL _GenerateCurrentRate( KINETIC_LAW_EVALUATER *evaluator, REACTION *reaction, int state ) {
    RET_VAL ret = SUCCESS;
    int oldStep = 0;
    double rate = 0.0;
    double level = 0.0;
    double stoichiometry = 0.0;
    SPECIES *species = NULL;
    IR_EDGE *edge = NULL;
    LINKED_LIST *list = NULL;
    KINETIC_LAW *kineticLaw = NULL;
    
    oldStep = (int)GetReactionRateUpdatedTime( reaction );     
    if( oldStep == state ) {
        return SUCCESS;
    }
    
    list = GetReactantEdges( (IR_NODE*)reaction );
    if( GetLinkedListSize( list ) > 0 ) {
        /** at most one reactant */
        edge = GetHeadEdge( list );
        stoichiometry = GetStoichiometryInIREdge( edge );
        species = GetSpeciesInIREdge( edge );
        level = _GetLevelInSpecies( species );
        if( level < stoichiometry ) {
            if( IS_FAILED( ( ret = SetReactionRate( reaction, 0.0 ) ) ) ) {
                return ret;
            }
            if( IS_FAILED( ( ret = SetReactionRateUpdatedTime( reaction, (double)state ) ) ) ) {
                return ret;
            }     
            return SUCCESS;            
        }
    }
        
    kineticLaw = GetKineticLawInReactionNode( reaction );  
    rate = evaluator->EvaluateWithCurrentConcentrations( evaluator, kineticLaw );        
    if( IS_FAILED( ( ret = SetReactionRate( reaction, rate ) ) ) ) {
        return ret;
    }
    if( IS_FAILED( ( ret = SetReactionRateUpdatedTime( reaction, (double)state ) ) ) ) {
        return ret;
    }
     
    return SUCCESS;
}

static RET_VAL _SetLevelInSpecies( SPECIES *species, double level ) {
    return SetAmountInSpeciesNode( species, level ); 
    /*    return SetConcentrationInSpeciesNode( species, level ); */
}

static double _GetLevelInSpecies( SPECIES *species ) {
    return GetAmountInSpeciesNode( species );
    
    /*return GetConcentrationInSpeciesNode( species );*/
}

static int _FindInitialState( IR2CTMC_TRANSFORMER *transformer ) {
    RET_VAL ret = SUCCESS;
    int i = 0;
    int initialLevelIndex = 0;    
    int multiply = 0;
    int speciesSize = transformer->speciesSize;
    int *multiplyArray = transformer->multiplyArray;
    int initialState = 0;
    SPECIES_CRITICAL_LEVEL *criticalLevel = NULL;
    SPECIES_CRITICAL_LEVEL **criticalLevelArray = transformer->criticalLevelArray;
    
    for( i = 0; i < speciesSize; i++ ) {
        criticalLevel = criticalLevelArray[i];
        initialLevelIndex = criticalLevel->initialLevelIndex;
        multiply = multiplyArray[i];
        initialState += (initialLevelIndex * multiply);
    }
    return initialState;
}




static RET_VAL _Print( IR2CTMC_TRANSFORMER *transformer, FILE *file ) {
    RET_VAL ret = SUCCESS;
    int i = 0;
    int stateSize = transformer->stateSize;
    int speciesSize = transformer->speciesSize;
    double *currentLevelArray = transformer->currentLevelArray;
    SPECIES *species = NULL;
    SPECIES_CRITICAL_LEVEL *criticalLevel = NULL;
    SPECIES_CRITICAL_LEVEL **criticalLevelArray = transformer->criticalLevelArray;
    
    
    fprintf( file, "# state-id" );
    for( i = 0; i < speciesSize; i++ ) {
        criticalLevel = criticalLevelArray[i];
        species = criticalLevel->species;
        fprintf( file, " %s", GetCharArrayOfString( GetSpeciesNodeName( species ) ) );
    }
    fprintf( file, NEW_LINE );
    
    if( IS_FAILED( ( ret = _ResetCurrentLevelArray( transformer ) ) ) ) {
        return ret;
    }
    for( i = 0; i < stateSize; i++ ){        
        if( IS_FAILED( ( ret = _PrintState( file, i, speciesSize, currentLevelArray ) ) ) ) {
            return ret;
        }
        if( IS_FAILED( ( ret = _IncrementCurrentLevelArray( transformer ) ) ) ) {
            return ret;
        }
    }
    
    return SUCCESS;
}

static RET_VAL _PrintState( FILE *file, int state, int size, double *currentLevelArray ) {
    RET_VAL ret = SUCCESS;
    int i = 0;
    
    fprintf( file, "%i", state );
    for( ; i < size; i++ ) {
        fprintf( file, " %g", currentLevelArray[i] );        
    }
    fprintf( file, NEW_LINE );
    
    return ret;
}


