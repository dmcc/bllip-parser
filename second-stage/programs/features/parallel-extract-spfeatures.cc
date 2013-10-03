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

// extract-spfeatures.cc
// Parallelized version by dmcc -- to be documented
//
// Mark Johnson, 2nd November 2005, last modified 2nd December 2007

const char usage[] =
"Usage:\n"
"\n"
"parallel-extract-spfeatures [-a] [-c] [-d <debug>] [-f <f>] [-i] [-l] \n"
"  train.nbest.cmd train.gold.cmd train.gz\n"
" (dev.nbest.cmd dev.gold.cmd dev.gz)*\n"
"\n"
"where:\n"
" -a causes the program to produce absolute feature counts (rather than relative counts),\n"
" -c collect features from correct examples,\n"
" -d <debug> turns on debugging output,\n"
" -f <f> uses feature classes <f>,\n"
" -i collect features from incorrect examples,\n"
" -l maps all words to lower case as trees are read,\n"
"\n"
" train.nbest.cmd produces the n-best parses for training the reranker,\n"
" train.gold.cmd is a command which produces the corresponding gold parses,\n"
" train.gz is the file into which the extracted features are written,\n"
" dev.nbest.cmd, dev.gold.cmd and dev.gz are corresponding development files.\n"
"\n"
"The extracted features are written to standard output.\n";

#include "custom_allocator.h"       // must be first

// #define _GLIBCPP_CONCEPT_CHECKS  // uncomment this for checking

// #include <boost/lexical_cast.hpp>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <unistd.h>
#include <vector>

#include "sp-data.h"
#include "features.h"
#include "utility.h"

int debug_level = 0;
bool absolute_counts = false;
bool collect_correct = false;
bool collect_incorrect = false;
bool lowercase_flag = false;

int main(int argc, char **argv) {

  std::ios::sync_with_stdio(false);

  const char* fcname = NULL;

  int c;
  while ((c = getopt(argc, argv, "acd:f:il")) != -1 )
    switch (c) {
    case 'a':
      absolute_counts = true;
      break;
    case 'c':
      collect_correct = true;
      break;
    case 'd':
      debug_level = atoi(optarg);
      break;
    case 'f':
      fcname = optarg;
      break;
    case 'i':
      collect_incorrect = true;
      break;
    case 'l':
      lowercase_flag = true;
      break;
    default:
      std::cerr << "## Error: can't interpret argument " << c << " " << optarg << std::endl;
      std::cerr << usage << std::endl;
      exit(EXIT_FAILURE);
    }

  if ((argc - optind) < 3 || (argc - optind) % 3 != 0) {
    std::cerr << "## Error: missing required arguments.\n" << usage << std::endl;
    exit(EXIT_FAILURE);
  }

  std::cerr 
    << "# debug_level (-d) = " << debug_level
    << ", featureclasses (-f) = " << (fcname ? fcname : "NULL")
    << ", absolute_counts (-a) = " << absolute_counts
    << ", collect_correct (-c) = " << collect_correct
    << ", collect_incorrect (-i) = " << collect_incorrect
    << ", lowercase_flag (-l) = " << lowercase_flag
    << std::endl;

  if (collect_correct == false && collect_incorrect == false) {
    std::cerr << "## Error: you must set at least one of -c or -i." << std::endl;
    exit(EXIT_FAILURE);
  }
  
  // initialize feature classes
  //
  FeatureClassPtrs fcps(fcname);

  // read in features from stdin
  int num_features = fcps.read_feature_ids(std::cin);
  std::cerr << "# number of features: " << num_features << std::endl;

  // process all arguments
  for (int i = optind; i+1 < argc; i += 3) {
    std::cerr << "# reading from test \"" << argv[i] 
	      << "\"" << std::endl;
    std::cerr << "# reading from gold \"" << argv[i + 1]
	      << "\"" << std::endl;
    std::cerr << "# writing to \"" << argv[i+2] << "\"" << std::endl;

    fcps.write_features(argv[i], argv[i+1], argv[i+2]);
    std::cerr << "# usage " << resource_usage() << std::endl;
  }

} // main()
