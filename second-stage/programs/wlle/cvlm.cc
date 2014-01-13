// Licensed under the Apache License, Version 2.0 (the "License"); you may
// not use this file except in compliance with the License.  You may obtain
// a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
// License for the specific language governing permissions and limitations
// under the License.

// cvlm.cc -- A linear model estimator that uses a variety of loss functions.
//            It sets the regularizer parameters using cross-validation.
//
// Mark Johnson, 11th April 2005, last modified 21st Nov 2007
//
// This is a version of wlle.cc reworked to use the tao-optimizer.h
// routines (so most of the Petsc/Tao junk is now moved out of this
// file).  It also sets regularizer factors by cross-validation.
// It writes out the weights file after each cross-validation estimation.
//
// This optimizier uses the TAO constrained optimization method tao_blmvm
// to avoid the variable discontinuity at zero if the -cv flag is set.
//
// For each variable x[i] in the original problem we introduce a pair of
// variables xp[i] and xn[i] and a pair of constraints xp[i] >= 0, xn[i] <= 0.
// Then x[i] = xp[i] + xn[i].
//
// The regularized function Q that is optimized is:
//
//  Q(xp,xn) = s * ( L(xp+xn) + R(xp,xn) )
//
// where s is a user-specified parameter, L is the unregularized loss 
// function and R is the regularizer:
//
//  R(xp,xn) = c * sum_i (pow(fabs(xp[i]),p) + pow(fabs(xn[i]),p))
//
// where c and p are user-specifier adjustable parameters.  (The fabs()
// is there just in case the optimization routine temporarily proposes an
// infeasible solution).
//
// Note that
//
//  d Q/d xp[i] = s * d L/ d xp[i] + s * c * p * sum_i pow(fabs(xp[i]),p-1) * sign(xp[i])
//
// where sign(xp[i]) is +1 if xp[i] is non-negative and -1 otherwise.
//
// Change log:
//
// 20th April, 2004: the regularizer weights disjunctive features proportial
// to the number of disjuncts they contain
//
const char usage[] =
"cvlm version of 21st November 2007\n"
"\n"
"	A constrained-variable weighted linear model estimator\n"
"	with regularizer factors set by cross-validation.\n"
"\n"
"cvlm estimates feature weights that estimate the parameters of a\n"
"linear model by minimizing a regularized loss of the feature\n"
"weights using the LVLM or BLVLM optimizer from the Petsc/Tao\n"
"optimization package (see http://www-fp.mcs.anl.gov/tao/ for details).\n"
"It can deal with partially labeled data (i.e., training instances\n"
"consist of one or more \"winners\" and one or more \"losers\").\n"
"\n"
"Usage: cvlm [-help] [-debug debug_level] [-c0 c0] [-c00 c00] [-p p] [-r r] [-s s] [-cv] \n"
"              [-l ltype] [-Pyx_factor f] [-Px_propto_g] [-max-nrounds maxnrounds] \n"
"              [-o weights-file]  [-e eval-file] [-x eval-file2]\n"
"              [-ns ns] [-f feat-file]\n"
"              tao-options*\n"
"	       < train-file\n"
"\n"
"where:\n"
"\n"
" debug_level > 0 controls the amount of output produced\n"
"\n"
" c0 is the initial value for the regularizer constant, and the weight of the regularizer\n"
" constant for the first feature class is multiplied by c00\n"
"\n"
" train-file, eval-file and eval-file2 are files from which training and evaluation\n"
" data are read (if eval-file ends in the suffix .bz2 then bzcat is used\n"
" to read it; if no eval-file is specified, then the program tests on the\n"
" training data),\n"
"\n"
" weights-file is a file to which the estimated weights are written,\n"
"\n"
" feat-file is a file of <featclass> <featuredetails> lines, used for\n"
" cross-validating regularizer weights,\n"
"\n"
" ns is the number of ':' characters to use to define the cross-validation\n"
" classes,\n" 
"\n"
" ltype identifies the type of loss function used:\n"
"\n"
"    -l 0 - log loss (c0 ~ 5)\n"
"    -l 1 - EM-style log loss (c0 ~ 5)\n"
"    -l 2 - pairwise log loss \n"
"    -l 3 - exp loss (c0 ~ 25, s ~ 1e-5)\n"
"    -l 4 - log exp loss (c0 ~ 1e-4)\n"
"    -l 5 - maximize expected F-score (c ~ ?)\n"
"\n"
" ns is the maximum number of ':' characters in a <featclass>, used to\n"
" determine how features are binned into feature classes (ns = -1 bins\n"
" all features into the same class)\n"
"\n"
" r specifies that the weights are initialized to random values in\n"
"   [-r ... +r],\n"
"\n"
" -Pyx_factor f indicates that a parse should be taken as correct\n"
"   proportional to f raised to its f-score, and\n"
"\n"
" -Px_propto_g indicates that each sentence is weighted by the number of\n"
"   edges in its gold parse.\n"
"\n"
" -max-nrounds maxnrounds specifies that at most maxnrounds of cross-validation\n"
"   is to be performed.\n"
"\n"
"The function that the program minimizes is:\n"
"\n"
"   Q(w) = s * (- L(w) + c * sum_j pow(fabs(w[j]), p) ), where:\n"
"\n"
"   L(w) is the loss function to be optimized.\n"
"\n"
"The -cv option instructs the program to optimize a function defined in\n"
"terms of vectors of variables u[] and v[], where w[j] = u[j] + v[j]\n"
"and v[j] <= 0 <= u[j], otherwise it optimizes the w[j] directly.\n"
"\n"
"With debug = 0, the program writes a single line to stdout:\n"
"\n"
"c p r s it nzeroweights/nweights neglogP/nsentence ncorrect/nsentences\n"
"\n"
"With debug >= 10, the program writes out a histogram of weights as well\n"
"\n"
"Data format:\n"
"-----------\n"
"\n"
"<Data>     --> [S=<NS>] <Sentence>*\n"
"<Sentence> --> [G=<G>] N=<N> <Parse>*\n"
"<Parse>    --> [P=<P>] [W=<W>] <FC>*,\n"
"<FC>       --> <F>[=<C>]\n"
"\n"
"NS is the number of sentences.\n"
"\n"
"Each <Sentence> consists of N <Parse>s.  <G> is the gold standard\n"
"score.  To get parsing precision and recall results, set <G> to the\n"
"number of edges in the gold standard parse.  To get accuracy results,\n"
"set <G> to 1 (the default).\n"
"\n"
"A <Parse> consists of <FC> pairs.  <P> is the parse's possible highest\n"
"score and <W> is the parse's actual score.  To get parsing precision and\n"
"recall results, set <P> to the number of edges in the parse and <W> to\n"
"the number of edges in common between the gold and parse trees.\n"
"\n"
"A <FC> consists of a feature (a non-negative integer) and an optional\n"
"count (a real).\n"
"\n"
"The default for all numbers except <W> is 1.  The default for <W> is 0.\n";

