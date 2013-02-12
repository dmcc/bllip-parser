/*
 * Copyright 1999, 2005 Brown University, Providence, RI.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.  You may obtain
 * a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#include "Term.h"
#include "utils.h"

Term*  Term::array_[MAXNUMNTTS];
map<ECString, Term*,less<ECString> > Term::termMap_;
int    Term::lastTagInt_ = 0;
int    Term::lastNTInt_ = 0;
const Term*  Term::stopTerm;
const Term*  Term::startTerm;
const Term*  Term::rootTerm;
ECStrings Term::Colons;
ECStrings Term::Finals;
ECString Term::Language = "En";

bool
Term::
isS() const 
{
  if(Term::Language == "Ch") return name_== "IP";
  else return name_ == "S";
}

Term::
Term()
:
  terminal_p_( 0 ),
  num_( -1 )
{}

Term::
Term(const ECString s, int terminal, int num )
:
  terminal_p_( terminal ),
  num_( num ),
  name_( s )
{}

Term::
Term( const Term& src )
:
  terminal_p_( src.terminal_p() ),
  name_( src.name() )
{}

ostream&
operator<< ( ostream& os, const Term& t )
{
    os << t.name();
    return os;
}

int
Term::
operator== ( const Term& rhs ) const
{
    if( this == &rhs || name_ == rhs.name() )
	return 1;
    return 0;
}


void
Term::
init(ECString & prefix)
{
  ECString          fileName(prefix);
  fileName += "terms.txt";
  ifstream           stream(fileName.c_str(), ios::in);
  if (!stream)
    {
      cerr << "Can't open terms file " << fileName << endl;;
      return;
    }
  
  ECString          termName;
  int ind, n;
  n = 0;
  bool seenNTs = false;
  while (stream >> termName)
    {
      stream >> ind;
      Term* nextTerm = new Term(termName, ind, n);
      termMap_[nextTerm->name()] = nextTerm;
      if(termName == "STOP") Term::stopTerm = nextTerm;
      else if(termName == "G4") Term::startTerm = nextTerm;
      else if(termName == "S1") Term::rootTerm = nextTerm;
      array_[n] = nextTerm;
      if(!ind && !seenNTs)
	{
	  assert(n > 0);
	  lastTagInt_ = n-1;
	  seenNTs = true;
	}
      n++;
      assert(n < 400);
    }
  assert(!ind);
  lastNTInt_ = n-1;
  //lastNTInt_ = n-4;  //??? hack to ignore G1 and G2 and G3;
  stream.close();
}

Const_Term_p
Term::
get(const ECString& nm)
{
  TermMap::iterator ti = termMap_.find(nm);
  if( ti == termMap_.end()) return NULL;
  return (*ti).second;
}

