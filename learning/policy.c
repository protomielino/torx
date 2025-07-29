// -*- Mode: c++ -*-
// $Id$

// copyright (c) 2004 by Christos Dimitrakakis <dimitrak@idiap.ch>
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <string.h>
#include "learning/learn_debug.h"
#include "learning/policy.h"
#include "learning/MathFunctions.h"
#ifdef WIN32
#include <float.h>
#define isnan _isnan
#endif // WIN32

#undef POLICY_LOG

#ifndef POLICY_LOG
#undef logmsg
#define logmsg empty_log
#endif

void empty_log(const char* s, ...)
{
}

//protected:
int DiscretePolicy_argMax(DiscretePolicy *this, real* Qs); ///< Get ID of maximum action
//public:
//DiscretePolicy* DiscretePolicy_ctor (int n_states, int n_actions, real alpha=0.1, real gamma=0.8, real lambda=0.8, bool softmax = false, real randomness=0.1, real init_eval=0.0);
/// Kill the agent and free everything.
/*virtual*/ //void DiscretePolicy_dtor(DiscretePolicy *this);
/// Set the learning rate.
/*virtual*/ void DiscretePolicy_setLearningRate (DiscretePolicy *this, real alpha)
{
    this->alpha = alpha;
}
/// Get the temporal difference error of the \em previous action.
/*virtual*/ real DiscretePolicy_getTDError (DiscretePolicy *this)
{
    return this->tdError;
}
/// Get the vale of the last action taken.
/*virtual*/ real DiscretePolicy_getLastActionValue (DiscretePolicy *this)
{
    return this->Q[this->ps][this->pa];
}
/*virtual*/ //int DiscretePolicy_SelectAction(DiscretePolicy *this, int s, real r, int forced_a=-1);
/*virtual*/ //void DiscretePolicy_Reset(DiscretePolicy *this);
/*virtual*/ //void DiscretePolicy_loadFile (DiscretePolicy *this, char* f);
/*virtual*/ //void DiscretePolicy_saveFile (DiscretePolicy *this, char* f);
/*virtual*/ //void DiscretePolicy_setQLearning(DiscretePolicy *this);
/*virtual*/ //void DiscretePolicy_setELearning(DiscretePolicy *this);
/*virtual*/ //void DiscretePolicy_setSarsa(DiscretePolicy *this);
/*virtual*/ //bool DiscretePolicy_useConfidenceEstimates(DiscretePolicy *this, bool confidence, real zeta=0.01, bool confidence_eligibility = false);
/*virtual*/ //void DiscretePolicy_setForcedLearning(DiscretePolicy *this, bool forced);
/*virtual*/ //void DiscretePolicy_setRandomness (DiscretePolicy *this, real epsilon);
/*virtual*/ //void DiscretePolicy_setGamma (DiscretePolicy *this, real gamma);
/*virtual*/ //void DiscretePolicy_setPursuit (DiscretePolicy *this, bool pursuit);
/*virtual*/ //void DiscretePolicy_setReplacingTraces (DiscretePolicy *this, bool replacing);
/*virtual*/ //void DiscretePolicy_useSoftmax (DiscretePolicy *this, bool softmax);
/*virtual*/ //void DiscretePolicy_setConfidenceDistribution (DiscretePolicy *this, enum ConfidenceDistribution cd);
/*virtual*/ //void DiscretePolicy_useGibbsConfidence (DiscretePolicy *this, bool gibbs);
/*virtual*/ //void DiscretePolicy_useReliabilityEstimate (DiscretePolicy *this, bool ri);
/*virtual*/ //void DiscretePolicy_saveState (DiscretePolicy *this, FILE* f);

