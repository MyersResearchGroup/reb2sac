#include <math.h>
#include <float.h>
#include "marginal_probability_density_evolution_monte_carlo.h"

static BOOL _IsModelConditionSatisfied(IR *ir);

static RET_VAL _InitializeRecord(MPDE_MONTE_CARLO_RECORD *rec, BACK_END_PROCESSOR *backend, IR *ir);
static RET_VAL _InitializeSimulation(MPDE_MONTE_CARLO_RECORD *rec, int runNum);
static RET_VAL _RunSimulation(MPDE_MONTE_CARLO_RECORD *rec, BACK_END_PROCESSOR *backend);

static RET_VAL _CleanSimulation(MPDE_MONTE_CARLO_RECORD *rec);
static RET_VAL _CleanRecord(MPDE_MONTE_CARLO_RECORD *rec);

static RET_VAL _CalculateTotalPropensities(MPDE_MONTE_CARLO_RECORD *rec);
static RET_VAL _CalculatePropensities(MPDE_MONTE_CARLO_RECORD *rec);
static RET_VAL _CalculatePropensity(MPDE_MONTE_CARLO_RECORD *rec, REACTION *reaction);
static RET_VAL _FindNextReactionTime(MPDE_MONTE_CARLO_RECORD *rec);
static RET_VAL _FindNextReaction(MPDE_MONTE_CARLO_RECORD *rec);
static RET_VAL _Update(MPDE_MONTE_CARLO_RECORD *rec);
static RET_VAL _Print(MPDE_MONTE_CARLO_RECORD *rec);
static RET_VAL _UpdateNodeValues(MPDE_MONTE_CARLO_RECORD *rec);
static RET_VAL _UpdateSpeciesValues(MPDE_MONTE_CARLO_RECORD *rec);
static RET_VAL _UpdateReactionRateUpdateTime(MPDE_MONTE_CARLO_RECORD *rec);
static RET_VAL _UpdateReactionRateUpdateTimeForSpecies(MPDE_MONTE_CARLO_RECORD *rec, SPECIES* species);
static RET_VAL _UpdateAllReactionRateUpdateTimes(MPDE_MONTE_CARLO_RECORD *rec, double time);

static int _ComparePropensity(REACTION *a, REACTION *b);
static BOOL _IsTerminationConditionMet(MPDE_MONTE_CARLO_RECORD *rec);

