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
#include "uniformly_distributed_random_generator.h"
#include "exponentially_distributed_random_generator.h"


double GetNextExponentialRandomNumber( double lambda ) {
    double unitUniform = 0.0;
    double exponential = 0.0;
    
    unitUniform = GetNextUnitUniformRandomNumber();
    exponential = -log( unitUniform ) / lambda; 
    return exponential;
}

double GetNextUnitExponentialRandomNumber( ) {
    double unitUniform = 0.0;
    double unitExponential = 0.0;
    
    unitUniform = GetNextUnitUniformRandomNumber();
    unitExponential = -log( unitUniform ); 
    return unitExponential;
}

