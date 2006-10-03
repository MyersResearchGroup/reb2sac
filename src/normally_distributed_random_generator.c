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
#include "uniformly_distributed_random_generator.h"
#include "normally_distributed_random_generator.h"



double GetNextNormalRandomNumber( double mean, double stdDeviation ) {
    double unitNormal = 0.0;
    double normal = 0.0;

    unitNormal = GetNextUnitNormalRandomNumber();
    normal = ( unitNormal * stdDeviation ) + mean;
    
    return normal;
}

double GetNextUnitNormalRandomNumber( ) {
    static BOOL haveNextNormal = FALSE;
    static double nextNormal = 0.0;
    
    double v1;
    double v2;
    double s;
    double multiplier;
    double unitNormal;

    if( haveNextNormal ) {
        haveNextNormal = FALSE;
        return nextNormal;
    }
    else {
        do {
            v1 = 2 * GetNextUnitUniformRandomNumber() - 1.0;
            v2 = 2 * GetNextUnitUniformRandomNumber() - 1.0;
            s = v1 * v1 + v2 * v2;
        } while( s >= 1.0 || s == 0.0 );
        multiplier = sqrt( -2.0 * log( s ) / s );
        haveNextNormal = TRUE;
        nextNormal = v2 * multiplier;
        unitNormal = v1 * multiplier;
        return unitNormal;
    }
}
