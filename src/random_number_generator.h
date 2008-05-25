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

void CreateRandomNumberGenerators( );
void SeedRandomNumberGenerators( double s );
void FreeRandomNumberGenerators( );

double GetNextUniformRandomNumber( double minUniform, double maxUniform );
double GetNextUnitUniformRandomNumber( );

double GetNextNormalRandomNumber( double mean, double stdDeviation );
double GetNextUnitNormalRandomNumber( );

double GetNextGammaRandomNumber( double a, double b );

double GetNextExponentialRandomNumber( double lambda );

double GetNextPoissonRandomNumber( double mu );

double GetNextBinomialRandomNumber( double p, unsigned int n );

double GetNextLogNormalRandomNumber( double zeta, double sigma );

double GetNextLogNormalRandomNumber( double zeta, double sigma );

double GetNextChiSquaredRandomNumber( double nu );

double GetNextLaplaceRandomNumber( double a );

double GetNextCauchyRandomNumber( double a );

double GetNextRayleighRandomNumber( double a );

double GetNextBernoulliRandomNumber( double a );

END_C_NAMESPACE

#endif
