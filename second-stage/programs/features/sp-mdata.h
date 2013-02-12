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

// sp-mdata.h -- Reads n-best parses with multiple scores per parse
//
// Mark Johnson, 6th Feb 2010, last modified 31st March, 2010
//
// Reads parse trees produced by m of Petrov's n-best parsers

#pragma once

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <functional>
#include <iostream>
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "popen.h"
#include "sptree.h"
#include "tree.h"

typedef double Float;
typedef std::map<symbol,Float> S_F;
typedef std::map<symbol,unsigned> S_U;
// typedef std::vector<Float> Floats;

// strip_function_tags() destructively removes the function tags from
// a treebank tree
//
template <typename label_type>
tree_node<label_type>* strip_function_tags(tree_node<label_type>* tp) {
  if (tp == NULL || tp->is_terminal()) 
    return tp;
  tp->label = tp->label.simplified_cat();
  strip_function_tags(tp->child);
  strip_function_tags(tp->next);
  return tp;
}

// parse_type{} holds the data for a single parse.  It has a pointer
// to the parse tree, but someone else must free it when it is deleted!
//
struct sp_parse_type {
  typedef std::set<symbol> Ss;
  Float sum_logprob;       //!< sum of log probs from each individual parser
  S_F parser_logprob;      //!< log probabilities from parsers
  S_F parser_logcondprob;  //!< log cond probabilities from parsers
  S_U parser_rank;         //!< rank that parser gives to this parse
  Ss failedparsers;        //!< parsers that failed
  size_t nedges;
  size_t ncorrect;
  float f_score;           //!< f-score of this parse
  sptree* parse;
  tree* parse0;
  
  // default constructor
  //
  sp_parse_type() : 
    sum_logprob(0), nedges(0), ncorrect(0), f_score(0), parse(NULL), parse0(NULL)
  { }

  static void write_next_thousand_chars(FILE* fp) {
    std::cerr << "Next 1000 characters:" << std::endl;
    for (size_t k = 0; k < 1000; ++k) {
      int chr = getc(fp);
      if (chr != EOF)
	std::cerr << char(chr);
      else {
	std::cerr << "\n--EOF--";
	break;
      }
    }
    std::cerr << std::endl;
  }  // sp_parse_type::write_next_thousand_chars()

  static void write_next_thousand_chars(std::istream& is) {
    std::cerr << "Next 1000 characters:" << std::endl;
    for (size_t k = 0; k < 1000; ++k) {
      int chr = is.get();
      if (chr != EOF)
	std::cerr << char(chr);
      else {
	std::cerr << "\n--EOF--";
	break;
      }
    }
    std::cerr << std::endl;
  }  // sp_parse_type::write_next_thousand_chars()

  //! read() reads merged n-best parser output with multiple parse probabilities
  //
  std::istream& read(std::istream& is, bool downcase_flag=false) {
    static symbol parser1key("p0-ll");
    std::string line;
    getline(is, line);  // skip end of line
    if (!is)
      return is;
    sum_logprob = 0;
    if (getline(is, line)) {
      std::istringstream iss(line);
      Float logprob;
      iss >> logprob;
      sum_logprob += logprob;
      if (!iss)
	std::cerr << "## Error: expected to read a logprob\n## line = " << line << std::endl;
      parser_logprob[parser1key] = logprob;
      std::string parserkey;
      while (iss >> " , " >> parserkey >> logprob) {
	parser_logprob[parserkey] = logprob;
	sum_logprob += logprob;
      }
      is >> parse0; 
      ASSERT(is);
      ASSERT(parse0 != NULL);
      parse0->label.cat = tree::label_type::root();
      parse = tree_sptree(parse0, downcase_flag);
      assert(parse != NULL);
      if (false) {
	std::cerr << "# line = " << line << std::endl;
	std::cerr << "# parser_logprob = " << parser_logprob << std::endl;
	std::cerr << "# parse0 = " << parse0 << std::endl;
      }
    }
    return is;
  }  // read()