/// \brief Create a new discrete policy.
/// \arg n_states Number of states for the agent
/// \arg n_actions Number of actions.
/// \arg alpha Learning rate.
/// \arg gamma Discount parameter.
/// \arg lambda Eligibility trace decay.
/// \arg softmax Use softmax if true (can be overridden later)
/// \arg randomness Amount of randomness.
/// \arg init_eval Initial evaluation of actions.
DiscretePolicy* DiscretePolicy_ctor (int n_states, int n_actions, real alpha,
				real gamma, real lambda, bool softmax,
				real randomness, real init_eval)
{
    DiscretePolicy *this = calloc(1, sizeof(DiscretePolicy));
	if (lambda<0.0f) lambda = 0.0f;
	if (lambda>0.99f) lambda = 0.99f;

	if (gamma<0.0f) gamma = 0.0f;
	if (gamma>0.99f) gamma = 0.99f;

	if (alpha<0.0f) alpha = 0.0f;
	if (alpha>1.0f) alpha = 1.0f;

	this->n_states = n_states;
	this->n_actions = n_actions;
	this->gamma = gamma;
	this->lambda = lambda;
	this->alpha = alpha;
	this->smax = softmax;
	this->temp = randomness;
	//logmsg ("RR:%f", temp);
	if (this->smax) {
		if (this->temp<0.1f)
		    this->temp = 0.1f;
	} else {
		if (this->temp<0.0f) {
		    this->temp = 0.0f;
		}
		if (this->temp>1.0f) {
		    this->temp = 1.0f;
		}
	}
	this->learning_method = Sarsa;

	logmsg ("#Making Sarsa(lambda) ");
	if (this->smax) {
		logmsg ("#softmax");
	} else {
		logmsg ("#e-greedy");
	}	
	logmsg (" policy with Q:[%d x %d] -> R, a:%f g:%f, l:%f, t:%f\n",
			this->n_states, this->n_actions, this->alpha, this->gamma, this->lambda, this->temp);

	this->P = calloc(n_states, sizeof(real*));
	this->Q = calloc(n_states, sizeof(real*));
	this->e = calloc(n_states, sizeof(real*));
	this->vQ = calloc(n_states, sizeof(real*));
	for (int s=0; s<n_states; s++) {
	    this->P[s] = calloc(n_actions, sizeof(real));
	    this->Q[s] = calloc(n_actions, sizeof(real));
	    this->e[s] = calloc(n_actions, sizeof(real));
	    this->vQ[s] = calloc(n_actions, sizeof(real));
		for (int a=0; a<n_actions; a++) {
		    this->P[s][a] = 1.0/((float)  n_actions);
		    this->Q[s][a] = init_eval;
		    this->e[s][a] = 0.0;
		    this->vQ[s][a] = 1.0;
		}
	}
	this->pQ = 0.0;
	this->ps = -1;
	this->pa = -1;
	this->min_el_state = 0;
	this->max_el_state = n_states -1;
	this->eval = calloc(n_actions, sizeof(real));
	this->sample = calloc(n_actions, sizeof(real));
	for (int a=0; a<n_actions; a++) {
	    this->eval[a] = 0.0;
	    this->sample[a] = 0.0;
	}
	this->forced_learning = false;
	this->confidence = false;
	this->confidence_uses_gibbs = true;
	this->confidence_distribution = SINGULAR;
	this->zeta = 0.01f;
	this->tdError = 0.0f;
	this->expected_r = 0.0f;
	this->expected_V = 0.0f;
	this->n_samples = 0;
	this->replacing_traces = false;
	return this;
}

/// \brief Save the current evaluations in text format to a file.
/// The values are saved as triplets (\c Q, \c P, \c
/// vQ). The columns are ordered by actions and the rows by state
/// number.
void DiscretePolicy_saveState(DiscretePolicy *this, FILE* f)
{
	if (!f)
		return;
	for (int s=0; s<this->n_states; s++) {
		
		//softMax(Q[s]);
		real sum2=0.0;
		int a;
		for (a=0; a<this->n_actions; a++) {
			sum2 += this->eval[a];
		}
		for (a=0; a<this->n_actions; a++) {
			fprintf (f, "%f ", this->Q[s][a]);
		}
		for (a=0; a<this->n_actions; a++) {
			fprintf (f, "%f ", this->P[s][a]);
		}
		for (a=0; a<this->n_actions; a++) {
			fprintf (f, "%f ", this->vQ[s][a]);
		}
	}

	fprintf (f, "\n");
}

