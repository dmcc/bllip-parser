// best-parses.cc
//
// Mark Johnson, 17th March 2004

const char usage[] =
"best-parses treefile.bz2 < best-indices.txt\n"
"\n"
"writes out one line per sentence of the form\n"
"\n"
"\tdelta-fscore sentence-no max-weight-parse max-weight-fscore best-parse best-fscore gold-parse\n"
"\n"
"where:\n\n"
" max-weight-parse is a parse with the highest weight for this sentence,\n"
" max-weight-fscore is its f-score,\n"
" best-parse is a parse with the highest f-score for this sentence,\n"
" best-fscore is its f-score,\n"
" gold-parse is the Treebank parse for this sentence.\n"
"\n"
"treefile.bz2 should be the tree files in dp-data.h format, and\n"
"best-indices.txt should be a list of lines in the format produced\n"
"by best-indices.cc\n";

#include "custom_allocator.h"

#include <cassert>
#include <iostream>
#include <string>

#include "dp-data.h"
#include "tree.h"

struct find_best {
  
  size_t nsentences;
  precrec_type precrec;

  find_best() : nsentences(0) { }

  void operator()(const sentence_type& s) {
    ++nsentences;
    double max_weight_fscore, best_fscore;
    int max_weight_index, best_index;
    int nparses;
    std::cin >> max_weight_index >> max_weight_fscore >> best_index >> best_fscore >> nparses;
    if (!std::cin) 
      std::cerr << "## Error reading best-index data for sentence " 
		<< nsentences << " from std::cin\n" << std::endl;
    if (size_t(nparses) != s.nparses()) {
      std::cerr << "## Error: best-index data says sentence "
		<< nsentences << " has " << nparses 
		<< " but treefile says it has " << s.nparses()
		<< " parses." << std::endl;
      exit(EXIT_FAILURE);
    }
    assert((best_index >= 0) == (max_weight_index >= 0));
    if (best_index >= 0) {
      assert(size_t(best_index) < s.parses.size());
      assert(size_t(max_weight_index) < s.parses.size());
      std::cout << best_fscore - max_weight_fscore << '\t' << nsentences << '\t'
		<< s.parses[max_weight_index].parse << '\t' << max_weight_fscore << '\t' 
		<< s.parses[best_index].parse << '\t' << best_fscore << '\t'
		<< s.gold << '\n';
      precrec(s.gold, s.parses[max_weight_index].parse);
    }
  }  // find_best::operator()

};  // find_best{}

int main(int argc, char* argv[])
{
  if (argc != 2) {
    std::cerr << usage << std::endl;
    exit(EXIT_FAILURE);
  }
  
  find_best fb;
  corpus_type::map_sentences(argv[1], fb);

  std::cerr << "## read " << fb.nsentences << " sentences, " << fb.precrec << std::endl;
}