  //! read() reads merged n-best parser output with multiple parse probabilities
  //! This version handles the situation when the parser probs and the 
  //! parse trees are on the same line
  /*
  std::istream& read(std::istream& is, bool downcase_flag=false) {
    static symbol parser1key("p0-ll");
    std::string line;
    is >> " ";  // skip blanks, including end of lines
    if (!is)
      return is;
    sum_logprob = 0;
    if (getline(is, line)) {
      std::istringstream iss(line);
      Float logprob;
      iss >> logprob;
      sum_logprob += logprob;
      if (!iss)
	std::cerr << "## Error: expected to read a logprob\n## line = " << line << std::endl;
      parser_logprob[parser1key] = logprob;
      std::string parserkey;
      while (iss >> " , " >> parserkey >> logprob)
	parser_logprob[parserkey] = logprob;
      iss.clear(iss.rdstate() & ~std::ios::failbit);  // clear failbit
      parse0 = NULL;
      iss >> parse0; 
      // ASSERT(is);
      ASSERT(parse0 != NULL);
      parse0->label.cat = tree::label_type::root();
      parse = tree_sptree(parse0, downcase_flag);
      assert(parse != NULL);
      if (false) {
	std::cerr << "# line = " << line << std::endl;
	std::cerr << "# parser_logprob = " << parser_logprob << std::endl;
	std::cerr << "# parse0 = " << parse0 << std::endl;
      }
    }
    return is;
  }  // read()
  */
};  // sp_parse_type{}


std::ostream& operator<< (std::ostream& os, const sp_parse_type& p) {
  return os << "(" << p.nedges << " " << p.ncorrect << p.sum_logprob << " " 
	    << p.parser_logprob << " " << p.parser_rank << " " << p.parse << ")";
}  // operator<< (sp_parse_type)


// sp_parses_type is a vector of sp_parse_type
//
typedef std::vector<sp_parse_type> sp_parses_type;

// sp_sentence_type{} holds the data for a single sentence.
//
struct sp_sentence_type {
  sptree* gold;			// gold standard parse
  tree* gold0;
  size_t gold_nedges;           // number of edges in the gold parse
  float max_fscore;             // the max f-score of all parses
  sp_parses_type parses;	// vector of parses
  size_t nparses() const { return parses.size(); }
  std::string label;

  //! precrec() increments pr by the score for parse i
  //
  precrec_type& precrec(size_t i, precrec_type& pr) const {
    assert(gold != NULL);
    assert(i < nparses());
    return pr(gold, parses[i].parse);
  }  // sp_sentence_type::precrec()

  //! precrec() returns a precrec structure for parse i
  //
  precrec_type precrec(size_t i) const {
    precrec_type pr;
    return precrec(i, pr);
  }  // sp_sentence_type::precrec()

  //! f_score() returns the f-score for parse i
  //
  float f_score(size_t i) const {
    return parses[i].f_score;
  }  // sp_sentence_type::f_score()

  // default constructor
  //
  sp_sentence_type() 
    : gold(NULL), gold0(NULL), gold_nedges(0), max_fscore(0) { }

  //! clear() deletes the gold and parse trees, and sets everything to default values
  //
  void clear() {
    delete gold;
    gold = NULL;
    delete gold0;
    gold0 = NULL;

    foreach (sp_parses_type, it, parses) {
      delete it->parse;
      delete it->parse0;
    }
    parses.clear();
    label.clear();
  }

  //! destructor
  //
  ~sp_sentence_type() { 
    delete gold;
    delete gold0;
    foreach (sp_parses_type, it, parses) {
      delete it->parse;
      delete it->parse0;
    }
  }

