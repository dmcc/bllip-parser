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

// sp-multidata.h -- Read the Charniak and Petrov n-best parse data files
//
// Mark Johnson, 24th November 2009
//
// Reads Charniak and Petrov's n-best parser output

#pragma once

#include <cassert>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "popen.h"
#include "sptree.h"
#include "tree.h"

typedef double Float;
typedef std::vector<Float> Floats;

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
  Floats logprobs;   // log probabilities from parsers
  Floats logcondprobs;
  Floats logrelprobs;
  size_t nedges;
  size_t ncorrect;
  float f_score;   // f-score of this parse
  sptree* parse;
  tree* parse0;

  // default constructor
  //
  sp_parse_type() : nedges(0), ncorrect(0), f_score(0), parse(NULL), parse0(NULL) { }

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
    std::string line;
    getline(is, line);  // skip end of line
    if (!is)
      return is;
    if (getline(is, line)) {
      std::istringstream iss(line);
      Float logprob;
      while (iss >> logprob) {
	assert(finite(logprob));
	logprobs.push_back(logprob);
      }
      is >> parse0;
      ASSERT(is);
      ASSERT(parse0 != NULL);
      parse0->label.cat = tree::label_type::root();
      parse = tree_sptree(parse0, downcase_flag);
      assert(parse != NULL);
      if (false) {
	std::cerr << "# line = " << line << std::endl;
	std::cerr << "# logprobs = " << logprobs << std::endl;
	std::cerr << "# parse0 = " << parse0 << std::endl;
      }
    }
    return is;
  }  // read_nbest()

};  // sp_parse_type{}


std::ostream& operator<< (std::ostream& os, const sp_parse_type& p) {
  return os << "(" << p.logprobs << " " << p.logcondprobs << " " << p.nedges 
	    << " " << p.ncorrect << " " << p.parse << ")";
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
  Floats logsumprobs;
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


  //! set_logcondprob() sets the log cond prob 
  //
  void set_logcondprob() {
    // calculate logcondprob and logsumprob (log(sum(P(parse)))) in a way 
    //  that avoids overflow/underflow
    
    if (false) {
      std::cerr << "# parses.size() = " << parses.size() << std::endl;
      std::cerr << "# parse0 = " << parses[0].parse0 << std::endl;
      for (unsigned i = 0; i < parses.size(); ++i)
	std::cerr << "# parses[" << i << "].logprobs = " << parses[i].logprobs << std::endl;
      std::cerr << std::endl;
    }

    if (parses.size() > 0) {
      size_t nlogprobs = parses[0].logprobs.size();
      logsumprobs.resize(nlogprobs);
      for (unsigned i = 0; i < parses.size(); ++i) {
	assert(nlogprobs == parses[i].logprobs.size());
	parses[i].logcondprobs.resize(nlogprobs);
	parses[i].logrelprobs.resize(nlogprobs);
      }
      for (size_t ip = 0; ip < nlogprobs; ++ip) {
	Float max_log_prob = parses[0].logprobs[ip];
	Float min_log_prob = parses[0].logprobs[ip];
	for (size_t i = 1; i < parses.size(); ++i) 
	  if (parses[i].logprobs[ip] != 0) {
	    if (max_log_prob == 0) 
	      min_log_prob = max_log_prob = parses[i].logprobs[ip];
	    else {
	      max_log_prob = std::max(max_log_prob, parses[i].logprobs[ip]);
	      min_log_prob = std::min(min_log_prob, parses[i].logprobs[ip]);
	    }
	  }
	assert(finite(max_log_prob));
	// assert(max_log_prob != 0); // not true for Petrov parses, since some sentence have no parses
	Float sumprob_maxprob = 0;
	for (size_t i = 0; i < parses.size(); ++i)
	  if (parses[i].logprobs[ip] != 0)
	    sumprob_maxprob += exp(parses[i].logprobs[ip] - max_log_prob);
	assert(finite(sumprob_maxprob));
	logsumprobs[ip] = log(sumprob_maxprob) + max_log_prob;
	assert(finite(logsumprobs[ip]));
	for (size_t i = 0; i < parses.size(); ++i) 
	  if (parses[i].logprobs[ip] != 0) {
	    parses[i].logcondprobs[ip] = parses[i].logprobs[ip] - logsumprobs[ip];
	    assert(finite(parses[i].logcondprobs[ip]));
	    parses[i].logrelprobs[ip] = parses[i].logprobs[ip] - min_log_prob;
	  }
      }
    }
  }  // sp_sentence_type::set_logcondprob()

  //! read() reads in a collection of n-best parses from is
  //! produced by Eugene Charniak's or Slav Petrov's n-best parser.
  //! It determines which to read by looking at the first character
  //! of the output.  If it begins with a '-' then we assume it is
  //! produced by Slav's parser, otherwise we assume it is produced
  //! by Eugene's parser.
  //
  std::istream& read(std::istream& is, bool downcase_flag=false) {
    clear();

    char c;
    unsigned nblanklines = 0;
    while (is.get(c) && isspace(c))
      if (c == '\n')
	++nblanklines;
    
    if (!is)
      return is;

    is.unget();
    
    if (c == '-') { // Petrov-style Berkeley parser output
      if (nblanklines == 0) {
	std::string line;
	while (getline(is, line)) {
	  if (line.find_first_not_of(" \n\r\t") == std::string::npos)
	    break;
	  if (line.compare(0, 9, "-Infinity") == 0)  // skip horribly improbable parses
	    continue;
	  std::istringstream iss(line);
	  ASSERT(iss);
	  parses.resize(parses.size()+1);
	  parses[parses.size()-1].read(iss, downcase_flag);
	  if (!iss) {
	    std::cerr << HERE << "\n## Error: failed to read n-best parser output." << std::endl
		      << "## line = " << line << std::endl;
	    return is;
	  }
	  ASSERT(parses[parses.size()-1].parse != NULL);
	}
	if (!parses.empty())
	  set_logcondprob();
      }
      else {
	while (--nblanklines > 0)
	  is.putback('\n');
      }
    }
    else { // Charniak-style parser output
      // std::cerr << HERE << ", c = '" << c << "'" << std::endl;
      // sp_parse_type::write_next_thousand_chars(is);
      // std::abort();
      size_t nparses;
      if (is >> nparses) {
	ASSERT(is);
	ASSERT(nparses > 0);
	
	is >> label;  // read sentence identifier

	if (false)
	  std::cerr << "# nparses = " << nparses << ", label = " << label << std::endl;

	parses.resize(nparses);
	for (size_t i = 0; i < nparses; ++i) {
	  parses[i].read(is, downcase_flag);
	  ASSERT(is);
	  ASSERT(parses[i].parse != NULL);
	}
	set_logcondprob();
      }
    }
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

    if ((!label.empty()) && label != goldlabel) {
      std::cerr << HERE << "\n## parse and gold labels don't match: label = " << label 
		<< ", goldlabel = " << goldlabel << std::endl;
      std::abort();
    }

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
	    << " " << s.parses << " " << s.logsumprobs << ")";
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

