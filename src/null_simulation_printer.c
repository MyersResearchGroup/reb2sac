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
#include "null_simulation_printer.h"

static RET_VAL _PrintStart( SIMULATION_PRINTER *printer, char *filenameStem );
static RET_VAL _PrintHeader( SIMULATION_PRINTER *printer );
static RET_VAL _PrintValues( SIMULATION_PRINTER *printer, double time );
static RET_VAL _PrintEnd( SIMULATION_PRINTER *printer );
static RET_VAL _Destroy( SIMULATION_PRINTER *printer );

 
DLLSCOPE SIMULATION_PRINTER * STDCALL CreateNullSimulationPrinter( BACK_END_PROCESSOR *backend, 
								   COMPARTMENT **compartmentArray, int compSize,
								   SPECIES **speciesArray, int size, 
								   REB2SAC_SYMBOL **symbolArray, int symSize,
								   BOOL isAmount ) {
    SIMULATION_PRINTER *printer = NULL;
    
    START_FUNCTION("CreateNullSimulationPrinter");
    
    if( ( printer = (SIMULATION_PRINTER*)MALLOC( sizeof(SIMULATION_PRINTER) ) ) == NULL ) {
        END_FUNCTION("CreateNullSimulationPrinter",  FAILING );
        return NULL;
    }
    
    printer->compartmentArray = compartmentArray;
    printer->compSize = compSize;
    printer->speciesArray = speciesArray;
    printer->size = size;
    printer->symbolArray = symbolArray;
    printer->symSize = symSize;
    printer->PrintStart = _PrintStart;
    printer->PrintHeader = _PrintHeader;
    printer->PrintValues = _PrintValues;
    printer->PrintEnd = _PrintEnd;
    printer->Destroy = _Destroy;
    
    END_FUNCTION("CreateNullSimulationPrinter",  SUCCESS );
    return printer;
}

static RET_VAL _PrintStart( SIMULATION_PRINTER *printer, char *filenameStem ) {
    RET_VAL ret = SUCCESS;
    return ret;            
}

static RET_VAL _PrintHeader( SIMULATION_PRINTER *printer ) {
    RET_VAL ret = SUCCESS;
    return ret;            
}

static RET_VAL _PrintValues( SIMULATION_PRINTER *printer, double time ) {
    RET_VAL ret = SUCCESS;
    return ret;            
}

static RET_VAL _PrintConcentrationValues( SIMULATION_PRINTER *printer, double time ) {
    RET_VAL ret = SUCCESS;
    return ret;            
}

static RET_VAL _PrintEnd( SIMULATION_PRINTER *printer ) {
    RET_VAL ret = SUCCESS;    
    return ret;            
}

static RET_VAL _Destroy( SIMULATION_PRINTER *printer ) {
    FREE( printer );
    return SUCCESS;
}


 
 
