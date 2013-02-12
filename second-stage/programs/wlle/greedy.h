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

// greedy.h -- greedy search
//
// Mark Johnson, 4th June 2005, modified for openmp 1st August 2008

#include "custom_allocator.h"  // must be first

#include <ext/hash_set>
#include "pqueue.h"

//! greedy() does a greedy search on xs.
//! xs should be a binary vector; its elements will
//! be set to 0 or 1 to turn elements on or off
//! in order to minimize f(xs).
//
template <typename f_type, typename xs_type>
void greedy(f_type& f, xs_type& xs) {
  typedef double Float;
  Float fxs = f(xs);
  xs_type best_xs(xs);
  Float best_fxs = fxs;
  ext::hash_set<xs_type> cache;
  cache.insert(xs);
  pqueue<xs_type,Float> pq;
  pq.set(xs, fxs);
  while (!pq.empty()) {
    xs_type xs0 = pq.top_key();
    pq.pop();  // remove analysis
# pragma omp parallel for default(shared)
    for (int i = 0; i < int(xs0.size()); ++i) {  // flip bit i
      xs_type xs1 = xs0;
      if (xs1[i] == 0)
	xs1[i] = 1;
      else
	xs1[i] = 0;
      bool inserted;
# pragma omp critical (greedy_cache0)
      inserted = cache.insert(xs1).second;
      if (inserted) {
	Float fxs1 = f(xs1);
# pragma omp critical (greedy_cache1)
	{
	  pq.set(xs1, fxs1);
	  if (fxs1 < best_fxs) {
	    best_fxs = fxs1;
	    best_xs = xs1;
	  }
	}
      }
    }  // end for parallel region
  }
  xs = best_xs;
}
