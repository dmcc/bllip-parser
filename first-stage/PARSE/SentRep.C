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

#include "Bchart.h"
#include "SentRep.h"
#include "Params.h"
#include "utils.h"
#include "ewDciTokStrm.h"
#include <assert.h>

//------------------------------

SentRep::
SentRep()
{
  sent_.reserve(Params::DEFAULT_SENT_LEN);
}

//------------------------------

SentRep::
SentRep(int size)
{
  sent_.reserve(size);
}

//------------------------------

SentRep::
SentRep(list<ECString> wtList) 
  : sent_( wtList.size() )
{
  int i = 0;
  for (list<ECString>::iterator wi = wtList.begin(); wi != wtList.end(); ++wi){
      sent_[i] = Wrd(*wi,i);
      i++;
  }
}

//------------------------------
// 05/30/06 ML
// This really belongs in ewDciTokStrm.h/.C, but as it appears it
// isn't needed elsewhere, I'll just avoid modifying another file.

static ewDciTokStrm& operator>>( ewDciTokStrm& istr, ECString& w)
{
  w = istr.read();
  return istr;
}

//------------------------------
// Avoid scaring anyone by exposing use of template in header file --
// instead just map operator>> calls to this file local function.

template<class T>
static T& readSentence( T& istr, vector<Wrd>& sent, ECString& name)
{
  sent.clear();
  ECString w;

  while (!(!istr))
    {
      istr >> w;
      if( w == "<s>" ) 
	break;
      if(w == "<s")
	{
	  istr >> name;
	  if ( name[name.length()-1] == '>' ) 
	    {	    
	      name = name.substr(0,name.length()-1); // discard trailing '>'
	    } 
	  else // "<s LABEL >"
	    {
	      istr >> w;
	      if ( w != ">" ) 
		WARN("No closing '>' delimiter found to match opening \"<s\"");
	    } 
	  break;
	}
    }
  while (!(!istr))
    {
      istr >> w;

      if (w == "</s>" )
	break;

      // XXX previously:
      // Wrd::substBracket(w);
      escapeParens(w);
      int pos = sent.size();
      sent.push_back(Wrd(w,pos));
    }

  return istr;
}


//------------------------------

istream& operator>> (istream& is, SentRep& sr) 
{
  return readSentence(is, sr.sent_, sr.name_);
}

//------------------------------

ewDciTokStrm& operator>> (ewDciTokStrm& is, SentRep& sr)
{
  return readSentence(is, sr.sent_, sr.name_);
}

//------------------------------

ostream& operator<< (ostream& os, const SentRep& sr) 
{
  for( int i = 0; i < sr.length(); i++ )
    os << sr[ i ] << " ";
  return os;
}

////////////////////////////////
// End of File
