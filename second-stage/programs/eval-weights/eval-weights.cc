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

// eval-weights.cc
//
// Mark Johnson, 22nd November 2005

const char usage[] =
"eval-weights [-a] [-f identifier-length] features-file.gz feature-counts-file.gz < weights\n"
"\n"
"evaluates the model defined by features-file.gz and weights\n"
"on the data file feature-counts-file.gz.\n"
"\n"
" -a               write out scores for each sentence\n"
" -f nseparators   analyse features grouped into classes based on first nseparators\n"
"\n"
"With the -f flag, eval-weights writes out one line per feature class, with entries\n"
"\n"
"delta-fscore	delta-neglogP	n-nonzero	nfeatures	mean-weight	sd-weight	feature-class\n"
"\n"
"where:\n"
"\n"
" delta-fscore is the change in f-score when these features are zeroed\n"
" delta-neglogP is the change in - log probability when these features are zeroed\n"
" nfeatures is the number of features in the feature class\n"
" mean-weight is the mean feature weight\n"
" sd-weight is the standard deviation of the feature weight\n"
" feature-class is the class of features zeroed.\n";

#include "custom_allocator.h"

#include <cassert>
#include <cctype>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <unistd.h>
#include <vector>

#include "lmdata.h"
#include "utility.h"

typedef std::vector<Float> Floats;
typedef std::map<std::string,size_t> S_C;
typedef std::vector<std::string> Ss;

int debug_level = 0;

template <typename T1, typename T2>
void exit_failure(T1 a1, T2 a2) {
  std::cerr << a1 << a2 << std::endl;
  std::cerr << usage << std::endl;
  exit(EXIT_FAILURE);
}  // exit_failure()


struct FeatureClasses {
  typedef std::vector<size_t> size_ts;
  typedef std::map<std::string,size_t> S_C;
  typedef std::vector<std::string> Ss;
  
  size_ts f_c;		//!< feature -> cross-validation class
  size_t nc;		//!< number of cross-validation classes
  S_C identifier_regclass; //!< map from feature class identifiers to regularization class
  Ss regclass_identifiers; //!< vector of class identifiers


