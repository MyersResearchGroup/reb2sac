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
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include "random_number_generator.h"

gsl_rng * r;

void CreateRandomNumberGenerators( ) {
  const gsl_rng_type * T;
     
  gsl_rng_env_setup();
     
  T = gsl_rng_default;
  r = gsl_rng_alloc(T);
}

void FreeRandomNumberGenerators( ) {

  gsl_rng_free(r);
}

void SeedRandomNumberGenerators( double s ) {

  gsl_rng_set(r,s);
}

double GetNextUniformRandomNumber( double minUniform, double maxUniform ) {
  return gsl_ran_flat( r, minUniform, maxUniform );
}

double GetNextUnitUniformRandomNumber( double minUniform, double maxUniform ) {
  return gsl_ran_flat( r, 0.0, 1.0 );
}

double GetNextNormalRandomNumber( double mean, double stdDeviation ) {
    double unitNormal = 0.0;
    double normal = 0.0;

    unitNormal = GetNextUnitNormalRandomNumber();
    normal = ( unitNormal * stdDeviation ) + mean;
    
    return normal;
}

double GetNextUnitNormalRandomNumber( ) {
  return gsl_ran_gaussian(r, 1.0);
  /*    static BOOL haveNextNormal = FALSE;
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
	} */
}

double GetNextExponentialRandomNumber( double lambda ) {
  return gsl_ran_exponential(r, 1/lambda);
}

double GetNextGammaRandomNumber( double a, double b ) {
    return gsl_ran_gamma(r, a, b);
}

double GetNextPoissonRandomNumber( double mu ) {
  return (double)gsl_ran_poisson(r, mu);
}

double GetNextBinomialRandomNumber( double p, unsigned int n ) {
  return (double)gsl_ran_binomial(r, p, n);
}

double GetNextLogNormalRandomNumber( double zeta, double sigma ) {
  return gsl_ran_lognormal(r, zeta, sigma);
}

double GetNextChiSquaredRandomNumber( double nu ) {
  return gsl_ran_chisq(r, nu);
}

double GetNextLaplaceRandomNumber( double a ) {
  return gsl_ran_laplace(r, a);
}

double GetNextCauchyRandomNumber( double a ) {
  return gsl_ran_cauchy(r, a);
}

double GetNextRayleighRandomNumber( double a ) {
  return gsl_ran_rayleigh(r, a);
}

double GetNextBernoulliRandomNumber( double a ) {
  return (double)gsl_ran_bernoulli(r, a);
}
