// main.cc
//
// Mark Johnson 16th August 2003, modified 11th November 2005

#include "custom_allocator.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <utility>
#include <vector>

#include "dp-data.h"
#include "histogram.h"
#include "sym.h"
#include "tree.h"
#include "utility.h"

struct beam_sizer_type {
  size_t nsentences;
  size_t nparses;
  precrec_type first_prs, maxprob_prs, best_prs;
  std::vector<precrec_type> precrecs;
  std::vector<size_t> nparses_counts;
  std::map<size_t,size_t> bestparseindex_count;
  std::map<Float,size_t> logrelprobbest_count;
  std::map<Float,size_t> logcondprobbest_count;
  std::vector<precrec_type> lrpt_prs;
  std::vector<size_t> lrpt_count;
  std::vector<precrec_type> lcpt_prs;
  std::vector<size_t> lcpt_count;

  beam_sizer_type(size_t n=100) 
    : nsentences(0), nparses(0), precrecs(n), nparses_counts(n), 
      lrpt_prs(51), lrpt_count(51), lcpt_prs(51), lcpt_count(51) { }

  void operator()(sentence_type& s) { 

    ++nsentences;
    nparses += s.parses.size();

    if (s.parses.empty())
      return;

    typedef std::pair<Float,precrec_type> FPR;
    std::vector<FPR> fprs;
    fprs.reserve(s.parses.size()); 
    precrec_type::edges gold_edges(s.gold);

    Float logmaxprob = s.parses[0].logprob;
    for (size_t i = 1; i < s.nparses(); ++i)
      if (logmaxprob < s.parses[i].logprob)
	logmaxprob = s.parses[i].logprob;

    Float sumprob_maxprob = 0; 
    for (size_t i = 0; i < s.nparses(); ++i)
      sumprob_maxprob += exp(s.parses[i].logprob-logmaxprob);

    Float logsumprob = log(sumprob_maxprob) + logmaxprob;

    for (size_t i = 0; i < s.nparses(); ++i) {
      precrec_type::edges parse_edges(s.parses[i].parse);
      fprs.push_back(std::make_pair(s.parses[i].logprob, 
				    precrec_type(gold_edges, parse_edges)));
    }

    first_prs += fprs[0].second;

    // sort by logprob, best first
    std::stable_sort(fprs.begin(), fprs.end(), first_greaterthan());
    assert(fprs.size() == s.nparses());

    maxprob_prs += fprs[0].second;

    // find best parse in sorted list

    size_t bestparseindex = 0;
    precrec_type best = fprs[0].second;
    Float bestlogprob = fprs[0].first;
    Float bestlogcondprob = bestlogprob - logsumprob;

    for (size_t i = 0; i < precrecs.size(); ++i) {
      if (i < s.nparses()) {
	++nparses_counts[i];
	if (best < fprs[i].second) {
	  best = fprs[i].second;
	  bestparseindex = i;
	  bestlogprob = fprs[i].first;
	  bestlogcondprob = bestlogprob - logsumprob;
	}
      }
      precrecs[i] += best;
    }

    best_prs += best;
    ++bestparseindex_count[bestparseindex];
    ++logrelprobbest_count[bestlogprob-logmaxprob];  // logrelprob of best parse
    ++logcondprobbest_count[bestlogcondprob];  // logcondprob of best parse

    // relative probability threshold

    best.ntest = 0;     // keep ngold from previous best parse
    best.ncommon = 0;

    size_t i = 0;
    for (size_t j = 0; j < lcpt_prs.size(); ++j) {
      Float logrelprobthresh = index_lrpt(j);
      while (i < fprs.size() && fprs[i].first-logmaxprob >= logrelprobthresh) { 
	if (best < fprs[i].second)
	  best = fprs[i].second;
	++i;
      }
      lrpt_prs[j] += best;
      lrpt_count[j] += i;
    }

    // conditional probability threshold

    best.ntest = 0;     // keep ngold from previous best parse
    best.ncommon = 0;

    i = 0;
    for (size_t j = 0; j < lcpt_prs.size(); ++j) {
      Float logcondprobthresh = index_lcpt(j);
      while (i < fprs.size() && fprs[i].first-logsumprob >= logcondprobthresh) { 
	if (best < fprs[i].second)
	  best = fprs[i].second;
	++i;
      }
      lcpt_prs[j] += best;
      lcpt_count[j] += i;
    }
  }  // beam_sizer_type::operator()

  static inline Float index_lcpt(size_t index) { return index/-4.0; }

  static inline Float index_lrpt(size_t index) { return index/-4.0; }
};

