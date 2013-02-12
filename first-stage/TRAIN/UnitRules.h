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
  void setData(ECString path);
  bool badPair(int par, int chi);
  void readData(ECString path);
  int& treeData(int i, int j) { return treeData_[i][j]; }
  int treeData(int i, int j) const { return treeData_[i][j]; }
 private:
  int numRules_;
  int unitRules[MAXNUMNTS];
  int treeData_[MAXNUMNTS][MAXNUMNTS];
  bool before_;
};

#endif /*UNITRULES_H */