/// Delete policy.
void DiscretePolicy_dtor(DiscretePolicy *this)
{
	real sum = 0.0;
	FILE* f = fopen ("/tmp/discrete","wb");

	int s;
	for (s=0; s<this->n_states; s++) {
		sum += this->Q[s][DiscretePolicy_argMax(this, this->Q[s])];
		if (f) {
			//softMax(Q[s]);
			real sum2=0.0;
			int a;
			for (a=0; a<this->n_actions; a++) {
				sum2 += this->eval[a];
			}
			for (a=0; a<this->n_actions; a++) {
				fprintf (f, "%f ", this->Q[s][a]);
			}
			for (a=0; a<this->n_actions; a++) {
				fprintf (f, "%f ", this->P[s][a]);
			}
			for (a=0; a<this->n_actions; a++) {
				fprintf (f, "%f ", this->vQ[s][a]);
			}
			fprintf (f, "\n");
		}
	}

	if (f) {
		fclose (f);
	}

	logmsg ("#Expected return of greedy policy over random distribution of states: %f\n", sum/((real) this->n_states));

	for (s=0; s<this->n_states; s++) {
		free(this->P[s]);
		free(this->Q[s]);
		free(this->e[s]);
		free(this->vQ[s]);
	}
	free(this->P);
	free(this->Q);
	free(this->vQ);
	free(this->e);
	free(this->eval);
	free(this->sample);
}

