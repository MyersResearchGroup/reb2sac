#if !defined(HAVE_MPDE_MONTE_CARLO)
#define HAVE_MPDE_MONTE_CARLO

#include "simulation_method.h"

BEGIN_C_NAMESPACE

DLLSCOPE RET_VAL STDCALL DoMPDEMonteCarloAnalysis(BACK_END_PROCESSOR *backend, IR *ir);
DLLSCOPE RET_VAL STDCALL CloseMPDEMonteCarloAnalyzer(BACK_END_PROCESSOR *backend);

#if 0
#define USE_BIOSPICE_TSD_FORMAT 1
#define GET_SEED_FROM_COMMAND_LINE 1
#endif

typedef struct {
    REACTION **reactionArray;
    UINT32 reactionsSize;
    SPECIES **speciesArray;
    UINT32 speciesSize;
    RULE **ruleArray;
    UINT32 rulesSize;
    UINT32 algebraicRulesSize;
    UINT32 numberFastSpecies;
    UINT32 numberFastReactions;
    double *fastCons;
    COMPARTMENT **compartmentArray;
    UINT32 compartmentsSize;
    REB2SAC_SYMBOL **symbolArray;
    UINT32 symbolsSize;
    CONSTRAINT **constraintArray;
    UINT32 constraintsSize;
    EVENT **eventArray;
    UINT32 eventsSize;
    REACTION *nextReaction;
    SIMULATION_PRINTER *meanPrinter;
    SIMULATION_PRINTER *varPrinter;
    SIMULATION_PRINTER *sdPrinter;
    SIMULATION_PRINTER *mpPrinter1;
    SIMULATION_PRINTER *mpPrinter2;
    SIMULATION_RUN_TERMINATION_DECIDER *decider;
    double time;
    double t;
    double printInterval;
    double minPrintInterval;
    UINT32 currentStep;
    UINT32 numberSteps;
    double nextPrintTime;
    double timeLimit;
    double timeStep;
    KINETIC_LAW_EVALUATER *evaluator;
    KINETIC_LAW_FIND_NEXT_TIME *findNextTime;
    double totalPropensities;
    UINT32 seed;
    UINT32 runs;
    char *outDir;
    int startIndex;
    double *oldSpeciesMeans;
    double *oldSpeciesVariances;
    double *newSpeciesMeans;
    double *newSpeciesVariances;
    double *speciesSD;
    int useMP;
} MPDE_MONTE_CARLO_RECORD;

// This struct contains the vectors holding the number of runs that went into each
// bifurcated path, as well as the mean value and mean path of each one

typedef struct {
    UINT32 *runsFirstCluster;
    UINT32 *runsSecondCluster;
    double *meansFirstCluster;
    double *meansSecondCluster;
    UINT32 *meanPathCluster1;
    UINT32 *meanPathCluster2;
    BOOL *isBifurcated;
} BIFURCATION_RECORD;

#define BIFURCATION_THRESHOLD 0.2
#define false 0
#define true 1

END_C_NAMESPACE

#endif
