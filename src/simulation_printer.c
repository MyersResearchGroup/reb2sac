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
#include "simulation_printer.h"
#include "tsd_simulation_printer.h"
#include "csv_simulation_printer.h"
#include "gnuplot_dat_simulation_printer.h"
#include "null_simulation_printer.h"

#define DEFAULT_SIMULATION_PRINTER TSD_SIMULATION_PRINTER

DLLSCOPE SIMULATION_PRINTER * STDCALL CreateSimulationPrinter( BACK_END_PROCESSOR *backend, 
							       COMPARTMENT **compartmentArray, UINT32 compSize,
							       SPECIES **speciesArray, UINT32 size, 
							       REB2SAC_SYMBOL **symbolArray, UINT32 symSize ) {
    int i = 0;
    SPECIES *species = NULL;
    SIMULATION_PRINTER *printer = NULL;
    char *valueString = NULL;
    COMPILER_RECORD_T *compRec = backend->record;
    REB2SAC_PROPERTIES *properties = NULL;
    BOOL isAmount = TRUE;
    
    START_FUNCTION("CreateSimulationPrinter");
    
    
    properties = compRec->properties;
    if( ( valueString = properties->GetProperty( properties, SIMULATION_PRINTER_TRACKING_QUANTITY_KEY ) ) == NULL ) {
        isAmount = TRUE;
    }
    else {
        if( strcmp( valueString, SIMULATION_PRINTER_TRACKING_QUANTITY_CONCENTRATION ) == 0 ) {
            isAmount = FALSE;
        }
        else {
            isAmount = TRUE;
        }
    }
    
    if( ( valueString = properties->GetProperty( properties, SIMULATION_PRINTER_KEY ) ) == NULL ) {
        valueString = DEFAULT_SIMULATION_PRINTER;
    }

    switch( valueString[0] ) {
        case 'c':
            if( strcmp( valueString, CSV_SIMULATION_PRINTER ) == 0 ) {
	      printer = CreateCsvSimulationPrinter( backend, compartmentArray, compSize,
						    speciesArray, size, 
						    symbolArray, symSize, isAmount );
            }
        break;
        
        case 'd':
            if( strcmp( valueString, GNUPLOT_DAT_SIMULATION_PRINTER ) == 0 ) {
	      printer = CreateGnuplotDatSimulationPrinter( backend, compartmentArray, compSize,
							   speciesArray, size, 
							   symbolArray, symSize, isAmount );
            }
        break;
        
        case 'n':
            if( strcmp( valueString, NULL_SIMULATION_PRINTER ) == 0 ) {
	      printer = CreateNullSimulationPrinter( backend, compartmentArray, compSize,
						     speciesArray, size, 
						     symbolArray, symSize, isAmount );
            }
        break;
        
        case 't':
            if( strcmp( valueString, TSD_SIMULATION_PRINTER ) == 0 ) {
	      printer = CreateTsdSimulationPrinter( backend, compartmentArray, compSize,
						    speciesArray, size, 
						    symbolArray, symSize, isAmount );
            }
        break;
        
        default:
            FREE( speciesArray );
            TRACE_1( "%s is not a valid simulation printer", valueString );
            END_FUNCTION("CreateSimulationPrinter",  FAILING );
            return NULL;        
        break;    
    }
        
    if( printer == NULL ) {
        FREE( speciesArray );
        TRACE_1( "failed to create %s", valueString );
        END_FUNCTION("CreateSimulationPrinter",  FAILING );
        return NULL;        
    }        
    
    END_FUNCTION("CreateSimulationPrinter",  SUCCESS );
    
    return printer;
}

DLLSCOPE RET_VAL STDCALL DestroySimulationPrinter( SIMULATION_PRINTER *printer ) {
    RET_VAL ret = SUCCESS;
    SPECIES **speciesArray = printer->speciesArray;
    
    START_FUNCTION("DestroySimulationPrinter");
    
    if( IS_FAILED( ( ret = printer->Destroy( printer ) ) ) ) {
        END_FUNCTION("DestroySimulationPrinter",  ret );
    }
    
    return SUCCESS;
}
 
 

