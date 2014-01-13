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

// best-indices.cc
//
// Mark Johnson, 17th March 2004

const char usage[] =
"best-indices feature-count-file.bz2 < feature-weights\n"
"\n"
"writes out one line per sentence of the form\n"
"\n"
"\tmax-weight-index max-weight-fscore best-index best-fscore nparses\n"
"\n"
"where:\n\n"
" max-weight-index is an index of a parse with highest weight for this sentence,\n"
" max-weight-fscore is its f-score,\n"
" best-index is an index of a highest f-score parse for this sentence,\n"
" best-fscore is its f-score, and\n"
" nparses is the number of parses for this sentence.\n";

#include "custom_allocator.h"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>

#include "data.h"

int main(int argc, char* argv[])
{
  if (argc != 2) {
    std::cerr << usage << std::endl;
    exit(EXIT_FAILURE);
  }

  // read corpus

  corpusflags_type cf;
  cf.Pyx_factor = cf.Px_propto_g = 0;
  const char* filesuffix = strrchr(argv[1], '.');
  std::string command(strcasecmp(filesuffix, ".bz2")
		      ? (strcasecmp(filesuffix, ".gz") ? "cat " : "gunzip -c ")
		      : "bzcat ");
  command += argv[1];
  FILE* fcfp = popen(command.c_str(), "r");
  if (fcfp == NULL) {
    std::cerr << "## Error: could not open feature-count-file.bz2 " 
	      << argv[1] << "\n\n" << usage << std::endl;
    exit(EXIT_FAILURE);
  }
  corpus_type *featcounts = read_corpus(&cf, fcfp, 0);
  pclose(fcfp);
  size_t nfeatures = featcounts->nfeatures;
  size_t nsentences = featcounts->nsentences;

  // read feature weights from stdin

  std::vector<Float> weights(nfeatures);
  unsigned int i;
  while (fscanf(stdin, " %u", &i) == 1) {
    Float weight = 1;
    fscanf(stdin, " =%lg", &weight);
    if (i >= weights.size()) {
      weights.resize(2*i);
      assert(weights.size() > i);
    }
    weights[i] = weight;
  }
  
  // find highest scoring parse
  
  std::vector<Float> score(featcounts->maxnparses);
  Float max_correct_score, max_score;

  for (size_t i = 0; i < nsentences; ++i) {
    sentence_type *s = &featcounts->sentence[i];
    size_t highest = 0, best = 0;
    if (s->nparses > 0) {
      highest = sentence_scores(s, &(weights[0]), 
				&(score[0]), &max_correct_score, &max_score);
      Float highest_fscore = 2 * s->parse[highest].w / (s->parse[highest].p + s->g);
      for (best = 0; best < s->nparses; ++best)
	if (s->parse[best].Pyx > 0.9)
	  break;
      assert(best < s->nparses);
      Float best_fscore = 2 * s->parse[best].w / (s->parse[best].p + s->g);
      std::cout << highest << '\t' << highest_fscore << '\t' 
		<< best << '\t' << best_fscore << '\t' 
		<< s->nparses << '\n';
    }
    else {
      std::cout << -1 << '\t' << 0 << '\t' << -1 << '\t' << 0 << '\t' << 0 << '\n';
    }
  }
}
