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

// dp-data.h -- Read Eugene Charniak's n-best parser output
//
// Mark Johnson, 12th October 2003, modified 11th November 2005
//
// Updated to read Michael's new data files

#ifndef DP_DATA_H
#define DP_DATA_H

#include <cassert>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include "tree.h"

typedef unsigned int size_type;

typedef double Float;
#define SCANF_FLOAT_FORMAT "%lf"

// parse_type{} holds the data for a single parse.  It "owns" the
//  parse tree, and frees the parse tree when it is destroyed.
//
struct parse_type {
  Float logprob;   // log probability from Michael's parser
  size_type nedges;   // number of edges in parse
  size_type ncorrect; // number of edges correct in parse
  float f_score;   // f-score of this parse
  tree* parse;     // parse tree
  
  // default constructor
  //
  parse_type() : logprob(0), nedges(0), ncorrect(0), f_score(0), parse(NULL) { }

  // destructor
  //
  ~parse_type() { if (parse) delete parse; }

  // assignment operator
  //
  parse_type& operator= (const parse_type& dp) { 
    if (this != &dp) {
      logprob = dp.logprob;
      nedges = dp.nedges;
      ncorrect = dp.ncorrect;
      f_score = dp.f_score;
      if (parse)
	delete parse;
      parse = dp.parse->copy_tree();
    }
    return *this;
  }  // parse_type::operator=

  // copy constructor
  //
  parse_type(const parse_type& dp) 
    : logprob(dp.logprob), nedges(dp.nedges), ncorrect(dp.ncorrect), f_score(dp.f_score)
  {   
    if (dp.parse != NULL)
      parse = dp.parse->copy_tree();
    else
      parse = NULL;
  }  // parse_type::parse_type()

  // read() reads from a FILE*, returning true if the read succeeded.
  //
  bool read(FILE* fp, bool downcase_flag = false, bool ignore_trees=false) {
    if (parse != NULL) {
      delete parse;
      parse = NULL;
    }
    int nread = fscanf(fp, " " SCANF_FLOAT_FORMAT " ",
		       &logprob);
    if (nread != 1) {
      std::cerr << "## Only read " << nread << " of 1 parse header variables"
		<< std::endl;
      return false;
    }
    if (ignore_trees) {  // skip to end of line
      int c;
      while ((c = fgetc(fp)) != EOF && c != '\n')
	;
    }
    else {
      char buffer[4000];
      char* ret = fgets(buffer, 4000, fp);
      if (ret == NULL) {
	std::cerr << "## Reading parse tree failed.\n## buffer = " << buffer << std::endl;	
	return false;
      }
      parse = readtree(buffer, downcase_flag);
    }
    return true;
  }  // parse_type::read()
};  // parse_type{}


// parses_type is a vector of parse_type
//
typedef std::vector<parse_type> parses_type;

// sentence_type{} holds the data for a single sentence.
//
struct sentence_type {
  tree* gold;			// gold standard parse
  size_type gold_nedges;           // number of edges in the gold parse
  float max_fscore;             // the max f-score of all parses
  parses_type parses;		// vector of parses
  std::string label;

  size_type nparses() const { return parses.size(); }

  // precrec() increments pr by the score for parse i
  //
  precrec_type& precrec(size_type i, precrec_type& pr) const {
    assert(gold != NULL);
    assert(i < nparses());
    return pr(gold, parses[i].parse);
  }  // sentence_type::precrec()

  // precrec() returns a precrec structure for parse i
  //
  precrec_type precrec(size_type i) const {
    precrec_type pr;
    return precrec(i, pr);
  }  // sentence_type::precrec()

  //! f_score() returns the f-score for parse i
  //
  float f_score(size_type i) const {
    return parses[i].f_score;
  }  // sentence_type::f_score()

  // default constructor
  //
  sentence_type() : gold(NULL), gold_nedges(0), max_fscore(0), parses() { }