/** Select an action a, given state s and reward from previous action.

   Optional argument a forces an action if setForcedLearning() has
   been called with true.

   Two algorithms are implemented, both of which converge. One of them
   calculates the value of the current policy, while the other that of
   the optimal policy. 

   Sarsa (\f$\lambda\f$) algorithmic description:

   1. Take action \f$a\f$, observe \f$r, s'\f$

   2. Choose \f$a'\f$ from \f$s'\f$ using some policy derived from \f$Q\f$

   3. \f$\delta = r + \gamma Q(s',a') - Q(s,a)\f$

   4. \f$e(s,a) = e(s,a)+ 1\f$, depending on trace settings

   5. for all \f$s,a\f$ :
   \f[
   Q_{t}(s,a) = Q_{t-1}(s,a) + \alpha \delta e_{t}(s,a),
   \f]
where \f$e_{t}(s,a) = \gamma \lambda e_{t-1}(s,a)\f$
   
	  end

   6. \f$a = a'\f$ (we will take this action at the next step)

   7. \f$s = s'\f$

   Watkins Q (l) algorithmic description:

   1. Take action \f$a\f$, observe \f$r\f$, \f$s'\f$

   2. Choose \f$a'\f$ from \f$s'\f$ using some policy derived from \f$Q\f$

   3. \f$a* = \arg \max_b Q(s',b)\f$

   3. \f$\delta = r + \gamma Q(s',a^*) - Q(s,a)\f$

   4. \f$e(s,a) = e(s,a)+ 1\f$, depending on eligibility traces

   5. for all \f$s,a\f$ :
\f[
        Q(s,a) = Q(s,a)+\alpha \delta e(s,a)
\f]
		if \f$(a'=a*)\f$ then \f$e(s,a)\f$ = \f$\gamma \lambda e(s,a)\f$
		           else \f$e(s,a) = 0\f$
	  end

   6. \f$a = a'\f$ (we will take this action at the next step)

   7. \f$s = s'\f$

   The most general algorithm is E-learning, currently under
   development, which is defined as follows:

   1. Take action \f$a\f$, observe \f$r\f$, \f$s'\f$

   2. Choose \f$a'\f$ from \f$s'\f$ using some policy derived from \f$Q\f$

   3. \f$\delta = r + \gamma E{Q(s',a^*)|\pi} - Q(s,a)\f$

   4. \f$e(s,a) = e(s,a)+ 1\f$, depending on eligibility traces

   5. for all \f$s,a\f$ :
\f[
        Q(s,a) = Q(s,a)+\alpha \delta e(s,a)
\f]
		\f$e(s,a)\f$ = \f$\gamma \lambda e(s,a) P(a|s,\pi) \f$

   6. \f$a = a'\f$ (we will take this action at the next step)

   7. \f$s = s'\f$

   Note that we also cut off the eligibility traces that have fallen below 0.1


*/
int DiscretePolicy_SelectAction (DiscretePolicy *this, int s, real r, int forced_a)
{
	if ((s<0)||(s>=this->n_states)) {
		return 0;
	}

	if ((this->ps>=0)&&(this->pa>=0)) {
	    this->expected_r += r;
	    this->expected_V += this->Q[this->ps][this->pa];
	    this->n_samples++;
		
		if (s==0) {
			real max_estimate = 0.0;
			real max_estimate_k = 0.0;
			for (int i=0; i<this->n_states; i++) {
				max_estimate += this->Q[i][DiscretePolicy_argMax (this, this->Q[i])];
				max_estimate_k += 1.0;
			}

#if 0
			logmsg ("%f %f %f %f#rTVV\n",
					expected_r/((real) n_samples), 
					temp,
					expected_V/((real) n_samples),
					max_estimate/max_estimate_k);
#endif
			this->expected_r = 0.0;
			this->expected_V= 0.0;
			this->n_samples = 0;
		}
	}
	int a, amax;
	int argmax = DiscretePolicy_argMax (this, this->Q[s]);

	this->P[s][argmax] += this->zeta*(1.0-this->P[s][argmax]);
	for (int j=0; j<this->n_actions; j++) {
		if (j!=argmax) {
		    this->P[s][j] += this->zeta*(0.0-this->P[s][j]);
		}
	}



	if (this->forced_learning) {
		a = forced_a;
	} else if (this->pursuit) {
		real sum = 0.0;
		a = -1;
		int j;
		for (j=0; j<this->n_actions; j++) {
			sum += this->P[s][j];
		}
		real X = urandom()*sum;
		real dsum=0.0;
		for (j=0; j<this->n_actions; j++) {
			dsum += this->P[s][j];
			if (X<=dsum) {
				a = j;
				break;
			}
		}
		if (a==-1) {
			fprintf (stderr, "No action selected with pursuit!\n");
		}
	} else if (this->confidence) {
		if (this->confidence_uses_gibbs && (this->confidence_distribution == SINGULAR)) {
			a = DiscretePolicy_confMax (this, this->Q[s],this->vQ[s], 1.0);
		} else {
			a = DiscretePolicy_confSample (this, this->Q[s], this->vQ[s]);
			if (this->confidence_uses_gibbs) { // and not SINGULAR distribution
				a = DiscretePolicy_softMax(this, this->sample); //use softmax on the sample values
			}
		}
	} else if (this->reliability_estimate) {
	    this->temp = sqrt(Sum(this->vQ[s], this->n_actions)/((real) this->n_actions));
		//temp = 0.1;
		a = DiscretePolicy_softMax(this, this->Q[s]);
		//printf ("%f\n", temp);
	} else if (this->smax) {
		a = DiscretePolicy_softMax (this, this->Q[s]);
		//printf ("Q[%d][%d]=%f\n", s, a, Q[s][a]);
	} else {
		a = DiscretePolicy_eGreedy (this, this->Q[s]);
	}

	if (a<0 || a>=this->n_actions) {
		fprintf (stderr, "Action %d out of bounds.. ", a);
		a = (int) floor (urandom()*((real) this->n_actions));
		fprintf (stderr, "mapping to %d\n", a);
	}

	real EQ_s = 0.0;
	int i;

	switch (this->learning_method) {
		
	case Sarsa: 
		amax = a;
		EQ_s = this->Q[s][amax];
		break;
	case QLearning:
		amax = argmax;
		EQ_s = this->Q[s][amax];
		break;
	case ELearning:
		amax = a; //? correct ? 
		Normalise(this->eval, this->eval, this->n_actions);
		EQ_s = 0.0;
		for (i=0; i<this->n_actions; i++) {
			EQ_s += this->eval[i] * this->Q[s][i];
		}
		break;
	default:
		amax = a;
		EQ_s = this->Q[s][amax];
		fprintf (stderr, "Unknown learning method\n");
	}
	if ((this->ps>=0)&&(this->pa>=0)) { // do not update at start of episode
		real delta = r + this->gamma*EQ_s - this->Q[this->ps][this->pa];
		this->tdError = delta;
		if (this->replacing_traces) {
		    this->e[this->ps][this->pa] = 1.0;
		} else {
		    this->e[this->ps][this->pa] += 1.0;
		}
		real ad = this->alpha*delta;
		real gl = this->gamma * this->lambda;
		real variance_threshold = 0.0001f;		
		if  (this->confidence_eligibility == false) {
		    this->vQ[this->ps][this->pa] = (1.0 - this->zeta)*this->vQ[this->ps][this->pa] + this->zeta*(ad*ad);
			if (this->vQ[this->ps][this->pa]<variance_threshold) {
			    this->vQ[this->ps][this->pa]=variance_threshold;
			}
		}
		if (this->ps<this->min_el_state) this->min_el_state = this->ps;
		if (this->ps>this->max_el_state) this->max_el_state = this->ps;
		

		for (i=0; i<this->n_states; i++) {
			//for (int i=min_el_state; i<=max_el_state; i++) {
			bool el=true;
			for (int j=0; j<this->n_actions; j++) {
				if (this->e[i][j]>0.01) {
				    this->Q[i][j] += ad * this->e[i][j];
					if (this->confidence_eligibility == true) {
						real zeta_el = this->zeta * this->e[i][j];
						this->vQ[i][j] = (1.0 - zeta_el)*this->vQ[i][j] + zeta_el*(ad*ad);
						if (this->vQ[i][j]<variance_threshold) {
						    this->vQ[i][j]=variance_threshold;
						}
					}
					//this is the same as setting e[ps][pa] += (1-P[ps][pa])
					// if P[][] remains unchanged between updates.
					// -- removed because it doesn't work! --
					//P[i][j] += 0.01*delta * e[i][j] * (1.-P[i][j]);
					if ((fabs (this->Q[i][j])>1000.0)||(isnan(this->Q[i][j]))) {
						printf ("u: %d %d %f %f\n", i,j,this->Q[i][j], ad * this->e[i][j]);
					}
					
					//This is only needed for Qlearning, but sarsa is not
					//affected since always amax==a;
					if (amax==a) {
					    this->e[i][j] *= gl;
					} else {
					    this->e[i][j] = 0.0;
					}
				} else {
				    this->e[i][j] = 0.0;
					el = false;
				}
			}
			if (el==false) {
				if (this->min_el_state==i)
				    this->min_el_state++;
			} else {
			    this->max_el_state = i;
			}
		}
	}

	//printf ("%d %d #STATE\n", min_el_state, max_el_state);
	//	printf ("Q[%d,%d]=%f r=%f e=%f ad=%f gl=%f #QV\n",
	//			ps, pa, Q[ps][pa], r, e[ps][pa], ad, gl);
	this->ps = s;
	this->pa = a;

	return a;
}

