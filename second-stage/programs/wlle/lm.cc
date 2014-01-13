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

// lm.cc -- Estimates a linear model using a variety of loss functions
//
// Mark Johnson, 15th Febuary 2005
//
// This is a version of wlle.cc reworked to use a variety of loss functions.
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

const char usage[] =
"lm version of 15th Febuary 2005\n"
"\n"
"lm estimates feature weights that estimate the parameters of a\n"
"linear model by minimizing the regularized loss of the feature\n"
"weights using the LVLM or BLVLM optimizer from the Petsc/Tao\n"
"optimization package (see http://www-fp.mcs.anl.gov/tao/ for details).\n"
"It can deal with partially labeled data (i.e., training instances\n"
"consist of one or more \"winners\" and one or more \"losers\").\n"
"\n"
"Usage: linmod [-help] [-debug debug_level] [-c c] [-d d] [-p p] [-r r] [-s s] [-cv]\n"
"              [-l l] [-Pyx_factor f] [-Px_propto_g] \n"
"              tao-options* [-o weights-file]  [-e eval-file] < train-file\n"
"\n"
"where:\n"
"\n"
" -debug debug_level > 0 controls the amount of output produced\n"
"\n"
" train-file and eval-file are files from which training and evaluation\n"
" data are read (if eval-file ends in the suffix .bz2 then bzcat is used\n"
" to read it; if no eval-file is specified, then the program tests on the\n"
" training data),\n"
"\n"
" weights-file is a file to which the estimated weights are written,\n"
"\n"
" -l l specifies the loss function to be optimized;\n"
"\n"
"    -l 0 - log loss (c ~ 5)\n"
"    -l 1 - EM-style log loss (c ~ 5)\n"
"    -l 2 - pairwise log loss \n"
"    -l 3 - exp loss (c ~ 25, s ~ 1e-5)\n"
"    -l 4 - log exp loss (c ~ 1e-4)\n"
"    -l 5 - maximize expected F-score (c ~ ?)\n"
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
"The function that the program minimizes is:\n"
"\n"
"   Q(w) = s*(-L(w) + c*sum_j pow(fabs(w[j]),p)/(1 + d*pow(fabs(w[j]),p))),\n"
"\n"
"   where:\n"
"\n"
"   L(w) is the basic loss function to be optimized\n"
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

#include <cassert>
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>

#include "custom_allocator.h"    // must come first
#include "lmdata.h"
#include "tao-optimizer.h"
#include "utility.h"

int debug_level = 0;

enum loss_type { log_loss, em_log_loss, pairwise_log_loss, exp_loss, log_exp_loss, 
		 expected_fscore_loss };

void print_histogram(int nx, double x[], int nbins=20) {
  int nx_nonzero = 0;
  for (int i = 0; i < nx; ++i)
    if (x[i] != 0)
      ++nx_nonzero;

  std::cerr << "#   There are " << nx_nonzero << " non-zero values and " 
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
    for (size_t j = 0; j < corpus->nfeatures; ++j)
      df_dx[j] = -df_dx[j];
    break;
  default:
    L = 0;
    std::cerr << "## Error: unrecognized loss_type loss = " << int(ltype) 
	      << std::endl;
  }
  
  if (debug_level >= 1000)
    std::cerr << " f score = " << 2*sum_w/(sum_g+sum_p) << ", " << std::flush;
  
  assert(finite(L));
  return L;
}


// Unconstrained is for unconstrained optimization
//
struct Unconstrained {
  const loss_type ltype;
  corpus_type* corpus;
  size_t nx;
  double c, d, p, s;
  int it;

  Unconstrained(loss_type ltype, corpus_type* corpus, 
		double c, double d, double p, double s) 
    : ltype(ltype), corpus(corpus), nx(corpus->nfeatures), 
      c(c), d(d), p(p), s(s), it(0) { }

