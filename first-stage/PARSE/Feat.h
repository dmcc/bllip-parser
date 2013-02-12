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

#ifndef FEAT_H
#define FEAT_H

#include "Feature.h"

class FeatureTree;
#define ISCALE 1
#define PARSE 2

class Feat
{
public:
  friend ostream& operator<< ( ostream& os, Feat& t );
  float& g() { return g_; }
  //float& lambda() { return uVals[1]; }
  //float& u(int i) { return uVals[i+1]; }
  //int& cnt() { return cnt_; }
  int& ind() { return ind_; }
  int ind_;
  //int cnt_;
  //float* uVals;
  float g_;
  static int Usage;
};

#endif /* ! FEAT_H */
