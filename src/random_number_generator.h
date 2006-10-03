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
#if !defined(HAVE_RANDOM_NUMBER_GENERATOR)
#define HAVE_RANDOM_NUMBER_GENERATOR


#include "common.h"

BEGIN_C_NAMESPACE

/*
typedef union {
    long discreteValue;
    double continuousValue;
} RANDOM_T;
*/

struct _RANDOM_NUMBER_GENERATOR;
typedef struct _RANDOM_NUMBER_GENERATOR  RANDOM_NUMBER_GENERATOR;

struct _RANDOM_NUMBER_GENERATOR {
    RET_VAL (*SetSeed)( UINT seed );

    double (*GetNextUniform)( double min, double max );
    double (*GetNextUnitUniform)( );

    double (*GetNextExponential)( double lambda );
    double (*GetNextUnitExponential)( );

    double (*GetNextNormal)( double mean, double stdDeviation );
    double (*GetNextUnitNormal)( );
/*
    UINT seed;
    double minUniform;
    double rangeUniform;
    double meanNormal;
    double stdDeviationNormal;
*/
};

RANDOM_NUMBER_GENERATOR *CreateRandomNumberGenerator( );
RET_VAL FreeRandomNumberGenerator( RANDOM_NUMBER_GENERATOR **generator );



END_C_NAMESPACE

#endif
