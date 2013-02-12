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

#include "FeatTreeIter.h"

void
FeatTreeIter::
next()
{
  //cerr << "FTI next on " << *curr << " " << currDepth << endl;
  int ans = 1;
  for( ; ; )
    {
      FTreeMap::iterator& fti = maps[currDepth];
      if(fti != curr->subtree.end())
	{
	  curr = (*fti).second;
	  fti++;
	  currDepth++;
	  maps[currDepth] = curr->subtree.begin();
	  break;
	}
      if(curr->auxNd)
	{
	  curr = curr->auxNd;
	  maps[currDepth] = curr->subtree.begin();
	  continue;
	}
      while(curr->ind == AUXIND)
	{
	  curr = curr->back;
	}
      curr = curr->back;
      if(!curr)
	{
	  ans = 0;
	  break;
	}
      currDepth--;
    }
  if(!ans)
    {
      alive_ = 0;
      return ;
    }
  //cerr << "found " << *curr << " " << currDepth << endl;
  if(curr->feats.empty()) next();
}
