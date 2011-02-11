// hlm.cc -- An estimator for hierarchical linear models

const char usage[] =
"hlm -- A hierarchical linear model estimator using a variety of user-selectable loss functions.\n"
"\n"
" Mark Johnson, 25th August 2008\n"
"\n"
" It uses a modified version of L-BFGS developed by Galen Andrew at Microsoft Research\n"
" to be especially efficient for L1-regularized loss functions.\n"
"\n"
" The regularizer weight(s) are set by cross-validation on development data.\n"
"\n"
"Usage: hlm [-h] [-d debug_level] [-c c0] [-p p] [-r r] [-s s] [-t tol]\n"
"           [-l ltype] [-F f] [-G] -f feat-file [-n ns] [-S nshared]\n"
"           [-o weights-file]  [-e eval-file] [-x eval-file2]\n"
"	    < train-file\n"
"\n"
"where:\n"
"\n"
" debug_level > 0 controls the amount of output produced\n"
"\n"
" -c c0 is the initial value for the regularizer constant.\n"
"\n"
" -l ltype identifies the type of loss function used:\n"
"\n"
"    -l 0 - log loss (c0 ~ 5)\n"
"    -l 1 - EM-style log loss (c0 ~ 5)\n"
"    -l 2 - pairwise log loss \n"
"    -l 3 - exp loss (c0 ~ 25, s ~ 1e-5)\n"
"    -l 4 - log exp loss (c0 ~ 1e-4)\n"
"    -l 5 - maximize expected F-score (c ~ ?)\n"
"\n"
" -r r specifies that the weights are initialized to random values in\n"
"   [-r ... +r],\n"
"\n"
" -t tol specifies the stopping tolerance for the IWLQN optimizer\n"
"\n"
" -F f indicates that a parse should be taken as correct\n"
"   proportional to f raised to its f-score, and\n"
"\n"
" -G indicates that each sentence is weighted by the number of\n"
"   edges in its gold parse.\n"
"\n"
" -f feat-file is a file of <featclass> <featuredetails> lines, used for\n"
" cross-validating regularizer weights,\n"
"\n"
" -n ns is the maximum number of ':' characters in a <featclass>, used to\n"
" determine how features are binned into feature classes\n"
"\n"
" -S nshared ensures that the first nshared regularizer classes appear in\n"
" the first-level classifiers\n"
"\n"
" train-file, eval-file and eval-file2 are files from which training and evaluation\n"
" data are read (if eval-file ends in the suffix .bz2 then bzcat is used\n"
" to read it; if no eval-file is specified, then the program tests on the\n"
" training data),\n"
"\n"
" weights-file is a file to which the estimated weights are written,\n"
"\n"
"The function that the program minimizes is:\n"
"\n"
"   Q(w) = s * (- L(w) + c * sum_j pow(fabs(w[j]), p) ), where:\n"
"\n"
"   L(w) is the loss function to be optimized.\n"
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

#include <cassert>
#include <cctype>
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <vector>

#include "lmdata.h"
#include "powell.h"
#include "utility.h"

#include "OWLQN.h"
#include "TerminationCriterion.h"

typedef std::vector<double> doubles;
typedef std::vector<feature_type> features_type;
typedef std::vector<features_type> featuress_type;

int debug_level = 0;

//! exit_failure() causes the program to halt immediately
//
inline std::ostream& exit_failure(std::ostream& os) {
  os << std::endl;
  exit(EXIT_FAILURE);
  return os;
}  // exit_failure

