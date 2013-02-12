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

// prepare-data2.cc -- converts Michael Collins new file into my training format
//
// Mark Johnson, 8th October 2003

const char usage[] =
"Usage: prepare-data penn-wsj-regex nsentences collins-tree-command collins-score-command\n"
"\n"
"where:\n"
"  penn-wsj-regex is a regular expression that expands to the names of\n"
"      the Penn WSJ treebank files,\n"
"  nsentences is the number of sentences in the data,\n"
"  collins-tree-command is a shell expression that writes Collins trees\n"
"      to stdout, and\n"
"  collins-score-command is a shell expression that writes Collins scores\n"
"      to stdout.\n"
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
"The parses are not sorted.\n"
"\n";

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

typedef ext::hash_map<symbol,size_t> S_C;
typedef std::vector<symbol> Ss;

// tree_terminals() appends the non-empty non-punctuation terminals to terms
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


// tree_preterminals() appends the non-empty terminals to terms
//
void tree_preterminals(const tree* t, Ss& preterms) 
{
  if (t->is_preterminal()) {
    if (!t->label.is_punctuation() && !t->label.is_none())
      preterms.push_back(t->label.cat);
  }
  else if (t->is_nonterminal())
    tree_preterminals(t->child, preterms);
  if (t->next != NULL)
    tree_preterminals(t->next, preterms);
}  // tree_preterminals()

struct treeinfo {
  tree*  t;
  symbol filename;
  size_t treeno;
  template <typename S>
  treeinfo(tree* t, const S& filename, size_t treeno) 
    : t(t), filename(filename), treeno(treeno) { }
}; // treeinfo

typedef ext::hash_multimap<Ss,treeinfo> Ss_TIs;
typedef Ss_TIs::const_iterator STIit;
typedef std::pair<STIit,STIit> STIrange;

typedef std::vector<treeinfo*> TIPtrs;

struct penn {
  Ss_TIs yield_trees;
  TIPtrs treeno_treeinfo;

  size_t treeno;

  penn() : treeno() { treeno_treeinfo.reserve(40000); }

  void operator()(const tree* tp0) {
    tree* tp = tp0->copy_without_empties();
    Ss yield;
    tree_terminals(tp, yield);
    const char* fn = rindex(readtree_filename, '/') + 1;
    if (fn == NULL)
      fn = readtree_filename;
    STIit it = yield_trees.insert(Ss_TIs::value_type(yield, treeinfo(tp,fn,treeno++)));
    treeno_treeinfo.push_back(&it->second);
    assert(treeno == treeno_treeinfo.size());
  }  // penn::operator()

};  // penn

struct collins_info {
  double logprob;
  tree*  t;
};

typedef ext::hash_map<std::string, collins_info> S_CI;

// npostagcorrect() returns the number of POS tags in the two sentences
// that are the same.
//
size_t npostagcorrect(const tree* t1, const tree* t2) {
  Ss pt1s, pt2s;
  tree_preterminals(t1, pt1s);
  tree_preterminals(t2, pt2s);
  if (pt1s.size() != pt2s.size()) 
    std::cerr << "## Error: pt1s.size() = " << pt1s.size() 
	      << ", pt2s.size() = " << pt2s.size() << '\n'
	      << "## t1 = " << t1 << '\n'
	      << "## t2 = " << t2 << std::endl;
  size_t ncorrect = 0;
  for (size_t i = 0; i < pt1s.size(); ++i)
    if (pt1s[i] == pt2s[i])
      ++ncorrect;
  return ncorrect;
}  // npostagcorrect()