/// Use at the end of every episode, after agent has entered the
/// absorbing state.
void DiscretePolicy_Reset (DiscretePolicy *this)
{
	for (int s=0; s<this->n_states; s++) {
		for (int a=0; a<this->n_actions; a++) {
		    this->e[s][a] = 0.0;
		}
	}
}

/// Load policy from a file.
void DiscretePolicy_loadFile (DiscretePolicy *this, char* f)
{
	FILE* fh = NULL;
	fh = fopen (f, "rb");
	if (fh==NULL) {
		fprintf (stderr, "Failed to read file %s\n", f);
		return;
	}
	char rtag[256];
	const char* start_tag="QSA";
	const char* close_tag="END";
	int n_read_states, n_read_actions;

	fread((void *) rtag, sizeof (char), strlen (start_tag)+1, fh);
	if (strcmp (rtag, start_tag)) {
		fprintf (stderr, "Could not find starting tag\n");
		return;
	}
	fread((void *) &n_read_states, sizeof(int), 1, fh);
	fread((void *) &n_read_actions, sizeof(int), 1, fh);
	
	if ((n_read_states!=this->n_states)||(n_read_actions!=this->n_actions)) {
		fprintf (stderr, "File has %dx%d space! Aborting read.\n", n_read_states, n_read_actions);
		fclose(fh);
		return;
	}

	int i, j;
	for (i=0; i<this->n_states; i++) {
		fread((void *) this->Q[i], sizeof(real), this->n_actions, fh);
		for (j=0; j<this->n_actions; j++) {
			if ((fabs (this->Q[i][j])>100.0)||(isnan(this->Q[i][j]))) {
				printf ("l: %d %d %f\n", i,j,this->Q[i][j]);
				this->Q[i][j] = 0.0;
			}
		}
	}
	for (i=0; i<this->n_states; i++) {
		for (j=0; j<this->n_actions; j++) {
			{
			    this->P[i][j] = 1.0/((real) this->n_actions);
			}
		}
		int argmax = DiscretePolicy_argMax (this, this->Q[i]);
		this->P[i][argmax] += 0.001*(1.0-this->P[i][argmax]);
		for (int j=0; j<this->n_actions; j++) {
			if (j!=argmax) {
			    this->P[i][j] += 0.001*(0.0-this->P[i][j]);
			}
		}
	}



	fread((void *) rtag, sizeof (char), strlen (close_tag)+1, fh);
	if (strcmp (rtag, close_tag)) {
		fprintf (stderr, "Could not find ending tag\n");
		fclose (fh);
		return;
	}


	fclose (fh);
}