//! project_subcorpus() conses up a subcorpus only using features in f_sf
//
corpus_type* project_subcorpus(const corpus_type* c0, const features_type& f_sf, feature_type nfeatures) {
  corpus_type* c = (corpus_type*) malloc(sizeof(corpus_type));
  assert(c);
  c->nsentences = c0->nsentences;
  c->nfeatures = nfeatures;
  c->maxnparses = c0->maxnparses;
  c->nloserparses = c0->nloserparses;
  c->sentence = (sentence_type*) malloc(c->nsentences*sizeof(sentence_type));
  assert(c->sentence);
  for (size_type i = 0; i < c->nsentences; ++i) {
    const sentence_type* s0 = &c0->sentence[i];
    sentence_type* s = &c->sentence[i];
    s->nparses = s0->nparses;
    s->parse = (parse_type*) malloc(s->nparses*sizeof(parse_type));
    assert(s->parse);
    s->correct_index = s0->correct_index;
    s->Px = s0->Px;
    s->g = s0->g;
    for (size_type j = 0; j < s0->nparses; ++j) {
      const parse_type* p0 = &s0->parse[j];
      parse_type* p = &s->parse[j];
      p->Pyx = p0->Pyx;
      p->p = p0->p;
      p->w = p0->w;
      size_type nf = 0;
      for (size_type k0 = 0; k0 < p0->nf; ++k0)
	if (f_sf[p0->f[k0]] != feature_type(-1))
	  ++nf;
      assert(nf <= p0->nf);
      p->nf = nf;
      p->f = (feature_type*) malloc(p->nf*sizeof(feature_type));
      assert(p->f);
      size_type k = 0;
      for (size_type k0 = 0; k0 < p0->nf; ++k0)
	if (f_sf[p0->f[k0]] != feature_type(-1))
	  p->f[k++] = f_sf[p0->f[k0]];
      assert(k == nf);

      size_type nfc = 0;
      for (size_type l0 = 0; l0 < p0->nfc; ++l0)
	if (f_sf[p0->fc[l0].f] != feature_type(-1))
	  ++nfc;
      assert(nfc <= p0->nfc);
      p->nfc = nfc;
      p->fc = (fc_type*) malloc(p->nfc*sizeof(fc_type));
      assert(p->fc);
      size_type l = 0;
      for (size_type l0 = 0; l0 < p0->nfc; ++l0)
	if (f_sf[p0->fc[l0].f] != feature_type(-1)) {
	  p->fc[l].f = f_sf[p0->fc[l0].f];
	  p->fc[l++].c = p0->fc[l0].c;
	}
      assert(l == nfc);
    }
  }
  return c;
}  // project_subcorpus()

//! free_subcorpus() free's all of the structures in a subcorpus.
//! Horrible things would happen if you tried to free a corpus that
//! wasn't constructed using malloc()
//
void free_subcorpus(corpus_type* c) {
  for (size_type i = 0; i < c->nsentences; ++i) {
    sentence_type* s = &c->sentence[i];
    for (size_type j = 0; j < s->nparses; ++j) {
      parse_type* p = &s->parse[j];
      if (p->f)
	free(p->f);
      if (p->fc)
	free(p->fc);
    }
    free(s->parse);
  }
  free(c->sentence);
  free(c);
}  // free_subcorpus()

bool check_corpus(const corpus_type* c) {
  for (size_type i = 0; i < c->nsentences; ++i) {
    const sentence_type* s = &c->sentence[i];
    assert(s->g > 0.0);
    assert(s->Px > 0.0);
    assert(s->correct_index < s->nparses);
    Float sum_Pyx = 0;
    for (size_type j = 0; j < s->nparses; ++j) {
      const parse_type* p = &s->parse[j];
      sum_Pyx += p->Pyx;
      assert(p->Pyx >= 0.0);
      assert(p->p > 0.0);
      assert(p->w >= 0.0);
      for (size_type k = 0; k < p->nf; ++k)
	assert(p->f[k] < c->nfeatures);
      for (size_type k = 0; k < p->nfc; ++k) {
	assert(p->fc[k].f < c->nfeatures);
	assert(finite(p->fc[k].c));
	assert(p->fc[k].c*1.0 == 1.0*p->fc[k].c);
      }
    }
    if (!finite(sum_Pyx) || sum_Pyx <= 0) {
      std::cerr << "## Error in hlm.cc: sum_Pyx = " << sum_Pyx 
		<< ", s->nparses = " << s->nparses 
		<< std::endl;
      assert(sum_Pyx > 0);
    }
  }
  return true;
}  // check_corpus()

enum loss_type { log_loss, em_log_loss, pairwise_log_loss, exp_loss, log_exp_loss, 
		 expected_fscore_loss };

