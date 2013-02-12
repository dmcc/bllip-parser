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

// prepare-ec-data100.cc
//
// converts Eugene's n-best parser output into format for my parser
// a hacked up version of prepare-ec-data that 
// only prints out long sentences (for debugging)
//
// Mark Johnson, 9th December 2004

unsigned int min_sentence_length = 90;  // ignore sentences shorter than this

const char usage[] =
"Usage: prepare-ec-data100 max-beam-size nsentences [gold-parses.txt]\n"
"\n"
"where:\n"
"  max-beam-size is a threshold on the beam size (0 = no threshold),"
"  nsentences is the number of sentences in the data set,"
"  gold-parses.txt is a file from which treebank gold parses can be read,"
"  nskip is the number of parses in this file to skip, and\n"
"  Eugene Charniak's n-best parses are read from stdin, and\n"
"  the combined data file is written to stdout.\n"
"\n"
"Format of the output data:\n"
"=========================\n"
"\n"
"The first line of the data is:\n"
"\n"
"       <NSENTENCES>\n"
"\n"
"which is the number of sentences in the data.\n"
"\n"
"The rest of the data consists of a sequence of blocks, one per\n"
"sentence.  Each sentence block begins with a line of the form:\n"
"\n"
"       <NPARSES> <GOLDTREE>\n"
"\n"
"where:\n"
"\n"
"       <NPARSES> is the number of parses,\n"
"       <GOLDTREE> is the treebank tree.\n"
"\n"
"This is followed by <NPARSES> lines, one for each parse, of the form:\n"
"\n"
"       <LOGPROB> <PARSETREE>\n"
"\n"
"where:\n"
"\n"
"       <LOGPROB> is the log probability of the parse, and\n"
"       <PARSETREE> is the parse tree.\n"
"\n";

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <utility>
#include <vector>

#include <boost/array.hpp>
#include <ext/hash_map>

#include "sym.h"
#include "tree.h"
#include "utility.h"


typedef std::vector<symbol> symbols;

int transform_charniak(std::istream& is, std::ostream& os, 
		       const int beam_size_threshold = 0, std::istream* gisp = NULL)
{
  int nsentences = 0, sum_nparses = 0;
  int sentence_no, nparses, nparses_to_print;
  int gold_sentence_no = -1;
  tree* tp;
  tree* gold_tp = NULL;

  while(is >> sentence_no >> nparses >> tp) {
    if (beam_size_threshold > 0)
      nparses_to_print = std::min(beam_size_threshold, nparses);
    else
      nparses_to_print = nparses;

    symbols words;
    tp->terminals(words);
    if (words.size() < min_sentence_length)
      nparses_to_print = 0;

    tp->label.cat = tree_label::root();
    if (gisp == NULL && nparses_to_print > 0) {
      os << nparses_to_print << ' ' << tp << std::endl;
      delete tp;
    }
    else {
      while (gold_sentence_no < sentence_no) {
	delete gold_tp;
	gold_tp = NULL;
	(*gisp) >> gold_tp;
	++gold_sentence_no;
	if (!(*gisp)) {
	  std::cerr << "## Error in prepare-ec-data.cc, failed to read gold_tp, sentence_no = "
		    << sentence_no << ", gold_sentence_no = " << gold_sentence_no << std::endl;
	  exit(EXIT_FAILURE);
	}
	if (gold_tp == NULL) {
	  std::cerr << "## Error in prepare-ec-data.cc, gold_tp == NULL, sentence_no = "
		    << sentence_no << ", gold_sentence_no = " << gold_sentence_no << std::endl;
	  exit(EXIT_FAILURE);
	}
      }
      if (gold_sentence_no != sentence_no) {
	std::cerr << "## Error in prepare-ec-data.cc, sentence_no = "
		  << sentence_no 
		  << ", gold_sentence_no = " << gold_sentence_no << std::endl;
	exit(EXIT_FAILURE);
      }
      gold_tp->label.cat = tree_label::root();
      symbols tp_terminals, gold_tp_terminals;
      tp->terminals(tp_terminals);
      gold_tp->terminals(gold_tp_terminals);
      if (tp_terminals != gold_tp_terminals) {
	std::cerr << "## Error in prepare-ec-data.cc, sentence_no = " << sentence_no 
		  << ", tp_terminals = " << tp_terminals 
		  << ", gold_tp_terminals = " << gold_tp_terminals
		  << ", tp = " << tp
		  << ", gold_tp = " << gold_tp
		  << std::endl;
	exit(EXIT_FAILURE);
      }
      if (nparses_to_print > 0)
	os << nparses_to_print << ' ' << gold_tp << std::endl;
      delete tp;
    }

    // actually print the parses

    for (int j = 0; j < nparses; ++j) {
      double logprob;
      if (!(is >> logprob >> tp)) {
	std::cerr << "## Error in prepare-ec-data: failed to read parse " 
		  << j+1 << " of sentence " << sentence_no << ", logprob = " << logprob << std::endl;
	exit(EXIT_FAILURE);
      }
      assert(tp != NULL);
      tp->label.cat = tree_label::root();
      if (!is)
        std::cerr << "## Error prepare-ec-data:  read failure: sentence_no = " 
		  << sentence_no << ", j = " << j << std::endl;
      if (j < nparses_to_print)
	os << logprob << ' ' << tp << std::endl;
      delete tp;
      tp = NULL;
    }
  }

  std::cerr << "\n\n# The PTB corpus contained " << nsentences << " sentences.\n"
            << "# There were " << sum_nparses << " parses for " << nsentences
            << " different sentences, averaging " << sum_nparses/(1e-100+nsentences)
            << " parses per sentence.\n" 
            << std::endl;

  return nsentences;
}


int main(int argc, char* argv[])
{
  if (argc < 3 || argc > 4) {
    std::cerr << usage << std::endl;
    exit(EXIT_FAILURE);
  }

  char* endptr = NULL;
  int beam_size_threshold = strtol(argv[1], &endptr, 10);
  if (endptr == NULL || *endptr != '\0' || beam_size_threshold < 0) {
    std::cerr << "## Error: expected a non-negative integer beam-size-threshold argument, but got " 
              << argv[1] << std::endl
              << usage << std::endl;
    exit(EXIT_FAILURE);
  }

  int nsentences0 = strtol(argv[2], &endptr, 10);
  if (endptr == NULL || *endptr != '\0') {
    std::cerr << "## Error: expected an integer nsentences0 argument, but got " 
              << argv[2] << std::endl
              << usage << std::endl;
    exit(EXIT_FAILURE);
  }

  std::cout << nsentences0 << '\n';

  int nsentences = 0;

  if (argc == 3) 
    nsentences = transform_charniak(std::cin, std::cout, beam_size_threshold);
  else if (argc == 4) {
    std::ifstream gis(argv[3]);
    if (!gis) {
      std::cerr << "## Error in prepare-ec-data: can't open file " << argv[3] << std::endl;
      exit(EXIT_FAILURE);
    }
    nsentences = transform_charniak(std::cin, std::cout, beam_size_threshold, &gis);
  }

  if (nsentences != nsentences0) {
    std::cerr << "## Error: the nsentences argument (" << nsentences0
              << ") is incorrect.\n## The correct number of sentences is " << nsentences
              << ".\n## Please rerun " << argv[0] << " with second argument = "
              << nsentences << std::endl;
    exit(EXIT_FAILURE);
  }

}
