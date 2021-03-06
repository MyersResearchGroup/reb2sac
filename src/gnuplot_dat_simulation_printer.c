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
#include "gnuplot_dat_simulation_printer.h"

static RET_VAL _PrintStart( SIMULATION_PRINTER *printer, char *filenameStem );
static RET_VAL _PrintHeader( SIMULATION_PRINTER *printer );
static RET_VAL _PrintValues( SIMULATION_PRINTER *printer, double time );
static RET_VAL _PrintConcentrationValues( SIMULATION_PRINTER *printer, double time );
static RET_VAL _PrintEnd( SIMULATION_PRINTER *printer );
static RET_VAL _Destroy( SIMULATION_PRINTER *printer );

 
DLLSCOPE SIMULATION_PRINTER * STDCALL CreateGnuplotDatSimulationPrinter( BACK_END_PROCESSOR *backend, 
									 COMPARTMENT **compartmentArray, int compSize,
									 SPECIES **speciesArray, int size, 
									 REB2SAC_SYMBOL **symbolArray, int symSize,
									 BOOL isAmount ) {
    SIMULATION_PRINTER *printer = NULL;
    
    START_FUNCTION("CreateGnuplotDatSimulationPrinter");
    
    if( ( printer = (SIMULATION_PRINTER*)MALLOC( sizeof(SIMULATION_PRINTER) ) ) == NULL ) {
        END_FUNCTION("CreateGnuplotDatSimulationPrinter",  FAILING );
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
    if( isAmount ) {
        printer->PrintValues = _PrintValues;
    }
    else {
        printer->PrintValues = _PrintConcentrationValues;
    }
    printer->PrintEnd = _PrintEnd;
    printer->Destroy = _Destroy;
    
    END_FUNCTION("CreateGnuplotDatSimulationPrinter",  SUCCESS );
    return printer;
}

static RET_VAL _PrintStart( SIMULATION_PRINTER *printer, char *filenameStem ) {
    RET_VAL ret = SUCCESS;
    char filename[1024];
    FILE *out = NULL;
    
    sprintf( filename, "%s.dat", filenameStem );
    if( ( out = fopen( filename, "w" ) ) == NULL ) {
        TRACE_1( "could not open %s", filename );
        return FAILING;
    }
    
    printer->out = out;    
    
    return ret;            
}

static RET_VAL _PrintHeader( SIMULATION_PRINTER *printer ) {
    RET_VAL ret = SUCCESS;
    FILE *out = printer->out;
    int i = 0;
    int j = 0;
    int compSize = printer->compSize;
    COMPARTMENT **compartmentArray = printer->compartmentArray;
    int size = printer->size;
    SPECIES **speciesArray = printer->speciesArray;
    int symSize = printer->symSize;
    REB2SAC_SYMBOL **symbolArray = printer->symbolArray;

    fprintf( out, "#(1, time)" );    
    j = 2;
    for( i = 0; i < compSize; i++ ) {
      if (PrintCompartment( compartmentArray[i] )) {
        fprintf( out, ", (%i, %s)", j,GetCharArrayOfString(GetCompartmentID( compartmentArray[i] )) );
	j++;
      }
    }
    for( i = 0; i < size; i++ ) {
      if (IsPrintFlagSetInSpeciesNode(speciesArray[i])) {
        fprintf( out, ", (%i, %s)", j, GetCharArrayOfString(GetSpeciesNodeName( speciesArray[i] )) );
	j++;
      }
    }                 
    for( i = 0; i < symSize; i++ ) {
      if (PrintSymbol( symbolArray[i] )) {
        fprintf( out, ", (%i, %s)", j, GetCharArrayOfString(GetSymbolID( symbolArray[i] )) );
	j++;
      }
    }
    fprintf( out, NEW_LINE );
    
    return ret;            
}

static RET_VAL _PrintValues( SIMULATION_PRINTER *printer, double time ) {
    RET_VAL ret = SUCCESS;
    FILE *out = printer->out;
    int i = 0;
    int compSize = printer->compSize;
    COMPARTMENT **compartmentArray = printer->compartmentArray;
    int size = printer->size;
    SPECIES **speciesArray = printer->speciesArray;
    int symSize = printer->symSize;
    REB2SAC_SYMBOL **symbolArray = printer->symbolArray;
    
    fprintf( out, "%.12g", time );
    for( i = 0; i < compSize; i++ ) {
      if (PrintCompartment( compartmentArray[i] )) {
        fprintf( out, " %.12g", GetCurrentSizeInCompartment( compartmentArray[i] ) );
      }
    }                 
    for( i = 0; i < size; i++ ) {
      if (IsPrintFlagSetInSpeciesNode(speciesArray[i])) {
	fprintf( out, " %.12g", GetAmountInSpeciesNode( speciesArray[i] ) );
      }
    }                 
    for( i = 0; i < symSize; i++ ) {
      if (PrintSymbol( symbolArray[i] )) {
        fprintf( out, " %.12g", GetCurrentRealValueInSymbol( symbolArray[i] ) );
      }
    }                 
    fprintf( out, NEW_LINE );
    
    return ret;            
}

static RET_VAL _PrintConcentrationValues( SIMULATION_PRINTER *printer, double time ) {
    RET_VAL ret = SUCCESS;
    FILE *out = printer->out;
    int i = 0;
    int compSize = printer->compSize;
    COMPARTMENT **compartmentArray = printer->compartmentArray;
    int size = printer->size;
    SPECIES **speciesArray = printer->speciesArray;
    int symSize = printer->symSize;
    REB2SAC_SYMBOL **symbolArray = printer->symbolArray;
    
    fprintf( out, "%.12g", time );
    for( i = 0; i < compSize; i++ ) {
      if (PrintCompartment( compartmentArray[i] )) {
        fprintf( out, " %.12g", GetCurrentSizeInCompartment( compartmentArray[i] ) );
      }
    }                 
    for( i = 0; i < size; i++ ) {
      if (IsPrintFlagSetInSpeciesNode(speciesArray[i])) {
        fprintf( out, " %.12g", GetConcentrationInSpeciesNode( speciesArray[i] ) );
      }
    }                 
    for( i = 0; i < symSize; i++ ) {
      if (PrintSymbol( symbolArray[i] )) {
        fprintf( out, " %.12g", GetCurrentRealValueInSymbol( symbolArray[i] ) );
      }
    }                 
    fprintf( out, NEW_LINE );
    
    return ret;            
}

static RET_VAL _PrintEnd( SIMULATION_PRINTER *printer ) {
    RET_VAL ret = SUCCESS;    
    FILE *out = printer->out;
    
    fflush( out );
    fclose( out );
    
    return ret;            
}

static RET_VAL _Destroy( SIMULATION_PRINTER *printer ) {
    FREE( printer );
    return SUCCESS;
}


 
 

 
 
