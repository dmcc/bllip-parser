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

#ifndef COMBINEBESTS_H
#define COMBINEBESTS_H

#include "AnswerTree.h"
#include "Term.h"
#include "AnsHeap.h"

typedef map<double,AnsTree> AnsMap;

class AnsTreeQueue
{
 public:
  AnsTreeQueue() : size(0), worstPos(-1), worstP(-1) 
    {
      for(int i = 0 ; i < NTH ; i++) ansList[i].first = -1;
    }
  void refresh()
    {
      for(int i = 0 ; i < size ; i++)
	{
	  ansList[i].first = -1;
	  ansList[i].second.subTrees.clear();
	}
      size = 0;
      worstPos = -1;
      worstP = -1;
    }
  AnsTreePair& pop();
  AnsTreePair& index(int i) { return ansList[i]; }
  void push(double prob, AnsTree& tree);
  int size;
  int worstPos;
  double worstP;
  AnsTreePair ansList[NTH];
};

class CombineBests
{
 public:
  void setBests(AnsTreeStr& atp);
  void addTo(AnsTreeStr& atp, double prob);
  AnsTreeQueue atq;
};

class CombineBestsT
{
 public:
  CombineBestsT(const Term* tm, double rprb);
  void extendTrees(AnsTreeStr& ats, int dir);
  double rprob;
  const Term* trm;
  AnsTreeQueue atq0;
  AnsTreeQueue atq1;
  bool whichIsCur;
  AnsTreeQueue& curAtq() { return whichIsCur ? atq1 : atq0; }
  AnsTreeQueue& pastAtq() { return whichIsCur ? atq0 : atq1; }
  void switchQueues()
    {
      whichIsCur = !whichIsCur;
      //cerr << "WIC " << curAtq().size << " " << pastAtq().size << endl;
      curAtq().refresh();
    }
};

class CombineBestsGh
{
 public:
  void setBests(AnsTreeStr& atp);
  void addTo(CombineBestsT& cbt);
  AnsTreeQueue atq;
};

#endif /* ! COMBINEBESTS_H */