#include "custom_allocator.h"    // must come first
#define _GLIBCPP_CONCEPT_CHECKS  // uncomment this for checking

#include <boost/lexical_cast.hpp>
#include <cassert>
#include <cctype>
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>

#include "lmdata.h"
#include "powell.h"
#include "utility.h"
#include "tao-optimizer.h"

typedef std::vector<double> doubles;
typedef std::vector<size_t> size_ts;

int debug_level = 0;

enum loss_type { log_loss, em_log_loss, em_log_loss_noomp, pairwise_log_loss, exp_loss, log_exp_loss, 
		 expected_fscore_loss };

const char* loss_type_name[] = { "log_loss", "em_log_loss", "em_log_loss_noomp", "pairwise_log_loss", "exp_loss", 
				 "log_exp_loss", "expected_fscore_loss" };

void print_histogram(int nx, const double x[], int nbins=20) {
  int nx_nonzero = 0;
  for (int i = 0; i < nx; ++i)
    if (x[i] != 0)
      ++nx_nonzero;

  std::cout << "#   There are " << nx_nonzero << " non-zero values and " 
	    << nx-nx_nonzero << " zero values." << std::endl;

  if (nx_nonzero > 0) {
    std::vector<double> s;
    s.reserve(nx_nonzero);
    for (int i = 0; i < nx; ++i)
      if (x[i] != 0)
	s.push_back(x[i]);
    std::sort(s.begin(), s.end());
    for (int i = 0; i <= nbins; ++i) {
      int j = i*(nx_nonzero-1);
      j /= nbins;
      std::cout << float(i)/float(nbins) << '\t' << s[j] << std::endl;
    }
  }
}  // print_histogram()

