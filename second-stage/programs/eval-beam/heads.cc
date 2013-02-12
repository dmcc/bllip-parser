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

// heads.cc
//
// Mark Johnson, 9th November 2001, last modified 22nd March 2004
//
// This file exists mainly to provide a compilation unit in which
// the head data structures can be constructed exactly once.
//
// heads::syntactic() and heads::semantic() are global functions
// that return references to the static head data objects.  They
// are used to avoid the static member initialization bug.

#include "custom_allocator.h"     // must be first

#include <cstdlib>
#include <ext/hash_map>
#include <vector>

#include "heads.h"
#include "sym.h"
#include "symset.h"
#include "utility.h"

namespace heads {

  // heads::syntactic() returns the static syntactic_data object.
  // This function exists to avoid the static initialization bug.
  //
  const syntactic_data& syntactic() {
    static syntactic_data data;
    return data;
  }  // heads::syntactic_heads()


  // heads::semantic() returns the static semantic_data object.
  // This function exists to avoid the static initialization bug.
  //
  const semantic_data& semantic() {
    static semantic_data data;
    return data;
  }  // heads::semantic_heads()


  //! initializer for syntactic_data{}
  //
  syntactic_data::syntactic_data() 
    : adjective(2), conjunction(1), interjection(1), noun(6),
      preposition(2), unknown(0), verb(4),
      rightheaded_nominals("NN NNS NNP NNPS $") 
  {
    adjective[0].set("$ CD JJ JJR JJS RB RBR RBS WRB");
    adjective[1].set("ADJP ADVP");
    conjunction[0].set("CC");
    interjection[0].set("INTJ UH");
    noun[0].set("POS");
    noun[1].set("DT WDT WP$ WP PRP EX");
    noun[2].set("NN NNS");
    noun[3].set("$ NNP NNPS");
    noun[4].set("-NONE- QP NP NP$ WHNP");
    noun[5].set("CD IN JJ JJR JJS PDT RB PP");
    preposition[0].set("IN RP TO");
    preposition[1].set("PP");
    verb[0].set("AUX AUXG MD");
    verb[1].set("VB VBD VBG VBN VBP VBZ");
    verb[2].set("VP");
    verb[3].set("ADJP JJ S SINV SQ TO");

    head_type[symbol("ADJP")] = &adjective;
    head_type[symbol("ADVP")] = &verb;
    head_type[symbol("CONJP")] = &conjunction;
    head_type[symbol("FRAG")] = &noun;
    head_type[symbol("INTJ")] = &interjection;
    head_type[symbol("LST")] = &noun;
    head_type[symbol("NAC")] = &noun;
    head_type[symbol("NP")] = &noun;
    head_type[symbol("NX")] = &noun;
    head_type[symbol("PP")] = &preposition;
    head_type[symbol("PRN")] = &noun;
    head_type[symbol("PRT")] = &preposition;
    head_type[symbol("QP")] = &noun;
    head_type[symbol("ROOT")] = &verb;
    head_type[symbol("RRC")] = &verb;
    head_type[symbol("S")] = &verb;
    head_type[symbol("SBAR")] = &verb;
    head_type[symbol("SBARQ")] = &verb;
    head_type[symbol("SINV")] = &verb;
    head_type[symbol("SQ")] = &verb;
    head_type[symbol("S1")] = &verb;
    head_type[symbol("UCP")] = &adjective;
    head_type[symbol("VP")] = &verb;
    head_type[symbol("WHADJP")] = &adjective;
    head_type[symbol("WHADVP")] = &adjective;
    head_type[symbol("WHNP")] = &noun;
    head_type[symbol("WHPP")] = &preposition;
    head_type[symbol("X")] = &unknown;
  }  // heads::syntactic_data::syntactic_data()

  //! initializer for semantic_data
  //
  semantic_data::semantic_data() 
    : adjective(2), conjunction(1), interjection(1), 
      noun(4), preposition(2), unknown(0), verb(4) 
  {
    adjective[0].set("$ CD JJ JJR JJS RB RBR RBS WRB");
    adjective[1].set("ADJP ADVP");
    conjunction[0].set("CC");
    interjection[0].set("INTJ UH");
    noun[0].set("EX NN NNS PRP WP");
    noun[1].set("$ NNP NNPS");
    noun[2].set("QP NP WP$");
    noun[3].set("CD DT IN JJ JJR JJS PDT POS RB WDT");
    preposition[0].set("IN RP TO");
    preposition[1].set("PP");
    verb[0].set("VP");
    verb[1].set("VB VBD VBG VBN VBP VBZ");
    verb[2].set("ADJP JJ S SINV SQ TO");
    verb[3].set("AUX AUXG MD");

    head_type[symbol("ADJP")] = &adjective;
    head_type[symbol("ADVP")] = &verb;
    head_type[symbol("CONJP")] = &conjunction;
    head_type[symbol("FRAG")] = &noun;
    head_type[symbol("INTJ")] = &interjection;
    head_type[symbol("LST")] = &noun;
    head_type[symbol("NAC")] = &noun;
    head_type[symbol("NP")] = &noun;
    head_type[symbol("NX")] = &noun;
    head_type[symbol("PP")] = &preposition;
    head_type[symbol("PRN")] = &noun;
    head_type[symbol("PRT")] = &preposition;
    head_type[symbol("QP")] = &noun;
    head_type[symbol("ROOT")] = &verb;
    head_type[symbol("RRC")] = &verb;
    head_type[symbol("S")] = &verb;
    head_type[symbol("SBAR")] = &verb;
    head_type[symbol("SBARQ")] = &verb;
    head_type[symbol("SINV")] = &verb;
    head_type[symbol("SQ")] = &verb;
    head_type[symbol("S1")] = &verb;
    head_type[symbol("UCP")] = &adjective;
    head_type[symbol("VP")] = &verb;
    head_type[symbol("WHADJP")] = &adjective;
    head_type[symbol("WHADVP")] = &adjective;
    head_type[symbol("WHNP")] = &noun;
    head_type[symbol("WHPP")] = &preposition;
    head_type[symbol("X")] = &unknown;
  }  // heads::semantic_data::semantic_data()

}  // namespace heads