int main(int argc, char** argv)
{
  assert(argc == 3);
  beam_sizer_type beam_sizer;
  size_t nsentences = corpus_type::map_sentences_cmd(argv[1], argv[2], beam_sizer);
  std::cout << "# Read " << nsentences << " sentences from " << argv[1] << " and " << argv[2] << std::endl;

  std::cout << "# Oracle " << beam_sizer.best_prs << std::endl;
  std::cout << "# Maxprob parse " << beam_sizer.maxprob_prs << std::endl;
  std::cout << "# First parse " << beam_sizer.first_prs << std::endl;

  std::cout << "\nset output \"ec-bestparseindex.pslatex\"\n" 
	    << "set xlabel \"Best parse index\"\n"
	    << "set ylabel \"\\\\rotatebox[origin=c]{90}{Probability}\"\n"
	    << "plot '-' using 1:2 notitle\n"
	    << "# bestparseindex\tprob" << std::endl;
  for (size_t i = 0; i < 50; ++i)
    std::cout << i << '\t' 
	      << dfind(beam_sizer.bestparseindex_count, i)/double(beam_sizer.nsentences) << std::endl;
  std::cout << std::endl;

  /*
  std::vector<Float> bestparseindex_percentiles(21);
  keyvalues_percentiles(beam_sizer.bestparseindex_count, bestparseindex_percentiles);
  std::cout << "\n# percentile\tmax_best_parse_index" << std::endl;
  for (size_t i = 0; i < bestparseindex_percentiles.size(); ++i)
    std::cout << Float(i)/(bestparseindex_percentiles.size() - 1)
	      << '\t' << bestparseindex_percentiles[i] << std::endl;
  */
  std::cout << "\nset output \"ec-beamsize-oracle.pslatex\"\n" 
	    << "set xlabel \"Beam size\"\n"
	    << "set ylabel \"\\\\rotatebox[origin=c]{90}{Oracle $f$-score}\"\n"
	    << "plot '-' using 1:3 notitle" << std::endl;
  std::cout << "# Beam_size\tcumulative_freq\toracle_fscore" << std::endl;
  for (size_t i = 0; i < beam_sizer.precrecs.size(); ++i ) {
    if (beam_sizer.nparses_counts[i] == 0)
      break;
    std::cout << i+1
	      << '\t' << beam_sizer.nparses_counts[i]/float(nsentences)
	      << '\t' << beam_sizer.precrecs[i].f_score() 
	      << std::endl;
  }
	
  /*
  std::vector<Float> logrelprobbest_percentiles(21);
  keyvalues_percentiles(beam_sizer.logrelprobbest_count, logrelprobbest_percentiles);
  std::cout << "\n# percentile\tmin_log_rel_prob_best" << std::endl;
  for (size_t i = 0; i < logrelprobbest_percentiles.size(); ++i)
    std::cout << Float(i)/(logrelprobbest_percentiles.size() - 1)
	      << '\t' << logrelprobbest_percentiles[i] << std::endl;
  */

  std::cout << "\nset output \"ec-logrelprob-oracle.pslatex\"\n" 
	    << "set xlabel \"$- log$ relative probability cutoff\"\n"
	    << "set ylabel \"\\\\rotatebox[origin=c]{90}{Oracle $f$-score}\"\n"
	    << "plot '-' using 1:2 notitle" << std::endl;
  std::cout << "\n# log_rel_prob_cutoff\toracle_fscore\tfrac_parses" << std::endl;
  for (size_t j = 0; j < beam_sizer.lrpt_prs.size(); ++j)
    std::cout << -beam_sizer_type::index_lrpt(j) 
	      << '\t' << beam_sizer.lrpt_prs[j].f_score() 
	      << '\t' << Float(beam_sizer.lrpt_count[j])/Float(beam_sizer.nparses)
	      << std::endl;


  std::vector<Float> logcondprobbest_percentiles(21);
  keyvalues_percentiles(beam_sizer.logcondprobbest_count, logcondprobbest_percentiles);
  std::cout << "\n# percentile\tmin_log_cond_prob_best" << std::endl;
  for (size_t i = 0; i < logcondprobbest_percentiles.size(); ++i)
    std::cout << Float(i)/(logcondprobbest_percentiles.size() - 1)
	      << '\t' << logcondprobbest_percentiles[i] << std::endl;

  std::cout << "\n# log_cond_prob_cutoff\toracle_fscore\tfrac_parses" << std::endl;
  for (size_t j = 0; j < beam_sizer.lcpt_prs.size(); ++j)
    std::cout << beam_sizer_type::index_lcpt(j) 
	      << '\t' << beam_sizer.lcpt_prs[j].f_score() 
	      << '\t' << Float(beam_sizer.lcpt_count[j])/Float(beam_sizer.nparses)
	      << std::endl;

  std::cout << "\n# " << resource_usage() << std::endl;

  /*
  std::cout << "# Index\tbest_parse_freq\tbest_parse_rel_freq" << std::endl;
  for (size_t i = 0; i <= beam_sizer.bestparseindex_count.rbegin()->first; ++i)
    std::cout << i << '\t'
	      << dfind(beam_sizer.bestparseindex_count, i) << '\t'
	      << dfind(beam_sizer.bestparseindex_count, i)/float(nsentences)
	      << std::endl;
  */
}