  double operator() (int nx, double x[], double df_dx[]) {
    it++;
    if (debug_level >= 1000)      
      std::cerr << "# it = " << it << ", " << std::flush;

    Float L = f_df(ltype, corpus, x, df_dx);
    
    if (s != 1) {
      L *= s;

      for (int i = 0; i < nx; ++i) {
	assert(finite(df_dx[i]));
	df_dx[i] *= s;
      }
    }

    if (c == 0) {
      if (debug_level >= 1000) {
	std::cout << ", L = " << L << std::endl;
	if (debug_level >= 10000) {
	  std::cerr << "# Histogram of derivatives:" << std::endl;
	  print_histogram(nx, &df_dx[0]);
	  std::cerr << "# --------------------------" << std::endl;
	}
      }

      return L;
    }

    double R = 0;
    if (d == 0) 
      for (int i = 0; i < nx; ++i) 
	R += pow(fabs(x[i]), p);
    else  // d != 0
      for (int i = 0; i < nx; ++i) {
	Float xp = pow(fabs(x[i]), p);
	R += xp/(1+d*xp);
      }

    R *= s * c;

    double Q = L + R;
    
    if (debug_level >= 1000)
      std::cerr << "Q = " << Q << " = L = " << L 
		<< " + R = " << R << std::endl;

    assert(finite(Q));
    
    double scp = s * c * p;
    if (d == 0) 
      for (int i = 0; i < nx; ++i) 
	df_dx[i] += scp * pow(fabs(x[i]), p-1) 
	  * (x[i] >= 0 ? (x[i] == 0 ? 0 : 1) : -1);
    else  // d != 0
      for (int i = 0; i < nx; ++i) 
	df_dx[i] += scp * pow(fabs(x[i]), p-1) 
	  * (x[i] >= 0 ? (x[i] == 0 ? 0 : 1) : -1)
	  * pow(1 + d * pow(fabs(x[i]), p), -2);
    
    if (debug_level >= 10000) {
      std::cerr << "# Histogram of derivatives:" << std::endl;
      print_histogram(nx, &df_dx[0]);
      std::cerr << "# --------------------------" << std::endl;
    }

    for (int i = 0; i < nx; ++i)
      assert(finite(df_dx[i]));
    return Q;
  }   // Unconstrained::operator()

};  // Unconstrained{}


// Constrained{} for the constrained optimization.
//
struct Constrained {
  loss_type ltype;
  corpus_type* corpus;
  size_t nx;
  double c, d, p, s;
  int it;
  std::vector<double> x, df_dx;

  Constrained(loss_type ltype, corpus_type* corpus, 
	      double c, double d, double p, double s) 
    : ltype(ltype), corpus(corpus), nx(corpus->nfeatures), 
      c(c), d(d), p(p), s(s), it(0), x(nx), df_dx(nx) { }

  double operator() (int nx2, double x2[], double df_dx2[]) {
    it++;
    if (debug_level >= 1000)      
      std::cerr << "# it = " << it << ", " << std::flush;

    assert(nx2 = 2 * nx);
    assert(x.size() == nx);
    for (size_t i = 0; i < nx; ++i) {
      assert(finite(x2[i]));
      assert(finite(x2[i+nx]));
      x[i] = x2[i] + x2[i+nx];
      assert(finite(x[i]));
    }

    Float L = f_df(ltype, corpus, &x[0], &df_dx[0]);
    
    L *= s;

    for (size_t i = 0; i < nx; ++i) {
      assert(finite(df_dx[i]));
      df_dx2[i] = df_dx2[i+nx] = s * df_dx[i];
    }

    if (c == 0) {
      if (debug_level >= 1000) {
	std::cout << ", L = " << L << std::endl;
	if (debug_level >= 10000) {
	  std::cerr << "# Histogram of derivatives:" << std::endl;
	  print_histogram(nx, &df_dx[0]);
	  std::cout << "--------------------------" << std::endl;
	}
      }
      return L;
    }

    double R = 0;
    if (d == 0) 
    for (int i = 0; i < nx2; ++i) 
      R += pow(fabs(x2[i]), p);
    else // d != 0
      for (int i = 0; i < nx2; ++i) {
	Float xp = pow(fabs(x2[i]), p);
	R += xp/(1+d*xp);
      }

    R *= s * c;

    double Q = L + R;
    
    if (debug_level >= 1000)
      std::cout << "Q = " << Q << " = L = " << L 
		<< " + R = " << R << std::endl;

    assert(finite(Q));
    
    double scp = s * c * p;
    for (size_t i = 0; i < nx; ++i) 
      if (d == 0) {
	df_dx2[i] += scp * pow(fabs(x2[i]), p-1) 
	  * (x2[i] >= 0 ? 1 : -1);
	df_dx2[i+nx] += scp * pow(fabs(x2[i+nx]), p-1) 
	  * (x2[i+nx] > 0 ? 1 : -1);
      }
      else { // d != 0
	df_dx2[i] += scp * pow(fabs(x2[i]), p-1) 
	  * (x2[i] >= 0 ? 1 : -1)
	  * pow(1 + d * pow(fabs(x2[i]), p), -2);
	df_dx2[i+nx] += scp * pow(fabs(x2[i+nx]), p-1) 
	  * (x2[i+nx] > 0 ? 1 : -1)
	  * pow(1 + d * pow(fabs(x2[i+nx]), p), -2);
      }
    
    if (debug_level >= 10000) {
      std::cout << "Histogram of derivatives:" << std::endl;
      print_histogram(nx, &df_dx[0]);
      std::cout << "--------------------------" << std::endl;
    }

    for (int i = 0; i < nx2; ++i)
      assert(finite(df_dx2[i]));
    return Q;
  }   // Constrained::operator()

};  // Constrained{}


