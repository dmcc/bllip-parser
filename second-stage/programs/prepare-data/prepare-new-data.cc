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

// prepare-new-data.cc -- converts Michael Collins files into my training format
//
// Mark Johnson, 11th October 2003, last modified 21st October 2004

const char usage[] =
"Usage: prepare-new-data nskip nsentences wsj-command collins-command\n"
"\n"
"where:\n"
"  nskip is the number of sentences in wsj-command to skip,\n"
"  nsentences is the number of sentences in the data set,\n"
"  wsj-command is a shell expression that writes the PTB parses to stdout, and\n"
"  collins-command is a shell expression that writes the Collins parses\n"
"      to stdout\n"
"\n"
"The combined data file is written to stdout.\n"
"\n"
"Format of the output data:\n"
"=========================\n"
"\n"
"The first line of the data is:\n"
"\n"
"	<NSENTENCES>\n"
"\n"
"which is the number of sentences in the data.\n"
"\n"
"The rest of the data consists of a sequence of blocks, one per\n"
"sentence.  Each sentence block begins with a line of the form:\n"
"\n"
"	<NPARSES> <GOLDTREE>\n"
"\n"
"where:\n"
"\n"
"	<NPARSES> is the number of parses,\n"
"	<GOLDTREE> is the treebank tree.\n"
"\n"
"This is followed by <NPARSES> lines, one for each parse, of the form:\n"
"\n"
"	<LOGPROB> <PARSETREE>\n"
"\n"
"where:\n"
"\n"
"	<LOGPROB> is the log probability of the parse, and\n"
"	<PARSETREE> is the parse tree.\n"
"\n"
"Duplicate parses produced by Collins' parser are deleted, but otherwise\n"
"the parses are in alphabetical (!) order.\n"
"\n";

// The general algorithm is:
//
// while !eof(parses):
//   read next batch of parses
//   while true:
//      read next Penn tree
//      if yield(Penn) == yield(Parse):
//         break
//      else:
//         print(Penn tree, no parses)
//   print(Penn tree, parses)

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <utility>
#include <vector>

#include <boost/array.hpp>
#include <boost/lexical_cast.hpp>

#include <ext/hash_map>

#include "symset.h"
#include "tree.h"
#include "utility.h"

typedef std::vector<symbol> Ss;
typedef ext::hash_map<std::string, double> S_D;

// tree_terminals() appends the non-empty non-punctuation 
//  terminals to terms
//
void tree_terminals(const tree* t, Ss& terms) 
{
  if (t->child == NULL) 
    terms.push_back(t->label.cat);
  else { // child != NULL
    if (!t->label.is_punctuation() && !t->label.is_none())
      tree_terminals(t->child, terms);
  }
  if (t->next != NULL)
    tree_terminals(t->next, terms);
}  // tree_terminals()


void write_collins(const std::string& penn, const S_D& parse_logprob, FILE* of)
{
  static int nsentence = 0;
  ++nsentence;
  tree* gold = readtree(penn.c_str());
  assert(gold != NULL);
  gold->label.cat = tree_label::root();
  fprintf(of, "%ud %s\n", unsigned(parse_logprob.size()), 
	  boost::lexical_cast<std::string>(gold).c_str());
  Ss gold_yield;
  gold->terminals(gold_yield);
  // tree_terminals(gold, gold_yield);

  if (false) {
    std::cerr << "\n# nsentence = " << ++nsentence << std::endl;
    std::cerr << "# gold_yield = " << gold_yield << std::endl;
    if (parse_logprob.empty()) 
      std::cerr << "# no parse" << std::endl;
    else {
      tree* parse = readtree(parse_logprob.begin()->first.c_str());
      Ss parse_yield;
      parse->terminals(parse_yield);
      std::cerr << "# first parse_yield = " << parse_yield << std::endl;
      delete parse;
    }
  }

  int nerrors = 0;

  cforeach (S_D, it, parse_logprob) {
    tree* parse = readtree(it->first.c_str());
    assert(parse != NULL);
    parse->label.cat = tree_label::root();
    fprintf(of, "%g %s\n", it->second, 
	    boost::lexical_cast<std::string>(parse).c_str());
    Ss parse_yield;
    parse->terminals(parse_yield);
    // tree_terminals(parse, parse_yield);
    if (gold_yield != parse_yield) {
      std::cerr << "\n## nsentence = " << nsentence << std::endl;
      std::cerr << "## Error: gold_yield = " << gold_yield << std::endl;
      std::cerr << "## Error: parse_yield = " << parse_yield << std::endl;
      ++nerrors;
      // exit(EXIT_FAILURE);
    }
    delete parse;
  }
  if (nerrors > 0)
    std::cerr << "## The mismatch occured " << nerrors << " times.\n" << std::endl;

  delete gold;
}

// read_line() reads the rest of the line from fp, and returns the
// character at which reading stopped.
//
int read_line(FILE* fp, std::string& s) {
  int c;
  while ((c = getc(fp)) != EOF && (c != '\n'))
    s.push_back(c);
  return c;
}


// skip_line() skips the rest of the line from fp
//
int skip_line(FILE* fp) {
  int c;
  while ((c = getc(fp)) != EOF && (c != '\n'))
    ;
  return c;
}

