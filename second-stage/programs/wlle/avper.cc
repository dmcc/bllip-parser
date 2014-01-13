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

// avper.cc -- An averaged perceptron classifier
//
// Mark Johnson, 24th October 2003, last modified 17th July 2008
//
// Modified to select training examples at random rather
// than in the order they appear in the training data.
//
// Modified to permit multiple runs
//
// Optional weight decay (corresponds to Gaussian regularizer)

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <errno.h>
#include <iostream>
#include <unistd.h>
#include <vector>

#include "lmdata.h"

const char usage[] =
"avper version of 17th July, 2008\n"
"\n"
"Usage: avper [-N nruns] [-b burnin] [-c weightdecay] [-d debug] [-e evalfile] [-F fweight] [-g]\n"
"             [-n nepochs] [-o outfile] [-r reduce] [-s randseed] [-f ignore] [-x ignore] < traindata\n"
"\n"
"where:\n"
"\n"
" -N nruns    - the number of times the perceptron learner is run,\n"
" -b burnin   - the number of epochs before averaging begins,\n"
" -c weightdecay - weight decay rate (i.e., Gaussian regularizer coefficient)\n"
" -d debug    - an integer which controls debugging output,\n"
" -e evalfile - file from which evaluation data is written,\n"
" -F          - weight each parse by its f-score,\n"
" -g          - weight each sentence by size of its gold parse,\n"
" -n nepochs  - the number of training epochs,\n"
" -o outfile  - file to which trained feature weights are written,\n"
" -r reduce   - factor at which the learning rate is decreased each epoch,\n"
" -s randseed - seed for random number generator.\n"
;

int debug_level = 0;

typedef std::vector<Float> Floats;

template <typename T1, typename T2>
void exit_failure(T1 a1, T2 a2) {
  std::cerr << a1 << a2 << std::endl;
  std::cerr << usage << std::endl;
  exit(EXIT_FAILURE);
}  // exit_failure()


void avper(corpus_type *traindata, Float b, Float n, Float r, Float weightdecay, Float w[])
{
  double dw = 1.0;
  double ddw = r == 0 ? 1 : pow(1.0-r, 1.0/traindata->nsentences);
  size_type nfeatures = traindata->nfeatures;  
  Float *sum_w = (Float *) calloc(nfeatures, sizeof(Float));  
  assert(sum_w != NULL);
  size_type *changed = (size_type *) calloc(nfeatures, sizeof(size_type));
  assert(changed != NULL);

  size_type index;
  double rfactor = double(traindata->nsentences)/(RAND_MAX+1.0);

  /* weight decay is per epoch; convert it to per sentence */

  weightdecay /= traindata->nsentences;  

  /* burn-in */

  if (b > 0) {
    for (size_type it = 0; it < b * traindata->nsentences; ++it) {
      index = size_type(rfactor*random());
      assert(index < traindata->nsentences);
      if (traindata->sentence[index].Px > 0)
	ap_sentence(&traindata->sentence[index], w, dw, weightdecay, sum_w, it, changed);
      dw *= ddw;
    }

    for (size_type j = 0; j < nfeatures; ++j) {
      sum_w[j] = 0;
      changed[j] = 0;
    }
  }

  if (debug_level >= 1000)
    std::cerr << "## burnin finished, starting main training" 
	      << ", dw = " << dw << std::endl;

  /* main training */

  size_type it;
  for (it = 0; it < n * traindata->nsentences; ++it) {
    index = size_type(rfactor*random());
    assert(index < traindata->nsentences);
    dw *= ddw;
    if (traindata->sentence[index].Px > 0)
      ap_sentence(&traindata->sentence[index], w, dw, weightdecay, sum_w, it, changed);
  }

  if (debug_level >= 1000)
    std::cerr << "## main training finished, it = " 
	      << it << ", dw = " << dw << std::endl;

  /* final update */

  for (size_type j = 0; j < nfeatures; ++j) {
    sum_w[j] += (it - changed[j]) * w[j];
    w[j] = sum_w[j]/it;
  }

  free(sum_w);
  free(changed);
}  // avper()
 
