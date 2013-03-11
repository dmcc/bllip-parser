/*
 * Copyright 1997, Brown University, Providence, RI.
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

#include "utils.h"
#include "Term.h"
#include <iostream>
#include <fstream>
#include <math.h>
#include "ECString.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

void
error( const char *s )
{
    cerr <<  "error: " << s << endl;
    abort();
    exit( 1 );
}

/* same as error(), but no abort() call. */
void
warn( const char *s )
{
    cerr <<  "warning: " << s << endl;
}

void
ignoreComment(ifstream& inpt)
{
  ECString nxt;
  char a;
  inpt.get(a);
  if(a == '/')
    {
      char b = inpt.peek();
      if(b == '*')
	{
	  while(inpt)
	    {
	      inpt >> nxt;
	      if(nxt == "*/") break;
	    }
	  return;
	}
    }
  inpt.putback(a);
  return;
}
	  

double 
ran()
{
    return (rand() * 4.656612875245796E-10);
}


char*
langAwareToLower(const char* str, char* temp)
{
  int l = strlen(str);
  assert(l < 1024);

  /* Arabic doesn't get lowercased, all other languages do (for now) */
  if (Term::Language == "Ar") {
    strcpy(temp, str);
    return temp;
  }
  else
    return toLower(str, temp);
}

char*
toLower(const char* str, char* temp)
{
  int l = strlen(str);
  assert(l < 1024);
  for(int i = 0 ; i <= l ; i++)
    {
      char n = str[i];
      int ni = (int)n;
      if(ni >= 65 && ni <= 90)
	{
	  temp[i] = (char)(ni+32);
	}
      else temp[i] = n;
    }
  return temp;
}


ECString
intToString(int i)
{
  char temp[16];
  sprintf(temp, "%i", i); 
  ECString ans(temp);
  return ans;
}

bool 
vECfind(ECString s, ECStrings& sts)
{
  ECStringsIter eci = sts.begin();
  for( ; eci != sts.end() ; eci++) if(s == (*eci)) return true;
  return false;
}

ECString lastCharacter(const ECString& s)
{
        ECString f;
        int len=s.length();
        assert(s!="");
        int c=(int)s[len-1];
        if (c<0||c>127) {
                assert(len>=2);
                f=f+s[len-2]+s[len-1];
        }else f=f+s[len-1];
        return f;
}

// returns whether string ends with pattern
bool endsWith(ECString str, ECString pattern) {
    int index = str.rfind(pattern);
    return index == ((signed int)str.size() - (signed int)pattern.size());
}

void repairPath(ECString& str) {
    if (!endsWith(str, "/")) {
        str = str + "/";
    }
}
