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

// prepare-data.cc -- converts Michael Collins files into my training format
//
// Mark Johnson, 14th August 2003

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
"	<LOGPROB> <CONDPROB> <PARSETREE>\n"
"\n"
"where:\n"
"\n"
"	<LOGPROB> is the log probability of the parse,\n"
"	<CONDPROB> is the conditional probability of the parse,\n"
"	<PARSETREE> is the parse tree.\n"
"\n"
"The parses are sorted in descending order by f-score and then log probability.\n"
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

// tree_terminals() appends the non-empty terminals to terms
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

struct penn {
  Ss_TIs yield_trees;
  size_t treeno;

  penn() : treeno() { }

  void operator()(const tree* tp0) {
    tree* tp = tp0->copy_without_empties();
    Ss yield;
    tree_terminals(tp, yield);
    const char* fn = rindex(readtree_filename, '/') + 1;
    if (fn == NULL)
      fn = readtree_filename;
    yield_trees.insert(Ss_TIs::value_type(yield, treeinfo(tp,fn,treeno++)));
}  // penn::operator()

};  // penn

struct collins_info {
  double logprob;
  double precrec;
  double condprob;
  double n_gold_constituents;
};

typedef ext::hash_map<std::string, collins_info> S_CI;

struct pci_cmp {   // sorts so parse with highest f-score is ordered first
  bool operator() (const S_CI::iterator& v1, const S_CI::iterator& v2) const
  {
    if (v1->second.precrec > v2->second.precrec)
      return true;
    else if (v1->second.precrec == v2->second.precrec
	     && v1->second.logprob > v2->second.logprob)
      return true;
    else
      return false;
  }
};  // pci_cmp{}


// final_punctuation() appends to str a string that encodes the final
//  punctuation of tree t.
//
void final_punctuation(const tree* t, std::string& str) {
  if (t->next != NULL)
    final_punctuation(t->next, str);
  else if (t->is_punctuation()) {
    str += " (";
    str += t->label.cat.string_reference();
    str += " ";
    str += t->child->label.cat.string_reference();
    str += ")";
  }
  else if (t->child != NULL)
    final_punctuation(t->child, str);
}

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


void write_collins_trees(const penn& p, S_CI& parse_stats, FILE* of, 
			 precrec_type& pr, bool writeflag = false)
{
  assert(!parse_stats.empty());
  typedef std::vector<S_CI::iterator> SCIs;
  SCIs parsestats;
  parsestats.reserve(parse_stats.size());
  foreach (S_CI, it, parse_stats)
    parsestats.push_back(it);
  std::sort(parsestats.begin(), parsestats.end(), pci_cmp());
  assert(parsestats.size() == parse_stats.size());
  // look up yield in p
  Ss yield;
  tree* best_parse = readtree_root(parsestats[0]->first.c_str());
  tree_terminals(best_parse, yield);             // this yield has no punctuation
  STIrange r = p.yield_trees.equal_range(yield); // r is the set of gold trees
  yield.clear();
  best_parse->terminals(yield);                  // now yield has punctuation
  std::string finalpunct, modified_treestring;
  if (r.first == r.second)
    std::cerr << "## Can't find yield " << yield 
	      << "\n## best_parse = " << best_parse
	      << "\n## string = " << parsestats[0]->first << std::endl;
  else {
    const tree* gold_parse = r.first->second.t;
    precrec_type::edges gold_edges, parse_edges;
    precrec_type::tree_nontermedges(gold_parse, gold_edges);
    precrec_type::tree_nontermedges(best_parse, parse_edges);
    pr(gold_edges, parse_edges);   // accumulate precision and recall
    final_punctuation(gold_parse, finalpunct);   // get final punctuation 
    if (writeflag) {
      for (STIit it = r.first; it != r.second; ++it)
	std::cerr << "# " << it->second.filename << ':' << it->second.treeno << ' ';
      std::cerr << std::endl;
    }
    
    // write out gold parse info

    fprintf(of, "%d %s\n", parsestats.size(), boost::lexical_cast<std::string>(gold_parse).c_str());
    // fprintf(of, "%d %d %d %s\n", parsestats.size(), yield.size(), gold_edges.size(),
    //         boost::lexical_cast<std::string>(gold_parse).c_str());

    cforeach (SCIs, iit, parsestats) {
      modified_treestring = (*iit)->first;
      size_t nc = modified_treestring.size();
      if (nc > 4 && modified_treestring[nc-1] == ')' && modified_treestring[nc-2] == ')'
	  && modified_treestring[nc-3] == ')' && modified_treestring[nc-4] == ')')
	modified_treestring.insert(nc-2, finalpunct);
      const tree* parse = readtree_root(modified_treestring.c_str());
      assert(parse != NULL);
      parse_edges.clear();
      precrec_type::tree_nontermedges(parse, parse_edges);
      precrec_type pr0(gold_edges, parse_edges);
      // write out parse info
      fprintf(of, "%g %g %s\n", (*iit)->second.logprob, (*iit)->second.condprob,
	      boost::lexical_cast<std::string>(parse).c_str());
      // fprintf(of, "%g %g %d %d %d %s\n", (*iit)->second.logprob, (*iit)->second.condprob,
      //         pr0.ntest, pr0.ncommon, npostagcorrect(gold_parse, parse), 
      //         boost::lexical_cast<std::string>(parse).c_str());
      delete parse;
    }
  }
  delete best_parse;
}  // write_collins_trees()

