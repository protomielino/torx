/* -*- Mode: C++; -*- */
// VER: $Id$
// copyright (c) 2004 by Christos Dimitrakakis <dimitrak@idiap.ch>
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "learning/Distribution.h"

static const double PI = 3.14159265358979323846;

void setRandomSeed(unsigned int seed)
{
	srand(seed);
}
real urandom()
{
	real x;
	do {
		x = ((real) rand())/((real) (RAND_MAX));
	} while (x==1.0);
	return x;
}

real urandom_range(real min, real max)
{
	return min + ((max-min)*urandom());
}

//public:
Distribution* Distribution_ctor()
{
    Distribution *this = calloc(1, sizeof(Distribution));
    return this;
}
/*virtual*/ void Distribution_dtor(Distribution *this) {}
/*virtual*/ real Distribution_generate(Distribution *this) {return 0;}
/*virtual*/ real Distribution_pdf(Distribution *this, real x) {return 0;}

//public:
ParametricDistribution* ParametricDistribution_ctor()
{
    ParametricDistribution *this = calloc(1, sizeof(ParametricDistribution));
    return this;
}
/*virtual*/ void ParametricDistribution_dtor(ParametricDistribution *this) {}
/*virtual*/ void ParametricDistribution_setVariance (ParametricDistribution *this, real var)/* = 0*/  {}
/*virtual*/ void ParametricDistribution_setMean (ParametricDistribution *this, real mean)/* = 0*/  {}

//public:
UniformDistribution* UniformDistribution_ctor1()
{
    UniformDistribution *this = calloc(1, sizeof(UniformDistribution));
    this->m=0.0;
    this->s=1.0;
    return this;
}
/// Create a uniform distribution with mean \c mean and standard deviation \c std
UniformDistribution* UniformDistribution_ctor2(real mean, real std)
{
    UniformDistribution *this = calloc(1, sizeof(UniformDistribution));
    UniformDistribution_setMean (this, mean);
    UniformDistribution_setVariance (this, std*std);
    return this;
}
/*virtual*/ void UniformDistribution_dtor(UniformDistribution *this) {}
/*virtual*/ //real UniformDistribution_generate(UniformDistribution *this);
/*virtual*/ //real UniformDistribution_pdf(UniformDistribution *this, real x);
/*virtual*/ void UniformDistribution_setVariance (UniformDistribution *this, real var)
{this->s = sqrt(12.0 * var);}
/*virtual*/ void UniformDistribution_setMean (UniformDistribution *this, real mean)
{this->m = mean;}

real UniformDistribution_generate(UniformDistribution *this)
{
	return this->m + (urandom()-0.5)*this->s;
}

real UniformDistribution_pdf(UniformDistribution *this, real x)
{
	real dx=x-this->m;
	real sh=.5*this->s;
	if ((dx > -sh)&&(dx <= sh))
		return 1.0/this->s;
	return 0.0;
}



//public:
NormalDistribution* NormalDistribution_ctor1()
{
    NormalDistribution *this = calloc(1, sizeof(NormalDistribution));
    this->m=0.0;
    this->s=1.0;
    this->cache = false;
    return this;
}
/// Normal dist. with given mean and std
NormalDistribution* NormalDistribution_ctor2(real mean, real std)
{
    NormalDistribution *this = calloc(1, sizeof(NormalDistribution));
    NormalDistribution_setMean (this, mean);
    NormalDistribution_setVariance (this, std*std);
    return this;
}
/*virtual*/ void NormalDistribution_dtor(NormalDistribution *this) { }
/*virtual*/ //real NormalDistribution_generate(NormalDistribution *this);
/*virtual*/ //real NormalDistribution_pdf(NormalDistribution *this, real x);
/*virtual*/ void NormalDistribution_setVariance (NormalDistribution *this, real var)
{this->s = sqrt(var);}
/*virtual*/ void NormalDistribution_setMean (NormalDistribution *this, real mean)
{this->m = mean;}
// Taken from numerical recipes in C
real NormalDistribution_generate(NormalDistribution *this)
{
	if(!this->cache) {
	    this->normal_x = urandom();
	    this->normal_y = urandom();
	    this->normal_rho = sqrt(-2.0 * log(1.0 - this->normal_y));
	    this->cache = true;
	} else {
	    this->cache = false;
	}
	
	if (this->cache) {
		return this->normal_rho * cos(2.0 * PI * this->normal_x) * this->s + this->m;
	} else {
		return this->normal_rho * sin(2.0 * PI * this->normal_x) * this->s + this->m;
	}
}

real NormalDistribution_pdf(NormalDistribution *this, real x)
{
	real d = (this->m-x)/this->s;
	return exp(-0.5 * d*d)/(sqrt(2.0 * PI) * this->s);
}

