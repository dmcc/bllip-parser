// copy-trees-ss.cc
//
// (c) Mark Johnson, 29th October 2004, last modified 10th May 2005

static const char usage[] =
"copy-trees-ss filename-base wsj-tree-filename-cmd < massi-ss-file\n"
"\n"
"reads Massi's supersense file from stdin, then reads the wsj file arguments\n"
"and writes out the trees in these files with their supersense labels";

#include "custom_allocator.h"

#include <cstdio>
#include <cstdlib>
#include <ext/hash_map>
#include <ext/stdio_filebuf.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>

#include "sym.h"
#include "tree.h"
#include "utility.h"

typedef std::pair<int,int> II;
typedef std::pair<symbol,II> SII;

struct SSt {
  symbol word;
  symbol supersense;
  symbol lemma;
  symbol POS;

  SSt(symbol word, symbol supersense, symbol lemma, symbol POS) :
    word(word), supersense(supersense), lemma(lemma), POS(POS) { }

  SSt() : word(symbol::undefined()), supersense(symbol::undefined()),
	  lemma(symbol::undefined()), POS(symbol::undefined()) { }

};  // SSt{}

std::ostream& operator<< (std::ostream& os, const SSt& sst) {
  return os << '(' << sst.word << ' ' << sst.supersense 
	    << ' ' << sst.lemma << ' ' << sst.POS << ')';
}

typedef ext::hash_map<SII,SSt> SII_SSt;

//! read_massi() reads lines from Massi's supersense data file
//! and stores it in a hash map that maps from filenames and
//! positions to supersenses.
//
void read_massi(std::istream& in, SII_SSt& pos_ss) {
  int nline = 0;
  std::string line;
  while (std::getline(in, line)) {
    ++nline;
    std::istringstream iss(line);
    std::string prefix, filename, word, supersense, lemma, POS;
    int sentenceno, wordno;
    if (!(iss >> prefix >> filename >> sentenceno >> wordno >> word >> supersense >> lemma >> POS)
	|| prefix != "wj") {
      std::cerr << "## Error in supersense data, line " << nline << ":" << line << std::endl;
      exit(EXIT_FAILURE);
    }
    filename.erase(0, 4);    // delete "wsj/"
    std::string::size_type pos = supersense.find('.');
    if (pos == std::string::npos) {
      std::cerr << "## Error: can't parse supersense = " << supersense << ", line = " << line << std::endl;
      exit(EXIT_FAILURE);
    }
    supersense.erase(0, pos+1);  // delete "noun."
    pos_ss[SII(filename, II(sentenceno,wordno))] = SSt(word, supersense, lemma, POS);
  }
} // read_massi()


//! annotate_tree() annotates the preterminal nodes of a tree with supersenses, and
//! returns the number of preterminals in the tree.
//
int annotate_tree(const SII_SSt& pos_ss, symbol infn, tree* tp, 
		  int treeno, int wordno = 0)
{
  static const symbol NONE("-NONE-");
  
  if (tp == NULL)
    return wordno;
  else if (tp->is_preterminal()) {
    if (tp->label.cat != NONE) {
      SII_SSt::const_iterator it = pos_ss.find(SII(infn, II(treeno,wordno)));
      if (it != pos_ss.end()) {
	std::string newlabel = tp->label.cat.string_reference();
	(newlabel += '.') += it->second.lemma.string_reference();
	(newlabel += '.') += it->second.supersense.string_reference();
	tp->label.cat = symbol(newlabel);
      }
      ++wordno;
    }
    return annotate_tree(pos_ss, infn, tp->next, treeno, wordno);
  }
  else // tp->is_nonterminal()
    return annotate_tree(pos_ss, infn, tp->next, treeno,
			 annotate_tree(pos_ss, infn, tp->child, treeno, wordno));
}

//! annote_file() annotates the trees in the treebank file infile with supersenses
//! and writes then to outfile.
//
void annotate_file(const SII_SSt& pos_ss, std::istream& is, symbol infn, 
		   std::ostream& os)
{
  static const symbol top("TOP");
  tree* tp;
  int treeno = 0;
  while (is >> tp) {
    tp->label.cat = top;
    annotate_tree(pos_ss, infn, tp, treeno++);
    os << tp << std::endl;
    delete tp;
  }
}


int main(int argc, char **argv)
{
  if (argc != 3) {
    std::cerr <<  usage << std::endl;
    exit(EXIT_FAILURE);
  }

  SII_SSt pos_ss;
  read_massi(std::cin, pos_ss);
  std::cerr << "# Read " << pos_ss.size() << " supersenses." << std::endl;

  FILE* fp = popen(argv[2], "r");
  if (fp == NULL) {
    std::cerr << "## Error: can't popen command \"" << argv[2] << "\"." << std::endl;
    exit(EXIT_FAILURE);
  }
  ext::stdio_filebuf<char> fb(fp, std::ios_base::in);
  std::istream is(&fb);
  std::string fn;
  while (is >> fn) {
    std::string infile(argv[1]);
    (infile += '/') += fn;
    std::ifstream is(infile.c_str());
    assert(is);
    annotate_file(pos_ss, is, fn, std::cout);
  }
  pclose(fp);
} // main()
