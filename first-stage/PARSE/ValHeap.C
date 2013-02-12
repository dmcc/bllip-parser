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

#include "Bst.h"

bool ValHeap::print = false;

ValHeap::
~ValHeap()
{
  ValsIter vi = array.begin();
  for(  ; vi != array.end() ; vi++) delete (*vi);
}

void
ValHeap::
push(Val* atp)
{
  assert(atp);
  if(print)
    cerr << "heap insertion of atp at " << unusedPos_ << endl;
  assert(static_cast<int>(array.size()) >= unusedPos_);
  if(static_cast<int>(array.size()) ==  unusedPos_) array.push_back(atp);
  else array[unusedPos_] = atp;
  upheap(unusedPos_);
  unusedPos_++;
}

bool
ValHeap::
upheap(int pos)
{
  if(print) cerr << "in Upheap " << pos << " " << array.size() << endl;
  if(pos == 0) return false;
  Val* atp = array[pos];
  assert(atp);
  double merit = atp->fom();
  int   parPos = parent(pos);
  Val* par = array[parPos];
  double pmerit = par->fom();
  if(print) cerr << "merits " << merit << " " << pmerit<< endl;
  if(merit > pmerit)
    {
      array[parPos] = atp;
      array[pos] = par;
      if(print) cerr << "Put " << pos << " in " << parPos << endl;
      upheap(parPos);
      return true;
    }
  else if(print)
    {
      cerr << "upheap of " << merit << "stopped by "
	<< parPos << " " << pmerit << endl;
    }
  return false;
}


Val*
ValHeap::
pop()
{
  if(print)
    cerr << "popping" << endl;
  if(unusedPos_ == 0) return NULL;
  Val* retVal = array[0];
  del_(0);
  return retVal;
}

void
ValHeap::
downHeap(int pos)
{
  if(print) cerr << "downHeap " << pos << endl;
  if(pos >= unusedPos_-1) return;
  Val* par = array[pos];
  double merit = par->fom();
  int lc = left_child(pos);
  int rc = right_child(pos);
  int largec;
  int lcthere = 0;
  Val* lct = NULL;
  if(lc < unusedPos_)
    {
      lct = array[lc];
      if(lct) lcthere = 1;
    }
  int rcthere = 0;
  Val* rct = NULL;
  if(rc < unusedPos_)
    {
      rct = array[rc];
      if(rct) rcthere = 1;
    }
  if(!lcthere && !rcthere) return;
  assert(lcthere);
  if(!rcthere || (lct->fom() > rct->fom()))
    largec = lc;
  else largec = rc;
  Val* largeatp = array[largec];
  if(merit >= largeatp->fom()) 
    {
      if(print) cerr << "downheap of " << merit << " stopped by "
		     << " " << largeatp->fom() << endl;
      return;
    }
  array[pos] = largeatp;
  array[largec] = par;
  downHeap(largec);
}

void
ValHeap::
del_(int pos)
{
  if(print) cerr << "del_ " << pos << endl;
  assert(unusedPos_);
  if(pos == (unusedPos_ - 1) )
    {
      unusedPos_--;
      array[unusedPos_] = NULL;
      return;
    }
  /* move the final edge in heap to empty position */
  array[pos] = array[unusedPos_ - 1];
  if(!array[pos])
    {
      error("Never get here");
      return;
    }
  unusedPos_--;
  array[unusedPos_] = NULL;
  downHeap(pos);
}