//public:
LaplacianDistribution* LaplacianDistribution_ctor1()
{
    LaplacianDistribution *this = calloc(1, sizeof(LaplacianDistribution));
    this->m=0.0;
    this->l=1.0;
    return this;
}
/// Create a Laplacian distribution with parameter \c lambda
LaplacianDistribution* LaplacianDistribution_ctor2(real lambda)
{
    LaplacianDistribution *this = calloc(1, sizeof(LaplacianDistribution));
    this->m = 0.0;
    this->l = lambda;
    return this;
}
LaplacianDistribution* LaplacianDistribution_ctor3(real mean, real var)
{
    LaplacianDistribution *this = calloc(1, sizeof(LaplacianDistribution));
    LaplacianDistribution_setMean (this, mean);
    LaplacianDistribution_setVariance (this, var);
    return this;
}
/*virtual*/ void LaplacianDistribution_dtor(LaplacianDistribution *this) {}
/*virtual*/ //real LaplacianDistribution_generate(LaplacianDistribution *this);
/*virtual*/ //real LaplacianDistribution_pdf(LaplacianDistribution *this, real x);
/*virtual*/ void LaplacianDistribution_setVariance (LaplacianDistribution *this, real var)
{this->l = sqrt(0.5 / var);}
/*virtual*/ void LaplacianDistribution_setMean (LaplacianDistribution *this, real mean)
{this->m = mean;}

real LaplacianDistribution_generate(LaplacianDistribution *this)
{
	real x = urandom(-1.0, 1.0);
	real absx = fabs (x);
	real sgnx;
	if (x>0.0) {
		sgnx = 1.0;
	} else {
		sgnx = -1.0;
	}
	
	return this->m + sgnx * log(1.0 - absx) / this->l;

}

real LaplacianDistribution_pdf(LaplacianDistribution *this, real x)
{
	return 0.5*this->l * exp (-this->l*fabs(x-this->m));
}

//public:
ExponentialDistribution* ExponentialDistribution_ctor1()
{
    ExponentialDistribution *this = calloc(1, sizeof(ExponentialDistribution));
    this->m=0.0;
    this->l=1.0;
    return this;
}
/// Create an exponential distribution with parameter \c lambda
ExponentialDistribution* ExponentialDistribution_ctor2(real lambda)
{
    ExponentialDistribution *this = calloc(1, sizeof(ExponentialDistribution));
    this->l = lambda;
    return this;
}
ExponentialDistribution* ExponentialDistribution_ctor3(real mean, real var)
{
    ExponentialDistribution *this = calloc(1, sizeof(ExponentialDistribution));
    ExponentialDistribution_setMean(this, mean);
    ExponentialDistribution_setVariance(this, var);
    return this;
}
/*virtual*/ void ExponentialDistribution_dtor(ExponentialDistribution *this) {}
/*virtual*/ //real ExponentialDistribution_generate(ExponentialDistribution *this);
/*virtual*/ //real ExponentialDistribution_pdf(ExponentialDistribution *this, real x);
/*virtual*/ void ExponentialDistribution_setVariance (ExponentialDistribution *this, real var)
{this->l = sqrt(1.0 / var);}
/*virtual*/ void ExponentialDistribution_setMean (ExponentialDistribution *this, real mean)
{this->m = mean;}

real ExponentialDistribution_generate(ExponentialDistribution *this)
{
	real x = urandom();
	return - log (1.0 - x) / this->l;
}

real ExponentialDistribution_pdf(ExponentialDistribution *this, real x)
{
	real d = x - this->m;
	if (d>0.0) {
		return this->l * exp (-this->l*d);
	}
	return 0.0;
}


DiscreteDistribution* DiscreteDistribution_ctor1()
{
    DiscreteDistribution *this = calloc(1, sizeof(DiscreteDistribution));
	this->p = NULL;
	this->n_outcomes=0;
	return this;
}

DiscreteDistribution* DiscreteDistribution_ctor2(int N)
{
    DiscreteDistribution *this = calloc(1, sizeof(DiscreteDistribution));
	this->p = NULL;
	this->n_outcomes = 0;
	this->p = (real*) malloc (sizeof(real) * N);
	this->n_outcomes = N;
	real invN = 1.0/((real) N);
	for (int i=0; i<N; i++) {
	    this->p[i] = invN;
	}
    return this;
}

void DiscreteDistribution_dtor(DiscreteDistribution *this)
{
	free (this->p);
}

real DiscreteDistribution_generate(DiscreteDistribution *this)
{
	real d=urandom();
	real sum = 0.0;
	for (int i=0; i<this->n_outcomes; i++) {
		sum += this->p[i];
		if (d < sum) {
			return (real) i;
		}
	}
	return 0.0;
}

real DiscreteDistribution_pdf(DiscreteDistribution *this, real x)
{
	int i=(int) floor(x);
	if ((i>=0)&&(i<this->n_outcomes)) {
		return this->p[i];
	} 
	return 0.0;
}

//public:
MultinomialGaussian* MultinomialGaussian_ctor1()
{
    MultinomialGaussian *this = calloc(1, sizeof(MultinomialGaussian));
    this->d=1;
    return this;
}
/// create a multinomial with \c d dimensions
MultinomialGaussian* MultinomialGaussian_ctor2(int d)
{
    MultinomialGaussian *this = calloc(1, sizeof(MultinomialGaussian));
    this->d = d;
    return this;
}
/*virtual*/ void MultinomialGaussian_dtor(MultinomialGaussian *this) {}
/*virtual*/ real MultinomialGaussian_generate() {return 0;}
/*virtual*/ real MultinomialGaussian_pdf(real x) {return 0;}