// read_parses() reads the next batch of non-empty parses from fp.
//
void read_parses(FILE* inf, S_D& parse_logprob)
{
  assert(inf != NULL);
  assert(!feof(inf));

  static int         last_id = EOF;  // EOF == last parse failed, >0 == normal last parse
  static std::string last_parse;
  static double      last_logprob = 0;
  static int         lineno = 0;

  parse_logprob.clear();

  if (last_id >= 0) {
    assert(!last_parse.empty());
    parse_logprob[last_parse] = last_logprob;
  }

  while (true) {
    double logprob;
    std::string parse;
    int id, rank, edge;

    int nread = fscanf(inf, " ID %d PROB %lf RANK %d EDGE %d ",
		       &id, &logprob, &rank, &edge);

    switch (nread) {
    case EOF:
      last_id = EOF;
      last_parse.clear();
      return;
      break;
    case 0:
      skip_line(inf);
      ++lineno;
      last_id = EOF;
      last_parse.clear();
      if (!parse_logprob.empty())
	return;
      break;
    case 4:
      read_line(inf, parse);
      ++lineno;
      if (last_id == id || last_id == -1) {
	last_id = id;
	std::pair<S_D::iterator,bool> itb = parse_logprob.insert(S_D::value_type(parse, logprob));
	if (itb.second == false && itb.first->second < logprob)
	  itb.first->second = logprob;
      }
      else {
	last_id = id;
	last_parse = parse;
	last_logprob = logprob;
	return;
      }
      break;
    }
  }
}  // read_parses()

int transform_collins(FILE* pennf, FILE* collinsf, FILE* of)
{
  assert(pennf != NULL);
  assert(collinsf != NULL);

  int ngolds = 0;
  int nparses = 0;
  int sum_parses = 0;

  while(!feof(collinsf)) {
    S_D parse_logprob, empty_parse_logprob;
 
    read_parses(collinsf, parse_logprob);
    ++nparses;
    sum_parses += parse_logprob.size();
    if (parse_logprob.empty())
      break;
    
    tree* parse_tree = readtree(parse_logprob.begin()->first.c_str());
    Ss parse_yield;
    parse_tree->terminals(parse_yield);
    delete parse_tree;

    std::string gold;

    while (true) {   // read PTB "gold" trees
      gold.clear();
      read_line(pennf, gold);
      ++ngolds;

      tree* gold_tree = readtree(gold.c_str());      
      if (!gold_tree) {
	std::cerr << "## Error: failed to read gold = " << gold << std::endl;
	exit(EXIT_FAILURE);
      }
      Ss gold_yield;
      gold_tree->terminals(gold_yield);
      delete gold_tree;

      if (parse_yield != gold_yield) {
	std::cerr << "# ngolds = " << ngolds 
	       // << ", parse_yield = " << parse_yield 
		  << ", gold_yield = " << gold_yield << std::endl;
	write_collins(gold, empty_parse_logprob, of);
      }
      else
	break;
    }
    write_collins(gold, parse_logprob, of);
  }

  std::cerr << "\n\n# The PTB corpus contained " << ngolds << " sentences.\n"
	    << "# There were " << sum_parses << " parses for " << nparses 
	    << " different sentences, averaging " << sum_parses/(1e-100+nparses) 
	    << " parses per sentence.\n" 
	    << "# " << ngolds - nparses << " sentences did not receive a parse.\n"
	    << std::endl;
  return ngolds;
}


int main(int argc, char* argv[])
{
  std::ios::sync_with_stdio(false);
 
  if (argc != 5) {
    std::cerr << usage << std::endl;
    exit(EXIT_FAILURE);
  }
  

  char* endptr = NULL;
  int nskip = strtol(argv[1], &endptr, 10);
  if (endptr == NULL || *endptr != '\0') {
    std::cerr << "## Error: expected an integer nskip argument, but got " 
	      << argv[1] << std::endl
	      << usage << std::endl;
    exit(EXIT_FAILURE);
  }

  int nsentences0 = strtol(argv[2], &endptr, 10);
  if (endptr == NULL || *endptr != '\0') {
    std::cerr << "## Error: expected an integer as nsentences, but got " 
	      << argv[2] << std::endl
	      << usage << std::endl;
    exit(EXIT_FAILURE);
  }

  fprintf(stdout, "%d\n", nsentences0);

  FILE* pennf = popen(argv[3], "r");
  assert(pennf != NULL);
  FILE* collinsf = popen(argv[4], "r");
  assert(collinsf != NULL);

  for (int i = 0; i < nskip; ++i) {
    skip_line(pennf);
    assert(!feof(pennf));
  }

  int nsentences = transform_collins(pennf, collinsf, stdout);

  pclose(pennf);
  pclose(collinsf);

  if (nsentences != nsentences0) {
    std::cerr << "## Error: the nsentences argument (" << nsentences0
	      << ") is incorrect.\n## The correct number of sentences is " << nsentences
	      << ".\n## Please rerun " << argv[0] << " with second argument = "
	      << nsentences << std::endl;
    exit(EXIT_FAILURE);
  }

}
