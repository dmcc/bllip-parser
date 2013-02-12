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

#include "UnitRules.h"
#include "Term.h"

void
UnitRules::
init()
{
  numRules_ = 0;
  for(int i = 0 ; i < MAXNUMNTS ; i++)
    for(int j = 0 ; j < MAXNUMNTS ; j++) treeData_[i][j] = 0;
}

void
UnitRules::
readData(ECString path)
{
  ECString fl(path);
  fl += "unitRules.txt";
  ifstream data(fl.c_str());
  assert(data);
  int lim = Term::lastNTInt() - Term::lastTagInt();
  for(numRules_ = 0 ; numRules_ < lim ; numRules_++)
    {
      if(!data) break;
      data >> unitRules[numRules_];
    }
}

void
UnitRules::
gatherData(InputTree* tree)
{
  const Term* trm = Term::get(tree->term());
  assert(trm);
  int parInt = trm->toInt();
  int rparI = parInt-( Term::lastTagInt() + 1);
  InputTreesIter iti = tree->subTrees().begin();
  int len = tree->subTrees().size();
  for( ; iti != tree->subTrees().end() ; iti++)
    {
      InputTree* stree = (*iti);
      if(len == 1)
	{
	  const Term* strm = Term::get(stree->term());
	  if(strm->terminal_p()) continue;
	  assert(strm);
	  int chiInt = strm->toInt();
	  if(chiInt == parInt) continue;
	  int rchiI = chiInt -( Term::lastTagInt() + 1);
	  treeData_[rparI][rchiI]++;
	  //cerr << "TD " << parInt<<" " << chiInt << " " << treeData_[rparI][rchiI] << endl;
	}
      gatherData(stree);
    }
}

  
bool
UnitRules::
badPair(int par, int chi)
{
  bool seenPar = false;
  for(int i = 0 ; i < numRules_ ; i++)
    {
      int nxt = unitRules[i];
      if(nxt == chi) return !seenPar;
      if(nxt == par) seenPar = true;
    }
  return true;
}

void
recMark(int p, int c, int bef[MAXNUMNTS][MAXNUMNTS], int lim)
{
  assert(bef[p][c] != 0);
  if(bef[p][c] >= 1) return;
  bef[p][c] = 1;
  bef[c][p] = 0;
  for(int k = 0 ; k < lim ; k++)
    {
      if(bef[c][k] > 0) recMark(p, k, bef, lim);
      if(bef[k][p] > 0) recMark(k, c, bef, lim);
    }
}

void
UnitRules::
setData(ECString path)
{
  int p,c,k;
  int bef[MAXNUMNTS][MAXNUMNTS];
  for(p = 0 ; p < MAXNUMNTS ; p++)
    {
      for(c = 0 ; c < MAXNUMNTS ; c++) bef[p][c] = -1;
      bef[p][p] = 0;
    }
  
  int fix = Term::lastTagInt()+1;
  int lim = Term::lastNTInt() - Term::lastTagInt();
  int numToDo = lim*(lim-1);
  int numDone = 0;
  //cerr << lim << " " << numToDo << endl;
  while(numDone < numToDo)
    {
      int bestPar = -1;
      int bestChi = -1;
      int bestVal = -1;
      for(p = 0 ; p < lim ; p++)
	for(c = 0 ; c < lim ; c++)
	  {
	    if(bef[p][c] >= 0) continue;
	    int val =treeData_[p][c];
	    if(val > bestVal)
	      {
		bestVal = val;
		bestPar = p;
		bestChi = c;
	      }
	  }
      if(bestVal <= 3) break;
      //cerr << "NBV " << bestPar+fix << " " << bestChi+fix << " " << bestVal << endl;
      recMark(bestPar, bestChi, bef, lim);
    }
  ECString fl(path);
  fl += "unitRules.txt";
  ofstream data(fl.c_str());
  assert(data);
  for(p = 0 ; p < MAXNUMNTS ; p++)
    for(c = 0 ; c < MAXNUMNTS ; c++)
      if(bef[p][c] > 0) data << p << "\t" << c << "\n";
}
	  
	  

  


