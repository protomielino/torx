// -*- Mode: c++ -*-
// copyright (c) 2004 by Christos Dimitrakakis <dimitrak@idiap.ch>
// $Id$


/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "learning/ann_policy.h"
#include "learning/ANN.h"

//public:
/// Make a new policy
//ANN_Policy* ANN_Policy_ctor (int n_states, int n_actions, int n_hidden/* = 0*/, real alpha/*=0.1*/, real gamma/*=0.8*/, real lambda/*=0.8*/, bool eligibility /*= false*/, bool softmax /*= false*/, real randomness/*=0.1*/, real init_eval/*=0.0*/, bool separate_actions /*= false*/);
/*virtual*/ //void ANN_Policy_dtor(ANN_Policy *this) {}
/// Select an action, given a vector of real numbers which
/// represents the state.
/*virtual*/ //int ANN_Policy_SelectAction(ANN_Policy *this, real* s, real r, int forced_a/*=-1*/);
/// Reset eligibility traces.
/*virtual*/ void ANN_Policy_Reset(ANN_Policy *this);
/// Return the last action value.
/*virtual*/ real ANN_Policy_getLastActionValue (ANN_Policy *this) {return this->J_ps_pa;}
/// \deprecated Get the probabilities of all actions - call after SelectAction().
/*virtual*/ real* ANN_Policy_getActionProbabilities (ANN_Policy *this)
{
    real sum = 0.0;
    int i;
    for (i=0; i<this->_dp->n_actions; i++) {
        sum += this->_dp->eval[i];
    }
    for (i=0; i<this->_dp->n_actions; i++) {
        this->_dp->eval[i] = this->_dp->eval[i]/sum;
    }
    return this->_dp->eval;
}
/*virtual*/ //bool ANN_Policy_useConfidenceEstimates(ANN_Policy *this, bool confidence, real zeta/*=0.01*/);

ANN_Policy* ANN_Policy_ctor (int n_states, int n_actions, int n_hidden/* = 0*/, real alpha/*=0.1*/, real gamma/*=0.8*/, real lambda/*=0.8*/, bool eligibility /*= false*/, bool softmax /*= false*/, real randomness/*=0.1*/, real init_eval/*=0.0*/, bool separate_actions /*= false*/)
/*        : DiscretePolicy (n_states, n_actions, alpha, gamma, lambda, softmax, randomness, init_eval)*/
{
    ANN_Policy *this = calloc(1, sizeof(ANN_Policy));
    this->_dp = DiscretePolicy_ctor (n_states, n_actions, alpha, gamma, lambda, softmax, randomness, init_eval);
	this->separate_actions = separate_actions;
	this->eligibility = eligibility;
	if (eligibility) {
		message ("Using eligibility traces");
	}	
	if (separate_actions) {
		message ("Separate actions");
		this->J = NULL;
		this->Ja = calloc(n_actions, sizeof(ANN*));
		this->JQs = calloc(n_actions, sizeof(real));
		for (int i=0; i<n_actions; i++) {
		    this->Ja[i] = NewANN (n_states, 1);
			if (n_hidden > 0) {
				ANN_AddHiddenLayer (this->Ja[i], n_hidden);
			}
			ANN_Init (this->Ja[i]);
			ANN_SetOutputsToLinear(this->Ja[i]);
			ANN_SetBatchMode(this->Ja[i], false);
			this->Ja[i]->eligibility_traces = eligibility;
			ANN_SetLambda(this->Ja[i],lambda*gamma);
			ANN_SetLearningRate (this->Ja[i], alpha);
		}
	} else {
	    this->JQs = NULL;
	    this->Ja = NULL;
	    this->J = NewANN (n_states, n_actions);
		if (n_hidden > 0) {
			ANN_AddHiddenLayer (this->J, n_hidden);
		}
		ANN_Init (this->J);
		ANN_SetOutputsToLinear(this->J);
		ANN_SetBatchMode(this->J, false);
		this->J->eligibility_traces = eligibility;
		ANN_SetLambda(this->J,lambda*gamma);
		ANN_SetLearningRate (this->J, alpha);
	}
	this->ps = calloc(n_states, sizeof(real));
	this->delta_vector = calloc(n_actions, sizeof(real));
	this->J_ps_pa = 0.0;
	return this;
}

void ANN_Policy_dtor(ANN_Policy *this)
{
    free(this->ps);
    free(this->delta_vector);
	if (this->separate_actions) {
		for (int i=0; i<this->_dp->n_actions; i++) {
			DeleteANN(this->Ja[i]);
		}
		free(this->Ja);
	} else {
		//ANN_ShowWeights(J);
		DeleteANN (this->J);
	}
	DiscretePolicy_dtor(this->_dp);
}