/// Save policy to a file.
void DiscretePolicy_saveFile (DiscretePolicy *this, char* f)
{
	FILE* fh = NULL;
	fh = fopen (f, "wb");
	if (fh==NULL) {
		fprintf (stderr, "Failed to write to file %s\n", f);
		return;
	}

	const char* start_tag="QSA";
	const char* close_tag="END";

	fwrite((void *) start_tag, sizeof (char), strlen (start_tag)+1, fh);
	fwrite((void *) &this->n_states, sizeof(int), 1, fh);
	fwrite((void *) &this->n_actions, sizeof(int), 1, fh);
	for (int i=0; i<this->n_states; i++) {
		fwrite((void *) this->Q[i], sizeof(real), this->n_actions, fh);
		for (int j=0; j<this->n_actions; j++) {
			if ((fabs (this->Q[i][j])>100.0)||(isnan(this->Q[i][j]))) {
				printf ("s: %d %d %f\n", i,j,this->Q[i][j]);
			}
		}
	}
	fwrite((void *) close_tag, sizeof (char), strlen (start_tag)+1, fh);
	fclose (fh);
}

/// \brief Set to use confidence estimates for action selection, with
/// variance smoothing zeta.
/// Variance smoothing currently uses a very simple method to estimate
/// the variance. 
bool DiscretePolicy_useConfidenceEstimates (DiscretePolicy *this, bool confidence, real zeta, bool confidence_eligibility)
{
	this->confidence = confidence;
	this->zeta = zeta;
	this->confidence_eligibility = confidence_eligibility;
	
	if (confidence_eligibility) {
		logmsg ("#+[ELIG_VAR]");
	}
	if (confidence) {
		logmsg ("#+[CONDIFENCE]");
	} else {
		logmsg ("#-[CONDIFENCE]\n");
	}
	
	return confidence;
}

