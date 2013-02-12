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

// symset.h
//
// Mark Johnson, 8th October 2001
//
// A symset is a set of symbols.  Its primary goal is to be easy to
// initialize.  You initialize a symset with a string, e.g.
//
// symset verb_pos("VB VBZ VBD VBP");
//
// It is based on the STL set, and should be efficient enough to use
// with hundreds of elements.  For bigger sets, you may get better
// performance by basing symsets on hash_sets.
//
// symset_base is the base type for symsets, parameterized by the
// type of symbol set to use.

#ifndef SYMSET_H
#define SYMSET_H

#include <set>
#include <iostream>
#include <string>
#include "sym.h"

template <class Set = std::set<symbol> >  // replace set by hash_set if needed
struct symset_base : public Set
{
  void set(const char* c=NULL, const char sep = ' ') {
    this->clear();
    if (c != NULL) {
      for (std::string s; true; ++c) {
	if (*c == sep || *c == '\0') {
	  this->insert(symbol(s));
	  s.clear();
	  if (*c == '\0')
	    break;
	}
	else
	  s.push_back(*c);
      }
    }
  }

  symset_base(const char* c=NULL, const char sep = ' ') {
    set(c, sep);
  }

  bool contains(symbol s) const { return this->count(s) == 1; }
  bool operator()(symbol s) const { return this->count(s) == 1; }
};

template <class Set>
inline std::ostream& operator<< (std::ostream& os, const symset_base<Set>& ss)
{
  os << '{';
  for (typename symset_base<Set>::const_iterator i = ss.begin(); i != ss.end(); ++i) {
    if (i != ss.begin())
      os << ' ';
    os << *i;
  }
  return os << '}';
}

typedef symset_base<> symset;   // symset is a shorthand for symset_base<>
  
#endif // SYMSET_H
