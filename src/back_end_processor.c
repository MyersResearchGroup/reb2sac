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

#include "abstraction_method_properties.h"
#include "back_end_processor.h"
#include "sbml_back_end_processor.h"
#include "dot_back_end_processor.h"
#include "hse_back_end_processor.h"
#include "hse2_back_end_processor.h"
#include "xhtml_back_end_processor.h"
#include "monte_carlo.h"
#include "euler_method.h"
#include "nary_level_back_end_process.h"
#include "embedded_runge_kutta_prince_dormand_method.h"
#include "embedded_runge_kutta_fehlberg_method.h"
#include "implicit_runge_kutta_4_method.h"
#include "implicit_gear1_method.h"
#include "implicit_gear2_method.h"
#include "marginal_probability_density_evolution_monte_carlo.h"
#include "type1pili_gillespie_ci.h"
#include "ctmc_analysis_back_end_processor.h"
#include "ssa_with_user_update.h"

static RET_VAL _AddPostProcessingMethods( COMPILER_RECORD_T *record, char *methodIDs[] );

static char *__MONTE_CARLO_POST_PROCESSING_METHODS[] = {
    "kinetic-law-constants-simplifier",
    "reversible-to-irreversible-transformer",
    NULL    
};

static char *__ODE_POST_PROCESSING_METHODS[] = {
    "kinetic-law-constants-simplifier",
    NULL    
};

static char *__SAC_POST_PROCESSING_METHODS[] = {
    /*    "pow-kinetic-law-transformer", */
    "nary-order-unary-transformer",
    "absolute-inhibition-generator",
    "final-state-generator",
    "stop-flag-generator",
    "kinetic-law-constants-simplifier",
    NULL    
};

static char *__NARY_LEVEL1_POST_PROCESSING_METHODS[] = {
    "pow-kinetic-law-transformer",
    "kinetic-law-constants-simplifier",
    NULL    
};

static char *__NARY_LEVEL2_POST_PROCESSING_METHODS[] = {
    "pow-kinetic-law-transformer",
    "kinetic-law-constants-simplifier",
    "nary-order-unary-transformer",
    NULL    
};

/*
typedef struct {
    char *id;
    char *type;
    char *outputNames;
    char *info;
} BACK_END_PROCESSOR_INFO;
*/

BACK_END_PROCESSOR_INFO *GetInfoOnBackEndProcessors() {
    static BACK_END_PROCESSOR_INFO info[] = {
        { "bunker", 1, "${out-dir}/run-${run-num}.${ext}", "perform Bunker's method, i.e., Gillepie's method except for the next reaction time is averaged" },
        { "dot", 0, "./${filename}.dot", "Generate a dot file of the network" },
        { "euler", 1, "${out-dir}/euler-run.${ext}", "ODE simulation with Euler method" },
        { "emc-sim", 1, "${out-dir}/run-${run-num}.${ext}", "Monte Carlo simulation with jump count as independent variable"},
        { "gillespie", 1, "${out-dir}/run-${run-num}.${ext}", "perform Gillespie's direct method" },
        { "mpde", 1, "${out-dir}/run-${run-num}.${ext}", "perform marginal probability density evolution" },
        { "mp", 1, "${out-dir}/run-${run-num}.${ext}", "perform mean path method" },
        { "mp-adaptive", 1, "${out-dir}/run-${run-num}.${ext}", "perform mean path adaptive method" },
        { "gear1", 1, "${out-dir}/gear1-run.${ext}", "ODE simulation with Gear method, M=1" },
        { "gear2", 1, "${out-dir}/gear2-run.${ext}", "ODE simulation with Gear method, M=2" },
        { NULL, -1, NULL, NULL }
    };
    
    return info;
}


char *GetTypeNameOfBackEndProcessor( int type ) {
    switch( type ) {
        case 0: return "compiler";
        
        case 1: return "simulator";
        
        default: return "unknown";
    };
}




