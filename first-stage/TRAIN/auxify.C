/*
 * Copyright 1999 Brown University, Providence, RI.
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

#include "auxify.h"
#include <iostream>
#include "Term.h"
#include "ECString.h"
#include "string.h"

char* 	suffixes[] = {
"'VE",
"'M",
"'LL",
"'D",
"'S",
"'RE",
0
};

char* 	auxgs[] = {
"BEIN",
"HAVING",
"BEING",
0
};


char* 	auxs[] = {
"MAHT",
"SHULD",
"WILL",
"WAS",
"OUGHTA",
"AHM",
"NEED",
"MAYE",
"WILLYA",
"WHADDYA",
"HATH",
"HAVE",
"WERE",
"IS",
"HAS",
"MUST",
"DID",
"HAD",
"DO",
"MIGHT",
"WOULD",
"SHALL",
"SHOULD",
"OUGHT",
"COULD",
"DOES",
"HAFTA",
"BE",
"KIN",
"CAN",
"ART",
"BEEN",
"DONE",
"ARE",
"DOO",
"MAY",
"AM",
0
};

bool
hasAuxSuf( ECString word )
{
    size_t pos = word.find_first_of("\'");
    if(pos == -1) return false;
    ECString apostrophe = word.substr(pos, word.length()-pos);
    for( int i = 0; suffixes[i]; i++)
    {
	if( apostrophe == suffixes[i] ) 
	    return true;
    }
    return false;
}

bool
isAux( ECString word )
{
    for( int i = 0; auxs[i]; i++)
    {
	if( word == auxs[i] )
	    return true;
    }
    return false;
}

bool
isAuxg( ECString word )
{
    for( int i = 0; auxgs[i]; i++)
    {
	if( word == auxgs[i] ) 
	    return true;
    }
    return false;
}

char* verbs[] = {
"VB",
"VBD",
"VBG",
"VBN",
"VBP",
"VBZ",
0
};

bool
isVerb( ECString tag )
{
    for( int i = 0; verbs[i]; i++)
	if( tag == verbs[i] ) 
	    return true;
    return false;
}

char*
toUpper(const char* str, char* temp)
{
  int l = strlen(str);
  assert(l < 128);
  for(int i = 0 ; i <= l ; i++)
    {
      char n = str[i];
      int ni = (int)n;
      if(ni >= 97 && ni <= 122)
	{
	  temp[i] = (char)(ni-32);
	}
      else temp[i] = n;
    }
  return temp;
}

ECString
auxify(ECString wM, ECString trmM)
{
  char temp[128];
  ECString w = toUpper(wM.c_str(),temp);
  ECString trm = toUpper(trmM.c_str(),temp);
  if( isVerb( trm ) )
    {
      //cout << "saw verb " << trm << " " << wM << endl;
      if( isAux( w ) || hasAuxSuf( w ) )
	{
	  //cout << "was aux " << w << endl;
	  return "AUX";
	}
      else if( isAuxg( w ) )
	{
	  //cout << "was auxg " << w << endl;
	  return "AUXG";
	}
    }
  return trmM;
}

void
treeauxify(InputTree* tree){
  if(tree->term() != "VP") return;
  InputTreesIter iti = tree->subTrees().begin();
  bool sawVP = false;
  for( ; iti != tree->subTrees().end() ; iti++)
    {
      InputTree* subtree = (*iti);
      ECString subtrmS = subtree->term();
      ConstTerm* subtrm = Term::get(subtrmS);
      if(subtrmS == "VP")
	{
	  sawVP = true;
	  continue;
	}
      else if(subtrm->terminal_p() == 2 || isVerb(subtrmS) ||
	      subtrmS == "ADVP" || subtrmS == "RB" ||
	      subtrmS == "UCP")
	continue;
      else return;
    }
  if(!sawVP) return;
  iti = tree->subTrees().begin();
  for( ; iti != tree->subTrees().end() ; iti++)
    {
      InputTree* subtree = (*iti);
      ECString subtrmS = subtree->term();
      if(subtree->word().empty()) continue;
      ECString newv = auxify(subtree->word(), subtree->term());
      subtree->term() = newv;
    }
  return;
}

				    

	 
