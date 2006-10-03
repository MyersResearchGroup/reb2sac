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
#include <math.h>

#include "random_number_generator.h"
#include "uniformly_distributed_random_generator.h"
#include "exponentially_distributed_random_generator.h"
#include "normally_distributed_random_generator.h"



static RET_VAL _SetSeed( UINT seed );
static double _GetNextUniform( double min, double max );
static double _GetNextUnitUniform( );
static double _GetNextExponential( double lambda );
static double _GetNextUnitExponential( );
static double _GetNextNormal( double mean, double stdDeviation );
static double _GetNextUnitNormal( );



RANDOM_NUMBER_GENERATOR *CreateRandomNumberGenerator( ) {
    static RANDOM_NUMBER_GENERATOR _randomNumberGeneratorInstance;
    
    if( _randomNumberGeneratorInstance.SetSeed == NULL ) {
        _randomNumberGeneratorInstance.SetSeed = _SetSeed;
        _randomNumberGeneratorInstance.GetNextUniform = _GetNextUniform;
        _randomNumberGeneratorInstance.GetNextUnitUniform = _GetNextUnitUniform;
        _randomNumberGeneratorInstance.GetNextExponential = _GetNextExponential;
        _randomNumberGeneratorInstance.GetNextUnitExponential = _GetNextUnitExponential;
        _randomNumberGeneratorInstance.GetNextNormal = _GetNextNormal;
        _randomNumberGeneratorInstance.GetNextUnitNormal = _GetNextUnitNormal;
    }
    return &_randomNumberGeneratorInstance;
}

RET_VAL FreeRandomNumberGenerator( RANDOM_NUMBER_GENERATOR **generator ) {
    (*generator)->SetSeed = NULL;
    return SUCCESS;
}

static RET_VAL _SetSeed( UINT seed ) {
    srand( seed );
    return SUCCESS;
}

static double _GetNextUniform( double min, double max ) {
    return GetNextUniformRandomNumber( min, max );
}

static double _GetNextUnitUniform( ) {
    return GetNextUnitUniformRandomNumber();
}

static double _GetNextExponential( double lambda ) {
    return GetNextExponentialRandomNumber( lambda ); 
}

static double _GetNextUnitExponential( ) {
    return GetNextUnitExponentialRandomNumber();
}

static double _GetNextNormal( double mean, double stdDeviation ) {
    return GetNextNormalRandomNumber( mean, stdDeviation );
}

static double _GetNextUnitNormal( ) {
    return GetNextUnitNormalRandomNumber();
}