// f_df() evaluates the statistics of the corpus, and prints the f-score
//  if required
//
double f_df(loss_type ltype, corpus_type* corpus, const double x[], double df_dx[]) {
  Float sum_g = 0, sum_p = 0, sum_w = 0, L = 0;
  
  switch (ltype) {
  case log_loss:
    L = corpus_stats(corpus, &x[0], &df_dx[0], &sum_g, &sum_p, &sum_w);
    break;
  case em_log_loss:
    L = emll_corpus_stats(corpus, &x[0], &df_dx[0], &sum_g, &sum_p, &sum_w);
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

// LossFn() is the loss function
//
class LossFn : public DifferentiableFunction {
public:

  const loss_type ltype;
  corpus_type* corpus;
  double c, p, s, x0;
  int it;
  double L, R, Q;

  LossFn(loss_type ltype, corpus_type* corpus, double c, double p, double s, double x0) 
    : ltype(ltype), corpus(corpus), c(c), p(p), s(s), x0(x0), it(0) { }

  virtual double Eval(const DblVec& x, DblVec& df_dx) {
    size_type nx = x.size();

    assert(nx == corpus->nfeatures);

    it++;

    if (debug_level >= 1000) {
      std::cerr << "#   it = " << it << ", " << std::flush;
      if (debug_level >= 10000)
	std::cerr << "x =" << x << ", " << std::flush;
    }

    for (size_type i = 0; i < nx; ++i) {
      assert(finite(x[i]));
      assert(!isnan(x[i]));
      assert(x[i]+1 != x[i]);
    }

    L = f_df(ltype, corpus, &x[0], &df_dx[0]);

    if (s != 1) {
      L *= s;

      for (size_type i = 0; i < nx; ++i) {
	assert(finite(df_dx[i]));
	df_dx[i] *= s;
      }
    }

    R = 0;
    if (p != 1) {
      for (size_type i = 0; i < nx; ++i) 
	R += pow(fabs(x[i]-x0), p);
      R *= c*s;
      double spc = s * p * c;
      for (size_type i = 0; i < nx; ++i) {
	double x1 = x[i] - x0;
	df_dx[i] += spc * pow(fabs(x1), p-1) 
	  * (x1 >= 0 ? (x1 == 0 ? 0 : 1) : -1);
      }
    }

    Q = L + R;
    
    if (debug_level >= 1000)
      std::cerr << "Q = " << Q << " = L = " << L << " + R = " << R << std::endl;

    assert(finite(Q));
    
    for (size_type i = 0; i < nx; ++i)
      assert(finite(df_dx[i]));
    return Q;
  }   // LossFn::operator()

};  // LossFn{}

// The Estimator1 does one round of estimation
//
struct Estimator1 {
  typedef std::vector<double> doubles;
  
  corpus_type* train;	//!< training data
  corpus_type* eval;	//!< evaluation data
  size_type nx;		//!< number of features

  double p;		//!< regularizer power
  double r;		//!< random initialization
  double s;		//!< scale factor
  double x0;            //!< default feature weight
  double tol;           //!< stopping tolerance
  bool opt_fscore;	//!< optimize f-score or - log likelihood
  loss_type ltype;	//!< type of loss function

  doubles x;		//!< feature -> weight

  size_type nits;	//!< number of iterations of last round
  size_type sum_nits;	//!< total number of iterations
  size_type nrounds;	//!< number of cross-validation rounds so far

  double best_score;    //!< best score seen so far
  double best_fscore;   //!< f-score at best c
  double best_neglogP;  //!< -log P at best c
  double best_c;
  doubles best_x;       //!< best feature -> weight seen so far

  Estimator1() 
    : train(NULL), eval(NULL), nx(0),
      nits(0), sum_nits(0), nrounds(0), 
      best_score(0), best_fscore(0), best_neglogP(0), best_c(0)
  { }  // Estimator1::Estimator1()

  //! set_data() sets the training and evaluation data
  //
  void set_data(corpus_type* t, corpus_type* e) {
    train = t;
    eval = e;
    nx = train->nfeatures;
    x.resize(nx);
    assert(eval == NULL || eval->nfeatures <= train->nfeatures);
  } // Estimator1::set_data()

  // evaluate() evaluates the current model on the eval data, prints
  // out debugging information if appropriate, and returns either
  // the - log likelihood or 1 - f-score.
  //
  double evaluate(bool opt_fscore) {
    std::vector<double> df_dx(nx);
    Float sum_g = 0, sum_p = 0, sum_w = 0;
    Float neglogP = corpus_stats(eval, &x[0], &df_dx[0], 
				 &sum_g, &sum_p, &sum_w);
    Float fscore = 2*sum_w/(sum_g+sum_p);
    double score = (opt_fscore ? 1 - fscore : neglogP);
    if (nrounds <= 1 || score < best_score) {
      best_score = score;
      best_x = x;
      best_fscore = fscore;
      best_neglogP = neglogP;
    }
    return score;
  } // Estimator1::evaluate()

  // operator() actually runs one round of estimation
  //
  double operator() (double lc) {
    double c = exp(lc);
    assert(finite(c));
    assert(x.size() == nx);
    nits = 0;
    nrounds++;

    if (debug_level >= 100) {
      if (nrounds <= 1)
	std::cerr << "#  round\tregularizer-c\tnfeval\tscore" << std::endl;
      std::cerr << "#  " << nrounds << '\t' << c << std::flush;
    }

    LossFn fn(ltype, train, c, p, s, x0);

    DblVec xx(nx, x0);

    if (r != 0) 
      for (size_type i = 0; i < nx; ++i)
	xx[i] = x0+r*double(random()-RAND_MAX/2)/double(RAND_MAX/2);
    
    OWLQN owlqn((debug_level < 1000));
    if (p == 1) 
      owlqn.Minimize(fn, xx, x, c, tol);
    else
      owlqn.Minimize(fn, xx, x, 0, tol);      
      
    nits = fn.it;

    // Clean up, collect stats

    sum_nits += nits;
    double score = evaluate(opt_fscore);
    if (debug_level >= 100) 
      std::cerr << '\t' << fn.it << '\t' << score << std::endl;
    return score;
  }  // Estimator1::operator()

  void estimate(double p_, double r_, double s_, double tol_, double c0, bool opt_fscore_, loss_type ltype_, double x0_=0, double dc0=4)
  {
    p = p_;
    r = r_;
    s = s_;
    x0 = x0_;
    tol = tol_;
    opt_fscore = opt_fscore_;
    ltype = ltype_;
    best_c = exp(powell::minimize1d(log(c0), *this, log(dc0), 1e-2, 12));
  }  // Estimator1::estimate()

};  // Estimator1{}

// Estimator2{} estimates the 2nd level of the hierarchy
//
struct Estimator2 {
  size_type nx;		//!< number of features
  corpus_type* train;	//!< training data
  corpus_type* eval;	//!< evaluation data
  corpus_type* eval2;	//!< 2nd evaluation data

  loss_type ltype;	//!< type of loss function
  double c0;		//!< initial first level regularizer factor
  double c1;		//!< initial second level regularizer factor
  double p;		//!< regularizer power
  double r;		//!< random initialization
  double s;		//!< scale factor
  double tol;           //!< stopping tolerance
  bool opt_fscore;	//!< optimize f-score or - log likelihood

  features_type f_c;	//!< feature -> cross-validation class
  size_type nc;         //!< number of feature classes

  doubles x;		//!< feature -> weight

  typedef std::map<std::string,size_t> S_C;
  S_C identifier_regclass; //!< map from feature class identifiers to regularization class
  typedef std::vector<std::string> Ss;
  Ss regclass_identifiers; //!< vector of class identifiers

  typedef std::vector<Estimator1> Estimator1s;
  Estimator1s e1s;       //!< first-stage models
  Estimator1 e2;         //!< second-stage model

  Estimator2(loss_type ltype, double c0, double c1, double p, double r, double s,
	     double tol, bool opt_fscore = true) 
    : nx(0), train(NULL), eval(NULL), eval2(NULL), 
      ltype(ltype), c0(c0), c1(c1), p(p), r(r), s(s), tol(tol), 
      opt_fscore(opt_fscore), nc(0) { }

  //! set_data() sets the training and evaluation data
  //
  void set_data(corpus_type* t, corpus_type* e, corpus_type* e2) {
    train = t;
    eval = e;
    eval2 = e2;
    nx = train->nfeatures;
    assert(eval == NULL || eval->nfeatures <= train->nfeatures);
    assert(eval2 == NULL || eval2->nfeatures <= train->nfeatures);
  } // Estimator2::set_data()

  //! read_featureclasses() reads the feature classes from a feature file
  //
  void read_featureclasses(const char* filename,
			   int nseparators, 
			   const char* separators) {
    const char* filesuffix = strrchr(filename, '.');
    bool popen_flag = false;
    FILE *in;
    if (strcasecmp(filesuffix, ".bz2") == 0) {
      std::string command("bzcat ");
      command += filename;
      in = popen(command.c_str(), "r");
      if (in == NULL) {
	perror("## Error in hlm: ");
	std::cerr << "## popen(\"" << command << "\", \"r\") failed, usage = " << resource_usage() << std::endl;
      }
      popen_flag = true;
    }
    else if (strcasecmp(filesuffix, ".gz") == 0) {
      std::string command("zcat ");
      command += filename;
      errno = 0;
      in = popen(command.c_str(), "r");
      if (in == NULL) {
	perror("## Error in hlm: ");
	std::cerr << "## popen(\"" << command << "\", \"r\") failed, usage = " << resource_usage() << std::endl;
      }
      popen_flag = true;
    }
    else
      in = fopen(filename, "r");
    if (in == NULL) {
      std::cerr << "## Error in hlm: Couldn't open featureclass file " << filename
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

    // construct regclass_identifiers
    
    regclass_identifiers.resize(nc);
    cforeach (S_C, it, identifier_regclass) {
      assert(it->second < regclass_identifiers.size());
      regclass_identifiers[it->second] = it->first;
    }

    if (debug_level >= 10) 
      std::cerr << "# Regularization classes: " << regclass_identifiers << std::endl;

    if (popen_flag)
      pclose(in);
    else
      fclose(in);
  }  // Estimator2::read_featureclasses()

  //! set_sfid() computes the mapping from feature ids to subset feature ids.
  //! An sfid = -1 means "ignore this feature"
  //
  static feature_type set_sfid(const features_type& f_c, size_type nshared, 
			       size_type c, features_type& f_sf) {
    f_sf.resize(f_c.size());
    feature_type nf = 0;
    for (feature_type f = 0; f < f_c.size(); ++f) 
      if (f_c[f] < nshared || f_c[f] == c) {
	f_sf[f] = nf++;
	assert(nf < feature_type(-1));
      }
      else
	f_sf[f] = feature_type(-1);
    return nf;
  }  // Estimator2::set_sfid()

  static double parse_score2(const parse_type* p, const features_type& f_sf, const doubles& w) {
    double score = 0;
    for (size_type i = 0; i < p->nf; ++i) {
      feature_type sf = f_sf[p->f[i]];
      if (sf != feature_type(-1)) {
	assert(sf < w.size());
	score += w[sf];
      }
    }
    for (size_type i = 0; i < p->nfc; ++i) {
      feature_type sf  = f_sf[p->fc[i].f];
      if (sf != feature_type(-1)) {
	assert(sf < w.size());
	score += p->fc[i].c * w[sf];
      }
    }
    assert(finite(score));
    return score;
  }  // Estimator2::parse_score2()

  corpus_type* second_stage_corpus(const corpus_type* c0, const featuress_type& fid_sfids) const {
    size_type nfeat = e1s.size();
    assert(fid_sfids.size() == nfeat);
    corpus_type* c = (corpus_type*) malloc(sizeof(corpus_type));
    assert(c);
    c->nsentences = c0->nsentences;
    c->nfeatures = e1s.size();
    c->maxnparses = c0->maxnparses;
    c->nloserparses = c0->nloserparses;
    c->sentence = (sentence_type*) malloc(c->nsentences*sizeof(sentence_type));
    assert(c->sentence);
    for (size_type i = 0; i < c->nsentences; ++i) {
      const sentence_type* s0 = &c0->sentence[i];
      sentence_type* s = &c->sentence[i];
      s->nparses = s0->nparses;
      s->parse = (parse_type*) malloc(s->nparses*sizeof(parse_type));
      assert(s->parse);
      s->correct_index = s0->correct_index;
      s->Px = s0->Px;
      s->g = s0->g;
      // copy parse info, allocate fc
      for (size_type j = 0; j < s->nparses; ++j) {
	const parse_type* p0 = &s0->parse[j];
	parse_type* p = &s->parse[j];
	p->Pyx = p0->Pyx;
	p->p = p0->p;
	p->w = p0->w;
	p->f = NULL;
	p->nf = 0;
	p->nfc = nfeat;
	p->fc = (fc_type*) malloc(p->nfc*sizeof(fc_type));
	assert(p->fc);
      }
      if (s->nparses > 0)
	for (size_type k = 0; k < nfeat; ++k) {
	  assert(k < fid_sfids.size());
	  assert(k < e1s.size());
	  double max_score = parse_score2(&s0->parse[0], fid_sfids[k], e1s[k].x);
	  for (size_type j = 1; j < s->nparses; ++j)
	    max_score = std::max(max_score, parse_score2(&s0->parse[j], fid_sfids[k], e1s[k].x));
	  assert(finite(max_score));
	  for (size_type j = 0; j < s->nparses; ++j) {
	    s->parse[j].fc[k].f = k;
	    s->parse[j].fc[k].c = parse_score2(&s0->parse[j], fid_sfids[k], e1s[k].x) - max_score;
	    assert(finite(s->parse[j].fc[k].c));
	  }
	}
    }
    return c;
  }  // Estimator2::second_stage_corpus()

  void estimate(size_type nshared) {
    // select regclass subset
    if (debug_level >= 10) 
      std::cerr << "# " << nc << " regularization classes, of which the first " 
		<< nshared << " are shared." << std::endl;
    assert(nshared < nc);
    e1s.resize(nc-nshared);
    featuress_type fid_sfids(nc-nshared);
    if (debug_level >= 10)
      std::cerr << "\n# Level 1:\n# Featureclass\tnx\tc\t-logP\tf-score" << std::endl;
    for (size_type i = 0; i < e1s.size(); ++i) {
      feature_type nf = set_sfid(f_c, nshared, nshared+i, fid_sfids[i]);
      corpus_type* subtrain = project_subcorpus(train, fid_sfids[i], nf);
      corpus_type* subeval = project_subcorpus(eval, fid_sfids[i], nf);
      assert(check_corpus(subtrain));
      assert(check_corpus(subeval));
      e1s[i].set_data(subtrain, subeval);
      e1s[i].estimate(p, r, s, tol, c0, opt_fscore, ltype);
      if (debug_level >= 10) 
	std::cerr << "# " << regclass_identifiers[i+nshared] << '\t' << e1s[i].nx << '\t' << e1s[i].best_c
		  << '\t' << e1s[i].best_neglogP << '\t' <<  e1s[i].best_fscore << std::endl;
      free_subcorpus(subtrain);
      free_subcorpus(subeval);
    }
    corpus_type* second_train = second_stage_corpus(train, fid_sfids);
    corpus_type* second_eval = second_stage_corpus(eval, fid_sfids);
    check_corpus(second_train);
    check_corpus(second_eval);
    Estimator1 e2;
    e2.set_data(second_train, second_eval);
    e2.estimate(p, r, s, tol, c1, opt_fscore, ltype, 1, 10);
    if (debug_level >= 10)
      std::cerr << "\n# Level 2: c = " << e2.best_c
		<< ", -logP = " << e2.best_neglogP 
		<< ", f-score = " << e2.best_fscore 
		<< "\n# x = " << e2.x << std::endl;
    x.clear();
    x.resize(nx);
    for (size_type i = 0; i < e1s.size(); ++i)
      for (feature_type j = 0; j < nx; ++j)
	if (fid_sfids[i][j] != feature_type(-1))
	  x[j] += e2.x[i]*(e1s[i].x[fid_sfids[i][j]]);
    free_subcorpus(second_train);
    free_subcorpus(second_eval);
  }  // Estimator2::estimate()
  
};  // Estimator2

void write_weights(const std::string& weightsfile, const doubles& x) {
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

int main(int argc, char** argv) 
{
  std::ios::sync_with_stdio(false);

  loss_type ltype = log_loss;
  double c0 = 2.0, c1 = 2.0;
  double p = 2.0;
  double r = 0.0;
  double s = 1.0;
  double tol = 1e-5;
  double Pyx_factor = 0.0;
  bool Px_propto_g = false;
  int nseparators = 1;
  int nshared = 1;
  std::string feat_file, weights_file, eval_file, eval2_file;
  int opt;
  while ((opt = getopt(argc, argv, "F:GS:c:C:d:e:f:hl:n:o:p:r:s:t:x:")) != -1) 
    switch (opt) {
    case 'h':
      std::cerr << usage << exit_failure;
      break;
    case 'd':
      debug_level = atoi(optarg);
      break;
    case 'S':
      nshared = atoi(optarg);
      break;
    case 'c':
      c0 = atof(optarg);
      break;
    case 'C':
      c1 = atof(optarg);
      break;
    case 'p':
      p = atof(optarg);
      break;
    case 'r':
      r = atof(optarg);
      break;
    case 's':
      s = atof(optarg);
      break;
    case 't':
      tol = atof(optarg);
      break;
    case 'l':
      ltype = loss_type(atoi(optarg));
      break;
    case 'F':
      Pyx_factor = atof(optarg);
      break;
    case 'G':
      Px_propto_g = true;
      break;
    case 'n':
      nseparators = atoi(optarg);
      break;
    case 'f':
      feat_file = optarg;
      break;
    case 'o':
      weights_file = optarg;
      break;
    case 'e':
      eval_file = optarg;
      break;
    case 'x':
      eval2_file = optarg;
      break;
    }

  if (debug_level >= 10)
    std::cerr << "#  debug_level -d = " << debug_level
	      << ", ltype -l = " << ltype
	      << ", initial level 1 regularizer weight -c = " << c0
	      << ", initial level 2 regularizer weight -C = " << c1
	      << ", power -p = " << p 
	      << ", scale -s = " << s
	      << ", tol -t = " << tol
	      << ", random init -r = " << r 
	      << ", Pyx_factor -F = " << Pyx_factor
	      << ", Px_propto_g -G = " << Px_propto_g
	      << ", feat_file -f = " << feat_file
	      << ", nseparators -n = " << nseparators
	      << ", nshared -S = " << nshared
	      << ", weights_file -o = " << weights_file
	      << ", eval_file -e = " << eval_file
	      << ", eval2_file -x = " << eval2_file
	      << std::endl;

  // Read in eval data first, as that way we may squeeze everything into 4GB
    
  corpusflags_type corpusflags = { Pyx_factor, Px_propto_g };

  corpus_type* evaldata = NULL;
  if (!eval_file.empty()) {
    evaldata = read_corpus_file(&corpusflags, eval_file.c_str());
    if (debug_level >= 10)
      std::cerr << "# read eval_file = " << eval_file 
		<< ", nsentences = " << evaldata->nsentences
		<< std::endl;
  }

  if (errno != 0) {
    perror("## hlm, after reading eval corpus, nonzero errno ");
    errno = 0;
  }

  corpus_type* evaldata2 = NULL;
  if (!eval2_file.empty()) {
    evaldata2 = read_corpus_file(&corpusflags, eval2_file.c_str());
    if (debug_level >= 10)
      std::cerr << "# read eval2_file = " << eval2_file
		<< ", nsentences = " << evaldata2->nsentences
		<< std::endl;
  }

  if (errno != 0) {
    perror("## hlm, after reading eval2 corpus, nonzero errno ");
    errno = 0;
  }

  corpus_type* traindata = read_corpus(&corpusflags, stdin);
  int nx = traindata->nfeatures;

  if (errno != 0) {
    perror("## hlm, after reading main corpus, nonzero errno ");
    errno = 0;
  }

  std::cerr << "# " << nx << " features in training data, " << resource_usage() << std::endl;

  if (evaldata == NULL)
    evaldata = traindata;

  Estimator2 e2(ltype, c0, c1, p, r, s, tol);
  e2.set_data(traindata, evaldata, evaldata2);
  e2.read_featureclasses(feat_file.c_str(), nseparators, ":"); 
  e2.estimate(nshared);
  
  if (!weights_file.empty()) 
    write_weights(weights_file, e2.x);

}  // main()