int ANN_Policy_SelectAction (ANN_Policy *this, real* s, real r, int forced_a)
{
	int a; // selected action
	int amax; //maximum evaluated action
	real* Q_s; // pointer to evaluations for state s
	if (this->_dp->confidence) {
		if (this->separate_actions) {
			for (int i=0; i<this->_dp->n_actions; i++) {
				ANN_StochasticInput (this->Ja[i], s);
				this->JQs[i] = ANN_GetOutput(this->Ja[i])[0];
			}
			Q_s = this->JQs;
		} else {
			ANN_StochasticInput (this->J, s);
			Q_s = ANN_GetOutput (this->J);
		}
	} else {
		if (this->separate_actions) {
			for (int i=0; i<this->_dp->n_actions; i++) {
				ANN_Input (this->Ja[i], s);
				this->JQs[i] = ANN_GetOutput(this->Ja[i])[0];
			}
			Q_s = this->JQs;
		} else {
			ANN_Input (this->J, s);
			Q_s = ANN_GetOutput (this->J);
		}
	}
	int argmax = DiscretePolicy_argMax (this->_dp, Q_s);

	if (this->_dp->forced_learning) {
		a = forced_a;
	} else if (this->_dp->confidence) {
		a = argmax;
	} else if (this->_dp->smax) {
		a = DiscretePolicy_softMax (this->_dp, Q_s);
		//printf ("Q[%d][%d]=%f\n", s, a, Q[s][a]);
	} else {
		a = DiscretePolicy_eGreedy (this->_dp, Q_s);
	}

	if (a<0 || a>=this->_dp->n_actions) {
		fprintf (stderr, "Action %d out of bounds\n", a);
	}

	switch (this->_dp->learning_method) {
		
	case Sarsa: 
		amax = a;
		break;
	case QLearning:
		amax = argmax;
		break;
	default:
		amax = a;
		fprintf (stderr, "Unknown learning method\n");
	}
	if (this->_dp->pa>=0) { // do not update at start of episode
		real delta = r + this->_dp->gamma*Q_s[amax] - this->J_ps_pa;
		this->_dp->tdError = delta;
		for (int j=0; j<this->_dp->n_actions; j++) {
		    this->delta_vector[j] = 0.0;
		}
		if (this->separate_actions) {
			if (this->eligibility) {
			    this->delta_vector[0] = 1.0;
				ANN_Delta_Train (this->Ja[this->_dp->pa], this->delta_vector, delta);
				// Reset other actions' traces.
				for (int i=0; i<this->_dp->n_actions; i++) {
					if (i!=this->_dp->pa) {
						ANN_Reset(this->Ja[i]);
					}
				}
			} else {
			    this->delta_vector[0] = delta;
				ANN_Delta_Train (this->Ja[this->_dp->pa], this->delta_vector, 0.0);
			}
		} else {
			if (this->J->eligibility_traces) {
			    this->delta_vector[this->_dp->pa] = 1.0;
				ANN_Delta_Train (this->J, this->delta_vector, delta);
			} else {
			    this->delta_vector[this->_dp->pa] = delta;
				ANN_Delta_Train (this->J, this->delta_vector, 0.0);
			}
		}


	}

	//printf ("%d %d #STATE\n", min_el_state, max_el_state);
	//	printf ("Q[%d,%d]=%f r=%f e=%f ad=%f gl=%f #QV\n",
	//			ps, pa, Q[ps][pa], r, e[ps][pa], ad, gl);

	this->J_ps_pa = Q_s[a];
	this->_dp->pa = a;

	return a;
}

void ANN_Policy_Reset(ANN_Policy *this)
{
	if (this->separate_actions) {
		for (int i=0; i<this->_dp->n_actions; i++) {
			ANN_Reset (this->Ja[i]);
		}
	} else {
		ANN_Reset(this->J);
	}
}

/// Set to use confidence estimates for action selection, with
/// variance smoothing zeta.
bool ANN_Policy_useConfidenceEstimates (ANN_Policy *this, bool confidence, real zeta) {
	this->_dp->confidence = confidence;
	this->_dp->zeta = zeta;
	if (this->separate_actions) {
		for (int i=0; i<this->_dp->n_actions; i++) {
			ANN_SetZeta(this->Ja[i], zeta);
		}
	} else {
		ANN_SetZeta(this->J, zeta);
	}
	if (confidence) {
		logmsg ("#+[CONDIFENCE]\n");
	} else {
		logmsg ("#-[CONDIFENCE]\n");
	}

	return confidence;
}
