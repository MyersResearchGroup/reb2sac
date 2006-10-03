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
#if !defined(HAVE_SIMULATION_PRINTER)
#define HAVE_SIMULATION_PRINTER

#include "common.h"
#include "species_node.h"
#include "back_end_processor.h"

BEGIN_C_NAMESPACE

#define SIMULATION_PRINTER_KEY "simulation.printer"
#define SIMULATION_PRINTER_TRACKING_QUANTITY_KEY "simulation.printer.tracking.quantity"
#define SIMULATION_PRINTER_TRACKING_QUANTITY_AMOUNT "amount"
#define SIMULATION_PRINTER_TRACKING_QUANTITY_CONCENTRATION "concentration"



struct _SIMULATION_PRINTER;
typedef struct _SIMULATION_PRINTER SIMULATION_PRINTER;

struct _SIMULATION_PRINTER {
    SPECIES **speciesArray;
    int size;
    FILE *out;
    RET_VAL (*PrintStart)( SIMULATION_PRINTER *printer, char *filenameStem );
    RET_VAL (*PrintHeader)( SIMULATION_PRINTER *printer );
    RET_VAL (*PrintValues)( SIMULATION_PRINTER *printer, double time );
    RET_VAL (*PrintEnd)( SIMULATION_PRINTER *printer );
    RET_VAL (*Destroy)( SIMULATION_PRINTER *printer );
};

DLLSCOPE SIMULATION_PRINTER * STDCALL CreateSimulationPrinter( BACK_END_PROCESSOR *backend, SPECIES **speciesArray, UINT32 size );
DLLSCOPE RET_VAL STDCALL DestroySimulationPrinter( SIMULATION_PRINTER *printer );

END_C_NAMESPACE

#endif