template <typename X1, typename X2>
inline bool approxeq(const X1& x1, const X2& x2) {
  if (fabs(x1-x2) <= 1e-4*fabs(x1+x2) + 1e-7) 
    return true;
  else {
    std::cerr << "## x1 = " << x1 << ", x2 = " << x2 << std::endl;
    return false;
  }
}  // approxeq()

int transform_collins(const penn& p, const char tree_command[], 
		      const char score_command[], FILE* of)
{
  FILE* tp = popen(tree_command, "r");
  assert(tp != NULL);
  FILE* sp = popen(score_command, "r");
  assert(sp != NULL);

  S_CI parse_stats;
  precrec_type pr;

  int old_parse_id = -1;
  int old_score_id = -1;
  int lineno = 0;
  int nsentences = 0;
  
  while(true) {
    int parse_id, rank, edge, score_id, parsenum, score_rank, best;
    double logprob, score, normscore, score_logprob, condprob; 
    int nread_parse = fscanf(tp, " ID %d PROB %lf RANK %d EDGE %d ",
			     &parse_id, &logprob, &rank, &edge);
    int nread_score = fscanf(sp, " %d %d %lf %lf %lf %d %lf %d ",
			     &score_id, &parsenum, &score, &normscore, 
			     &score_logprob, &score_rank, &condprob, &best);
    ++lineno;
    if (nread_parse == EOF) {
      assert(nread_score == EOF);
      ++nsentences;
      write_collins_trees(p, parse_stats, of, pr, true);
      std::cerr << "# Oracle " << pr << std::endl;
      break;
    }
    assert(nread_parse == 4);
    assert(nread_score == 8);
    assert(approxeq(logprob, score_logprob));
    assert(approxeq(rank, score_rank));

    if (old_parse_id == parse_id) 
      assert(old_score_id == score_id);
    else {
      assert(old_score_id != score_id);
      if (old_parse_id != -1) {
	++nsentences;
	write_collins_trees(p, parse_stats, of, pr, nsentences==1);
	parse_stats.clear();
      }
      old_parse_id = parse_id;
      old_score_id = score_id;
    }

    {
      char cparse[1024];
      char* cp = fgets(cparse, 1024, tp);
      assert(cp != NULL);
      assert(cparse[0] != '\0');
      for (size_t i = strlen(cparse)-1; i >= 0 && isspace(cparse[i]); --i)
	cparse[i] = '\0';  // replace final space with '\0'
      assert(strncmp(cparse, "(TOP ", 5) == 0);
      cparse[4] = '(';
      std::string parse(cparse+4);
      if (parse_stats.count(parse) == 0 
	  || parse_stats[parse].logprob < logprob) {
	collins_info& ci = parse_stats[parse];
	ci.logprob = logprob;
	ci.precrec = normscore;
	ci.condprob = condprob;
	if (normscore > 0)
	  ci.n_gold_constituents = score/normscore;
	else
	  ci.n_gold_constituents = -1;
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
 
  if (argc != 5) {
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
  int nsentences = transform_collins(p, argv[3], argv[4], stdout);
  if (nsentences != nsentences0) {
    std::cerr << "## Error: number of sentences from second argument (" << nsentences0
	      << ") is incorrect.\n## The correct number of sentences is " << nsentences
	      << ".\n## Please rerun " << argv[0] << " with second argument = "
	      << nsentences << std::endl;
    exit(EXIT_FAILURE);
  }

}