static double fireEvents(MPDE_MONTE_CARLO_RECORD *rec, double time);
static void fireEvent(EVENT *event, MPDE_MONTE_CARLO_RECORD *rec);
static void ExecuteAssignments(MPDE_MONTE_CARLO_RECORD *rec);
static void SetEventAssignmentsNextValues(EVENT *event, MPDE_MONTE_CARLO_RECORD *rec);

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
    if (timeout > (rec.speciesSize + rec.compartmentsSize + rec.symbolsSize)) {
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
    double printInterval = 0.0;
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

    list = ir->GetListOfReactionNodes(ir);
    rec->reactionsSize = GetLinkedListSize(list);
    //    if (rec->reactionsSize==0) {
    //        return ErrorReport( FAILING, "_InitializeRecord", "no reactions in the model" );
    //}
    if (rec->reactionsSize > 0) {
        if ((reactions = (REACTION**) MALLOC(rec->reactionsSize * sizeof(REACTION*))) == NULL) {
            return ErrorReport(FAILING, "_InitializeRecord", "could not allocate memory for reaction array");
        }
    }
    i = 0;
    ResetCurrentElement(list);
    while ((reaction = (REACTION*) GetNextFromLinkedList(list)) != NULL) {
        reactions[i] = reaction;
        i++;
    }
    rec->reactionArray = reactions;

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
    ResetCurrentElement(list);
    while ((rule = (RULE*) GetNextFromLinkedList(list)) != NULL) {
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
    ResetCurrentElement(list);
    while ((species = (SPECIES*) GetNextFromLinkedList(list)) != NULL) {
        speciesArray[i] = species;
        oldSpeciesMeans[i] = 0;
        oldSpeciesVariances[i] = 0;
        newSpeciesMeans[i] = 0;
        newSpeciesVariances[i] = 0;
        speciesSD[i] = 0;
        i++;
    }
    rec->speciesArray = speciesArray;
    rec->oldSpeciesMeans = oldSpeciesMeans;
    rec->oldSpeciesVariances = oldSpeciesVariances;
    rec->newSpeciesMeans = newSpeciesMeans;
    rec->newSpeciesVariances = newSpeciesVariances;
    rec->speciesSD = speciesSD;

    for (i = 0; i < rec->rulesSize; i++) {
        if (GetRuleType(rec->ruleArray[i]) == RULE_TYPE_ASSIGNMENT || GetRuleType(rec->ruleArray[i]) == RULE_TYPE_RATE) {
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

    if ((valueString = properties->GetProperty(properties, MONTE_CARLO_SIMULATION_PRINT_INTERVAL)) == NULL) {
        if ((valueString = properties->GetProperty(properties, MONTE_CARLO_SIMULATION_NUMBER_STEPS)) == NULL) {
            rec->numberSteps = DEFAULT_MONTE_CARLO_SIMULATION_NUMBER_STEPS_VALUE;
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
    if (backend->useMP == 1 || backend->useMP == 2) {
        if ((rec->mpPrinter = CreateSimulationPrinter(backend, compartmentArray, rec->compartmentsSize, speciesArray,
                rec->speciesSize, symbolArray, rec->symbolsSize)) == NULL) {
            return ErrorReport(FAILING, "_InitializeRecord", "could not create simulation sd printer");
        }
    } else {
        rec->mpPrinter = NULL;
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
    char mpFilenameStem[512];
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
    SIMULATION_PRINTER *mpPrinter = rec->mpPrinter;
    double compSize = 0.0;
    COMPARTMENT *compartment = NULL;
    COMPARTMENT **compartmentArray = rec->compartmentArray;
    REB2SAC_SYMBOL *symbol = NULL;
    REB2SAC_SYMBOL **symbolArray = rec->symbolArray;
    KINETIC_LAW *law = NULL;
    BOOL change = FALSE;

    sprintf(meanFilenameStem, "%s%cmean", rec->outDir, FILE_SEPARATOR);
    sprintf(varFilenameStem, "%s%cvariance", rec->outDir, FILE_SEPARATOR);
    sprintf(sdFilenameStem, "%s%cstandard_deviation", rec->outDir, FILE_SEPARATOR);
    sprintf(mpFilenameStem, "%s%crun-1", rec->outDir, FILE_SEPARATOR);
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
    if (rec->useMP == 1 || rec->useMP == 2) {
        if (IS_FAILED((ret = mpPrinter->PrintStart(mpPrinter, mpFilenameStem)))) {
            return ret;
        }
        if (IS_FAILED((ret = mpPrinter->PrintHeader(mpPrinter)))) {
            return ret;
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
        if (rec->evaluator->EvaluateWithCurrentAmounts(rec->evaluator, (KINETIC_LAW*) GetTriggerInEvent(
                rec->eventArray[i]))) {
            SetTriggerEnabledInEvent(rec->eventArray[i], TRUE);
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

    if (change)
        return CHANGE;
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
    SIMULATION_PRINTER *mpPrinter = NULL;
    SIMULATION_RUN_TERMINATION_DECIDER *decider = NULL;
    int nextEvent = 0;
    double nextEventTime = 0;
    UINT32 size = rec->speciesSize;
    UINT32 numberSteps = rec->numberSteps;
    SPECIES *species = NULL;
    SPECIES **speciesArray = rec->speciesArray;
    double end;
    double newValue;
    int useMP = rec->useMP;
    double mpRun[size];
    double distance;
    double newDistance;
    int index;
    double mpRuns[rec->runs][size];
    int n;

    printf("Size = %d\n", size);
    meanPrinter = rec->meanPrinter;
    varPrinter = rec->varPrinter;
    sdPrinter = rec->sdPrinter;
    if (useMP == 1 || useMP == 2) {
        mpPrinter = rec->mpPrinter;
    }
    rec->currentStep++;
    nextPrintTime = (rec->currentStep * rec->timeLimit) / numberSteps;
    if (IS_FAILED((ret = meanPrinter->PrintValues(meanPrinter, rec->time)))) {
        return ret;
    }
    if (useMP == 1 || useMP == 2) {
        if (IS_FAILED((ret = mpPrinter->PrintValues(mpPrinter, rec->time)))) {
            return ret;
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
    if (useMP == 1 || useMP == 2) {
        for (l = 0; l < size; l++) {
            mpRun[l] = rec->oldSpeciesMeans[l];
        }
    }
    while (rec->time < timeLimit) {
        rec->time = time;
        end = rec->time + timeStep;
        if (timeLimit < end) {
            end = timeLimit;
        }
        if (useMP == 2) {
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
            if (IS_FAILED((ret = _CalculateTotalPropensities(rec)))) {
                return ret;
            }
            if (rec->totalPropensities == 0) {
                n = timeStep;
            } else {
                n = (timeStep / rec->totalPropensities);
            }
            if ((n + time) > nextPrintTime) {
                end = nextPrintTime;
            } else {
                end = n + time;
            }
            if (timeLimit < end) {
                end = timeLimit;
            }
        }
        for (k = 1; k <= rec->runs; k++) {
            rec->time = time;
            i = 0;
            if ((rec->decider = CreateSimulationRunTerminationDecider(backend, speciesArray, rec->speciesSize,
                    rec->reactionArray, rec->reactionsSize, rec->constraintArray, rec->constraintsSize, rec->evaluator,
                    FALSE, end)) == NULL) {
                return ErrorReport(FAILING, "_RunSimulation", "could not create simulation decider");
            }
            decider = rec->decider;
            if (useMP == 1 || useMP == 2) {
                for (l = 0; l < size; l++) {
                    species = speciesArray[l];
                    newValue = mpRun[l];
                    SetAmountInSpeciesNode(species, newValue);
                }
                if (IS_FAILED((ret = _UpdateAllReactionRateUpdateTimes(rec, rec->time)))) {
                    return ret;
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
                    if (IS_FAILED((ret = _UpdateAllReactionRateUpdateTimes(rec, rec->time)))) {
                        return ret;
                    }
                } while ((decider->IsTerminationConditionMet(decider, reaction, rec->time)));
            }
            while (!(decider->IsTerminationConditionMet(decider, reaction, rec->time))) {
                i++;
                //if (timeStep == DBL_MAX) {
                maxTime = DBL_MAX;
                //} else {
                //maxTime = maxTime + timeStep;
                //}
                nextEventTime = fireEvents(rec, rec->time);
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
            }
            if (k == 1) {
                for (l = 0; l < size; l++) {
                    species = speciesArray[l];
                    rec->newSpeciesMeans[l] = GetAmountInSpeciesNode(species);
                    rec->newSpeciesVariances[l] = 0;
                    if (useMP == 1 || useMP == 2) {
                        mpRuns[k - 1][l] = GetAmountInSpeciesNode(species);
                    }
                }
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
                    if (useMP == 1 || useMP == 2) {
                        mpRuns[k - 1][l] = GetAmountInSpeciesNode(species);
                    }
                }
            }
        }
        time = end;
        for (l = 0; l < size; l++) {
            species = speciesArray[l];
            rec->oldSpeciesMeans[l] = rec->newSpeciesMeans[l];
            rec->oldSpeciesVariances[l] = rec->newSpeciesVariances[l];
            rec->speciesSD[l] = sqrt(rec->newSpeciesVariances[l]);
        }
        if (useMP == 1 || useMP == 2) {
            distance = -1;
            index = -1;
            for (k = 0; k < rec->runs; k++) {
                newDistance = 0;
                for (l = 0; l < size; l++) {
                    newDistance += pow(mpRuns[k][l] - rec->oldSpeciesMeans[l], 2);
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
        }
        if (time >= nextPrintTime && time != timeLimit) {
            rec->time = nextPrintTime;
            rec->currentStep++;
            nextPrintTime = (rec->currentStep * rec->timeLimit) / numberSteps;
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
            if (useMP == 1 || useMP == 2) {
                for (l = 0; l < size; l++) {
                    species = speciesArray[l];
                    SetAmountInSpeciesNode(species, mpRun[l]);
                }
                if (IS_FAILED((ret = mpPrinter->PrintValues(mpPrinter, rec->time)))) {
                    return ret;
                }
            }
        }
    }
    if (rec->time >= timeLimit) {
        rec->time = timeLimit;
    }
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
    if (useMP == 1 || useMP == 2) {
        for (l = 0; l < size; l++) {
            species = speciesArray[l];
            SetAmountInSpeciesNode(species, mpRun[l]);
        }
        if (IS_FAILED((ret = mpPrinter->PrintValues(mpPrinter, rec->time)))) {
            return ret;
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
    if (useMP == 1 || useMP == 2) {
        if (IS_FAILED((ret = mpPrinter->PrintEnd(mpPrinter)))) {
            return ret;
        }
    }

    if (IS_FAILED((ret = _CleanSimulation(rec)))) {
        return ret;
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
    SIMULATION_PRINTER *mpPrinter = rec->mpPrinter;
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
    if (rec->useMP == 1 || rec->useMP == 2) {
        mpPrinter->Destroy(mpPrinter);
    }
    decider->Destroy(decider);

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

    edges = GetReactantEdges((IR_NODE*) reaction);
    ResetCurrentElement(edges);
    while ((edge = GetNextEdge(edges)) != NULL) {
        stoichiometry = GetStoichiometryInIREdge(edge);
        species = GetSpeciesInIREdge(edge);
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
        stoichiometry = GetStoichiometryInIREdge( edge );
        species = GetSpeciesInIREdge( edge );
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
    propensity = evaluator->EvaluateWithCurrentAmounts(evaluator, law);
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
    SIMULATION_PRINTER *mpPrinter = rec->mpPrinter;

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
        if (rec->useMP == 1 || rec->useMP == 2) {
            if (IS_FAILED((ret = mpPrinter->PrintValues(mpPrinter, nextPrintTime)))) {
                return ret;
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

    do {
        eventFired = FALSE;
        for (i = 0; i < rec->eventsSize; i++) {
            nextEventTime = GetNextEventTimeInEvent(rec->eventArray[i]);
            triggerEnabled = GetTriggerEnabledInEvent(rec->eventArray[i]);
            if (nextEventTime != -1.0) {
                if (time >= nextEventTime) {
                    if (!GetUseValuesFromTriggerTime(rec->eventArray[i])) {
                        SetEventAssignmentsNextValues(rec->eventArray[i], rec);
                    }
                    fireEvent(rec->eventArray[i], rec);
                    SetNextEventTimeInEvent(rec->eventArray[i], -1.0);
                    eventFired = TRUE;
                } else {
                    if ((firstEventTime == -1.0) || (nextEventTime < firstEventTime)) {
                        firstEventTime = nextEventTime;
                    }
                }
            }
            nextEventTime = rec->time + rec->findNextTime->FindNextTimeWithCurrentAmounts(rec->findNextTime,
                    (KINETIC_LAW*) GetTriggerInEvent(rec->eventArray[i]));
            if ((firstEventTime == -1.0) || (nextEventTime < firstEventTime)) {
                firstEventTime = nextEventTime;
            }
            if (!triggerEnabled) {
                if (rec->evaluator->EvaluateWithCurrentAmounts(rec->evaluator, (KINETIC_LAW*) GetTriggerInEvent(
                        rec->eventArray[i]))) {
                    SetTriggerEnabledInEvent(rec->eventArray[i], TRUE);
                    if (GetDelayInEvent(rec->eventArray[i]) == NULL) {
                        deltaTime = 0;
                    } else {
                        deltaTime = rec->evaluator->EvaluateWithCurrentAmounts(rec->evaluator,
                                (KINETIC_LAW*) GetDelayInEvent(rec->eventArray[i]));
                    }
                    if (deltaTime > 0) {
                        SetNextEventTimeInEvent(rec->eventArray[i], time + deltaTime);
                        if (GetUseValuesFromTriggerTime(rec->eventArray[i])) {
                            SetEventAssignmentsNextValues(rec->eventArray[i], rec);
                        }
                        if ((firstEventTime == -1.0) || (time + deltaTime < firstEventTime)) {
                            firstEventTime = time + deltaTime;
                        }
                    } else if (deltaTime == 0) {
                        if (GetUseValuesFromTriggerTime(rec->eventArray[i])) {
                            SetEventAssignmentsNextValues(rec->eventArray[i], rec);
                        }
                        fireEvent(rec->eventArray[i], rec);
                        eventFired = TRUE;
                    } else {
                        ErrorReport(FAILING, "_Update", "delay for event evaluates to a negative number");
                        return -2;
                    }
                }
            } else {
                if (!rec->evaluator->EvaluateWithCurrentAmounts(rec->evaluator, (KINETIC_LAW*) GetTriggerInEvent(
                        rec->eventArray[i]))) {
                    SetTriggerEnabledInEvent(rec->eventArray[i], FALSE);
                }
            }
        }
        if (eventFired) {
            ExecuteAssignments(rec);
        }
    } while (eventFired);
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
        SetEventAssignmentNextValue(eventAssignment, amount);
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
        amount = GetEventAssignmentNextValue(eventAssignment);
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
            amount = rec->evaluator->EvaluateWithCurrentAmounts(rec->evaluator, (KINETIC_LAW*) GetMathInRule(
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

    for (i = 0; i < rec->rulesSize; i++) {
        if (GetRuleType(rec->ruleArray[i]) == RULE_TYPE_RATE) {
            change = rec->evaluator->EvaluateWithCurrentAmounts(rec->evaluator, (KINETIC_LAW*) GetMathInRule(
                    rec->ruleArray[i]));
            varType = GetRuleVarType(rec->ruleArray[i]);
            j = GetRuleIndex(rec->ruleArray[i]);
            if (varType == SPECIES_RULE) {
                amount = GetAmountInSpeciesNode(rec->speciesArray[j]);
                amount += (change * rec->t);
                SetRuleCurValue(rec->ruleArray[i], amount);
            } else if (varType == COMPARTMENT_RULE) {
                amount = GetCurrentSizeInCompartment(rec->compartmentArray[j]);
                amount += (change * rec->t);
                SetRuleCurValue(rec->ruleArray[i], amount);
            } else {
                amount = GetCurrentRealValueInSymbol(rec->symbolArray[j]);
                amount += (change * rec->t);
                SetRuleCurValue(rec->ruleArray[i], amount);
            }
        }
    }
    for (i = 0; i < rec->rulesSize; i++) {
        if (GetRuleType(rec->ruleArray[i]) == RULE_TYPE_RATE) {
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
            stoichiometry = GetStoichiometryInIREdge(edge);
            species = GetSpeciesInIREdge(edge);
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
            stoichiometry = GetStoichiometryInIREdge(edge);
            species = GetSpeciesInIREdge(edge);
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

