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

// oracle.cc
//
// Mark Johnson, 12th October 2003, updated 20th November 2010

#include <cassert>
#include <iostream>
#include <vector>

#include "lmdata.h"


int main(int argc, char* argv[])
{
  assert(argc == 2);
  corpusflags_type cflags = { 0.0, 0 };
  corpus_type* corpus = read_corpus_file(&cflags, argv[1]);

  std::cout << "There are " << corpus->nsentences << " sentences," << std::flush;

  // Evaluate the oracle rate

  double np = 0, ng = 0, nw = 0;
  size_t nparsed = 0, sum_nparses = 0;;
  for (size_t i = 0; i < corpus->nsentences; ++i) {
    sentence_type* sentence = &corpus->sentence[i];
    ng += sentence->g;
    sum_nparses += sentence->nparses;
    for (size_t j = 0; j < sentence->nparses; ++j) {
      parse_type* parse = &sentence->parse[j];
      if (parse->Pyx == 1) {
	++nparsed;
	np += parse->p;
	nw += parse->w;
      }
    }
  }

  std::cout << " of which " << nparsed << " have parses." << std::endl;
  std::cout << "On average each parsed sentence has " << sum_nparses 
	    << "/" << nparsed << " = " << double(sum_nparses)/double(nparsed)
	    << " parses." << std::endl;
  std::cout << "Oracle precision = " << nw/np << ", recall = " << nw/ng 
	    << ", f-score = " << 2*nw/(np+ng) << std::endl;

  std::vector<Float> x(corpus->nfeatures), df_dx(corpus->nfeatures);
  Float sum_g = 0, sum_p = 0, sum_w = 0;
  Float LL = corpus_stats(corpus, &x[0], &df_dx[0], &sum_g, &sum_p, &sum_w);
  std::cout << "Uniform precision = " << sum_w/sum_p << ", recall = " << sum_w/sum_g
	    << ", f-score = " << 2*sum_w/(sum_p+sum_g) 
	    << ", -log P = " << LL << std::endl;

  sum_g = sum_p = sum_w = 0;
  x[0] = 1;
  LL = corpus_stats(corpus, &x[0], &df_dx[0], &sum_g, &sum_p, &sum_w);
  std::cout << "LogProb feature precision = " << sum_w/sum_p << ", recall = " << sum_w/sum_g
	    << ", f-score = " << 2*sum_w/(sum_p+sum_g) 
	    << ", -log P = " << LL << std::endl;

} // main()
