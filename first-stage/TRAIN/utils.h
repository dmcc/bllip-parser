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

#include <stdlib.h>
#include <fstream>
#include <list>
#include "ECString.h"
#include <vector>

void error(const char *s);
void warn(const char *s);
double ran();
char* langAwareToLower(const char* str, char* temp);
char* toLower(const char* str, char* temp);
ECString intToString(int i);
typedef vector<ECString> ECStrings;
typedef ECStrings::iterator ECStringsIter;
bool vECfind(ECString st, ECStrings& sts);

void ignoreComment(ifstream& input);
ECString lastCharacter(const ECString& s);
ECString firstCharacter(const ECString& s);
bool endsWith(ECString str, ECString pattern);
void repairPath(ECString& str);

#endif /* ! UTILS_H */