  //! assignment operator
  //
  sp_sentence_type& operator= (const sp_sentence_type& s) {
    if (this != &s) {
      gold_nedges = s.gold_nedges;
      max_fscore = s.max_fscore;
      delete gold;
      if (s.gold != NULL)
	gold = s.gold->copy_tree();
      else
	gold = NULL;
      delete gold0;
      if (s.gold0 != NULL)
	gold0 = s.gold0->copy_tree();
      else
	gold0 = NULL;
      foreach (sp_parses_type, it, parses) {
	delete it->parse;
	delete it->parse0;
      }
      parses = s.parses;
      foreach (sp_parses_type, it, parses) {
	it->parse = it->parse->copy_tree();
	it->parse0 = it->parse0->copy_tree();
      }
    }
    return *this;
  }  // sp_sentence_type::operator=

  //! copy constructor
  //
  sp_sentence_type(const sp_sentence_type& s) 
    : gold_nedges(s.gold_nedges), max_fscore(s.max_fscore), parses(s.parses)
  {
    if (s.gold == NULL)
      gold = NULL;
    else
      gold = s.gold->copy_tree();
    foreach (sp_parses_type, it, parses) {
      it->parse = it->parse->copy_tree(); 
      it->parse0 = it->parse0->copy_tree();
    }
  }  // sp_sentence_type::sp_sentence_type()

  //! rank_elements_cmp{} is a function object used by rank_elements.
  //
  template <typename Xs, typename Cmp> 
  struct rank_elements_cmp {
    const Xs& xs;
    const Cmp& cmp;
    rank_elements_cmp(const Xs& xs, const Cmp& cmp) : xs(xs), cmp(cmp) { }
    template <typename I, typename J> bool operator() (I i, J j) const { return cmp(xs[i], xs[j]); }
  };  // rank_elements_cmp{}

  //! rank_elements() sets rs such that xs[rs[i]] <= xs[rs[i+1]], 
  //!  i.e., rs is a permutation such that xs is in sorted order.
  //!  rs should be of some suitable integer type.
  //
  template <typename Xs, typename Rs, typename Cmp> 
  inline static void
  rank_elements(const Xs& xs, Rs& rs, const Cmp& cmp) {
    rs.resize(xs.size());
    for (unsigned i = 0; i < rs.size(); ++i)
      rs[i] = i;
    std::sort(rs.begin(), rs.end(), rank_elements_cmp<Xs, Cmp>(xs, cmp));
  }   // rank_elements()

  //! set_parser_ranks() 
  //
  void set_parser_ranks() {
    typedef std::vector<Float> Fs;
    typedef std::vector<unsigned> Us;
    typedef std::map<symbol,Fs> S_Fs;
    S_Fs parser_index_logprob;
    for (unsigned i = 0; i < parses.size(); ++i) {
      cforeach (S_F, it, parses[i].parser_logprob) {
	symbol parser = it->first;
	Float prob = it->second;
	Fs& index_logprob = parser_index_logprob[parser];
	if (index_logprob.empty())
	  index_logprob.resize(parses.size(), -std::numeric_limits<Float>::infinity());
	index_logprob[i] = prob;
      }
    }
    cforeach (S_Fs, it, parser_index_logprob) {
      symbol parser = it->first;
      const Fs& index_logprob = it->second;
      Us ordering;
      rank_elements(index_logprob, ordering, std::greater<Float>());
      for (unsigned rank = 0; rank < ordering.size(); ++rank) {
	unsigned i = ordering[rank];
	assert(i < parses.size());
	if (parses[i].parser_logprob.count(parser) > 0)
	  parses[i].parser_rank[parser] = rank;
	else
	  parses[i].failedparsers.insert(parser);
      }
    }
    // calculate logcondprobs
    cforeach (S_Fs, it, parser_index_logprob) {
      symbol parser = it->first;
      const Fs& index_logprob = it->second;
      Float max_log_prob = -std::numeric_limits<Float>::infinity();
      cforeach (Fs, it1, index_logprob)
	max_log_prob = std::max(max_log_prob, *it1);
      assert(finite(max_log_prob));
      Float sumprob_maxprob = 0;
      cforeach (Fs, it1, index_logprob)
	if (*it1 != -std::numeric_limits<Float>::infinity())
	  sumprob_maxprob += exp(*it1 - max_log_prob);
      assert(finite(sumprob_maxprob));
      Float logsumprob = log(sumprob_maxprob) + max_log_prob;
      assert(index_logprob.size() == parses.size());
      for (unsigned ip = 0; ip < parses.size(); ++ip)
	if (index_logprob[ip] != -std::numeric_limits<Float>::infinity())
	  parses[ip].parser_logcondprob[parser] = index_logprob[ip] - logsumprob;
    }
  } // sp_sentence_type::set_parser_ranks()

