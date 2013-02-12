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

/* only needed for sel */
void
UnitRules::
init()
{
}

void
UnitRules::
printData(ECString path)
{
  ECString fl(path);
  fl += "unitRules.txt";
  ofstream data(fl.c_str());
  assert(data);
  int lim = Term::lastNTInt() - Term::lastTagInt();
  for(int i = 0 ; i < lim ; i++)
    {
      assert(data);
      data  << unitRules[i] << "\n";
    }
  ECString tmp;
  data << tmp;
}
     

void
UnitRules::
readData(ECString path)
{
  int p, c;
  for(p = 0 ; p < MAXNUMNTS ; p++)
    for(c = 0 ; c < MAXNUMNTS ; c++) bef_[p][c] = false;
  
  ECString fl(path);
  fl += "unitRules.txt";
  ifstream data(fl.c_str());
  assert(data);
  for( ; ; )
    {
      data >> p;
      if(!data) break;
      data >> c;
      bef_[p][c] = true;
      //cerr << "PCT " << p << " " << c << endl;
    }
}

bool
UnitRules::
badPair(int par, int chi)
{
  int parInt = par-Term::lastTagInt()-1;
  int chiInt = chi-Term::lastTagInt()-1;
  //cerr << "BP " << parInt << " " << chiInt << endl;
  return !bef_[parInt][chiInt];
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