// f_df() evaluates the statistics of the corpus, and prints the f-score
//  if required
//
double f_df(loss_type ltype, corpus_type* corpus, double x[], double df_dx[]) {
  Float sum_g = 0, sum_p = 0, sum_w = 0, L = 0;
  
  switch (ltype) {
  case log_loss:
    L = corpus_stats(corpus, &x[0], &df_dx[0], &sum_g, &sum_p, &sum_w);
    break;
  case em_log_loss:
    L = emll_corpus_stats(corpus, &x[0], &df_dx[0], &sum_g, &sum_p, &sum_w);
    break;
  case em_log_loss_noomp:
    L = emll_corpus_stats_noomp(corpus, &x[0], &df_dx[0], &sum_g, &sum_p, &sum_w);
    break;
  case pairwise_log_loss:
    L = pwlog_corpus_stats(corpus, &x[0], &df_dx[0], &sum_g, &sum_p, &sum_w);
    break;
  case exp_loss:
    L = exp_corpus_stats(corpus, &x[0], &df_dx[0], &sum_g, &sum_p, &sum_w);
    break;
  case log_exp_loss:
    L = log_exp_corpus_stats(corpus, &x[0], &df_dx[0], &sum_g, &sum_p, &sum_w);
    break;
  case expected_fscore_loss:
    L = 1 - fscore_corpus_stats(corpus, &x[0], &df_dx[0], &sum_g, &sum_p, &sum_w);
    for (size_type j = 0; j < corpus->nfeatures; ++j)
      df_dx[j] = -df_dx[j];
    break;
  default:
    L = 0;
    std::cerr << "## Error: unrecognized loss_type loss = " << int(ltype) 
	      << std::endl;
  }
  
  if (debug_level >= 1000)
    std::cerr << "f score = " << 2*sum_w/(sum_g+sum_p) << ", " << std::flush;
  
  assert(finite(L));
  return L;
}


// Unconstrained is for unconstrained optimization
//
struct Unconstrained {
  const loss_type ltype;
  corpus_type* corpus;
  const size_ts& f_c;	//!< feature -> cross-validation class
  const doubles& cs;	//!< cross-validation class -> regularizer factor
  double p, s;
  int it;
  double L, R, Q;

  Unconstrained(loss_type ltype, corpus_type* corpus, const size_ts& f_c, 
		const doubles& cs, double p, double s) 
    : ltype(ltype), corpus(corpus), f_c(f_c), cs(cs), p(p), s(s), it(0) 
  { 
    assert(f_c.size() == corpus->nfeatures);
    for (size_type f = 0; f < f_c.size(); ++f)
      assert(f_c[f] < cs.size());
  }

  double operator() (int nx, double x[], double df_dx[]) {
    it++;
    if (debug_level >= 1000)      
      std::cerr << "it = " << it << ", " << std::flush;

    L = f_df(ltype, corpus, x, df_dx);

    if (s != 1) {
      L *= s;

      for (int i = 0; i < nx; ++i) {
	assert(finite(df_dx[i]));
	df_dx[i] *= s;
      }
    }

    assert(size_t(nx) == corpus->nfeatures);
    assert(f_c.size() == corpus->nfeatures);

    R = 0;
    for (int i = 0; i < nx; ++i) 
      R +=  cs[f_c[i]] * pow(fabs(x[i]), p);
    R *= s;

    Q = L + R;
    
    if (debug_level >= 1000)
      std::cerr << "Q = " << Q << " = L = " << L << " + R = " << R << std::endl;

    assert(finite(Q));
    
    double sp = s * p;
    for (int i = 0; i < nx; ++i) 
      df_dx[i] += sp * cs[f_c[i]] * pow(fabs(x[i]), p-1) 
	* (x[i] >= 0 ? (x[i] == 0 ? 0 : 1) : -1);

    if (debug_level >= 10000) {
      std::cerr << "Histogram of derivatives:" << std::endl;
      print_histogram(nx, &df_dx[0]);
      std::cerr << "--------------------------" << std::endl;
    }

    for (int i = 0; i < nx; ++i)
      assert(finite(df_dx[i]));
    return Q;
  }   // Unconstrained::operator()

};  // Unconstrained{}