/// Set the algorithm to QLearning mode
void DiscretePolicy_setQLearning(DiscretePolicy *this)
{
    this->learning_method = QLearning;
	logmsg ("#[Q-learning]\n");
}

/// Set the algorithm to ELearning mode
void DiscretePolicy_setELearning(DiscretePolicy *this)
{
    this->learning_method = ELearning;
	logmsg ("#[E-learning]\n");
}

/// \brief Set the algorithm to SARSA mode.
/// A unified framework for action selection.
void DiscretePolicy_setSarsa(DiscretePolicy *this)
{
    this->learning_method = Sarsa;
	logmsg ("#[Sarsa]\n");
}

/// Use Pursuit for action selection.
void DiscretePolicy_setPursuit(DiscretePolicy *this, bool pursuit)
{
	this->pursuit = pursuit;
	if (pursuit) {
		logmsg ("#+[PURSUIT]\n");
	} else {
		logmsg ("#-[PURSUIT]\n");
	}
}

/// Use Pursuit for action selection.
void DiscretePolicy_setReplacingTraces (DiscretePolicy *this, bool replacing)
{
	this->replacing_traces = replacing;
	if (replacing) {
		logmsg ("#[REPLACING TRACES]\n");
	} else {
		logmsg ("#[ACCUMULATING TRACES]\n");
	}
}
/// Set forced learning (force-feed actions)
void DiscretePolicy_setForcedLearning(DiscretePolicy *this, bool forced)
{
    this->forced_learning = forced;
}

/// Set randomness for action selection. Does not affect confidence mode.
void DiscretePolicy_setRandomness (DiscretePolicy *this, real epsilon)
{
    this->temp = epsilon;
	if (this->smax) {
		if (this->temp<0.01) {
		    this->smax = false;
		}
	}
}

/// Set the gamma of the sum to be maximised.
void DiscretePolicy_setGamma (DiscretePolicy *this, real gamma)
{
	this->gamma = gamma;
}

/// Set action selection to softmax.
void DiscretePolicy_useSoftmax (DiscretePolicy *this, bool softmax)
{
    this->smax = softmax;
	if (this->smax) {
		logmsg ("#+[SMAX]\n");
	} else {
		logmsg ("#-[SMAX]\n");
	}
}

/// Use the reliability estimate method for action selection.
void DiscretePolicy_useReliabilityEstimate (DiscretePolicy *this, bool ri)
{
    this->reliability_estimate = ri;
	if (ri) {
		logmsg("#+[RI]\n");
	} else {
		logmsg("#-[RI]\n");
	}
}

/// Set the distribution for direct action sampling.
void DiscretePolicy_setConfidenceDistribution (DiscretePolicy *this, enum ConfidenceDistribution cd)
{
	switch (cd) {
	case SINGULAR:
		logmsg("#[SINGULAR CONFIDENCE]\n"); break;
	case BOUNDED:
		logmsg("#[BOUNDED CONFIDENCE]\n"); break;
	case GAUSSIAN:
		logmsg("#[GAUSSIAN CONFIDENCE]\n"); break;
	case LAPLACIAN:
		logmsg("#[LAPLACIAN CONFIDENCE]\n"); break;
	default:
		Serror ("Unknown type %d\n", cd);
	}
	this->confidence_distribution = cd;
}

/// \brief Add Gibbs sampling for confidences.
/// This can be used in conjuction with any confidence distribution,
/// however it mostly makes sense for SINGULAR.
void DiscretePolicy_useGibbsConfidence (DiscretePolicy *this, bool gibbs)
{
	if (gibbs) {
		logmsg ("#+[GIBBS CONFIDENCE]\n");
	} else {
		logmsg ("#-[GIBBS CONFIDENCE]\n");
	}
	this->confidence_uses_gibbs = gibbs;
}