std::ostream& data_info(std::ostream& os, corpus_type* data) {
  os << "nsentences = " << data->nsentences
     << ", nfeatures = " << data->nfeatures
     << ", maxnparses = " << data->maxnparses;

  size_t sum_nparses = 0;
  DataFloat sum_g = 0;
  DataFloat sum_Px = 0;

  for (size_t i = 0; i < data->nsentences; ++i) {
    sum_nparses += data->sentence[i].nparses;
    sum_g += data->sentence[i].g;
    sum_Px += data->sentence[i].Px;
  }

  os << ", sum_nparses = " << sum_nparses
     << ", sum_g = " << sum_g
     << ", sum_Px = " << sum_Px
     << ".";

  return os;
} // data_info()

int main(int argc, char** argv) 
{
  std::ios::sync_with_stdio(false);
  
  // Initialize TAO and PETSc

  tao_environment tao_env(argc, argv);

  if (tao_env.get_bool_option("-help")) {
    std::cerr << "-help\n" << usage << std::endl;
    exit(EXIT_SUCCESS);
  }

  debug_level = tao_env.get_int_option("-debug", debug_level);
  loss_type ltype = loss_type(tao_env.get_int_option("-l", 0));
  double c = tao_env.get_double_option("-c", 0.0);
  double d = tao_env.get_double_option("-d", 0.0);
  double p = tao_env.get_double_option("-p", 2.0);
  double r = tao_env.get_double_option("-r", 0.0);
  double s = tao_env.get_double_option("-s", 1.0);
  bool cv = tao_env.get_bool_option("-cv");
  double Pyx_factor = tao_env.get_double_option("-Pyx_factor", 0.0);
  bool Px_propto_g = tao_env.get_bool_option("-Px_propto_g");

  if (debug_level >= 10)
    std::cerr << "# ltype = " << ltype
	      << ", regularization c = " << c 
	      << ", d = " << d
	      << ", power p = " << p 
	      << ", scale s = " << s
	      << "; random init r = " << r 
	      << ", constrained var optimization cv = " << cv
	      << ", Pyx_factor = " << Pyx_factor
	      << ", Px_propto_g = " << Px_propto_g
	      << std::endl;

  corpusflags_type corpusflags = { Pyx_factor, Px_propto_g };
  corpus_type* traindata = read_corpus(&corpusflags, stdin);
  int nx = traindata->nfeatures;

  // Read in eval data
  
  corpus_type* evaldata = traindata;
  const char* evalfile = tao_env.get_cstr_option("-e");
  if (evalfile) {
    const char* filesuffix = strrchr(evalfile, '.');
    bool popen_flag = false;
    FILE *in;
    if (strcasecmp(filesuffix, ".bz2") == 0) {
      std::string command("bzcat ");
      command += evalfile;
      in = popen(command.c_str(), "r");
      popen_flag = true;
    }
    else if (strcasecmp(filesuffix, ".gz") == 0) {
      std::string command("gunzip -c ");
      command += evalfile;
      in = popen(command.c_str(), "r");
      popen_flag = true;
    }
    else
      in = fopen(evalfile, "r");
    if (in == NULL) {
      std::cerr << "## Couldn't open evalfile " << evalfile
		<< ", errno = " << errno << "\n" 
		<< usage << std::endl;
      exit(EXIT_FAILURE);
    }
    evaldata = read_corpus(&corpusflags, in);
    if (popen_flag)
      pclose(in);
    else
      fclose(in);
    int nxe = evaldata->nfeatures;
    assert(nxe <= nx);
  }

  if (debug_level >= 100) {
    std::cerr << "# traindata: ";
    data_info(std::cerr, traindata);
    std::cerr << std::endl;
    std::cerr << "# evaldata: ";
    data_info(std::cerr, evaldata);
    std::cerr << std::endl;
    std::cerr << "# resource usage: " << resource_usage() << std::endl;
  }

  std::vector<double> x;
  int nits = 0;

  if (cv) {

    // Constrained variable optimization

    Constrained fn(ltype, traindata, c, d, p, s);
    tao_constrained_optimizer<Constrained> tao_opt(2*nx, fn);

    if (r != 0) 
      for (int i = 0; i < 2*nx; ++i)
	tao_opt[i] = r*double(random()-RAND_MAX/2)/double(RAND_MAX/2);
    
    for (int i = 0; i < nx; ++i) {
      tao_opt.lower_bound[i] = 0;
      tao_opt.lower_bound[i+nx] = TAO_NINFINITY;
      tao_opt.upper_bound[i] = TAO_INFINITY;
      tao_opt.upper_bound[i+nx] = 0;
    }

    tao_opt.optimize();
    
    nits = fn.it;
    x.resize(nx);

    for (int i = 0; i < nx; ++i)
      x[i] = tao_opt[i] + tao_opt[i+nx];
  }
  else {

    // Unconstrained optimization

    Unconstrained fn(ltype, traindata, c, d, p, s);
    tao_optimizer<Unconstrained> tao_opt(nx, fn);

    if (r != 0) 
      for (int i = 0; i < nx; ++i)
	tao_opt[i] = r*double(random()-RAND_MAX/2)/double(RAND_MAX/2);

    tao_opt.optimize();

    nits = fn.it;
    x.resize(nx);

    for (int i = 0; i < nx; ++i)
      x[i] = tao_opt[i];
  }
   
  int nzeros = 0;
  for (int i = 0; i < nx; ++i) 
    if (x[i] == 0)
      ++nzeros;
  
  std::vector<double> df_dx(nx);
  Float sum_g = 0, sum_p = 0, sum_w = 0;
  Float neglogP = corpus_stats(evaldata, &x[0], &df_dx[0], &sum_g, &sum_p, &sum_w);
  if (debug_level > 0)
    std::cerr << "# ltype c p r s it nzeroweights/nweights neglogP/nsentences precision recall f-score" << std::endl;
  std::cout << ltype << '\t' << c << '\t' << p << '\t' << r << '\t' << s 
	    << '\t' << nits << '\t' << double(nzeros)/nx 
	    << '\t' << neglogP/evaldata->nsentences
	    << '\t' << sum_w/sum_p 
	    << '\t' << sum_w/sum_g
	    << '\t' << 2*sum_w/(sum_g+sum_p)
	    << std::endl;
  
  if (debug_level >= 10) {
    sum_g = 0; sum_p = 0; sum_w = 0;
    neglogP = corpus_stats(traindata, &x[0], &df_dx[0], &sum_g, &sum_p, &sum_w);
    std::cerr << "# Training -log likelihood/nsentences = " 
	      << neglogP/traindata->nsentences 
	      << ", f score = " << 2*sum_w/(sum_g+sum_p)
	      << std::endl;
    std::cerr << "# Resource usage: " << resource_usage() << std::endl;
  }

  if (debug_level >= 100) {
    std::cerr << "# Cumulative distribution of feature weights:" << std::endl;
    print_histogram(nx, &x[0]);
  }

  const char* filename = tao_env.get_cstr_option("-o");

  if (filename != NULL) {
    FILE* out = fopen(filename, "w");
    // fprintf(out, "%d@", nx-nzeros);
    for (int i = 0; i < nx; ++i) 
      if (x[i] != 0) {
	fprintf(out, "%d", i);
	if (x[i] != 1)
	  fprintf(out, "=%g", x[i]);
	fprintf(out, "\n");
      }
    fclose(out);
  }
}  // main()
