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
#if !defined(HAVE_GNUPLOT_DAT_SIMULATION_PRINTER)
#define HAVE_GNUPLOT_DAT_SIMULATION_PRINTER

#include "simulation_printer.h"

BEGIN_C_NAMESPACE

#define GNUPLOT_DAT_SIMULATION_PRINTER "dat.printer"


DLLSCOPE SIMULATION_PRINTER * STDCALL CreateGnuplotDatSimulationPrinter( BACK_END_PROCESSOR *backend, 
									 COMPARTMENT **compartmentArray, int compSize,
									 SPECIES **speciesArray, int size, 
									 REB2SAC_SYMBOL **symbolArray, int symSize,
									 BOOL isAmount );

END_C_NAMESPACE

#endif
