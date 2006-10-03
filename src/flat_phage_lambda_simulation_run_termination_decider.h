/***************************************************************************
 *   Copyright (C) 2004 by Hiroyuki Kuwahara                               *
 *   kuwahara@cs.utah.edu                                                 *
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
#if !defined(HAVE_FLAT_PHAGE_LAMBDA_SIMULATION_RUN_TERMINATION_DECIDER)
#define HAVE_FLAT_PHAGE_LAMBDA_SIMULATION_RUN_TERMINATION_DECIDER

#include "simulation_run_termination_decider.h"

BEGIN_C_NAMESPACE

/*
<listOfSpecies>
<species id = "cI2" name = "cI2" compartment = "default" initialConcentration = "0.0"/>
<species id = "Cro2" name = "Cro2" compartment = "default" initialConcentration = "0.0"/>
<species id = "MOI" name = "MOI" compartment = "default" initialConcentration = "12"/>
<species id = "P1cIII" name = "P1cIII" compartment = "default" initialConcentration = "0.0"/>
<species id = "ORCrocI" name = "ORCrocI" compartment = "default" initialConcentration = "0.0"/>
<species id = "NUTRN4" name = "NUTRN4" compartment = "default" initialConcentration = "0.0"/>
<species id = "OR13RNAPcI" name = "OR13RNAPcI" compartment = "default" initialConcentration = "0.0"/>
<species id = "Kd" name = "Kd" compartment = "default" initialConcentration = "1.0"/>
<species id = "ORCro2cI" name = "ORCro2cI" compartment = "default" initialConcentration = "0.0"/>
<species id = "N" name = "N" compartment = "default" initialConcentration = "0.0"/>
<species id = "ORcI" name = "ORcI" compartment = "default" initialConcentration = "0.0"/>
<species id = "PRECro" name = "PRECro" compartment = "default" initialConcentration = "0.0"/>
<species id = "OR" name = "OR" compartment = "default" initialConcentration = "1.0"/>
<species id = "ORCro" name = "ORCro" compartment = "default" initialConcentration = "0.0"/>
<species id = "PRE" name = "PRE" compartment = "default" initialConcentration = "1.0"/>
<species id = "ORRNAP2CrocI" name = "ORRNAP2CrocI" compartment = "default" initialConcentration = "0.0"/>
<species id = "NUTRN2" name = "NUTRN2" compartment = "default" initialConcentration = "0.0"/>
<species id = "OLCro" name = "OLCro" compartment = "default" initialConcentration = "0.0"/>
<species id = "P2cIII" name = "P2cIII" compartment = "default" initialConcentration = "0.0"/>
<species id = "ORRNAPcICro" name = "ORRNAPcICro" compartment = "default" initialConcentration = "0.0"/>
<species id = "OR2RNAP" name = "OR2RNAP" compartment = "default" initialConcentration = "0.0"/>
<species id = "P2" name = "P2" compartment = "default" initialConcentration = "100.0"/>
<species id = "OR2cI" name = "OR2cI" compartment = "default" initialConcentration = "0.0"/>
<species id = "OR3cI" name = "OR3cI" compartment = "default" initialConcentration = "0.0"/>
<species id = "PREcIIRNAP" name = "PREcIIRNAP" compartment = "default" initialConcentration = "0.0"/>
<species id = "ORRNAPCro" name = "ORRNAPCro" compartment = "default" initialConcentration = "0.0"/>
<species id = "NUTR4" name = "NUTR4" compartment = "default" initialConcentration = "1.0"/>
<species id = "NUTL" name = "NUTL" compartment = "default" initialConcentration = "1.0"/>
<species id = "OR12RNAP" name = "OR12RNAP" compartment = "default" initialConcentration = "0.0"/>
<species id = "OLcI" name = "OLcI" compartment = "default" initialConcentration = "0.0"/>
<species id = "ORCroRNAP" name = "ORCroRNAP" compartment = "default" initialConcentration = "0.0"/>
<species id = "Cro" name = "Cro" compartment = "default" initialConcentration = "0.0"/>
<species id = "ORRNAP2Cro" name = "ORRNAP2Cro" compartment = "default" initialConcentration = "0.0"/>
<species id = "NUTRN3" name = "NUTRN3" compartment = "default" initialConcentration = "0.0"/>
<species id = "Basal_error" name = "Basal_error" compartment = "default" initialConcentration = "1.0"/>
<species id = "ORcIRNAP" name = "ORcIRNAP" compartment = "default" initialConcentration = "0.0"/>
<species id = "cIII" name = "cIII" compartment = "default" initialConcentration = "0.0"/>
<species id = "P1cII" name = "P1cII" compartment = "default" initialConcentration = "0.0"/>
<species id = "PREcII" name = "PREcII" compartment = "default" initialConcentration = "0.0"/>
<species id = "OLcICro" name = "OLcICro" compartment = "default" initialConcentration = "0.0"/>
<species id = "OL" name = "OL" compartment = "default" initialConcentration = "1.0"/>
<species id = "OR3RNAP" name = "OR3RNAP" compartment = "default" initialConcentration = "0.0"/>
<species id = "OL2Cro" name = "OL2Cro" compartment = "default" initialConcentration = "0.0"/>
<species id = "cI" name = "cI" compartment = "default" initialConcentration = "0.0"/>
<species id = "OR2Cro" name = "OR2Cro" compartment = "default" initialConcentration = "0.0"/>
<species id = "ORRNAP2cI" name = "ORRNAP2cI" compartment = "default" initialConcentration = "0.0"/>
<species id = "OL2cI" name = "OL2cI" compartment = "default" initialConcentration = "0.0"/>
<species id = "NUTLN" name = "NUTLN" compartment = "default" initialConcentration = "0.0"/>
<species id = "OR2CrocI" name = "OR2CrocI" compartment = "default" initialConcentration = "0.0"/>
<species id = "P1" name = "P1" compartment = "default" initialConcentration = "40.0"/>
<species id = "RNAP" name = "RNAP" compartment = "default" initialConcentration = "30.0"/>
<species id = "NUTR" name = "NUTR" compartment = "default" initialConcentration = "1.0"/>
<species id = "NUTR3" name = "NUTR3" compartment = "default" initialConcentration = "1.0"/>
<species id = "OLRNAP" name = "OLRNAP" compartment = "default" initialConcentration = "0.0"/>
<species id = "PRERNAP" name = "PRERNAP" compartment = "default" initialConcentration = "0.0"/>
<species id = "cII" name = "cII" compartment = "default" initialConcentration = "0.0"/>
<species id = "P2cII" name = "P2cII" compartment = "default" initialConcentration = "0.0"/>
<species id = "ORRNAPcI" name = "ORRNAPcI" compartment = "default" initialConcentration = "0.0"/>
<species id = "OR3Cro" name = "OR3Cro" compartment = "default" initialConcentration = "0.0"/>
<species id = "NUTR2" name = "NUTR2" compartment = "default" initialConcentration = "1.0"/>
<species id = "NUTRN" name = "NUTRN" compartment = "default" initialConcentration = "0.0"/>
</listOfSpecies>
*/

