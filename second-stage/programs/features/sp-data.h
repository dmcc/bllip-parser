// sp-data.h -- Read Mark Johnson's discriminative parsing data files
//
// Mark Johnson, 16th October 2003, last modified 11th November 2005
//
// Updated to read Michael's new data files, and to store sptrees.
//
// This file is incompatible with dp-data.h, as it gives different
// declarations for the typenames sp_parse_type, sp_sentence_type, etc.

#ifndef SP_DATA_H
#define SP_DATA_H

#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include "sptree.h"
#include "tree.h"

#define BUFSIZE 8000   // size of line buffer

typedef double Float;
#define SCANF_FLOAT_FORMAT "%lf"

// parse_type{} holds the data for a single parse.  It has a pointer
// to the parse tree, but someone else must free it when it is deleted!
//
struct sp_parse_type {
  Float logprob;   // log probability from parser
  Float logcondprob;
  size_t nedges;
  size_t ncorrect;
  float f_score;   // f-score of this parse
  sptree* parse;
  tree* parse0;

  // default constructor
  //
  sp_parse_type() : logprob(0), logcondprob(0), nedges(0), ncorrect(0),
		    f_score(0), parse(NULL), parse0(NULL) { }

  // read() reads from a FILE*, returning true if the read succeeded.
  //
  bool read(FILE* fp, bool downcase_flag=false) {

    int nread = fscanf(fp, " " SCANF_FLOAT_FORMAT " ", &logprob);
    if (nread != 1) {
      std::cerr << "## Only read " << nread << " of 1 parse header variables"
		<< std::endl;
      return false;
    }

    if (logprob > 0) {
      std::cerr << "## Positive logprob = " << logprob  << std::endl;
      write_next_thousand_chars(fp);
      return false;
    }

    {
      char buffer[BUFSIZE];
      char* ret = fgets(buffer, BUFSIZE, fp);
      if (ret == NULL) {
	std::cerr << "## Reading parse tree failed.\n## buffer = " << buffer << std::endl;	
	return false;
      }
      if (buffer[strlen(buffer)-1] != '\n') {
	std::cerr << "## Parse tree buffer not terminated by '\\n'"
	          << " (buffer probably too small, increase BUFSIZE in sp-data.h)\n"
		  << "## buffer = " << buffer << std::endl;
	return false;
      }
      parse0 = readtree(buffer);
      assert(parse0 != NULL);
      parse0->label.cat = tree::label_type::root();
      parse = tree_sptree(parse0, downcase_flag);
      // std::cerr << "parse0 = " << parse0 << ", parse = " << parse << std::endl;
    }
    return true;
  }  // sp_parse_type::read()


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

  //! read_ec_nbest() reads Eugene's n-best parser output
  //
  std::istream& read_ec_nbest(std::istream& is, bool downcase_flag=false) {
    if (is >> logprob >> parse0) {
      if (!finite(logprob)) {
	std::cerr << "## sp-data.h error reading n-best parses: "
		  << "logprob read from treebank is not finite, logprob = "
		  << logprob << ", parse0 = " << parse0 << std::endl;
	exit(EXIT_FAILURE);
      }      
      assert(parse0 != NULL);
      parse0->label.cat = tree::label_type::root();
      parse = tree_sptree(parse0, downcase_flag);
      assert(parse != NULL);
    }
    return is;
  }  // read_ec_nbest()
};  // sp_parse_type{}


