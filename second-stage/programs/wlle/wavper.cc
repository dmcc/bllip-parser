// wavper.cc -- A weighted averaged perceptron classifier
//
// Mark Johnson, 26th Sept 2003, last modified 9th April 2005

#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <cstdio>
#include <map>
#include <iostream>
#include <unistd.h>
#include <vector>

#include "lmdata.h"
// #include "powell.h"
#include "amoeba.h"
#include "utility.h"

const char usage[] =
"wavper version of 26th September, 2003\n"
"\n"
"Usage: wavper [-b burnin] [-d debug] [-f] [-g] [-n nepochs] [-o outfile]\n"
"    [-c c0] [-h feat.bz2] [-e evalfile] [-x evalfile2] [-r reduce] [-s randseed] < traindata\n"
"\n"
"where:\n"
"\n"
" -b burnin   - the number of epochs before averaging begins,\n"
" -c c0       - initial feature class factors,\n"
" -d debug    - an integer which controls debugging output,\n"
" -e evalfile - file from which evaluation data is written,\n"
" -f          - weight each parse by its f-score,\n"
" -g          - weight each sentence by size of its gold parse,\n"
" -h feat.bz2 - features used (used to adjust feature class weights),\n"
" -n nepochs  - the number of training epochs, and\n"
" -o outfile  - file to which trained feature weights are written, and\n"
" -r reduce   - factor at which the learning rate is decreased each epoch.\n"
" -s randseed - random number seed.\n"
" -x evalfile2 - 2nd evaluation file\n";

int debug_level = 0;

typedef std::vector<Float> Floats;

template <typename T1, typename T2>
void exit_failure(T1 a1, T2 a2) {
  std::cerr << a1 << a2 << std::endl;
  std::cerr << usage << std::endl;
  exit(EXIT_FAILURE);
}  // exit_failure()


 
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


// The Estimator1 does one round of estimation
//
struct Estimator1 {
  typedef std::vector<size_type> size_types;
  typedef std::vector<double> doubles;
  
  corpus_type* train;	//!< training data
  size_type nx;		//!< number of features
  corpus_type* eval;	//!< evaluation data
  corpus_type* eval2;	//!< 2nd evaluation data
  double c0;		//!< default regularizer factor
  Float burnin; 
  Float nepochs; 
  Float reduce;

  Float best_fscore;    //!< best f-score seen so far

  doubles x;		//!< feature -> weight
  size_types f_c;	//!< feature -> cross-validation class

  doubles lcs;		//!< cross-validation class -> log factor
  size_type nc;		//!< number of cross-validation classes
  size_type nrounds;	//!< number of cross-validation rounds so far
  std::string weightsfile; //!< name of weights file

  typedef std::map<std::string,size_type> S_C;
  //! map from feature class identifiers to regularization class
  S_C identifier_regclass; 
  typedef std::vector<std::string> Ss;
  Ss regclass_identifiers; //!< vector of class identifiers

  Estimator1(corpus_type* train, corpus_type* eval, corpus_type* eval2,
	     double c0, Float burnin, Float nepochs, Float reduce,
	     const char* weightsfile = NULL) 
    : train(train), nx(train->nfeatures), eval(eval), eval2(eval2),
      c0(c0), burnin(burnin), nepochs(nepochs), reduce(reduce),
      best_fscore(0),
      x(nx), f_c(nx), lcs(1, log(c0)), nc(1), 
      nrounds(0), 
      weightsfile(weightsfile == NULL ? "" : weightsfile)
  { }  // Estimator1::Estimator1()