/*
typedef struct {
    char *name;
    int index;
    int number;
} FLAT_PHAGE_LAMBDA_SPECIES_OF_INTEREST_RECORD; 

static FLAT_PHAGE_LAMBDA_SPECIES_OF_INTEREST_RECORD FLAT_PHAGE_LAMBDA_CI2_NAMES[] = { 
    {"cI2", -1, 1},
    {"ORCrocI", -1,  1},
    {"OR13RNAPcI", -1,  1},
    {"ORCro2cI", -1,  2},
    {"ORcI", -1,  1},
    {"ORRNAP2CrocI", -1,  1},
    {"ORRNAPcICro", -1,  1},
    {"OR2cI", -1,  2},
    {"OR3cI", -1,  3},
    {"OLcI", -1,  1},
    {"ORcIRNAP", -1,  1},
    {"OLcICro", -1,  1},
    {"ORRNAP2cI", -1,  2},
    {"OL2cI", -1,  2},
    {"OR2CrocI", -1,  1},
    {"ORRNAPcI", -1,  1}
};

static FLAT_PHAGE_LAMBDA_SPECIES_OF_INTEREST_RECORD FLAT_PHAGE_LAMBDA_CRO2_NAMES[] = {
    {"Cro2", -1,  1},
    {"ORCrocI", -1,  1},
    {"ORCro2cI", -1,  1},
    {"PRECro", -1,  1},
    {"ORCro", -1,  1},
    {"ORRNAP2CrocI", -1,  2},
    {"OLCro", -1,  1},
    {"ORRNAPcICro", -1,  1},
    {"ORRNAPCro", -1,  1},
    {"ORCroRNAP", -1,  1},
    {"ORRNAP2Cro", -1,  2},
    {"OLcICro", -1,  1},
    {"OL2Cro", -1,  2},
    {"OR2Cro", -1,  2},
    {"OR2CrocI", -1,  2},
    {"OR3Cro", -1,  3}    
};
*/


