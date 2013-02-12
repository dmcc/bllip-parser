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

#include <map>
#include <assert.h>
#include <fstream>
#include <iostream>
#include "Feat.h"
#include <set>
#include "Feature.h"

class FeatureTree;
typedef map<int,FeatureTree*, less<int> > FTreeMap;
typedef map<int, int, less<int> > IntIntMap;
typedef map<int, Feat, less<int> > FeatMap;
typedef set<int, less<int> > IntSet;
typedef map<int,IntSet, less<int> > IntSetMap;


class FeatureTree
{
public:
  FeatureTree() : specFeatures(0), auxNd(NULL), back(NULL), marked(-1),
    count(0), featureInt(-1) {}
  FeatureTree(int i) : ind(i), specFeatures(0), auxNd(NULL), back(NULL),
     marked(-1), count(0), featureInt(-1) {}
  FeatureTree(int i, FeatureTree* b)
    : ind(i), specFeatures(0), auxNd(NULL), back(b), marked(-1),
      count(0), featureInt (-1) {}
  FeatureTree(istream& is);
  int  readOneLevel0(istream& is);
  FeatureTree* next(int val, int auxCnt);
  FeatureTree* follow(int val, int auxCnt);
  static FeatureTree* roots(int which) { return roots_[which]; }
  static FeatureTree*& root() { return roots_[Feature::whichInt]; }
  void   printFTree(int asVal, ostream& os);
  friend ostream&  operator<<(ostream& os, const FeatureTree& ft);
  int ind;
  int count;
  int featureInt;
  int specFeatures;
  float marked;
  FeatureTree* back;
  FeatureTree* auxNd;
  FeatMap feats;
  //BinaryArray featA;
  FTreeMap subtree;
  static int       totParams;
  static int       minCount;
 private:
  static FeatureTree* roots_[15];
  void read(istream& is, FTypeTree* ftt);
  void othReadFeatureTree(istream& is, FTypeTree* ftt, int c);
  void         printFfCounts(int asVal, int depth,
			     ostream& os, FTypeTree* ftt);
  void printFfCounts2(int asVal, int depth, ostream& os, FTypeTree* ftt);
};

#endif /* ! FEATURETREE_H */
