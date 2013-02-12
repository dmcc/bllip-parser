/*
 * Copyright 2006 Brown University, Providence, RI.
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

#ifndef READT_H
#define READT_H
#include "InputTree.h"

class ReadTree
{
 public:
  ReadTree(istream* is): isp(is){}
  static void readParse(istream& is,InputTree& ans);
  static InputTree* newParse(istream& is, int& strt, InputTree* par);
  static ECString& readNext( istream& is ); 
  static void parseTerm(istream& is, ECString& a, ECString& b, int& num);
  static ECString tempword[400];
  static int      tempwordnum;
  static int      sentenceCount;
  istream*  isp;
};

#endif
