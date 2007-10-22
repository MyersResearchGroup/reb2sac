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
#include <float.h>
#include <math.h>
#include "gsl/gsl_errno.h"
#include "gsl/gsl_matrix.h"
#include "gsl/gsl_odeiv.h"
#include "implicit_gear1_method.h"


static BOOL _IsModelConditionSatisfied( IR *ir );


static RET_VAL _InitializeRecord( IMPLICIT_GEAR1_SIMULATION_RECORD *rec, BACK_END_PROCESSOR *backend, IR *ir );
static RET_VAL _InitializeSimulation( IMPLICIT_GEAR1_SIMULATION_RECORD *rec, int runNum );
static RET_VAL _RunSimulation( IMPLICIT_GEAR1_SIMULATION_RECORD *rec );

static RET_VAL _CleanSimulation( IMPLICIT_GEAR1_SIMULATION_RECORD *rec );
static RET_VAL _CleanRecord( IMPLICIT_GEAR1_SIMULATION_RECORD *rec );

static RET_VAL _CalculateReactionRates( IMPLICIT_GEAR1_SIMULATION_RECORD *rec );
static RET_VAL _CalculateReactionRate( IMPLICIT_GEAR1_SIMULATION_RECORD *rec, REACTION *reaction );
static int _Update( double t, const double y[], double f[], IMPLICIT_GEAR1_SIMULATION_RECORD *rec );
static RET_VAL _Print( IMPLICIT_GEAR1_SIMULATION_RECORD *rec, double time );




DLLSCOPE RET_VAL STDCALL DoImplicitGear1Simulation( BACK_END_PROCESSOR *backend, IR *ir ) {
    RET_VAL ret = SUCCESS;
    UINT i = 0;   
    UINT runs = 1;
    char *namePrefix = NULL;
    static IMPLICIT_GEAR1_SIMULATION_RECORD rec;
    
    START_FUNCTION("DoImplicitGear1Simulation");
    
    if( !_IsModelConditionSatisfied( ir ) ) {
        return ErrorReport( FAILING, "DoImplicitGear1Simulation", 
            "gear 1 method cannot be applied to the model" );
    }
    
    if( IS_FAILED( ( ret = _InitializeRecord( &rec, backend, ir ) ) ) )  {
        return ErrorReport( ret, "DoImplicitGear1Simulation", "initialization of the record failed" );
    }
    
    if( IS_FAILED( ( ret = _InitializeSimulation( &rec, i ) ) ) ) {
        return ErrorReport( ret, "DoImplicitGear1Simulation", "initialization of the %i-th simulation failed", i );
    }
    if( IS_FAILED( ( ret = _RunSimulation( &rec ) ) ) ) {
        return ErrorReport( ret, "DoImplicitGear1Simulation", "%i-th simulation failed at time %f", i, rec.time );
    }
    if( IS_FAILED( ( ret = _CleanSimulation( &rec ) ) ) ) {
        return ErrorReport( ret, "DoImplicitGear1Simulation", "cleaning of the %i-th simulation failed", i );
    }         
    
    END_FUNCTION("DoImplicitGear1Simulation", SUCCESS );
    return ret;            
}

DLLSCOPE RET_VAL STDCALL CloseImplicitGear1Simulation( BACK_END_PROCESSOR *backend ) {
    RET_VAL ret = SUCCESS;
    IMPLICIT_GEAR1_SIMULATION_RECORD *rec = (IMPLICIT_GEAR1_SIMULATION_RECORD*)(backend->_internal1);
        
    START_FUNCTION("CloseImplicitGear1Simulation");
    
    if( IS_FAILED( ( ret = _CleanRecord( rec ) ) ) )  {
        return ErrorReport( ret, "CloseImplicitGear1Simulation", "cleaning of the record failed" );
    }
        
    END_FUNCTION("CloseImplicitGear1Simulation",  SUCCESS );
    return ret;            
}


static BOOL _IsModelConditionSatisfied( IR *ir ) {
    return TRUE;
}


