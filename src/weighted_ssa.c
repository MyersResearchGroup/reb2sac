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
#include <math.h>
#include <float.h>
#include "gsl/gsl_vector.h"
#include "gsl/gsl_multiroots.h"
#include "weighted_ssa.h"


static BOOL _IsModelConditionSatisfied(IR* ir);

static RET_VAL _InitializeRecord(WEIGHTED_MONTE_CARLO_RECORD* rec, BACK_END_PROCESSOR* backend, IR* ir);
static RET_VAL _InitializeSimulation(WEIGHTED_MONTE_CARLO_RECORD* rec, int runNum);
static RET_VAL _RunSimulation(WEIGHTED_MONTE_CARLO_RECORD* rec);

static RET_VAL _CleanSimulation(WEIGHTED_MONTE_CARLO_RECORD* rec);
static RET_VAL _CleanRecord(WEIGHTED_MONTE_CARLO_RECORD* rec);

static RET_VAL _CalculateTotalPropensities(WEIGHTED_MONTE_CARLO_RECORD* rec);
static RET_VAL _CalculatePropensities(WEIGHTED_MONTE_CARLO_RECORD* rec);
static RET_VAL _CalculatePropensity(WEIGHTED_MONTE_CARLO_RECORD* rec, REACTION* reaction);
static RET_VAL _CalculateTotalPredilections(WEIGHTED_MONTE_CARLO_RECORD* rec);
static RET_VAL _CalculatePredilections(WEIGHTED_MONTE_CARLO_RECORD* rec);
static RET_VAL _CalculatePredilection(WEIGHTED_MONTE_CARLO_RECORD* rec, REACTION* reaction);
static RET_VAL _FindNextReactionTime(WEIGHTED_MONTE_CARLO_RECORD* rec);
static RET_VAL _FindNextReaction(WEIGHTED_MONTE_CARLO_RECORD* rec);
static RET_VAL _UpdateWeight(WEIGHTED_MONTE_CARLO_RECORD* rec);
static RET_VAL _Update(WEIGHTED_MONTE_CARLO_RECORD* rec);
static RET_VAL _Print(WEIGHTED_MONTE_CARLO_RECORD* rec);
static RET_VAL _PrintStatistics(WEIGHTED_MONTE_CARLO_RECORD* rec, FILE* file);
static RET_VAL _UpdateNodeValues(WEIGHTED_MONTE_CARLO_RECORD* rec);
static RET_VAL _UpdateSpeciesValues(WEIGHTED_MONTE_CARLO_RECORD* rec);
static RET_VAL _UpdateReactionRateUpdateTime(WEIGHTED_MONTE_CARLO_RECORD* rec);
static RET_VAL _UpdateReactionRateUpdateTimeForSpecies(WEIGHTED_MONTE_CARLO_RECORD* rec, SPECIES* species);
static RET_VAL _UpdateAllReactionRateUpdateTimes(WEIGHTED_MONTE_CARLO_RECORD* rec, double time);

static int _ComparePropensity(REACTION* a, REACTION* b);
static BOOL _IsTerminationConditionMet(WEIGHTED_MONTE_CARLO_RECORD* rec);

static double fireEvents(WEIGHTED_MONTE_CARLO_RECORD* rec, double time);
static void fireEvent(EVENT* event, WEIGHTED_MONTE_CARLO_RECORD* rec);
static void ExecuteAssignments(WEIGHTED_MONTE_CARLO_RECORD* rec);
static void SetEventAssignmentsNextValues(EVENT* event, WEIGHTED_MONTE_CARLO_RECORD* rec);
static void SetEventAssignmentsNextValuesTime(EVENT* event, WEIGHTED_MONTE_CARLO_RECORD* rec, double time);
static RET_VAL EvaluateAlgebraicRules(WEIGHTED_MONTE_CARLO_RECORD* rec);
static RET_VAL ExecuteFastReactions(WEIGHTED_MONTE_CARLO_RECORD* rec);

DLLSCOPE RET_VAL STDCALL DoWeightedMonteCarloAnalysis(BACK_END_PROCESSOR* backend, IR* ir) {
    RET_VAL ret = SUCCESS;
    UINT i = 0;
    UINT runs = 1;
    char* namePrefix = NULL;
    static WEIGHTED_MONTE_CARLO_RECORD rec;
    UINT timeout = 0;
    rec.weightSum = 0.0;
    rec.squareWeightSum = 0.0;
    //double removeLater = 0.0;
    double zstar = 1.64485;
    double stddev = 0.0;

    START_FUNCTION("DoWeightedMonteCarloAnalysis");

    if (!_IsModelConditionSatisfied(ir)) {
        return ErrorReport(FAILING, "DoWeightedMonteCarloAnalysis", " method cannot be applied to the model");
    }

    if (IS_FAILED((ret = _InitializeRecord(&rec, backend, ir)))) {
        return ErrorReport(ret, "DoWeightedMonteCarloAnalysis", "initialization of the record failed");
    }


    runs = rec.runs;
    for (i = 1; i <= runs; i++) {
        SeedRandomNumberGenerators(rec.seed);
        rec.seed = GetNextUniformRandomNumber(0, RAND_MAX);
        timeout = 0;
        do {
            SeedRandomNumberGenerators(rec.seed);
            if (IS_FAILED((ret = _InitializeSimulation(&rec, i)))) {
                return ErrorReport(ret, "DoWeightedMonteCarloAnalysis", "initialization of the %i-th simulation failed", i);
            }
            timeout++;
        } while ((ret == CHANGE) && (timeout <= (rec.speciesSize + rec.compartmentsSize + rec.symbolsSize)));
        if (timeout > (rec.speciesSize + rec.compartmentsSize + rec.symbolsSize + 1)) {
            return ErrorReport(ret, "DoWeightedMonteCarloAnalysis", "Cycle detected in initial and rule assignments");
        }
        if (IS_FAILED((ret = _RunSimulation(&rec)))) {
            return ErrorReport(ret, "DoWeightedMonteCarloAnalysis", "%i-th simulation failed at time %f", i, rec.time);
        }
        if (IS_FAILED((ret = _CleanSimulation(&rec)))) {
            return ErrorReport(ret, "DoWeightedMonteCarloAnalysis", "cleaning of the %i-th simulation failed", i);
        }
        printf("Run = %d\n", i);
        //printf("q/n = %.10f\n", rec.weightSum / i);
        //if (rec.weightSum > removeLater) {
        //    printf("Error State Achieved\n");
        //}
        //removeLater = rec.weightSum;
        fflush(stdout);
    }
    stddev = sqrt(rec.squareWeightSum / runs - rec.weightSum / runs * rec.weightSum / runs);
    printf("Mean = %g\n", rec.weightSum / runs);
    printf("90 Percent Confidence Interval: [%g,%g]\n", rec.weightSum / runs - zstar*stddev, rec.weightSum / runs + zstar * stddev);
    //printf("second moment=%g\n", rec.squareWeightSum / runs);
    END_FUNCTION("DoWeightedMonteCarloAnalysis", SUCCESS);
    return ret;
}

DLLSCOPE RET_VAL STDCALL CloseWeightedMonteCarloAnalyzer(BACK_END_PROCESSOR* backend) {
    RET_VAL ret = SUCCESS;
    WEIGHTED_MONTE_CARLO_RECORD* rec = (WEIGHTED_MONTE_CARLO_RECORD*)(backend->_internal1);

    START_FUNCTION("CloseWeightedMonteCarloAnalyzer");

    if (IS_FAILED((ret = _CleanRecord(rec)))) {
        return ErrorReport(ret, "CloseWeightedMonteCarloAnalyzer", "cleaning of the record failed");
    }

    END_FUNCTION("CloseWeightedMonteCarloAnalyzer", SUCCESS);
    return ret;
}

static BOOL _IsModelConditionSatisfied(IR* ir) {
    REACTION* reaction = NULL;
    LINKED_LIST* reactions = NULL;

    reactions = ir->GetListOfReactionNodes(ir);
    while ((reaction = (REACTION*)GetNextFromLinkedList(reactions)) != NULL) {
        if (IsReactionReversibleInReactionNode(reaction)) {
            TRACE_0("the input model contains reversible reaction(s), and cannot be used for stochastic simulation method");
            return FALSE;
        }
    }
    return TRUE;
}

