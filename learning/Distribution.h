/* -*- Mode: C++; -*- */
/* VER: $Id$*/
// copyright (c) 2004 by Christos Dimitrakakis <dimitrak@idiap.ch>
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DISTRIBUTION_H
#define DISTRIBUTION_H

#include "learning/real.h"
#include <math.h>
#include <stdbool.h>

void setRandomSeed(unsigned int seed);
real urandom();
real urandom_range(real min, real max);

/// Probability distribution
typedef struct Distribution_
{
} Distribution;
//public:
Distribution* Distribution_ctor();
/*virtual*/ void Distribution_dtor(Distribution *this);
/*virtual*/ real Distribution_generate(Distribution *this)/* = 0*/; ///< generate a value from this distribution
/*virtual*/ real Distribution_pdf(Distribution *this, real x)/* = 0*/; ///< return the density at point x

/// \brief Parametric distribution.
/// For distribution simply defined via moments-related parameters.
typedef struct ParametricDistribution_ /*: public Distribution*/
{
//public:
    Distribution *_d;
} ParametricDistribution;
//public:
ParametricDistribution* ParametricDistribution_ctor();
/*virtual*/ void ParametricDistribution_dtor(ParametricDistribution *this);
/*virtual*/ void ParametricDistribution_setVariance (ParametricDistribution *this, real var)/* = 0*/; ///< set the variance
/*virtual*/ void ParametricDistribution_setMean (ParametricDistribution *this, real mean)/* = 0*/; ///< set the mean


/// Discrete probability distribution
typedef struct DiscreteDistribution_ /*: public Distribution*/
{
    Distribution *_d;
//public:
	int n_outcomes; ///< number of possible outcomes
	real* p; ///< probabilities of outcomes
} DiscreteDistribution;
//public:
DiscreteDistribution* DiscreteDistribution_ctor1();
/// Make a discrete distribution with N outcomes
DiscreteDistribution* DiscreteDistribution_ctor2(int N);
/*virtual*/ void DiscreteDistribution_dtor(DiscreteDistribution *this);
/*virtual*/ real DiscreteDistribution_generate(DiscreteDistribution *this);
/*virtual*/ real DiscreteDistribution_pdf(DiscreteDistribution *this, real x);

/// Gaussian probability distribution
typedef struct NormalDistribution_ /*: public ParametricDistribution*/
{
//public:
    ParametricDistribution *_pd;
//private:
	bool cache;
	real normal_x, normal_y, normal_rho;
//public:
	real m; ///< mean
	real s; ///< standard deviation
} NormalDistribution;
//public:
NormalDistribution* NormalDistribution_ctor1();
/// Normal dist. with given mean and std
NormalDistribution* NormalDistribution_ctor2(real mean, real std);
/*virtual*/ void NormalDistribution_dtor(NormalDistribution *this);
/*virtual*/ real NormalDistribution_generate(NormalDistribution *this);
/*virtual*/ real NormalDistribution_pdf(NormalDistribution *this, real x);
/*virtual*/ void NormalDistribution_setVariance (NormalDistribution *this, real var);
/*virtual*/ void NormalDistribution_setMean (NormalDistribution *this, real mean);

/// Uniform probability distribution
typedef struct UniformDistribution_ /*: public ParametricDistribution*/
{
//public:
    ParametricDistribution *_pd;
//public:
	real m; ///< mean
	real s; ///< standard deviation
} UniformDistribution;
//public:
UniformDistribution* UniformDistribution_ctor1();
/// Create a uniform distribution with mean \c mean and standard deviation \c std
UniformDistribution* UniformDistribution_ctor2(real mean, real std);
/*virtual*/ void UniformDistribution_dtor(UniformDistribution *this);
/*virtual*/ real UniformDistribution_generate(UniformDistribution *this);
/*virtual*/ real UniformDistribution_pdf(UniformDistribution *this, real x);
/*virtual*/ void UniformDistribution_setVariance (UniformDistribution *this, real var);
/*virtual*/ void UniformDistribution_setMean (UniformDistribution *this, real mean);


/// Laplacian probability distribution
typedef struct LaplacianDistribution_ /*: public ParametricDistribution*/
{
//public:
    ParametricDistribution *_pd;
//public:
	real l; ///< lambda
	real m; ///< mean
} LaplacianDistribution;
//public:
LaplacianDistribution* LaplacianDistribution_ctor1();
/// Create a Laplacian distribution with parameter \c lambda
LaplacianDistribution* LaplacianDistribution_ctor2(real lambda);
LaplacianDistribution* LaplacianDistribution_ctor3(real mean, real var);
/*virtual*/ void LaplacianDistribution_dtor(LaplacianDistribution *this);
/*virtual*/ real LaplacianDistribution_generate(LaplacianDistribution *this);
/*virtual*/ real LaplacianDistribution_pdf(LaplacianDistribution *this, real x);
/*virtual*/ void LaplacianDistribution_setVariance (LaplacianDistribution *this, real var);
/*virtual*/ void LaplacianDistribution_setMean (LaplacianDistribution *this, real mean);

/// Exponential probability distribution
typedef struct ExponentialDistribution_ /*: public ParametricDistribution*/
{
    ParametricDistribution *_pd;
//public:
	real l; ///< lambda
	real m; ///< mean
} ExponentialDistribution;
//public:
ExponentialDistribution* ExponentialDistribution_ctor1();
/// Create an exponential distribution with parameter \c lambda
ExponentialDistribution* ExponentialDistribution_ctor2(real lambda);
ExponentialDistribution* ExponentialDistribution_ctor3(real mean, real var);
/*virtual*/ void ExponentialDistribution_dtor(ExponentialDistribution *this);
/*virtual*/ real ExponentialDistribution_generate(ExponentialDistribution *this);
/*virtual*/ real ExponentialDistribution_pdf(ExponentialDistribution *this, real x);
/*virtual*/ void ExponentialDistribution_setVariance (ExponentialDistribution *this, real var);
/*virtual*/ void ExponentialDistribution_setMean (ExponentialDistribution *this, real mean);

/// Multinomial gaussian probability distribution
typedef struct MultinomialGaussian_ /*: public Distribution*/
{
//public:
    Distribution *_d;
//public:
	int d; ///< number of dimensions
} MultinomialGaussian;
//public:
MultinomialGaussian* MultinomialGaussian_ctor1();
/// create a multinomial with \c d dimensions
MultinomialGaussian* MultinomialGaussian_ctor2(int d);
/*virtual*/ void MultinomialGaussian_dtor(MultinomialGaussian *this);
/*virtual*/ real MultinomialGaussian_generate();
/*virtual*/ real MultinomialGaussian_pdf(real x);

#endif