  //! read() reads in a collection of n-best parses from is
  //! produced by Eugene Charniak's or Slav Petrov's n-best parser.
  //! It determines which to read by looking at the first character
  //! of the output.  If it begins with a '-' then we assume it is
  //! produced by Slav's parser, otherwise we assume it is produced
  //! by Eugene's parser.
  //
  std::istream& read(std::istream& is, bool downcase_flag=false) {
    clear();

    size_t nparses;
    if (is >> nparses) {
      ASSERT(is);
      // ASSERT(nparses > 0);  // Petrov parser sometimes fails to produce any parses
      
      is >> label;  // read sentence identifier
	
      if (false)
	std::cerr << "# nparses = " << nparses << ", label = " << label << std::endl;
      
      parses.resize(nparses);
      for (unsigned i = 0; i < nparses; ++i) {
	parses[i].read(is, downcase_flag);
	ASSERT(is);
	ASSERT(parses[i].parse != NULL);
      }
      set_parser_ranks();
    }

    if (false)
      for (unsigned i = 0; i < nparses; ++i)
	std::cerr << "# Parse " << i << " = " << parses[i] << std::endl;

    return is;
  }  // sp_sentence_type::read()

  //! read() reads a set of trees from parsestream and the corresponding tree
  //! from goldstream.
  //
  void read(std::istream& parsestream, std::istream& goldstream, 
	    bool downcase_flag=false) {

    typedef std::vector<symbol> symbols;

    read(parsestream, downcase_flag);

    std::string goldlabel;
    goldstream >> goldlabel;
    if (!goldstream) {
      std::cerr << HERE << "\n## Error; failed to read gold sentence header variable."
		<< std::endl;
      sp_parse_type::write_next_thousand_chars(goldstream);
      std::abort();
    }

    /*
    if ((!label.empty()) && label != goldlabel) {
      std::cerr << HERE << "\n## parse and gold labels don't match: label = " << label 
		<< ", goldlabel = " << goldlabel << std::endl;
      std::abort();
    }
    */

    goldstream >> gold0;

    if (!goldstream) {
      std::cerr << HERE << "\n## Error; failed to read expect number of gold parses."
		<< std::endl;
      sp_parse_type::write_next_thousand_chars(goldstream);
      std::abort();
    }

    if (!parsestream) {
      symbols gold0_words;
      gold0->terminals(gold0_words);
      std::cerr << HERE << "\n## Error; failed to read expected number of parses from parser output, goldlabel = " 
		<< goldlabel << std::endl
		<< "## gold0_words = " << gold0_words << std::endl;
      sp_parse_type::write_next_thousand_chars(parsestream);
      std::abort();
    }

    ASSERT(gold0 != NULL);
    gold0->label.cat = tree::label_type::root();
    strip_function_tags(gold0);
    tree* gold1 = gold0->copy_without_empties();
    // gold1->delete_unary_same_label_chains();
    gold = tree_sptree(gold1, downcase_flag);
    delete gold1;
    ASSERT(gold != NULL);
    precrec_type::edges gold_edges(gold);
    gold_nedges = gold_edges.nedges();

    symbols gold_words;
    gold->terminals(gold_words);
    
    for (size_t i = 0; i < parses.size(); ++i) {
      symbols parse_words;
      parses[i].parse->terminals(parse_words);
      if (gold_words != parse_words) {
	std::cerr << HERE << "\n## Error; gold and parse words don't match, goldlabel = " << goldlabel << std::endl
		  << "## gold_words = " << gold_words << std::endl
		  << "## parse_words = " << parse_words << std::endl
		  << "## parses.size() = " << parses.size() << std::endl
		  << "## parse[" << i << "] = " << parses[i].parse << std::endl
	          << "## gold = " << gold << std::endl
		  << "\n## next thousand chars in parsestream:" << std::endl;
	sp_parse_type::write_next_thousand_chars(parsestream);
	std::cerr << "\n## next thousand chars in goldstream:" << std::endl;
	sp_parse_type::write_next_thousand_chars(goldstream);
	std::abort();
      }
      precrec_type pr(gold_edges, parses[i].parse);
      parses[i].nedges = pr.ntest;
      parses[i].ncorrect = pr.ncommon;
      float f_score = pr.f_score();
      parses[i].f_score = f_score;
      if (f_score > max_fscore)
	max_fscore = f_score;
    }

    if (parses.empty()) {
      std::cerr << HERE << "\n## Warning; n-best parser failed to produce any parses for sentence " << goldlabel << "." << std::endl;
      symbols gold0_words;
      gold0->terminals(gold0_words);
      std::cerr << "## gold0_words = " << gold0_words << '\n' << std::endl;
    }

  }  // sp_sentence_type::read()

};  // sp_sentence_type{}


