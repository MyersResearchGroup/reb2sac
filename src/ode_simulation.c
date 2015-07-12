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
#include "gsl/gsl_vector.h"
#include "gsl/gsl_multiroots.h"
#include "gsl/gsl_errno.h"
#include "gsl/gsl_matrix.h"
#include "gsl/gsl_odeiv.h"
#include "gsl/gsl_linalg.h"
#include "ode_simulation.h"


static BOOL _IsModelConditionSatisfied( IR *ir );


static RET_VAL _InitializeRecord( ODE_SIMULATION_RECORD *rec, BACK_END_PROCESSOR *backend, IR *ir );
static RET_VAL _InitializeSimulation( ODE_SIMULATION_RECORD *rec, int runNum );
static RET_VAL _RunSimulation( ODE_SIMULATION_RECORD *rec );

static RET_VAL _CleanSimulation( ODE_SIMULATION_RECORD *rec );
static RET_VAL _CleanRecord( ODE_SIMULATION_RECORD *rec );

static RET_VAL _CalculateReactionRates( ODE_SIMULATION_RECORD *rec );
static RET_VAL _CalculateReactionRate( ODE_SIMULATION_RECORD *rec, REACTION *reaction );
static int _Update( double t, const double y[], double f[], ODE_SIMULATION_RECORD *rec );
static RET_VAL _Print( ODE_SIMULATION_RECORD *rec, double time );
static RET_VAL _PrintStatistics( ODE_SIMULATION_RECORD *rec, FILE *file);

static double fireEvents( ODE_SIMULATION_RECORD *rec, double time );
static void fireEvent( EVENT *event, ODE_SIMULATION_RECORD *rec );
static void ExecuteAssignments( ODE_SIMULATION_RECORD *rec );
static void SetEventAssignmentsNextValues( EVENT *event, ODE_SIMULATION_RECORD *rec );
static void SetEventAssignmentsNextValuesTime( EVENT *event, ODE_SIMULATION_RECORD *rec, double time );
static RET_VAL EvaluateAlgebraicRules( ODE_SIMULATION_RECORD *rec );
static RET_VAL ExecuteFastReactions( ODE_SIMULATION_RECORD *rec );

DLLSCOPE RET_VAL STDCALL DoODESimulation( BACK_END_PROCESSOR *backend, IR *ir ) {
    RET_VAL ret = SUCCESS;
    UINT i = 0;
    UINT runs = 1;
    char *namePrefix = NULL;
    static ODE_SIMULATION_RECORD rec;
    UINT timeout = 0;

    START_FUNCTION("DoODESimulation");

    if( !_IsModelConditionSatisfied( ir ) ) {
        return ErrorReport( FAILING, "DoODESimulation",
                            "Embedded Runge-Kutta-Fehlberg method cannot be applied to the model" );
    }

    if( IS_FAILED( ( ret = _InitializeRecord( &rec, backend, ir ) ) ) )  {
        return ErrorReport( ret, "DoODESimulation", "initialization of the record failed" );
    }
    runs = rec.runs;
    for( i = 1; i <= runs; i++ ) {
      timeout = 0;
      SeedRandomNumberGenerators( rec.seed );
      rec.seed = GetNextUniformRandomNumber(0,RAND_MAX);
      do {
	SeedRandomNumberGenerators( rec.seed );
	if( IS_FAILED( ( ret = _InitializeSimulation( &rec, i ) ) ) ) {
	  return ErrorReport( ret, "DoODESimulation", "initialization of the %i-th simulation failed", i );
	}
	timeout++;
      } while ( (ret == CHANGE) && (timeout <= (rec.speciesSize + rec.compartmentsSize + rec.symbolsSize)) );
      if (timeout > (rec.speciesSize + rec.compartmentsSize + rec.symbolsSize + 1)) {
	return ErrorReport( ret, "DoODESimulation", "Cycle detected in initial and rule assignments" );
      }
      if( IS_FAILED( ( ret = _RunSimulation( &rec ) ) ) ) {
        return ErrorReport( ret, "DoODESimulation", "%i-th simulation failed at time %f", i, rec.time );
      }
      if( IS_FAILED( ( ret = _CleanSimulation( &rec ) ) ) ) {
        return ErrorReport( ret, "DoODESimulation", "cleaning of the %i-th simulation failed", i );
      }
      printf("Run = %d\n",i);
      fflush(stdout);
    }

    END_FUNCTION("DoODESimulation", SUCCESS );
    return ret;
}

DLLSCOPE RET_VAL STDCALL CloseODESimulation( BACK_END_PROCESSOR *backend ) {
    RET_VAL ret = SUCCESS;
    ODE_SIMULATION_RECORD *rec = (ODE_SIMULATION_RECORD*)(backend->_internal1);

    START_FUNCTION("CloseODESimulation");

    if( IS_FAILED( ( ret = _CleanRecord( rec ) ) ) )  {
        return ErrorReport( ret, "CloseODESimulation", "cleaning of the record failed" );
    }

    END_FUNCTION("CloseODESimulation",  SUCCESS );
    return ret;
}


static BOOL _IsModelConditionSatisfied( IR *ir ) {
    return TRUE;
}