// write_collins_trees() returns the tree 
size_t write_collins_trees(const penn& p, S_CI& parse_stats, FILE* of, 
			   int& treeno, bool writeflag = false)
{
  assert(!parse_stats.empty());
  treeinfo* gold_tip = NULL;

  if (treeno < 0) {
    // first tree; look it up
    Ss yield;
    tree* parse = readtree_root(parse_stats.begin()->first.c_str());
    parse->terminals(yield);
    STIrange r = p.yield_trees.equal_range(yield); // r is the set of gold trees
    if (r.first == r.second) {
      std::cerr << "## Error: Can't find first parse's yield " << yield 
		<< "\n## parse = " << best_parse << std::endl;
      exit(EXIT_FAILURE);
    }
    else if (r.second - r.first != 1) {
      std::cerr << "## Error: First parse's yield matches " << r.second - r.first << " Penn trees\n"
		<< "## yield = " << yield << std::endl;
      exit(EXIT_FAILURE);
    }
    else {
      // exactly one gold tree with this yield -- great!!
      gold_tip = &r.first->second;
      treeno = tip->treeno;
    }
  }
  else {
    // not first tree; look up treeinfo based on treeno
    ++treeno;
    assert(treeno < p.treeno_treeinfo.size());
    gold_tip = p.treeno_treeinfo[treeno];
  }
      
  if (writeflag) 
    std::cerr << "# " << gold_tip->filename << ':' << gold_tip->treeno << std::endl;

  // look up yield in p
  Ss gold_yield;
  gold_tip->t->terminals(gold_yield);

  size_t nparses = 0;
  
  
  cforeach (S_CI, it, parse_stats) {
    const tree* parse = readtree_root(it->first.c_str());
    if (parse == NULL) {
      std::cerr << "## Couldn't read parse tree from string " << it->first 
		<< "\n## " << gold_tip->filename << ':' << gold_tip->treeno
		<< "\n## " << gold_yield << std::endl;
      it->second.t = NULL;
    }
    else {
      Ss yield;
      parse->terminals(yield);
      if (yield != gold_yield) {
	std::cerr << "## Parse's yield differs from gold tree's yield " 
		  << gold_tip->filename << ':' << gold_tip->treeno
		  << "\n## parse yield " << yield 
		  << "\n## gold yield " << gold_yield
		  << std::endl;
	delete parse;
	it->second.t = NULL;
      }
      else {
	// Got a matching parse
	++nparses;
	it->second.t = parse;
      }
    }
  }

  // write out gold parse info
  
  fprintf(of, "%d %s\n", nparses, 
	  boost::lexical_cast<std::string>(gold_tip->t).c_str());

  cforeach (S_CI, it, parse_stats) 
    if (it->second.t != NULL) {
      fprintf(of, "%g %s\n", it->second.logprob, 
	      boost::lexical_cast<std::string>(it->second.t).c_str());
      delete it->second.t;
    }
}  // write_collins_trees()


int transform_collins(const penn& p, const char tree_command[], FILE* of)
{
  FILE* tp = popen(tree_command, "r");
  assert(tp != NULL);

  S_CI parse_stats;

  int old_parse_id = -1;
  int lineno = 0;
  int nsentences = 0;
  int treeno = -1;  // indicates no tree found yet

  while(true) {
    int parse_id, rank, edge, score_id, parsenum, score_rank, best;
    double logprob, score, normscore, score_logprob, condprob; 
    int nread_parse = fscanf(tp, " ID %d PROB %lf RANK %d EDGE %d ",
			     &parse_id, &logprob, &rank, &edge);
    ++lineno;
    if (nread_parse == EOF) {
      ++nsentences;
      write_collins_trees(p, parse_stats, of, treeno, true);
      std::cerr << "# Oracle " << pr << std::endl;
      break;
    }
    assert(nread_parse == 4);

    if (old_parse_id != parse_id) {
      if (old_parse_id != -1) {
	++nsentences;
	write_collins_trees(p, parse_stats, of, treeno, nsentences==1);
	parse_stats.clear();
      }
      old_parse_id = parse_id;
    }

    {
      char cparse[1024];
      char* cp = fgets(cparse, 1024, tp);
      assert(cp != NULL);
      assert(cparse[0] != '\0');
      for (int i = strlen(cparse)-1; i >= 0 && isspace(cparse[i]); --i)
	cparse[i] = '\0';  // replace final space with '\0'
      assert(strncmp(cparse, "(TOP ", 5) == 0);
      cparse[4] = '(';
      std::string parse(cparse+4);
      if (parse_stats.count(parse) == 0 
	  || parse_stats[parse].logprob < logprob) {
	collins_info& ci = parse_stats[parse];
	ci.logprob = logprob;
      }
    }
  }
  pclose(tp);
  pclose(sp);
  return nsentences;
}  // transform_collins()

int main(int argc, char* argv[])
{
  std::ios::sync_with_stdio(false);
 
  if (argc != 4) {
    std::cerr << usage << std::endl;
    exit(EXIT_FAILURE);
  }
  
  penn p;
  map_regex_trees(argv[1], p);

  std::cerr << "# Read " << p.yield_trees.size() 
	    << " penn trees." << std::endl;

  char* endptr = NULL;
  int nsentences0 = strtol(argv[2], &endptr, 10);
  if (endptr == NULL || *endptr != '\0') {
    std::cerr << "## Error: expected an integer as second argument, but got " 
	      << argv[2] << std::endl;
    exit(EXIT_FAILURE);
  }

  fprintf(stdout, "%d\n", nsentences0);
  int nsentences = transform_collins(p, argv[3], stdout);
  if (nsentences != nsentences0) {
    std::cerr << "## Error: number of sentences from second argument (" << nsentences0
	      << ") is incorrect.\n## The correct number of sentences is " << nsentences
	      << ".\n## Please rerun " << argv[0] << " with second argument = "
	      << nsentences << std::endl;
    exit(EXIT_FAILURE);
  }

}
