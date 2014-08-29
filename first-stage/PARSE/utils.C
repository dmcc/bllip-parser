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
#include <assert.h>
#include <stdlib.h> // abort(), exit(), rand()
#include <algorithm>
#include "string.h"

extern int sentenceCount; // from parseIt.C

// this makes error() "weak" so we can override it in SWIG.
// unfortunately, this is a gcc specific trick.
#ifdef __GNUC__
#include "weakdecls.h"
#endif

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

ECString
langAwareToLower(ECString str)
{
    /* Arabic doesn't get lowercased, all other languages do (for now) */
    if (Term::Language == "Ar") {
        return str;
    }
    else {
        string lowercased(str);
        std::transform(lowercased.begin(), lowercased.end(),
            lowercased.begin(), ::tolower);
        return lowercased;
    }
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

void findAndReplace(string& text, string oldPattern, string newPattern) {
    size_t pos = 0;
    size_t oldLength = oldPattern.length();
    size_t newLength = newPattern.length();

    if (oldLength == 0) {
        return;
    }

    for (; (pos = text.find(oldPattern, pos)) != string::npos; ) {
        text.replace(pos, oldLength, newPattern);
        pos += newLength;
    }
}

void escapeParens(string& word) {
    findAndReplace(word, "(", "-LRB-");
    findAndReplace(word, ")", "-RRB-");
    findAndReplace(word, "{", "-LCB-");
    findAndReplace(word, "}", "-RCB-");
    findAndReplace(word, "[", "-LSB-");
    findAndReplace(word, "]", "-RSB-");
}

void unescapeParens(string& word) {
    findAndReplace(word, "-LRB-", "(");
    findAndReplace(word, "-RRB-", ")");
    findAndReplace(word, "-LCB-", "{");
    findAndReplace(word, "-RCB-", "}");
    findAndReplace(word, "-LSB-", "[");
    findAndReplace(word, "-RSB-", "]");
}

// returns whether string ends with pattern
bool endsWith(ECString str, ECString pattern) {
    int index = str.rfind(pattern);
    return index == ((signed int)str.size() - (signed int)pattern.size());
}
