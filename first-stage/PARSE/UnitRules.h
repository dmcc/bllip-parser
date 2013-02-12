/*
 * Copyright 2005 Brown University, Providence, RI.
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

#ifndef UNITRULES_H
#define UNITRULES_H

#include "InputTree.h"
#include <iostream>
#include <fstream>
#include "Feature.h"

class UnitRules
{
 public:
  void init();
  void readTrees(istream& dataStream);
  void gatherData(InputTree* tree);
  void setData();
  bool badPair(int par, int chi);
  bool badPairB(int par, int chi);
  void printData(ECString path);
  void readData(ECString path);
 private:
  int unitRules[MAXNUMNTS];
  //int treeData_[MAXNUMNTS][MAXNUMNTS];
  bool bef_[MAXNUMNTS][MAXNUMNTS];
};

#endif /*UNITRULES_H */