  FeatureClasses(const char* filename, 
		 size_t nseparators = 1,
		 const char* separators = ":") {
    
    const char* filesuffix = strrchr(filename, '.');
    std::string command(strcasecmp(filesuffix, ".bz2")
			? (strcasecmp(filesuffix, ".gz") ? "cat " : "gunzip -c ")
			: "bzcat ");
    command += filename;
    FILE* in = popen(command.c_str(), "r");

    if (in == NULL) {
      std::cerr << "## Error: can't popen command " << command << std::endl;
      exit(EXIT_FAILURE);
    }

    unsigned int featno;
    unsigned int nf = 0;

    // read feature number 

    while (fscanf(in, " %u ", &featno) == 1) {
      ++nf;
      int c;
      
      // read the prefix of the feature class identifier
      
      std::string identifier;
      size_t iseparators = 0;
      while ((c = getc(in)) != EOF && !isspace(c)) {
	if (index(separators, c) != NULL)
	  if (++iseparators > nseparators)
	    break;
	identifier.push_back(c);
      }

      // skip the rest of the line

      if (c != EOF && c != '\n')
	while ((c = getc(in)) != EOF && c != '\n')
	  ;

      // insert the prefix into the prefix -> regularization class map
      
      S_C::iterator it 
	= identifier_regclass.insert(S_C::value_type(identifier, 
						     identifier_regclass.size())).first;
      
      size_t cl = it->second;    // regularization class

      if (featno >= f_c.size())
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
      std::cout << "# Regularization classes: " << regclass_identifiers 
		<< std::endl;

    pclose(in);
  }  // FeatureClasses::FeatureClasses()
    
};  // FeatureClasses{}


void evaluate(corpus_type* eval, const Floats& x, 
	      Float& neglogP, Float& sum_g, Float& sum_p, Float& sum_w,
	      Float& fscore, bool trace_flag = false)
{
  Floats df_dx(x.size(), 0), score(eval->maxnparses, 0);
  sum_g = sum_p = sum_w = 0;
  neglogP = 0;
  unsigned int i;

  for (i = 0; i < eval->nsentences; ++i) {  /* collect stats from sentences */
    Float g0 = 0, p0 = 0, w0 = 0;
    Float neglogP0 = sentence_stats(&eval->sentence[i], &x[0], &score[0], &df_dx[0], 
				    &g0, &p0, &w0);
    if (trace_flag)
      std::cout << i << '\t' << w0 << '\t' << g0 << '\t' << p0 << '\t' 
		<< 2*w0/(g0+p0) << '\t' << neglogP0 << std::endl;

    sum_g += g0;
    sum_p += p0;
    sum_w += w0;
    neglogP += neglogP0;
  }
  fscore = 2*sum_w/(sum_g+sum_p);
}  // evaluate()

int main(int argc, char* argv[])
{
  std::ios::sync_with_stdio(false);
  int nseparators = -1;
  bool trace_flag = false;

  char c, *cp;
  while ((c = getopt(argc, argv, "af:")) != -1) 
    switch (c) {
    case 'a':
      trace_flag = true;
      break;
    case 'f':
      nseparators = strtol(optarg, &cp, 10);
      if (cp == NULL || *cp != '\0')
	exit_failure("Expected a positive integer argument for -f, saw ", optarg);
      break;
    default:
      exit_failure("Unable to parse command line option ", optopt);
      break;
    }

  if (argc - optind != 2)
    exit_failure("Error: missing required feature and devset files", "");

  FeatureClasses fc(argv[optind], nseparators);
  
  Floats xs(fc.f_c.size());
  unsigned int i;
  while (fscanf(stdin, " %u", &i) == 1) {
    Float x = 1;
    fscanf(stdin, " = %lg", &x);
    if (i >= xs.size())
      xs.resize(i+1);
    xs[i] = x;
  }

  if (fc.f_c.size() < xs.size()) {
    std::cerr << "## Error: fc.f_c.size() = " << fc.f_c.size()
	      << ", xs.size()) = " << xs.size() << std::endl;
  }
  assert(fc.f_c.size() == xs.size());

  corpusflags_type cf;
  cf.Pyx_factor = cf.Px_propto_g = 0;
  const char* filesuffix = strrchr(argv[optind+1], '.');
  std::string command(strcasecmp(filesuffix, ".bz2")
		      ? (strcasecmp(filesuffix, ".gz") ? "cat " : "gunzip -c ")
		      : "bzcat ");
  command += argv[optind+1];
  std::cout << "# Evaluating " << argv[optind+1] << std::endl;
  FILE* evalfp = popen(command.c_str(), "r");
  corpus_type* eval = read_corpus(&cf, evalfp);
  pclose(evalfp);
  if (xs.size() < eval->nfeatures)
    std::cerr << "## Error: eval->nfeatures = " << eval->nfeatures 
	      << ", xs.size() = " << xs.size() << std::endl;
  assert(eval->nfeatures <= xs.size());
  Float neglogP_all, fscore_all, sum = 0, sum_sq = 0;
  size_t n_nonzero = 0;
  for (size_t j = 0; j < xs.size(); ++j) 
    if (xs[j] != 0) {
      sum += xs[j];
      sum_sq += xs[j]*xs[j];
      ++n_nonzero;
    }

  if (trace_flag)
    std::cout << "# id\tcorrect\tgold\tparse\tf-score\t-logP" << std::endl;

  Float g_all = 0, p_all = 0, w_all = 0;
  evaluate(eval, xs, neglogP_all, g_all, p_all, w_all, fscore_all, trace_flag);
  std::cout << "# " << xs.size() << " features in " << argv[optind] << std::endl;
  std::cout << "# " << eval->nsentences << " sentences in " << argv[optind+1] << std::endl;
  std::cout << "# ncorrect = " << w_all << ", ngold = " << g_all << ", nparse = " << p_all
	    << ", f-score = " << fscore_all
  	    << ", -log P = " << neglogP_all
	    << ", " << n_nonzero << " nonzero features"
	    << ", mean w = " << sum /xs.size()
	    << ", sd w = " << (sum_sq - sum*sum/xs.size())/(xs.size()-1)
  	    << std::endl;

  if (nseparators >= 0) {
    for (size_t leftout = 0; leftout < fc.nc; ++leftout) {
      Floats xs1(xs);
      size_t nleftout = 0;
      sum = 0;
      sum_sq = 0;
      n_nonzero = 0;
      for (size_t j = 0; j < fc.f_c.size(); ++j)
	if (fc.f_c[j] == leftout) {
	  xs1[j] = 0;
	  ++nleftout;
	  if (xs[j] != 0) {
	    ++n_nonzero;
	    sum += xs[j];
	    sum_sq += xs[j] * xs[j];
	  }
	}
      
      Float neglogP, fscore, g = 0, p = 0, w = 0;
      evaluate(eval, xs1, neglogP, g, p, w, fscore);
      std::cout << fscore-fscore_all
		<< '\t' << neglogP-neglogP_all
		<< '\t' << nleftout
		<< '\t' << n_nonzero
		<< '\t' << sum/nleftout
		<< '\t' << (sum_sq - sum*sum/nleftout)/(nleftout-1)
		<< '\t' << fc.regclass_identifiers[leftout]
		<< std::endl;
    }
  }

} // main()