  // destructor
  //
  ~sentence_type() { if (gold) delete gold; }

  // assignment operator
  //
  sentence_type& operator= (const sentence_type& s) {
    if (this != &s) {
      gold_nedges = s.gold_nedges;
      max_fscore = s.max_fscore;
      parses = s.parses;
      if (gold != NULL)
	delete gold;
      if (s.gold != NULL)
	gold = s.gold->copy_tree();
      else
	gold = NULL;
    }
    return *this;
  }  // sentence_type::operator=

  // copy constructor
  //
  sentence_type(const sentence_type& s) 
    : gold_nedges(s.gold_nedges), max_fscore(s.max_fscore), parses(s.parses) 
  {
    if (s.gold == NULL)
      gold = NULL;
    else
      gold = s.gold->copy_tree();
  }  // sentence_type::sentence_type()

  // read() returns true if read was successful.
  //
  bool read(FILE* parsefp, FILE* goldfp, bool downcase_flag = false, 
	    bool ignore_trees=false) {

    delete gold;
    gold = NULL;

    size_type nparses;
    char parselabel[256];
    int nread = fscanf(parsefp, " %u %255s ", &nparses, parselabel);
    if (nread != 2) {
      std::cerr << "## Fatal error: Only read " << nread << " of 2 sentence header variables."
		<< std::endl;
      std::cerr << "Next 1000 characters:" << std::endl;
      for (size_type k = 0; k < 1000; ++k) {
	int chr = getc(parsefp);
	if (chr != EOF)
	  std::cerr << char(chr);
	else {
	  std::cerr << "\n--EOF--";
	  break;
	}
      }
      std::cerr << std::endl;
      return false;
    }

    char goldlabel[256];
    nread = fscanf(goldfp, " %255s ", goldlabel);
    if (nread != 1) {
      std::cerr << "## Fatal error: Only read " << nread << " of 1 gold sentence header variables."
		<< std::endl;
      return false;
    }

    if (strcmp(parselabel, goldlabel)) {
      std::cerr << "## Fatal error: parselabel = " << parselabel << ", goldlabel = " << goldlabel
		<< std::endl;
      return false;
    }

    label = parselabel;

    if (ignore_trees) {  // skip to end of line
      int c;
      while ((c = fgetc(goldfp)) != EOF && c != '\n')
	;
    }
    else {
      char buffer[4000];
      char* ret = fgets(buffer, 4000, goldfp);
      if (ret == NULL) {
	std::cerr << "## Reading gold tree failed.\n## buffer = " << buffer << std::endl;
	return false;
      }
      gold = readtree(buffer, downcase_flag);
    }

    assert(gold != NULL);
    precrec_type::edges gold_edges(gold);
    gold_nedges = gold_edges.nedges();

    parses.resize(nparses);
    for (size_type i = 0; i < nparses; ++i) 
      if (parses[i].read(parsefp, downcase_flag, ignore_trees)) {
	precrec_type pr(gold_edges, parses[i].parse);
	parses[i].nedges = pr.ntest;
	parses[i].ncorrect = pr.ncommon;
	float f_score = pr.f_score();
	parses[i].f_score = f_score;
	if (f_score > max_fscore)
	  max_fscore = f_score;
      }
      else {
	std::cerr << "## Reading parse tree " << i << " failed." << std::endl;
	return false;
      }

    return true;
  }  // sentence_type::read()
};  // sentence_type{}


// sentences_type is a vector of sentence_type
//
typedef std::vector<sentence_type> sentences_type;

// corpus_type{} holds an entire corpus of data.
//
struct corpus_type {
  sentences_type sentences;
  size_type nsentences() const { return sentences.size(); }

  // default constructor
  //
  corpus_type() : sentences() { }

  // constructor from a file pointer
  //
  corpus_type(FILE* parsefp, FILE* goldfp, bool downcase_flag = false, bool ignore_trees=false) 
    : sentences() 
  {
    bool status = read(parsefp, goldfp, downcase_flag, ignore_trees);
    assert(status == true);
  }  // corpus_type::corpus_type()

