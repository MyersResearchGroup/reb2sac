#include <math.h>
#include <float.h>
#include "gsl/gsl_vector.h"
#include "gsl/gsl_multiroots.h"
#include "marginal_probability_density_evolution_monte_carlo.h"
#include "conservation_analysis.c"

static BOOL _IsModelConditionSatisfied(IR *ir);

static RET_VAL _InitializeRecord(MPDE_MONTE_CARLO_RECORD *rec, BACK_END_PROCESSOR *backend, IR *ir);
static RET_VAL _InitializeSimulation(MPDE_MONTE_CARLO_RECORD *rec, int runNum);
static RET_VAL _RunSimulation(MPDE_MONTE_CARLO_RECORD *rec, BACK_END_PROCESSOR *backend);
static RET_VAL _CheckBifurcation(MPDE_MONTE_CARLO_RECORD *rec, double **mpRuns, double *mpTimes, int useMP, BIFURCATION_RECORD *birec, int previousNumberFirstCluster, FILE *file);

static RET_VAL _CleanSimulation(MPDE_MONTE_CARLO_RECORD *rec);
static RET_VAL _CleanRecord(MPDE_MONTE_CARLO_RECORD *rec);

static RET_VAL _CalculateTotalPropensities(MPDE_MONTE_CARLO_RECORD *rec);
static RET_VAL _CalculatePropensities(MPDE_MONTE_CARLO_RECORD *rec);
static RET_VAL _CalculatePropensity(MPDE_MONTE_CARLO_RECORD *rec, REACTION *reaction);
static RET_VAL _FindNextReactionTime(MPDE_MONTE_CARLO_RECORD *rec);
static RET_VAL _FindNextReaction(MPDE_MONTE_CARLO_RECORD *rec);
static RET_VAL _Update(MPDE_MONTE_CARLO_RECORD *rec);
static RET_VAL _Print(MPDE_MONTE_CARLO_RECORD *rec);
static RET_VAL _PrintBifurcationStatistics( double time, double numberFirstCluster, double numberSecondCluster, UINT32 runs, FILE *file);
static RET_VAL _PrintStatistics( MPDE_MONTE_CARLO_RECORD *rec, FILE *file);
static RET_VAL _UpdateNodeValues(MPDE_MONTE_CARLO_RECORD *rec);
static RET_VAL _UpdateSpeciesValues(MPDE_MONTE_CARLO_RECORD *rec);
static RET_VAL _UpdateReactionRateUpdateTime(MPDE_MONTE_CARLO_RECORD *rec);
static RET_VAL _UpdateReactionRateUpdateTimeForSpecies(MPDE_MONTE_CARLO_RECORD *rec, SPECIES* species);
static RET_VAL _UpdateAllReactionRateUpdateTimes(MPDE_MONTE_CARLO_RECORD *rec, double time);

static int _ComparePropensity(REACTION *a, REACTION *b);
static BOOL _IsTerminationConditionMet(MPDE_MONTE_CARLO_RECORD *rec);
static gsl_matrix* _GetStoichiometricMatrix(MPDE_MONTE_CARLO_RECORD *rec);

static double fireEvents(MPDE_MONTE_CARLO_RECORD *rec, double time);
static void fireEvent(EVENT *event, MPDE_MONTE_CARLO_RECORD *rec);
static void ExecuteAssignments(MPDE_MONTE_CARLO_RECORD *rec);
static void SetEventAssignmentsNextValues(EVENT *event, MPDE_MONTE_CARLO_RECORD *rec);
static void SetEventAssignmentsNextValuesTime( EVENT *event, MPDE_MONTE_CARLO_RECORD *rec, double time );
static RET_VAL EvaluateAlgebraicRules( MPDE_MONTE_CARLO_RECORD *rec );
static RET_VAL ExecuteFastReactions( MPDE_MONTE_CARLO_RECORD *rec );

DLLSCOPE RET_VAL STDCALL DoMPDEMonteCarloAnalysis(BACK_END_PROCESSOR *backend, IR *ir) {
    RET_VAL ret = SUCCESS;
    UINT i = 0;
    UINT runs = 1;
    char *namePrefix = NULL;
    static MPDE_MONTE_CARLO_RECORD rec;
    UINT timeout = 0;

    START_FUNCTION("DoMPDEMonteCarloAnalysis");

    if (!_IsModelConditionSatisfied(ir)) {
        return ErrorReport(FAILING, "DoMPDEMonteCarloAnalysis",
                "Marginal Probability Density Evolution method cannot be applied to the model");
    }
    if (IS_FAILED((ret = _InitializeRecord(&rec, backend, ir)))) {
        return ErrorReport(ret, "DoMPDEMonteCarloAnalysis", "initialization of the record failed");
    }

    runs = rec.runs;
    rec.useMP = backend->useMP;
    rec.useBifur = backend->useBifur;
    //for( i = 1; i <= runs; i++ ) {
    SeedRandomNumberGenerators(rec.seed);
    rec.seed = GetNextUniformRandomNumber(0, RAND_MAX);
    timeout = 0;
    do {
        SeedRandomNumberGenerators(rec.seed);
        if (IS_FAILED((ret = _InitializeSimulation(&rec, 1)))) {
            return ErrorReport(ret, "DoMPDEMonteCarloAnalysis", "initialization of the %i-th simulation failed", i);
        }
        timeout++;
    } while ((ret == CHANGE) && (timeout <= (rec.speciesSize + rec.compartmentsSize + rec.symbolsSize)));
    if (timeout > (rec.speciesSize + rec.compartmentsSize + rec.symbolsSize + 1)) {
        return ErrorReport(ret, "DoMPDEMonteCarloAnalysis", "Cycle detected in initial and rule assignments");
    }
    if (IS_FAILED((ret = _RunSimulation(&rec, backend)))) {
        return ErrorReport(ret, "DoMPDEMonteCarloAnalysis", "%i-th simulation failed at time %f", i, rec.time);
    }
    if (IS_FAILED((ret = _CleanSimulation(&rec)))) {
        return ErrorReport(ret, "DoMPDEMonteCarloAnalysis", "cleaning of the %i-th simulation failed", i);
    }
    //}
    END_FUNCTION("DoMPDEMonteCarloAnalysis", SUCCESS);
    return ret;
}

DLLSCOPE RET_VAL STDCALL CloseMPDEMonteCarloAnalyzer(BACK_END_PROCESSOR *backend) {
    RET_VAL ret = SUCCESS;
    MPDE_MONTE_CARLO_RECORD *rec = (MPDE_MONTE_CARLO_RECORD *) (backend->_internal1);

    START_FUNCTION("CloseMPDEMonteCarloAnalyzer");

    if (IS_FAILED((ret = _CleanRecord(rec)))) {
        return ErrorReport(ret, "CloseMPDEMonteCarloAnalyzer", "cleaning of the record failed");
    }

    END_FUNCTION("CloseMPDEMonteCarloAnalyzer", SUCCESS);
    return ret;
}

static BOOL _IsModelConditionSatisfied(IR *ir) {
    REACTION *reaction = NULL;
    LINKED_LIST *reactions = NULL;

    reactions = ir->GetListOfReactionNodes(ir);
    while ((reaction = (REACTION*) GetNextFromLinkedList(reactions)) != NULL) {
        if (IsReactionReversibleInReactionNode(reaction)) {
            TRACE_0(
                    "the input model contains reversible reaction(s), and cannot be used for Marginal Probability Density Evolution method");
            return FALSE;
        }
    }
    return TRUE;
}