// Constrained{} for the constrained optimization.
//
struct Constrained {
  const loss_type ltype;
  corpus_type* corpus;
  const size_ts& f_c;	//!< feature -> cross-validation class
  const doubles& cs;	//!< cross-validation class -> regularizer factor
  double p, s;
  int it;
  doubles x, df_dx;
  double L, R, Q;

  Constrained(loss_type ltype, corpus_type* corpus, const size_ts& f_c, 
	      const doubles& cs, double p, double s) 
    : ltype(ltype), corpus(corpus), f_c(f_c), cs(cs), p(p), s(s), it(0),
      x(corpus->nfeatures, 0), df_dx(corpus->nfeatures) { 
    assert(f_c.size() == corpus->nfeatures);
    for (size_type f = 0; f < f_c.size(); ++f)
      assert(f_c[f] < cs.size());
  }

  double operator() (int nx2, double x2[], double df_dx2[]) {
    it++;
    if (debug_level >= 1000)      std::cerr << "it = " << it << ", " << std::flush;
    
    size_type nx = corpus->nfeatures;
    assert(nx2 = 2 * nx);
    assert(x.size() == nx);
    for (size_type i = 0; i < nx; ++i) {
      assert(finite(x2[i]));
      assert(finite(x2[i+nx]));
      x[i] = x2[i] + x2[i+nx];
      assert(finite(x[i]));
    }

    L = f_df(ltype, corpus, &x[0], &df_dx[0]);
    
    L *= s;

    for (size_type i = 0; i < nx; ++i) {
      assert(finite(df_dx[i]));
      df_dx2[i] = df_dx2[i+nx] = s * df_dx[i];
    }

    R = 0;
    for (size_type i = 0; i < nx; ++i)
      R += cs[f_c[i]] * pow(fabs(x2[i]), p) + pow(fabs(x2[i+nx]), p);
    R *= s;

    Q = L + R;
    
    if (debug_level >= 1000)
      std::cerr << "Q = " << Q << " = L = " << L << " + R = " << R << std::endl;

    assert(finite(Q));
    
    double sp = s * p;
    for (size_type i = 0; i < nx; ++i) {
      df_dx2[i] += sp * cs[f_c[i]] * pow(fabs(x2[i]), p-1) * (x2[i] >= 0 ? 1 : -1);
      df_dx2[i+nx] += sp * cs[f_c[i]] * pow(fabs(x2[i+nx]), p-1) * (x2[i+nx] > 0 ? 1 : -1);
    }

    if (debug_level >= 10000) {
      std::cerr << "Histogram of derivatives:" << std::endl;
      print_histogram(nx, &df_dx[0]);
      std::cerr << "--------------------------" << std::endl;
    }

    for (int i = 0; i < nx2; ++i)
      assert(finite(df_dx2[i]));
    return Q;
  }   // Constrained::operator()

};  // Constrained{}


// The Estimator1 does one round of estimation
//
struct Estimator1 {
  typedef std::vector<size_t> size_ts;
  typedef std::vector<double> doubles;
  
  corpus_type* train;	//!< training data
  size_type nx;		//!< number of features
  corpus_type* eval;	//!< evaluation data
  corpus_type* eval2;	//!< 2nd evaluation data
  loss_type ltype;	//!< type of loss function
  double c0;		//!< default regularizer factor
  double c00;           //!< multiply default regularizer factor for first feature class
  double p;		//!< regularizer power
  double r;		//!< random initialization
  double s;		//!< scale factor
  bool cv;		//!< use constrained variables
  bool opt_fscore;	//!< optimize f-score or - log likelihood

