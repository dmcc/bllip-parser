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

#ifndef FEATURETREE_H
#define FEATURETREE_H

#include <assert.h>
#include <fstream>
#include <iostream>
#include "Feat.h"
#include "FBinaryArray.h"

class FeatureTree;

#define ROOTIND -99
#define AUXIND -9
#define NULLIND 9999999
class FeatureTree
{
public:
  FeatureTree() :
    auxNd(NULL),
    back(NULL),
    ind_(NULLIND),
    count(0)
      {}
  FeatureTree(int i) :
    auxNd(NULL),
    back(NULL),
    ind_(i),
    count(0)
      {}
  FeatureTree(int i, FeatureTree* b)
    :
    auxNd(NULL),
    back(b),
    ind_(i)
      {}
  FeatureTree(istream& is);
  //FeatureTree(istream& is, istream& idxs, int asVal);
  void read(istream& is, FTypeTree* ftt);
  int  readOneLevel0(istream& is, int c);
  FeatureTree* follow(int val, int auxCnt);
  static FeatureTree* roots(int which) { return roots_[which]; }
  void         printFfCounts(int asVal, int depth, ostream& os);
  friend ostream&  operator<<(ostream& os, const FeatureTree& ft);

  int ind() const { return ind_; }
  FeatureTree* auxNd;
  FeatureTree* back;
  int ind_;
  double count;
  //int specFeatures;
   FBinaryArray feats;
  FTreeBinaryArray subtree;
 private:
  static FeatureTree* roots_[20];
  void othReadFeatureTree(istream& is, FTypeTree* ftt, int cnt);
  void printFfCounts2(int asVal, int depth, ostream& os);
};

#endif /* ! FEATURETREE_H */
