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

// heads-german.cc
//
// Mark Johnson, 9th November 2001, last modified 22nd March 2004
//
// This file exists mainly to provide a compilation unit in which
// the head data structures can be constructed exactly once.
//
// heads::syntactic() and heads::semantic() are global functions
// that return references to the static head data objects.  They
// are used to avoid the static member initialization bug.

//
// Modifications for German head rules (Negra corpus), made by
// Shashi Narayan (snaraya2@inf.ed.ac.uk) and
// Shay Cohen (scohen@inf.ed.ac.uk).
//
// Date: July, 2015.

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
    : adjective(2), conjunction(1), interjection(1), 
      noun(8), preposition(5), unknown(0), verb(4)
  {
    adjective[0].set("CARD ORD ADJA ADJD ADV PAV PROAV");
    adjective[1].set("AA AP AVP");
    conjunction[0].set("KOUI KOUS KON KOKOM");
    interjection[0].set("ITJ");
    noun[0].set("ART");
    noun[1].set("CARD ORD FM");
    noun[2].set("NN NE");
    noun[3].set("PDS PDAT PIS PIAT PIDAT PPER PPOSS PPOSAT");
    noun[4].set("PRELS PRELAT PRF");
    noun[5].set("PWS PWAT PWAV");
    noun[6].set("XY SGML SPELL TRUNC --");
    noun[7].set("NP");
    preposition[0].set("APPR APPRART");
    preposition[1].set("APPO");
    preposition[2].set("APZR");
    preposition[3].set("PTKZU PTKNEG PTKVZ PTKANT PTKA");
    preposition[4].set("PP");
    verb[0].set("VAFIN VAIMP VAINF VAPP VMFIN VMINF VMPP");
    verb[1].set("VVFIN VVIMP VVINF VVIZU VVPP");
    verb[2].set("VP");
    verb[3].set("S S1 VZ");

    head_type[symbol("--")] = &noun;
    head_type[symbol("AA")] = &adjective;
    head_type[symbol("AP")] = &adjective;
    head_type[symbol("AVP")] = &adjective;
    head_type[symbol("CAC")] = &conjunction;
    head_type[symbol("CAP")] = &conjunction;
    head_type[symbol("CAVP")] = &conjunction;
    head_type[symbol("CCP")] = &conjunction;
    head_type[symbol("CH")] = &verb;
    head_type[symbol("CNP")] = &conjunction;
    head_type[symbol("CO")] = &conjunction;
    head_type[symbol("CPP")] = &conjunction;
    head_type[symbol("CS")] = &conjunction;
    head_type[symbol("CVP")] = &conjunction;
    head_type[symbol("CVZ")] = &conjunction;
    head_type[symbol("DL")] = &unknown;
    head_type[symbol("ISU")] = &unknown;
    head_type[symbol("MPN")] = &noun;
    head_type[symbol("MTA")] = &adjective;
    head_type[symbol("NM")] = &noun;
    head_type[symbol("NP")] = &noun;
    head_type[symbol("PP")] = &preposition;
    head_type[symbol("QL")] = &noun;
    head_type[symbol("S")] = &verb;
    head_type[symbol("S1")] = &verb;
    head_type[symbol("VP")] = &verb;
    head_type[symbol("VZ")] = &verb;

  }  // heads::syntactic_data::syntactic_data()

  //! initializer for semantic_data
  //
  semantic_data::semantic_data() 
    : adjective(2), conjunction(1), interjection(1), 
      noun(8), preposition(5), unknown(0), verb(4)
  {
    adjective[0].set("CARD ORD ADJA ADJD ADV PAV PROAV");
    adjective[1].set("AA AP AVP");
    conjunction[0].set("KOUI KOUS KON KOKOM");
    interjection[0].set("ITJ");
    noun[0].set("ART");
    noun[1].set("CARD ORD FM");
    noun[2].set("NN NE");
    noun[3].set("PDS PDAT PIS PIAT PIDAT PPER PPOSS PPOSAT");
    noun[4].set("PRELS PRELAT PRF");
    noun[5].set("PWS PWAT PWAV");
    noun[6].set("XY SGML SPELL TRUNC --");
    noun[7].set("NP");
    preposition[0].set("APPR APPRART");
    preposition[1].set("APPO");
    preposition[2].set("APZR");
    preposition[3].set("PTKZU PTKNEG PTKVZ PTKANT PTKA");
    preposition[4].set("PP");
    verb[0].set("VAFIN VAIMP VAINF VAPP VMFIN VMINF VMPP");
    verb[1].set("VVFIN VVIMP VVINF VVIZU VVPP");
    verb[2].set("VP");
    verb[3].set("S S1 VZ");
    
    head_type[symbol("--")] = &noun;
    head_type[symbol("AA")] = &adjective;
    head_type[symbol("AP")] = &adjective;
    head_type[symbol("AVP")] = &adjective;
    head_type[symbol("CAC")] = &conjunction;
    head_type[symbol("CAP")] = &conjunction;
    head_type[symbol("CAVP")] = &conjunction;
    head_type[symbol("CCP")] = &conjunction;
    head_type[symbol("CH")] = &verb;
    head_type[symbol("CNP")] = &conjunction;
    head_type[symbol("CO")] = &conjunction;
    head_type[symbol("CPP")] = &conjunction;
    head_type[symbol("CS")] = &conjunction;
    head_type[symbol("CVP")] = &conjunction;
    head_type[symbol("CVZ")] = &conjunction;
    head_type[symbol("DL")] = &unknown;
    head_type[symbol("ISU")] = &unknown;
    head_type[symbol("MPN")] = &noun;
    head_type[symbol("MTA")] = &adjective;
    head_type[symbol("NM")] = &noun;
    head_type[symbol("NP")] = &noun;
    head_type[symbol("PP")] = &preposition;
    head_type[symbol("QL")] = &noun;
    head_type[symbol("S")] = &verb;
    head_type[symbol("S1")] = &verb;
    head_type[symbol("VP")] = &verb;
    head_type[symbol("VZ")] = &verb;

  }  // heads::semantic_data::semantic_data()

}  // namespace heads