  // constructor from a bzip2'd filename
  //
  corpus_type(const char parsefilename[], const char goldfilename[],
	      bool downcase_flag = false, bool ignore_trees=false) 
    : sentences() 
  {
    FILE* parsefp = popen_decompress(parsefilename);
    FILE* goldfp = popen_decompress(goldfilename);
    bool successful_read = read(parsefp, goldfp, downcase_flag, ignore_trees);
    assert(successful_read);
    pclose(parsefp);
    pclose(goldfp);
  }  // corpus_type::corpus_type()

  // popen_decompress() returns a popen FILE* to filename.
  //
  inline static FILE* popen_decompress(const char filename[]) {
    std::string command("bzcat ");
    command += filename;
    FILE* fp = popen(command.c_str(), "r");
    if (fp == NULL) {
      std::cerr << "## Error: could not popen command " << filename << std::endl;
      exit(EXIT_FAILURE);
    }
    return fp;
  }  // popen_file()

  // read() returns true if the corpus was successfully read.
  //
  bool read(FILE* parsefp, FILE* goldfp, bool downcase_flag = false, bool ignore_trees=false) {
    size_type nsentences;
    int nread = fscanf(goldfp, " %u ", &nsentences);
    if (nread != 1) {
      std::cerr << "## Failed to read number of sentences at start of file." << std::endl;
      return false;
    }
    sentences.resize(nsentences);
    for (size_type i = 0; i < nsentences; ++i)
      if (!sentences[i].read(parsefp, goldfp, downcase_flag, ignore_trees)) {
	std::cerr << "## Reading sentence tree " << i << " failed." << std::endl;	
	return false;
      }
    return true;
  }  // corpus_type::read()

  // map_sentences() calls fn on every sentence.
  //
  template <typename Proc>
  static size_type map_sentences(FILE* parsefp, FILE* goldfp, Proc& proc, 
			      bool downcase_flag = false, bool ignore_trees=false) {
    size_type nsentences;
    int nread = fscanf(goldfp, " %u ", &nsentences);
    if (nread != 1) {
      std::cerr << "## Failed to read number of sentences at start of file." << std::endl;
      return 0;
    }
    sentence_type sentence;
    for (size_type i = 0; i < nsentences; ++i) {
      if (!sentence.read(parsefp, goldfp, downcase_flag, ignore_trees)) {
	std::cerr << "## Reading sentence tree " << i << " failed." << std::endl;	
	return 0;
      }
      proc(sentence);
    }
    return nsentences;
  }  // corpus_type::map_sentences()

  // map_sentences() calls fn on every sentence.
  //
  template <typename Proc>
  static size_type map_sentences(const char parsefilename[], const char goldfilename[], Proc& proc, 
			      bool downcase_flag = false, bool ignore_trees=false) {
    FILE* parsefp = popen_decompress(parsefilename);
    FILE* goldfp = popen_decompress(goldfilename);
    size_type nsentences = map_sentences(parsefp, goldfp, proc, downcase_flag, ignore_trees);
    pclose(parsefp);
    pclose(goldfp);
    return nsentences;
  }  // corpus_type::map_sentences()

  // map_sentences() calls fn on every sentence.
  //
  template <typename Proc>
  static size_type map_sentences_cmd(const char parsecmd[], const char goldcmd[], Proc& proc, 
			      bool downcase_flag = false, bool ignore_trees=false) {
    FILE* parsefp = popen(parsecmd, "r");
    FILE* goldfp = popen(goldcmd, "r");
    size_type nsentences = map_sentences(parsefp, goldfp, proc, downcase_flag, ignore_trees);
    pclose(parsefp);
    pclose(goldfp);
    return nsentences;
  }  // corpus_type::map_sentences()

};  // corpus_type{}

#endif // DP_DATA_H
