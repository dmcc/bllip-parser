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

#include "CombineBests.h"

int AnsTreeStr::numCreated = 0;

void
AnsTreeQueue::
push(double prob, AnsTree& tree)
{
  /* by this point we already have checked that prob is better
     than worstP */
  //cerr << "PSH " << prob << " " << size << endl;
  if(size < NTH)
    {
      ansList[size].first = prob;
      ansList[size].second = tree;
      if(prob < worstP || size == 0)
	{
	  worstP = prob;
	  worstPos = size;
	}
      size++;
      return;
    }
  ansList[worstPos].first = prob;
  ansList[worstPos].second = tree;
  for(int i = 0 ; i < size ; i++)
    {
      if(ansList[i].first < worstP || i == 0)
	{
	  worstP = ansList[i].first;
	  worstPos = i;
	}
    }
}

AnsTreePair&
AnsTreeQueue::
pop()
{
  assert(size > 0);
  int bestPos = -1;
  double bestProb = 0;
  for(int i = 0 ; i < NTH ; i++)
    {
      AnsTreePair& atp = ansList[i];
      if(atp.first < bestProb) continue;
      bestPos = i;
      bestProb = atp.first;
    }
  assert(bestPos >= 0);
  size--;
  return ansList[bestPos];
}

//-------------------------------
	  
void
CombineBests::
addTo(AnsTreeStr& ats, double prob)
{
  for(int i = 0 ; i < NTH ; i++)
    {
      double atsp = ats.probs[i];
      if(atsp < 0) break;
      double p1 = prob * atsp;
      if(atq.size == NTH && p1 < atq.worstP) break;
      atq.push(p1, ats.trees[i]);
      //cerr << "CBAT " << p1 << " " << ats.trees[i].trm << endl;
    }
}
      

void
CombineBests::
setBests(AnsTreeStr& ats)
{
  int lim = atq.size;
  for(int i = 0 ; i < lim ; i++)
    {
      AnsTreePair& atp = atq.pop();
      ats.probs[i] = atp.first;
      ats.trees[i] = atp.second;
      assert(ats.trees[i].trm >= 0);
      atp.first = -1;
      //cerr << "CBSB " << i << " " << atp.first << " " << atp.second.trm << endl;
    }
}

//-----------------------

void
CombineBestsGh::
setBests(AnsTreeStr& ats)
{
  int lim = atq.size;
  for(int i = 0 ; i < lim ; i++)
    {
      AnsTreePair& atp = atq.pop();
      ats.probs[i] = atp.first;
      ats.trees[i] = atp.second;
      assert(ats.trees[i].trm >= 0);
      atp.first = -1;
      //cerr << "CBSBGH " << i << " " << atp.first << " " << atp.second.trm << endl;
    }
}
	  
void
CombineBestsGh::
addTo(CombineBestsT& cbt)
{
  for(int i = 0 ; i < cbt.curAtq().size ; i++)
    {
      double p1 = cbt.curAtq().index(i).first;
      if(atq.size < NTH || p1 > atq.worstP)
	atq.push(p1, cbt.curAtq().index(i).second);
      //cerr << "ATGH " << i << " " << p1 << endl;
    }
}


//-----------------------

CombineBestsT::
CombineBestsT(const Term* tm, double prb) : rprob(prb), trm(tm), whichIsCur(false)
{
  AnsTree at;
  at.trm = trm->toInt();
  curAtq().push(prb, at);
  //cerr << "CBT " << this << " " << curAtq().size << endl;
}

void
CombineBestsT::
extendTrees(AnsTreeStr& ats,int dir)
{
  switchQueues(); //???;
  //cerr << "ET0 " << this << " " << pastAtq().size << " " << curAtq().size << endl;
  for(int i = 0 ; i < NTH ; i++)
    {
      double p1 = ats.probs[i];
      //cerr << "ET1 " << i << " " << p1 <<endl;
      if(p1 < 0) break;
      bool addedNew = false;
      AnsTree& subat = ats.trees[i];
      for(int j = 0 ; j < pastAtq().size ; j++)
	{
	  AnsTreePair& atp = pastAtq().index(j);
	  double p2 = atp.first *p1;
	  //cerr << "ET2 " << p2 << " " << curAtq().size << endl;
	  if(curAtq().size == NTH && p2 <= curAtq().worstP) continue;
	  addedNew = true;
	  AnsTree tree2 = atp.second;
	  if(dir == 2) tree2.subTrees.push_back(&subat);
	  else tree2.subTrees.push_front(&subat);
	  if(dir == 0) tree2.wrd = subat.wrd;
	  curAtq().push(p2, tree2);
	  //cerr << "CBET " << i << " " << p2 << " " << tree2.trm << endl;
	}
      if(!addedNew) break;
    }
}
