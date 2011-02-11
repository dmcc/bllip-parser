/*
 * Copyright 1997, Brown University, Providence, RI.
 * 
 *                         All Rights Reserved
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose other than its incorporation into a
 * commercial product is hereby granted without fee, provided that the
 * above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Brown University not be used in
 * advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.
 * 
 * BROWN UNIVERSITY DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR ANY
 * PARTICULAR PURPOSE.  IN NO EVENT SHALL BROWN UNIVERSITY BE LIABLE FOR
 * ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include "utils.h"
#include "Term.h"
#include <iostream>
#include <assert.h>
#include <stdlib.h> // abort(), exit(), rand()
#include <algorithm>
#include "string.h"

extern int sentenceCount; // from parseIt.C

void 
warn( const char *filename, int filelinenum, const char *msg )
{
  cerr <<  "Warning [" << filename << ":" << filelinenum << "]";
  cerr << " Sentence " << sentenceCount << ": " << msg << endl;
}

void 
error( const char *filename, int filelinenum, const char *msg )
{
  cerr <<  "Warning [" << filename << ":" << filelinenum << "]";
  cerr << " Sentence " << sentenceCount << ": " << msg << endl;
  abort();
  exit( 1 );
}

void 
error(const char *s) 
{ 
  ERROR( s ); 
}

char*
langAwareToLower(const char* str, char* temp)
{
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
  assert(l < 512);
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
vECfind(const ECString& s, ECStrings& sts)
{
  return ( find(sts.begin(),sts.end(),s) != sts.end() );
}

/*
UNUSED ---------------------------------------

double 
ran()
{
    return (rand() * 4.656612875245796E-10);
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
*/
#if 0 // can't comment out code below since it uses '*/'

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
	  
--------------------------------------- 
#endif // 0
