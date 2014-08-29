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

#ifndef UTILS_H 
#define UTILS_H

#include "ECString.h"
#include <vector>
#include <clocale>

#define WARN( msg ) if (!Bchart::silent) { warn( __FILE__, __LINE__, msg ); }
#define ERROR( msg ) error( __FILE__, __LINE__, msg )

void warn( const char *filename, int filelinenum, const char *msg );
void error( const char *filename, int filelinenum, const char *msg );
void error(const char *s); // backwards compatibility

ECString langAwareToLower(ECString str);
ECString intToString(int i);

typedef vector<ECString> ECStrings;
typedef ECStrings::iterator ECStringsIter;
bool vECfind(const ECString& st, ECStrings& sts);

void findAndReplace(string& text, string oldPattern, string newPattern);
void escapeParens(string& word);
void unescapeParens(string& word);
bool endsWith(ECString str, ECString pattern);

#endif /* ! UTILS_H */