  doubles x;		//!< feature -> weight
  size_ts f_c;		//!< feature -> cross-validation class
  doubles lcs;		//!< cross-validation class -> log factor
  size_type nc;		//!< number of cross-validation classes

  size_type nits;	//!< number of iterations of last round
  size_type sum_nits;	//!< total number of iterations
  size_type nrounds;	//!< number of cross-validation rounds so far
  size_type max_nrounds; //!< number of cross-validation rounds to perform
  double best_score;    //!< best score seen so far
  std::string weightsfile; //!< name of weights file

  typedef std::map<std::string,size_t> S_C;
  S_C identifier_regclass; //!< map from feature class identifiers to regularization class
  typedef std::vector<std::string> Ss;
  Ss regclass_identifiers; //!< vector of class identifiers

  Estimator1(loss_type ltype, double c0, double c00, double p, double r, double s, bool cv, 
	     bool opt_fscore = true, size_type max_nrounds = 0, const char* weightsfile = NULL) 
    : train(NULL), nx(0), eval(NULL), eval2(NULL),
      ltype(ltype), c0(c0), c00(c00), p(p), r(r), s(s), cv(cv), 
      opt_fscore(opt_fscore), x(nx), f_c(nx), lcs(1, log(c0)), nc(1), 
      nits(0), sum_nits(0), nrounds(0), max_nrounds(max_nrounds), best_score(0),
      weightsfile(weightsfile == NULL ? "" : weightsfile)
  { }  // Estimator1::Estimator1()

  //! set_data() sets the training and evaluation data
  //
  void set_data(corpus_type* t, corpus_type* e, corpus_type* e2) {
    train = t;
    eval = e;
    eval2 = e2;
    nx = train->nfeatures;
    x.resize(nx);
    assert(f_c.size() <= nx);
    assert(eval == NULL || eval->nfeatures <= train->nfeatures);
    assert(eval2 == NULL || eval2->nfeatures <= train->nfeatures);
  } // Estimator1::set_data()

  // operator() actually runs one round of estimation
  //
  double operator() (const doubles& lccs) {
    assert(lccs.size() == nc);
    doubles ccs(nc);
    double L, R, Q;

    for (size_type i = 0; i < nc; ++i)
      ccs[i] = exp(lccs[i]);

    assert(x.size() == nx);
    nits = 0;
    nrounds++;

    if (debug_level >= 10) {
      if (nrounds == 1) 
	std::cerr << "# round	nfeval	L	R	Q	neglogP	f-score	css" << std::endl;
      std::cerr << nrounds << std::flush;
    }
    if (cv) {

      // Constrained variable optimization

      Constrained fn(ltype, train, f_c, ccs, p, s);
      tao_constrained_optimizer<Constrained> tao_opt(2*nx, fn);

      if (r != 0) 
	for (size_type i = 0; i < 2*nx; ++i)
	  tao_opt[i] = r*double(random()-RAND_MAX/2)/double(RAND_MAX/2);
    
      for (size_type i = 0; i < nx; ++i) {
	tao_opt.lower_bound[i] = 0;
	tao_opt.lower_bound[i+nx] = TAO_NINFINITY;
	tao_opt.upper_bound[i] = TAO_INFINITY;
	tao_opt.upper_bound[i+nx] = 0;
      }
      
      tao_opt.optimize();
    
      nits = fn.it;
      L = fn.L;
      R = fn.R;
      Q = fn.Q;

      for (size_type i = 0; i < nx; ++i)
	x[i] = tao_opt[i] + tao_opt[i+nx];
    }
    else {
      
      // Unconstrained optimization
      
      Unconstrained fn(ltype, train, f_c, ccs, p, s);
      tao_optimizer<Unconstrained> tao_opt(nx, fn);

      if (r != 0) 
	for (size_type i = 0; i < nx; ++i)
	  tao_opt[i] = r*double(random()-RAND_MAX/2)/double(RAND_MAX/2);
      
      tao_opt.optimize();
      
      nits = fn.it;
      L = fn.L;
      R = fn.R;
      Q = fn.Q;

      for (size_type i = 0; i < nx; ++i)
	x[i] = tao_opt[i];
    }

    // Clean up, collect stats

    sum_nits += nits;

    if (debug_level >= 10)
      std::cerr << '\t' << nits << '\t' << L << '\t' << R << '\t' << Q;

    double score = evaluate(opt_fscore, true);

    if (debug_level >= 10)
      std::cerr << '\t' << ccs << std::endl;

    return score;

  }  // Estimator1::operator()

