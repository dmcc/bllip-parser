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

// ptb.cc -- Maps Penn Treebank trees
//
// Mark Johnson, 9th April 2005, last modified 15th November 2009
//
// Maps Penn treebank trees into Charniak parser input strings,
// Berkeley parser input strings, or EVALB format trees

const char info[] =
"Usage: ptb [-c] [-e] [-g] [-i i] [-n n] [-x x] filename ...\n"
"\n"
"maps Penn Treebank trees into Charniak parser input strings,\n"
"EVALB format trees or reranker training format.\n"
"\n"
" -b writes input strings for Berkeley parser to stdout.\n"
" -c writes input strings for Charniak parser to stdout.\n"
" -e writes trees in EVALB format to stdout.\n"
" -f strip function tags from trees.\n"
" -g writes trees in gold-standard format needed for training reranker.\n"
" -n n divide the data into n equal-sized folds.\n"
" -i i only include fold i.\n"
" -x x exclude fold x.\n"
"\n"
"You must select one of the -c -e or -g flags.";

#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>
#include <utility>
#include <vector>

#include "lexical_cast.h"
#include "sym.h"
#include "tree.h"
#include "popen.h"

typedef std::vector<symbol> symbols;

static symbol top("TOP");

template <typename label_type>
tree_node<label_type>* strip_function_tags(tree_node<label_type>* tp) {
  if (tp == NULL || tp->is_terminal()) 
    return tp;
  tp->label = tp->label.simplified_cat();
  strip_function_tags(tp->child);
  strip_function_tags(tp->next);
  return tp;
}

int main(int argc, char** argv) {

  int nfolds = -1;
  int ifold = -1;
  int xfold = -1;

  bool berkeley_strings = false;
  bool charniak_strings = false;
  bool evalb_trees = false;
  bool no_function_tags = false;
  bool reranker_trees = false;
  bool no_top_node = false;
  int  noutset = 0;

  opterr = 0;
  int c;

  while ((c = getopt (argc, argv, "bcefgi:n:tx:")) != -1)
    switch (c) {
    case 'b': berkeley_strings = true; ++noutset; break;
    case 'c': charniak_strings = true; ++noutset; break;
    case 'e': evalb_trees = true; ++noutset; break;
    case 'f': no_function_tags = true; break;
    case 'g': reranker_trees = true; ++noutset; break;
    case 'i': ifold = atoi(optarg); break;
    case 'n': nfolds = atoi(optarg); break;
    case 't': no_top_node = true; break;
    case 'x': xfold = atoi(optarg); break;
    default: std::cerr << "Unknown option in ptb:\n"
		       << info << std::endl; exit(EXIT_FAILURE); break;
    }

  if (nfolds != -1)
    if (!((ifold == -1 && xfold >= 0 && xfold < nfolds) ||
	  (xfold == -1 && ifold >= 0 && ifold < nfolds)))
      std::cerr << "## ptb.cc: inconsistent options:"
		<< " -i " << ifold << " -n " << nfolds << " -x " << xfold
		<< std::endl;

  if (noutset != 1)
    std::cerr << "## Warning from ptb.cc: normally you should set exactly one of -b -c -e -g" 
	      << std::endl;
     
  int nsentences = 0;
  {
    // count sentences
    for (int index = optind; index < argc; ++index) {
      izstream is(argv[index]);
      tree* tp;
      while (is >> tp) {
	++nsentences;
	delete tp;
      }
    }
  }

  int nsentences_toprint = nsentences;

  int fold_start = 0;
  int fold_end = -1;
  if (nfolds > 0) {
    int fold = std::max(ifold, xfold);
    fold_start = (fold*nsentences)/nfolds;
    fold_end = ((fold+1)*nsentences)/nfolds;
    if (ifold >= 0)
      nsentences_toprint = fold_end - fold_start;
    else
      nsentences_toprint = nsentences - (fold_end - fold_start);
  }

  // write number of sentences in data if required
  if (reranker_trees) 
    std::cout << nsentences_toprint << std::endl;

  int sentenceno = 0;
  int nsentences_printed = 0;

  for (int index = optind; index < argc; index++) {
    int idno = 0;
    izstream is(argv[index]);
    std::string id(argv[index]);
    std::string::size_type pos = id.rfind("/");
    if (pos != std::string::npos && pos+1 < id.size())
      id.erase(0, pos+1);
    pos = id.rfind(".");
    if (pos != std::string::npos)
      id.erase(pos);    
    tree* tp;
    while (is >> tp) {
      if ((ifold < 0 && xfold < 0)   // no folds
	  || (ifold >= 0 && sentenceno >= fold_start && sentenceno < fold_end)
	  || (xfold >= 0 && (sentenceno < fold_start || sentenceno >= fold_end))) {
	++nsentences_printed;
	std::string idstring(id+"."+lexical_cast<std::string>(idno));
	if (no_function_tags)
	  strip_function_tags(tp);
	if (berkeley_strings) {
	  symbols words;
	  tp->terminals(words);
	  cforeach (symbols, it, words) {
	    if (it != words.begin())
	      std::cout << ' ';
	    std::cout << it->string_reference();
	  }
	  std::cout << "\n";
	}
	if (charniak_strings) {
	  std::cout << "<s " << idstring << " >";
	  symbols words;
	  tp->terminals(words);
	  cforeach (symbols, it, words)
	    std::cout << ' ' << it->string_reference();
	  std::cout << " </s>\n";
	}
	if (evalb_trees) {
	  if (!no_top_node)
	    tp->label.cat = top;
	  write_tree_noquote(std::cout, tp);
	  std::cout << std::endl;
	}
	if (reranker_trees) {
	  std::cout << idstring << "\t";
	  tp->label.cat = tree_label::root();
	  write_tree_noquote(std::cout, tp);
	  std::cout << std::endl;	  
	}
      }
      delete tp;
      ++idno;
      ++sentenceno;
    }
  }

  if (nsentences_printed != nsentences_toprint)
    std::cerr << "## nsentences_printed = " << nsentences_printed 
	      << ", nsentences_toprint = " << nsentences_toprint
	      << ", fold_start = " << fold_start
	      << ", fold_end = " << fold_end
	      << std::endl;

  assert(nsentences_printed == nsentences_toprint);
}  // main()