static RET_VAL _InitializeRecord(MPDE_MONTE_CARLO_RECORD *rec, BACK_END_PROCESSOR *backend, IR *ir) {
    RET_VAL ret = SUCCESS;
    UINT32 i = 0;
    UINT32 j = 0;
    UINT32 k = 0;
    UINT32 size = 0;
    UINT32 algebraicVars = 0;
    double printInterval = 0.0;
    double minPrintInterval = -1.0;
    UINT32 numberSteps = 0;
    char buf[512];
    char *valueString = NULL;
    SPECIES *species = NULL;
    SPECIES **speciesArray = NULL;
    REACTION *reaction = NULL;
    REACTION **reactions = NULL;
    COMPARTMENT *compartment = NULL;
    COMPARTMENT **compartmentArray = NULL;
    COMPARTMENT_MANAGER *compartmentManager;
    REB2SAC_SYMBOL *symbol = NULL;
    REB2SAC_SYMBOL **symbolArray = NULL;
    REB2SAC_SYMTAB *symTab;
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
    COMPILER_RECORD_T *compRec = backend->record;
    LINKED_LIST *list = NULL;
    REB2SAC_PROPERTIES *properties = NULL;
    double *oldSpeciesMeans = NULL;
    double *oldSpeciesVariances = NULL;
    double *newSpeciesMeans = NULL;
    double *newSpeciesVariances = NULL;
    double *speciesSD = NULL;

#if GET_SEED_FROM_COMMAND_LINE
    PROPERTIES *options = NULL;
#endif

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

    if ((ruleManager = ir->GetRuleManager(ir)) == NULL) {
        return ErrorReport(FAILING, "_InitializeRecord", "could not get the rule manager");
    }
    list = ruleManager->CreateListOfRules(ruleManager);
    rec->rulesSize = GetLinkedListSize(list);
    if (rec->rulesSize > 0) {
        if ((ruleArray = (RULE**) MALLOC(rec->rulesSize * sizeof(RULE*))) == NULL) {
            return ErrorReport(FAILING, "_InitializeRecord", "could not allocate memory for rules array");
        }
    }
    i = 0;
    ResetCurrentElement( list );
    while( ( rule = (RULE*)GetNextFromLinkedList( list ) ) != NULL ) {
        ruleArray[i] = rule;
        i++;
    }
    rec->ruleArray = ruleArray;

    if ((symTab = ir->GetGlobalSymtab(ir)) == NULL) {
        return ErrorReport(FAILING, "_InitializeRecord", "could not get the symbol table");
    }
    list = symTab->GenerateListOfSymbols(symTab);
    rec->symbolsSize = GetLinkedListSize(list);
    if (rec->symbolsSize > 0) {
        if ((symbolArray = (REB2SAC_SYMBOL**) MALLOC(rec->symbolsSize * sizeof(REB2SAC_SYMBOL*))) == NULL) {
            return ErrorReport(FAILING, "_InitializeRecord", "could not allocate memory for symbols array");
        }
    }
    i = 0;
    ResetCurrentElement(list);
    while ((symbol = (REB2SAC_SYMBOL*) GetNextFromLinkedList(list)) != NULL) {
        symbolArray[i] = symbol;
	if (IsSymbolAlgebraic( symbol )) {
	  algebraicVars++;
	}
        i++;
    }
    rec->symbolArray = symbolArray;

    if ((compartmentManager = ir->GetCompartmentManager(ir)) == NULL) {
        return ErrorReport(FAILING, "_InitializeRecord", "could not get the compartment manager");
    }
    list = compartmentManager->CreateListOfCompartments(compartmentManager);
    rec->compartmentsSize = GetLinkedListSize(list);
    if (rec->compartmentsSize > 0) {
        if ((compartmentArray = (COMPARTMENT**) MALLOC(rec->compartmentsSize * sizeof(RULE*))) == NULL) {
            return ErrorReport(FAILING, "_InitializeRecord", "could not allocate memory for compartment array");
        }
    }
    i = 0;
    ResetCurrentElement(list);
    while ((compartment = (COMPARTMENT*) GetNextFromLinkedList(list)) != NULL) {
        compartmentArray[i] = compartment;
	if (IsCompartmentAlgebraic( compartment )) {
	  algebraicVars++;
	}
        i++;
    }
    rec->compartmentArray = compartmentArray;

    list = ir->GetListOfSpeciesNodes(ir);
    rec->speciesSize = GetLinkedListSize(list);
    //    if (rec->speciesSize==0) {
    //    return ErrorReport( FAILING, "_InitializeRecord", "no species remaining in the model" );
    //}
    if (rec->speciesSize > 0) {
        if ((speciesArray = (SPECIES**) MALLOC(rec->speciesSize * sizeof(SPECIES*))) == NULL) {
            return ErrorReport(FAILING, "_InitializeRecord", "could not allocate memory for species array");
        }
        if ((oldSpeciesMeans = (double*) MALLOC(rec->speciesSize * sizeof(double))) == NULL) {
            return ErrorReport(FAILING, "_InitializeRecord", "could not allocate memory for old species means array");
        }
        if ((oldSpeciesVariances = (double*) MALLOC(rec->speciesSize * sizeof(double))) == NULL) {
            return ErrorReport(FAILING, "_InitializeRecord",
                    "could not allocate memory for old species variances array");
        }
        if ((newSpeciesMeans = (double*) MALLOC(rec->speciesSize * sizeof(double))) == NULL) {
            return ErrorReport(FAILING, "_InitializeRecord", "could not allocate memory for new species means array");
        }
        if ((newSpeciesVariances = (double*) MALLOC(rec->speciesSize * sizeof(double))) == NULL) {
            return ErrorReport(FAILING, "_InitializeRecord",
                    "could not allocate memory for new species variances array");
        }
        if ((speciesSD = (double*) MALLOC(rec->speciesSize * sizeof(double))) == NULL) {
            return ErrorReport(FAILING, "_InitializeRecord",
                    "could not allocate memory for species standard deviations array");
        }
    }

    properties = compRec->properties;

    i = 0;
    rec->numberFastSpecies = 0;
    ResetCurrentElement( list );
    while( ( species = (SPECIES*)GetNextFromLinkedList( list ) ) != NULL ) {
        speciesArray[i] = species;
        oldSpeciesMeans[i] = 0;
        oldSpeciesVariances[i] = 0;
        newSpeciesMeans[i] = 0;
        newSpeciesVariances[i] = 0;
        speciesSD[i] = 0;
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
    }
    rec->speciesArray = speciesArray;
    rec->oldSpeciesMeans = oldSpeciesMeans;
    rec->oldSpeciesVariances = oldSpeciesVariances;
    rec->newSpeciesMeans = newSpeciesMeans;
    rec->newSpeciesVariances = newSpeciesVariances;
    rec->speciesSD = speciesSD;
    if ( algebraicVars > rec->algebraicRulesSize ) {
      return ErrorReport( FAILING, "_InitializeRecord", "model underdetermined" );
    } else if ( algebraicVars < rec->algebraicRulesSize ) {
      return ErrorReport( FAILING, "_InitializeRecord", "model overdetermined" );
    } 

    for (i = 0; i < rec->rulesSize; i++) {
        if (GetRuleType(rec->ruleArray[i]) == RULE_TYPE_ASSIGNMENT || GetRuleType(rec->ruleArray[i]) == RULE_TYPE_RATE_ASSIGNMENT) {
            for (j = 0; j < rec->speciesSize; j++) {
                if (strcmp(GetCharArrayOfString(GetRuleVar(rec->ruleArray[i])), GetCharArrayOfString(GetSpeciesNodeID(
                        rec->speciesArray[j]))) == 0) {
                    SetRuleVarType(ruleArray[i], SPECIES_RULE);
                    SetRuleIndex(ruleArray[i], j);
                    break;
                }
            }
            for (j = 0; j < rec->compartmentsSize; j++) {
                if (strcmp(GetCharArrayOfString(GetRuleVar(rec->ruleArray[i])), GetCharArrayOfString(GetCompartmentID(
                        rec->compartmentArray[j]))) == 0) {
                    SetRuleVarType(ruleArray[i], COMPARTMENT_RULE);
                    SetRuleIndex(ruleArray[i], j);
                    break;
                }
            }
            for (j = 0; j < rec->symbolsSize; j++) {
                if (strcmp(GetCharArrayOfString(GetRuleVar(rec->ruleArray[i])), GetCharArrayOfString(GetSymbolID(
                        rec->symbolArray[j]))) == 0) {
                    SetRuleVarType(ruleArray[i], PARAMETER_RULE);
                    SetRuleIndex(ruleArray[i], j);
                    break;
                }
            }
        }
    }

    if ((rec->evaluator = CreateKineticLawEvaluater()) == NULL) {
        return ErrorReport(FAILING, "_InitializeRecord", "could not create evaluator");
    }

    if ((rec->findNextTime = CreateKineticLawFind_Next_Time()) == NULL) {
        return ErrorReport(FAILING, "_InitializeRecord", "could not create find next time");
    }

    if ((valueString = properties->GetProperty(properties, MONTE_CARLO_SIMULATION_START_INDEX)) == NULL) {
        rec->startIndex = DEFAULT_MONTE_CARLO_SIMULATION_START_INDEX;
    } else {
        if (IS_FAILED((ret = StrToUINT32((UINT32*) &(rec->startIndex), valueString)))) {
            rec->startIndex = DEFAULT_MONTE_CARLO_SIMULATION_START_INDEX;
        }
    }

    if ((valueString = properties->GetProperty(properties, MONTE_CARLO_SIMULATION_TIME_LIMIT)) == NULL) {
        rec->timeLimit = DEFAULT_MONTE_CARLO_SIMULATION_TIME_LIMIT_VALUE;
    } else {
        if (IS_FAILED((ret = StrToFloat(&(rec->timeLimit), valueString)))) {
            rec->timeLimit = DEFAULT_MONTE_CARLO_SIMULATION_TIME_LIMIT_VALUE;
        }
    }

    if ((valueString = properties->GetProperty(properties, MONTE_CARLO_SIMULATION_TIME_STEP)) == NULL) {
        rec->timeStep = DEFAULT_MONTE_CARLO_SIMULATION_TIME_STEP;
    } else {
        if (IS_FAILED((ret = StrToFloat(&(rec->timeStep), valueString)))) {
            rec->timeStep = DEFAULT_MONTE_CARLO_SIMULATION_TIME_STEP;
        }
    }

    rec->minPrintInterval = -1.0;
    if ((valueString = properties->GetProperty(properties, MONTE_CARLO_SIMULATION_PRINT_INTERVAL)) == NULL) {
        if ((valueString = properties->GetProperty(properties, MONTE_CARLO_SIMULATION_NUMBER_STEPS)) == NULL) {
            if ((valueString = properties->GetProperty(properties, MONTE_CARLO_SIMULATION_MINIMUM_PRINT_INTERVAL))
                    == NULL) {
                rec->numberSteps = DEFAULT_MONTE_CARLO_SIMULATION_NUMBER_STEPS_VALUE;
            } else {
                if (IS_FAILED((ret = StrToFloat(&(minPrintInterval), valueString)))) {
                    rec->numberSteps = DEFAULT_MONTE_CARLO_SIMULATION_NUMBER_STEPS_VALUE;
                } else {
                    rec->minPrintInterval = minPrintInterval;
                }
            }
        } else {
            if (IS_FAILED((ret = StrToUINT32(&(rec->numberSteps), valueString)))) {
                rec->numberSteps = DEFAULT_MONTE_CARLO_SIMULATION_NUMBER_STEPS_VALUE;
            }
        }
    } else {
        if (IS_FAILED((ret = StrToFloat(&(printInterval), valueString)))) {
            rec->numberSteps = DEFAULT_MONTE_CARLO_SIMULATION_NUMBER_STEPS_VALUE;
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
        if( IS_FAILED( ( ret = StrToUINT32( &(rec->seed), valueString ) ) ) ) {
            rec->seed = DEFAULT_MONTE_CARLO_SIMULATION_RANDOM_SEED_VALUE;
        }
        TRACE_1("seed from command line is %i", rec->seed );
    }
#else
    if ((valueString = properties->GetProperty(properties, MONTE_CARLO_SIMULATION_RANDOM_SEED)) == NULL) {
        rec->seed = DEFAULT_MONTE_CARLO_SIMULATION_RANDOM_SEED_VALUE;
    } else {
        if (IS_FAILED((ret = StrToUINT32(&(rec->seed), valueString)))) {
            rec->seed = DEFAULT_MONTE_CARLO_SIMULATION_RANDOM_SEED_VALUE;
        }
    }
#endif

    if ((valueString = properties->GetProperty(properties, MONTE_CARLO_SIMULATION_RUNS)) == NULL) {
        rec->runs = DEFAULT_MONTE_CARLO_SIMULATION_RUNS_VALUE;
    } else {
        if (IS_FAILED((ret = StrToUINT32(&(rec->runs), valueString)))) {
            rec->runs = DEFAULT_MONTE_CARLO_SIMULATION_RUNS_VALUE;
        }
    }

    if ((rec->outDir = properties->GetProperty(properties, MONTE_CARLO_SIMULATION_OUT_DIR)) == NULL) {
        rec->outDir = DEFAULT_MONTE_CARLO_SIMULATION_OUT_DIR_VALUE;
    }

    if ((rec->meanPrinter = CreateSimulationPrinter(backend, compartmentArray, rec->compartmentsSize, speciesArray,
            rec->speciesSize, symbolArray, rec->symbolsSize)) == NULL) {
        return ErrorReport(FAILING, "_InitializeRecord", "could not create simulation mean printer");
    }
    if ((rec->varPrinter = CreateSimulationPrinter(backend, compartmentArray, rec->compartmentsSize, speciesArray,
            rec->speciesSize, symbolArray, rec->symbolsSize)) == NULL) {
        return ErrorReport(FAILING, "_InitializeRecord", "could not create simulation var printer");
    }
    if ((rec->sdPrinter = CreateSimulationPrinter(backend, compartmentArray, rec->compartmentsSize, speciesArray,
            rec->speciesSize, symbolArray, rec->symbolsSize)) == NULL) {
        return ErrorReport(FAILING, "_InitializeRecord", "could not create simulation sd printer");
    }
    if (backend->useMP != 0) {
        if ((rec->mpPrinter1 = CreateSimulationPrinter(backend, compartmentArray, rec->compartmentsSize, speciesArray,
                rec->speciesSize, symbolArray, rec->symbolsSize)) == NULL) {
            return ErrorReport(FAILING, "_InitializeRecord", "could not create simulation mp printer");
        }
        if (backend->useBifur) {
        	if ((rec->mpPrinter2 = CreateSimulationPrinter(backend, compartmentArray, rec->compartmentsSize, speciesArray,
        			rec->speciesSize, symbolArray, rec->symbolsSize)) == NULL) {
        		return ErrorReport(FAILING, "_InitializeRecord", "could not create simulation mp printer");
        	}
        }
    } else {
        rec->mpPrinter1 = NULL;
        rec->mpPrinter2 = NULL;
    }

    if ((constraintManager = ir->GetConstraintManager(ir)) == NULL) {
        return ErrorReport(FAILING, "_InitializeRecord", "could not get the constraint manager");
    }
    list = constraintManager->CreateListOfConstraints(constraintManager);
    rec->constraintsSize = GetLinkedListSize(list);
    if (rec->constraintsSize > 0) {
        if ((constraintArray = (CONSTRAINT**) MALLOC(rec->constraintsSize * sizeof(CONSTRAINT*))) == NULL) {
            return ErrorReport(FAILING, "_InitializeRecord", "could not allocate memory for constraints array");
        }
    }
    i = 0;
    ResetCurrentElement(list);
    while ((constraint = (CONSTRAINT*) GetNextFromLinkedList(list)) != NULL) {
        constraintArray[i] = constraint;
        i++;
    }
    rec->constraintArray = constraintArray;

    if ((rec->decider = CreateSimulationRunTerminationDecider(backend, speciesArray, rec->speciesSize, reactions,
            rec->reactionsSize, rec->constraintArray, rec->constraintsSize, rec->evaluator, FALSE, rec->timeLimit))
            == NULL) {
        return ErrorReport(FAILING, "_InitializeRecord", "could not create simulation printer");
    }

    if ((eventManager = ir->GetEventManager(ir)) == NULL) {
        return ErrorReport(FAILING, "_InitializeRecord", "could not get the event manager");
    }
    list = eventManager->CreateListOfEvents(eventManager);
    rec->eventsSize = GetLinkedListSize(list);
    if (rec->eventsSize > 0) {
        if ((eventArray = (EVENT**) MALLOC(rec->eventsSize * sizeof(EVENT*))) == NULL) {
            return ErrorReport(FAILING, "_InitializeRecord", "could not allocate memory for events array");
        }
    }
    i = 0;
    ResetCurrentElement(list);
    while ((event = (EVENT*) GetNextFromLinkedList(list)) != NULL) {
        eventArray[i] = event;
        i++;
    }
    rec->eventArray = eventArray;

    for (i = 0; i < rec->eventsSize; i++) {
        list = GetEventAssignments(rec->eventArray[i]);
        ResetCurrentElement(list);
        while ((eventAssignment = (EVENT_ASSIGNMENT*) GetNextFromLinkedList(list)) != NULL) {
            for (j = 0; j < rec->speciesSize; j++) {
                if (strcmp(GetCharArrayOfString(eventAssignment->var), GetCharArrayOfString(GetSpeciesNodeID(
                        rec->speciesArray[j]))) == 0) {
                    SetEventAssignmentVarType(eventAssignment, SPECIES_EVENT_ASSIGNMENT);
                    SetEventAssignmentIndex(eventAssignment, j);
                    break;
                }
            }
            for (j = 0; j < rec->compartmentsSize; j++) {
                if (strcmp(GetCharArrayOfString(eventAssignment->var), GetCharArrayOfString(GetCompartmentID(
                        rec->compartmentArray[j]))) == 0) {
                    SetEventAssignmentVarType(eventAssignment, COMPARTMENT_EVENT_ASSIGNMENT);
                    SetEventAssignmentIndex(eventAssignment, j);
                    break;
                }
            }
            for (j = 0; j < rec->symbolsSize; j++) {
                if (strcmp(GetCharArrayOfString(eventAssignment->var), GetCharArrayOfString(GetSymbolID(
                        rec->symbolArray[j]))) == 0) {
                    SetEventAssignmentVarType(eventAssignment, PARAMETER_EVENT_ASSIGNMENT);
                    SetEventAssignmentIndex(eventAssignment, j);
                    break;
                }
            }
        }
    }

    backend->_internal1 = (CADDR_T) rec;

    return ret;
}

static RET_VAL _InitializeSimulation(MPDE_MONTE_CARLO_RECORD *rec, int runNum) {
    RET_VAL ret = SUCCESS;
    char meanFilenameStem[512];
    char varFilenameStem[512];
    char sdFilenameStem[512];
    char mpFilenameStem1[512];
    char mpFilenameStem2[512];
    double amount = 0;
    double param = 0;
    UINT32 i = 0;
    UINT32 size = 0;
    SPECIES *species = NULL;
    SPECIES **speciesArray = rec->speciesArray;
    REACTION *reaction = NULL;
    REACTION **reactionArray = rec->reactionArray;
    KINETIC_LAW_EVALUATER *evaluator = rec->evaluator;
    SIMULATION_PRINTER *meanPrinter = rec->meanPrinter;
    SIMULATION_PRINTER *varPrinter = rec->varPrinter;
    SIMULATION_PRINTER *sdPrinter = rec->sdPrinter;
    SIMULATION_PRINTER *mpPrinter1 = rec->mpPrinter1;
    SIMULATION_PRINTER *mpPrinter2 = rec->mpPrinter2;
    double compSize = 0.0;
    COMPARTMENT *compartment = NULL;
    COMPARTMENT **compartmentArray = rec->compartmentArray;
    REB2SAC_SYMBOL *symbol = NULL;
    REB2SAC_SYMBOL **symbolArray = rec->symbolArray;
    KINETIC_LAW *law = NULL;
    BOOL change = FALSE;
    char filename[512];
    FILE *file = NULL;

    sprintf(meanFilenameStem, "%s%cmean", rec->outDir, FILE_SEPARATOR);
    sprintf(varFilenameStem, "%s%cvariance", rec->outDir, FILE_SEPARATOR);
    sprintf(sdFilenameStem, "%s%cstandard_deviation", rec->outDir, FILE_SEPARATOR);
    sprintf(mpFilenameStem1, "%s%crun-1", rec->outDir, FILE_SEPARATOR);
    sprintf(mpFilenameStem2, "%s%crun-2", rec->outDir, FILE_SEPARATOR);
    if (IS_FAILED((ret = meanPrinter->PrintStart(meanPrinter, meanFilenameStem)))) {
        return ret;
    }
    if (IS_FAILED((ret = meanPrinter->PrintHeader(meanPrinter)))) {
        return ret;
    }
    if (IS_FAILED((ret = varPrinter->PrintStart(varPrinter, varFilenameStem)))) {
        return ret;
    }
    if (IS_FAILED((ret = varPrinter->PrintHeader(varPrinter)))) {
        return ret;
    }
    if (IS_FAILED((ret = sdPrinter->PrintStart(sdPrinter, sdFilenameStem)))) {
        return ret;
    }
    if (IS_FAILED((ret = sdPrinter->PrintHeader(sdPrinter)))) {
        return ret;
    }
    if (rec->useMP != 0) {
        if (IS_FAILED((ret = mpPrinter1->PrintStart(mpPrinter1, mpFilenameStem1)))) {
            return ret;
        }
        if (IS_FAILED((ret = mpPrinter1->PrintHeader(mpPrinter1)))) {
            return ret;
        }
        if (rec->useBifur) {
        	if (IS_FAILED((ret = mpPrinter2->PrintStart(mpPrinter2, mpFilenameStem2)))) {
        		return ret;
        	}
        	if (IS_FAILED((ret = mpPrinter2->PrintHeader(mpPrinter2)))) {
        		return ret;
        	}
        }
    }
    rec->time = 0.0;
    rec->nextPrintTime = 0.0;
    rec->currentStep = 0;
    size = rec->compartmentsSize;
    for (i = 0; i < size; i++) {
        compartment = compartmentArray[i];
        if ((law = (KINETIC_LAW*) GetInitialAssignmentInCompartment(compartment)) == NULL) {
            compSize = GetSizeInCompartment(compartment);
        } else {
            law = CloneKineticLaw(law);
            SimplifyInitialAssignment(law);
            if (law->valueType == KINETIC_LAW_VALUE_TYPE_REAL) {
                compSize = GetRealValueFromKineticLaw(law);
            } else if (law->valueType == KINETIC_LAW_VALUE_TYPE_INT) {
                compSize = (double) GetIntValueFromKineticLaw(law);
            }
            if (GetSizeInCompartment(compartment) != compSize) {
                SetSizeInCompartment(compartment, compSize);
                change = TRUE;
            }
            FreeKineticLaw(&(law));
        }
        if (IS_FAILED((ret = SetCurrentSizeInCompartment(compartment, compSize)))) {
            return ret;
        }
    }
    size = rec->speciesSize;
    for (i = 0; i < size; i++) {
        species = speciesArray[i];
        if ((law = (KINETIC_LAW*) GetInitialAssignmentInSpeciesNode(species)) == NULL) {
            if (IsInitialQuantityInAmountInSpeciesNode(species)) {
                amount = GetInitialAmountInSpeciesNode(species);
                rec->oldSpeciesMeans[i] = amount;
            } else {
                amount = GetInitialAmountInSpeciesNode(species);
                rec->oldSpeciesMeans[i] = amount;
            }
        } else {
            law = CloneKineticLaw(law);
            SimplifyInitialAssignment(law);
            if (law->valueType == KINETIC_LAW_VALUE_TYPE_REAL) {
                amount = GetRealValueFromKineticLaw(law);
            } else if (law->valueType == KINETIC_LAW_VALUE_TYPE_INT) {
                amount = (double) GetIntValueFromKineticLaw(law);
            }
            if (IsInitialQuantityInAmountInSpeciesNode(species)) {
                if (GetInitialAmountInSpeciesNode(species) != amount) {
                    SetInitialAmountInSpeciesNode(species, amount);
                    rec->oldSpeciesMeans[i] = amount;
                    change = TRUE;
                }
            } else {
                if (GetInitialAmountInSpeciesNode(species) != amount) {
                    SetInitialAmountInSpeciesNode(species, amount);
                    rec->oldSpeciesMeans[i] = amount;
                    change = TRUE;
                }
            }
            FreeKineticLaw(&(law));
        }
        if (IS_FAILED((ret = SetAmountInSpeciesNode(species, amount)))) {
            return ret;
        }
    }
    size = rec->symbolsSize;
    for (i = 0; i < size; i++) {
        symbol = symbolArray[i];
        if ((law = (KINETIC_LAW*) GetInitialAssignmentInSymbol(symbol)) == NULL) {
            param = GetRealValueInSymbol(symbol);
        } else {
            law = CloneKineticLaw(law);
            SimplifyInitialAssignment(law);
            if (law->valueType == KINETIC_LAW_VALUE_TYPE_REAL) {
                param = GetRealValueFromKineticLaw(law);
            } else if (law->valueType == KINETIC_LAW_VALUE_TYPE_INT) {
                param = (double) GetIntValueFromKineticLaw(law);
            }
            if (GetRealValueInSymbol(symbol) != param) {
                SetRealValueInSymbol(symbol, param);
                change = TRUE;
            }
            FreeKineticLaw(&(law));
        }
        if (IS_FAILED((ret = SetCurrentRealValueInSymbol(symbol, param)))) {
            return ret;
        }
    }
    for (i = 0; i < rec->eventsSize; i++) {
        /* SetTriggerEnabledInEvent( rec->eventArray[i], FALSE ); */
        /* Use the line below to support true SBML semantics, i.e., nothing can be trigger at t=0 */
        if (rec->evaluator->EvaluateWithCurrentAmountsDeter(rec->evaluator, (KINETIC_LAW*) GetTriggerInEvent(
                rec->eventArray[i]))) {
	  if (GetTriggerInitialValue( rec->eventArray[i] )) {
            SetTriggerEnabledInEvent(rec->eventArray[i], TRUE);
	  } else {
	    SetTriggerEnabledInEvent( rec->eventArray[i], FALSE );
	  }
        } else {
            SetTriggerEnabledInEvent(rec->eventArray[i], FALSE);
        }
    }
    size = rec->reactionsSize;
    for (i = 0; i < size; i++) {
        reaction = reactionArray[i];
        if (IS_FAILED((ret = ResetReactionFireCount(reaction)))) {
            return ret;
        }
    }
    if (IS_FAILED((ret = _UpdateAllReactionRateUpdateTimes(rec, 0)))) {
        return ret;
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

// This function detects if at least one species bifurcated based on a specified threshold
// value and returns the number of runs, the mean values and the mean paths that went to
// bifurcated route. All this values are returned into the <birec> struct.
// If no bifurcation happened, no memory is allocated and this struct is returned empty.
// On the other hand, if a bifurcation happened, the memory allocated for each array must
// be freed to avoid memory leaks

static RET_VAL _CheckBifurcation(MPDE_MONTE_CARLO_RECORD *rec, double **mpRuns, double *mpTimes, int useMP, BIFURCATION_RECORD *birec, int previousNumberFirstCluster, FILE *file) {
    RET_VAL ret = SUCCESS;
    int i = 0;
    int k = 0;
    int l = 0;
    double min_val = 0;
    double max_val = 0;
    double mpRuns_k_i = 0;
    double min_dist1 = 0;
    double min_dist2 = 0;
    double newDistance1 = 0;
    double newDistance2 = 0;
    int index1 = 0;
    int index2 = 0;
    double timesFirstCluster = 0;
    double timesSecondCluster = 0;
    double meanTimeFirstCluster = 0;
    double meanTimeSecondCluster = 0;
    double newMeanTimeFirstCluster = 0;
    double newMeanTimeSecondCluster = 0;
    int firstToFirst = 0;
    double percentFirst = 0;
    double percentFirstToFirst = 0;
    double mpTimes_k = 0;
    BOOL bifurcationHappened = false;
    UINT32 size = rec->speciesSize;
    UINT32 runs = rec->runs;
    SPECIES **speciesArray = rec->speciesArray;
    double meanCluster1[size];
    double meanCluster2[size];
    int half = 0;
    BOOL converge = false;
    int iterations = 0;

    // Allocate memory for vector indicating which species bifurcated

    if( ( birec->isBifurcated = (BOOL*)MALLOC( size * sizeof(BOOL) ) ) == NULL ) {
        return ErrorReport( FAILING, "_CheckBifurcation", "could not allocate memory for isBifurcated array" );
    }

    for (i = 0; i < size; i++) {
        if (rec->newSpeciesMeans[i] != 0.0) {
            if (rec->speciesSD[i] / rec->newSpeciesMeans[i] > BIFURCATION_THRESHOLD) {
                bifurcationHappened = true;
                birec->isBifurcated[i] = true;
            }
            else birec->isBifurcated[i] = false;
        }
    }

    bifurcationHappened = true;

    if (! bifurcationHappened) {
        FREE( birec->isBifurcated );
        birec->isBifurcated = NULL;
        return ret;
    }

    // If at least one species bifurcated, allocate memory and perform clustering analysis

    if( ( birec->runsFirstCluster = (double*)MALLOC( size * sizeof(double) ) ) == NULL ) {
        return ErrorReport( FAILING, "_CheckBifurcation", "could not allocate memory for runsFirstCluster array" );
    }

    if( ( birec->runsSecondCluster = (double*)MALLOC( size * sizeof(double) ) ) == NULL ) {
        return ErrorReport( FAILING, "_CheckBifurcation", "could not allocate memory for runsSecondCluster array" );
    }

    if( ( birec->meansFirstCluster = (double*)MALLOC( size * sizeof(double) ) ) == NULL ) {
        return ErrorReport( FAILING, "_CheckBifurcation", "could not allocate memory for meansFirstCluster array" );
    }

    if( ( birec->meansSecondCluster = (double*)MALLOC( size * sizeof(double) ) ) == NULL ) {
        return ErrorReport( FAILING, "_CheckBifurcation", "could not allocate memory for meansSecondCluster array" );
    }

    if( ( birec->meanPathCluster1 = (double*)MALLOC( size * sizeof(double) ) ) == NULL ) {
        return ErrorReport( FAILING, "_CheckBifurcation", "could not allocate memory for meanPathCluster1 array" );
    }

    if( ( birec->meanPathCluster2 = (double*)MALLOC( size * sizeof(double) ) ) == NULL ) {
        return ErrorReport( FAILING, "_CheckBifurcation", "could not allocate memory for meanPathCluster2 array" );
    }

//    if (useMP == 3) {
//    	timesFirstCluster = 0;
//    	timesSecondCluster = 0;
//
//    	min_val = mpTimes[0];
//    	max_val = mpTimes[0];
//
//    	for (k = 1; k < runs; k++) {
//    		mpTimes_k = mpTimes[k];
//    		if ( min_val > mpTimes[k] ) min_val = mpTimes_k;
//    		if ( max_val < mpTimes[k] ) max_val = mpTimes_k;
//    	}
//
//    	for (k = 0; k < runs; k++) {
//    		mpTimes_k = mpTimes[k];
//    		if ( mpTimes_k - min_val == max_val - mpTimes_k ) {
//    			timesSecondCluster += 0.5;
//    			meanCluster2 += (mpTimes_k / 2);
//    			timesFirstCluster += 0.5;
//    			meanCluster1 += (mpTimes_k / 2);
//    		}
//    		else if ( mpTimes_k - min_val > max_val - mpTimes_k ) {
//    			timesFirstCluster++;
//    			meanCluster1 += mpTimes_k;
//    		} else {
//    			timesSecondCluster++;
//    			meanCluster2 += mpTimes_k;
//    		}
//    	}
//
//    	meanCluster1 = meanCluster1 / timesFirstCluster;
//    	meanTimeFirstCluster = meanCluster1;
//    	meanCluster2 = meanCluster2 / timesSecondCluster;
//    	meanTimeSecondCluster = meanCluster2;

//    	min_dist1 = mpTimes[0] - meanCluster1;
//    	min_dist2 = mpTimes[0] - meanCluster2;
//
//    	for (k = 1; k < runs; k++) {
//    		mpTimes_k = mpTimes[k];
//    		if ( mpTimes_k - meanCluster1 < min_dist1 ) {
//    			min_dist1 = mpTimes_k - meanCluster1;
//    			birec->timeFirstCluster = k;
//    		}
//    		if ( mpTimes_k - meanCluster2 < min_dist2 ) {
//    			min_dist2 = mpTimes_k - meanCluster2;
//    			birec->timeSecondCluster = k;
//    		}
//    	}
//    }

    half = runs/2;
    for (i = 0; i < size; i++) {
    	birec->meansFirstCluster[i] = 0;
    	birec->meansSecondCluster[i] = 0;
    }
    if (useMP == 3) {
    	meanTimeFirstCluster = 0;
    	meanTimeSecondCluster = 0;
    }
    for (k = 0; k < half; k++) {
    	for (i = 0; i < size; i++) {
    		if (IsPrintFlagSetInSpeciesNode(speciesArray[i])) {
    			birec->meansFirstCluster[i] += mpRuns[k][i];
    		}
    	}
    	if (useMP == 3) {
    		meanTimeFirstCluster += mpTimes[k];
    	}
    }
    for (i = 0; i < size; i++) {
    	if (IsPrintFlagSetInSpeciesNode(speciesArray[i])) {
    		birec->meansFirstCluster[i] /= half;
    	}
    }
    if (useMP == 3) {
    	meanTimeFirstCluster /= half;
    }
    half = runs - half;
    for (k = runs/2; k < runs; k++) {
    	for (i = 0; i < size; i++) {
    		if (IsPrintFlagSetInSpeciesNode(speciesArray[i])) {
    			birec->meansSecondCluster[i] += mpRuns[k][i];
    		}
    	}
    	if (useMP == 3) {
    		meanTimeSecondCluster += mpTimes[k];
    	}
    }
    for (i = 0; i < size; i++) {
    	if (IsPrintFlagSetInSpeciesNode(speciesArray[i])) {
    		birec->meansSecondCluster[i] /= half;
    	}
    }
    if (useMP == 3) {
    	meanTimeSecondCluster /= half;
    }
    iterations = 0;
    while (!converge && iterations < 100) {
    	for (i = 0; i < size; i++) {
    		meanCluster1[i] = 0;
    		meanCluster2[i] = 0;
    	}
    	newMeanTimeFirstCluster = 0;
    	newMeanTimeSecondCluster = 0;
    	birec->numberFirstCluster = 0;
    	birec->numberSecondCluster = 0;
    	for (k = 0; k < runs; k++) {
    		newDistance1 = 0;
    		newDistance2 = 0;
    		for (i = 0; i < size; i++) {
    			if (IsPrintFlagSetInSpeciesNode(speciesArray[i])) {
    				newDistance1 += pow(mpRuns[k][i] - birec->meansFirstCluster[i], 2);
    				newDistance2 += pow(mpRuns[k][i] - birec->meansSecondCluster[i], 2);
    			}
    		}
    		if (useMP == 3) {
    			newDistance1 += pow(mpTimes[k] - meanTimeFirstCluster, 2);
    			newDistance2 += pow(mpTimes[k] - meanTimeSecondCluster, 2);
    		}
    		if (newDistance1 <= newDistance2) {
    			for (i = 0; i < size; i++) {
    				if (IsPrintFlagSetInSpeciesNode(speciesArray[i])) {
    					meanCluster1[i] += mpRuns[k][i];
    				}
    			}
    			if (useMP == 3) {
    				newMeanTimeFirstCluster += mpTimes[k];
    			}
    			birec->numberFirstCluster++;
    		}
    		else {
    			for (i = 0; i < size; i++) {
    				if (IsPrintFlagSetInSpeciesNode(speciesArray[i])) {
    					meanCluster2[i] += mpRuns[k][i];
    				}
    			}
    			if (useMP == 3) {
    				newMeanTimeSecondCluster += mpTimes[k];
    			}
    			birec->numberSecondCluster++;
    		}
    	}
    	converge = true;
    	newMeanTimeFirstCluster /= birec->numberFirstCluster++;
    	newMeanTimeSecondCluster /= birec->numberSecondCluster++;
    	if (newMeanTimeFirstCluster != meanTimeFirstCluster || newMeanTimeSecondCluster != meanTimeSecondCluster) {
    		converge = false;
    	}
    	meanTimeFirstCluster = newMeanTimeFirstCluster;
    	meanTimeSecondCluster = newMeanTimeSecondCluster;
    	for (i = 0; i < size; i++) {
    		if (IsPrintFlagSetInSpeciesNode(speciesArray[i])) {
    			meanCluster1[i] /= birec->numberFirstCluster;
    			meanCluster2[i] /= birec->numberSecondCluster;
    			if (meanCluster1[i] != birec->meansFirstCluster[i] || meanCluster2[i] != birec->meansSecondCluster[i]) {
    				converge = false;
    			}
    			birec->meansFirstCluster[i] = meanCluster1[i];
    			birec->meansSecondCluster[i] = meanCluster2[i];
    		}
    	}
    	iterations ++;
    }

//    for (i = 0; i < size; i++) {
//
//        birec->runsFirstCluster[i] = 0;
//        birec->runsSecondCluster[i] = 0;
//
//        min_val = mpRuns[0][i];
//        max_val = mpRuns[0][i];
//
//        for (k = 1; k < runs; k++) {
//            mpRuns_k_i = mpRuns[k][i];
//            if ( min_val > mpRuns[k][i] ) min_val = mpRuns_k_i;
//            if ( max_val < mpRuns[k][i] ) max_val = mpRuns_k_i;
//        }
//
//        for (k = 0; k < runs; k++) {
//            mpRuns_k_i = mpRuns[k][i];
//            if ( mpRuns_k_i - min_val == max_val - mpRuns_k_i ) {
//            	birec->runsSecondCluster[i] += 0.5;
//            	meanCluster2 += (mpRuns_k_i / 2);
//            	birec->runsFirstCluster[i] += 0.5;
//            	meanCluster1 += (mpRuns_k_i / 2);
//            }
//            else if ( mpRuns_k_i - min_val > max_val - mpRuns_k_i ) {
//                birec->runsFirstCluster[i]++;
//                meanCluster1 += mpRuns_k_i;
//            } else {
//                birec->runsSecondCluster[i]++;
//                meanCluster2 += mpRuns_k_i;
//            }
//        }
//
//        meanCluster1 = meanCluster1 / birec->runsFirstCluster[i];
//        birec->meansFirstCluster[i] = meanCluster1;
//        meanCluster2 = meanCluster2 / birec->runsSecondCluster[i];
//        birec->meansSecondCluster[i] = meanCluster2;

        //printf("Species %d: Mean 1: %f, Mean 2: %f\n", i, birec->meansFirstCluster[i], birec->meansSecondCluster[i]);

//        min_dist1 = mpRuns[0][i] - meanCluster1;
//        min_dist2 = mpRuns[0][i] - meanCluster2;
//
//        for (k = 1; k < runs; k++) {
//            mpRuns_k_i = mpRuns[k][i];
//            if ( mpRuns_k_i - meanCluster1 < min_dist1 ) {
//                 min_dist1 = mpRuns_k_i - meanCluster1;
//                 birec->meanPathCluster1[i] = k;
//            }
//            if ( mpRuns_k_i - meanCluster2 < min_dist2 ) {
//                 min_dist2 = mpRuns_k_i - meanCluster2;
//                 birec->meanPathCluster2[i] = k;
//            }
//        }
//    }

    min_dist1 = -1;
    min_dist2 = -1;
    index1 = -1;
    index2 = -1;
    birec->numberFirstCluster = 0;
    birec->numberSecondCluster = 0;
    for (k = 0; k < runs; k++) {
    	newDistance1 = 0;
    	newDistance2 = 0;
    	for (l = 0; l < size; l++) {
    		if (IsPrintFlagSetInSpeciesNode(speciesArray[l])) {
    			newDistance1 += pow(mpRuns[k][l] - birec->meansFirstCluster[l], 2);
    			newDistance2 += pow(mpRuns[k][l] - birec->meansSecondCluster[l], 2);
    		}
    	}
    	if (useMP == 3) {
    		newDistance1 += pow(mpTimes[k] - meanTimeFirstCluster, 2);
    		newDistance2 += pow(mpTimes[k] - meanTimeSecondCluster, 2);
    	}
    	if (min_dist1 == -1) {
    		min_dist1 = newDistance1;
    		index1 = k;
    	} else if (newDistance1 < min_dist1) {
    		min_dist1 = newDistance1;
    		index1 = k;
    	}
    	if (min_dist2 == -1) {
    		min_dist2 = newDistance2;
    		index2 = k;
    	} else if (newDistance2 < min_dist2) {
    		min_dist2 = newDistance2;
    		index2 = k;
    	}
    	if (newDistance1 <= newDistance2) {
    		birec->numberFirstCluster++;
    		if (k < previousNumberFirstCluster) {
    			firstToFirst++;
    		}
    	}
    	else {
    		birec->numberSecondCluster++;
    	}
    }
    if( IS_FAILED( ( ret = _PrintBifurcationStatistics( rec->time, birec->numberFirstCluster, birec->numberSecondCluster, runs, file ) ) ) ) {
    	return ret;
    }
    percentFirst = ((double) previousNumberFirstCluster) / ((double) runs);
    percentFirstToFirst = ((double) firstToFirst) / ((double) birec->numberFirstCluster);
    for (l = 0; l < size; l++) {
    	if (percentFirst > percentFirstToFirst) {
    		birec->meanPathCluster1[l] = mpRuns[index2][l];
    		birec->meanPathCluster2[l] = mpRuns[index1][l];
    	}
    	else {
    		birec->meanPathCluster1[l] = mpRuns[index1][l];
    		birec->meanPathCluster2[l] = mpRuns[index2][l];
    	}
    }
    if (useMP == 3) {
    	if (percentFirst > percentFirstToFirst) {
    		birec->timeFirstCluster = mpTimes[index2];
    		birec->timeSecondCluster = mpTimes[index1];
    	}
    	else {
    		birec->timeFirstCluster = mpTimes[index1];
    		birec->timeSecondCluster = mpTimes[index2];
    	}
    }

    return ret;
}

static RET_VAL _RunSimulation(MPDE_MONTE_CARLO_RECORD *rec, BACK_END_PROCESSOR *backend) {
    RET_VAL ret = SUCCESS;
    int i = 0;
    int j = 0;
    int k = 0;
    int l = 0;
    double timeLimit = rec->timeLimit;
    double timeStep = rec->timeStep;
    double time = 0.0;
    double maxTime = 0.0;
    double nextPrintTime = 0.0;
    REACTION *reaction = NULL;
    SIMULATION_PRINTER *meanPrinter = NULL;
    SIMULATION_PRINTER *varPrinter = NULL;
    SIMULATION_PRINTER *sdPrinter = NULL;
    SIMULATION_PRINTER *mpPrinter1 = NULL;
    SIMULATION_PRINTER *mpPrinter2 = NULL;
    SIMULATION_RUN_TERMINATION_DECIDER *decider = NULL;
    int nextEvent = 0;
    double nextEventTime = 0;
    UINT32 size = rec->speciesSize;
    UINT32 numberSteps = rec->numberSteps;
    SPECIES *species = NULL;
    SPECIES **speciesArray = rec->speciesArray;
    SPECIES **speciesOrder = NULL;
    double end;
    double newValue;
    int useMP = rec->useMP;
    BOOL useBifur = rec->useBifur;
    double mpRun[size];
    double distance;
    double newDistance;
    int index;
    double **mpRuns;
    double *mpTimes;
    double n;
    int eventCounter = 0;
    int maxEvents = ceil(rec->timeStep);
    double minPrintInterval = rec->minPrintInterval;
    gsl_matrix *stoich_matrix = NULL;
    gsl_matrix *L_matrix = NULL;
    gsl_matrix *Lo_matrix = NULL;
    gsl_matrix *G_matrix = NULL;
    BIFURCATION_RECORD *birec = NULL;
    UINT32 reacSize = rec->reactionsSize;
    double smallProp = 0.0;
    double prop = 0.0;
    REACTION **reactionArray = rec->reactionArray;
    int numberFirstCluster = 0;
    FILE *bifurFile = NULL;
    char filename[512];

    mpRuns = (double**)MALLOC(rec->runs * sizeof(double*));
    for (i = 0 ; i < rec->runs; i ++) {
    	mpRuns[i] = (double*)MALLOC(size * sizeof(double));
    }
    mpTimes = (double*)MALLOC(rec->runs * sizeof(double));
    if (useBifur) {
    	birec = (BIFURCATION_RECORD*)MALLOC(sizeof(BIFURCATION_RECORD));
    	birec->runsFirstCluster = NULL;
    	birec->runsSecondCluster = NULL;
    	birec->meansFirstCluster = NULL;
    	birec->meansSecondCluster = NULL;
    	birec->meanPathCluster1 = NULL;
    	birec->meanPathCluster2 = NULL;
    	birec->isBifurcated = NULL;
    	birec->timeFirstCluster = 0;
    	birec->timeSecondCluster = 0;
    	birec->numberFirstCluster = 0;
    	birec->numberSecondCluster = 0;
    	sprintf( filename, "%s%cbifurcation_statistics.txt", rec->outDir, FILE_SEPARATOR );
    	if( ( bifurFile = fopen( filename, "w" ) ) == NULL ) {
    		return ErrorReport( FAILING, "_RunSimulation", "could not create a bifurcation statistics file" );
    	}
    }

//    if (useMP == 0) {
//        speciesOrder = malloc(sizeof(SPECIES*)*size);
//        for (i = 0; i < size; i ++ ) {
//        	speciesOrder[i] = speciesArray[i];
//        }
//        stoich_matrix = _GetStoichiometricMatrix(rec);
//        printf("\nStoich = \n");
//	    disp_mat(stoich_matrix);
//	    printf("\n");
//	    for (i = 0; i < size; i ++ ) {
//	    	printf("%d ", GetSpeciesNodeID(speciesOrder[i]));reaction
//	    }
//	    printf("\n\n");
//        L_matrix = conservation(stoich_matrix, speciesOrder);
//        for (i = 0; i < size; i ++ ) {
//        	printf("%d ", GetSpeciesNodeID(speciesArray[i]));
//        }mpTimes_k
//        printf("\n");
//        for (i = 0; i < size; i ++ ) {
//            printf("%d ", GetSpeciesNodeID(speciesOrder[i]));
//         }reaction
//        printf("\n");
//        Lo_matrix = linkzero(L_matrix);
//        G_matrix = gamma_matrix(Lo_matrix);
//        printf("\nG = \n");
//	    disp_mat(G_matrix);
//    }

    meanPrinter = rec->meanPrinter;
    varPrinter = rec->varPrinter;
    sdPrinter = rec->sdPrinter;
    if (useMP != 0) {
        mpPrinter1 = rec->mpPrinter1;
        if (useBifur) {
        	mpPrinter2 = rec->mpPrinter2;
        }
    }
    rec->currentStep++;
    if (minPrintInterval >= 0.0) {
        nextPrintTime = minPrintInterval;
    } else {
        nextPrintTime = (rec->currentStep * rec->timeLimit) / numberSteps;
    }
    if (IS_FAILED((ret = meanPrinter->PrintValues(meanPrinter, rec->time)))) {
        return ret;
    }
    if (useMP != 0) {
        if (IS_FAILED((ret = mpPrinter1->PrintValues(mpPrinter1, rec->time)))) {
            return ret;
        }
        if (useBifur) {
        	if (IS_FAILED((ret = mpPrinter2->PrintValues(mpPrinter2, rec->time)))) {
        		return ret;
        	}
        }
    }
    for (l = 0; l < size; l++) {
        species = speciesArray[l];
        SetAmountInSpeciesNode(species, 0.0);
    }
    if (IS_FAILED((ret = varPrinter->PrintValues(varPrinter, rec->time)))) {
        return ret;
    }
    if (IS_FAILED((ret = sdPrinter->PrintValues(sdPrinter, rec->time)))) {
        return ret;
    }
    for (l = 0; l < size; l++) {
        species = speciesArray[l];
        SetAmountInSpeciesNode(species, GetInitialAmountInSpeciesNode(species));
    }
    if (useMP != 0) {
        for (l = 0; l < size; l++) {
            mpRun[l] = rec->oldSpeciesMeans[l];
        }
    }
    while (rec->time < timeLimit) {

       // printf("Hi, rec->time is %f, timeLimit is %f\n", rec->time, timeLimit);
       // fflush(stdout);


        rec->time = time;
        if (minPrintInterval >= 0.0) {
            if (useMP == 3) {
                end = timeLimit;
            } else if (useMP == 2) {
                for (l = 0; l < size; l++) {
                    species = speciesArray[l];
                    newValue = mpRun[l];
                    SetAmountInSpeciesNode(species, newValue);
                }
                if (IS_FAILED((ret = _UpdateAllReactionRateUpdateTimes(rec, rec->time)))) {
                    return ret;
                }
                if (IS_FAILED((ret = _CalculatePropensities(rec)))) {
                    return ret;
                }
                if (reacSize > 0) {
                	smallProp = GetReactionRate(reactionArray[0]);
                    for (i = 1; i < reacSize; i++) {
                    	prop = GetReactionRate(reactionArray[i]);
                    	if (IS_REAL_EQUAL(smallProp, 0.0)) {
                    		smallProp = prop;
                    	}
                    	else if (smallProp > prop && !IS_REAL_EQUAL(prop, 0.0)) {
                    		smallProp = prop;
                    	}
                    }
                }
                //if (IS_FAILED((ret = _CalculateTotalPropensities(rec)))) {
                //    return ret;
                //}
                //if (IS_REAL_EQUAL(rec->totalPropensities, 0.0)) {
                //    n = timeStep;
                //} else {
                //    n = (timeStep / rec->totalPropensities);
                //}
                if (IS_REAL_EQUAL(smallProp, 0.0)) {
                	n = timeStep;
                } else {
                	n = (timeStep / smallProp);
                }
                end = n + rec->time;
            } else {
                end = rec->time + timeStep;
            }
            if (timeLimit < end) {
                end = timeLimit;
            }
        } else {
            if (useMP == 3) {
                end = nextPrintTime;
            } else if (useMP == 2) {
                for (l = 0; l < size; l++) {
                    species = speciesArray[l];
                    newValue = mpRun[l];
                    SetAmountInSpeciesNode(species, newValue);
                }
                if (IS_FAILED((ret = _UpdateAllReactionRateUpdateTimes(rec, rec->time)))) {
                    return ret;
                }
                if (IS_FAILED((ret = _CalculatePropensities(rec)))) {
                    return ret;
                }
                if (reacSize > 0) {
                	smallProp = GetReactionRate(reactionArray[0]);
                	for (i = 1; i < reacSize; i++) {
                		prop = GetReactionRate(reactionArray[i]);
                		if (IS_REAL_EQUAL(smallProp, 0.0)) {
                			smallProp = prop;
                		}
                		else if (smallProp > prop && !IS_REAL_EQUAL(prop, 0.0)) {
                			smallProp = prop;
                		}
                	}
                }
                //if (IS_FAILED((ret = _CalculateTotalPropensities(rec)))) {
                //    return ret;
                //}
                //if (IS_REAL_EQUAL(rec->totalPropensities, 0.0)) {
                //	n = timeStep;
                //} else {
                //	n = (timeStep / rec->totalPropensities);
                //}
                if (IS_REAL_EQUAL(smallProp, 0.0)) {
                    n = timeStep;
                } else {
                    n = (timeStep / smallProp);
                }
                if ((n + rec->time) > nextPrintTime) {
                    end = nextPrintTime;
                } else {
                    end = n + rec->time;
                }
            } else {
                end = rec->time + timeStep;
            }
            if (timeLimit < end) {
                end = timeLimit;
            }
        }
	/*
        if ((rec->decider = CreateSimulationRunTerminationDecider(backend, speciesArray, rec->speciesSize,
                rec->reactionArray, rec->reactionsSize, rec->constraintArray, rec->constraintsSize, rec->evaluator,
                FALSE, end)) == NULL) {
            return ErrorReport(FAILING, "_RunSimulation", "could not create simulation decider");
        }
	*/
	rec->decider->timeLimit = end;
        decider = rec->decider;
        for (k = 1; k <= rec->runs; k++) {

            eventCounter = 0;
            rec->time = time;
            i = 0;
            if (useMP != 0) {
            	if (!useBifur || birec->isBifurcated == NULL) {
            		for (l = 0; l < size; l++) {
                    	species = speciesArray[l];
                    	newValue = mpRun[l];
                    	SetAmountInSpeciesNode(species, newValue);
                	}
            	}
            	else if (birec->isBifurcated != NULL) {
            		if (k <= birec->numberFirstCluster) {
            			for (l = 0; l < size; l++) {
            				species = speciesArray[l];
            			    newValue = birec->meanPathCluster1[l];
            			    SetAmountInSpeciesNode(species, newValue);
            			}
            			if (useMP == 3) {
            				rec->time = birec->timeFirstCluster;
            			}
            		}
            		else {
            			for (l = 0; l < size; l++) {
            			    species = speciesArray[l];
            			    newValue = birec->meanPathCluster2[l];
            			    SetAmountInSpeciesNode(species, newValue);
            			}
            			if (useMP == 3) {
            				rec->time = birec->timeSecondCluster;
            			}
            		}
            	}
            } else {
                do {
                    for (l = 0; l < size; l++) {
                        species = speciesArray[l];
                        if (rec->speciesSD[l] == 0) {
                            newValue = rec->oldSpeciesMeans[l];
                        } else {
                            newValue = GetNextNormalRandomNumber(rec->oldSpeciesMeans[l], rec->speciesSD[l]);
                        }
                        newValue = round(newValue);
                        if (newValue < 0.0)
                            newValue = 0.0;
                        SetAmountInSpeciesNode(species, newValue);
                    }
                } while ((decider->IsTerminationConditionMet(decider, reaction, rec->time)));
            }
            if (IS_FAILED((ret = _UpdateAllReactionRateUpdateTimes(rec, rec->time)))) {
                return ret;
            }
            while (!(decider->IsTerminationConditionMet(decider, reaction, rec->time))) {
                i++;

                if (useMP == 2 || useMP == 3) {
                    maxTime = DBL_MAX;
                } else {
                    if (timeStep == DBL_MAX) {
                        maxTime = DBL_MAX;
                    } else {
                        maxTime = maxTime + timeStep;
                    }
                }
                nextEventTime = fireEvents(rec, rec->time);
		if (decider->IsTerminationConditionMet( decider, reaction, rec->time )) break;
                if (nextEventTime == -2.0) {
                    return FAILING;
                }
                if ((nextEventTime != -1) && (nextEventTime < maxTime)) {
                    maxTime = nextEventTime;
                }

                if (IS_FAILED((ret = _CalculatePropensities(rec)))) {
                    return ret;
                }
                if (IS_FAILED((ret = _CalculateTotalPropensities(rec)))) {
                    return ret;
                }
                if (IS_REAL_EQUAL(rec->totalPropensities, 0.0)) {
                    TRACE_1("the total propensity is 0 at iteration %i", i);
		    rec->t = maxTime - rec->time;
		    rec->time = maxTime;
                    reaction = NULL;
                    rec->nextReaction = NULL;
                    if (IS_FAILED((ret = _UpdateNodeValues(rec)))) {
                        return ret;
                    }
                    //if( IS_FAILED( ( ret = _Print( rec ) ) ) ) {
                    //  return ret;
                    //}
                } else {
                    if (IS_FAILED((ret = _FindNextReactionTime(rec)))) {
                        return ret;
                    }
                    if (maxTime < rec->time) {
                        rec->time -= rec->t;
                        rec->t = maxTime - rec->time;
                        rec->time = maxTime;
                        reaction = NULL;
                        rec->nextReaction = NULL;
                        if (IS_FAILED((ret = _UpdateNodeValues(rec)))) {
                            return ret;
                        }
                        //if( IS_FAILED( ( ret = _Print( rec ) ) ) ) {
                        //  return ret;
                        //}
                    } else {
                        maxTime = rec->time;
                        if (rec->time < timeLimit) {
                            if (IS_FAILED((ret = _FindNextReaction(rec)))) {
                                return ret;
                            }
                            reaction = rec->nextReaction;
                            if (IS_FAILED((ret = _UpdateNodeValues(rec)))) {
                                return ret;
                            }
                        } else {
                            //if( IS_FAILED( ( ret = _Print( rec ) ) ) ) {
                            //  return ret;
                            //}
                        }
                    }
                }
                if (useMP == 3) {
                    eventCounter++;
                    if (eventCounter >= maxEvents) {
                        break;
                    }
                }

            }

            if (k == 1) {
                for (l = 0; l < size; l++) {
                    species = speciesArray[l];
                    rec->newSpeciesMeans[l] = GetAmountInSpeciesNode(species);
                    rec->newSpeciesVariances[l] = 0;
                    if (useMP != 0) {
                        mpRuns[k - 1][l] = GetAmountInSpeciesNode(species);
                    }
                }
                printf("\n");
                fflush(stdout);
            } else {
                for (l = 0; l < size; l++) {
                    species = speciesArray[l];
                    double old = rec->newSpeciesMeans[l];
                    rec->newSpeciesMeans[l] = old + ((GetAmountInSpeciesNode(species) - old) / k);
                    double new = rec->newSpeciesMeans[l];
                    double oldVary = rec->newSpeciesVariances[l];
                    double newVary = (((k - 2) * oldVary) + (GetAmountInSpeciesNode(species) - new)
                            * (GetAmountInSpeciesNode(species) - old)) / (k - 1);
                    rec->newSpeciesVariances[l] = newVary;
                    if (useMP != 0) {
                        mpRuns[k - 1][l] = GetAmountInSpeciesNode(species);
                    }
                }
            }
            if (useMP == 3) {
                mpTimes[k - 1] = rec->time;
            }
        }

        if (useMP != 3) {
            time = end;
        }
        for (l = 0; l < size; l++) {
            species = speciesArray[l];
            rec->oldSpeciesMeans[l] = rec->newSpeciesMeans[l];
            rec->oldSpeciesVariances[l] = rec->newSpeciesVariances[l];
            rec->speciesSD[l] = sqrt(rec->newSpeciesVariances[l]);
        }
        if (useMP != 0) {
            distance = -1;
            index = -1;
            for (k = 0; k < rec->runs; k++) {
                newDistance = 0;
                for (l = 0; l < size; l++) {
                    newDistance += pow(mpRuns[k][l] - rec->oldSpeciesMeans[l], 2);
                }
                if (useMP == 3) {
                    newDistance += pow(mpTimes[k] - time, 2);
                }
                if (distance == -1) {
                    distance = newDistance;
                    index = k;
                } else if (newDistance < distance) {
                    distance = newDistance;
                    index = k;
                }
            }
            for (l = 0; l < size; l++) {
                mpRun[l] = mpRuns[index][l];
            }
            if (useMP == 3) {
                time = mpTimes[index];
                rec->time = time;
            }
        }
        if (useBifur) {
        	numberFirstCluster = birec->numberFirstCluster;
        	FREE(birec->runsFirstCluster);
        	FREE(birec->runsSecondCluster);
        	FREE(birec->meansFirstCluster);
        	FREE(birec->meansSecondCluster);
        	FREE(birec->meanPathCluster1);
        	FREE(birec->meanPathCluster2);
        	FREE(birec->isBifurcated);
        	_CheckBifurcation(rec, mpRuns, mpTimes, useMP, birec, numberFirstCluster, bifurFile);
        }
        if (time >= nextPrintTime && time != timeLimit) {
            if (minPrintInterval >= 0.0) {
                nextPrintTime += minPrintInterval;
            } else {
                time = nextPrintTime;
                rec->time = nextPrintTime;
                rec->currentStep++;
                nextPrintTime = (rec->currentStep * rec->timeLimit) / numberSteps;
            }
            printf("Time = %g\n", time);
            fflush(stdout);
            for (l = 0; l < size; l++) {
                species = speciesArray[l];
                SetAmountInSpeciesNode(species, rec->newSpeciesMeans[l]);
            }
            if (IS_FAILED((ret = meanPrinter->PrintValues(meanPrinter, rec->time)))) {
                return ret;
            }
            for (l = 0; l < size; l++) {
                species = speciesArray[l];
                SetAmountInSpeciesNode(species, rec->newSpeciesVariances[l]);
            }
            if (IS_FAILED((ret = varPrinter->PrintValues(varPrinter, rec->time)))) {
                return ret;
            }
            for (l = 0; l < size; l++) {
                species = speciesArray[l];
                SetAmountInSpeciesNode(species, rec->speciesSD[l]);
            }
            if (IS_FAILED((ret = sdPrinter->PrintValues(sdPrinter, rec->time)))) {
                return ret;
            }
            if (useMP != 0) {
            	if (!useBifur || birec->isBifurcated == NULL) {
            		for (l = 0; l < size; l++) {
            			species = speciesArray[l];
            			SetAmountInSpeciesNode(species, mpRun[l]);
            		}
            		if (IS_FAILED((ret = mpPrinter1->PrintValues(mpPrinter1, rec->time)))) {
            			return ret;
            		}
            		if (useBifur) {
            			if (IS_FAILED((ret = mpPrinter2->PrintValues(mpPrinter2, rec->time)))) {
            				return ret;
            			}
            		}
            	}
            	else if (birec->isBifurcated != NULL) {
            		for (l = 0; l < size; l++) {
            			species = speciesArray[l];
            		    SetAmountInSpeciesNode(species, birec->meanPathCluster1[l]);
            		}
            		if (useMP == 3) {
            			rec->time = birec->timeFirstCluster;
            		}
            		if (IS_FAILED((ret = mpPrinter1->PrintValues(mpPrinter1, rec->time)))) {
            		    return ret;
            		}
            		for (l = 0; l < size; l++) {
            			species = speciesArray[l];
            			SetAmountInSpeciesNode(species, birec->meanPathCluster2[l]);
            		}
            		if (useMP == 3) {
            			rec->time = birec->timeSecondCluster;
            		}
            		if (IS_FAILED((ret = mpPrinter2->PrintValues(mpPrinter2, rec->time)))) {
            			return ret;
            		}
            	}
            }
        }
    }
    if (rec->time >= timeLimit) {
        rec->time = timeLimit;
    }
    nextEventTime = fireEvents(rec, rec->time);
    printf("Time = %g\n", timeLimit);
    fflush(stdout);
    for (l = 0; l < size; l++) {
        species = speciesArray[l];
        SetAmountInSpeciesNode(species, rec->newSpeciesMeans[l]);
    }
    if (IS_FAILED((ret = meanPrinter->PrintValues(meanPrinter, rec->time)))) {
        return ret;
    }
    for (l = 0; l < size; l++) {
        species = speciesArray[l];
        SetAmountInSpeciesNode(species, rec->newSpeciesVariances[l]);
    }
    if (IS_FAILED((ret = varPrinter->PrintValues(varPrinter, rec->time)))) {
        return ret;
    }
    for (l = 0; l < size; l++) {
        species = speciesArray[l];
        SetAmountInSpeciesNode(species, rec->speciesSD[l]);
    }
    if (IS_FAILED((ret = sdPrinter->PrintValues(sdPrinter, rec->time)))) {
        return ret;
    }
    if (useMP != 0) {
    	if (!useBifur || birec->isBifurcated == NULL) {
    		for (l = 0; l < size; l++) {
    			species = speciesArray[l];
    			SetAmountInSpeciesNode(species, mpRun[l]);
    		}
    		if (IS_FAILED((ret = mpPrinter1->PrintValues(mpPrinter1, rec->time)))) {
    			return ret;
    		}
    		if (useBifur) {
    			if (IS_FAILED((ret = mpPrinter2->PrintValues(mpPrinter2, rec->time)))) {
    				return ret;
    			}
    		}
    	}
    	else if (birec->isBifurcated == NULL) {
    		for (l = 0; l < size; l++) {
    			species = speciesArray[l];
    			SetAmountInSpeciesNode(species, birec->meanPathCluster1[l]);
    		}
    		if (IS_FAILED((ret = mpPrinter1->PrintValues(mpPrinter1, rec->time)))) {
    			return ret;
    		}
    		for (l = 0; l < size; l++) {
    			species = speciesArray[l];
    			SetAmountInSpeciesNode(species, birec->meanPathCluster2[l]);
    		}
    		if (IS_FAILED((ret = mpPrinter2->PrintValues(mpPrinter2, rec->time)))) {
    			return ret;
    		}
    	}
    }
    /*
     if( IS_FAILED( ( ret = printer->PrintValues( printer, rec->time ) ) ) ) {
     return ret;
     }
     */
    if (IS_FAILED((ret = meanPrinter->PrintEnd(meanPrinter)))) {
        return ret;
    }
    if (IS_FAILED((ret = varPrinter->PrintEnd(varPrinter)))) {
        return ret;
    }
    if (IS_FAILED((ret = sdPrinter->PrintEnd(sdPrinter)))) {
        return ret;
    }
    if (useMP != 0) {
        if (IS_FAILED((ret = mpPrinter1->PrintEnd(mpPrinter1)))) {
            return ret;
        }
        if (useBifur) {
        	if (IS_FAILED((ret = mpPrinter2->PrintEnd(mpPrinter2)))) {
        		return ret;
        	}
        }
    }

    if (IS_FAILED((ret = _CleanSimulation(rec)))) {
        return ret;
    }
    for (i = 0 ; i < rec->runs; i ++) {
    	FREE(mpRuns[i]);
    }
    FREE(mpRuns);
    FREE(mpTimes);
    if (useBifur) {
    	FREE(birec->runsFirstCluster);
    	FREE(birec->runsSecondCluster);
    	FREE(birec->meansFirstCluster);
    	FREE(birec->meansSecondCluster);
    	FREE(birec->meanPathCluster1);
    	FREE(birec->meanPathCluster2);
    	FREE(birec->isBifurcated);
    	FREE(birec);
    	fclose( bifurFile );
    }

    return ret;
}

static RET_VAL _CleanSimulation(MPDE_MONTE_CARLO_RECORD *rec) {
    RET_VAL ret = SUCCESS;
    /* something goes wrong if I put fclose here */

#if 0
    if( 0 != fclose( rec->out ) ) {
        TRACE_0("file close error" );

        rec->out = NULL;
    }
#endif
    return ret;
}

static RET_VAL _CleanRecord(MPDE_MONTE_CARLO_RECORD *rec) {
    RET_VAL ret = SUCCESS;
    char filename[512];
    FILE *file = NULL;
    SIMULATION_PRINTER *meanPrinter = rec->meanPrinter;
    SIMULATION_PRINTER *varPrinter = rec->varPrinter;
    SIMULATION_PRINTER *sdPrinter = rec->sdPrinter;
    SIMULATION_PRINTER *mpPrinter1 = rec->mpPrinter1;
    SIMULATION_PRINTER *mpPrinter2 = rec->mpPrinter2;
    SIMULATION_RUN_TERMINATION_DECIDER *decider = rec->decider;

    sprintf(filename, "%s%csim-rep.txt", rec->outDir, FILE_SEPARATOR);
    if ((file = fopen(filename, "w")) == NULL) {
        return ErrorReport(FAILING, "_CleanRecord", "could not create a report file");
    }
    if (IS_FAILED((ret = decider->Report(decider, file)))) {
        return ret;
    }
    fclose(file);

    if (rec->evaluator != NULL) {
        FreeKineticLawEvaluater(&(rec->evaluator));
    }
    if (rec->reactionArray != NULL) {
        FREE(rec->reactionArray);
    }
    if (rec->speciesArray != NULL) {
        FREE(rec->speciesArray);
    }
    if (rec->oldSpeciesMeans != NULL) {
        FREE(rec->oldSpeciesMeans);
    }
    if (rec->oldSpeciesMeans != NULL) {
        FREE(rec->oldSpeciesMeans);
    }
    if (rec->oldSpeciesVariances != NULL) {
        FREE(rec->oldSpeciesVariances);
    }
    if (rec->newSpeciesMeans != NULL) {
        FREE(rec->newSpeciesMeans);
    }
    if (rec->newSpeciesVariances != NULL) {
        FREE(rec->newSpeciesVariances);
    }
    if (rec->speciesSD != NULL) {
        FREE(rec->speciesSD);
    }

    meanPrinter->Destroy(meanPrinter);
    varPrinter->Destroy(varPrinter);
    sdPrinter->Destroy(sdPrinter);
    if (rec->useMP != 0) {
        mpPrinter1->Destroy(mpPrinter1);
        if (rec->useBifur) {
        	mpPrinter2->Destroy(mpPrinter2);
        }
    }
    decider->Destroy(decider);

    return ret;
}

static RET_VAL _PrintBifurcationStatistics(double time, double numberFirstCluster, double numberSecondCluster, UINT32 runs, FILE *file) {
	RET_VAL ret = SUCCESS;

	fprintf( file, "At time %f:" NEW_LINE, time);
	fprintf( file, "%f (%d/%d) percent of the runs went to the first cluster." NEW_LINE, (numberFirstCluster / (double)runs) * 100, (int)numberFirstCluster, runs);
	fprintf( file, "%f (%d/%d) percent of the runs went to the second cluster." NEW_LINE, (numberSecondCluster / (double)runs)  * 100, (int)numberSecondCluster, runs);
	fprintf( file, NEW_LINE);

	return ret;
}

static RET_VAL _PrintStatistics(MPDE_MONTE_CARLO_RECORD *rec, FILE *file) {
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

	for (i = 0; i < symbolArray; i++) {
		symbol = symbolArray[i];
		fprintf( file, "%s = %f" NEW_LINE, *GetSymbolID(symbol), GetCurrentRealValueInSymbol(symbol));
	}
	fprintf( file, NEW_LINE);

	fprintf( file, "Initial State Vector:" NEW_LINE);

	for (i = 0; i < speciesSize; i++) {
		species = speciesArray[i];
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
		fprintf( file, "%s = %f" NEW_LINE, *GetReactionNodeID(reaction), GetReactionRate(reaction));
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
		fprintf( file, "%s = %s" NEW_LINE, *GetReactionNodeID(reaction), *ToStringKineticLaw(GetKineticLawInReactionNode(reaction)));
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

static BOOL _IsTerminationConditionMet(MPDE_MONTE_CARLO_RECORD *rec) {
    return FALSE;
}

static RET_VAL _CalculateTotalPropensities(MPDE_MONTE_CARLO_RECORD *rec) {
    RET_VAL ret = SUCCESS;
    UINT32 i = 0;
    UINT32 size = rec->reactionsSize;
    double total = 0.0;
    REACTION **reactionArray = rec->reactionArray;

    if (size > 0) {
        total = GetReactionRate(reactionArray[0]);
        for (i = 1; i < size; i++) {
            total += GetReactionRate(reactionArray[i]);
        }
    }
    rec->totalPropensities = total;
    TRACE_1("the total propensity is %f", total);
    return ret;
}

static RET_VAL _CalculatePropensities(MPDE_MONTE_CARLO_RECORD *rec) {
    RET_VAL ret = SUCCESS;
    int i = 0;
    int size = 0;
    double time = rec->time;
    double updatedTime = 0.0;
    REACTION *reaction = NULL;
    REACTION **reactionArray = rec->reactionArray;

    size = rec->reactionsSize;
    for (i = 0; i < size; i++) {
        reaction = reactionArray[i];
        updatedTime = GetReactionRateUpdatedTime(reaction);
        if (IS_REAL_EQUAL(updatedTime, time)) {
            if (IS_FAILED((ret = _CalculatePropensity(rec, reaction)))) {
                return ret;
            }
        }
    }
#if 0
    /* qsort is not good way to sort an array which is nearly sorted. */
    qsort( rec->reactionArray, size, sizeof(REACTION*), (int(*)(const void *, const void *))_ComparePropensity );
#endif

#ifdef DEBUG
    for( i = 0; i < size; i++ ) {
        printf( "(%s, %f), ", GetCharArrayOfString( GetReactionNodeName( rec->reactionArray[i] ) ), GetReactionRate( rec->reactionArray[i] ) );
    }
    printf( NEW_LINE );
#endif
    return ret;
}

static RET_VAL _CalculatePropensity(MPDE_MONTE_CARLO_RECORD *rec, REACTION *reaction) {
    RET_VAL ret = SUCCESS;
    double stoichiometry = 0.0;
    double amount = 0;
    double propensity = 0.0;
    double time = rec->time;
    SPECIES *species = NULL;
    IR_EDGE *edge = NULL;
    LINKED_LIST *edges = NULL;
    KINETIC_LAW *law = NULL;
    KINETIC_LAW_EVALUATER *evaluator = rec->evaluator;
    REB2SAC_SYMBOL *speciesRef = NULL;
    REB2SAC_SYMBOL *convFactor = NULL;

    edges = GetReactantEdges((IR_NODE*) reaction);
    ResetCurrentElement(edges);
    while ((edge = GetNextEdge(edges)) != NULL) {
      speciesRef = GetSpeciesRefInIREdge( edge );
      if (speciesRef) {
	stoichiometry = GetCurrentRealValueInSymbol( speciesRef );
      } else {
	stoichiometry = GetStoichiometryInIREdge( edge );
      }
      species = GetSpeciesInIREdge(edge);
      if (( convFactor = GetConversionFactorInSpeciesNode( species ) )!=NULL) {
	stoichiometry *= GetCurrentRealValueInSymbol( convFactor );
      }
      amount = GetAmountInSpeciesNode(species);
      if (amount < stoichiometry) {
	if (IS_FAILED((ret = SetReactionRate(reaction, 0.0)))) {
	  return ret;
	}
#ifdef DEBUG
	printf( "(%s, %f)" NEW_LINE, GetCharArrayOfString( GetReactionNodeName( reaction ) ),
		GetReactionRate( reaction ) );
#endif
	return SUCCESS;
      }
    }
#if 0
    edges = GetModifierEdges( (IR_NODE*)reaction );
    ResetCurrentElement( edges );
    while( ( edge = GetNextEdge( edges ) ) != NULL ) {
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
      amount = GetAmountInSpeciesNode( species );
      if( amount < stoichiometry ) {
	if( IS_FAILED( ( ret = SetReactionRate( reaction, 0.0 ) ) ) ) {
	  return ret;
	}
#ifdef DEBUG
	printf( "(%s, %f)" NEW_LINE, GetCharArrayOfString( GetReactionNodeName( reaction ) ),
		GetReactionRate( reaction ) );
#endif
	return SUCCESS;
      }
    }
#endif

    evaluator = rec->evaluator;
    law = GetKineticLawInReactionNode(reaction);
    propensity = evaluator->EvaluateWithCurrentAmountsDeter(evaluator, law);
    if (propensity <= 0.0) {
        if (IS_FAILED((ret = SetReactionRate(reaction, 0.0)))) {
            return ret;
        }
    }
    /* in case nan */
    else if (!(propensity < DBL_MAX)) {
        if (IS_FAILED((ret = SetReactionRate(reaction, 0.0)))) {
            return ret;
        }
    } else {
        if (IS_FAILED((ret = SetReactionRate(reaction, propensity)))) {
            return ret;
        }
    }
#ifdef DEBUG
    printf( "(%s, %f)" NEW_LINE, GetCharArrayOfString( GetReactionNodeName( reaction ) ),
            GetReactionRate( reaction ) );
#endif
    return ret;
}

static RET_VAL _FindNextReactionTime(MPDE_MONTE_CARLO_RECORD *rec) {
    RET_VAL ret = SUCCESS;
    double random = 0.0;
    double t = 0.0;

    random = GetNextUnitUniformRandomNumber();
    t = log(1.0 / random) / rec->totalPropensities;
    rec->time += t;
    rec->t = t;
    if (rec->time > rec->timeLimit) {
        rec->t -= rec->time - rec->timeLimit;
        rec->time = rec->timeLimit;
    }
    TRACE_1("time to next reaction is %f", t);
    return ret;
}

static RET_VAL _FindNextReaction(MPDE_MONTE_CARLO_RECORD *rec) {
    RET_VAL ret = SUCCESS;
    UINT32 i = 0;
    UINT32 size = rec->reactionsSize - 1;
    double random = 0.0;
    double threshold = 0.0;
    double sum = 0.0;
    REACTION *reaction = NULL;
    REACTION **reactionArray = rec->reactionArray;

    random = GetNextUnitUniformRandomNumber();
    threshold = random * rec->totalPropensities;

    TRACE_1("next reaction threshold is %f", threshold);

    for (i = 0; i < size; i++) {
        sum += GetReactionRate(reactionArray[i]);
        if (sum >= threshold) {
            break;
        }
    }

    rec->nextReaction = reactionArray[i];
    TRACE_1("next reaction is %s", GetCharArrayOfString(GetReactionNodeName(rec->nextReaction)));
    return ret;
}

static RET_VAL _Update(MPDE_MONTE_CARLO_RECORD *rec) {
    RET_VAL ret = SUCCESS;

    if (IS_FAILED((ret = _Print(rec)))) {
        return ret;
    }
    if (IS_FAILED((ret = _UpdateNodeValues(rec)))) {
        return ret;
    }

    return ret;
}

static RET_VAL _Print(MPDE_MONTE_CARLO_RECORD *rec) {
    RET_VAL ret = SUCCESS;
    double nextPrintTime = rec->nextPrintTime;
    double time = rec->time;
    double printInterval = rec->printInterval;
    REACTION *reaction = NULL;
    SIMULATION_PRINTER *meanPrinter = rec->meanPrinter;
    SIMULATION_PRINTER *varPrinter = rec->varPrinter;
    SIMULATION_PRINTER *sdPrinter = rec->sdPrinter;
    SIMULATION_PRINTER *mpPrinter1 = rec->mpPrinter1;
    SIMULATION_PRINTER *mpPrinter2 = rec->mpPrinter2;

    while ((nextPrintTime < time) && (nextPrintTime < rec->timeLimit)) {
        if (nextPrintTime > 0)
            printf("Time = %g\n", nextPrintTime);
        if (IS_FAILED((ret = meanPrinter->PrintValues(meanPrinter, nextPrintTime)))) {
            return ret;
        }
        if (IS_FAILED((ret = varPrinter->PrintValues(varPrinter, nextPrintTime)))) {
            return ret;
        }
        if (IS_FAILED((ret = sdPrinter->PrintValues(sdPrinter, nextPrintTime)))) {
            return ret;
        }
        if (rec->useMP != 0) {
            if (IS_FAILED((ret = mpPrinter1->PrintValues(mpPrinter1, nextPrintTime)))) {
                return ret;
            }
            if (rec->useBifur) {
            	if (IS_FAILED((ret = mpPrinter2->PrintValues(mpPrinter2, nextPrintTime)))) {
            		return ret;
            	}
            }
        }
        nextPrintTime += printInterval;
    }
    rec->nextPrintTime = nextPrintTime;
    return ret;
}

static RET_VAL _UpdateNodeValues(MPDE_MONTE_CARLO_RECORD *rec) {
    RET_VAL ret = SUCCESS;

    if (IS_FAILED((ret = _UpdateSpeciesValues(rec)))) {
        return ret;
    }
    if (IS_FAILED((ret = _UpdateReactionRateUpdateTime(rec)))) {
        return ret;
    }

    return ret;
}

static double fireEvents(MPDE_MONTE_CARLO_RECORD *rec, double time) {
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

    do {
      eventFired = FALSE;
      eventToFire = -1;
      for (i = 0; i < rec->eventsSize; i++) {
	nextEventTime = GetNextEventTimeInEvent( rec->eventArray[i] );
	triggerEnabled = GetTriggerEnabledInEvent( rec->eventArray[i] );
	if (nextEventTime != -1.0) {
	  /* Disable event, if necessary */
	  if ((triggerEnabled) && (GetTriggerCanBeDisabled( rec->eventArray[i] ))) {
	    if (!rec->evaluator->EvaluateWithCurrentAmountsDeter( rec->evaluator,
							     (KINETIC_LAW*)GetTriggerInEvent( rec->eventArray[i] ) )) { 
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
	      priority = rec->evaluator->EvaluateWithCurrentAmounts( rec->evaluator,
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
	nextEventTime = rec->time + 
	  rec->findNextTime->FindNextTimeWithCurrentAmounts( rec->findNextTime,
							     (KINETIC_LAW*)GetTriggerInEvent( rec->eventArray[i] ));
	if ((firstEventTime == -1.0) || (nextEventTime < firstEventTime)) {
	  firstEventTime = nextEventTime;
	}
	if (!triggerEnabled) {
	  /* Check if event has been triggered */
	  if (rec->evaluator->EvaluateWithCurrentAmountsDeter( rec->evaluator,
							  (KINETIC_LAW*)GetTriggerInEvent( rec->eventArray[i] ) )) {
	    SetTriggerEnabledInEvent( rec->eventArray[i], TRUE );
	    /* Calculate delay until the event fires */
	    if (GetDelayInEvent( rec->eventArray[i] )==NULL) {
	      deltaTime = 0;
	    }
	    else {
	      deltaTime = rec->evaluator->EvaluateWithCurrentAmounts( rec->evaluator,
								      (KINETIC_LAW*)GetDelayInEvent( rec->eventArray[i] ) );
	    }
	    if (deltaTime == 0) eventFired = TRUE;
	    if (deltaTime >= 0) {
	      /* Set time for event to fire and get assignment values, if necessary */
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
	  if (!rec->evaluator->EvaluateWithCurrentAmountsDeter( rec->evaluator,
							   (KINETIC_LAW*)GetTriggerInEvent( rec->eventArray[i] ) )) {
	    SetTriggerEnabledInEvent( rec->eventArray[i], FALSE );
	  } 
	}
      }
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

static void SetEventAssignmentsNextValues(EVENT *event, MPDE_MONTE_CARLO_RECORD *rec) {
    LINKED_LIST *list = NULL;
    EVENT_ASSIGNMENT *eventAssignment;
    double amount = 0.0;
    UINT j;
    BYTE varType;

    list = GetEventAssignments(event);
    ResetCurrentElement(list);
    while ((eventAssignment = (EVENT_ASSIGNMENT*) GetNextFromLinkedList(list)) != NULL) {
        amount = rec->evaluator->EvaluateWithCurrentAmounts(rec->evaluator, eventAssignment->assignment);
        SetEventAssignmentNextValueTime(eventAssignment, amount, rec->time);
    }
}

static void SetEventAssignmentsNextValuesTime(EVENT *event, MPDE_MONTE_CARLO_RECORD *rec, double time) {
    LINKED_LIST *list = NULL;
    EVENT_ASSIGNMENT *eventAssignment;
    double amount = 0.0;
    UINT j;
    BYTE varType;

    list = GetEventAssignments(event);
    ResetCurrentElement(list);
    while ((eventAssignment = (EVENT_ASSIGNMENT*) GetNextFromLinkedList(list)) != NULL) {
        amount = rec->evaluator->EvaluateWithCurrentAmounts(rec->evaluator, eventAssignment->assignment);
        SetEventAssignmentNextValueTime(eventAssignment, amount, time);
    }
}

static void fireEvent(EVENT *event, MPDE_MONTE_CARLO_RECORD *rec) {
    LINKED_LIST *list = NULL;
    EVENT_ASSIGNMENT *eventAssignment;
    double amount = 0.0;
    UINT j;
    BYTE varType;

    list = GetEventAssignments(event);
    ResetCurrentElement(list);
    while ((eventAssignment = (EVENT_ASSIGNMENT*) GetNextFromLinkedList(list)) != NULL) {
        //printf("Firing event %s\n",GetCharArrayOfString(eventAssignment->var));
        varType = GetEventAssignmentVarType(eventAssignment);
        j = GetEventAssignmentIndex(eventAssignment);
        //printf("varType = %d j = %d\n",varType,j);
	amount = GetEventAssignmentNextValueTime(eventAssignment, rec->time);
        //printf("conc = %g\n",amount);
        if (varType == SPECIES_EVENT_ASSIGNMENT) {
            SetAmountInSpeciesNode(rec->speciesArray[j], amount);
            _UpdateReactionRateUpdateTimeForSpecies(rec, rec->speciesArray[j]);
        } else if (varType == COMPARTMENT_EVENT_ASSIGNMENT) {
            SetCurrentSizeInCompartment(rec->compartmentArray[j], amount);
            _UpdateAllReactionRateUpdateTimes(rec, rec->time);
        } else {
            SetCurrentRealValueInSymbol(rec->symbolArray[j], amount);
            _UpdateAllReactionRateUpdateTimes(rec, rec->time);
        }
    }
}

/* Update values using assignments rules */
static void ExecuteAssignments(MPDE_MONTE_CARLO_RECORD *rec) {
    UINT32 i = 0;
    UINT32 j = 0;
    double amount = 0.0;
    BYTE varType;

    for (i = 0; i < rec->rulesSize; i++) {
        if (GetRuleType(rec->ruleArray[i]) == RULE_TYPE_ASSIGNMENT) {
            amount = rec->evaluator->EvaluateWithCurrentAmountsDeter(rec->evaluator, (KINETIC_LAW*) GetMathInRule(
                    rec->ruleArray[i]));
            varType = GetRuleVarType(rec->ruleArray[i]);
            j = GetRuleIndex(rec->ruleArray[i]);
            if (varType == SPECIES_RULE) {
                SetAmountInSpeciesNode(rec->speciesArray[j], amount);
                _UpdateReactionRateUpdateTimeForSpecies(rec, rec->speciesArray[j]);
            } else if (varType == COMPARTMENT_RULE) {
                SetCurrentSizeInCompartment(rec->compartmentArray[j], amount);
                _UpdateAllReactionRateUpdateTimes(rec, rec->time);
            } else {
                SetCurrentRealValueInSymbol(rec->symbolArray[j], amount);
                _UpdateAllReactionRateUpdateTimes(rec, rec->time);
            }
        }
    }
}

int MPDEalgebraicRules(const gsl_vector * x, void *params, gsl_vector * f) {
  UINT32 i = 0;
  UINT32 j = 0;
  double amount = 0.0;
  MPDE_MONTE_CARLO_RECORD *rec = ((MPDE_MONTE_CARLO_RECORD*)params);
  SPECIES *species = NULL;
  COMPARTMENT *compartment = NULL;
  REB2SAC_SYMBOL *symbol = NULL;
  
  for( i = 0; i < rec->speciesSize; i++ ) {
    species = rec->speciesArray[i];
    if (IsSpeciesNodeAlgebraic( species )) {
      amount = gsl_vector_get (x, j);
      j++; 
      SetAmountInSpeciesNode( species, amount );
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
      amount = rec->evaluator->EvaluateWithCurrentAmountsDeter( rec->evaluator,
							   (KINETIC_LAW*)GetMathInRule( rec->ruleArray[i] ) );
      gsl_vector_set (f, j, amount);
      j++;
    } 
  }
     
  return GSL_SUCCESS;
}

int MPDE_print_state (size_t iter,gsl_multiroot_fsolver * s,int n) {
  UINT32 i = 0;

  printf("x = [ ");
  for (i = 0; i < n; i++) {
    printf(" %g ",gsl_vector_get (s->x, i));
  }
  printf("] f = [ ");
  for (i = 0; i < n; i++) {
    printf(" %g ",gsl_vector_get (s->f, i));
  }
  printf("]\n");
}

static RET_VAL EvaluateAlgebraicRules( MPDE_MONTE_CARLO_RECORD *rec ) {
  const gsl_multiroot_fsolver_type *T;
  gsl_multiroot_fsolver *s;
  SPECIES *species = NULL;
  COMPARTMENT *compartment = NULL;
  REB2SAC_SYMBOL *symbol = NULL;
     
  int status;
  size_t i, j, iter = 0;
  double amount;

  const size_t n = rec->algebraicRulesSize;

  gsl_multiroot_function f = {&MPDEalgebraicRules, n, rec};
  gsl_vector *x = gsl_vector_alloc (n);
     
  j = 0;
  for( i = 0; i < rec->speciesSize; i++ ) {
    species = rec->speciesArray[i];
    if (IsSpeciesNodeAlgebraic( species )) {
      amount = GetAmountInSpeciesNode( species );
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
     
  //MPDE_print_state (iter, s, n);
     
  do {
      iter++;
      status = gsl_multiroot_fsolver_iterate (s);
      
      //MPDE_print_state (iter, s, n);
      
      if (status)   /* check if solver is stuck */
	break;
      
      status = gsl_multiroot_test_residual (s->f, 1e-7);
  } while (status == GSL_CONTINUE && iter < 1000);
     
  //printf ("status = %s\n", gsl_strerror (status));
     
  gsl_multiroot_fsolver_free (s);
  gsl_vector_free (x);
  return 0;
}

int MPDEfastReactions(const gsl_vector * x, void *params, gsl_vector * f) {
  RET_VAL ret = SUCCESS;
  UINT32 i = 0;
  UINT32 j;
  double amount = 0.0;
  MPDE_MONTE_CARLO_RECORD *rec = ((MPDE_MONTE_CARLO_RECORD*)params);
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
      } else {
	SetConcentrationInSpeciesNode( species, amount );
      }
    }
  }
  _CalculatePropensities( rec );
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
	if (HasBoundaryConditionInSpeciesNode( species )) {
	  amount = GetAmountInSpeciesNode( species );
	  boundary = TRUE;
	} else {
	  amount = GetAmountInSpeciesNode( species ) / stoichiometry;
	  boundary = FALSE;
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
	if (HasBoundaryConditionInSpeciesNode( species )) {
	  amount = GetAmountInSpeciesNode( species );
	  boundary = TRUE;
	} else {
	  amount = GetAmountInSpeciesNode( species ) / stoichiometry;
	  boundary = FALSE;
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
	}
      }
      if (!boundary) {
	amount = GetReactionRate( reaction );
	gsl_vector_set (f, j, amount);
	j++;
      }
      break;
    }
  } 
       
  return GSL_SUCCESS;
}

static RET_VAL ExecuteFastReactions( MPDE_MONTE_CARLO_RECORD *rec ) {
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
	if (HasBoundaryConditionInSpeciesNode( species )) {
	  amount = GetAmountInSpeciesNode( species );
	  boundary = TRUE;
	} else {
	  amount = GetAmountInSpeciesNode( species ) / stoichiometry;
	  boundary = FALSE;
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
	if (HasBoundaryConditionInSpeciesNode( species )) {
	  amount = GetAmountInSpeciesNode( species );
	  boundary = TRUE;
	} else {
	  amount = GetAmountInSpeciesNode( species ) / stoichiometry;
	  boundary = FALSE;
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
	}
      }
      break;
    }
  } 

  gsl_multiroot_function f = {&MPDEfastReactions, n, rec};

  gsl_vector *x = gsl_vector_alloc (n);
  
  j = 0;
  for( i = 0; i < rec->speciesSize; i++ ) {
    species = rec->speciesArray[i];
    if (!HasBoundaryConditionInSpeciesNode( species ) && IsSpeciesNodeFast( species )) {
      amount = GetAmountInSpeciesNode( species );
      gsl_vector_set (x, j, amount);
      j++;
    } 
  }
     
  T = gsl_multiroot_fsolver_hybrids;
  s = gsl_multiroot_fsolver_alloc (T, n);
  gsl_multiroot_fsolver_set (s, &f, x);

  //printf("time = %g\n",rec->time);
  //MPDE_print_state (iter, s, n);
     
  do {
      iter++;
      status = gsl_multiroot_fsolver_iterate (s);
      
      //MPDE_print_state (iter, s, n);
      
      if (status)   /* check if solver is stuck */
	break;
      
      status = gsl_multiroot_test_residual (s->f, 1e-7);
  } while (status == GSL_CONTINUE && iter < 1000);
     
  //printf ("status = %s\n", gsl_strerror (status));

  j = 0;
  for( i = 0; i < rec->speciesSize; i++ ) {
    species = rec->speciesArray[i];
    if (!HasBoundaryConditionInSpeciesNode( species ) && IsSpeciesNodeFast( species )) {
      //if (IsSpeciesNodeFast( species )) {
      amount = gsl_vector_get (s->x, j);
      j++; 
      SetAmountInSpeciesNode( species, amount );
    }
  }
     
  gsl_multiroot_fsolver_free (s);
  gsl_vector_free (x);
  return 0;
}

static RET_VAL _UpdateSpeciesValues(MPDE_MONTE_CARLO_RECORD *rec) {
    RET_VAL ret = SUCCESS;
    double stoichiometry = 0;
    double amount = 0;
    double change = 0;
    SPECIES *species = NULL;
    IR_EDGE *edge = NULL;
    REACTION *reaction = rec->nextReaction;
    LINKED_LIST *edges = NULL;
    KINETIC_LAW_EVALUATER *evaluator = rec->evaluator;
    UINT i = 0;
    UINT j = 0;
    double deltaTime;
    BOOL triggerEnabled;
    BYTE varType;
    REB2SAC_SYMBOL *speciesRef = NULL;
    REB2SAC_SYMBOL *convFactor = NULL;

    for (i = 0; i < rec->rulesSize; i++) {
        if (GetRuleType(rec->ruleArray[i]) == RULE_TYPE_RATE_ASSIGNMENT) {
            change = rec->evaluator->EvaluateWithCurrentAmountsDeter(rec->evaluator, (KINETIC_LAW*) GetMathInRule(
                    rec->ruleArray[i]));
            varType = GetRuleVarType(rec->ruleArray[i]);
            j = GetRuleIndex(rec->ruleArray[i]);
            if (varType == SPECIES_RULE) {
	      SetRateInSpeciesNode( rec->speciesArray[j], change );
	      amount = GetAmountInSpeciesNode(rec->speciesArray[j]);
	      amount += (change * rec->t);
	      SetRuleCurValue(rec->ruleArray[i], amount);
            } else if (varType == COMPARTMENT_RULE) {
	      SetCurrentRateInCompartment( rec->compartmentArray[j], change );
	      amount = GetCurrentSizeInCompartment(rec->compartmentArray[j]);
	      amount += (change * rec->t);
	      SetRuleCurValue(rec->ruleArray[i], amount);
            } else {
	      SetCurrentRateInSymbol( rec->symbolArray[j], change );
	      amount = GetCurrentRealValueInSymbol(rec->symbolArray[j]);
	      amount += (change * rec->t);
	      SetRuleCurValue(rec->ruleArray[i], amount);
            }
        }
    }
    for (i = 0; i < rec->rulesSize; i++) {
        if (GetRuleType(rec->ruleArray[i]) == RULE_TYPE_RATE_ASSIGNMENT) {
            amount = GetRuleCurValue(rec->ruleArray[i]);
            varType = GetRuleVarType(rec->ruleArray[i]);
            j = GetRuleIndex(rec->ruleArray[i]);
            if (varType == SPECIES_RULE) {
                SetAmountInSpeciesNode(rec->speciesArray[j], amount);
                _UpdateReactionRateUpdateTimeForSpecies(rec, rec->speciesArray[j]);
            } else if (varType == COMPARTMENT_RULE) {
                SetCurrentSizeInCompartment(rec->compartmentArray[j], amount);
                _UpdateAllReactionRateUpdateTimes(rec, rec->time);
            } else {
                SetCurrentRealValueInSymbol(rec->symbolArray[j], amount);
                _UpdateAllReactionRateUpdateTimes(rec, rec->time);
            }
        }
    }

    if (reaction) {
        edges = GetReactantEdges((IR_NODE*) reaction);
        ResetCurrentElement(edges);
        while ((edge = GetNextEdge(edges)) != NULL) {
	  speciesRef = GetSpeciesRefInIREdge( edge );
	  if (speciesRef) {
	    stoichiometry = GetCurrentRealValueInSymbol( speciesRef );
	  } else {
	    stoichiometry = GetStoichiometryInIREdge( edge );
	  }
	  species = GetSpeciesInIREdge(edge);
	  if (( convFactor = GetConversionFactorInSpeciesNode( species ) )!=NULL) {
	    stoichiometry *= GetCurrentRealValueInSymbol( convFactor );
	  }
	  if (HasBoundaryConditionInSpeciesNode(species))
	    continue;
	  amount = GetAmountInSpeciesNode(species) - stoichiometry;
	  TRACE_3("the amount of %s decreases from %g to %g", GetCharArrayOfString(GetSpeciesNodeName(species)),
		  GetAmountInSpeciesNode(species), amount);
	  SetAmountInSpeciesNode(species, amount);
	  if (IS_FAILED((ret = evaluator->SetSpeciesValue(evaluator, species, amount)))) {
	    return ret;
	  }
        }
        edges = GetProductEdges((IR_NODE*) reaction);
        ResetCurrentElement(edges);
        while ((edge = GetNextEdge(edges)) != NULL) {
	  speciesRef = GetSpeciesRefInIREdge( edge );
	  if (speciesRef) {
	    stoichiometry = GetCurrentRealValueInSymbol( speciesRef );
	  } else {
	    stoichiometry = GetStoichiometryInIREdge( edge );
	  }
	  species = GetSpeciesInIREdge(edge);
	  if (( convFactor = GetConversionFactorInSpeciesNode( species ) )!=NULL) {
	    stoichiometry *= GetCurrentRealValueInSymbol( convFactor );
	  }
	  if (HasBoundaryConditionInSpeciesNode(species))
	    continue;
	  amount = GetAmountInSpeciesNode(species) + stoichiometry;
	  TRACE_3("the amount of %s increases from %g to %g", GetCharArrayOfString(GetSpeciesNodeName(species)),
		  GetAmountInSpeciesNode(species), amount);
	  SetAmountInSpeciesNode(species, amount);
        }
    }

    for (j = 0; j < rec->symbolsSize; j++) {
        if ((strcmp(GetCharArrayOfString(GetSymbolID(rec->symbolArray[j])), "t") == 0) || (strcmp(GetCharArrayOfString(
                GetSymbolID(rec->symbolArray[j])), "time") == 0)) {
            SetCurrentRealValueInSymbol(rec->symbolArray[j], rec->time);
        }
    }

    ExecuteAssignments(rec);
    if (rec->algebraicRulesSize > 0) {
      EvaluateAlgebraicRules( rec );
    }
    if (rec->numberFastSpecies > 0) {
      ExecuteFastReactions( rec );
    }

    return ret;
}

static RET_VAL _UpdateAllReactionRateUpdateTimes(MPDE_MONTE_CARLO_RECORD *rec, double time) {
    RET_VAL ret = SUCCESS;
    UINT i;

    for (i = 0; i < rec->reactionsSize; i++) {
        if (IS_FAILED((ret = SetReactionRateUpdatedTime(rec->reactionArray[i], time)))) {
            return ret;
        }
    }
    return ret;
}

static RET_VAL _UpdateReactionRateUpdateTimeForSpecies(MPDE_MONTE_CARLO_RECORD *rec, SPECIES* species) {
    RET_VAL ret = SUCCESS;
    double time = rec->time;
    IR_EDGE *updateEdge = NULL;
    REACTION *reaction = NULL;
    LINKED_LIST *updateEdges = NULL;

    updateEdges = GetReactantEdges((IR_NODE*) species);
    ResetCurrentElement(updateEdges);
    while ((updateEdge = GetNextEdge(updateEdges)) != NULL) {
        reaction = GetReactionInIREdge(updateEdge);
        if (IS_FAILED((ret = SetReactionRateUpdatedTime(reaction, time)))) {
            return ret;
        }
    }

    updateEdges = GetModifierEdges((IR_NODE*) species);
    ResetCurrentElement(updateEdges);
    while ((updateEdge = GetNextEdge(updateEdges)) != NULL) {
        reaction = GetReactionInIREdge(updateEdge);
        if (IS_FAILED((ret = SetReactionRateUpdatedTime(reaction, time)))) {
            return ret;
        }
    }

    updateEdges = GetProductEdges((IR_NODE*) species);
    ResetCurrentElement(updateEdges);
    while ((updateEdge = GetNextEdge(updateEdges)) != NULL) {
        reaction = GetReactionInIREdge(updateEdge);
        if (IS_FAILED((ret = SetReactionRateUpdatedTime(reaction, time)))) {
            return ret;
        }
    }
    return ret;
}

static RET_VAL _UpdateReactionRateUpdateTime(MPDE_MONTE_CARLO_RECORD *rec) {
    RET_VAL ret = SUCCESS;
    double rate = 0.0;
    IR_EDGE *edge = NULL;
    LINKED_LIST *edges = NULL;
    SPECIES *species = NULL;

    if (rec->nextReaction == NULL) {
        return ret;
    }
    edges = GetReactantEdges((IR_NODE*) rec->nextReaction);
    ResetCurrentElement(edges);
    while ((edge = GetNextEdge(edges)) != NULL) {
        species = GetSpeciesInIREdge(edge);
        if (IS_FAILED((ret = _UpdateReactionRateUpdateTimeForSpecies(rec, species)))) {
            return ret;
        }
    }

    edges = GetProductEdges((IR_NODE*) rec->nextReaction);
    ResetCurrentElement(edges);
    while ((edge = GetNextEdge(edges)) != NULL) {
        species = GetSpeciesInIREdge(edge);
        if (IS_FAILED((ret = _UpdateReactionRateUpdateTimeForSpecies(rec, species)))) {
            return ret;
        }
    }
    return ret;
}

static int _ComparePropensity(REACTION *a, REACTION *b) {
    double d1 = 0.0;
    double d2 = 0.0;

    d1 = GetReactionRate(a);
    d2 = GetReactionRate(b);

    if (IS_REAL_EQUAL(d1, d2)) {
        return 0;
    }
    return (d1 < d2) ? -1 : 1;
}

static gsl_matrix* _GetStoichiometricMatrix(MPDE_MONTE_CARLO_RECORD *rec) {
    double stoichiometry = 0;
    UINT32 i = 0;
    UINT32 j = 0;
    UINT32 reactionsSize = rec->reactionsSize;
    UINT32 speciesSize = rec->speciesSize;
    REACTION *reaction = NULL;
    REACTION **reactionArray = rec->reactionArray;
    SPECIES *species = NULL;
    SPECIES **speciesArray = rec->speciesArray;
    REB2SAC_SYMBOL *speciesRef = NULL;
    REB2SAC_SYMBOL *convFactor = NULL;
    IR_EDGE *edge = NULL;
    LINKED_LIST *edges = NULL;
    gsl_matrix *matrix = gsl_matrix_alloc(speciesSize,reactionsSize);

    for (i = 0; i < reactionsSize; i++) {
        for (j = 0; j < speciesSize; j++) {
          gsl_matrix_set(matrix,j,i,0);
        }
    }

    for (i = 0; i < reactionsSize; i++) {
        reaction = reactionArray[i];
        edges = GetReactantEdges((IR_NODE*) reaction);
        ResetCurrentElement(edges);
        while ((edge = GetNextEdge(edges)) != NULL) {
	  speciesRef = GetSpeciesRefInIREdge( edge );
	  if (speciesRef) {
	    stoichiometry = GetCurrentRealValueInSymbol( speciesRef );
	  } else {
	    stoichiometry = GetStoichiometryInIREdge( edge );
	  }
	  species = GetSpeciesInIREdge(edge);
	  if (( convFactor = GetConversionFactorInSpeciesNode( species ) )!=NULL) {
	    stoichiometry *= GetCurrentRealValueInSymbol( convFactor );
	  }
	  if (HasBoundaryConditionInSpeciesNode(species))
	    continue;
          for (j = 0; j < speciesSize; j++) {
            if(species == speciesArray[j]) {
              gsl_matrix_set(matrix,j,i,gsl_matrix_get(matrix,j,i)-stoichiometry);
            }
          }
        }
        edges = GetProductEdges((IR_NODE*) reaction);
        ResetCurrentElement(edges);
        while ((edge = GetNextEdge(edges)) != NULL) {
	  speciesRef = GetSpeciesRefInIREdge( edge );
	  if (speciesRef) {
	    stoichiometry = GetCurrentRealValueInSymbol( speciesRef );
	  } else {
	    stoichiometry = GetStoichiometryInIREdge( edge );
	  }
	  species = GetSpeciesInIREdge(edge);
	  if (( convFactor = GetConversionFactorInSpeciesNode( species ) )!=NULL) {
	    stoichiometry *= GetCurrentRealValueInSymbol( convFactor );
	  }
	  if (HasBoundaryConditionInSpeciesNode(species))
	    continue;
          for (j = 0; j < speciesSize; j++) {
            if(species == speciesArray[j]) {
              gsl_matrix_set(matrix,j,i,gsl_matrix_get(matrix,j,i)+stoichiometry);
            }
          }
        }
    }

    return matrix;
}