  // evaluate() evaluates the current model on the eval data, prints
  // out debugging information if appropriate, and returns either
  // the - log likelihood or 1 - f-score.
  //
  double evaluate(bool opt_fscore = false, bool internal = false) {

    std::vector<double> df_dx(nx);
    Float sum_g = 0, sum_p = 0, sum_w = 0;
    Float neglogP = corpus_stats(eval, &x[0], &df_dx[0], 
				 &sum_g, &sum_p, &sum_w);
    Float fscore = 2*sum_w/(sum_g+sum_p);
    
    if (internal) {  // internal evaluation, use a short print-out
      if (debug_level >= 10) {
	std::cerr << '\t' << neglogP << '\t' << fscore; 
	if (eval2 != NULL) {
	  Float sum_g2 = 0, sum_p2 = 0, sum_w2 = 0;
	  Float neglogP2 = corpus_stats(eval2, &x[0], &df_dx[0], 
					&sum_g2, &sum_p2, &sum_w2);
	  Float fscore2 = 2*sum_w2/(sum_g2+sum_p2);
	  std::cerr << '\t' << neglogP2 << '\t' << fscore2;
	}
      }
    }
    else { // final evaluation, print out more info
    
      int nzeros = 0;
      for (size_type i = 0; i < nx; ++i) 
	if (x[i] == 0)
	  ++nzeros;
  
      std::cerr << "# Regularizer power p = " << p << std::endl;
      std::cerr << "# " << nx-nzeros << " non-zero feature weights of " 
		<< nx << " features." << std::endl;
      std::cerr << "# Eval neglogP = " << neglogP 
		<< ", neglogP/nsentences = " << neglogP/eval->nsentences
		<< std::endl;
      std::cerr << "# Eval precision = " << sum_w/sum_p 
		<< ", recall = " << sum_w/sum_g
		<< ", f-score = " << 2*sum_w/(sum_g+sum_p)
		<< std::endl;
      if (eval2 != NULL) {
	Float sum_g = 0, sum_p = 0, sum_w = 0;
	Float neglogP = corpus_stats(eval2, &x[0], &df_dx[0], 
				     &sum_g, &sum_p, &sum_w);
	std::cerr << "# Eval2 neglogP = " << neglogP 
		  << ", neglogP/nsentences = " << neglogP/train->nsentences
		  << std::endl;
	std::cerr << "# Eval2 precision = " << sum_w/sum_p 
		  << ", recall = " << sum_w/sum_g
		  << ", f-score = " << 2*sum_w/(sum_g+sum_p)
		  << std::endl;
      }
      {
	Float sum_g = 0, sum_p = 0, sum_w = 0;
	Float neglogP = corpus_stats(train, &x[0], &df_dx[0], 
				     &sum_g, &sum_p, &sum_w);
	std::cerr << "# Train neglogP = " << neglogP 
		  << ", neglogP/nsentences = " << neglogP/train->nsentences
		  << std::endl;
	std::cerr << "# Train precision = " << sum_w/sum_p 
		  << ", recall = " << sum_w/sum_g
		  << ", f-score = " << 2*sum_w/(sum_g+sum_p)
		  << std::endl;
      }

      std::cerr << "# regclass_identifiers = " << regclass_identifiers << std::endl;
      std::cerr << "# lcs = " << lcs << std::endl;
      {
	doubles cs(nc);
	for (size_type i = 0; i < nc; ++i)
	  cs[i] = exp(lcs[i]);
	std::cerr << "# cs = " << cs << std::endl;
      }
      if (debug_level >= 100) {
	std::cerr << "# Cumulative distribution of feature weights:" << std::endl;
	print_histogram(nx, &x[0]);
      }
    }

    double score = (opt_fscore ? 1 - fscore : neglogP);

    if (nrounds == 1 || score < best_score) {
      best_score = score;

      // Write out weights file
    
      if (!weightsfile.empty()) {
	FILE* out = fopen(weightsfile.c_str(), "w");
	// fprintf(out, "%d@", nx-nzeros);
	for (size_type i = 0; i < x.size(); ++i) 
	  if (x[i] != 0) {
	    fprintf(out, "%d", i);
	    if (x[i] != 1)
	      fprintf(out, "=%g", x[i]);
	    fprintf(out, "\n");
	  }
	fclose(out);
      }      
    }
    return score;
  } // Estimator1::evaluate()