void print_histogram(int nx, double x[], int nbins=20) {
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


int main(int argc, char **argv)
{
  std::ios::sync_with_stdio(false);

  debug_level = 0;

  Float burnin = 0, nepochs = 1, reduce = 0, weightdecay = 0;
  char *evalfile = NULL, *outfile = NULL;
  Float Pyx_f = 0;
  bool Px_g = 0;
  size_t randseed = 0;
  size_type nruns = 1;

  opterr = 0;
  
  char c, *cp;
  while ((c = getopt(argc, argv, "F:N:f:gb:c:d:n:o:r:e:s:x:")) != -1)
    switch (c) {
    case 'N':
      nruns = strtol(optarg, &cp, 10);
      if (cp == NULL || *cp != '\0' || nruns < 1)
	exit_failure("Expected a positive integer argument for -N, saw ", optarg);
      break;
    case 'b':
      burnin = strtod(optarg, &cp);
      if (cp == NULL || *cp != '\0')
	exit_failure("Expected a float argument for -b, saw ", optarg);
      break;
    case 'c':
      weightdecay = strtod(optarg, &cp);
      if (cp == NULL || *cp != '\0')
	exit_failure("Expected a float argument for -c, saw ", optarg);
      break;
    case 'd':
      debug_level = strtol(optarg, &cp, 10);
      if (cp == NULL || *cp != '\0')
	exit_failure("Expected an integer argument for -d, saw ", optarg);
      break;
    case 'e':
      evalfile = optarg;
      break;
    case 'F':
      Pyx_f = strtod(optarg, &cp);
      if (cp == NULL || *cp != '\0')
	exit_failure("Expected a float argument for -f, saw ", optarg);
      break;
    case 'f':
      break;
    case 'g':
      Px_g = 1;
      break;
    case 'n':
      nepochs = strtod(optarg, &cp);
      if (cp == NULL || *cp != '\0')
	exit_failure("Expected a float argument for -n, saw ", optarg);
      break;
    case 'o':
      outfile = optarg;
      break;
    case 'r':
      reduce = strtod(optarg, &cp);
      if (cp == NULL || *cp != '\0')
	exit_failure("Expected a float argument for -r, saw ", optarg);
      break;
    case 's':
      randseed = strtoul(optarg, &cp, 10);
      if (cp == NULL || *cp != '\0')
	exit_failure("Expected a positive argument for -s, saw ", optarg);
      break;
    case 'x':
      break;
    default:
      exit_failure("Unable to parse command line option ", char(optopt));
      break;
    }

  if (optind != argc)
    exit_failure("Unparsable command line options", "");

  if (debug_level >= 10)
    std::cout << "## nruns = " << nruns
	      << ", burnin = " << burnin 
	      << ", debug_level = " << debug_level 
	      << ", Pyx_f = " << Pyx_f
	      << ", Px_g = " << Px_g
	      << ", nepochs = " << nepochs 
	      << ", reduce = " << reduce 
	      << ", randseed = " << randseed
	      << ", weightdecay = " << weightdecay
	      << std::endl;

  srandom(randseed+1);

  corpusflags_type corpusflags = { Pyx_f, Px_g };

  corpus_type* traindata = read_corpus(&corpusflags, stdin);
  int nx = traindata->nfeatures;

  corpus_type* evaldata = traindata;
  if (evalfile != NULL) {
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
      errno = 0;
      in = popen(command.c_str(), "r");
      if (in == NULL) {
	perror("## Error in cvlm: ");
	std::cerr << "## popen(" << command << "\"r\")" << std::endl;
      }
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
  
  std::cerr << "# nx = " << nx << std::endl;
  std::vector<double> x(nx, 0);

  Float ssum_g = 0, ssum_p = 0, ssum_w = 0;

  for (size_type run = 0; run < nruns; ++run) {
    x.clear();
    x.resize(nx, 0);
    
    avper(traindata, burnin, nepochs, reduce, weightdecay, &x[0]);

    int nzeros = 0;
    for (int i = 0; i < nx; ++i) 
      if (x[i] == 0)
	++nzeros;
  
    std::vector<double> df_dx(nx);
    Float sum_g = 0, sum_p = 0, sum_w = 0;
    Float neglogP = corpus_stats(evaldata, &x[0], &df_dx[0], &sum_g, &sum_p, &sum_w);

    ssum_g += sum_g;
    ssum_p += sum_p;
    ssum_w += sum_w;

    if (run == 0 || debug_level >= 10)
      std::cout << "# run b n  r nzeroweights/nweights neglogP/nsentences precision recall f-score"
		<< std::endl;
    std::cout << run << '\t' << burnin << '\t' << nepochs << '\t' << reduce
	      << '\t' << double(nzeros)/nx 
	      << '\t' << neglogP/evaldata->nsentences
	      << '\t' << sum_w/sum_p 
	      << '\t' << sum_w/sum_g
	      << '\t' << 2*sum_w/(sum_g+sum_p)
	      << std::endl;
  
    if (debug_level >= 10) {
      std::vector<double> df_dx(nx);
      Float sum_g = 0, sum_p = 0, sum_w = 0;
      Float neglogP = corpus_stats(traindata, &x[0], &df_dx[0], &sum_g, &sum_p, &sum_w);

      std::cout << "# Training data neglogP/nsentences precision recall f-score"
	      << std::endl;
      std::cout << "# " << neglogP/evaldata->nsentences
		<< '\t' << sum_w/sum_p 
		<< '\t' << sum_w/sum_g
		<< '\t' << 2*sum_w/(sum_g+sum_p)
	      << std::endl;

      if (debug_level >= 100) {
	std::cout << "# Cumulative distribution of feature weights:" 
		  << std::endl;
	print_histogram(nx, &x[0]);
      }
    }
  }

  std::cout << "\n# Average results:\n# precision recall f-score\n" 
	    << ssum_w/ssum_p << '\t' << ssum_w/ssum_g << '\t' << 2*ssum_w/(ssum_g+ssum_p) 
	    << std::endl;
    
  if (outfile) {
    FILE* out = fopen(outfile, "w");
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