static RET_VAL _InitializeRecord( IMPLICIT_GEAR1_SIMULATION_RECORD *rec, BACK_END_PROCESSOR *backend, IR *ir ) {
    RET_VAL ret = SUCCESS;
    UINT32 i = 0;
    char buf[512];
    char *valueString = NULL;
    SPECIES *species = NULL;
    SPECIES **speciesArray = NULL;
    REACTION *reaction = NULL;
    REACTION **reactions = NULL;
    COMPILER_RECORD_T *compRec = backend->record;
    LINKED_LIST *list = NULL;
    REB2SAC_PROPERTIES *properties = NULL;

#if GET_SEED_FROM_COMMAND_LINE
    PROPERTIES *options = NULL;
#endif    
            
    list = ir->GetListOfReactionNodes( ir );
    rec->reactionsSize = GetLinkedListSize( list );
    if( ( reactions = (REACTION**)MALLOC( rec->reactionsSize * sizeof(REACTION*) ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not allocate memory for reaction array" );
    }
    i = 0;
    ResetCurrentElement( list );
    while( ( reaction = (REACTION*)GetNextFromLinkedList( list ) ) != NULL ) {
        reactions[i] = reaction;
        i++;        
    }
    rec->reactionArray = reactions;    
    
    list = ir->GetListOfSpeciesNodes( ir );
    rec->speciesSize = GetLinkedListSize( list );
    if( ( speciesArray = (SPECIES**)MALLOC( rec->speciesSize * sizeof(SPECIES*) ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not allocate memory for species array" );
    }
    i = 0;
    ResetCurrentElement( list );
    while( ( species = (SPECIES*)GetNextFromLinkedList( list ) ) != NULL ) {
        speciesArray[i] = species;
        i++;        
    }
    rec->speciesArray = speciesArray;    

    if( ( rec->concentrations = (double*)MALLOC( rec->speciesSize * sizeof( double ) ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not allocate memory for concentrations" );
    }
    
    if( ( rec->evaluator = CreateKineticLawEvaluater() ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not create evaluator" );
    }                
    
    properties = compRec->properties;
    if( ( valueString = properties->GetProperty( properties, ODE_SIMULATION_ABSOLUTE_ERROR ) ) == NULL ) {
      rec->absoluteError = DEFAULT_ODE_SIMULATION_ABSOLUTE_ERROR;
    }
    else {
      if( IS_FAILED( ( ret = StrToFloat( &(rec->absoluteError), valueString ) ) ) ) {
	rec->absoluteError = DEFAULT_ODE_SIMULATION_ABSOLUTE_ERROR;
      }
    }    

    if( ( valueString = properties->GetProperty( properties, ODE_SIMULATION_TIME_LIMIT ) ) == NULL ) {
        rec->timeLimit = DEFAULT_ODE_SIMULATION_TIME_LIMIT_VALUE;
    }
    else {
        if( IS_FAILED( ( ret = StrToFloat( &(rec->timeLimit), valueString ) ) ) ) {
            rec->timeLimit = DEFAULT_ODE_SIMULATION_TIME_LIMIT_VALUE;
        }
    }    
    
    if( ( valueString = properties->GetProperty( properties, ODE_SIMULATION_PRINT_INTERVAL ) ) == NULL ) {
        rec->printInterval = DEFAULT_ODE_SIMULATION_PRINT_INTERVAL_VALUE;
    }
    else {
        if( IS_FAILED( ( ret = StrToFloat( &(rec->printInterval), valueString ) ) ) ) {
            rec->printInterval = DEFAULT_ODE_SIMULATION_PRINT_INTERVAL_VALUE;
        }
    }    

    if( ( rec->outDir = properties->GetProperty( properties, ODE_SIMULATION_OUT_DIR ) ) == NULL ) {
        rec->outDir = DEFAULT_ODE_SIMULATION_OUT_DIR_VALUE;
    }
    
    if( ( rec->printer = CreateSimulationPrinter( backend, speciesArray, rec->speciesSize ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not create simulation printer" );
    }                
    
    if( ( rec->decider = 
        CreateSimulationRunTerminationDecider( backend, speciesArray, rec->speciesSize, reactions, rec->reactionsSize, rec->timeLimit ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not create simulation printer" );
    }
        
    backend->_internal1 = (CADDR_T)rec;
    
    return ret;            
}

static RET_VAL _InitializeSimulation( IMPLICIT_GEAR1_SIMULATION_RECORD *rec, int runNum ) {
    RET_VAL ret = SUCCESS;
    char filenameStem[512];
    double concentration = 0.0;
    double *concentrations = rec->concentrations;
    UINT32 i = 0;
    UINT32 size = 0;
    SPECIES *species = NULL;
    SPECIES **speciesArray = rec->speciesArray;
    REACTION *reaction = NULL;
    REACTION **reactionArray = rec->reactionArray;
    KINETIC_LAW_EVALUATER *evaluator = rec->evaluator;
    SIMULATION_PRINTER *printer = rec->printer;
    
    sprintf( filenameStem, "%s%cgear1-run", rec->outDir, FILE_SEPARATOR );        
    if( IS_FAILED( (  ret = printer->PrintStart( printer, filenameStem ) ) ) ) {
        return ret;            
    }
    if( IS_FAILED( (  ret = printer->PrintHeader( printer ) ) ) ) {
        return ret;            
    }
    rec->time = 0.0;
    rec->nextPrintTime = 0.0;
    size = rec->speciesSize;        
    for( i = 0; i < size; i++ ) {
        species = speciesArray[i];
        if( IsInitialQuantityInAmountInSpeciesNode( species ) ) {
            concentration = GetInitialAmountInSpeciesNode( species );
        }
        else {
            concentration = GetInitialConcentrationInSpeciesNode( species ); 
        }
        if( IS_FAILED( ( ret = SetConcentrationInSpeciesNode( species, concentration ) ) ) ) {
            return ret;            
        }
        concentrations[i] = concentration;
    }
    
    return ret;            
}

static RET_VAL _RunSimulation( IMPLICIT_GEAR1_SIMULATION_RECORD *rec ) {
    RET_VAL ret = SUCCESS;
    int status = GSL_SUCCESS;
    double h = IMPLICIT_GEAR1_H;
    double *y = rec->concentrations;
    double printInterval = rec->printInterval;
    double nextPrintTime = rec->time;
    double time = rec->time;
    SIMULATION_PRINTER *printer = NULL;
    SIMULATION_RUN_TERMINATION_DECIDER *decider = NULL;
    gsl_odeiv_step_type *stepType = gsl_odeiv_step_gear1;
    gsl_odeiv_step *step = NULL;
    gsl_odeiv_control *control = NULL;
    gsl_odeiv_evolve *evolve = NULL;
    int size = rec->speciesSize;
    gsl_odeiv_system system = 
    { 
        (int(*)(double , const double [], double [], void*))_Update, 
        NULL, 
        size,
        rec
    };        

    step = gsl_odeiv_step_alloc( stepType, size );
    control = gsl_odeiv_control_y_new( rec->absoluteError, IMPLICIT_GEAR1_LOCAL_ERROR );
    evolve = gsl_odeiv_evolve_alloc( size );
    
    printer = rec->printer;
    decider = rec->decider;
    while( !(decider->IsTerminationConditionMet( decider, NULL, time )) ) {
      if (time > 0.0) printf("Time = %.2f\n",time);
      fflush(stdout);
        if( IS_FAILED( ( ret = _Print( rec, time ) ) ) ) {
            return ret;            
        }
        nextPrintTime += printInterval;
        while( time < nextPrintTime ) {
            status = gsl_odeiv_evolve_apply( evolve, control, step, 
                                            &system, &time, nextPrintTime,
                                            &h, y ); 
        }
        if( status != GSL_SUCCESS ) {
            return FAILING;
        }
    }
    if( IS_FAILED( ( ret = _Print( rec, time ) ) ) ) {
        return ret;
    }
    
    if( IS_FAILED( ( ret = printer->PrintEnd( printer ) ) ) ) {
        return ret;
    }
    
    if( IS_FAILED( ( ret = _CleanSimulation( rec ) ) ) ) {
        return ret;
    }
        
    return ret;            
}

static RET_VAL _CleanSimulation( IMPLICIT_GEAR1_SIMULATION_RECORD *rec ) {
    RET_VAL ret = SUCCESS;
    return ret;            
}

static RET_VAL _CleanRecord( IMPLICIT_GEAR1_SIMULATION_RECORD *rec ) {
    RET_VAL ret = SUCCESS;
    SIMULATION_PRINTER *printer = rec->printer;
    SIMULATION_RUN_TERMINATION_DECIDER *decider = rec->decider;
    
    if( rec->evaluator != NULL ) {
        FreeKineticLawEvaluater( &(rec->evaluator) );
    }
    if( rec->reactionArray != NULL ) {
        FREE( rec->reactionArray );    
    }
    if( rec->speciesArray != NULL ) {
        FREE( rec->speciesArray );    
    }
    if( rec->concentrations != NULL ) {
        FREE( rec->concentrations );    
    }
    
    printer->Destroy( printer );
    decider->Destroy( decider );
    
    return ret;            
}

static RET_VAL _Print( IMPLICIT_GEAR1_SIMULATION_RECORD *rec, double time ) {
    RET_VAL ret = SUCCESS;
    SIMULATION_PRINTER *printer = rec->printer;

    if( IS_FAILED( ( ret = printer->PrintValues( printer, time ) ) ) ) {
        return ret;
    }
    return ret;            
}



static RET_VAL _CalculateReactionRates( IMPLICIT_GEAR1_SIMULATION_RECORD *rec ) {
    RET_VAL ret = SUCCESS;
    int i = 0;
    int size = 0;
    REACTION **reactionArray = rec->reactionArray;    
    
    size = rec->reactionsSize;    
    for( i = 0; i < size; i++ ) {
        if( IS_FAILED( ( ret = _CalculateReactionRate( rec, reactionArray[i] ) ) ) ) {
            return ret;        
        }
    }
    return ret;          
}


static RET_VAL _CalculateReactionRate( IMPLICIT_GEAR1_SIMULATION_RECORD *rec, REACTION *reaction ) {
    RET_VAL ret = SUCCESS;
    double rate = 0.0;
    SPECIES *species = NULL;
    KINETIC_LAW *law = NULL;
    KINETIC_LAW_EVALUATER *evaluator = rec->evaluator;
    
    law = GetKineticLawInReactionNode( reaction );
    rate = evaluator->EvaluateWithCurrentConcentrations( evaluator, law );
    if( !( rate < DBL_MAX ) ) {
        rate = 0.0;
    }
    if( IS_FAILED( ( ret = SetReactionRate( reaction, rate ) ) ) ) {
        return ret;         
    }
    return ret;         
}


static int _Update( double t, const double y[], double f[], IMPLICIT_GEAR1_SIMULATION_RECORD *rec ) {
    RET_VAL ret = SUCCESS;
    UINT32 i = 0;
    UINT32 speciesSize = rec->speciesSize;
    long stoichiometry = 0;
    double concentration = 0.0;    
    double change = 0.0;
    double deltaTime = 0.0;    
    double rate = 0.0;
    IR_EDGE *edge = NULL;
    REACTION *reaction = NULL;
    SPECIES *species = NULL;
    SPECIES **speciesArray = rec->speciesArray;
    LINKED_LIST *edges = NULL;
    KINETIC_LAW_EVALUATER *evaluator = rec->evaluator;

    for( i = 0; i < speciesSize; i++ ) {
        species = speciesArray[i];
        if( IS_FAILED( ( ret = SetConcentrationInSpeciesNode( species, y[i] ) ) ) ) {
            return GSL_FAILURE;            
        }        
    }
    
    if( IS_FAILED( ( ret = _CalculateReactionRates( rec ) ) ) ) {
        return GSL_FAILURE;            
    }
    
    for( i = 0; i < speciesSize; i++ ) {    
        species = speciesArray[i];
        change = 0.0;                
        concentration = GetConcentrationInSpeciesNode( species );
        TRACE_2( "%s changes from %g", GetCharArrayOfString( GetSpeciesNodeName( species ) ), 
            concentration );
        edges = GetReactantEdges( species );
        ResetCurrentElement( edges );
        while( ( edge = GetNextEdge( edges ) ) != NULL ) {
            stoichiometry = (long)GetStoichiometryInIREdge( edge );
            reaction = GetReactionInIREdge( edge );
            rate = GetReactionRate( reaction );
            change -= ((double)stoichiometry * rate);
            TRACE_2( "\tchanges from %s is %g", GetCharArrayOfString( GetReactionNodeName( reaction ) ), 
               -((double)stoichiometry * rate));
        }                
        edges = GetProductEdges( species );
        ResetCurrentElement( edges );
        while( ( edge = GetNextEdge( edges ) ) != NULL ) {
            stoichiometry = (long)GetStoichiometryInIREdge( edge );
            reaction = GetReactionInIREdge( edge );
            rate = GetReactionRate( reaction );
            change += ((double)stoichiometry * rate);
            TRACE_2( "\tchanges from %s is %g", GetCharArrayOfString( GetReactionNodeName( reaction ) ), 
               ((double)stoichiometry * rate));
        }
        f[i] = change;
        TRACE_2( "change of %s is %g", GetCharArrayOfString( GetSpeciesNodeName( species ) ), change );
    }
    return GSL_SUCCESS;            
}