static RET_VAL _InitializeRecord( ODE_SIMULATION_RECORD *rec, BACK_END_PROCESSOR *backend, IR *ir ) {
    RET_VAL ret = SUCCESS;
    UINT32 i = 0;
    UINT32 j = 0;
    UINT32 k = 0;
    UINT32 l = 0;
    UINT32 algebraicVars = 0;
    double printInterval = 0.0;
    double minPrintInterval = 0.0;
    char buf[512];
    char *valueString = NULL;
    SPECIES *species = NULL;
    SPECIES **speciesArray = NULL;
    REACTION *reaction = NULL;
    REACTION **reactions = NULL;
    RULE *rule = NULL;
    RULE **ruleArray = NULL;
    RULE_MANAGER *ruleManager;
    CONSTRAINT *constraint = NULL;
    CONSTRAINT **constraintArray = NULL;
    CONSTRAINT_MANAGER *constraintManager;
    EVENT *event = NULL;
    EVENT **eventArray = NULL;
    EVENT_MANAGER *eventManager;
    EVENT_ASSIGNMENT *eventAssignment;
    COMPARTMENT *compartment = NULL;
    COMPARTMENT **compartmentArray = NULL;
    COMPARTMENT_MANAGER *compartmentManager;
    REB2SAC_SYMBOL *symbol = NULL;
    REB2SAC_SYMBOL **symbolArray = NULL;
    REB2SAC_SYMTAB *symTab;
    COMPILER_RECORD_T *compRec = backend->record;
    LINKED_LIST *list = NULL;
    REB2SAC_PROPERTIES *properties = NULL;
    double stoichiometry = 0.0;
    LINKED_LIST *edges = NULL;
    IR_EDGE *edge = NULL;

#if GET_SEED_FROM_COMMAND_LINE
    PROPERTIES *options = NULL;
#endif

    rec->encoding = backend->encoding;
    list = ir->GetListOfReactionNodes( ir );
    rec->reactionsSize = GetLinkedListSize( list );
    rec->numberFastReactions = 0;
    if (rec->reactionsSize!=0) {
      if( ( reactions = (REACTION**)MALLOC( rec->reactionsSize * sizeof(REACTION*) ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not allocate memory for reaction array" );
      }
      i = 0;
      ResetCurrentElement( list );
      while( ( reaction = (REACTION*)GetNextFromLinkedList( list ) ) != NULL ) {
        reactions[i] = reaction;
        i++;
	if (IsReactionFastInReactionNode( reaction )) {
	  rec->numberFastReactions++;
	  /*
	  if (IsReactionReversibleInReactionNode( reaction )) {
	    rec->numberFastReactions++;
	  }
	  */
	}
      }
    }
    rec->reactionArray = reactions;
    if( rec->numberFastReactions > 1 ) {
        return ErrorReport( FAILING, "_InitializeRecord",
                            "Simulator supports only a single fast reaction" );
    }

    if( ( ruleManager = ir->GetRuleManager( ir ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not get the rule manager" );
    }
    list = ruleManager->CreateListOfRules( ruleManager );
    rec->rulesSize = GetLinkedListSize( list );
    if ( rec->rulesSize > 0 ) {
      if( ( ruleArray = (RULE**)MALLOC( rec->rulesSize * sizeof(RULE*) ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not allocate memory for rules array" );
      }
    }
    i = 0;
    rec->algebraicRulesSize = 0;
    ResetCurrentElement( list );
    while( ( rule = (RULE*)GetNextFromLinkedList( list ) ) != NULL ) {
      ruleArray[i] = rule;
      i++;
      if ( GetRuleType( rule ) == RULE_TYPE_ALGEBRAIC ) {
	rec->algebraicRulesSize++;
      }
    }
    rec->ruleArray = ruleArray;

    if( ( symTab = ir->GetGlobalSymtab( ir ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not get the symbol table" );
    }
    list = symTab->GenerateListOfSymbols( symTab );
    rec->symbolsSize = GetLinkedListSize( list );
    if ( rec->symbolsSize > 0 ) {
      if( ( symbolArray = (REB2SAC_SYMBOL**)MALLOC( rec->symbolsSize * sizeof(REB2SAC_SYMBOL*) ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not allocate memory for symbols array" );
      }
    }
    i = 0;
    ResetCurrentElement( list );
    while( ( symbol = (REB2SAC_SYMBOL*)GetNextFromLinkedList( list ) ) != NULL ) {
        symbolArray[i] = symbol;
	if (IsSymbolAlgebraic( symbol )) {
	  algebraicVars++;
	}
        i++;
    }
    rec->symbolArray = symbolArray;

    if( ( compartmentManager = ir->GetCompartmentManager( ir ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not get the compartment manager" );
    }
    list = compartmentManager->CreateListOfCompartments( compartmentManager );
    rec->compartmentsSize = GetLinkedListSize( list );
    if ( rec->compartmentsSize > 0 ) {
      if( ( compartmentArray = (COMPARTMENT**)MALLOC( rec->compartmentsSize * sizeof(RULE*) ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not allocate memory for compartment array" );
      }
    }
    i = 0;
    ResetCurrentElement( list );
    while( ( compartment = (COMPARTMENT*)GetNextFromLinkedList( list ) ) != NULL ) {
        compartmentArray[i] = compartment;
	if (IsCompartmentAlgebraic( compartment )) {
	  algebraicVars++;
	}
        i++;
    }
    rec->compartmentArray = compartmentArray;

    list = ir->GetListOfSpeciesNodes( ir );
    rec->speciesSize = GetLinkedListSize( list );
    if ( rec->speciesSize > 0 ) {
      if( ( speciesArray = (SPECIES**)MALLOC( rec->speciesSize * sizeof(SPECIES*) ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not allocate memory for species array" );
      }
    }

    i = 0;
    rec->numberFastSpecies = 0;
    ResetCurrentElement( list );
    while( ( species = (SPECIES*)GetNextFromLinkedList( list ) ) != NULL ) {
        speciesArray[i] = species;
	if (IsSpeciesNodeAlgebraic( species )) {
	  algebraicVars++;
	}
	if (IsSpeciesNodeFast( species )) {
	  rec->numberFastSpecies++;
	}
        i++;
    }
    if (rec->numberFastSpecies > 0) {
      if( ( rec->fastCons = (double*)MALLOC( rec->numberFastSpecies * sizeof(double) ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not allocate memory for fastCons array" );
      }
      /*
      rec->fastStoicMatrix = gsl_matrix_alloc (rec->numberFastSpecies, 2*rec->numberFastReactions);
      k = 0;
      for (i = 0; i < rec->speciesSize; i++) {
	species = speciesArray[i];
	if (!HasBoundaryConditionInSpeciesNode( species ) && IsSpeciesNodeFast( species )) {
	  l = 0;
	  for(j = 0; j < rec->reactionsSize; j++ ) {
	    reaction = rec->reactionArray[j];
	    if (IsReactionFastInReactionNode( reaction )) {
	      stoichiometry = 0.0;
	      edges = GetReactantsInReactionNode( reaction );
	      ResetCurrentElement( edges );
	      while( ( edge = GetNextEdge( edges ) ) != NULL ) {
		if (species == GetSpeciesInIREdge( edge )) {
		  stoichiometry -= GetStoichiometryInIREdge( edge );
		}
	      }
	      edges = GetProductsInReactionNode( reaction );
	      ResetCurrentElement( edges );
	      while( ( edge = GetNextEdge( edges ) ) != NULL ) {
		if (species == GetSpeciesInIREdge( edge )) {
		  stoichiometry += GetStoichiometryInIREdge( edge );
		}
	      }
	      printf("k = %d l = %d stoic = %g\n",k,l,stoichiometry);
	      gsl_matrix_set (rec->fastStoicMatrix, k, l, stoichiometry);
	      l++;
	      if (IsReactionReversibleInReactionNode( reaction )) {
		printf("k = %d l = %d stoic = %g\n",k,l,stoichiometry);
		gsl_matrix_set (rec->fastStoicMatrix, k, l, (-1)*stoichiometry);
		l++;
	      }
	    }
	  }
	  k++;
	}
      }

      gsl_matrix *V = gsl_matrix_alloc (2*rec->numberFastReactions,2*rec->numberFastReactions);
      gsl_vector *S = gsl_vector_alloc (2*rec->numberFastReactions);
      gsl_vector *work = gsl_vector_alloc (2*rec->numberFastReactions);

      printf("A:\n");
      for (i = 0; i < rec->numberFastSpecies; i++) {
	for (j = 0; j < 2*rec->numberFastReactions; j++) {
	  printf("%g ",gsl_matrix_get(rec->fastStoicMatrix,i,j));
	}
	printf("\n");
      }

      gsl_linalg_SV_decomp(rec->fastStoicMatrix,V,S,work);
      printf("U:\n");
      for (i = 0; i < rec->numberFastSpecies; i++) {
	for (j = 0; j < 2*rec->numberFastReactions; j++) {
	  printf("%g ",gsl_matrix_get(rec->fastStoicMatrix,i,j));
	}
	printf("\n");
      }
      printf("S:\n");
      for (i = 0; i < 2*rec->numberFastReactions; i++) {
	printf("%g ",gsl_vector_get(S,i));
      }
      printf("\n");
      printf("V:\n");
      for (i = 0; i < 2*rec->numberFastReactions; i++) {
	for (j = 0; j < 2*rec->numberFastReactions; j++) {
	  printf("%g ",gsl_matrix_get(V,i,j));
	}
	printf("\n");
      }

      gsl_matrix *At = gsl_matrix_alloc(2*rec->numberFastReactions, rec->numberFastSpecies);
      gsl_matrix_transpose_memcpy(At,rec->fastStoicMatrix);
      printf("At:\n");
      for (i = 0; i < 2*rec->numberFastReactions; i++) {
	for (j = 0; j < rec->numberFastSpecies; j++) {
	  printf("%g ",gsl_matrix_get(At,i,j));
	}
	printf("\n");
      }
      int min = (2*rec->numberFastReactions < rec->numberFastSpecies?2*rec->numberFastReactions:rec->numberFastSpecies);
      gsl_vector *tau = gsl_vector_alloc(min);
      gsl_linalg_QR_decomp(At,tau);
      printf("R:\n");
      for (i = 0; i < 2*rec->numberFastReactions; i++) {
	for (j = 0; j < rec->numberFastSpecies; j++) {
	  if (j < i) gsl_matrix_set(At,i,j,0.0);
	  printf("%g ",gsl_matrix_get(At,i,j));
	}
	printf("\n");
      }
      */
    }

    rec->speciesArray = speciesArray;
    if ( algebraicVars > rec->algebraicRulesSize ) {
      return ErrorReport( FAILING, "_InitializeRecord", "model underdetermined" );
    } else if ( algebraicVars < rec->algebraicRulesSize ) {
      return ErrorReport( FAILING, "_InitializeRecord", "model overdetermined" );
    } 

    for (i = 0; i < rec->rulesSize; i++) {
      if ( GetRuleType( rec->ruleArray[i] ) == RULE_TYPE_ASSIGNMENT ||
	   GetRuleType( rec->ruleArray[i] ) == RULE_TYPE_RATE_ASSIGNMENT ) {
	for (j = 0; j < rec->speciesSize; j++) {
	  if ( strcmp( GetCharArrayOfString(GetRuleVar( rec->ruleArray[i] )),
		       GetCharArrayOfString(GetSpeciesNodeID( rec->speciesArray[j] ) ) ) == 0 ) {
	    SetRuleVarType( ruleArray[i], SPECIES_RULE );
	    SetRuleIndex( ruleArray[i], j );
	    break;
	  }
	}
	for (j = 0; j < rec->compartmentsSize; j++) {
	  if ( strcmp( GetCharArrayOfString(GetRuleVar( rec->ruleArray[i] )),
		       GetCharArrayOfString(GetCompartmentID( rec->compartmentArray[j] ) ) ) == 0 ) {
	    SetRuleVarType( ruleArray[i], COMPARTMENT_RULE );
	    SetRuleIndex( ruleArray[i], j );
	    break;
	  }
	}
	for (j = 0; j < rec->symbolsSize; j++) {
	  if ( strcmp( GetCharArrayOfString(GetRuleVar( rec->ruleArray[i] )),
		       GetCharArrayOfString(GetSymbolID( rec->symbolArray[j] ) ) ) == 0 ) {
	    SetRuleVarType( ruleArray[i], PARAMETER_RULE );
	    SetRuleIndex( ruleArray[i], j );
	    break;
	  }
	}
      }
    }

    if( ( rec->concentrations = (double*)MALLOC( (rec->symbolsSize + rec->compartmentsSize + rec->speciesSize) * sizeof( double ) ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not allocate memory for concentrations" );
    }

    if( ( rec->evaluator = CreateKineticLawEvaluater() ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not create evaluator" );
    }

    if( ( rec->findNextTime = CreateKineticLawFind_Next_Time() ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not create find next time" );
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

    if( ( valueString = properties->GetProperty( properties, ODE_SIMULATION_TIME_STEP ) ) == NULL ) {
        rec->timeStep = DEFAULT_ODE_SIMULATION_TIME_STEP;
    }
    else {
      if (strcmp(valueString,"inf")==0) {
	rec->timeStep = DBL_MAX;
      } else if( IS_FAILED( ( ret = StrToFloat( &(rec->timeStep), valueString ) ) ) ) {
	rec->timeStep = DEFAULT_ODE_SIMULATION_TIME_STEP;
      }
    }
    rec->originalTimeStep = rec->timeStep;

    if( ( valueString = properties->GetProperty( properties, ODE_SIMULATION_MIN_TIME_STEP ) ) == NULL ) {
        rec->minTimeStep = DEFAULT_ODE_SIMULATION_MIN_TIME_STEP;
    }
    else {
      if( IS_FAILED( ( ret = StrToFloat( &(rec->minTimeStep), valueString ) ) ) ) {
	rec->minTimeStep = DEFAULT_ODE_SIMULATION_MIN_TIME_STEP;
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

    rec->minPrintInterval = -1.0;
    if ((valueString = properties->GetProperty(properties, ODE_SIMULATION_PRINT_INTERVAL)) == NULL) {
        if ((valueString = properties->GetProperty(properties, ODE_SIMULATION_NUMBER_STEPS)) == NULL) {
            if ((valueString = properties->GetProperty(properties, ODE_SIMULATION_MINIMUM_PRINT_INTERVAL))
                    == NULL) {
                rec->numberSteps = DEFAULT_ODE_SIMULATION_NUMBER_STEPS_VALUE;
            } else {
                if (IS_FAILED((ret = StrToFloat(&(minPrintInterval), valueString)))) {
                    rec->numberSteps = DEFAULT_ODE_SIMULATION_NUMBER_STEPS_VALUE;
                } else {
                    rec->minPrintInterval = minPrintInterval;
                }
            }
        } else {
            if (IS_FAILED((ret = StrToUINT32(&(rec->numberSteps), valueString)))) {
                rec->numberSteps = DEFAULT_ODE_SIMULATION_NUMBER_STEPS_VALUE;
            }
        }
    } else {
        if (IS_FAILED((ret = StrToFloat(&(printInterval), valueString)))) {
            rec->numberSteps = DEFAULT_ODE_SIMULATION_NUMBER_STEPS_VALUE;
        } else {
            rec->numberSteps = rec->timeLimit / printInterval;
        }
    }

#if GET_SEED_FROM_COMMAND_LINE
    options = compRec->options;
    if( ( valueString = options->GetProperty( options, "random.seed" ) ) == NULL ) {
        rec->seed = DEFAULT_MONTE_CARLO_SIMULATION_RANDOM_SEED_VALUE;
    }
    else {
        if( IS_FAILED( ( ret = StrToUINT32( (UINT32*)&(rec->seed), valueString ) ) ) ) {
            rec->seed = DEFAULT_MONTE_CARLO_SIMULATION_RANDOM_SEED_VALUE;
        }
        TRACE_1("seed from command line is %i", rec->seed );
    }
#else
    if( ( valueString = properties->GetProperty( properties, MONTE_CARLO_SIMULATION_RANDOM_SEED ) ) == NULL ) {
        rec->seed = DEFAULT_MONTE_CARLO_SIMULATION_RANDOM_SEED_VALUE;
    }
    else {
        if( IS_FAILED( ( ret = StrToUINT32( &(rec->seed), valueString ) ) ) ) {
            rec->seed = DEFAULT_MONTE_CARLO_SIMULATION_RANDOM_SEED_VALUE;
        }
    }
#endif

    if( ( valueString = properties->GetProperty( properties, MONTE_CARLO_SIMULATION_START_INDEX ) ) == NULL ) {
        rec->startIndex = DEFAULT_MONTE_CARLO_SIMULATION_START_INDEX;
    }
    else {
      if( IS_FAILED( ( ret = StrToUINT32( (UINT32*)&(rec->startIndex), valueString ) ) ) ) {
	  rec->startIndex = DEFAULT_MONTE_CARLO_SIMULATION_START_INDEX;
        }
    }

    if( ( valueString = properties->GetProperty( properties, MONTE_CARLO_SIMULATION_RUNS ) ) == NULL ) {
        rec->runs = DEFAULT_MONTE_CARLO_SIMULATION_RUNS_VALUE;
    }
    else {
        if( IS_FAILED( ( ret = StrToUINT32( &(rec->runs), valueString ) ) ) ) {
            rec->runs = DEFAULT_MONTE_CARLO_SIMULATION_RUNS_VALUE;
        }
    }

    if( ( rec->outDir = properties->GetProperty( properties, ODE_SIMULATION_OUT_DIR ) ) == NULL ) {
        rec->outDir = DEFAULT_ODE_SIMULATION_OUT_DIR_VALUE;
    }

    if( ( rec->printer = CreateSimulationPrinter( backend, compartmentArray, rec->compartmentsSize,
						  speciesArray, rec->speciesSize,
						  symbolArray, rec->symbolsSize ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not create simulation printer" );
    }

    if( ( constraintManager = ir->GetConstraintManager( ir ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not get the constraint manager" );
    }
    list = constraintManager->CreateListOfConstraints( constraintManager );
    rec->constraintsSize = GetLinkedListSize( list );
    if ( rec->constraintsSize > 0 ) {
      if( ( constraintArray = (CONSTRAINT**)MALLOC( rec->constraintsSize * sizeof(CONSTRAINT*) ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not allocate memory for constraints array" );
      }
    }
    i = 0;
    ResetCurrentElement( list );
    while( ( constraint = (CONSTRAINT*)GetNextFromLinkedList( list ) ) != NULL ) {
        constraintArray[i] = constraint;
        i++;
    }
    rec->constraintArray = constraintArray;

    if( ( rec->decider =
        CreateSimulationRunTerminationDecider( backend, speciesArray, rec->speciesSize, reactions, rec->reactionsSize,
					       rec->constraintArray, rec->constraintsSize, rec->evaluator, TRUE, rec->timeLimit ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not create simulation printer" );
    }

    if( ( eventManager = ir->GetEventManager( ir ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not get the event manager" );
    }
    list = eventManager->CreateListOfEvents( eventManager );
    rec->eventsSize = GetLinkedListSize( list );
    if ( rec->eventsSize > 0 ) {
      if( ( eventArray = (EVENT**)MALLOC( rec->eventsSize * sizeof(EVENT*) ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not allocate memory for events array" );
      }
    }
    i = 0;
    ResetCurrentElement( list );
    while( ( event = (EVENT*)GetNextFromLinkedList( list ) ) != NULL ) {
        eventArray[i] = event;
        i++;
    }
    rec->eventArray = eventArray;

    for (i = 0; i < rec->eventsSize; i++) {
      list = GetEventAssignments( rec->eventArray[i] );
      ResetCurrentElement( list );
      while( ( eventAssignment = (EVENT_ASSIGNMENT*)GetNextFromLinkedList( list ) ) != NULL ) {
	for (j = 0; j < rec->speciesSize; j++) {
	  if ( strcmp( GetCharArrayOfString(eventAssignment->var),
		       GetCharArrayOfString(GetSpeciesNodeID( rec->speciesArray[j] ) ) ) == 0 ) {
	    SetEventAssignmentVarType( eventAssignment, SPECIES_EVENT_ASSIGNMENT );
	    SetEventAssignmentIndex( eventAssignment, j );
	    break;
	  }
	}
	for (j = 0; j < rec->compartmentsSize; j++) {
	  if ( strcmp( GetCharArrayOfString(eventAssignment->var),
		       GetCharArrayOfString(GetCompartmentID( rec->compartmentArray[j] ) ) ) == 0 ) {
	    SetEventAssignmentVarType( eventAssignment, COMPARTMENT_EVENT_ASSIGNMENT );
	    SetEventAssignmentIndex( eventAssignment, j );
	    break;
	  }
	}
	for (j = 0; j < rec->symbolsSize; j++) {
	  if ( strcmp( GetCharArrayOfString(eventAssignment->var),
		       GetCharArrayOfString(GetSymbolID( rec->symbolArray[j] ) ) ) == 0 ) {
	    SetEventAssignmentVarType( eventAssignment, PARAMETER_EVENT_ASSIGNMENT );
	    SetEventAssignmentIndex( eventAssignment, j );
	    break;
	  }
	}
      }
    }

    backend->_internal1 = (CADDR_T)rec;

    return ret;
}

static RET_VAL _InitializeSimulation( ODE_SIMULATION_RECORD *rec, int runNum ) {
    RET_VAL ret = SUCCESS;
    char filenameStem[512];
    double concentration = 0.0;
    double oldSize = 0.0;
    double compSize = 0.0;
    double param = 0.0;
    double *concentrations = rec->concentrations;
    UINT32 i = 0;
    UINT32 size = 0;
    SPECIES *species = NULL;
    SPECIES **speciesArray = rec->speciesArray;
    COMPARTMENT *compartment = NULL;
    COMPARTMENT **compartmentArray = rec->compartmentArray;
    REB2SAC_SYMBOL *symbol = NULL;
    REB2SAC_SYMBOL **symbolArray = rec->symbolArray;
    REACTION *reaction = NULL;
    REACTION **reactionArray = rec->reactionArray;
    KINETIC_LAW_EVALUATER *evaluator = rec->evaluator;
    SIMULATION_PRINTER *printer = rec->printer;
    KINETIC_LAW *law = NULL;
    BOOL change = FALSE;
    char filename[512];
    FILE *file = NULL;

    sprintf( filenameStem, "%s%crun-%i", rec->outDir, FILE_SEPARATOR, (runNum + rec->startIndex - 1) );
    if( IS_FAILED( (  ret = printer->PrintStart( printer, filenameStem ) ) ) ) {
        return ret;
    }
    if( IS_FAILED( (  ret = printer->PrintHeader( printer ) ) ) ) {
        return ret;
    }
    rec->time = 0.0;
    rec->nextPrintTime = 0.0;
    size = rec->compartmentsSize;
    for( i = 0; i < size; i++ ) {
        compartment = compartmentArray[i];
	if ( (law = (KINETIC_LAW*)GetInitialAssignmentInCompartment( compartment )) == NULL ) {
	  compSize = GetSizeInCompartment( compartment );
	} else {
	  law = CloneKineticLaw( law );
	  SimplifyInitialAssignment(law);
	  if (law->valueType == KINETIC_LAW_VALUE_TYPE_REAL) {
	    compSize = GetRealValueFromKineticLaw(law);
	  } else if (law->valueType == KINETIC_LAW_VALUE_TYPE_INT) {
	    compSize = (double)GetIntValueFromKineticLaw(law);
	  }
	  if (GetSizeInCompartment( compartment ) != compSize) {
	    oldSize = GetSizeInCompartment( compartment );
	    SetSizeInCompartment( compartment, compSize );
	    for (i = 0; i <  rec->speciesSize; i++) {
	      species = speciesArray[i];
	      if (GetCompartmentInSpeciesNode( species ) == compartment) {
		if( !IsInitialQuantityInAmountInSpeciesNode( species ) ) {
		  concentration = GetInitialAmountInSpeciesNode( species );
		  SetInitialAmountInSpeciesNode( species, concentration / oldSize * compSize );
		}
	      }
	    }
	    change = TRUE;
	  }
	  FreeKineticLaw( &(law) );
	}
        if( IS_FAILED( ( ret = SetCurrentSizeInCompartment( compartment, compSize ) ) ) ) {
            return ret;
        }
	concentrations[rec->speciesSize + i] = compSize;
    }
    size = rec->speciesSize;
    for( i = 0; i < size; i++ ) {
        species = speciesArray[i];
	if ( (law = (KINETIC_LAW*)GetInitialAssignmentInSpeciesNode( species )) == NULL ) {
	  if( IsInitialQuantityInAmountInSpeciesNode( species ) ) {
	    concentration = GetInitialAmountInSpeciesNode( species );
	    SetAmountInSpeciesNode( species, concentration );
	  }
	  else {
	    concentration = GetInitialConcentrationInSpeciesNode( species );
	    SetConcentrationInSpeciesNode( species, concentration );
	    concentration = GetAmountInSpeciesNode( species );
	  }
	} else {
	  law = CloneKineticLaw( law );
	  SimplifyInitialAssignment(law);
	  if (law->valueType == KINETIC_LAW_VALUE_TYPE_REAL) {
	    concentration = GetRealValueFromKineticLaw(law);
	  } else if (law->valueType == KINETIC_LAW_VALUE_TYPE_INT) {
	    concentration = (double)GetIntValueFromKineticLaw(law);
	  }
	  if( HasOnlySubstanceUnitsInSpeciesNode( species ) ) {
	    if (GetAmountInSpeciesNode( species ) != concentration) {
	      SetAmountInSpeciesNode( species, concentration );
	      change = TRUE;
	    }
	    SetInitialAmountInSpeciesNode( species, concentration );
	  }
	  else {
	    compSize = GetCurrentSizeInCompartment( GetCompartmentInSpeciesNode( species ) ); 
	    if (compSize == -1.0) {
	      compSize = 1.0;
	    }
	    if (GetAmountInSpeciesNode( species ) != concentration * compSize) {
	      SetAmountInSpeciesNode( species, concentration * compSize );
	      change = TRUE;
	    }
	    SetInitialConcentrationInSpeciesNode( species, concentration );
	    concentration = concentration * compSize;
	  }
	  FreeKineticLaw( &(law) );
	}
/*         if( IS_FAILED( ( ret = SetConcentrationInSpeciesNode( species, concentration ) ) ) ) { */
/*             return ret; */
/*         } */
        concentrations[i] = concentration;
    }
    size = rec->symbolsSize;
    for( i = 0; i < size; i++ ) {
        symbol = symbolArray[i];
	if ( (law = (KINETIC_LAW*)GetInitialAssignmentInSymbol( symbol )) == NULL ) {
	  param = GetRealValueInSymbol( symbol );
	  //if (isnan(param)) param = 0;
	} else {
	  law = CloneKineticLaw( law );
	  SimplifyInitialAssignment(law);
	  if (law->valueType == KINETIC_LAW_VALUE_TYPE_REAL) {
	    param = GetRealValueFromKineticLaw(law);
	  } else if (law->valueType == KINETIC_LAW_VALUE_TYPE_INT) {
	    param = (double)GetIntValueFromKineticLaw(law);
	  } else {
	    param = 0;
	  }
	  if( GetRealValueInSymbol( symbol ) != param ) {
	    SetRealValueInSymbol( symbol, param );
	    change = TRUE;
	  }
	  FreeKineticLaw( &(law) );
	}
	if( IS_FAILED( ( ret = SetCurrentRealValueInSymbol( symbol, param ) ) ) ) {
	  return ret;
	}
	concentrations[rec->compartmentsSize + rec->speciesSize + i] = param;
    }
    for (i = 0; i < rec->eventsSize; i++) {
      /* SetTriggerEnabledInEvent( rec->eventArray[i], FALSE ); */
      /* Use the line below to support true SBML semantics, i.e., nothing can be trigger at t=0 */
      if (rec->evaluator->EvaluateWithCurrentConcentrationsDeter( rec->evaluator,
							     (KINETIC_LAW*)GetTriggerInEvent( rec->eventArray[i] ) )) {
	if (GetTriggerInitialValue( rec->eventArray[i] )) {
    	  SetTriggerEnabledInEvent( rec->eventArray[i], TRUE );
	} else {
	  SetTriggerEnabledInEvent( rec->eventArray[i], FALSE );
	}
      } else {
    	  SetTriggerEnabledInEvent( rec->eventArray[i], FALSE );
      }
    }
    if (rec->algebraicRulesSize > 0) {
      EvaluateAlgebraicRules( rec );
    }
    if (rec->numberFastSpecies > 0) {
      ExecuteFastReactions( rec );
    }

    sprintf( filename, "%s%cstatistics.txt", rec->outDir, FILE_SEPARATOR );
    if( ( file = fopen( filename, "w" ) ) == NULL ) {
    	return ErrorReport( FAILING, "_InitializeSimulation", "could not create a statistics file" );
    }
    if( IS_FAILED( ( ret = _PrintStatistics( rec, file ) ) ) ) {
    	return ret;
    }
    fclose( file );

    if (change)
      return CHANGE;
    return ret;
}

static RET_VAL _RunSimulation( ODE_SIMULATION_RECORD *rec ) {
    RET_VAL ret = SUCCESS;
    int status = GSL_SUCCESS;
    int i;
    double h = ODE_SIMULATION_H;
    double *y = rec->concentrations;
    double *y_err;
    double nextPrintTime = rec->time;
    double minPrintInterval = rec->minPrintInterval;
    double time = rec->time;
    double timeLimit = rec->timeLimit;
    double nextEventTime;
    double maxTime;
    double minTimeStep = rec->minTimeStep;
    int curStep = 0;
    double numberSteps = rec->numberSteps;
    SIMULATION_PRINTER *printer = NULL;
    SIMULATION_RUN_TERMINATION_DECIDER *decider = NULL;
    const gsl_odeiv_step_type *stepType;
    gsl_odeiv_step *step = NULL;
    gsl_odeiv_control *control = NULL;
    gsl_odeiv_evolve *evolve = NULL;
    int size = rec->speciesSize + rec->compartmentsSize + rec->symbolsSize;
    gsl_odeiv_system system =
    {
        (int(*)(double , const double [], double [], void*))_Update,
        NULL,
        size,
        rec
    };
    if( ( y_err = (double*)MALLOC( (rec->symbolsSize + rec->compartmentsSize + rec->speciesSize) * sizeof( double ) ) ) == NULL ) {
        return ErrorReport( FAILING, "_RunSimulation", "could not allocate memory for y_err" );
    }
    if (strcmp(rec->encoding,"rkf45")==0) {
      stepType = gsl_odeiv_step_rkf45;
    } else if (strcmp(rec->encoding,"rk8pd")==0) {
      stepType = gsl_odeiv_step_rk8pd;
    } else if (strcmp(rec->encoding,"rk4imp")==0) {
      stepType = gsl_odeiv_step_rk4imp;
    } else if (strcmp(rec->encoding,"gear1")==0) {
      stepType = gsl_odeiv_step_gear1;
    } else {
      stepType = gsl_odeiv_step_gear2;
    } 
 
    step = gsl_odeiv_step_alloc( stepType, size );
    control = gsl_odeiv_control_y_new( rec->absoluteError, ODE_SIMULATION_LOCAL_ERROR );
    evolve = gsl_odeiv_evolve_alloc( size );

    /* This is a hack as it should be done in InitializeSimulation, not sure why it does not stick */
    if (rec->algebraicRulesSize > 0) {
      EvaluateAlgebraicRules( rec );
    }
    if (rec->numberFastSpecies > 0) {
      ExecuteFastReactions( rec );
    }

    printer = rec->printer;
    decider = rec->decider;
    while( !(decider->IsTerminationConditionMet( decider, NULL, time )) ) {
      nextEventTime = fireEvents( rec, time );
      if (rec->algebraicRulesSize > 0) {
	EvaluateAlgebraicRules( rec );
      }
      if (rec->numberFastSpecies > 0) {
	ExecuteFastReactions( rec );
      }
      if (decider->IsTerminationConditionMet( decider, NULL, time )) break;
      if( IS_FAILED( ( ret = _Print( rec, time ) ) ) ) {
	return ret;
      }
      curStep++;
      if (minPrintInterval == -1.0) {
	nextPrintTime = (curStep/numberSteps) * timeLimit;
      } else {
	nextPrintTime = time + minPrintInterval;
	if (nextPrintTime > timeLimit) 
	  nextPrintTime = timeLimit;
      }
      while( time < nextPrintTime ) {
	if ((rec->timeStep == DBL_MAX) || (maxTime + rec->timeStep > nextPrintTime)) {
	  if (minPrintInterval == -1.0) {
	    maxTime = nextPrintTime;
	  } else {
	    maxTime = timeLimit;
	  }
	} else {
	  maxTime = maxTime + rec->timeStep;
	  if (maxTime > timeLimit) {
	    maxTime = timeLimit;
	  }
	}
	nextEventTime = fireEvents( rec, time );
	if (nextEventTime==-2.0) {
	  return FAILING;
	}
	if ((nextEventTime != -1) && (nextEventTime < maxTime)) {
	  maxTime = nextEventTime;
	}
	if (rec->numberFastSpecies > 0) {
	  ExecuteFastReactions( rec );
	}
	if (time > maxTime) {
	  maxTime = time;
	}
	if (h >= minTimeStep) {
	  status = gsl_odeiv_evolve_apply( evolve, control, step,
					   &system, &time, maxTime,
					   &h, y );
	  //if (h < ODE_SIMULATION_H) h = ODE_SIMULATION_H;
	} else {
	  h = minTimeStep;
	  if (time + h > maxTime) {
	    h = maxTime - time;
	  }
	  status = gsl_odeiv_step_apply( step, time, h, y, y_err, NULL, NULL, &system );
	  time = time + h;
	}
	if (status == GSL_ETOL) {
	  maxTime = time + rec->timeStep;
	  for( i = 0; i < rec->speciesSize; i++ ) {
	    SetAmountInSpeciesNode( rec->speciesArray[i], y[i] );
	  }
	  for( i = 0; i < rec->compartmentsSize; i++ ) {
	    SetCurrentSizeInCompartment( rec->compartmentArray[i], y[rec->speciesSize + i] );
	  }
	  for( i = 0; i < rec->symbolsSize; i++ ) {
	    SetCurrentRealValueInSymbol( rec->symbolArray[i], y[rec->speciesSize + rec->compartmentsSize + i] );
	  }
	  ExecuteAssignments( rec );
	  status = GSL_SUCCESS;
	  //printf("Adjusting time step\n");
	}
	if( status != GSL_SUCCESS ) {
	  return FAILING;
	}
      }
      if (time > 0.0) printf("Time = %g\n",time);
      fflush(stdout);
    }
    nextEventTime = fireEvents( rec, time );
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

static RET_VAL _CleanSimulation( ODE_SIMULATION_RECORD *rec ) {
    RET_VAL ret = SUCCESS;
    return ret;
}

static RET_VAL _CleanRecord( ODE_SIMULATION_RECORD *rec ) {
    RET_VAL ret = SUCCESS;
    char filename[512];
    FILE *file = NULL;
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

static RET_VAL _PrintStatistics(ODE_SIMULATION_RECORD *rec, FILE *file) {
	RET_VAL ret = SUCCESS;
	double stoichiometry = 0;
	UINT32 i = 0;
	UINT32 j = 0;
	UINT32 reactionsSize = rec->reactionsSize;
	UINT32 speciesSize = rec->speciesSize;
	UINT32 symbolsSize = rec->symbolsSize;
	REACTION *reaction = NULL;
	REACTION **reactionArray = rec->reactionArray;
	SPECIES *species = NULL;
	SPECIES **speciesArray = rec->speciesArray;
	REB2SAC_SYMBOL *symbol = NULL;
	REB2SAC_SYMBOL **symbolArray = rec->symbolArray;
	REB2SAC_SYMBOL *speciesRef = NULL;
	REB2SAC_SYMBOL *convFactor = NULL;
	IR_EDGE *edge = NULL;
	LINKED_LIST *edges = NULL;

	if ((speciesSize <= 0) || (reactionsSize <= 0)) return ret;

	fprintf( file, "Parameter Values:" NEW_LINE);

	for (i = 0; i < symbolsSize; i++) {
		symbol = symbolArray[i];
		if (IsRealValueSymbol(symbol)) {
			fprintf( file, "%s = %f" NEW_LINE, *GetSymbolID(symbol), GetRealValueInSymbol(symbol));
		}
	}
	fprintf( file, NEW_LINE);

	fprintf( file, "Initial State Vector:" NEW_LINE);

	for (i = 0; i < speciesSize; i++) {
		species = speciesArray[i];
		SetAmountInSpeciesNode(species, GetInitialAmountInSpeciesNode(species));
		fprintf( file, "%s = %f" NEW_LINE, *GetSpeciesNodeID(species), GetInitialAmountInSpeciesNode(species));
	}
	fprintf( file, NEW_LINE);

	gsl_matrix *delta_matrix = gsl_matrix_alloc(speciesSize, reactionsSize);
	gsl_matrix *reactant_matrix = gsl_matrix_alloc(speciesSize, reactionsSize);

	for (i = 0; i < reactionsSize; i++) {
		for (j = 0; j < speciesSize; j++) {
			gsl_matrix_set(delta_matrix, j, i, 0);
			gsl_matrix_set(reactant_matrix, j, i, 0);
		}
	}

	fprintf( file, "Initial Reaction Rate Array:" NEW_LINE);

	for (i = 0; i < reactionsSize; i++) {
		reaction = reactionArray[i];
		fprintf( file, "%f" NEW_LINE, GetReactionRate(reaction));
		edges = GetReactantEdges((IR_NODE*) reaction);
		ResetCurrentElement(edges);
		while ((edge = GetNextEdge(edges)) != NULL) {
			speciesRef = GetSpeciesRefInIREdge(edge);
			if (speciesRef) {
				stoichiometry = GetCurrentRealValueInSymbol(speciesRef);
			} else {
				stoichiometry = GetStoichiometryInIREdge(edge);
			}
			species = GetSpeciesInIREdge(edge);
			if ((convFactor = GetConversionFactorInSpeciesNode(species)) != NULL) {
				stoichiometry *= GetCurrentRealValueInSymbol(convFactor);
			}
			if (HasBoundaryConditionInSpeciesNode(species))
				continue;
			for (j = 0; j < speciesSize; j++) {
				if (species == speciesArray[j]) {
					gsl_matrix_set(delta_matrix, j, i, gsl_matrix_get(delta_matrix, j, i) - stoichiometry);
					gsl_matrix_set(reactant_matrix, j, i, gsl_matrix_get(reactant_matrix, j, i)+stoichiometry);
				}
			}
		}
		edges = GetProductEdges((IR_NODE*) reaction);
		ResetCurrentElement(edges);
		while ((edge = GetNextEdge(edges)) != NULL) {
			speciesRef = GetSpeciesRefInIREdge(edge);
			if (speciesRef) {
				stoichiometry = GetCurrentRealValueInSymbol(speciesRef);
			} else {
				stoichiometry = GetStoichiometryInIREdge(edge);
			}
			species = GetSpeciesInIREdge(edge);
			if ((convFactor = GetConversionFactorInSpeciesNode(species)) != NULL) {
				stoichiometry *= GetCurrentRealValueInSymbol(convFactor);
			}
			if (HasBoundaryConditionInSpeciesNode(species))
				continue;
			for (j = 0; j < speciesSize; j++) {
				if (species == speciesArray[j]) {
					gsl_matrix_set(delta_matrix, j, i, gsl_matrix_get(delta_matrix, j, i) + stoichiometry);
				}
			}
		}
	}
	fprintf( file, NEW_LINE);

	fprintf( file, "Reaction Rate Equation Array:" NEW_LINE);

	for (i = 0; i < reactionsSize; i++) {
		reaction = reactionArray[i];
		fprintf( file, "%s" NEW_LINE, *ToStringKineticLaw(GetKineticLawInReactionNode(reaction)));
	}
	fprintf( file, NEW_LINE);

	fprintf( file, "Reactant Matrix:" NEW_LINE);
	for (i = 0; i < reactionsSize; i++) {
		for (j = 0; j < speciesSize; j++) {
			fprintf( file, "%f ", gsl_matrix_get(reactant_matrix, j, i));
		}
		fprintf( file, NEW_LINE);
	}

	fprintf( file, NEW_LINE);

	fprintf( file, "Delta Matrix:" NEW_LINE);
	for (i = 0; i < reactionsSize; i++) {
		for (j = 0; j < speciesSize; j++) {
			fprintf(file, "%f ", gsl_matrix_get(delta_matrix, j, i));
		}
		fprintf(file, NEW_LINE);
	}

	return ret;
}

static RET_VAL _Print( ODE_SIMULATION_RECORD *rec, double time ) {
    RET_VAL ret = SUCCESS;
    SIMULATION_PRINTER *printer = rec->printer;

    if( IS_FAILED( ( ret = printer->PrintValues( printer, time ) ) ) ) {
        return ret;
    }
    return ret;
}



static RET_VAL _CalculateReactionRates( ODE_SIMULATION_RECORD *rec ) {
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


static RET_VAL _CalculateReactionRate( ODE_SIMULATION_RECORD *rec, REACTION *reaction ) {
    RET_VAL ret = SUCCESS;
    double rate = 0.0;
    SPECIES *species = NULL;
    KINETIC_LAW *law = NULL;
    KINETIC_LAW_EVALUATER *evaluator = rec->evaluator;

    law = GetKineticLawInReactionNode( reaction );
    rate = evaluator->EvaluateWithCurrentConcentrationsDeter( evaluator, law );
    if( !( rate < DBL_MAX ) ) {
        rate = 0.0;
    }
    if( IS_FAILED( ( ret = SetReactionRate( reaction, rate ) ) ) ) {
        return ret;
    }
    return ret;
}

static double fireEvents( ODE_SIMULATION_RECORD *rec, double time ) {
    int i;
    double nextEventTime;
    BOOL triggerEnabled;
    double deltaTime;
    BOOL eventFired = FALSE;
    double firstEventTime = -1.0;
    int eventToFire = -1;
    double prMax,prMax2;
    double priority = 0.0;
    double randChoice = 0.0;

    //printf("FireEvents at %g\n",time);
    do {
      eventFired = FALSE;
      eventToFire = -1;
      for (i = 0; i < rec->eventsSize; i++) {
	nextEventTime = GetNextEventTimeInEvent( rec->eventArray[i] );
	triggerEnabled = GetTriggerEnabledInEvent( rec->eventArray[i] );
	//printf("time = %g i = %d nextEventTime = %g triggerEnabled = %d\n",time,i,nextEventTime,triggerEnabled);
	if (nextEventTime != -1.0) {
	  /* Disable event, if necessary */
	  if ((triggerEnabled) && (GetTriggerCanBeDisabled( rec->eventArray[i] ))) {
	    //printf("Checking\n");
	    if (!rec->evaluator->EvaluateWithCurrentConcentrationsDeter( rec->evaluator,
									 (KINETIC_LAW*)GetTriggerInEvent( rec->eventArray[i] ) )) { 
	      //printf("Disabled\n");
	      nextEventTime = -1.0;
	      SetNextEventTimeInEvent( rec->eventArray[i], -1.0 );
	      SetTriggerEnabledInEvent( rec->eventArray[i], FALSE );
	      continue;
	    }
	  }
	  if (time >= nextEventTime) {
	    if (GetPriorityInEvent( rec->eventArray[i] )==NULL) {
	      priority = 0;
	    }
	    else {
	      priority = rec->evaluator->EvaluateWithCurrentConcentrations( rec->evaluator,
								     (KINETIC_LAW*)GetPriorityInEvent( rec->eventArray[i] ) );
	    }
	    if ((eventToFire==(-1)) || (priority > prMax)) {
	      eventToFire = i;
	      prMax = priority;
	      prMax2=GetNextUniformRandomNumber(0,1);	   
	    } else if (priority == prMax) {
	      randChoice=GetNextUniformRandomNumber(0,1);	   
	      if (randChoice > prMax2) {
		eventToFire = i;
		prMax2 = randChoice;
	      }
	    }
	  } else {
	    /* Determine time to next event */
	    if ((firstEventTime == -1.0) || (nextEventTime < firstEventTime)) {
	      firstEventTime = nextEventTime;
	    }
	  }
	}
	/* Try to find time to next event trigger */
	nextEventTime = time + 
	  rec->findNextTime->FindNextTimeWithCurrentAmounts( rec->findNextTime,
							     (KINETIC_LAW*)GetTriggerInEvent( rec->eventArray[i] ));
	if ((firstEventTime == -1.0) || (nextEventTime < firstEventTime)) {
	  firstEventTime = nextEventTime;
	}
	if (!triggerEnabled) {
	  /* Check if event has been triggered */
	  if (rec->evaluator->EvaluateWithCurrentConcentrationsDeter( rec->evaluator,
								 (KINETIC_LAW*)GetTriggerInEvent( rec->eventArray[i] ) )) {
	    //printf("Enabled at %g\n",time);
	    SetTriggerEnabledInEvent( rec->eventArray[i], TRUE );
	    /* Calculate delay until the event fires */
	    if (GetDelayInEvent( rec->eventArray[i] )==NULL) {
	      deltaTime = 0;
	    }
	    else {
	      deltaTime = rec->evaluator->EvaluateWithCurrentConcentrations( rec->evaluator,
								      (KINETIC_LAW*)GetDelayInEvent( rec->eventArray[i] ) );
	    }
	    if (deltaTime == 0) eventFired = TRUE;
	    if (deltaTime >= 0) {
	      /* Set time for event to fire and get assignment values, if necessary */
	      //printf("Setting event at %g\n",time);
	      SetNextEventTimeInEvent( rec->eventArray[i], time + deltaTime );
	      if (GetUseValuesFromTriggerTime( rec->eventArray[i] )) {
		SetEventAssignmentsNextValuesTime( rec->eventArray[i], rec, time + deltaTime ); 
	      }
	      if ((firstEventTime == -1.0) || (time + deltaTime < firstEventTime)) {
		firstEventTime = time + deltaTime;
	      }
	    } /* else if (deltaTime == 0) {
	      SetEventAssignmentsNextValues( rec->eventArray[i], rec ); 
	      fireEvent( rec->eventArray[i], rec );
	      eventFired = TRUE;
	      } */ else {
	      ErrorReport( FAILING, "_Update", "delay for event evaluates to a negative number" );
	      return -2;
	    }
	  }
	} else {
	  /* Set trigger enabled to false, if it has become disabled */
	  if (!rec->evaluator->EvaluateWithCurrentConcentrationsDeter( rec->evaluator,
							   (KINETIC_LAW*)GetTriggerInEvent( rec->eventArray[i] ) )) {
	    SetTriggerEnabledInEvent( rec->eventArray[i], FALSE );
	    //printf("Disabled at %g\n",time);
	  } 
	}
      }
      //if (eventToFire >= 0)
      //printf("eventToFire = %d\n",eventToFire);
      /* Fire event */
      if (eventToFire >= 0) {
	if (!GetUseValuesFromTriggerTime( rec->eventArray[eventToFire] )) {
	  SetEventAssignmentsNextValues( rec->eventArray[eventToFire], rec ); 
	}
	rec->time = time;
	fireEvent( rec->eventArray[eventToFire], rec );
	SetNextEventTimeInEvent( rec->eventArray[eventToFire], -1.0 );
	eventFired = TRUE;
	eventToFire = -1;
	firstEventTime = -1;

	/* When an event fires, update algebraic rules and fast reactions */
	ExecuteAssignments( rec );
	if (rec->algebraicRulesSize > 0) {
	  EvaluateAlgebraicRules( rec );
	}
	if (rec->numberFastSpecies > 0) {
	  ExecuteFastReactions( rec );
	}
      }
      /* Repeat as long as events are firing */
    } while (eventFired);
    /* Return the time for the next event firing or potential triggering */
    return firstEventTime;
}

static BOOL canTriggerEvent( ODE_SIMULATION_RECORD *rec, double time ) {
    int i;
    double nextEventTime;
    BOOL triggerEnabled;

    rec->time = time;
    for (i = 0; i < rec->eventsSize; i++) {
      nextEventTime = GetNextEventTimeInEvent( rec->eventArray[i] );
      triggerEnabled = GetTriggerEnabledInEvent( rec->eventArray[i] );
      if (nextEventTime != -1.0) {
	if  (time >= nextEventTime) {
	  /*
	  if (!GetUseValuesFromTriggerTime( rec->eventArray[i] )) {
	    SetEventAssignmentsNextValues( rec->eventArray[i], rec ); 
	  }
	  */
	  return TRUE;
	}
      }
      if (!triggerEnabled) {
	if (rec->evaluator->EvaluateWithCurrentConcentrationsDeter( rec->evaluator,
								    (KINETIC_LAW*)GetTriggerInEvent( rec->eventArray[i] ) )) {
	  return TRUE;
	}
      }
    }
    return FALSE;
}

static void SetEventAssignmentsNextValues( EVENT *event, ODE_SIMULATION_RECORD *rec ) {
  LINKED_LIST *list = NULL;
  EVENT_ASSIGNMENT *eventAssignment;
  double concentration = 0.0;
  UINT j;
  BYTE varType;

  list = GetEventAssignments( event );
  ResetCurrentElement( list );
  while( ( eventAssignment = (EVENT_ASSIGNMENT*)GetNextFromLinkedList( list ) ) != NULL ) {
    concentration = rec->evaluator->EvaluateWithCurrentConcentrations( rec->evaluator, eventAssignment->assignment );
    SetEventAssignmentNextValueTime( eventAssignment, concentration, rec->time );
  }
}

static void SetEventAssignmentsNextValuesTime( EVENT *event, ODE_SIMULATION_RECORD *rec, double time ) {
  LINKED_LIST *list = NULL;
  EVENT_ASSIGNMENT *eventAssignment;
  double concentration = 0.0;
  UINT j;
  BYTE varType;

  list = GetEventAssignments( event );
  ResetCurrentElement( list );
  //printf("Event = %s\n",GetCharArrayOfString( GetEventId(event)));
  while( ( eventAssignment = (EVENT_ASSIGNMENT*)GetNextFromLinkedList( list ) ) != NULL ) {
    concentration = rec->evaluator->EvaluateWithCurrentConcentrations( rec->evaluator, eventAssignment->assignment );
    //printf("event = %d time = %g concentration = %g\n",GetEventAssignmentIndex(eventAssignment),time,concentration);
    SetEventAssignmentNextValueTime( eventAssignment, concentration, time );
  }
}

static void fireEvent( EVENT *event, ODE_SIMULATION_RECORD *rec ) {
  LINKED_LIST *list = NULL;
  EVENT_ASSIGNMENT *eventAssignment;
  double concentration = 0.0;
  UINT j;
  BYTE varType;

  rec->timeStep = rec->originalTimeStep;
  list = GetEventAssignments( event );
  //printf("Firing event %s\n",GetCharArrayOfString( GetEventId( event ) ));
  ResetCurrentElement( list );
  while( ( eventAssignment = (EVENT_ASSIGNMENT*)GetNextFromLinkedList( list ) ) != NULL ) {
    varType = GetEventAssignmentVarType( eventAssignment );
    j = GetEventAssignmentIndex( eventAssignment );
    concentration = GetEventAssignmentNextValueTime( eventAssignment, rec->time );
    //printf("Firing event assignment to %s at time %g varType = %d j = %d conc = %g\n",
    //	   GetCharArrayOfString(eventAssignment->var),rec->time,varType,j,concentration);
    if ( varType == SPECIES_EVENT_ASSIGNMENT ) {
	if (HasOnlySubstanceUnitsInSpeciesNode( rec->speciesArray[j] )) {
	  SetAmountInSpeciesNode( rec->speciesArray[j], concentration );
	  rec->concentrations[j] = concentration;
	} else {
	  SetConcentrationInSpeciesNode( rec->speciesArray[j], concentration );
	  rec->concentrations[j] = GetAmountInSpeciesNode(rec->speciesArray[j]);
	}
    } else if ( varType == COMPARTMENT_EVENT_ASSIGNMENT ) {
      SetCurrentSizeInCompartment( rec->compartmentArray[j], concentration );
      rec->concentrations[rec->speciesSize + j] = concentration;
    } else {
      SetCurrentRealValueInSymbol( rec->symbolArray[j], concentration );
      rec->concentrations[rec->speciesSize + rec->compartmentsSize + j] = concentration;
    }
  }
}

/* Update values using assignments rules */
static void ExecuteAssignments( ODE_SIMULATION_RECORD *rec ) {
  UINT32 i = 0;
  UINT32 j = 0;
  double concentration = 0.0;
  BYTE varType;

  for (i = 0; i < rec->rulesSize; i++) {
    if ( GetRuleType( rec->ruleArray[i] ) == RULE_TYPE_ASSIGNMENT ) {
      concentration = rec->evaluator->EvaluateWithCurrentConcentrationsDeter( rec->evaluator,
									 (KINETIC_LAW*)GetMathInRule( rec->ruleArray[i] ) );
      varType = GetRuleVarType( rec->ruleArray[i] );
      //printf("Conc=%g\n",concentration);
      j = GetRuleIndex( rec->ruleArray[i] );
      if ( varType == SPECIES_RULE ) {
	if (HasOnlySubstanceUnitsInSpeciesNode( rec->speciesArray[j] )) {
	  SetAmountInSpeciesNode( rec->speciesArray[j], concentration );
	  rec->concentrations[j] = concentration;
	} else {
	  SetConcentrationInSpeciesNode( rec->speciesArray[j], concentration );
	  rec->concentrations[j] = GetAmountInSpeciesNode(rec->speciesArray[j]);
	}
      } else if ( varType == COMPARTMENT_RULE ) {
	SetCurrentSizeInCompartment( rec->compartmentArray[j], concentration );
	rec->concentrations[rec->speciesSize + j] = concentration;
      } else {
	SetCurrentRealValueInSymbol( rec->symbolArray[j], concentration );
	rec->concentrations[rec->speciesSize + rec->compartmentsSize + j] = concentration;
      }
    }
  }
}

int ODE_print_state (size_t iter,gsl_multiroot_fsolver * s,int n) {
  int i = 0;

  printf("i= %d x = [ ",iter);
  for (i = 0; i < n; i++) {
    printf(" %g ",gsl_vector_get (s->x, i));
  }
  printf("] f = [ ");
  for (i = 0; i < n; i++) {
    printf(" %g ",gsl_vector_get (s->f, i));
  }
  printf("]\n");
}

int ODEalgebraicRules(const gsl_vector * x, void *params, gsl_vector * f) {
  UINT32 i = 0;
  UINT32 j;
  double amount = 0.0;
  ODE_SIMULATION_RECORD *rec = ((ODE_SIMULATION_RECORD*)params);
  SPECIES *species = NULL;
  COMPARTMENT *compartment = NULL;
  REB2SAC_SYMBOL *symbol = NULL;
  
  j = 0;
  for( i = 0; i < rec->speciesSize; i++ ) {
    species = rec->speciesArray[i];
    if (IsSpeciesNodeAlgebraic( species )) {
      amount = gsl_vector_get (x, j);
      j++; 
      if (HasOnlySubstanceUnitsInSpeciesNode( species )) {
	SetAmountInSpeciesNode( species, amount );
      } else {
	SetConcentrationInSpeciesNode( species, amount );
      }
    }
  }
  for( i = 0; i < rec->compartmentsSize; i++ ) {
    compartment = rec->compartmentArray[i];
    if (IsCompartmentAlgebraic( compartment )) {
      amount = gsl_vector_get (x, j);
      j++;
      SetCurrentSizeInCompartment( compartment, amount );
    }
  }
  for( i = 0; i < rec->symbolsSize; i++ ) {
    symbol = rec->symbolArray[i];
    if (IsSymbolAlgebraic( symbol )) {
      amount = gsl_vector_get (x, j);
      j++;
      SetCurrentRealValueInSymbol( symbol, amount );
    }
  }
  j = 0;
  for (i = 0; i < rec->rulesSize; i++) {
    if ( GetRuleType( rec->ruleArray[i] ) == RULE_TYPE_ALGEBRAIC ) {
      amount = rec->evaluator->EvaluateWithCurrentConcentrationsDeter( rec->evaluator,
							   (KINETIC_LAW*)GetMathInRule( rec->ruleArray[i] ) );
      gsl_vector_set (f, j, amount);
      j++;
    } 
  }
     
  return GSL_SUCCESS;
}

static RET_VAL EvaluateAlgebraicRules( ODE_SIMULATION_RECORD *rec ) {
  const gsl_multiroot_fsolver_type *T;
  gsl_multiroot_fsolver *s;
  SPECIES *species = NULL;
  COMPARTMENT *compartment = NULL;
  REB2SAC_SYMBOL *symbol = NULL;
     
  int status;
  size_t i, j, iter = 0;
  double amount;

  const size_t n = rec->algebraicRulesSize;

  gsl_multiroot_function f = {&ODEalgebraicRules, n, rec};

  gsl_vector *x = gsl_vector_alloc (n);
  
  j = 0;
  for( i = 0; i < rec->speciesSize; i++ ) {
    species = rec->speciesArray[i];
    if (IsSpeciesNodeAlgebraic( species )) {
      if (HasOnlySubstanceUnitsInSpeciesNode( species )) {
	amount = GetAmountInSpeciesNode( species );
      } else {
	amount = GetConcentrationInSpeciesNode( species );
      }
      gsl_vector_set (x, j, amount);
      j++;
    } 
  }
  for( i = 0; i < rec->compartmentsSize; i++ ) {
    compartment = rec->compartmentArray[i];
    if (IsCompartmentAlgebraic( compartment )) {
      amount = GetCurrentSizeInCompartment( compartment );
      gsl_vector_set (x, j, amount);
      j++;
    }
  }
  for( i = 0; i < rec->symbolsSize; i++ ) {
    symbol = rec->symbolArray[i];
    if (IsSymbolAlgebraic( symbol )) {
      amount = GetCurrentRealValueInSymbol( symbol );
      gsl_vector_set (x, j, amount);
      j++;
    }
  }
     
  T = gsl_multiroot_fsolver_hybrids;
  s = gsl_multiroot_fsolver_alloc (T, n);
  gsl_multiroot_fsolver_set (s, &f, x);
     
  // ODE_print_state (iter, s, n);
     
  do {
      iter++;
      status = gsl_multiroot_fsolver_iterate (s);
      
      // ODE_print_state (iter, s, n);
      
      if (status)   /* check if solver is stuck */
	break;
      
      status = gsl_multiroot_test_residual (s->f, 1e-7);
  } while (status == GSL_CONTINUE && iter < 1000);
     
  // printf ("status = %s\n", gsl_strerror (status));
     
  gsl_multiroot_fsolver_free (s);
  gsl_vector_free (x);
  return 0;
}

int ODEfastReactions(const gsl_vector * x, void *params, gsl_vector * f) {
  RET_VAL ret = SUCCESS;
  UINT32 i = 0;
  UINT32 j;
  double amount = 0.0;
  ODE_SIMULATION_RECORD *rec = ((ODE_SIMULATION_RECORD*)params);
  SPECIES *species = NULL;
  REACTION *reaction = NULL;
  LINKED_LIST *Redges = NULL;
  LINKED_LIST *Pedges = NULL;
  IR_EDGE *edge = NULL;
  double stoichiometry = 0.0;
  BOOL boundary = FALSE;
  REB2SAC_SYMBOL *speciesRef = NULL;
  REB2SAC_SYMBOL *convFactor = NULL;

  j = 0;
  for( i = 0; i < rec->speciesSize; i++ ) {
    species = rec->speciesArray[i];
    if (IsSpeciesNodeFast( species )) {
      amount = gsl_vector_get (x, j);
      j++; 
      if (HasOnlySubstanceUnitsInSpeciesNode( species )) {
	SetAmountInSpeciesNode( species, amount );
	rec->concentrations[i] = amount;
      } else {
	SetConcentrationInSpeciesNode( species, amount );
	rec->concentrations[i] = amount;
      }
    }
  }
  if( IS_FAILED( ( ret = _CalculateReactionRates( rec ) ) ) ) {
    return GSL_FAILURE;
  }
  j = 0;
  for( i = 0; i < rec->reactionsSize; i++ ) {
    reaction = rec->reactionArray[i];
    if (IsReactionFastInReactionNode( reaction )) {
      amount = 0.0;
      Redges = GetReactantsInReactionNode( reaction );
      Pedges = GetProductsInReactionNode( reaction );
      ResetCurrentElement( Redges );
      if ( ( edge = GetNextEdge( Redges ) ) != NULL ) {
	speciesRef = GetSpeciesRefInIREdge( edge );
	if (speciesRef) {
	  stoichiometry = GetCurrentRealValueInSymbol( speciesRef );
	} else {
	  stoichiometry = GetStoichiometryInIREdge( edge );
	}
	species = GetSpeciesInIREdge( edge );
	if (( convFactor = GetConversionFactorInSpeciesNode( species ) )!=NULL) {
	  stoichiometry *= GetCurrentRealValueInSymbol( convFactor );
	}
	if (HasOnlySubstanceUnitsInSpeciesNode( species )) {
	  if (HasBoundaryConditionInSpeciesNode( species )) {
	    amount = GetAmountInSpeciesNode( species );
	    boundary = TRUE;
	  } else {
	    amount = GetAmountInSpeciesNode( species ) / stoichiometry;
	    boundary = FALSE;
	  }
	} else {
	  if (HasBoundaryConditionInSpeciesNode( species )) {
	    amount = GetConcentrationInSpeciesNode( species );
	    boundary = TRUE;
	  } else {
	    amount = GetConcentrationInSpeciesNode( species ) / stoichiometry;
	    boundary = FALSE;
	  }
	}
	ResetCurrentElement( Pedges );
	while( ( edge = GetNextEdge( Pedges ) ) != NULL ) {
	  speciesRef = GetSpeciesRefInIREdge( edge );
	  if (speciesRef) {
	    stoichiometry = GetCurrentRealValueInSymbol( speciesRef );
	  } else {
	    stoichiometry = GetStoichiometryInIREdge( edge );
	  }
	  species = GetSpeciesInIREdge( edge );
	  if (( convFactor = GetConversionFactorInSpeciesNode( species ) )!=NULL) {
	    stoichiometry *= GetCurrentRealValueInSymbol( convFactor );
	  }
	  if (HasOnlySubstanceUnitsInSpeciesNode( species )) {
	    if (boundary) {
	      if (HasBoundaryConditionInSpeciesNode( species )) {
		gsl_vector_set (f, j, amount + GetAmountInSpeciesNode( species ) - rec->fastCons[j]);
	      } else {
		gsl_vector_set (f, j, amount - rec->fastCons[j]);
	      }
	    } else {
	      if (HasBoundaryConditionInSpeciesNode( species )) {
		gsl_vector_set (f, j, GetAmountInSpeciesNode( species ) - rec->fastCons[j]);
	      } else {
		gsl_vector_set (f, j, amount + GetAmountInSpeciesNode( species ) / stoichiometry - rec->fastCons[j] );
	      }
	    }
	    j++;
	  } else {
	    if (boundary) {
	      if (HasBoundaryConditionInSpeciesNode( species )) {
		gsl_vector_set (f, j, amount + GetConcentrationInSpeciesNode( species ) - rec->fastCons[j]);
	      } else {
		gsl_vector_set (f, j, amount - rec->fastCons[j]);
	      }
	    } else {
	      if (HasBoundaryConditionInSpeciesNode( species )) {
		gsl_vector_set (f, j, GetConcentrationInSpeciesNode( species ) - rec->fastCons[j]);
	      } else {
		gsl_vector_set (f, j, amount + GetConcentrationInSpeciesNode( species ) / stoichiometry - rec->fastCons[j]);
	      }
	    }
	    j++;
	  }
	}
      }
      ResetCurrentElement( Pedges );
      if ( ( edge = GetNextEdge( Pedges ) ) != NULL ) {
	speciesRef = GetSpeciesRefInIREdge( edge );
	if (speciesRef) {
	  stoichiometry = GetCurrentRealValueInSymbol( speciesRef );
	} else {
	  stoichiometry = GetStoichiometryInIREdge( edge );
	}
	species = GetSpeciesInIREdge( edge );
	if (( convFactor = GetConversionFactorInSpeciesNode( species ) )!=NULL) {
	  stoichiometry *= GetCurrentRealValueInSymbol( convFactor );
	}
	if (HasOnlySubstanceUnitsInSpeciesNode( species )) {
	  if (HasBoundaryConditionInSpeciesNode( species )) {
	    amount = GetAmountInSpeciesNode( species );
	    boundary = TRUE;
	  } else {
	    amount = GetAmountInSpeciesNode( species ) / stoichiometry;
	    boundary = FALSE;
	  }
	} else {
	  if (HasBoundaryConditionInSpeciesNode( species )) {
	    amount = GetConcentrationInSpeciesNode( species );
	    boundary = TRUE;
	  } else {
	    amount = GetConcentrationInSpeciesNode( species ) / stoichiometry;
	    boundary = FALSE;
	  }
	}
	while( ( edge = GetNextEdge( Redges ) ) != NULL ) {
	  speciesRef = GetSpeciesRefInIREdge( edge );
	  if (speciesRef) {
	    stoichiometry = GetCurrentRealValueInSymbol( speciesRef );
	  } else {
	    stoichiometry = GetStoichiometryInIREdge( edge );
	  }
	  species = GetSpeciesInIREdge( edge );
	  if (( convFactor = GetConversionFactorInSpeciesNode( species ) )!=NULL) {
	    stoichiometry *= GetCurrentRealValueInSymbol( convFactor );
	  }
	  if (HasOnlySubstanceUnitsInSpeciesNode( species )) {
	    if (boundary) {
	      if (HasBoundaryConditionInSpeciesNode( species )) {
		gsl_vector_set (f, j, amount + GetAmountInSpeciesNode( species ) - rec->fastCons[j]);
	      } else {
		gsl_vector_set (f, j, amount - rec->fastCons[j]);
	      }
	    } else {
	      if (HasBoundaryConditionInSpeciesNode( species )) {
		gsl_vector_set (f, j, GetAmountInSpeciesNode( species ) - rec->fastCons[j]);
	      } else {
		gsl_vector_set (f, j, amount + GetAmountInSpeciesNode( species ) / stoichiometry - rec->fastCons[j]);
	      }
	    }
	    j++;
	  } else {
	    if (boundary) {
	      if (HasBoundaryConditionInSpeciesNode( species )) {
		gsl_vector_set (f, j, amount + GetConcentrationInSpeciesNode( species ) - rec->fastCons[j]);
	      } else {
		gsl_vector_set (f, j, amount - rec->fastCons[j]);
	      }
	    } else {
	      if (HasBoundaryConditionInSpeciesNode( species )) {
		gsl_vector_set (f, j, GetConcentrationInSpeciesNode( species ) - rec->fastCons[j]);
	      } else {
		gsl_vector_set (f, j, amount + GetConcentrationInSpeciesNode( species ) / stoichiometry - rec->fastCons[j]);
	      }
	    }
	    j++;
	  }
	}
      }
      //break;
    }
  } 
  for( i = 0; i < rec->reactionsSize; i++ ) {
    reaction = rec->reactionArray[i];
    if (IsReactionFastInReactionNode( reaction )) {
      amount = GetReactionRate( reaction );
      gsl_vector_set (f, j, amount);
      j++;
    }
  }
   
  return GSL_SUCCESS;
}

static RET_VAL ExecuteFastReactions( ODE_SIMULATION_RECORD *rec ) {
  const gsl_multiroot_fsolver_type *T;
  gsl_multiroot_fsolver *s;
  SPECIES *species = NULL;
  REACTION *reaction = NULL;
  LINKED_LIST *Redges = NULL;
  LINKED_LIST *Pedges = NULL;
  IR_EDGE *edge = NULL;
  double stoichiometry = 0.0;
  int status;
  size_t i, j, iter = 0;
  double amount;
  const size_t n = rec->numberFastSpecies;
  BOOL boundary = FALSE;
  REB2SAC_SYMBOL *speciesRef = NULL;
  REB2SAC_SYMBOL *convFactor = NULL;

  j=0;
  for( i = 0; i < rec->reactionsSize; i++ ) {
    reaction = rec->reactionArray[i];
    if (IsReactionFastInReactionNode( reaction )) {
      amount = 0.0;
      Redges = GetReactantsInReactionNode( reaction );
      Pedges = GetProductsInReactionNode( reaction );
      ResetCurrentElement( Redges );
      if ( ( edge = GetNextEdge( Redges ) ) != NULL ) {
	speciesRef = GetSpeciesRefInIREdge( edge );
	if (speciesRef) {
	  stoichiometry = GetCurrentRealValueInSymbol( speciesRef );
	} else {
	  stoichiometry = GetStoichiometryInIREdge( edge );
	}
	species = GetSpeciesInIREdge( edge );
	if (( convFactor = GetConversionFactorInSpeciesNode( species ) )!=NULL) {
	  stoichiometry *= GetCurrentRealValueInSymbol( convFactor );
	}
	//printf("1:%s/%g\n",GetCharArrayOfString(GetSpeciesNodeID(species)),stoichiometry);
	if (HasOnlySubstanceUnitsInSpeciesNode( species )) {
	  if (HasBoundaryConditionInSpeciesNode( species )) {
	    amount = GetAmountInSpeciesNode( species );
	    boundary = TRUE;
	  } else {
	    amount = GetAmountInSpeciesNode( species ) / stoichiometry;
	    boundary = FALSE;
	  }
	} else {
	  if (HasBoundaryConditionInSpeciesNode( species )) {
	    amount = GetConcentrationInSpeciesNode( species );
	    boundary = TRUE;
	  } else {
	    amount = GetConcentrationInSpeciesNode( species ) / stoichiometry;
	    boundary = FALSE;
	  }
	}
	ResetCurrentElement( Pedges );
	while( ( edge = GetNextEdge( Pedges ) ) != NULL ) {
	  speciesRef = GetSpeciesRefInIREdge( edge );
	  if (speciesRef) {
	    stoichiometry = GetCurrentRealValueInSymbol( speciesRef );
	  } else {
	    stoichiometry = GetStoichiometryInIREdge( edge );
	  }
	  species = GetSpeciesInIREdge( edge );
	  if (( convFactor = GetConversionFactorInSpeciesNode( species ) )!=NULL) {
	    stoichiometry *= GetCurrentRealValueInSymbol( convFactor );
	  }
	  if (HasOnlySubstanceUnitsInSpeciesNode( species )) {
	    if (boundary) {
	      rec->fastCons[j] = amount;
	      if (HasBoundaryConditionInSpeciesNode( species )) {
		rec->fastCons[j] += GetAmountInSpeciesNode( species );
	      }
	    } else {
	      if (HasBoundaryConditionInSpeciesNode( species )) {
		rec->fastCons[j] = GetAmountInSpeciesNode( species );
	      } else {
		rec->fastCons[j] = amount + GetAmountInSpeciesNode( species ) / stoichiometry;
	      }
	    }
	    //printf(" + %s/%g = %g",GetCharArrayOfString(GetSpeciesNodeID(species)),stoichiometry,rec->fastCons[j]);
	    j++;
	  } else {
	    if (boundary) {
	      rec->fastCons[j] = amount;
	      if (HasBoundaryConditionInSpeciesNode( species )) {
		rec->fastCons[j] += GetConcentrationInSpeciesNode( species );
	      }
	    } else {
	      if (HasBoundaryConditionInSpeciesNode( species )) {
		rec->fastCons[j] = GetConcentrationInSpeciesNode( species );
	      } else {
		rec->fastCons[j] = amount + GetConcentrationInSpeciesNode( species ) / stoichiometry;
	      }
	    }
	    j++;
	  }
	}
      }
      ResetCurrentElement( Pedges );
      if ( ( edge = GetNextEdge( Pedges ) ) != NULL ) {
	speciesRef = GetSpeciesRefInIREdge( edge );
	if (speciesRef) {
	  stoichiometry = GetCurrentRealValueInSymbol( speciesRef );
	} else {
	  stoichiometry = GetStoichiometryInIREdge( edge );
	}
	species = GetSpeciesInIREdge( edge );
	if (( convFactor = GetConversionFactorInSpeciesNode( species ) )!=NULL) {
	  stoichiometry *= GetCurrentRealValueInSymbol( convFactor );
	}
	//printf("2:%s/%g\n",GetCharArrayOfString(GetSpeciesNodeID(species)),stoichiometry);
	if (HasOnlySubstanceUnitsInSpeciesNode( species )) {
	  if (HasBoundaryConditionInSpeciesNode( species )) {
	    amount = GetAmountInSpeciesNode( species );
	    boundary = TRUE;
	  } else {
	    amount = GetAmountInSpeciesNode( species ) / stoichiometry;
	    boundary = FALSE;
	  }
	} else {
	  if (HasBoundaryConditionInSpeciesNode( species )) {
	    amount = GetConcentrationInSpeciesNode( species );
	    boundary = TRUE;
	  } else {
	    amount = GetConcentrationInSpeciesNode( species ) / stoichiometry;
	    boundary = FALSE;
	  }
	}
	while( ( edge = GetNextEdge( Redges ) ) != NULL ) {
	  speciesRef = GetSpeciesRefInIREdge( edge );
	  if (speciesRef) {
	    stoichiometry = GetCurrentRealValueInSymbol( speciesRef );
	  } else {
	    stoichiometry = GetStoichiometryInIREdge( edge );
	  }
	  species = GetSpeciesInIREdge( edge );
	  if (( convFactor = GetConversionFactorInSpeciesNode( species ) )!=NULL) {
	    stoichiometry *= GetCurrentRealValueInSymbol( convFactor );
	  }
	  if (HasOnlySubstanceUnitsInSpeciesNode( species )) {
	    if (boundary) {
	      rec->fastCons[j] = amount;
	      if (HasBoundaryConditionInSpeciesNode( species )) {
		rec->fastCons[j] += GetAmountInSpeciesNode( species );
	      }
	    } else {
	      if (HasBoundaryConditionInSpeciesNode( species )) {
		rec->fastCons[j] = GetAmountInSpeciesNode( species );
	      } else {
		rec->fastCons[j] = amount + GetAmountInSpeciesNode( species ) / stoichiometry;
	      }
	    }
	    j++;
	  } else {
	    if (boundary) {
	      rec->fastCons[j] = amount;
	      if (HasBoundaryConditionInSpeciesNode( species )) {
		rec->fastCons[j] += GetConcentrationInSpeciesNode( species );
	      }
	    } else {
	      if (HasBoundaryConditionInSpeciesNode( species )) {
		rec->fastCons[j] = GetConcentrationInSpeciesNode( species );
	      } else {
		rec->fastCons[j] = amount + GetConcentrationInSpeciesNode( species ) / stoichiometry;
	      }
	    }
	    //printf(" + %s/%g = %g",GetCharArrayOfString(GetSpeciesNodeID(species)),stoichiometry,rec->fastCons[j]);
	    j++;
	  }
	}
      }
      //break;
    }
  } 

  gsl_multiroot_function f = {&ODEfastReactions, n, rec};

  gsl_vector *x = gsl_vector_alloc (n);
  
  j = 0;
  for( i = 0; i < rec->speciesSize; i++ ) {
    species = rec->speciesArray[i];
    //if (!HasBoundaryConditionInSpeciesNode( species ) && IsSpeciesNodeFast( species )) {
    if (IsSpeciesNodeFast( species )) {
      if (HasOnlySubstanceUnitsInSpeciesNode( species )) {
	amount = GetAmountInSpeciesNode( species );
      } else {
	amount = GetConcentrationInSpeciesNode( species );
      }
      gsl_vector_set (x, j, amount);
      j++;
    } 
  }
     
  T = gsl_multiroot_fsolver_hybrids;
  s = gsl_multiroot_fsolver_alloc (T, n);
  gsl_multiroot_fsolver_set (s, &f, x);

  //printf("time = %g\n",rec->time);
  //ODE_print_state (iter, s, n);
     
  do {
      iter++;
      status = gsl_multiroot_fsolver_iterate (s);
      
      //ODE_print_state (iter, s, n);
      
      if (status)   /* check if solver is stuck */
	break;
      
      status = gsl_multiroot_test_residual (s->f, 1e-7);
  } while (status == GSL_CONTINUE && iter < 1000);
     
  //printf ("status = %s\n", gsl_strerror (status));

  j = 0;
  for( i = 0; i < rec->speciesSize; i++ ) {
    species = rec->speciesArray[i];
    //if (!HasBoundaryConditionInSpeciesNode( species ) && IsSpeciesNodeFast( species )) {
    if (IsSpeciesNodeFast( species )) {
      amount = gsl_vector_get (s->x, j);
      j++; 
      if (HasOnlySubstanceUnitsInSpeciesNode( species )) {
	SetAmountInSpeciesNode( species, amount );
	rec->concentrations[i] = amount;
      } else {
	SetConcentrationInSpeciesNode( species, amount );
	rec->concentrations[i] = amount;
      }
    }
  }
     
  gsl_multiroot_fsolver_free (s);
  gsl_vector_free (x);
  return 0;
}

static int _Update( double t, const double y[], double f[], ODE_SIMULATION_RECORD *rec ) {
    RET_VAL ret = SUCCESS;
    UINT32 i = 0;
    UINT32 j = 0;
    UINT32 speciesSize = rec->speciesSize;
    UINT32 compartmentsSize = rec->compartmentsSize;
    double stoichiometry = 0.0;
    double concentration = 0.0;
    double size = 0.0;
    double deltaTime = 0.0;
    double rate = 0.0;
    IR_EDGE *edge = NULL;
    REACTION *reaction = NULL;
    COMPARTMENT *compartment = NULL;
    COMPARTMENT **compartmentsArray = rec->compartmentArray;
    UINT32 symbolsSize = rec->symbolsSize;
    REB2SAC_SYMBOL *symbol = NULL;
    REB2SAC_SYMBOL **symbolArray = rec->symbolArray;
    SPECIES *species = NULL;
    SPECIES **speciesArray = rec->speciesArray;
    LINKED_LIST *edges = NULL;
    KINETIC_LAW_EVALUATER *evaluator = rec->evaluator;
    double nextEventTime;
    BOOL triggerEnabled;
    BYTE varType;
    REB2SAC_SYMBOL *speciesRef = NULL;
    REB2SAC_SYMBOL *convFactor = NULL;

    /* Update values from y[] */
    for( i = 0; i < speciesSize; i++ ) {
        species = speciesArray[i];
	if (f[i] != 0.0) {
	  if( IS_FAILED( ( ret = SetAmountInSpeciesNode( species, y[i] ) ) ) ) {
	    return GSL_FAILURE;
	  }
	}
	if( IS_FAILED( ( ret = SetRateInSpeciesNode( species, f[i] ) ) ) ) {
	  return GSL_FAILURE;
	}
	f[i] = 0.0;
    }
    for( i = 0; i < compartmentsSize; i++ ) {
        compartment = compartmentsArray[i];
	if (f[speciesSize + i] != 0.0) {
	  if( IS_FAILED( ( ret = SetCurrentSizeInCompartment( compartment, y[speciesSize + i] ) ) ) ) {
            return GSL_FAILURE;
	  }
	}
	if( IS_FAILED( ( ret = SetCurrentRateInCompartment( compartment, f[speciesSize + i] ) ) ) ) {
	  return GSL_FAILURE;
	}
	f[speciesSize + i] = 0.0;
    }
    for( i = 0; i < symbolsSize; i++ ) {
        symbol = symbolArray[i];
	if (f[speciesSize + compartmentsSize + i] != 0.0) {
	  if( IS_FAILED( ( ret = SetCurrentRealValueInSymbol( symbol, y[speciesSize + compartmentsSize + i] ) ) ) ) {
            return GSL_FAILURE;
	  }
	}
	if( IS_FAILED( ( ret = SetCurrentRateInSymbol( symbol, f[speciesSize + compartmentsSize + i] ) ) ) ) {
	  return GSL_FAILURE;
	}
	f[speciesSize + compartmentsSize + i] = 0.0;
	if ((strcmp(GetCharArrayOfString( GetSymbolID(symbol) ),"time")==0) ||
	    (strcmp(GetCharArrayOfString( GetSymbolID(symbol) ),"t")==0)) {
	  f[speciesSize + compartmentsSize + i] = 1.0;
	}
    }

    ExecuteAssignments( rec );
    if (rec->algebraicRulesSize > 0) {
      EvaluateAlgebraicRules( rec );
    }
    if ((rec->timeStep != 0.00001) && (canTriggerEvent( rec, t ))) {
      rec->timeStep = 0.00001;
      return GSL_ETOL;
    }

    /* Update rates using rate rules */
    for (i = 0; i < rec->rulesSize; i++) {
      if (GetRuleType( rec->ruleArray[i] ) == RULE_TYPE_RATE_ASSIGNMENT ) {
	rate = rec->evaluator->EvaluateWithCurrentConcentrationsDeter( rec->evaluator,
								  (KINETIC_LAW*)GetMathInRule( rec->ruleArray[i] ) );
	varType = GetRuleVarType( rec->ruleArray[i] );
	j = GetRuleIndex( rec->ruleArray[i] );
	if ( varType == SPECIES_RULE ) {
	  if (HasOnlySubstanceUnitsInSpeciesNode( speciesArray[j] )) {
	    f[j] = rate;
	  } else {
	    size = GetCurrentSizeInCompartment( GetCompartmentInSpeciesNode( speciesArray[j] ) );
	    f[j] = rate * size;
	  }
	} else if ( varType == COMPARTMENT_RULE ) {
	  f[speciesSize + j] = rate;
	} else {
	  f[speciesSize + compartmentsSize + j] = rate;
	}
      }
    }

    /* Update rates using kinetic laws from the reactions */
    if( IS_FAILED( ( ret = _CalculateReactionRates( rec ) ) ) ) {
        return GSL_FAILURE;
    }
    for( i = 0; i < speciesSize; i++ ) {
        species = speciesArray[i];
	if (HasBoundaryConditionInSpeciesNode(species)) continue;
	/* size = GetCurrentSizeInCompartment( GetCompartmentInSpeciesNode( species ) ); */
	TRACE_2( "%s changes from %g", GetCharArrayOfString( GetSpeciesNodeName( species ) ),
            concentration );
        edges = GetReactantEdges( (IR_NODE*)species );
        ResetCurrentElement( edges );
        while( ( edge = GetNextEdge( edges ) ) != NULL ) {
	    speciesRef = GetSpeciesRefInIREdge( edge );
	    if (speciesRef) {
	      stoichiometry = GetCurrentRealValueInSymbol( speciesRef );
	    } else {
	      stoichiometry = GetStoichiometryInIREdge( edge );
	    }
            reaction = GetReactionInIREdge( edge );
	    if (IsReactionFastInReactionNode( reaction )) continue;
            rate = GetReactionRate( reaction );
	    f[i] -= (stoichiometry * rate);
            TRACE_2( "\tchanges from %s is %g", GetCharArrayOfString( GetReactionNodeName( reaction ) ),
               -(stoichiometry * rate));
        }
        edges = GetProductEdges( (IR_NODE*)species );
        ResetCurrentElement( edges );
        while( ( edge = GetNextEdge( edges ) ) != NULL ) {
	    speciesRef = GetSpeciesRefInIREdge( edge );
	    if (speciesRef) {
	      stoichiometry = GetCurrentRealValueInSymbol( speciesRef );
	    } else {
	      stoichiometry = GetStoichiometryInIREdge( edge );
	    }
            reaction = GetReactionInIREdge( edge );
	    if (IsReactionFastInReactionNode( reaction )) continue;
            rate = GetReactionRate( reaction );
	    f[i] += (stoichiometry * rate);
            TRACE_2( "\tchanges from %s is %g", GetCharArrayOfString( GetReactionNodeName( reaction ) ),
               (stoichiometry * rate));
        }
	if (( convFactor = GetConversionFactorInSpeciesNode( species ) )!=NULL) {
	  f[i] *= GetCurrentRealValueInSymbol( convFactor );
	}
        TRACE_2( "change of %s is %g\n", GetCharArrayOfString( GetSpeciesNodeName( species ) ), f[i] );
    }
    return GSL_SUCCESS;
}