  //! fc_bin() maps a feature count to its corresponding bin
  //
  static int fc_bin(double feature_count_base, int feature_count) {
    if (feature_count <= 4)
      return feature_count;
    else
      return lrint(4.0 + pow(feature_count_base, 
			     lrint(log(feature_count-4)/log(feature_count_base))));
  }  // Estimator1::fc_bin()

  // read_featureclasses() reads the feature classes from a feature file
  //
  void read_featureclasses(const char* filename, 
			   int nseparators = 1,
			   const char* separators = ":") {
    
    const char* filesuffix = strrchr(filename, '.');
    bool popen_flag = false;
    FILE *in;
    if (strcasecmp(filesuffix, ".bz2") == 0) {
      std::string command("bzcat ");
      command += filename;
      in = popen(command.c_str(), "r");
      if (in == NULL) {
	perror("## Error in lm-owlqn: ");
	std::cerr << "## popen(\"" << command << "\", \"r\") failed, usage = " << resource_usage() << std::endl;
      }
      popen_flag = true;
    }
    else if (strcasecmp(filesuffix, ".gz") == 0) {
      std::string command("gunzip -c ");
      command += filename;
      errno = 0;
      in = popen(command.c_str(), "r");
      if (in == NULL) {
	perror("## Error in lm-owlqn: ");
	std::cerr << "## popen(\"" << command << "\", \"r\") failed, usage = " << resource_usage() << std::endl;
      }
      popen_flag = true;
    }
    else
      in = fopen(filename, "r");
    if (in == NULL) {
      std::cerr << "## Couldn't open evalfile " << filename
		<< ", errno = " << errno << "\n" 
		<< usage << std::endl;
      exit(EXIT_FAILURE);
    }

    size_type featno;

    // read feature number 

    while (fscanf(in, " %u ", &featno) == 1) {
      int c = ':';
      
      // read the prefix of the feature class identifier
      
      std::string identifier;
      int iseparators = 0;
      if (nseparators >= 0)
	while ((c = getc(in)) != EOF && !isspace(c)) {
	  if (index(separators, c) != NULL)
	    if (++iseparators > nseparators)
	      break;
	  identifier.push_back(c);
	}
      
      // skip the rest of the line

      while ((c = getc(in)) != EOF && c != '\n')
	;

      // insert the prefix into the prefix -> regularization class map
      
      S_C::iterator it 
	= identifier_regclass.insert(S_C::value_type(identifier, 
						     identifier_regclass.size())).first;
      
      size_type cl = it->second;    // regularization class

      f_c.resize(featno+1);
      f_c[featno] = cl;          // set feature's regularization class
    }
      
    nc = identifier_regclass.size();   // set nc
    lcs.resize(nc, log(c0));           // set each regularizer class' factor to c0
    lcs[0] += log(c00);                // increment first regularizer class' factor by c00

    // construct regclass_identifiers
    
    regclass_identifiers.resize(nc);
    cforeach (S_C, it, identifier_regclass) {
      assert(it->second < regclass_identifiers.size());
      regclass_identifiers[it->second] = it->first;
    }

    if (debug_level >= 0) 
      std::cerr << "# Regularization classes: " << regclass_identifiers << std::endl;

    if (popen_flag)
      pclose(in);
    else
      fclose(in);
  }  // Estimator1::read_featureclasses() 
    