static RET_VAL _InitializeRecord(WEIGHTED_MONTE_CARLO_RECORD* rec, BACK_END_PROCESSOR* backend, IR* ir) {
    RET_VAL ret = SUCCESS;
    UINT32 i = 0;
    UINT32 j = 0;
    UINT32 size = 0;
    UINT32 algebraicVars = 0;
    double printInterval = 0.0;
    double minPrintInterval = 0.0;
    char buf[512];
    char* valueString = NULL;
    SPECIES* species = NULL;
    SPECIES** speciesArray = NULL;
    REACTION* reaction = NULL;
    REACTION** reactions = NULL;
    COMPARTMENT* compartment = NULL;
    COMPARTMENT** compartmentArray = NULL;
    COMPARTMENT_MANAGER* compartmentManager;
    REB2SAC_SYMBOL* symbol = NULL;
    REB2SAC_SYMBOL** symbolArray = NULL;
    REB2SAC_SYMTAB* symTab;
    RULE* rule = NULL;
    RULE** ruleArray = NULL;
    RULE_MANAGER* ruleManager;
    CONSTRAINT* constraint = NULL;
    CONSTRAINT** constraintArray = NULL;
    CONSTRAINT_MANAGER* constraintManager;
    EVENT* event = NULL;
    EVENT** eventArray = NULL;
    EVENT_MANAGER* eventManager;
    EVENT_ASSIGNMENT* eventAssignment;
    COMPILER_RECORD_T* compRec = backend->record;
    LINKED_LIST* list = NULL;
    REB2SAC_PROPERTIES* properties = NULL;

#if GET_SEED_FROM_COMMAND_LINE
    PROPERTIES* options = NULL;
#endif

    rec->encoding = backend->encoding;
    list = ir->GetListOfReactionNodes(ir);
    rec->reactionsSize = GetLinkedListSize(list);
    rec->numberFastReactions = 0;
    if (rec->reactionsSize != 0) {
        if ((reactions = (REACTION**)MALLOC(rec->reactionsSize * sizeof(REACTION*))) == NULL) {
            return ErrorReport(FAILING, "_InitializeRecord", "could not allocate memory for reaction array");
        }
        i = 0;
        ResetCurrentElement(list);
        while ((reaction = (REACTION*)GetNextFromLinkedList(list)) != NULL) {
            reactions[i] = reaction;
            i++;
            if (IsReactionFastInReactionNode(reaction)) {
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

    if (rec->numberFastReactions > 1) {
        return ErrorReport(FAILING, "_InitializeRecord",
            "Simulator supports only a single fast reaction");
    }

    if ((ruleManager = ir->GetRuleManager(ir)) == NULL) {
        return ErrorReport(FAILING, "_InitializeRecord", "could not get the rule manager");
    }
    list = ruleManager->CreateListOfRules(ruleManager);
    rec->rulesSize = GetLinkedListSize(list);
    if (rec->rulesSize > 0) {
        if ((ruleArray = (RULE**)MALLOC(rec->rulesSize * sizeof(RULE*))) == NULL) {
            return ErrorReport(FAILING, "_InitializeRecord", "could not allocate memory for rules array");
        }
    }
    i = 0;
    rec->algebraicRulesSize = 0;
    ResetCurrentElement(list);
    while ((rule = (RULE*)GetNextFromLinkedList(list)) != NULL) {
        ruleArray[i] = rule;
        i++;
        if (GetRuleType(rule) == RULE_TYPE_ALGEBRAIC) {
            rec->algebraicRulesSize++;
        }
    }
    rec->ruleArray = ruleArray;

    if ((symTab = ir->GetGlobalSymtab(ir)) == NULL) {
        return ErrorReport(FAILING, "_InitializeRecord", "could not get the symbol table");
    }
    list = symTab->GenerateListOfSymbols(symTab);
    rec->symbolsSize = GetLinkedListSize(list);
    if (rec->symbolsSize > 0) {
        if ((symbolArray = (REB2SAC_SYMBOL**)MALLOC(rec->symbolsSize * sizeof(REB2SAC_SYMBOL*))) == NULL) {
            return ErrorReport(FAILING, "_InitializeRecord", "could not allocate memory for symbols array");
        }
    }
    i = 0;
    ResetCurrentElement(list);
    while ((symbol = (REB2SAC_SYMBOL*)GetNextFromLinkedList(list)) != NULL) {
        symbolArray[i] = symbol;
        if (IsSymbolAlgebraic(symbol)) {
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
        if ((compartmentArray = (COMPARTMENT**)MALLOC(rec->compartmentsSize * sizeof(RULE*))) == NULL) {
            return ErrorReport(FAILING, "_InitializeRecord", "could not allocate memory for compartment array");
        }
    }
    i = 0;
    ResetCurrentElement(list);
    while ((compartment = (COMPARTMENT*)GetNextFromLinkedList(list)) != NULL) {
        compartmentArray[i] = compartment;
        if (IsCompartmentAlgebraic(compartment)) {
            algebraicVars++;
        }
        i++;
    }
    rec->compartmentArray = compartmentArray;

    list = ir->GetListOfSpeciesNodes(ir);
    rec->speciesSize = GetLinkedListSize(list);
    if (rec->speciesSize > 0) {
        if ((speciesArray = (SPECIES**)MALLOC(rec->speciesSize * sizeof(SPECIES*))) == NULL) {
            return ErrorReport(FAILING, "_InitializeRecord", "could not allocate memory for species array");
        }
    }

    properties = compRec->properties;

    i = 0;
    rec->numberFastSpecies = 0;
    ResetCurrentElement(list);
    while ((species = (SPECIES*)GetNextFromLinkedList(list)) != NULL) {
        speciesArray[i] = species;
        if (IsSpeciesNodeAlgebraic(species)) {
            algebraicVars++;
        }
        if (IsSpeciesNodeFast(species)) {
            rec->numberFastSpecies++;
        }
        i++;
    }
    if (rec->numberFastSpecies > 0) {
        if ((rec->fastCons = (double*)MALLOC(rec->numberFastSpecies * sizeof(double))) == NULL) {
            return ErrorReport(FAILING, "_InitializeRecord", "could not allocate memory for fastCons array");
        }
    }
    rec->speciesArray = speciesArray;
    if (algebraicVars > rec->algebraicRulesSize) {
        return ErrorReport(FAILING, "_InitializeRecord", "model underdetermined");
    }
    else if (algebraicVars < rec->algebraicRulesSize) {
        return ErrorReport(FAILING, "_InitializeRecord", "model overdetermined");
    }

    for (i = 0; i < rec->rulesSize; i++) {
        if (GetRuleType(rec->ruleArray[i]) == RULE_TYPE_ASSIGNMENT ||
            GetRuleType(rec->ruleArray[i]) == RULE_TYPE_RATE_ASSIGNMENT) {
            for (j = 0; j < rec->speciesSize; j++) {
                if (strcmp(GetCharArrayOfString(GetRuleVar(rec->ruleArray[i])),
                    GetCharArrayOfString(GetSpeciesNodeID(rec->speciesArray[j]))) == 0) {
                    SetRuleVarType(ruleArray[i], SPECIES_RULE);
                    SetRuleIndex(ruleArray[i], j);
                    break;
                }
            }
            for (j = 0; j < rec->compartmentsSize; j++) {
                if (strcmp(GetCharArrayOfString(GetRuleVar(rec->ruleArray[i])),
                    GetCharArrayOfString(GetCompartmentID(rec->compartmentArray[j]))) == 0) {
                    SetRuleVarType(ruleArray[i], COMPARTMENT_RULE);
                    SetRuleIndex(ruleArray[i], j);
                    break;
                }
            }
            for (j = 0; j < rec->symbolsSize; j++) {
                if (strcmp(GetCharArrayOfString(GetRuleVar(rec->ruleArray[i])),
                    GetCharArrayOfString(GetSymbolID(rec->symbolArray[j]))) == 0) {
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
    }
    else {
        if (IS_FAILED((ret = StrToUINT32((UINT32*)&(rec->startIndex), valueString)))) {
            rec->startIndex = DEFAULT_MONTE_CARLO_SIMULATION_START_INDEX;
        }
    }

    if ((valueString = properties->GetProperty(properties, SIMULATION_OUTPUT_START_TIME)) == NULL) {
        rec->outputStartTime = DEFAULT_SIMULATION_OUTPUT_START_TIME_VALUE;
    }
    else {
        if (IS_FAILED((ret = StrToFloat(&(rec->outputStartTime), valueString)))) {
            rec->outputStartTime = DEFAULT_SIMULATION_OUTPUT_START_TIME_VALUE;
        }
    }

    if ((valueString = properties->GetProperty(properties, SIMULATION_INITIAL_TIME)) == NULL) {
        rec->initialTime = DEFAULT_SIMULATION_INITIAL_TIME_VALUE;
    }
    else {
        if (IS_FAILED((ret = StrToFloat(&(rec->initialTime), valueString)))) {
            rec->initialTime = DEFAULT_SIMULATION_INITIAL_TIME_VALUE;
        }
    }

    if ((valueString = properties->GetProperty(properties, MONTE_CARLO_SIMULATION_TIME_LIMIT)) == NULL) {
        rec->timeLimit = DEFAULT_MONTE_CARLO_SIMULATION_TIME_LIMIT_VALUE;
    }
    else {
        if (IS_FAILED((ret = StrToFloat(&(rec->timeLimit), valueString)))) {
            rec->timeLimit = DEFAULT_MONTE_CARLO_SIMULATION_TIME_LIMIT_VALUE;
        }
    }

    if ((valueString = properties->GetProperty(properties, MONTE_CARLO_SIMULATION_TIME_STEP)) == NULL) {
        rec->timeStep = DEFAULT_MONTE_CARLO_SIMULATION_TIME_STEP;
    }
    else {
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
            }
            else {
                if (IS_FAILED((ret = StrToFloat(&(minPrintInterval), valueString)))) {
                    rec->numberSteps = DEFAULT_MONTE_CARLO_SIMULATION_NUMBER_STEPS_VALUE;
                }
                else {
                    rec->minPrintInterval = minPrintInterval;
                }
            }
        }
        else {
            if (IS_FAILED((ret = StrToUINT32(&(rec->numberSteps), valueString)))) {
                rec->numberSteps = DEFAULT_MONTE_CARLO_SIMULATION_NUMBER_STEPS_VALUE;
            }
        }
    }
    else {
        if (IS_FAILED((ret = StrToFloat(&(printInterval), valueString)))) {
            rec->numberSteps = DEFAULT_MONTE_CARLO_SIMULATION_NUMBER_STEPS_VALUE;
        }
        else {
            rec->numberSteps = rec->timeLimit / printInterval;
        }
    }

#if GET_SEED_FROM_COMMAND_LINE
    options = compRec->options;
    if ((valueString = options->GetProperty(options, "random.seed")) == NULL) {
        rec->seed = DEFAULT_MONTE_CARLO_SIMULATION_RANDOM_SEED_VALUE;
    }
    else {
        if (IS_FAILED((ret = StrToUINT32(&(rec->seed), valueString)))) {
            rec->seed = DEFAULT_MONTE_CARLO_SIMULATION_RANDOM_SEED_VALUE;
        }
        TRACE_1("seed from command line is %i", rec->seed);
    }
#else
    if ((valueString = properties->GetProperty(properties, MONTE_CARLO_SIMULATION_RANDOM_SEED)) == NULL) {
        rec->seed = DEFAULT_MONTE_CARLO_SIMULATION_RANDOM_SEED_VALUE;
    }
    else {
        if (IS_FAILED((ret = StrToUINT32(&(rec->seed), valueString)))) {
            rec->seed = DEFAULT_MONTE_CARLO_SIMULATION_RANDOM_SEED_VALUE;
        }
    }
#endif

    if ((valueString = properties->GetProperty(properties, MONTE_CARLO_SIMULATION_RUNS)) == NULL) {
        rec->runs = DEFAULT_MONTE_CARLO_SIMULATION_RUNS_VALUE;
    }
    else {
        if (IS_FAILED((ret = StrToUINT32(&(rec->runs), valueString)))) {
            rec->runs = DEFAULT_MONTE_CARLO_SIMULATION_RUNS_VALUE;
        }
    }

    if ((rec->outDir = properties->GetProperty(properties, MONTE_CARLO_SIMULATION_OUT_DIR)) == NULL) {
        rec->outDir = DEFAULT_MONTE_CARLO_SIMULATION_OUT_DIR_VALUE;
    }

    if ((rec->printer = CreateSimulationPrinter(backend, compartmentArray, rec->compartmentsSize,
        speciesArray, rec->speciesSize,
        symbolArray, rec->symbolsSize)) == NULL) {
        return ErrorReport(FAILING, "_InitializeRecord", "could not create simulation printer");
    }

    if ((constraintManager = ir->GetConstraintManager(ir)) == NULL) {
        return ErrorReport(FAILING, "_InitializeRecord", "could not get the constraint manager");
    }
    list = constraintManager->CreateListOfConstraints(constraintManager);
    rec->constraintsSize = GetLinkedListSize(list);
    if (rec->constraintsSize > 0) {
        if ((constraintArray = (CONSTRAINT**)MALLOC(rec->constraintsSize * sizeof(CONSTRAINT*))) == NULL) {
            return ErrorReport(FAILING, "_InitializeRecord", "could not allocate memory for constraints array");
        }
    }
    i = 0;
    ResetCurrentElement(list);
    while ((constraint = (CONSTRAINT*)GetNextFromLinkedList(list)) != NULL) {
        constraintArray[i] = constraint;
        i++;
    }
    rec->constraintArray = constraintArray;

    if ((rec->decider =
        CreateSimulationRunTerminationDecider(backend, speciesArray, rec->speciesSize, reactions, rec->reactionsSize,
            rec->constraintArray, rec->constraintsSize, rec->evaluator, FALSE, rec->timeLimit)) == NULL) {
        return ErrorReport(FAILING, "_InitializeRecord", "could not create simulation decider");
    }

    if ((eventManager = ir->GetEventManager(ir)) == NULL) {
        return ErrorReport(FAILING, "_InitializeRecord", "could not get the event manager");
    }
    list = eventManager->CreateListOfEvents(eventManager);
    rec->eventsSize = GetLinkedListSize(list);
    if (rec->eventsSize > 0) {
        if ((eventArray = (EVENT**)MALLOC(rec->eventsSize * sizeof(EVENT*))) == NULL) {
            return ErrorReport(FAILING, "_InitializeRecord", "could not allocate memory for events array");
        }
    }
    i = 0;
    ResetCurrentElement(list);
    while ((event = (EVENT*)GetNextFromLinkedList(list)) != NULL) {
        eventArray[i] = event;
        i++;
    }
    rec->eventArray = eventArray;

    for (i = 0; i < rec->eventsSize; i++) {
        list = GetEventAssignments(rec->eventArray[i]);
        ResetCurrentElement(list);
        while ((eventAssignment = (EVENT_ASSIGNMENT*)GetNextFromLinkedList(list)) != NULL) {
            for (j = 0; j < rec->speciesSize; j++) {
                if (strcmp(GetCharArrayOfString(eventAssignment->var),
                    GetCharArrayOfString(GetSpeciesNodeID(rec->speciesArray[j]))) == 0) {
                    SetEventAssignmentVarType(eventAssignment, SPECIES_EVENT_ASSIGNMENT);
                    SetEventAssignmentIndex(eventAssignment, j);
                    break;
                }
            }
            for (j = 0; j < rec->compartmentsSize; j++) {
                if (strcmp(GetCharArrayOfString(eventAssignment->var),
                    GetCharArrayOfString(GetCompartmentID(rec->compartmentArray[j]))) == 0) {
                    SetEventAssignmentVarType(eventAssignment, COMPARTMENT_EVENT_ASSIGNMENT);
                    SetEventAssignmentIndex(eventAssignment, j);
                    break;
                }
            }
            for (j = 0; j < rec->symbolsSize; j++) {
                if (strcmp(GetCharArrayOfString(eventAssignment->var),
                    GetCharArrayOfString(GetSymbolID(rec->symbolArray[j]))) == 0) {
                    SetEventAssignmentVarType(eventAssignment, PARAMETER_EVENT_ASSIGNMENT);
                    SetEventAssignmentIndex(eventAssignment, j);
                    break;
                }
            }
        }
    }

    backend->_internal1 = (CADDR_T)rec;

    return ret;
}

static RET_VAL _InitializeSimulation(WEIGHTED_MONTE_CARLO_RECORD* rec, int runNum) {
    RET_VAL ret = SUCCESS;
    char filenameStem[512];
    double amount = 0;
    double param = 0;
    UINT32 i = 0;
    UINT32 size = 0;
    SPECIES* species = NULL;
    SPECIES** speciesArray = rec->speciesArray;
    REACTION* reaction = NULL;
    REACTION** reactionArray = rec->reactionArray;
    KINETIC_LAW_EVALUATER* evaluator = rec->evaluator;
    SIMULATION_PRINTER* printer = rec->printer;
    double compSize = 0.0;
    COMPARTMENT* compartment = NULL;
    COMPARTMENT** compartmentArray = rec->compartmentArray;
    REB2SAC_SYMBOL* symbol = NULL;
    REB2SAC_SYMBOL** symbolArray = rec->symbolArray;
    KINETIC_LAW* law = NULL;
    BOOL change = FALSE;
    char filename[512];
    FILE* file = NULL;

    sprintf(filenameStem, "%s%crun-%i", rec->outDir, FILE_SEPARATOR, (runNum + rec->startIndex - 1));
    if (IS_FAILED((ret = printer->PrintStart(printer, filenameStem)))) {
        return ret;
    }
    if (IS_FAILED((ret = printer->PrintHeader(printer)))) {
        return ret;
    }
    rec->time = rec->initialTime;
    rec->nextPrintTime = rec->outputStartTime;
    rec->currentStep = 0;
    size = rec->compartmentsSize;
    for (i = 0; i < size; i++) {
        compartment = compartmentArray[i];
        if ((law = (KINETIC_LAW*)GetInitialAssignmentInCompartment(compartment)) == NULL) {
            compSize = GetSizeInCompartment(compartment);
        }
        else {
            law = CloneKineticLaw(law);
            SimplifyInitialAssignment(law);
            if (law->valueType == KINETIC_LAW_VALUE_TYPE_REAL) {
                compSize = GetRealValueFromKineticLaw(law);
            }
            else if (law->valueType == KINETIC_LAW_VALUE_TYPE_INT) {
                compSize = (double)GetIntValueFromKineticLaw(law);
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
        if ((law = (KINETIC_LAW*)GetInitialAssignmentInSpeciesNode(species)) == NULL) {
            if (IsInitialQuantityInAmountInSpeciesNode(species)) {
                amount = GetInitialAmountInSpeciesNode(species);
            }
            else {
                amount = GetInitialAmountInSpeciesNode(species);
            }
        }
        else {
            law = CloneKineticLaw(law);
            SimplifyInitialAssignment(law);
            if (law->valueType == KINETIC_LAW_VALUE_TYPE_REAL) {
                amount = GetRealValueFromKineticLaw(law);
            }
            else if (law->valueType == KINETIC_LAW_VALUE_TYPE_INT) {
                amount = (double)GetIntValueFromKineticLaw(law);
            }
            if (IsInitialQuantityInAmountInSpeciesNode(species)) {
                if (GetInitialAmountInSpeciesNode(species) != amount) {
                    SetInitialAmountInSpeciesNode(species, amount);
                    change = TRUE;
                }
            }
            else {
                if (GetInitialAmountInSpeciesNode(species) != amount) {
                    SetInitialAmountInSpeciesNode(species, amount);
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
        if ((law = (KINETIC_LAW*)GetInitialAssignmentInSymbol(symbol)) == NULL) {
            param = GetRealValueInSymbol(symbol);
            if ((strcmp(GetCharArrayOfString(GetSymbolID(symbol)), "time") == 0) ||
                (strcmp(GetCharArrayOfString(GetSymbolID(symbol)), "t") == 0)) {
                param = rec->time;
            }
        }
        else {
            law = CloneKineticLaw(law);
            SimplifyInitialAssignment(law);
            if (law->valueType == KINETIC_LAW_VALUE_TYPE_REAL) {
                param = GetRealValueFromKineticLaw(law);
            }
            else if (law->valueType == KINETIC_LAW_VALUE_TYPE_INT) {
                param = (double)GetIntValueFromKineticLaw(law);
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
        if (rec->evaluator->EvaluateWithCurrentAmountsDeter(rec->evaluator,
            (KINETIC_LAW*)GetTriggerInEvent(rec->eventArray[i]))) {
            if (GetTriggerInitialValue(rec->eventArray[i])) {
                SetTriggerEnabledInEvent(rec->eventArray[i], TRUE);
            }
            else {
                SetTriggerEnabledInEvent(rec->eventArray[i], FALSE);
            }
        }
        else {
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
        EvaluateAlgebraicRules(rec);
    }
    if (rec->numberFastSpecies > 0) {
        ExecuteFastReactions(rec);
    }

    sprintf(filename, "%s%cstatistics.txt", rec->outDir, FILE_SEPARATOR);
    if ((file = fopen(filename, "w")) == NULL) {
        return ErrorReport(FAILING, "_InitializeSimulation", "could not create a statistics file");
    }
    if (IS_FAILED((ret = _PrintStatistics(rec, file)))) {
        return ret;
    }
    fclose(file);

    if (change)
        return CHANGE;
    return ret;
}

static RET_VAL _RunSimulation(WEIGHTED_MONTE_CARLO_RECORD* rec) {
    RET_VAL ret = SUCCESS;
    int i = 0;
    int j = 0;
    double timeLimit = rec->timeLimit;
    double timeStep = rec->timeStep;
    double maxTime = 0.0;
    double nextPrintTime = rec->time;
    REACTION* reaction = NULL;
    SIMULATION_PRINTER* printer = NULL;
    SIMULATION_RUN_TERMINATION_DECIDER* decider = NULL;
    int nextEvent = 0;
    double nextEventTime = 0;
    rec->weight = 1.0;

    printer = rec->printer;
    decider = rec->decider;
    while (!(decider->IsTerminationConditionMet(decider, reaction, rec->time))) {
        i++;
        if (timeStep == DBL_MAX) {
            maxTime = DBL_MAX;
        }
        else {
            maxTime = maxTime + timeStep;
        }
        nextEventTime = fireEvents(rec, rec->time);
        if (rec->algebraicRulesSize > 0) {
            EvaluateAlgebraicRules(rec);
        }
        if (rec->numberFastSpecies > 0) {
            ExecuteFastReactions(rec);
        }
        if (decider->IsTerminationConditionMet(decider, reaction, rec->time)) break;
        if (nextEventTime == -2.0) {
            return FAILING;
        }
        if ((nextEventTime != -1) && (nextEventTime < maxTime) /*&& (nextEventTime >= rec->time)*/) {
            maxTime = nextEventTime;
        }
        if (IS_FAILED((ret = _CalculatePropensities(rec)))) {
            return ret;
        }
        if (IS_FAILED((ret = _CalculateTotalPropensities(rec)))) {
            return ret;
        }
        if (IS_FAILED((ret = _CalculatePredilections(rec)))) {
            return ret;
        }
        if (IS_FAILED((ret = _CalculateTotalPredilections(rec)))) {
            return ret;
        }
        if (IS_REAL_EQUAL(rec->totalPropensities, 0.0)) {
            TRACE_1("the total propensity is 0 at iteration %i", i);
            rec->t = maxTime - rec->time; //timeLimit - rec->time;
            if (rec->t > timeLimit) rec->t = timeLimit;
            rec->time = maxTime; //timeLimit;
            reaction = NULL;
            rec->nextReaction = NULL;
            if (IS_FAILED((ret = _Update(rec)))) {
                return ret;
            }
            if (IS_FAILED((ret = _Print(rec)))) {
                return ret;
            }
        }
        else {
            if (IS_FAILED((ret = _FindNextReactionTime(rec)))) {
                return ret;
            }
            if (maxTime < rec->time) {
                rec->time -= rec->t;
                rec->t = maxTime - rec->time;
                if (rec->t > timeLimit) rec->t = timeLimit;
                rec->time = maxTime;
                reaction = NULL;
                rec->nextReaction = NULL;
                if (IS_FAILED((ret = _Update(rec)))) {
                    return ret;
                }
                if (IS_FAILED((ret = _Print(rec)))) {
                    return ret;
                }
            }
            else {
                maxTime = rec->time;
                if (rec->time < timeLimit) {
                    if (IS_FAILED((ret = _FindNextReaction(rec)))) {
                        return ret;
                    }
                    if (IS_FAILED((ret = _UpdateWeight(rec)))) {
                        return ret;
                    }
                    reaction = rec->nextReaction;
                    if (IS_FAILED((ret = _Update(rec)))) {
                        return ret;
                    }
                }
                else {
                    if (IS_FAILED((ret = _Print(rec)))) {
                        return ret;
                    }
                }
            }
        }
    }
    if (rec->time >= timeLimit) {
        rec->time = timeLimit;
    } else {
      rec->weightSum += rec->weight;
      rec->squareWeightSum += (rec->weight) * (rec->weight);
    }
    nextEventTime = fireEvents(rec, rec->time);
    if (IS_FAILED((ret = printer->PrintValues(printer, rec->time)))) {
        return ret;
    }
    if (IS_FAILED((ret = printer->PrintEnd(printer)))) {
        return ret;
    }

    if (IS_FAILED((ret = _CleanSimulation(rec)))) {
        return ret;
    }

    return ret;
}

static RET_VAL _CleanSimulation(WEIGHTED_MONTE_CARLO_RECORD* rec) {
    RET_VAL ret = SUCCESS;
    /* something goes wrong if I put fclose here */

#if 0
    if (0 != fclose(rec->out)) {
        TRACE_0("file close error");

        rec->out = NULL;
    }
#endif
    return ret;
}

static RET_VAL _CleanRecord(WEIGHTED_MONTE_CARLO_RECORD* rec) {
    RET_VAL ret = SUCCESS;
    char filename[512];
    FILE* file = NULL;
    SIMULATION_PRINTER* printer = rec->printer;
    SIMULATION_RUN_TERMINATION_DECIDER* decider = rec->decider;

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
    if (rec->findNextTime != NULL) {
        FreeKineticLawFind_Next_Time(&(rec->findNextTime));
    }
    if (rec->reactionArray != NULL) {
        FREE(rec->reactionArray);
    }
    if (rec->speciesArray != NULL) {
        FREE(rec->speciesArray);
    }

    printer->Destroy(printer);
    decider->Destroy(decider);

    return ret;
}

static RET_VAL _PrintStatistics(WEIGHTED_MONTE_CARLO_RECORD* rec, FILE* file) {
    RET_VAL ret = SUCCESS;
    double stoichiometry = 0;
    UINT32 i = 0;
    UINT32 j = 0;
    UINT32 reactionsSize = rec->reactionsSize;
    UINT32 speciesSize = rec->speciesSize;
    UINT32 symbolsSize = rec->symbolsSize;
    REACTION* reaction = NULL;
    REACTION** reactionArray = rec->reactionArray;
    SPECIES* species = NULL;
    SPECIES** speciesArray = rec->speciesArray;
    REB2SAC_SYMBOL* symbol = NULL;
    REB2SAC_SYMBOL** symbolArray = rec->symbolArray;
    REB2SAC_SYMBOL* speciesRef = NULL;
    REB2SAC_SYMBOL* convFactor = NULL;
    IR_EDGE* edge = NULL;
    LINKED_LIST* edges = NULL;

    if ((speciesSize <= 0) || (reactionsSize <= 0)) return ret;

    fprintf(file, "Parameter Values:" NEW_LINE);

    for (i = 0; i < symbolsSize; i++) {
        symbol = symbolArray[i];
        if (IsRealValueSymbol(symbol)) {
          fprintf(file, "%s = %f" NEW_LINE, GetCharArrayOfString(GetSymbolID(symbol)), GetRealValueInSymbol(symbol));
        }
    }
    fprintf(file, NEW_LINE);

    fprintf(file, "Initial State Vector:" NEW_LINE);

    for (i = 0; i < speciesSize; i++) {
        species = speciesArray[i];
        SetAmountInSpeciesNode(species, GetInitialAmountInSpeciesNode(species));
        fprintf(file, "%s = %f" NEW_LINE, *GetSpeciesNodeID(species), GetInitialAmountInSpeciesNode(species));
    }
    fprintf(file, NEW_LINE);

    gsl_matrix* delta_matrix = gsl_matrix_alloc(speciesSize, reactionsSize);
    gsl_matrix* reactant_matrix = gsl_matrix_alloc(speciesSize, reactionsSize);

    for (i = 0; i < reactionsSize; i++) {
        for (j = 0; j < speciesSize; j++) {
            gsl_matrix_set(delta_matrix, j, i, 0);
            gsl_matrix_set(reactant_matrix, j, i, 0);
        }
    }

    if (IS_FAILED((ret = _UpdateAllReactionRateUpdateTimes(rec, rec->time)))) {
        return ret;
    }
    if (IS_FAILED((ret = _CalculatePropensities(rec)))) {
        return ret;
    }

    fprintf(file, "Initial Reaction Rate Array:" NEW_LINE);

    for (i = 0; i < reactionsSize; i++) {
        reaction = reactionArray[i];
        fprintf(file, "%f" NEW_LINE, GetReactionRate(reaction));
        edges = GetReactantEdges((IR_NODE*)reaction);
        ResetCurrentElement(edges);
        while ((edge = GetNextEdge(edges)) != NULL) {
            speciesRef = GetSpeciesRefInIREdge(edge);
            if (speciesRef) {
                stoichiometry = GetCurrentRealValueInSymbol(speciesRef);
            }
            else {
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
                    gsl_matrix_set(reactant_matrix, j, i, gsl_matrix_get(reactant_matrix, j, i) + stoichiometry);
                }
            }
        }
        edges = GetProductEdges((IR_NODE*)reaction);
        ResetCurrentElement(edges);
        while ((edge = GetNextEdge(edges)) != NULL) {
            speciesRef = GetSpeciesRefInIREdge(edge);
            if (speciesRef) {
                stoichiometry = GetCurrentRealValueInSymbol(speciesRef);
            }
            else {
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
    fprintf(file, NEW_LINE);

    fprintf(file, "Reaction Rate Equation Array:" NEW_LINE);

    for (i = 0; i < reactionsSize; i++) {
        reaction = reactionArray[i];
        fprintf(file, "%s" NEW_LINE, *ToStringKineticLaw(GetKineticLawInReactionNode(reaction)));
    }
    fprintf(file, NEW_LINE);

    fprintf(file, "Reactant Matrix:" NEW_LINE);
    for (i = 0; i < reactionsSize; i++) {
        for (j = 0; j < speciesSize; j++) {
            fprintf(file, "%f ", gsl_matrix_get(reactant_matrix, j, i));
        }
        fprintf(file, NEW_LINE);
    }

    fprintf(file, NEW_LINE);

    fprintf(file, "Delta Matrix:" NEW_LINE);
    for (i = 0; i < reactionsSize; i++) {
        for (j = 0; j < speciesSize; j++) {
            fprintf(file, "%f ", gsl_matrix_get(delta_matrix, j, i));
        }
        fprintf(file, NEW_LINE);
    }

    return ret;
}

static BOOL _IsTerminationConditionMet(WEIGHTED_MONTE_CARLO_RECORD* rec) {
    return FALSE;
}

static RET_VAL _CalculateTotalPredilections(WEIGHTED_MONTE_CARLO_RECORD* rec) {
    RET_VAL ret = SUCCESS;
    rec->originalTotalPropensities = rec->totalPropensities;
    _CalculateTotalPropensities(rec);
    return ret;
}

static RET_VAL _CalculateTotalPropensities(WEIGHTED_MONTE_CARLO_RECORD* rec) {
    RET_VAL ret = SUCCESS;
    UINT32 i = 0;
    UINT32 size = rec->reactionsSize;
    double total = 0.0;
    REACTION** reactionArray = rec->reactionArray;

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

static RET_VAL _CalculatePredilections(WEIGHTED_MONTE_CARLO_RECORD* rec) {
    // loop through all reactions (see _CalculatePropensities)
    RET_VAL ret = SUCCESS;
    int i = 0;
    int size = 0;
    double time = rec->time;
    double updatedTime = 0.0;
    REACTION* reaction = NULL;
    REACTION** reactionArray = rec->reactionArray;

    size = rec->reactionsSize;
    for (i = 0; i < size; i++) {
        reaction = reactionArray[i];
        updatedTime = GetReactionRateUpdatedTime(reaction);
        if (IS_REAL_EQUAL(updatedTime, time)) {
            if (IS_FAILED((ret = _CalculatePredilection(rec, reaction)))) {
                return ret;
            }
        }
    }
    return ret;
}

static RET_VAL _CalculatePredilection(WEIGHTED_MONTE_CARLO_RECORD* rec, REACTION* reaction) {
    RET_VAL ret = SUCCESS;
    // alpha = GetAlpha( reaction ) - stub this one
    alpha = GetReactionWeight(reaction);
    double propensity = 0.0;
    //            printf("(%s, %f)" NEW_LINE, GetCharArrayOfString(GetReactionNodeName(reaction)),
    //            GetReactionRate(reaction));
    // alpha = GetReactionWeight(reaction);
    if (strcmp(GetCharArrayOfString(GetReactionNodeName(reaction)),"R0") == 0) {
      alpha = 1.2;
      //printf("R0");
    }
    else if (strcmp(GetCharArrayOfString(GetReactionNodeName(reaction)), "R1") == 0) {
        alpha = 1/1.2;
        //printf("R1");
    }
    else if (strcmp(GetCharArrayOfString(GetReactionNodeName(reaction)), "R3P") == 0) {
        alpha = 0.5;
        //printf("R3P");
    }
    else if (strcmp(GetCharArrayOfString(GetReactionNodeName(reaction)), "R6P") == 0) {
        alpha = 2.0;
        //printf("R6P");
    }
    else if (strcmp(GetCharArrayOfString(GetReactionNodeName(reaction)), "R1M") == 0) {
      alpha = 1.0; //0.47866;
        //printf("R1M");
    }
    else if (strcmp(GetCharArrayOfString(GetReactionNodeName(reaction)), "R2M") == 0) {
      alpha = 1.0; //0.4415;
        //printf("R2M");
    }
    else if (strcmp(GetCharArrayOfString(GetReactionNodeName(reaction)), "R3M") == 0) {
      alpha = 0.01; //0.01025;
        //printf("R3M");
    }
    else if (strcmp(GetCharArrayOfString(GetReactionNodeName(reaction)), "R4M") == 0) {
      alpha = 1.0; //1.53989;
        //printf("R4M");
    }
    else if (strcmp(GetCharArrayOfString(GetReactionNodeName(reaction)), "R5M") == 0) {
      alpha = 1.0; //0.06135;
        //printf("R5M");
    }
    else if (strcmp(GetCharArrayOfString(GetReactionNodeName(reaction)), "R6M") == 0) {
      alpha = 10.0; //14.65988;
        //printf("R6M");
    }
    // propensity = GetReactionRate( reaction )
    propensity = GetReactionRate(reaction);
    // SetOriginalReactionRate( reaction, propensity )
    SetOriginalReactionRate(reaction, propensity);
    // SetReactionRate( reaction, alpha * propensity )
    SetReactionRate(reaction, alpha * propensity);
    return ret;
}

static RET_VAL _CalculatePropensities(WEIGHTED_MONTE_CARLO_RECORD* rec) {
    RET_VAL ret = SUCCESS;
    int i = 0;
    int size = 0;
    double time = rec->time;
    double updatedTime = 0.0;
    REACTION* reaction = NULL;
    REACTION** reactionArray = rec->reactionArray;

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
    qsort(rec->reactionArray, size, sizeof(REACTION*), (int(*)(const void*, const void*))_ComparePropensity);
#endif

#ifdef DEBUG
    for (i = 0; i < size; i++) {
        printf("(%s, %f), ", GetCharArrayOfString(GetReactionNodeName(rec->reactionArray[i])), GetReactionRate(rec->reactionArray[i]));
    }
    printf(NEW_LINE);
#endif
    return ret;
}

static RET_VAL _CalculatePropensity(WEIGHTED_MONTE_CARLO_RECORD* rec, REACTION* reaction) {
    RET_VAL ret = SUCCESS;
    double stoichiometry = 0.0;
    double amount = 0.0;
    double propensity = 0.0;
    double time = rec->time;
    SPECIES* species = NULL;
    IR_EDGE* edge = NULL;
    LINKED_LIST* edges = NULL;
    KINETIC_LAW* law = NULL;
    KINETIC_LAW_EVALUATER* evaluator = rec->evaluator;
    REB2SAC_SYMBOL* speciesRef = NULL;
    REB2SAC_SYMBOL* convFactor = NULL;

    edges = GetReactantEdges((IR_NODE*)reaction);
    ResetCurrentElement(edges);
    while ((edge = GetNextEdge(edges)) != NULL) {
        speciesRef = GetSpeciesRefInIREdge(edge);
        if (speciesRef) {
            stoichiometry = GetCurrentRealValueInSymbol(speciesRef);
        }
        else {
            stoichiometry = GetStoichiometryInIREdge(edge);
        }
        species = GetSpeciesInIREdge(edge);
        if ((convFactor = GetConversionFactorInSpeciesNode(species)) != NULL) {
            stoichiometry *= GetCurrentRealValueInSymbol(convFactor);
        }
        amount = GetAmountInSpeciesNode(species);
        if (amount < stoichiometry) {
            if (IS_FAILED((ret = SetReactionRate(reaction, 0.0)))) {
                return ret;
            }
#ifdef DEBUG
            printf("(%s, %f)" NEW_LINE, GetCharArrayOfString(GetReactionNodeName(reaction)),
                GetReactionRate(reaction));
#endif
            return SUCCESS;
        }
    }
#if 0
    edges = GetModifierEdges((IR_NODE*)reaction);
    ResetCurrentElement(edges);
    while ((edge = GetNextEdge(edges)) != NULL) {
        speciesRef = GetSpeciesRefInIREdge(edge);
        if (speciesRef) {
            stoichiometry = GetCurrentRealValueInSymbol(speciesRef);
        }
        else {
            stoichiometry = GetStoichiometryInIREdge(edge);
        }
        species = GetSpeciesInIREdge(edge);
        if ((convFactor = GetConversionFactorInSpeciesNode(species)) != NULL) {
            stoichiometry *= GetCurrentRealValueInSymbol(convFactor);
        }
        amount = GetAmountInSpeciesNode(species);
        if (amount < stoichiometry) {
            if (IS_FAILED((ret = SetReactionRate(reaction, 0.0)))) {
                return ret;
            }
#ifdef DEBUG
            printf("(%s, %f)" NEW_LINE, GetCharArrayOfString(GetReactionNodeName(reaction)),
                GetReactionRate(reaction));
#endif
            return SUCCESS;
        }
    }
#endif

    evaluator = rec->evaluator;
    law = GetKineticLawInReactionNode(reaction);
    //STRING* string = ToStringKineticLaw( law );
    //printf( "Law=%s" NEW_LINE, GetCharArrayOfString( string ) );
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
    }
    else {
        if (IS_FAILED((ret = SetReactionRate(reaction, propensity)))) {
            return ret;
        }
    }
#ifdef DEBUG
    printf("(%s, %f)" NEW_LINE, GetCharArrayOfString(GetReactionNodeName(reaction)),
        GetReactionRate(reaction));
#endif
    return ret;
}

static RET_VAL _FindNextReactionTime(WEIGHTED_MONTE_CARLO_RECORD* rec) {
    RET_VAL ret = SUCCESS;
    double random = 0.0;
    double average = 0.0;
    double t = 0.0;

    random = GetNextUnitUniformRandomNumber();
    t = log(1.0 / random) / rec->originalTotalPropensities;
    rec->time += t;
    rec->t = t;
    if (rec->time > rec->timeLimit) {
      rec->t -= rec->time - rec->timeLimit;
      rec->time = rec->timeLimit;
    }

    TRACE_1("time to next reaction is %f", t);
    return ret;
}

static RET_VAL _FindNextReaction(WEIGHTED_MONTE_CARLO_RECORD* rec) {
    RET_VAL ret = SUCCESS;
    UINT32 i = 0;
    UINT32 size = rec->reactionsSize - 1;
    double random = 0.0;
    double threshold = 0.0;
    double sum = 0.0;
    REACTION* reaction = NULL;
    REACTION** reactionArray = rec->reactionArray;

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

static RET_VAL _UpdateWeight(WEIGHTED_MONTE_CARLO_RECORD* rec) {
    RET_VAL ret = SUCCESS;
    double nextReactionPropensity = GetOriginalReactionRate(rec->nextReaction);
    double nextReactionPredilection = GetReactionRate(rec->nextReaction);
    
    double propensityRatio = nextReactionPropensity / rec->originalTotalPropensities;
    double predilectionRatio = nextReactionPredilection / rec->totalPropensities;

    rec->weight = rec->weight * (propensityRatio / predilectionRatio);
    return ret;
}

static RET_VAL _Update(WEIGHTED_MONTE_CARLO_RECORD* rec) {
    RET_VAL ret = SUCCESS;

    if (IS_FAILED((ret = _Print(rec)))) {
        return ret;
    }
    if (IS_FAILED((ret = _UpdateNodeValues(rec)))) {
        return ret;
    }

    return ret;
}

static RET_VAL _Print(WEIGHTED_MONTE_CARLO_RECORD* rec) {
    RET_VAL ret = SUCCESS;
    UINT32 numberSteps = rec->numberSteps;
    double nextPrintTime = rec->nextPrintTime;
    double time = rec->time;
    REACTION* reaction = NULL;
    SIMULATION_PRINTER* printer = rec->printer;

    if (time < rec->outputStartTime) return ret;
    if (rec->minPrintInterval == -1.0) {
        while ((nextPrintTime < time) && (nextPrintTime < rec->timeLimit)) {
            if (nextPrintTime > 0) {
                //printf("Time = %g\n", nextPrintTime);
                fflush(stdout);
            }
            if (IS_FAILED((ret = printer->PrintValues(printer, nextPrintTime)))) {
                return ret;
            }
            rec->currentStep++;
            nextPrintTime = rec->outputStartTime + (((rec->currentStep * (rec->timeLimit - rec->outputStartTime)) / numberSteps));
        }
    }
    else {
        if ((nextPrintTime < time) && (nextPrintTime < rec->timeLimit)) {
            if (nextPrintTime > rec->initialTime) {
                printf("Time = %g\n", nextPrintTime);
                fflush(stdout);
            }
            if (IS_FAILED((ret = printer->PrintValues(printer, time)))) {
                return ret;
            }
            nextPrintTime = time + rec->minPrintInterval;
        }
    }
    rec->nextPrintTime = nextPrintTime;
    return ret;
}

static RET_VAL _UpdateNodeValues(WEIGHTED_MONTE_CARLO_RECORD* rec) {
    RET_VAL ret = SUCCESS;

    if (IS_FAILED((ret = _UpdateSpeciesValues(rec)))) {
        return ret;
    }
    if (IS_FAILED((ret = _UpdateReactionRateUpdateTime(rec)))) {
        return ret;
    }

    return ret;
}

static double fireEvents(WEIGHTED_MONTE_CARLO_RECORD* rec, double time) {
    int i;
    double nextEventTime;
    BOOL triggerEnabled;
    double deltaTime;
    BOOL eventFired = FALSE;
    double firstEventTime = -1.0;
    int eventToFire = -1;
    double prMax, prMax2;
    double priority = 0.0;
    double randChoice = 0.0;

    do {
        eventFired = FALSE;
        eventToFire = -1;
        for (i = 0; i < rec->eventsSize; i++) {
            nextEventTime = GetNextEventTimeInEvent(rec->eventArray[i]);
            triggerEnabled = GetTriggerEnabledInEvent(rec->eventArray[i]);
            if (nextEventTime != -1.0) {
                /* Disable event, if necessary */
                if ((triggerEnabled) && (GetTriggerCanBeDisabled(rec->eventArray[i]))) {
                    if (!rec->evaluator->EvaluateWithCurrentAmountsDeter(rec->evaluator,
                        (KINETIC_LAW*)GetTriggerInEvent(rec->eventArray[i]))) {
                        nextEventTime = -1.0;
                        SetNextEventTimeInEvent(rec->eventArray[i], -1.0);
                        SetTriggerEnabledInEvent(rec->eventArray[i], FALSE);
                        continue;
                    }
                }
                if (time >= nextEventTime) {
                    if (GetPriorityInEvent(rec->eventArray[i]) == NULL) {
                        priority = 0;
                    }
                    else {
                        priority = rec->evaluator->EvaluateWithCurrentAmounts(rec->evaluator,
                            (KINETIC_LAW*)GetPriorityInEvent(rec->eventArray[i]));
                    }
                    if ((eventToFire == (-1)) || (priority > prMax)) {
                        eventToFire = i;
                        prMax = priority;
                        prMax2 = GetNextUniformRandomNumber(0, 1);
                    }
                    else if (priority == prMax) {
                        randChoice = GetNextUniformRandomNumber(0, 1);
                        if (randChoice > prMax2) {
                            eventToFire = i;
                            prMax2 = randChoice;
                        }
                    }
                }
                else {
                    /* Determine time to next event */
                    if ((firstEventTime == -1.0) || (nextEventTime < firstEventTime)) {
                        firstEventTime = nextEventTime;
                    }
                }
            }
            /* Try to find time to next event trigger */
            nextEventTime = /*time +*/
                rec->findNextTime->FindNextTimeWithCurrentAmounts(rec->findNextTime,
                    (KINETIC_LAW*)GetTriggerInEvent(rec->eventArray[i]));
            if (nextEventTime >= 0)
                nextEventTime = time + nextEventTime;
            if ((firstEventTime == -1.0) || (nextEventTime < firstEventTime)) {
                firstEventTime = nextEventTime;
            }
            if (!triggerEnabled) {
                /* Check if event has been triggered */
                if (rec->evaluator->EvaluateWithCurrentAmountsDeter(rec->evaluator,
                    (KINETIC_LAW*)GetTriggerInEvent(rec->eventArray[i]))) {
                    SetTriggerEnabledInEvent(rec->eventArray[i], TRUE);
                    /* Calculate delay until the event fires */
                    if (GetDelayInEvent(rec->eventArray[i]) == NULL) {
                        deltaTime = 0;
                    }
                    else {
                        deltaTime = rec->evaluator->EvaluateWithCurrentAmounts(rec->evaluator,
                            (KINETIC_LAW*)GetDelayInEvent(rec->eventArray[i]));
                    }
                    if (deltaTime == 0) eventFired = TRUE;
                    if (deltaTime >= 0) {
                        /* Set time for event to fire and get assignment values, if necessary */
                        SetNextEventTimeInEvent(rec->eventArray[i], time + deltaTime);
                        if (GetUseValuesFromTriggerTime(rec->eventArray[i])) {
                            SetEventAssignmentsNextValuesTime(rec->eventArray[i], rec, time + deltaTime);
                        }
                        if ((firstEventTime == -1.0) || (time + deltaTime < firstEventTime)) {
                            firstEventTime = time + deltaTime;
                        }
                    } /* else if (deltaTime == 0) {
                      SetEventAssignmentsNextValues( rec->eventArray[i], rec );
                      fireEvent( rec->eventArray[i], rec );
                      eventFired = TRUE;
                      } */ else {
                        ErrorReport(FAILING, "_Update", "delay for event evaluates to a negative number");
                        return -2;
                    }
                }
            }
            else {
                /* Set trigger enabled to false, if it has become disabled */
                if (!rec->evaluator->EvaluateWithCurrentAmountsDeter(rec->evaluator,
                    (KINETIC_LAW*)GetTriggerInEvent(rec->eventArray[i]))) {
                    SetTriggerEnabledInEvent(rec->eventArray[i], FALSE);
                }
            }
        }
        /* Fire event */
        if (eventToFire >= 0) {
            if (!GetUseValuesFromTriggerTime(rec->eventArray[eventToFire])) {
                SetEventAssignmentsNextValues(rec->eventArray[eventToFire], rec);
            }
            rec->time = time;
            fireEvent(rec->eventArray[eventToFire], rec);
            SetNextEventTimeInEvent(rec->eventArray[eventToFire], -1.0);
            eventFired = TRUE;
            eventToFire = -1;
            firstEventTime = -1;

            /* When an event fires, update algebraic rules and fast reactions */
            ExecuteAssignments(rec);
            if (rec->algebraicRulesSize > 0) {
                EvaluateAlgebraicRules(rec);
            }
            if (rec->numberFastSpecies > 0) {
                ExecuteFastReactions(rec);
            }
        }
        /* Repeat as long as events are firing */
    } while (eventFired);
    /* Return the time for the next event firing or potential triggering */
    return firstEventTime;
}

static void SetEventAssignmentsNextValues(EVENT* event, WEIGHTED_MONTE_CARLO_RECORD* rec) {
    LINKED_LIST* list = NULL;
    EVENT_ASSIGNMENT* eventAssignment;
    double amount = 0.0;
    UINT j;
    BYTE varType;

    list = GetEventAssignments(event);
    ResetCurrentElement(list);
    while ((eventAssignment = (EVENT_ASSIGNMENT*)GetNextFromLinkedList(list)) != NULL) {
        amount = rec->evaluator->EvaluateWithCurrentAmounts(rec->evaluator, eventAssignment->assignment);
        SetEventAssignmentNextValueTime(eventAssignment, amount, rec->time);
    }
}

static void SetEventAssignmentsNextValuesTime(EVENT* event, WEIGHTED_MONTE_CARLO_RECORD* rec, double time) {
    LINKED_LIST* list = NULL;
    EVENT_ASSIGNMENT* eventAssignment;
    double amount = 0.0;
    UINT j;
    BYTE varType;

    list = GetEventAssignments(event);
    ResetCurrentElement(list);
    while ((eventAssignment = (EVENT_ASSIGNMENT*)GetNextFromLinkedList(list)) != NULL) {
        amount = rec->evaluator->EvaluateWithCurrentAmounts(rec->evaluator, eventAssignment->assignment);
        SetEventAssignmentNextValueTime(eventAssignment, amount, time);
    }
}

static void fireEvent(EVENT* event, WEIGHTED_MONTE_CARLO_RECORD* rec) {
    LINKED_LIST* list = NULL;
    EVENT_ASSIGNMENT* eventAssignment;
    double amount = 0.0;
    UINT j;
    BYTE varType;
    /* STRING eventId; */

    /*  eventId = GetEventId( event );
        printf("Firing event %s\n",GetCharArrayOfString( eventId )); */
    list = GetEventAssignments(event);
    ResetCurrentElement(list);
    while ((eventAssignment = (EVENT_ASSIGNMENT*)GetNextFromLinkedList(list)) != NULL) {
        /* printf("Firing event %s\n",GetCharArrayOfString(eventAssignment->var)); */
        varType = GetEventAssignmentVarType(eventAssignment);
        j = GetEventAssignmentIndex(eventAssignment);
        /* printf("varType = %d j = %d\n",varType,j); */
        amount = GetEventAssignmentNextValueTime(eventAssignment, rec->time);
        /* printf("conc = %g\n",amount); */
        if (varType == SPECIES_EVENT_ASSIGNMENT) {
            SetAmountInSpeciesNode(rec->speciesArray[j], amount);
            _UpdateReactionRateUpdateTimeForSpecies(rec, rec->speciesArray[j]);
        }
        else if (varType == COMPARTMENT_EVENT_ASSIGNMENT) {
            SetCurrentSizeInCompartment(rec->compartmentArray[j], amount);
            _UpdateAllReactionRateUpdateTimes(rec, rec->time);
        }
        else {
            SetCurrentRealValueInSymbol(rec->symbolArray[j], amount);
            _UpdateAllReactionRateUpdateTimes(rec, rec->time);
        }
    }
}

/* Update values using assignments rules */
static void ExecuteAssignments(WEIGHTED_MONTE_CARLO_RECORD* rec) {
    UINT32 i = 0;
    UINT32 j = 0;
    double amount = 0.0;
    BYTE varType;

    for (i = 0; i < rec->rulesSize; i++) {
        if (GetRuleType(rec->ruleArray[i]) == RULE_TYPE_ASSIGNMENT) {
            amount = rec->evaluator->EvaluateWithCurrentAmountsDeter(rec->evaluator,
                (KINETIC_LAW*)GetMathInRule(rec->ruleArray[i]));
            varType = GetRuleVarType(rec->ruleArray[i]);
            j = GetRuleIndex(rec->ruleArray[i]);
            if (varType == SPECIES_RULE) {
                SetAmountInSpeciesNode(rec->speciesArray[j], amount);
                _UpdateReactionRateUpdateTimeForSpecies(rec, rec->speciesArray[j]);
            }
            else if (varType == COMPARTMENT_RULE) {
                SetCurrentSizeInCompartment(rec->compartmentArray[j], amount);
                _UpdateAllReactionRateUpdateTimes(rec, rec->time);
            }
            else {
                SetCurrentRealValueInSymbol(rec->symbolArray[j], amount);
                _UpdateAllReactionRateUpdateTimes(rec, rec->time);
            }
        }
    }
}

int wSSA_algebraicRules(const gsl_vector* x, void* params, gsl_vector* f) {
    UINT32 i = 0;
    UINT32 j = 0;
    double amount = 0.0;
    WEIGHTED_MONTE_CARLO_RECORD* rec = ((WEIGHTED_MONTE_CARLO_RECORD*)params);
    SPECIES* species = NULL;
    COMPARTMENT* compartment = NULL;
    REB2SAC_SYMBOL* symbol = NULL;

    for (i = 0; i < rec->speciesSize; i++) {
        species = rec->speciesArray[i];
        if (IsSpeciesNodeAlgebraic(species)) {
            amount = gsl_vector_get(x, j);
            j++;
            SetAmountInSpeciesNode(species, amount);
        }
    }
    for (i = 0; i < rec->compartmentsSize; i++) {
        compartment = rec->compartmentArray[i];
        if (IsCompartmentAlgebraic(compartment)) {
            amount = gsl_vector_get(x, j);
            j++;
            SetCurrentSizeInCompartment(compartment, amount);
        }
    }
    for (i = 0; i < rec->symbolsSize; i++) {
        symbol = rec->symbolArray[i];
        if (IsSymbolAlgebraic(symbol)) {
            amount = gsl_vector_get(x, j);
            j++;
            SetCurrentRealValueInSymbol(symbol, amount);
        }
    }
    j = 0;
    for (i = 0; i < rec->rulesSize; i++) {
        if (GetRuleType(rec->ruleArray[i]) == RULE_TYPE_ALGEBRAIC) {
            amount = rec->evaluator->EvaluateWithCurrentAmountsDeter(rec->evaluator,
                (KINETIC_LAW*)GetMathInRule(rec->ruleArray[i]));
            gsl_vector_set(f, j, amount);
            j++;
        }
    }

    return GSL_SUCCESS;
}

void _wSSA_print_state(size_t iter, gsl_multiroot_fsolver* s, int n) {
    UINT32 i = 0;

    printf("x = [ ");
    for (i = 0; i < n; i++) {
        printf(" %g ", gsl_vector_get(s->x, i));
    }
    printf("] f = [ ");
    for (i = 0; i < n; i++) {
        printf(" %g ", gsl_vector_get(s->f, i));
    }
    printf("]\n");
}

static RET_VAL EvaluateAlgebraicRules(WEIGHTED_MONTE_CARLO_RECORD* rec) {
    const gsl_multiroot_fsolver_type* T;
    gsl_multiroot_fsolver* s;
    SPECIES* species = NULL;
    COMPARTMENT* compartment = NULL;
    REB2SAC_SYMBOL* symbol = NULL;

    int status;
    size_t i, j, iter = 0;
    double amount;

    const size_t n = rec->algebraicRulesSize;

    gsl_multiroot_function f = { &wSSA_algebraicRules, n, rec };
    gsl_vector* x = gsl_vector_alloc(n);

    j = 0;
    for (i = 0; i < rec->speciesSize; i++) {
        species = rec->speciesArray[i];
        if (IsSpeciesNodeAlgebraic(species)) {
            amount = GetAmountInSpeciesNode(species);
            gsl_vector_set(x, j, amount);
            j++;
        }
    }
    for (i = 0; i < rec->compartmentsSize; i++) {
        compartment = rec->compartmentArray[i];
        if (IsCompartmentAlgebraic(compartment)) {
            amount = GetCurrentSizeInCompartment(compartment);
            gsl_vector_set(x, j, amount);
            j++;
        }
    }
    for (i = 0; i < rec->symbolsSize; i++) {
        symbol = rec->symbolArray[i];
        if (IsSymbolAlgebraic(symbol)) {
            amount = GetCurrentRealValueInSymbol(symbol);
            gsl_vector_set(x, j, amount);
            j++;
        }
    }

    T = gsl_multiroot_fsolver_hybrids;
    s = gsl_multiroot_fsolver_alloc(T, n);
    gsl_multiroot_fsolver_set(s, &f, x);

    //print_state (iter, s, n);

    do {
        iter++;
        status = gsl_multiroot_fsolver_iterate(s);

        //print_state (iter, s, n);

        if (status)   /* check if solver is stuck */
            break;

        status = gsl_multiroot_test_residual(s->f, 1e-7);
    } while (status == GSL_CONTINUE && iter < 1000);

    //printf ("status = %s\n", gsl_strerror (status));

    gsl_multiroot_fsolver_free(s);
    gsl_vector_free(x);
    return 0;
}

int wSSAfastReactions(const gsl_vector* x, void* params, gsl_vector* f) {
    RET_VAL ret = SUCCESS;
    UINT32 i = 0;
    UINT32 j;
    double amount = 0.0;
    WEIGHTED_MONTE_CARLO_RECORD* rec = ((WEIGHTED_MONTE_CARLO_RECORD*)params);
    SPECIES* species = NULL;
    REACTION* reaction = NULL;
    LINKED_LIST* Redges = NULL;
    LINKED_LIST* Pedges = NULL;
    IR_EDGE* edge = NULL;
    double stoichiometry = 0.0;
    BOOL boundary = FALSE;
    REB2SAC_SYMBOL* speciesRef = NULL;
    REB2SAC_SYMBOL* convFactor = NULL;

    j = 0;
    for (i = 0; i < rec->speciesSize; i++) {
        species = rec->speciesArray[i];
        if (IsSpeciesNodeFast(species)) {
            amount = gsl_vector_get(x, j);
            j++;
            SetAmountInSpeciesNode(species, amount);
        }
    }
    _CalculatePropensities(rec);
    j = 0;
    for (i = 0; i < rec->reactionsSize; i++) {
        reaction = rec->reactionArray[i];
        if (IsReactionFastInReactionNode(reaction)) {
            amount = 0.0;
            Redges = GetReactantsInReactionNode(reaction);
            Pedges = GetProductsInReactionNode(reaction);
            ResetCurrentElement(Redges);
            if ((edge = GetNextEdge(Redges)) != NULL) {
                speciesRef = GetSpeciesRefInIREdge(edge);
                if (speciesRef) {
                    stoichiometry = GetCurrentRealValueInSymbol(speciesRef);
                }
                else {
                    stoichiometry = GetStoichiometryInIREdge(edge);
                }
                species = GetSpeciesInIREdge(edge);
                if ((convFactor = GetConversionFactorInSpeciesNode(species)) != NULL) {
                    stoichiometry *= GetCurrentRealValueInSymbol(convFactor);
                }
                if (HasBoundaryConditionInSpeciesNode(species)) {
                    amount = GetAmountInSpeciesNode(species);
                    boundary = TRUE;
                }
                else {
                    amount = GetAmountInSpeciesNode(species) / stoichiometry;
                    boundary = FALSE;
                }
                ResetCurrentElement(Pedges);
                while ((edge = GetNextEdge(Pedges)) != NULL) {
                    speciesRef = GetSpeciesRefInIREdge(edge);
                    if (speciesRef) {
                        stoichiometry = GetCurrentRealValueInSymbol(speciesRef);
                    }
                    else {
                        stoichiometry = GetStoichiometryInIREdge(edge);
                    }
                    species = GetSpeciesInIREdge(edge);
                    if ((convFactor = GetConversionFactorInSpeciesNode(species)) != NULL) {
                        stoichiometry *= GetCurrentRealValueInSymbol(convFactor);
                    }
                    if (boundary) {
                        if (HasBoundaryConditionInSpeciesNode(species)) {
                            gsl_vector_set(f, j, amount + GetAmountInSpeciesNode(species) - rec->fastCons[j]);
                        }
                        else {
                            gsl_vector_set(f, j, amount - rec->fastCons[j]);
                        }
                    }
                    else {
                        if (HasBoundaryConditionInSpeciesNode(species)) {
                            gsl_vector_set(f, j, GetAmountInSpeciesNode(species) - rec->fastCons[j]);
                        }
                        else {
                            gsl_vector_set(f, j, amount + GetAmountInSpeciesNode(species) / stoichiometry - rec->fastCons[j]);
                        }
                    }
                    j++;
                }
            }
            ResetCurrentElement(Pedges);
            if ((edge = GetNextEdge(Pedges)) != NULL) {
                speciesRef = GetSpeciesRefInIREdge(edge);
                if (speciesRef) {
                    stoichiometry = GetCurrentRealValueInSymbol(speciesRef);
                }
                else {
                    stoichiometry = GetStoichiometryInIREdge(edge);
                }
                species = GetSpeciesInIREdge(edge);
                if ((convFactor = GetConversionFactorInSpeciesNode(species)) != NULL) {
                    stoichiometry *= GetCurrentRealValueInSymbol(convFactor);
                }
                if (HasBoundaryConditionInSpeciesNode(species)) {
                    amount = GetAmountInSpeciesNode(species);
                    boundary = TRUE;
                }
                else {
                    amount = GetAmountInSpeciesNode(species) / stoichiometry;
                    boundary = FALSE;
                }
                while ((edge = GetNextEdge(Redges)) != NULL) {
                    speciesRef = GetSpeciesRefInIREdge(edge);
                    if (speciesRef) {
                        stoichiometry = GetCurrentRealValueInSymbol(speciesRef);
                    }
                    else {
                        stoichiometry = GetStoichiometryInIREdge(edge);
                    }
                    species = GetSpeciesInIREdge(edge);
                    if ((convFactor = GetConversionFactorInSpeciesNode(species)) != NULL) {
                        stoichiometry *= GetCurrentRealValueInSymbol(convFactor);
                    }
                    if (boundary) {
                        if (HasBoundaryConditionInSpeciesNode(species)) {
                            gsl_vector_set(f, j, amount + GetAmountInSpeciesNode(species) - rec->fastCons[j]);
                        }
                        else {
                            gsl_vector_set(f, j, amount - rec->fastCons[j]);
                        }
                    }
                    else {
                        if (HasBoundaryConditionInSpeciesNode(species)) {
                            gsl_vector_set(f, j, GetAmountInSpeciesNode(species) - rec->fastCons[j]);
                        }
                        else {
                            gsl_vector_set(f, j, amount + GetAmountInSpeciesNode(species) / stoichiometry - rec->fastCons[j]);
                        }
                    }
                    j++;
                }
            }
            amount = GetReactionRate(reaction);
            gsl_vector_set(f, j, amount);
            j++;
            break;
        }
    }

    return GSL_SUCCESS;
}

static RET_VAL ExecuteFastReactions(WEIGHTED_MONTE_CARLO_RECORD* rec) {
    const gsl_multiroot_fsolver_type* T;
    gsl_multiroot_fsolver* s;
    SPECIES* species = NULL;
    REACTION* reaction = NULL;
    LINKED_LIST* Redges = NULL;
    LINKED_LIST* Pedges = NULL;
    IR_EDGE* edge = NULL;
    double stoichiometry = 0.0;
    int status;
    size_t i, j, iter = 0;
    double amount;
    const size_t n = rec->numberFastSpecies;
    BOOL boundary = FALSE;
    REB2SAC_SYMBOL* speciesRef = NULL;
    REB2SAC_SYMBOL* convFactor = NULL;

    j = 0;
    for (i = 0; i < rec->reactionsSize; i++) {
        reaction = rec->reactionArray[i];
        if (IsReactionFastInReactionNode(reaction)) {
            amount = 0.0;
            Redges = GetReactantsInReactionNode(reaction);
            Pedges = GetProductsInReactionNode(reaction);
            ResetCurrentElement(Redges);
            if ((edge = GetNextEdge(Redges)) != NULL) {
                speciesRef = GetSpeciesRefInIREdge(edge);
                if (speciesRef) {
                    stoichiometry = GetCurrentRealValueInSymbol(speciesRef);
                }
                else {
                    stoichiometry = GetStoichiometryInIREdge(edge);
                }
                species = GetSpeciesInIREdge(edge);
                if ((convFactor = GetConversionFactorInSpeciesNode(species)) != NULL) {
                    stoichiometry *= GetCurrentRealValueInSymbol(convFactor);
                }
                if (HasBoundaryConditionInSpeciesNode(species)) {
                    amount = GetAmountInSpeciesNode(species);
                    boundary = TRUE;
                }
                else {
                    amount = GetAmountInSpeciesNode(species) / stoichiometry;
                    boundary = FALSE;
                }
                ResetCurrentElement(Pedges);
                while ((edge = GetNextEdge(Pedges)) != NULL) {
                    speciesRef = GetSpeciesRefInIREdge(edge);
                    if (speciesRef) {
                        stoichiometry = GetCurrentRealValueInSymbol(speciesRef);
                    }
                    else {
                        stoichiometry = GetStoichiometryInIREdge(edge);
                    }
                    species = GetSpeciesInIREdge(edge);
                    if ((convFactor = GetConversionFactorInSpeciesNode(species)) != NULL) {
                        stoichiometry *= GetCurrentRealValueInSymbol(convFactor);
                    }
                    if (boundary) {
                        rec->fastCons[j] = amount;
                        if (HasBoundaryConditionInSpeciesNode(species)) {
                            rec->fastCons[j] += GetAmountInSpeciesNode(species);
                        }
                    }
                    else {
                        if (HasBoundaryConditionInSpeciesNode(species)) {
                            rec->fastCons[j] = GetAmountInSpeciesNode(species);
                        }
                        else {
                            rec->fastCons[j] = amount + GetAmountInSpeciesNode(species) / stoichiometry;
                        }
                    }
                    j++;
                }
            }
            ResetCurrentElement(Pedges);
            if ((edge = GetNextEdge(Pedges)) != NULL) {
                speciesRef = GetSpeciesRefInIREdge(edge);
                if (speciesRef) {
                    stoichiometry = GetCurrentRealValueInSymbol(speciesRef);
                }
                else {
                    stoichiometry = GetStoichiometryInIREdge(edge);
                }
                species = GetSpeciesInIREdge(edge);
                if ((convFactor = GetConversionFactorInSpeciesNode(species)) != NULL) {
                    stoichiometry *= GetCurrentRealValueInSymbol(convFactor);
                }
                if (HasBoundaryConditionInSpeciesNode(species)) {
                    amount = GetAmountInSpeciesNode(species);
                    boundary = TRUE;
                }
                else {
                    amount = GetAmountInSpeciesNode(species) / stoichiometry;
                    boundary = FALSE;
                }
                while ((edge = GetNextEdge(Redges)) != NULL) {
                    speciesRef = GetSpeciesRefInIREdge(edge);
                    if (speciesRef) {
                        stoichiometry = GetCurrentRealValueInSymbol(speciesRef);
                    }
                    else {
                        stoichiometry = GetStoichiometryInIREdge(edge);
                    }
                    species = GetSpeciesInIREdge(edge);
                    if ((convFactor = GetConversionFactorInSpeciesNode(species)) != NULL) {
                        stoichiometry *= GetCurrentRealValueInSymbol(convFactor);
                    }
                    if (boundary) {
                        rec->fastCons[j] = amount;
                        if (HasBoundaryConditionInSpeciesNode(species)) {
                            rec->fastCons[j] += GetAmountInSpeciesNode(species);
                        }
                    }
                    else {
                        if (HasBoundaryConditionInSpeciesNode(species)) {
                            rec->fastCons[j] = GetAmountInSpeciesNode(species);
                        }
                        else {
                            rec->fastCons[j] = amount + GetAmountInSpeciesNode(species) / stoichiometry;
                        }
                    }
                    j++;
                }
            }
            break;
        }
    }

    gsl_multiroot_function f = { &wSSAfastReactions, n, rec };

    gsl_vector* x = gsl_vector_alloc(n);

    j = 0;
    for (i = 0; i < rec->speciesSize; i++) {
        species = rec->speciesArray[i];
        if (!HasBoundaryConditionInSpeciesNode(species) && IsSpeciesNodeFast(species)) {
            amount = GetAmountInSpeciesNode(species);
            gsl_vector_set(x, j, amount);
            j++;
        }
    }

    T = gsl_multiroot_fsolver_hybrids;
    s = gsl_multiroot_fsolver_alloc(T, n);
    gsl_multiroot_fsolver_set(s, &f, x);

    //printf("time = %g\n",rec->time);
    //print_state (iter, s, n);

    do {
        iter++;
        status = gsl_multiroot_fsolver_iterate(s);

        //print_state (iter, s, n);

        if (status)   /* check if solver is stuck */
            break;

        status = gsl_multiroot_test_residual(s->f, 1e-7);
    } while (status == GSL_CONTINUE && iter < 1000);

    //printf ("status = %s\n", gsl_strerror (status));

    j = 0;
    for (i = 0; i < rec->speciesSize; i++) {
        species = rec->speciesArray[i];
        if (!HasBoundaryConditionInSpeciesNode(species) && IsSpeciesNodeFast(species)) {
            //if (IsSpeciesNodeFast( species )) {
            amount = gsl_vector_get(s->x, j);
            j++;
            SetAmountInSpeciesNode(species, amount);
        }
    }

    gsl_multiroot_fsolver_free(s);
    gsl_vector_free(x);
    return 0;
}

static RET_VAL _UpdateSpeciesValues(WEIGHTED_MONTE_CARLO_RECORD* rec) {
    RET_VAL ret = SUCCESS;
    double stoichiometry = 0;
    double amount = 0;
    double change = 0;
    SPECIES* species = NULL;
    IR_EDGE* edge = NULL;
    REACTION* reaction = rec->nextReaction;
    LINKED_LIST* edges = NULL;
    KINETIC_LAW_EVALUATER* evaluator = rec->evaluator;
    UINT i = 0;
    UINT j = 0;
    double deltaTime;
    BOOL triggerEnabled;
    BYTE varType;
    REB2SAC_SYMBOL* speciesRef = NULL;
    REB2SAC_SYMBOL* convFactor = NULL;

    for (i = 0; i < rec->rulesSize; i++) {
        if (GetRuleType(rec->ruleArray[i]) == RULE_TYPE_RATE_ASSIGNMENT) {
            change = rec->evaluator->EvaluateWithCurrentAmountsDeter(rec->evaluator,
                (KINETIC_LAW*)GetMathInRule(rec->ruleArray[i]));
            varType = GetRuleVarType(rec->ruleArray[i]);
            j = GetRuleIndex(rec->ruleArray[i]);
            if (varType == SPECIES_RULE) {
                SetRateInSpeciesNode(rec->speciesArray[j], change);
                amount = GetAmountInSpeciesNode(rec->speciesArray[j]);
                amount += (change * rec->t);
                SetRuleCurValue(rec->ruleArray[i], amount);
            }
            else if (varType == COMPARTMENT_RULE) {
                SetCurrentRateInCompartment(rec->compartmentArray[j], change);
                amount = GetCurrentSizeInCompartment(rec->compartmentArray[j]);
                amount += (change * rec->t);
                SetRuleCurValue(rec->ruleArray[i], amount);
            }
            else {
                SetCurrentRateInSymbol(rec->symbolArray[j], change);
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
            }
            else if (varType == COMPARTMENT_RULE) {
                SetCurrentSizeInCompartment(rec->compartmentArray[j], amount);
                _UpdateAllReactionRateUpdateTimes(rec, rec->time);
            }
            else {
                SetCurrentRealValueInSymbol(rec->symbolArray[j], amount);
                _UpdateAllReactionRateUpdateTimes(rec, rec->time);
            }
        }
    }

    if (reaction) {
        edges = GetReactantEdges((IR_NODE*)reaction);
        ResetCurrentElement(edges);
        while ((edge = GetNextEdge(edges)) != NULL) {
            speciesRef = GetSpeciesRefInIREdge(edge);
            if (speciesRef) {
                stoichiometry = GetCurrentRealValueInSymbol(speciesRef);
            }
            else {
                stoichiometry = GetStoichiometryInIREdge(edge);
            }
            species = GetSpeciesInIREdge(edge);
            if ((convFactor = GetConversionFactorInSpeciesNode(species)) != NULL) {
                stoichiometry *= GetCurrentRealValueInSymbol(convFactor);
            }
            if (HasBoundaryConditionInSpeciesNode(species)) continue;
            amount = GetAmountInSpeciesNode(species) - stoichiometry;
            TRACE_3("the amount of %s decreases from %g to %g",
                GetCharArrayOfString(GetSpeciesNodeName(species)),
                GetAmountInSpeciesNode(species),
                amount);
            SetAmountInSpeciesNode(species, amount);
            if (IS_FAILED((ret = evaluator->SetSpeciesValue(evaluator, species, amount)))) {
                return ret;
            }
        }
        edges = GetProductEdges((IR_NODE*)reaction);
        ResetCurrentElement(edges);
        while ((edge = GetNextEdge(edges)) != NULL) {
            speciesRef = GetSpeciesRefInIREdge(edge);
            if (speciesRef) {
                stoichiometry = GetCurrentRealValueInSymbol(speciesRef);
            }
            else {
                stoichiometry = GetStoichiometryInIREdge(edge);
            }
            species = GetSpeciesInIREdge(edge);
            if ((convFactor = GetConversionFactorInSpeciesNode(species)) != NULL) {
                stoichiometry *= GetCurrentRealValueInSymbol(convFactor);
            }
            if (HasBoundaryConditionInSpeciesNode(species)) continue;
            amount = GetAmountInSpeciesNode(species) + stoichiometry;
            TRACE_3("the amount of %s increases from %g to %g",
                GetCharArrayOfString(GetSpeciesNodeName(species)),
                GetAmountInSpeciesNode(species),
                amount);
            SetAmountInSpeciesNode(species, amount);
        }
    }

    for (j = 0; j < rec->symbolsSize; j++) {
        if ((strcmp(GetCharArrayOfString(GetSymbolID(rec->symbolArray[j])), "t") == 0) ||
            (strcmp(GetCharArrayOfString(GetSymbolID(rec->symbolArray[j])), "time") == 0)) {
            SetCurrentRealValueInSymbol(rec->symbolArray[j], rec->time);
        }
    }

    ExecuteAssignments(rec);
    if (rec->algebraicRulesSize > 0) {
        EvaluateAlgebraicRules(rec);
    }
    if (rec->numberFastSpecies > 0) {
        ExecuteFastReactions(rec);
    }

    return ret;
}

static RET_VAL _UpdateAllReactionRateUpdateTimes(WEIGHTED_MONTE_CARLO_RECORD* rec, double time) {
    RET_VAL ret = SUCCESS;
    UINT i;

    for (i = 0; i < rec->reactionsSize; i++) {
        if (IS_FAILED((ret = SetReactionRateUpdatedTime(rec->reactionArray[i], time)))) {
            return ret;
        }
    }
    return ret;
}

static RET_VAL _UpdateReactionRateUpdateTimeForSpecies(WEIGHTED_MONTE_CARLO_RECORD* rec, SPECIES* species) {
    RET_VAL ret = SUCCESS;
    double time = rec->time;
    IR_EDGE* updateEdge = NULL;
    REACTION* reaction = NULL;
    LINKED_LIST* updateEdges = NULL;

    updateEdges = GetReactantEdges((IR_NODE*)species);
    ResetCurrentElement(updateEdges);
    while ((updateEdge = GetNextEdge(updateEdges)) != NULL) {
        reaction = GetReactionInIREdge(updateEdge);
        if (IS_FAILED((ret = SetReactionRateUpdatedTime(reaction, time)))) {
            return ret;
        }
    }

    updateEdges = GetModifierEdges((IR_NODE*)species);
    ResetCurrentElement(updateEdges);
    while ((updateEdge = GetNextEdge(updateEdges)) != NULL) {
        reaction = GetReactionInIREdge(updateEdge);
        if (IS_FAILED((ret = SetReactionRateUpdatedTime(reaction, time)))) {
            return ret;
        }
    }

    updateEdges = GetProductEdges((IR_NODE*)species);
    ResetCurrentElement(updateEdges);
    while ((updateEdge = GetNextEdge(updateEdges)) != NULL) {
        reaction = GetReactionInIREdge(updateEdge);
        if (IS_FAILED((ret = SetReactionRateUpdatedTime(reaction, time)))) {
            return ret;
        }
    }
    return ret;
}

static RET_VAL _UpdateReactionRateUpdateTime(WEIGHTED_MONTE_CARLO_RECORD* rec) {
    RET_VAL ret = SUCCESS;
    double rate = 0.0;
    IR_EDGE* edge = NULL;
    LINKED_LIST* edges = NULL;
    SPECIES* species = NULL;

    if (rec->nextReaction == NULL) {
        return ret;
    }
    edges = GetReactantEdges((IR_NODE*)rec->nextReaction);
    ResetCurrentElement(edges);
    while ((edge = GetNextEdge(edges)) != NULL) {
        species = GetSpeciesInIREdge(edge);
        if (IS_FAILED((ret = _UpdateReactionRateUpdateTimeForSpecies(rec, species)))) {
            return ret;
        }
    }

    edges = GetProductEdges((IR_NODE*)rec->nextReaction);
    ResetCurrentElement(edges);
    while ((edge = GetNextEdge(edges)) != NULL) {
        species = GetSpeciesInIREdge(edge);
        if (IS_FAILED((ret = _UpdateReactionRateUpdateTimeForSpecies(rec, species)))) {
            return ret;
        }
    }
    return ret;
}

static int _ComparePropensity(REACTION* a, REACTION* b) {
    double d1 = 0.0;
    double d2 = 0.0;

    d1 = GetReactionRate(a);
    d2 = GetReactionRate(b);

    if (IS_REAL_EQUAL(d1, d2)) {
        return 0;
    }
    return (d1 < d2) ? -1 : 1;
}
