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
: num_( -1 ), terminal_p_( 0 )
{}

Term::
Term(const ECString s, int terminal, int num )
: name_( s ), num_( num ), terminal_p_( terminal )
{}

Term::
Term( const Term& src )
: name_( src.name() ), 
  //num_(src.toInt()),
  terminal_p_( src.terminal_p() )
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
  ifstream           strm(fileName.c_str(), ios::in);
  if (!strm)
    {
      cerr << "Can't open terms file " << fileName << endl;;
      return;
    }
  
  ECString          termName;
  int ind, n;
  n = 0;
  bool seenNTs = false;
  while (strm >> termName)
    {
      strm >> ind;
      Term* nextTerm = new Term(termName, ind, n);
      termMap_[nextTerm->name()] = nextTerm;
      array_[n] = nextTerm;
      if(!ind && !seenNTs)
	{
	  assert(n > 0);
	  lastTagInt_ = n-1;
	  seenNTs = true;
	}
      n++;
      assert(n < MAXNUMNTTS);
    }
  assert(!ind);
  lastNTInt_ = n-1;
  strm.close();
  stopTerm = get("STOP");
  rootTerm = get("S1");
}

Const_Term_p
Term::
get(const ECString& nm)
{
  TermMap::iterator ti = termMap_.find(nm);
  if( ti == termMap_.end()) return NULL;
  return (*ti).second;
}