  void estimate()
  {
    if (max_nrounds == 1) 
      operator()(lcs);
    else {
      if(max_nrounds == 0)
	max_nrounds = (lcs.size() > 1) ? 11 : 51;
      powell::control cntrl(1e-4, 1e-2, 0, max_nrounds);
      powell::minimize(lcs, *this, log(2), cntrl);
    }

    if (debug_level > 0) {
      std::cerr << "# Regularizer class weights = (";
      for (size_type i = 0; i < lcs.size(); ++i) {
	if (i > 0)
	  std::cerr << ' ';
	std::cerr << exp(lcs[i]);
      }
      std::cerr << ')' << std::endl;
    }

  }  // Estimator1::estimate()

};  // Estimator1{}


int main(int argc, char** argv) 
{
  errno = 0;

  std::ios::sync_with_stdio(false);

  // Initialize TAO and PETSc

  tao_environment tao_env(argc, argv);

  if (tao_env.get_bool_option("-help") || tao_env.get_bool_option("--help")) {
    std::cerr << "-help\n" << usage << std::endl;
    exit(EXIT_SUCCESS);
  }

  debug_level = tao_env.get_int_option("-debug", debug_level);
  loss_type ltype = loss_type(tao_env.get_int_option("-l", 0));
  double c0 = tao_env.get_double_option("-c0", 2.0);
  double c00 = tao_env.get_double_option("-c00", 1.0);
  double p = tao_env.get_double_option("-p", 2.0);
  double r = tao_env.get_double_option("-r", 0.0);
  double s = tao_env.get_double_option("-s", 1.0);
  bool cv = tao_env.get_bool_option("-cv");
  double Pyx_factor = tao_env.get_double_option("-Pyx_factor", 0.0);
  bool Px_propto_g = tao_env.get_bool_option("-Px_propto_g");
  size_type max_nrounds = tao_env.get_int_option("-max-nrounds", 0);

  if (debug_level >= 10)
    std::cerr << "#  ltype = " << ltype
	      << " (" << loss_type_name[ltype] << ")"
	      << ", regularization c0 = " << c0
	      << ", c00 = " << c00
	      << ", power p = " << p 
	      << ", scale s = " << s
	      << "; random init r = " << r 
	      << ", constrained var optimization cv = " << cv
	      << ", Pyx_factor = " << Pyx_factor
	      << ", Px_propto_g = " << Px_propto_g
	      << ", max_nrounds = " << max_nrounds
	      << std::endl;

  // I discovered a couple of years after I wrote this program that popen
  // uses fork, which doubles your virtual memory for a short instant!

  Estimator1 e(ltype, c0, c00, p, r, s, cv, true,
	       max_nrounds, tao_env.get_cstr_option("-o"));

  int nseparators = tao_env.get_int_option("-ns", 1);
  const char* filename = tao_env.get_cstr_option("-f");
  if (filename != NULL)
    e.read_featureclasses(filename, nseparators, ":");  

  // Read in eval data first, as that way we may squeeze everything into 4GB

  corpusflags_type corpusflags = { Pyx_factor, Px_propto_g };
  
  corpus_type* evaldata = NULL;
  const char* evalfile = tao_env.get_cstr_option("-e");
  if (evalfile != NULL) {
    evaldata = read_corpus_file(&corpusflags, evalfile);
    if (debug_level >= 10)
      std::cerr << "# read evalfile = " << evalfile 
		<< ", nsentences = " << evaldata->nsentences
		<< std::endl;
  }

  corpus_type* evaldata2 = NULL;
  const char* evalfile2 = tao_env.get_cstr_option("-x");
  if (evalfile2 != NULL) {
    evaldata2 = read_corpus_file(&corpusflags, evalfile2);
    if (debug_level >= 10)
      std::cerr << "# read evalfile2 = " << evalfile2 
		<< ", nsentences = " << evaldata2->nsentences
		<< std::endl;
  }

  corpus_type* traindata = read_corpus(&corpusflags, stdin);
  int nx = traindata->nfeatures;

  if (errno != 0) {
    perror("## cvlm, after reading main corpus, nonzero errno  ");
    errno = 0;
  }

  std::cerr << "# " << nx << " features in training data, " << resource_usage() << std::endl;

  if (evaldata == NULL)
    evaldata = traindata;

  e.set_data(traindata, evaldata, evaldata2);
  e.estimate();

}  // main()
