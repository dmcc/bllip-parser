// histogram.h
//
// Mark Johnson, 17th October 2002
// Mark Johnson, 26th December, 2004

#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <cassert>

template <typename map_type, typename vector_type> 
typename map_type::mapped_type 
keyvalues_percentiles(const map_type& key_count, 
		      vector_type& percentiles)
{
  typedef typename map_type::mapped_type count_type;
  typedef typename map_type::const_iterator it_type;

  count_type sum = 0;

  for (it_type it = key_count.begin(); it != key_count.end(); ++it)
    sum += it->second;

  if (sum == 0) {
    percentiles.clear();
    return 0;
  }

  if (percentiles.empty())
    percentiles.resize(11);

  assert(!key_count.empty());
  size_t nbins = percentiles.size()-1;

  percentiles[0] = key_count.begin()->first;
  count_type current_sum = 0;
  size_t last_bin = 0;
  for (it_type it = key_count.begin(); it != key_count.end(); ++it) {
    current_sum += it->second;
    size_t current_bin = (nbins*current_sum)/sum;
    while (last_bin < current_bin)
      percentiles[++last_bin] = it->first;
  }
  return sum;
}

template <typename value_count_type, typename valueprobs_type>
typename value_count_type::mapped_type
valuecount_valueprobs(const value_count_type& value_count,
		      valueprobs_type& valueprobs,
		      size_t nvalues = 21)
{
  typedef typename value_count_type::key_type value_type;
  typedef typename value_count_type::mapped_type count_type;
  typedef typename valueprobs_type::value_type valueprob_type;

  valueprobs.clear();

  std::vector<value_type> percentiles(nvalues);
  count_type sum = keyvalues_percentiles(value_count, percentiles);
  
  valueprobs.push_back(valueprob_type(percentiles[0], 0));
  
  for (size_t i = 1; i < nvalues; ++i) 
    valueprobs.push_back(valueprob_type((percentiles[i-1]+percentiles[i])/2, 
					1.0/((nvalues-1.0)*(percentiles[i]-percentiles[i-1]))));

  valueprobs.push_back(valueprob_type(percentiles[nvalues-1], 0));

  return sum;
}


struct valueprobsprinter_type {
  typedef std::pair<double,double> valueprob_type;
  typedef std::vector<valueprob_type> valueprobs_type;
  valueprobs_type valueprobs;
};

std::ostream& operator<< (std::ostream& os, 
			  const valueprobsprinter_type& valueprobsprinter) {
  cforeach (valueprobsprinter_type::valueprobs_type, it, valueprobsprinter.valueprobs)
    os << it->first << '\t' << it->second << std::endl;
  return os;
}

template <typename value_count_type>
valueprobsprinter_type write_valueprobs(const value_count_type& value_count,
					size_t nvalues = 21) {
  valueprobsprinter_type valueprobsprinter;
  valuecount_valueprobs(value_count, valueprobsprinter.valueprobs, nvalues);
  return valueprobsprinter;
}
  
template <typename KeyValues, typename MinKey, typename MaxKey>
void min_max_key(const KeyValues& kvs, 
		 MinKey& min_key, MaxKey& max_key)
{
  assert(!kvs.empty());
  min_key = kvs.begin()->first;
  max_key = min_key;
  for (typename KeyValues::const_iterator it = kvs.begin();
       it != kvs.end(); ++it)
    if (it->first < min_key)
      min_key = it->first;
    else if (it->first > max_key)
      max_key = it->first;
}  // min_max_key()


template <typename ValueCount, typename Counts>
void histogram(const ValueCount& value_count, 
	       float lower_bound, float upper_bound,
	       Counts& counts)
{
  float nbins = counts.size();
  for (typename ValueCount::const_iterator it = value_count.begin();
       it != value_count.end(); ++it) 
    if (it->first >= lower_bound && it->first <= upper_bound) {
      size_t bin = size_t(nbins*(it->first-lower_bound)/(upper_bound-lower_bound));
      if (bin >= counts.size())
	bin = counts.size()-1;
      counts[bin] += it->second;
    }
}  // histogram()

#endif // HISTOGRAM_H
