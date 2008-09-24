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
#if !defined(HAVE_ABSTRACTION_METHOD_PROPERTIES)
#define HAVE_ABSTRACTION_METHOD_PROPERTIES

#include "common.h"

BEGIN_C_NAMESPACE

typedef struct {
    char *id;
    char *valueType;
    BOOL isMandatory;
    char *defaultValueString;
    char *info;
} ABSTRACTION_PROPERTY_INFO;

#define REB2SAC_ABSTRACTION_METHOD_KEY_PREFIX "reb2sac.abstraction.method."
#define REB2SAC_INTERESTING_SPECIES_KEY_PREFIX "reb2sac.interesting.species."

#define REB2SAC_PRINT_VARIABLES "reb2sac.print.variables"


#define REB2SAC_RNAP_MIN_CONCENTRATION_THRESHOLD_KEY "reb2sac.rnap.min.concentration.threshold"
#define DEFAULT_REB2SAC_RNAP_MIN_CONCENTRATION_THRESHOLD ((double)20.0)


#define REB2SAC_OPERATOR_MAX_CONCENTRATION_THRESHOLD_KEY "reb2sac.operator.max.concentration.threshold"
#define DEFAULT_REB2SAC_OPERATOR_MAX_CONCENTRATION_THRESHOLD ((double)2.0)


#define REB2SAC_ABSOLUTE_ACTIVATION_INHIBITION_THRESHOLD_KEY_PREFIX "reb2sac.absolute.activation.inhibition.threshold."

#define REB2SAC_ABSOLUTE_ACTIVATION_INHIBITION_DEFAULT_VALUE_TO_EVALUATE_MODIFIERS 0.5
#define REB2SAC_ABSOLUTE_ACTIVATION_INHIBITION_VALUE1_TO_EVALUATE_MODIFIERS 0.01
#define REB2SAC_ABSOLUTE_ACTIVATION_INHIBITION_VALUE2_TO_EVALUATE_MODIFIERS 10.0
#define REB2SAC_TYPE_ABSOLUTE_ACTIVATION 0
#define REB2SAC_TYPE_ABSOLUTE_INHIBITION 1

#define REB2SAC_ABSOLUTE_INHIBITION_THRESHOLD_KEY_PREFIX "reb2sac.absolute.inhibition.threshold."


#define REB2SAC_FINAL_STATE_KEY_PREFIX "reb2sac.final.state."
#define REB2SAC_TYPE_FINAL_STATE_HIGH 0
#define REB2SAC_TYPE_FINAL_STATE_LOW 1


#define REB2SAC_CRITICAL_CONCENTRATION_LEVEL_KEY_PREFIX "reb2sac.concentration.level."
#define REB2SAC_LOGICAL_REACTION_NAME_SIZE 4096
#define REB2SAC_LOGICAL_SPECIES_NAME_SIZE 1024

#define REB2SAC_CRITICAL_CONCENTRATION_LEVEL_SPECIFICATION_TYPE_KEY_PREFIX "reb2sac.concentration.level.specification.type."
#define REB2SAC_CRITICAL_CONCENTRATION_LEVEL_SPECIFICATION_TYPE_DEFAULT 0
#define REB2SAC_CRITICAL_CONCENTRATION_LEVEL_SPECIFICATION_TYPE_ONLY_PROPERTIES 1
#define REB2SAC_CRITICAL_CONCENTRATION_LEVEL_SPECIFICATION_TYPE_ONLY_CALCULATION 2

#define REB2SAC_CRITICAL_CONCENTRATION_LEVEL_SPECIFICATION_TYPE_ONLY_PROPERTIES_STRING "only.properties"
#define REB2SAC_CRITICAL_CONCENTRATION_LEVEL_SPECIFICATION_TYPE_ONLY_CALCULATION_STRING "only.calculation"

#define REB2SAC_NARY_ORDER_DECIDER_KEY "reb2sac.nary.order.decider"
#define REB2SAC_DEFAULT_NARY_ORDER_DECIDER "distinct"


#define REB2SAC_ANALYSIS_STOP_ENABLED_KEY "reb2sac.analysis.stop.enabled"
#define REB2SAC_ANALYSIS_STOP_ENABLED_VALUE_TRUE "true"
#define REB2SAC_ANALYSIS_STOP_ENABLED_VALUE_FALSE "false"
#define DEFAULT_REB2SAC_ANALYSIS_STOP_ENABLED_VALUE REB2SAC_ANALYSIS_STOP_ENABLED_VALUE_FALSE

#define REB2SAC_ANALYSIS_STOP_RATE_KEY "reb2sac.analysis.stop.rate"
#define DEFAULT_REB2SAC_ANALYSIS_STOP_RATE 0.0005

#define REB2SAC_STOP_SPECIES_NAME_PREFIX "stop"
#define REB2SAC_STOP_REACTION_NAME "_stop_analysis"

#define REB2SAC_CRITICAL_CONCENTRATION_AMPLIFIER_KEY "reb2sac.critical.concentration.amplifier"
#define DEFAULT_REB2SAC_CRITICAL_CONCENTRATION_AMPLIFIER 0.5


#define REB2SAC_RAPID_EQUILIBRIUM_CONDITION_1_KEY "reb2sac.rapid.equilibrium.condition.1"
#define DEFAULT_REB2SAC_RAPID_EQUILIBRIUM_CONDITION_1 0.5

#define REB2SAC_RAPID_EQUILIBRIUM_CONDITION_2_KEY "reb2sac.rapid.equilibrium.condition.2"
#define DEFAULT_REB2SAC_RAPID_EQUILIBRIUM_CONDITION_2 0.5

#define REB2SAC_QSSA_CONDITION_1_KEY "reb2sac.qssa.condition.1"
#define DEFAULT_REB2SAC_QSSA_CONDITION_1 0.5

#define REB2SAC_STOICHIOMETRY_AMPLIFIER_KEY "reb2sac.stoichiometry.amplifier"
#define DEFAULT_REB2SAC_STOICHIOMETRY_AMPLIFIER 2

#define REB2SAC_DEGRADATION_STOICHIOMETRY_AMPLIFIER_KEY "reb2sac.degradation.stoichiometry.amplifier"
#define DEFAULT_REB2SAC_DEGRADATION_STOICHIOMETRY_AMPLIFIER 2

#define REB2SAC_MAX_SPECIES_OSCILLATION_AMOUNT_KEY_PREFIX "reb2sac.max.oscillation.amount."
#define REB2SAC_MIN_SPECIES_OSCILLATION_AMOUNT_KEY_PREFIX "reb2sac.min.oscillation.amount."
#define REB2SAC_MAX_SPECIES_OSCILLATION_REACTION_RATE_KEY "reb2sac.max.oscillation.reaction.rate"
#define DEFAULT_REB2SAC_MAX_SPECIES_OSCILLATION_REACTION_RATE 9999999.99




END_C_NAMESPACE

#endif
