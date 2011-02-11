// compare-models.cc
//
// Mark Johnson, 10th May 2005

const char usage[] =
  "Usage: compare-models weights1 features1 [weights2 [features2]]\n"
  "\n"
  " compares the performance of the model weights1 with\n"
  " the model weights2.  features1 should be the feature vectors\n"
  " corresponding to weights1.  If weights2 is not given, the second\n"
  " model is taken to be one that puts all its weight on the first\n"
  " feature.  If features2 is not given, then features2 is assumed\n"
  " to be the same as features1.\n";

#include "custom_allocator.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <utility>
#include <vector>

#include "cephes.h"
#include "lmdata.h"
#include "popen.h"
#include "utility.h"

typedef std::vector<Float> Floats;
typedef std::pair<Float,Float> FF;

//! pr_type{} calculates standard parseval precision and recall scores.
//
struct pr_type {
  size_type ncommon;     // number of edges in common
  size_type ngold;       // number of edges in gold trees
  size_type ntest;       // number of edges in test trees

  pr_type(size_type ncommon = 0, size_type ngold = 0, size_type ntest = 0) 
    : ncommon(ncommon), ngold(ngold), ntest(ntest) { }

  void clear() { ncommon = ngold = ntest = 0; }

  Float precision() const { 
    return (ntest == 0) ? 0 : Float(ncommon)/ntest;
  }

  Float recall() const {
    return (ngold == 0) ? 1 : Float(ncommon)/ngold;
  }

  Float f_score() const {
    return (ntest == 0 && ngold == 0) ? 0 : (2.0 * ncommon) / (ntest+ngold);
  } // pr_type::f_score()

  Float error_rate() const {
    return (ngold + ntest - 2.0*ncommon) / ngold;
  } // pr_type::error_rate()

  pr_type& operator+= (const pr_type& pr) {
    ncommon += pr.ncommon;
    ngold += pr.ngold;
    ntest += pr.ntest;
    return *this;    
  }  // pr_type::operator+=()

};  // pr_type{}


typedef std::vector<pr_type> prs_type;

void read_weights_file(const char* filename, Floats& ws) {
  izstream is(filename);
  if (!is) {
    std::cerr << "## Error in compare-models: unable to read weights file "
	      << filename << "\n\n" << usage << std::endl;
    exit(EXIT_FAILURE);
  }
  size_type f;
  Float w;
  while (is >> f >> " =" >> w) {
    assert(f >= 0);
    assert(f < ws.size());
    ws[f] = w;
  }
} // read_weights_file()

void corpus_prs(const corpus_type* corpus, const Floats& ws, 
		prs_type& prs, pr_type& prsum) {
  size_type nsentences = corpus->nsentences;
  prs.resize(corpus->nsentences);
  prsum.clear();
  
  for (size_type i = 0; i < nsentences; ++i) {
    const sentence_type* s = &corpus->sentence[i];
    prs[i].ngold = lround(s->g);
    if (s->nparses > 0) {
      size_type j = max_score_index(s, &ws[0]);
      const parse_type* p = &s->parse[j];
      prs[i].ntest = lround(p->p);
      prs[i].ncommon = lround(p->w);
    }
    prsum += prs[i];
  }
} // corpus_prs()

// fscore_difference sets n1better, n2better and n12tied based on the
// fscores in prs1 and prs2
//
void fscore_difference(const prs_type& prs1, const prs_type& prs2,
		       size_type& n1better, size_type& n2better,
		       size_type& n12tied) {
  assert(prs1.size() == prs2.size());
  n1better = 0;
  n2better = 0;
  n12tied = 0;
  for (size_type i = 0; i < prs1.size(); ++i) {
    Float f1 = prs1[i].f_score();
    Float f2 = prs2[i].f_score();
    if (f1 > f2)
      ++n1better;
    else if (f2 > f1)
      ++n2better;
    else
      ++n12tied;
  }
} // fscore_difference()

//! binomial_significance returns the probability of obtaining k or fewer
//! successes in n trials
//
Float binomial_significance(size_type n, size_type k) {
  assert(n >= 1);
  assert(n >= k);
  if (k <= n/2)
    k = n - k;  // swap if k is less than half n
  return 2*incbet(k, n-k+1, 0.5);
}


//! permutation_test() estimates the significance of the difference
//! in corpus f-scores between prs1 and prs2.
//
Float permutation_test(const prs_type& prs1, const prs_type& prs2, 
		       const size_type ns = 100000) {
  assert(prs1.size() == prs2.size());
  size_type m = prs1.size();
  pr_type prsum1, prsum2;
  for (size_type j = 0; j < m; ++j) {
    prsum1 += prs1[j];
    prsum2 += prs2[j];
  }
  Float emp_abs_delta_fscore = fabs(prsum1.f_score()-prsum2.f_score());

  size_type ngreater = 0;
  for (size_type i = 0; i < ns; ++i) {
    prsum1.clear();
    prsum2.clear();
    
    for (size_type j = 0; j < m; ++j)
      if (rand() < RAND_MAX/2) {  // randomly permute sentence scores
	prsum1 += prs1[j];
	prsum2 += prs2[j];
      }
      else {
	prsum1 += prs2[j];
	prsum2 += prs1[j];
      }
    
    Float sim_abs_delta_fscore = fabs(prsum1.f_score()-prsum2.f_score());
    if (sim_abs_delta_fscore >= emp_abs_delta_fscore)
      ++ngreater;
  }
  return Float(ngreater)/Float(ns);
}  // permutation_test()