  // operator() actually runs one round of estimation
  //
  double operator() (const doubles& lccs) {
    assert(lccs.size() == nc);
    doubles ccs(nc);

    for (size_type i = 0; i < nc; ++i)
      ccs[i] = exp(lccs[i]);

    assert(x.size() == nx);
    nrounds++;

    if (debug_level >= 10) {
      if (nrounds == 1) 
	std::cout << "# round f-score(s) ccs" << std::endl;
      std::cout << nrounds << std::flush;
    }

    for (size_type i = 0; i < x.size(); ++i)
      x[i] = 0;
    
    avper(burnin, nepochs, reduce, &x[0], &f_c[0], &ccs[0]);

    Float fscore = evaluate(true);

    if (debug_level >= 10) 
      std::cout << '\t' << ccs << std::endl;
    
    if (fscore > best_fscore) {
      best_fscore = fscore;
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
    return 1 - fscore;
  }  // Estimator1::operator()


  //! avper() runs 1 iteration of the averaged perceptron
  //
  void avper(Float b,                     //!< burn-in
	     Float n,                     //!< number of training epochs
	     Float r,                     //!< discount factor per epoch
	     Float w[],                   //!< weight vector
	     size_type feat_class[],      //!< feature -> class
	     const double class_factor[]  //!< class -> class weight factor
	     )
  {
    double dw = 1.0;
    double ddw = r == 0 ? 1 : pow(1.0-r, 1.0/train->nsentences);
    size_type nfeatures = train->nfeatures;  
    Float *sum_w = (Float *) calloc(nfeatures, sizeof(Float));  
    assert(sum_w != NULL);
    size_type *changed = (size_type *) calloc(nfeatures, sizeof(size_type));
    assert(changed != NULL);
    
    size_type index;
    double rfactor = double(train->nsentences)/(RAND_MAX+1.0);

    /* burn-in */

    if (b > 0) {
      for (size_type it = 0; it < b * train->nsentences; ++it) {
	index = size_t(rfactor*random());
	assert(index < train->nsentences);
	if (train->sentence[index].Px > 0)
	  wap_sentence(&train->sentence[index], w, dw, feat_class, class_factor,
		       sum_w, it, changed);
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
    for (it = 0; it < n * train->nsentences; ++it) {
      index = size_t(rfactor*random());
      assert(index < train->nsentences);
      dw *= ddw;
      if (train->sentence[index].Px > 0)
	wap_sentence(&train->sentence[index], w, dw, feat_class, class_factor, 
		     sum_w, it, changed);
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
  }  // Evaluate1::avper()

  // evaluate() evaluates the current model on the eval data, prints
  // out debugging information if appropriate, and returns the f-score.
  //
  double evaluate(bool internal = false) const {

    std::vector<double> df_dx(nx);
    Float sum_g = 0, sum_p = 0, sum_w = 0;
    corpus_stats(eval, &x[0], &df_dx[0], &sum_g, &sum_p, &sum_w);
    Float fscore = 2*sum_w/(sum_g+sum_p);

    Float sum_g2 = 0, sum_p2 = 0, sum_w2 = 0;
    Float fscore2 = 0;
    if (eval2 != NULL) {
      corpus_stats(eval2, &x[0], &df_dx[0], &sum_g2, &sum_p2, &sum_w2);
      fscore2 = 2*sum_w2/(sum_g2+sum_p2);
    }
    
    if (internal) {  // internal evaluation, use a short print-out

      if (debug_level >= 10) {
	std::cout << ' ' << fscore;
	if (eval2 != NULL)
	  std::cout << ' ' << fscore2;
	// std::cout << std::endl;
      }
    }
    else { // final evaluation, print out more info
    
      int nzeros = 0;
      for (size_type i = 0; i < nx; ++i) 
	if (x[i] == 0)
	  ++nzeros;
  
      std::cout << "# " << nzeros << " zero feature weights of " 
		<< nx << " features." << std::endl;
      std::cout << "# Eval precision = " << sum_w/sum_p 
		<< ", recall = " << sum_w/sum_g
		<< ", f-score = " << 2*sum_w/(sum_g+sum_p)
		<< std::endl;
      if (eval2 != NULL) 
	std::cout << "# Eval2 precision = " << sum_w2/sum_p2 
		  << ", recall = " << sum_w2/sum_g2
		  << ", f-score = " << 2*sum_w2/(sum_g2+sum_p2)
		  << std::endl;
      {
	Float sum_g = 0, sum_p = 0, sum_w = 0;
	corpus_stats(train, &x[0], &df_dx[0], 
		     &sum_g, &sum_p, &sum_w);
	std::cout << "# Train precision = " << sum_w/sum_p 
		  << ", recall = " << sum_w/sum_g
		  << ", f-score = " << 2*sum_w/(sum_g+sum_p)
		  << std::endl;
      }

      std::cout << "# regclass_identifiers = " << regclass_identifiers << std::endl;
      std::cout << "# lcs = " << lcs << std::endl;
      {
	doubles cs(nc);
	for (size_type i = 0; i < nc; ++i)
	  cs[i] = exp(lcs[i]);
	std::cerr << "# cs = " << cs << std::endl;
      }      

      if (debug_level >= 100) {
	std::cout << "# Cumulative distribution of feature weights:" << std::endl;
	print_histogram(nx, &x[0]);
      }
    }

    return fscore;
  } // Estimator1::evaluate()

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
	perror("## Error in cvlm: ");
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
	perror("## Error in cvlm: ");
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
    size_type nf = 0;

    // read feature number 

    while (fscanf(in, " %u ", &featno) == 1) {
      if (featno >= nx) {
	std::cerr << "## Error: read featno = " << featno << ", nx = " << nx << std::endl;
	exit(EXIT_FAILURE);
      }
      ++nf;
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

      assert(featno < f_c.size());
      f_c[featno] = cl;          // set feature's regularization class
    }
      
    if (nx != nf) {
      std::cerr << "## Error: only read " << nf << " features from " 
		<< filename << ", but data contains " << nx << " features."
		<< std::endl;
      exit(EXIT_FAILURE);
    }

    nc = identifier_regclass.size();   // set nc
    lcs.resize(nc, log(c0));           // set each regularizer class' factor to c0
    if (nc >= 1)
      lcs[0] += 0;                     // boost LogProb class

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
    // powell::control cntrl(1e-4, 1e-2, 0, 10);
    // powell::minimize(lcs, *this, 1.0, cntrl);
    amoeba(*this, lcs, -1);
    
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



int main(int argc, char **argv)
{
  std::ios::sync_with_stdio(false);

  debug_level = 0;

  Float c0 = 1, burnin = 0, nepochs = 1, reduce = 0;
  char *evalfile = NULL, *evalfile2 = NULL, *featfile = NULL, *outfile = NULL;
  Float Pyx_f = 0;
  bool Px_g = 0;
  size_t randseed = 0;

  opterr = 0;
  
  char c, *cp;
  while ((c = getopt(argc, argv, "b:c:d:e:f:gh:n:o:r:s:x:")) != -1)
    switch (c) {
    case 'b':
      burnin = strtod(optarg, &cp);
      if (cp == NULL || *cp != '\0')
	exit_failure("Expected a float argument for -b, saw ", optarg);
      break;
    case 'c':
      c0 = strtod(optarg, &cp);
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
    case 'f':
      Pyx_f = strtod(optarg, &cp);
      if (cp == NULL || *cp != '\0')
	exit_failure("Expected a float argument for -f, saw ", optarg);
      break;
     case 'g':
      Px_g = 1;
      break;
    case 'h':
      featfile = optarg;
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
      srandom(randseed);  // reset the random seed
      break;
    case 'x':
      evalfile2 = optarg;
      break;
    default:
      exit_failure("Unable to parse command line option ", optopt);
      break;
    }

  if (optind != argc)
    exit_failure("Unparsable command line options", "");

  if (debug_level >= 10)
    std::cout << "## burnin = " << burnin 
	      << ", debug_level = " << debug_level 
	      << ", Pyx_f = " << Pyx_f
	      << ", Px_g = " << Px_g
	      << ", nepochs = " << nepochs 
	      << ", reduce = " << reduce 
	      << ", randseed = " << randseed
	      << ", featfile = " << featfile
	      << std::endl;

  corpusflags_type corpusflags = { Pyx_f, Px_g };

  corpus_type* traindata = read_corpus(&corpusflags, stdin);
  int nx = traindata->nfeatures;

  corpus_type* evaldata = traindata;
  if (evalfile != NULL) {
    evaldata = read_corpus_file(&corpusflags, evalfile);
    int nxe = evaldata->nfeatures;
    assert(nxe <= nx);
  }

  corpus_type* evaldata2 = NULL;
  if (evalfile2 != NULL) {
    evaldata2 = read_corpus_file(&corpusflags, evalfile2);
    int nxe = evaldata->nfeatures;
    assert(nxe <= nx);
  }

  Estimator1 e(traindata, evaldata, evaldata2, c0, burnin, nepochs, reduce,
	       outfile);

  if (featfile != NULL)
    e.read_featureclasses(featfile, 100, ":");   // number of separators

  e.estimate();

}  // main()
