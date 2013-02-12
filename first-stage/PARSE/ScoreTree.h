/*
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

#ifndef SCORET_H
#define SCORET_H

#include "InputTree.h"

class ScoreTree
{
 public:
  int equivInt(int x);
  int puncEquiv(int i, vector<ECString>& sr);
  void setEquivInts(vector<ECString>& sr);
  bool lexact2(InputTree* tree);
  void recordGold(InputTree* tree, ParseStats& parseStats);
  void precisionRecall( InputTree* tree, ParseStats& parseStats );
  bool scorePunctuation( const ECString trmString );
  multiset<int> trips;
  int equivPos[401];

};

#endif /* ! SCORET_H */