std::ostream& operator<< (std::ostream& os, const sp_parse_type& p) {
  return os << "(" << p.logprob << " " << p.logcondprob << " " << p.nedges 
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
  Float logsumprob;
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
    : gold(NULL), gold0(NULL), gold_nedges(0), max_fscore(0), logsumprob(0) { }

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

    if (parses.size() > 0) {
      Float logmaxprob = parses[0].logprob;
      for (size_t i = 1; i < parses.size(); ++i)
	logmaxprob = std::max(logmaxprob, parses[i].logprob);
      assert(finite(logmaxprob));
      Float sumprob_maxprob = 0;
      for (size_t i = 0; i < parses.size(); ++i)
	sumprob_maxprob += exp(parses[i].logprob - logmaxprob);
      assert(finite(sumprob_maxprob));
      logsumprob = log(sumprob_maxprob) + logmaxprob;
      assert(finite(logsumprob));

      for (size_t i = 0; i < parses.size(); ++i) {
	parses[i].logcondprob = parses[i].logprob - logsumprob;
	assert(finite(parses[i].logcondprob));
      }
    }
  }  // sp_sentence_type::set_logcondprob()


  //! read() returns true if read was successful.
  //
  bool read(FILE* parsefp, FILE* goldfp, bool downcase_flag=false) {
    delete gold;
    gold = NULL;
    delete gold0;
    gold0 = NULL;
    foreach (sp_parses_type, it, parses) {
      delete it->parse;
      delete it->parse0;
    }
    parses.clear();
    unsigned int nparses;
    char parselabel[256];
    int nread = fscanf(parsefp, " %u %255s ", &nparses, parselabel);
    if (nread != 2) {
      std::cerr << "## Fatal error: Only read " << nread << " of 2 parse sentence header variables."
		<< std::endl;
      sp_parse_type::write_next_thousand_chars(parsefp);
      return false;
    }

    char goldlabel[256];
    nread = fscanf(goldfp, " %255s ", goldlabel);
    if (nread != 1) {
      std::cerr << "## Fatal error: Only read " << nread << " of 1 gold sentence header variables."
		<< std::endl;
      sp_parse_type::write_next_thousand_chars(goldfp);
      return false;
    }

    if (strcmp(parselabel, goldlabel)) {
      std::cerr << "## Fatal error: parselabel = " << parselabel << ", goldlabel = " << goldlabel
		<< std::endl;
      return false;
    }

    label = parselabel;

    {
      char buffer[BUFSIZE];
      char* ret = fgets(buffer, BUFSIZE, goldfp);
      if (ret == NULL) {
	std::cerr << "## Reading gold tree failed.\n## buffer = " << buffer << std::endl;
	return false;
      }
      if (buffer[strlen(buffer)-1] != '\n') {
	std::cerr << "## Gold tree buffer not terminated by '\\n'"
	          << " (buffer probably too small, increase BUFSIZE in sp-data.h)" << std::endl
		  << "## buffer = " << buffer << std::endl;
	return false;
      }
      gold0 = readtree(buffer);
      assert(gold0 != NULL);
      gold0->label.cat = tree::label_type::root();
      tree* gold1 = gold0->copy_without_empties();
      // gold1->delete_unary_same_label_chains();
      gold = tree_sptree(gold1, downcase_flag);
      delete gold1;
    }

    assert(gold != NULL);
    precrec_type::edges gold_edges(gold);
    gold_nedges = gold_edges.nedges();

    typedef std::vector<symbol> symbols;
    symbols gold_words;
    gold->terminals(gold_words);
    
    parses.resize(nparses);
    for (size_t i = 0; i < nparses; ++i) 
      if (parses[i].read(parsefp, downcase_flag)) {
	symbols parse_words;
	parses[i].parse->terminals(parse_words);
	if (gold_words != parse_words) {
	  std::cerr << "## Error on example " << label 
		    << ", gold_words = " << gold_words 
		    << ", parse_words = " << parse_words
		    << std::endl;
	  exit(EXIT_FAILURE);
	}
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

    set_logcondprob();

    return true;
  }  // sp_sentence_type::read()


  //! read_ec_nbest() reads in a collection of n-best parses 
  //! produced by Eugene Charniak's n-best parser.
  //
  std::istream& read_ec_nbest(std::istream& is, bool downcase_flag=false) {
    delete gold;
    gold = NULL;
    delete gold0;
    gold0 = NULL;

    foreach (sp_parses_type, it, parses) {
      delete it->parse;
      delete it->parse0;
    }
    parses.clear();

    size_t dummy;
    if (!(is >> dummy))
      return is;

    size_t nparses;
    if (is >> nparses) {
      assert(nparses > 0);
      parses.resize(nparses);
      for (size_t i = 0; i < nparses; ++i) {
	parses[i].read_ec_nbest(is, downcase_flag);
	assert(is);
	assert(parses[i].parse != NULL);
      }
      set_logcondprob();
    }
    return is;
  }  // sp_sentence_type::read_ec_nbest()

  //! read_ec_nbest_15aug05() reads in a collection of n-best parses 
  //! produced by Eugene Charniak's n-best parser, in the format
  //! he made up on the 15th August 2005.
  //
  std::istream& read_ec_nbest_15aug05(std::istream& is, bool downcase_flag=false) {
    delete gold;
    gold = NULL;
    delete gold0;
    gold0 = NULL;

    foreach (sp_parses_type, it, parses) {
      delete it->parse;
      delete it->parse0;
    }
    parses.clear();

    size_t nparses;
    if (is >> nparses) {
      assert(nparses > 0);

      is >> label;  // read sentence identifier

      parses.resize(nparses);
      for (size_t i = 0; i < nparses; ++i) {
	parses[i].read_ec_nbest(is, downcase_flag);
	assert(is);
	assert(parses[i].parse != NULL);
      }
      set_logcondprob();
    }
    return is;
  }  // sp_sentence_type::read_ec_nbest_15aug05()
};  // sp_sentence_type{}


