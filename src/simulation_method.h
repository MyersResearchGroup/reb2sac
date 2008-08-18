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
#if !defined(HAVE_SIMULATION_METHOD)
#define HAVE_SIMULATION_METHOD

#include "common.h"
#include "compiler_def.h"
#include "back_end_processor.h"
#include "IR.h"
#include "kinetic_law_evaluater.h"
#include "kinetic_law_find_next_time.h"
#include "strconv.h"
#include "simulation_printer.h"
#include "simulation_run_termination_decider.h"



#define ODE_SIMULATION_ABSOLUTE_ERROR "ode.simulation.absolute.error"
#define DEFAULT_ODE_SIMULATION_ABSOLUTE_ERROR 1.0e-9

#define ODE_SIMULATION_TIME_LIMIT "ode.simulation.time.limit"
#define DEFAULT_ODE_SIMULATION_TIME_LIMIT_VALUE 30000.0

#define ODE_SIMULATION_PRINT_INTERVAL "ode.simulation.print.interval"
#define DEFAULT_ODE_SIMULATION_PRINT_INTERVAL_VALUE 5.0

#define ODE_SIMULATION_TIME_STEP "ode.simulation.time.step"
#define DEFAULT_ODE_SIMULATION_TIME_STEP 1.0

#define ODE_SIMULATION_OUT_DIR "ode.simulation.out.dir"
#define DEFAULT_ODE_SIMULATION_OUT_DIR_VALUE "."


#define MONTE_CARLO_SIMULATION_TIME_LIMIT "monte.carlo.simulation.time.limit"
#define DEFAULT_MONTE_CARLO_SIMULATION_TIME_LIMIT_VALUE 30000.0

#define MONTE_CARLO_SIMULATION_PRINT_INTERVAL "monte.carlo.simulation.print.interval"
#define DEFAULT_MONTE_CARLO_SIMULATION_PRINT_INTERVAL_VALUE 20.0

#define MONTE_CARLO_SIMULATION_TIME_STEP "monte.carlo.simulation.time.step"
#define DEFAULT_MONTE_CARLO_SIMULATION_TIME_STEP 1.0

#define MONTE_CARLO_SIMULATION_OUT_DIR "monte.carlo.simulation.out.dir"
#define DEFAULT_MONTE_CARLO_SIMULATION_OUT_DIR_VALUE "."

#define MONTE_CARLO_SIMULATION_RANDOM_SEED "monte.carlo.simulation.random.seed"
#define DEFAULT_MONTE_CARLO_SIMULATION_RANDOM_SEED_VALUE 12345

#define MONTE_CARLO_SIMULATION_RUNS "monte.carlo.simulation.runs"
#define DEFAULT_MONTE_CARLO_SIMULATION_RUNS_VALUE 1

#define MONTE_CARLO_CI_CONFIDENCE_LEVEL "monte.carlo.ci.confidence.level"
#define DEFAULT_MONTE_CARLO_CI_CONFIDENCE_LEVEL_VALUE 0.95
#define MONTE_CARLO_CI_ERROR_TOLERANCE "monte.carlo.ci.error.tolerance"
#define DEFAULT_MONTE_CARLO_CI_ERROR_TOLERANCE_VALUE 0.1

#define MONTE_CARLO_SIMULATION_START_INDEX  "monte.carlo.simulation.start.index"
#define DEFAULT_MONTE_CARLO_SIMULATION_START_INDEX  1

#define DEFAULT_MONTE_CARLO_SIMULATION_RUNS_VALUE 1

#define UNIFORM_RANDOM_BASE 1000000

BEGIN_C_NAMESPACE


END_C_NAMESPACE

#endif