RET_VAL InitBackendProcessor( COMPILER_RECORD_T *record, BACK_END_PROCESSOR *backend ) {
    RET_VAL ret = SUCCESS;
    PROPERTIES *options = NULL;
    
    START_FUNCTION("InitBackendProcessor");
    

    backend->record = record;
    options = record->options;
    
    if( ( backend->encoding = options->GetProperty( options, REB2SAC_TARGET_ENCODING_KEY ) ) == NULL ) {
        backend->encoding = REB2SAC_DEFAULT_TARGET_ENCODING;
    } 
    
    backend->outputFilename = options->GetProperty( options, REB2SAC_OUT_KEY );   
        
    switch( backend->encoding[0] ) {
        case 'b':
            if( strcmp( backend->encoding, "bunker" ) == 0 ) {
                if( IS_FAILED( ( ret = _AddPostProcessingMethods( record, __MONTE_CARLO_POST_PROCESSING_METHODS ) ) ) ) {
                    return ret;
                }
                backend->Process = DoMonteCarloAnalysis;
                backend->Close = CloseMonteCarloAnalyzer;
            }
            else {
                fprintf( stderr, "target backend->encoding type %s is invalid", backend->encoding ); 
                return ErrorReport( FAILING, "InitBackendProcessor", "target backend->encoding type %s is invalid", backend->encoding );
            }
        break;
        
        case 'c':
            if( strcmp( backend->encoding, "ctmc-transient" ) == 0 ) {
                backend->Process = ProcessCTMCAnalysisBackend;
                backend->Close = CloseCTMCAnalysisBackend;
            }
            else {
                fprintf( stderr, "target backend->encoding type %s is invalid", backend->encoding ); 
                return ErrorReport( FAILING, "InitBackendProcessor", "target backend->encoding type %s is invalid", backend->encoding );
            }
        break;
        
        case 'd':
            if( strcmp( backend->encoding, "dot" ) == 0 ) {
                backend->Process = ProcessDotBackend;
                backend->Close = CloseDotBackend;
            }
            else {
                fprintf( stderr, "target backend->encoding type %s is invalid", backend->encoding ); 
                return ErrorReport( FAILING, "InitBackendProcessor", "target backend->encoding type %s is invalid", backend->encoding );
            }
        break;
        
        case 'e':
            if( strcmp( backend->encoding, "euler" ) == 0 ) {
                if( IS_FAILED( ( ret = _AddPostProcessingMethods( record, __ODE_POST_PROCESSING_METHODS ) ) ) ) {
                    return ret;
                }
                backend->Process = DoEulerSimulation;
                backend->Close = CloseEulerSimulation;
            }
            else if( strcmp( backend->encoding, "emc-sim" ) == 0 ) {
                if( IS_FAILED( ( ret = _AddPostProcessingMethods( record, __MONTE_CARLO_POST_PROCESSING_METHODS ) ) ) ) {
                    return ret;
                }
                backend->Process = DoMonteCarloAnalysis;
                backend->Close = CloseMonteCarloAnalyzer;
            }
            else {
                fprintf( stderr, "target backend->encoding type %s is invalid", backend->encoding ); 
                return ErrorReport( FAILING, "InitBackendProcessor", "target backend->encoding type %s is invalid", backend->encoding );
            }
        break;
        
        case 'g':
            if( strcmp( backend->encoding, "gillespie" ) == 0 ) {
                if( IS_FAILED( ( ret = _AddPostProcessingMethods( record, __MONTE_CARLO_POST_PROCESSING_METHODS ) ) ) ) {
                    return ret;
                }
                backend->Process = DoMonteCarloAnalysis;
                backend->Close = CloseMonteCarloAnalyzer;
            }
            else if(strcmp( backend->encoding, "gear1" ) == 0 ) {
                if( IS_FAILED( ( ret = _AddPostProcessingMethods( record, __ODE_POST_PROCESSING_METHODS ) ) ) ) {
                    return ret;
                }
                backend->Process = DoImplicitGear1Simulation;
                backend->Close = CloseImplicitGear1Simulation;
            }
            else if(strcmp( backend->encoding, "gear2" ) == 0 ) {
                if( IS_FAILED( ( ret = _AddPostProcessingMethods( record, __ODE_POST_PROCESSING_METHODS ) ) ) ) {
                    return ret;
                }
                backend->Process = DoImplicitGear2Simulation;
                backend->Close = CloseImplicitGear2Simulation;
            }
            else {
                fprintf( stderr, "target backend->encoding type %s is invalid", backend->encoding ); 
                return ErrorReport( FAILING, "InitBackendProcessor", "target backend->encoding type %s is invalid", backend->encoding );
            }
        break;

        case 'h':
            if( strcmp( backend->encoding, "hse2" ) == 0 ) {
                if( IS_FAILED( ( ret = _AddPostProcessingMethods( record, __SAC_POST_PROCESSING_METHODS ) ) ) ) {
                    return ret;
                }
                backend->Process = ProcessHse2Backend;
                backend->Close = CloseHse2Backend;
            }
            else if( strcmp( backend->encoding, "hse" ) == 0 ) {
                if( IS_FAILED( ( ret = _AddPostProcessingMethods( record, __SAC_POST_PROCESSING_METHODS ) ) ) ) {
                    return ret;
                }
                backend->Process = ProcessHseBackend;
                backend->Close = CloseHseBackend;
            }
            else {
                fprintf( stderr, "target backend->encoding type %s is invalid", backend->encoding ); 
                return ErrorReport( FAILING, "InitBackendProcessor", "target backend->encoding type %s is invalid", backend->encoding );
            }
        break;

        case 'm':
            if( strcmp( backend->encoding, "mpde" ) == 0 ) {
                if( IS_FAILED( ( ret = _AddPostProcessingMethods( record, __MONTE_CARLO_POST_PROCESSING_METHODS ) ) ) ) {
                    return ret;
                }
                backend->Process = DoMPDEMonteCarloAnalysis;
                backend->Close = CloseMPDEMonteCarloAnalyzer;
                backend->useMP = 0;
            }
            else if( strcmp( backend->encoding, "mp" ) == 0 ) {
                if( IS_FAILED( ( ret = _AddPostProcessingMethods( record, __MONTE_CARLO_POST_PROCESSING_METHODS ) ) ) ) {
                    return ret;
                }
                backend->Process = DoMPDEMonteCarloAnalysis;
                backend->Close = CloseMPDEMonteCarloAnalyzer;
                backend->useMP = 1;
            }
            else if( strcmp( backend->encoding, "mp-adaptive" ) == 0 ) {
                if( IS_FAILED( ( ret = _AddPostProcessingMethods( record, __MONTE_CARLO_POST_PROCESSING_METHODS ) ) ) ) {
                    return ret;
                }
                backend->Process = DoMPDEMonteCarloAnalysis;
                backend->Close = CloseMPDEMonteCarloAnalyzer;
                backend->useMP = 2;
            }
            else {
                fprintf( stderr, "target backend->encoding type %s is invalid", backend->encoding ); 
                return ErrorReport( FAILING, "InitBackendProcessor", "target backend->encoding type %s is invalid", backend->encoding );
            }
        break;
                
        case 'n':
            if( strcmp( backend->encoding, "nary-level" ) == 0 ) {
                if( IS_FAILED( ( ret = _AddPostProcessingMethods( record, __NARY_LEVEL1_POST_PROCESSING_METHODS ) ) ) ) {
                    return ret;
                }
                backend->Process = ProcessNaryLevelBackend;
                backend->Close = CloseNaryLevelBackend;
            }
            else if( strcmp( backend->encoding, "nary-level2" ) == 0 ) {
                if( IS_FAILED( ( ret = _AddPostProcessingMethods( record, __NARY_LEVEL2_POST_PROCESSING_METHODS ) ) ) ) {
                    return ret;
                }
                backend->Process = ProcessNaryLevelBackend2;
                backend->Close = CloseNaryLevelBackend2;
            }
            else if(strcmp( backend->encoding, "nmc" ) == 0 ) {
                if( IS_FAILED( ( ret = _AddPostProcessingMethods( record, __MONTE_CARLO_POST_PROCESSING_METHODS ) ) ) ) {
                    return ret;
                }
                backend->Process = DoMonteCarloAnalysis;
                backend->Close = CloseMonteCarloAnalyzer;
            }
            else {
                fprintf( stderr, "target backend->encoding type %s is invalid", backend->encoding ); 
                return ErrorReport( FAILING, "InitBackendProcessor", "target backend->encoding type %s is invalid", backend->encoding );
            }
        break;
        
        case 'p':
            if( strcmp( backend->encoding, "pili-gillespie-ci" ) == 0 ) {
                if( IS_FAILED( ( ret = _AddPostProcessingMethods( record, __MONTE_CARLO_POST_PROCESSING_METHODS ) ) ) ) {
                    return ret;
                }
                backend->Process = DoGillespiesForType1PiliWithCI;
                backend->Close = CloseGillespiesForType1PiliWithCI;
            }
            else {
                fprintf( stderr, "target backend->encoding type %s is invalid", backend->encoding ); 
                return ErrorReport( FAILING, "InitBackendProcessor", "target backend->encoding type %s is invalid", backend->encoding );
            }
            break;
        
        case 'r':
            if( strcmp( backend->encoding, "rk4imp" ) == 0 ) {
                if( IS_FAILED( ( ret = _AddPostProcessingMethods( record, __ODE_POST_PROCESSING_METHODS ) ) ) ) {
                    return ret;
                }
                backend->Process = DoImplicitRungeKutta4Simulation;
                backend->Close = CloseImplicitRungeKutta4Simulation;
            }
            else if(strcmp( backend->encoding, "rk8pd" ) == 0 ) {
                if( IS_FAILED( ( ret = _AddPostProcessingMethods( record, __ODE_POST_PROCESSING_METHODS ) ) ) ) {
                    return ret;
                }
                backend->Process = DoEmbeddedRungeKuttaPrinceDormandSimulation;
                backend->Close = CloseEmbeddedRungeKuttaPrinceDormandSimulation;
            }
            else if(strcmp( backend->encoding, "rkf45" ) == 0 ) {
                if( IS_FAILED( ( ret = _AddPostProcessingMethods( record, __ODE_POST_PROCESSING_METHODS ) ) ) ) {
                    return ret;
                }
                backend->Process = DoEmbeddedRungeKuttaFehlbergSimulation;
                backend->Close = CloseEmbeddedRungeKuttaFehlbergSimulation;
            }
            else {
                fprintf( stderr, "target backend->encoding type %s is invalid", backend->encoding ); 
                return ErrorReport( FAILING, "InitBackendProcessor", "target backend->encoding type %s is invalid", backend->encoding );
            }
            break;
        
        case 's':
            if( strcmp( backend->encoding, "sbml" ) == 0 ) {
                backend->Process = ProcessSBMLBackend;
                backend->Close = CloseSBMLBackend;
            }
            else if( strcmp( backend->encoding, "ssa-with-user-update" ) == 0 ) {
                if( IS_FAILED( ( ret = _AddPostProcessingMethods( record, __MONTE_CARLO_POST_PROCESSING_METHODS ) ) ) ) {
                    return ret;
                }
                backend->Process = DoSSAWithUserUpdateAnalysis;
                backend->Close = CloseSSAWithUserUpdateAnalyzer;
            }
            else {
                fprintf( stderr, "target backend->encoding type %s is invalid", backend->encoding ); 
                return ErrorReport( FAILING, "InitBackendProcessor", "target backend->encoding type %s is invalid", backend->encoding );
            }
        break;
        
        
        case 'x':
            if( strcmp( backend->encoding, "xhtml" ) == 0 ) {
                backend->Process = ProcessXHTMLBackend;
                backend->Close = CloseXHTMLBackend;
            }
            else {
                fprintf( stderr, "target backend->encoding type %s is invalid", backend->encoding ); 
                return ErrorReport( FAILING, "InitBackendProcessor", "target backend->encoding type %s is invalid", backend->encoding );
            }
        break;
        
        
        default:
            fprintf( stderr, "target backend->encoding type %s is invalid", backend->encoding ); 
        return ErrorReport( FAILING, "InitBackendProcessor", "target backend->encoding type %s is invalid", backend->encoding );
    }
   
    END_FUNCTION("InitBackendProcessor", SUCCESS );
    return ret;
}


static RET_VAL _AddPostProcessingMethods( COMPILER_RECORD_T *record, char *methodIDs[] ) {
    RET_VAL ret = SUCCESS;
    int i = 0;
    int j = 0;
    char buf[512];
    char *methodID = NULL;
    REB2SAC_PROPERTIES *properties = NULL;
        
    properties = record->properties;        
    for( i = 1, j = 0; methodIDs[j] != NULL; i++ ) {        
        sprintf( buf, "%s3.%i", REB2SAC_ABSTRACTION_METHOD_KEY_PREFIX, i );
        if( ( methodID = properties->GetProperty( properties, buf ) ) != NULL ) {
            continue;
        }
        if( IS_FAILED( ( ret = properties->SetProperty( properties, buf, methodIDs[j] ) ) ) ) {
            return ret;
        }
        j++;
    }
    
    return SUCCESS;
}