// ---------- action selection helpers -------------
int DiscretePolicy_confMax(DiscretePolicy *this, real* Qs, real* vQs, real p)
{
	real sum=0.0;
	int a;
#if 0
	for (a=0; a<n_actions; a++) {
		eval[a] = exp(pow(Qs[a]/sqrt(vQs[a]), p));
		sum += eval[a];
	}
#else
	for (a=0; a<this->n_actions; a++) {
		real Q = Qs[a];
		real cum = 1.0;
		//real v = sqrt(vQs[a]);
		for (int j=0; j<this->n_actions; j++) {
			if (j!=a) {
				cum += exp ((Qs[j]-Q)/sqrt(vQs[j]));
			}
		}
		this->eval[a] = 1.0/(cum);//#exp(Qs[a]/sqrt(vQs[a]));
		sum += this->eval[a];
	}
#endif
	real X = urandom()*sum;
	real dsum = 0.0;
	for (a=0; a<this->n_actions; a++) {
		dsum += this->eval[a];
		if (X<=dsum) 
			return a;

	}
	fprintf (stderr, "ConfMax: No action selected! %f %f %f\n",X,dsum,sum);
	return -1;
}

int DiscretePolicy_confSample(DiscretePolicy *this, real* Qs, real* vQs)
{
	static NormalDistribution gaussian;
	static LaplacianDistribution laplacian;
	static UniformDistribution uniform;

	for (int a=0; a<this->n_actions; a++) {
		//eval[a] = Qs[a] + urandom(-1.0,1.0)*vQs[a];
		switch(this->confidence_distribution) {
		case SINGULAR:
		    this->sample[a] = Qs[a];
			break;
		case BOUNDED:
			UniformDistribution_setMean(&uniform, Qs[a]);
			UniformDistribution_setVariance(&uniform, vQs[a]);
			this->sample[a] = UniformDistribution_generate(&uniform);
			break;
		case GAUSSIAN:
			NormalDistribution_setMean(&gaussian, Qs[a]);
			NormalDistribution_setVariance(&gaussian, vQs[a]);
			this->sample[a] = NormalDistribution_generate(&gaussian);
			break;
		case LAPLACIAN:
			LaplacianDistribution_setMean(&laplacian, Qs[a]);
			LaplacianDistribution_setVariance(&laplacian, vQs[a]);
			this->sample[a] = Qs[a] + LaplacianDistribution_generate(&laplacian);
			break;
		default:
			Serror ("Unknown distribution ID:%d\n", this->confidence_distribution);
			break;
		}
	}
	return DiscretePolicy_argMax(this, this->sample);
}

int DiscretePolicy_softMax(DiscretePolicy *this, real* Qs)
{
	real sum=0.0f;
	real beta = 1.0f/this->temp;
	int a;
	for (a=0; a<this->n_actions; a++) {
	    this->eval[a] = exp(beta * Qs[a]);
		sum += this->eval[a];
	}
	real X = urandom()*sum;
	real dsum = 0.0;
	for (a=0; a<this->n_actions; a++) {
		dsum += this->eval[a];
		if (X<=dsum) 
			return a;

	}
	fprintf (stderr, "softMax: No action selected! %f %f %f\nT:%f\n",X,dsum,sum,this->temp);
	return -1;
}
int DiscretePolicy_eGreedy(DiscretePolicy *this, real* Qs)
{
	real X = urandom();
	int amax = DiscretePolicy_argMax(this, Qs);
	real base_prob = this->temp/((real) this->n_actions);
	for (int a=0; a<this->n_actions; a++) {
	    this->eval[a] = base_prob;
	}
	this->eval[amax] += 1.0-this->temp;
	if (X<this->temp) {
		return rand()%this->n_actions;
	}
	return DiscretePolicy_argMax(this, Qs);
}

int DiscretePolicy_argMax(DiscretePolicy *this, real* Qs)
{
	real max = Qs[0];
	int arg_max = 0;
	for (int a=1; a<this->n_actions; a++) {
		if (max<Qs[a]) {
			max = Qs[a];
			arg_max = a;
		}
	}
	return arg_max;
}


