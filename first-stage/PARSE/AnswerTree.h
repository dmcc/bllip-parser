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

#ifndef ANSWERTREE_H
#define ANSWERTREE_H

#include "Edge.h"
#include <vector>
#include <list>
#include "CntxArray.h"

#define NTH 50
class AnsTree;

typedef list<AnsTree*> AnsTrees;
typedef AnsTrees::iterator AnsTreeIter;
typedef pair<double, AnsTree> AnsTreePair;

class
AnsTree
{
 public:
  AnsTree() : trm(-1), wrd(-1) {};
  void operator=(const AnsTree& rhs)
    {
      trm = rhs.trm;
      wrd = rhs.wrd;
      subTrees = rhs.subTrees;
    }
  short trm;
  int wrd;
  AnsTrees subTrees;
};

class AnsTreeStr
{
public:
  AnsTreeStr() : sum(0)
    {
      int i;
      numCreated++;
      for(i = 0 ; i < NTH ; i++)
	{
	  probs[i] = -1;
	}
    }
  double sum;
  double probs[NTH];
  AnsTree  trees[NTH];
  static int numCreated;
};

typedef map<CntxArray, AnsTreeStr, less<CntxArray> > AnsTreeMap;
AnsTreeStr& atpFind(CntxArray& hi, AnsTreeMap& atm);

#endif