std::ostream& operator<< (std::ostream& os, const sp_sentence_type& s) {
  return os << "(" << s.gold << " " << s.gold_nedges << " " << s.max_fscore
	    << " " << s.parses << ")";
}


// sp_sentences_type is a vector of sp_sentence_type
//
typedef std::vector<sp_sentence_type> sp_sentences_type;

// sp_corpus_type{} holds an entire corpus of data.
//
struct sp_corpus_type {
  sp_sentences_type sentences;
  size_t nsentences() const { return sentences.size(); }

  // default constructor
  //
  sp_corpus_type() : sentences() { }

  // map_sentences() calls fn on every sentence.
  //
  template <typename Proc>
  static size_t map_sentences(std::istream& parsestream, std::istream& goldstream,
			      Proc& proc, bool downcase_flag=false) {
    size_t nsentences;
    goldstream >> nsentences;
    ASSERT(goldstream);
    sp_sentence_type sentence;
    for (size_t i = 0; i < nsentences; ++i) {
      sentence.read(parsestream, goldstream, downcase_flag);
      if (!parsestream) 
	std::cerr << "## Error in sp-data.h:map_sentence(), failed to read n-best parses for sentence " << i << ", nsentences = " << nsentences << std::endl;
      if (!goldstream) 
	std::cerr << "## Error in sp-data.h:map_sentence(), failed to read gold parse for sentence " << i << ", nsentences = " << nsentences << std::endl;
      ASSERT(parsestream);
      ASSERT(goldstream);
      proc(sentence);
    }
    return nsentences;
  }  // sp_corpus_type::map_sentences()

  // map_sentences_cmd() calls fn on every sentence.
  //
  template <typename Proc>
  static size_t map_sentences_cmd(const char parsecmd[], const char goldcmd[], Proc& proc, 
				  bool downcase_flag = false) {
    ipstream parsestream(parsecmd);
    if (!parsestream) {
      std::cerr << "## Error in sp-data::map_sentences_cmd(): Can't popen " << parsecmd << std::endl;
      exit(EXIT_FAILURE);
    }
    ipstream goldstream(goldcmd);
    if (!goldstream) {
      std::cerr << "## Error in sp-data::map_sentences_cmd(): Can't popen " << goldcmd << std::endl;
      std::abort();
    }
    size_t nsentences = map_sentences(parsestream, goldstream, proc, downcase_flag);
    return nsentences;
  }  // sp_corpus_type::map_sentences_cmd()

};  // sp_corpus_type{}