//! bootstrap_interval() provides a bootstrap resampling estimate of 
//!  a confidence interval of the f-score.
//

FF boostrap_interval(const prs_type& prs, 
		     Float lower = 0.025, Float upper = 0.975,
		     const size_type ns = 100000) {

  Floats fscores(ns);

  for (size_type is = 0; is < ns; ++is) {
    int rfactor = RAND_MAX/prs.size();
    pr_type pr_sum;
    for (size_type i = 0; i < prs.size(); ++i) {
      size_type j;
      do j = rand()/rfactor;
      while (j >= prs.size());
      pr_sum += prs[j];
    }
    fscores[is] = pr_sum.f_score();
  }
  size_type lower_index = lrint(lower * ns);
  assert(lower_index < ns);
  std::nth_element(fscores.begin(), fscores.begin()+lower_index, fscores.end());
  Float lowerbound = fscores[lower_index];
  size_type upper_index = lrint(upper * ns);
  assert(upper_index < ns);
  std::nth_element(fscores.begin(), fscores.begin()+upper_index, fscores.end());
  Float upperbound = fscores[upper_index];
  return FF(lowerbound,upperbound);
}  // bootstrap_interval()


int main(int argc, char* argv[]) {

  if (argc < 3 || argc > 5) {
    std::cerr << usage << std::endl;
    exit(EXIT_FAILURE);
  }

  corpus_type* corpus1 = read_corpus_file(NULL, argv[2]);
  if (corpus1 == NULL) {
    std::cerr << "## Error in compare-models: unable to read corpus file "
	      << argv[2]
	      << "\n\n" << usage << std::endl;
    exit(EXIT_FAILURE);
  }

  size_type nf1 = corpus1->nfeatures;
  Floats ws1(nf1), ws2(nf1);
  read_weights_file(argv[1], ws1);

  corpus_type* corpus2 = corpus1;
  size_type nf2 = nf1;

  if (argc >= 4) {
    
    if (argc >= 5) {
      corpus2 = read_corpus_file(NULL, argv[4]);
      if (corpus2 == NULL) {
	std::cerr << "## Error in compare-models: unable to read corpus file "
	      << argv[4]
		  << "\n\n" << usage << std::endl;
	exit(EXIT_FAILURE);
      }
      assert(corpus1->nsentences == corpus2->nsentences);
      assert(corpus1->maxnparses == corpus2->maxnparses);
    }
    
    nf2 = corpus2->nfeatures;
    ws2.resize(nf2);
    read_weights_file(argv[3], ws2);
  }
  else {
    ws2[0] = ws1[0];  // if no weights file given, put all weight onto feature 0
    if (ws2[0] == 0)
      ws2[0] = -1;
  }
  
  pr_type pr1, pr2;
  prs_type prs1, prs2;
  corpus_prs(corpus1, ws1, prs1, pr1);
  corpus_prs(corpus2, ws2, prs2, pr2);

  size_type nsentences = corpus1->nsentences;
  std::cout << "nsentences = " << nsentences << " in test corpus." << std::endl;

  std::cout << "model 1 nfeatures = " << ws1.size() << ", corpus f-score = " << pr1.f_score() << std::endl;
  std::cout << "model 2 nfeatures = " << ws2.size() << ", corpus f-score = " << pr2.f_score() << std::endl;
  std::cout << "permutation test significance of corpus f-score difference = " 
	    << permutation_test(prs1, prs2) << std::endl;

  size_type n1better, n2better, n12tied;
  fscore_difference(prs1, prs2, n1better, n2better, n12tied);

  std::cout << "model 1 better on " << n1better << " = " << (100.0*n1better)/nsentences << "% sentences" << std::endl;
  std::cout << "model 2 better on " << n2better << " = " << (100.0*n2better)/nsentences << "% sentences" << std::endl;
  std::cout << "models 1 and 2 tied on " << n12tied << " = " << (100*n12tied)/nsentences << "% sentences" << std::endl;
  std::cout << "binomial 2-sided significance of sentence-by-sentence comparison = " 
	    << binomial_significance(n1better+n2better, n2better) << std::endl;

  std::cout << "bootstrap 95% confidence interval for model 1 f-scores = " << boostrap_interval(prs1) << std::endl;
  std::cout << "bootstrap 95% confidence interval for model 2 f-scores = " << boostrap_interval(prs2) << std::endl;

}  // main()

