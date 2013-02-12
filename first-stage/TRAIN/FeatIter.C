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

#include "FeatIter.h"

FeatIter::
FeatIter(FeatureTree* ft) : alive_(1), fti(ft)
{
  fmi = fti.curr->feats.begin();
  curr = &((*fmi).second);
}

void
FeatIter::
next()
{
  fmi++;
  if(fmi == fti.curr->feats.end())
    {
      fti.next();
      if(!fti.alive())
	{
	  alive_ = 0;
	  return;
	}
      fmi = fti.curr->feats.begin();
    }
  curr = &((*fmi).second);
}