std::ostream& operator<< (std::ostream& os, const sp_sentence_type& s) {
  return os << "(" << s.gold << " " << s.gold_nedges << " " << s.max_fscore
	    << " " << s.parses << " " << s.logsumprob << ")";
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

  // constructor from a file pointer
  //
  sp_corpus_type(FILE* parsefp, FILE* goldfp, bool downcase_flag=false) 
    : sentences() {
    bool status = read(parsefp, goldfp, downcase_flag);
    assert(status == true);
  }  // sp_corpus_type::sp_corpus_type()

  // constructor from a bzip2'd filename
  //
  sp_corpus_type(const char parsefname[], const char goldfname[],
		 bool downcase_flag = false) : sentences() {
    FILE* parsefp = popen_decompress(parsefname);
    FILE* goldfp = popen_decompress(goldfname);
    bool successful_read = read(parsefp, goldfp, downcase_flag);
    assert(successful_read);
    pclose(goldfp);
    pclose(parsefp);
  }  // sp_corpus_type::sp_corpus_type()

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
  bool read(FILE* parsefp, FILE* goldfp, bool downcase_flag=false) {
    unsigned int nsentences;
    int nread = fscanf(goldfp, " %u ", &nsentences);
    if (nread != 1) {
      std::cerr << "## Failed to read number of sentences at start of file." << std::endl;
      return false;
    }
    sentences.resize(nsentences);
    for (size_t i = 0; i < nsentences; ++i)
      if (!sentences[i].read(parsefp, goldfp, downcase_flag)) {
	std::cerr << "## Reading sentence tree " << i << " failed." << std::endl;	
	return false;
      }
    return true;
  }  // sp_corpus_type::read()

  // map_sentences() calls fn on every sentence.
  //
  template <typename Proc>
  static size_t map_sentences(FILE* parsefp, FILE* goldfp,
			      Proc& proc, bool downcase_flag=false) {
    unsigned int nsentences;
    int nread = fscanf(goldfp, " %u ", &nsentences);
    if (nread != 1) {
      std::cerr << "## Failed to read number of sentences at start of file." << std::endl;
      return 0;
    }
    sp_sentence_type sentence;
    for (size_t i = 0; i < nsentences; ++i) {
      if (!sentence.read(parsefp, goldfp, downcase_flag)) {
	std::cerr << "## Reading sentence tree " << i << " failed." << std::endl;	
	return 0;
      }
      proc(sentence);
    }
    return nsentences;
  }  // sp_corpus_type::map_sentences()

  // map_sentences_cmd() calls fn on every sentence.
  //
  template <typename Proc>
  static size_t map_sentences_cmd(const char parsecmd[], const char goldcmd[], Proc& proc, 
				  bool downcase_flag = false) {
    FILE* parsefp = popen(parsecmd, "r");
    FILE* goldfp = popen(goldcmd, "r");
    size_t nsentences = map_sentences(parsefp, goldfp, proc, downcase_flag);
    pclose(goldfp);
    pclose(parsefp);
    return nsentences;
  }  // sp_corpus_type::map_sentences_cmd()

  // map_sentences() calls fn on every sentence.
  //
  template <typename Proc>
  static size_t map_sentences(const char parsefname[], const char goldfname[], Proc& proc, 
			      bool downcase_flag = false) {
    FILE* parsefp = popen_decompress(parsefname);
    FILE* goldfp = popen_decompress(goldfname);
    size_t nsentences = map_sentences(parsefp, goldfp, proc, downcase_flag);
    pclose(goldfp);
    pclose(parsefp);
    return nsentences;
  }  // sp_corpus_type::map_sentences()
};  // sp_corpus_type{}

#endif // SP_DATA_H