typedef struct {
    SPECIES *species;
    int number;
} FLAT_PHAGE_LAMBDA_SPECIES_OF_INTEREST_ELEMENT; 

typedef struct {
    FLAT_PHAGE_LAMBDA_SPECIES_OF_INTEREST_ELEMENT **elements;
    int size;
} FLAT_PHAGE_LAMBDA_SPECIES_OF_INTEREST_ARRAY; 

#define FLAT_PHAGE_LAMBDA_SIMULATION_RUN_TERMINATION_DECIDER_ID "flat.phage.lambda.1"
#define FLAT_PHAGE_LAMBDA_NAME_OF_SPECIES_OF_INTEREST_FOR_LYSOGENY "cI2"
#define FLAT_PHAGE_LAMBDA_NAME_OF_SPECIES_OF_INTEREST_FOR_LYSIS "Cro2"

#define FLAT_PHAGE_LAMBDA_CI2_THRESHOLD_KEY "flat.phage.lambda.cI2.threshold"
#define FLAT_PHAGE_LAMBDA_CRO2_THRESHOLD_KEY "flat.phage.lambda.cro2.threshold"

#define DEFAULT_FLAT_PHAGE_LAMBDA_CI2_THRESHOLD_VALUE 145.0
#define DEFAULT_FLAT_PHAGE_LAMBDA_CRO2_THRESHOLD_VALUE 55.0


struct _FLAT_PHAGE_LAMBDA_SIMULATION_RUN_TERMINATION_DECIDER;
typedef struct _FLAT_PHAGE_LAMBDA_SIMULATION_RUN_TERMINATION_DECIDER FLAT_PHAGE_LAMBDA_SIMULATION_RUN_TERMINATION_DECIDER;

struct _FLAT_PHAGE_LAMBDA_SIMULATION_RUN_TERMINATION_DECIDER {
    SPECIES **speciesArray;
    int size;
    REACTION **reactionArray;
    int reactionSize;
    double timeLimit;
    BOOL (*IsTerminationConditionMet)( SIMULATION_RUN_TERMINATION_DECIDER *decider, REACTION *reaction, double time );
    RET_VAL (*Report)( SIMULATION_RUN_TERMINATION_DECIDER *decider, FILE *file );
    RET_VAL (*Destroy)( SIMULATION_RUN_TERMINATION_DECIDER *decider );    
    
    FLAT_PHAGE_LAMBDA_SPECIES_OF_INTEREST_ARRAY *lysogenySpeciesArray;
    FLAT_PHAGE_LAMBDA_SPECIES_OF_INTEREST_ARRAY *lysisSpeciesArray;
    double cI2Threshold;
    double cro2Threshold;
    int lysogenyNum;
    int lysisNum;     
};



DLLSCOPE SIMULATION_RUN_TERMINATION_DECIDER * STDCALL 
    CreateFlatPhageLambdaSimulationRunTerminationDecider( 
        BACK_END_PROCESSOR *backend, SPECIES **speciesArray, int size, 
        REACTION **reactionArray, int reactionSize, double timeLimit );


END_C_NAMESPACE

#endif

