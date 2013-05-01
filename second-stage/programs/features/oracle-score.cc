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

// oracle-score.cc
//
// Mark Johnson, 19th June 2004, rewritten 12th November 2005

static const char usage[] = 
"Usage: oracle-score [-a] parses-cmd gold-cmd\n"
"\n"
" -a : print results for each sentence.\n";

#include <cmath>
#include <iostream>
#include <getopt.h>

#include "dp-data.h"
#include "tree.h"

struct visitor {

  size_t nsentences;
  size_t nparsed;
  size_t n_exact_match;
  bool trace;
  precrec_type pr, pr0;
  double sum_log2_condp;
  double sum_log2_condp0;

  visitor() 
    : nsentences(0), nparsed(0), n_exact_match(0), trace(false), 
      pr(), pr0(), sum_log2_condp(0), sum_log2_condp0(0) { }

  void operator() (const sentence_type& s) {
    ++nsentences;
    pr0.ngold += s.gold_nedges;
    pr0.ntest += s.parses[0].nedges;
    pr0.ncommon += s.parses[0].ncorrect;
    
    double sum_p = 0;
    pr.ngold += s.gold_nedges;
    if (!s.parses.empty()) {
      ++nparsed;
      size_t best_index = 0;
      float best_f_score = s.parses[0].f_score;
      for (size_t i = 0; i < s.parses.size(); ++i) {
	sum_p += pow(2.0, s.parses[i].logprob - s.parses[0].logprob);
	if (s.parses[i].f_score > best_f_score) {
	  best_f_score = s.parses[i].f_score;
	  best_index = i;
	}
      }
      double log2_condp0 = -log2(sum_p);
      sum_log2_condp0 += log2_condp0;
      double log2_condp = s.parses[best_index].logprob - (log2(sum_p) + s.parses[0].logprob);
      sum_log2_condp += log2_condp;
      pr.ntest += s.parses[best_index].nedges;
      pr.ncommon += s.parses[best_index].ncorrect;
      if (s.parses[best_index].nedges == s.parses[best_index].ncorrect)
	++n_exact_match;
      if (trace)
	std::cout << " " << nsentences 
		  << "\t" << s.gold_nedges 
		  << "\t" << s.parses[best_index].nedges
		  << "\t" << s.parses[best_index].ncorrect
		  << "\t" << best_f_score
		  << "\t" << log2_condp
		  << "\n";
    }
  }  // visitor::operator()

}; // visitor{}


int main(int argc, char **argv) {

  std::ios::sync_with_stdio(false);

  visitor v;

  int c;
  while ((c = getopt(argc, argv, "a")) != -1 )
    switch (c) {
    case 'a': 
      v.trace = true;
      break;
    default:
      std::cerr << usage << std::endl;
      exit(EXIT_FAILURE);
      break;
    }

  if (argc != optind+2) {
    std::cerr << usage << std::endl;
    exit(EXIT_FAILURE);
  }

  if (v.trace)
    std::cout << " sent no\tGold\tBest\tCorrect\tf-score\tlog2 CP\n";

  corpus_type::map_sentences_cmd(argv[optind], argv[optind+1], v);

  std::cout << "The corpus contains " << v.nsentences << " sentences, of which "
	    << v.nparsed << " were parsed.\n"
	    << "First parse " << v.pr0 << ", average log2 CP = " << v.sum_log2_condp0/v.nsentences << "." << std::endl
	    << "Oracle " << v.pr << ", average log2 CP = " << v.sum_log2_condp/v.nsentences << "." << std::endl
	    << v.n_exact_match << "/" << v.nparsed << " = " 
	    << double(v.n_exact_match)/double(v.nparsed) 
	    << " sentences had an exact match parse." << std::endl;
  
} // main()
