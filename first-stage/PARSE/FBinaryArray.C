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

#include "FBinaryArray.h"
#include "Feat.h"
#include "FeatureTree.h"
void
FBinaryArray::
set(int sz) { size_ = sz; array_ = new Feat[sz]; };

Feat*
FBinaryArray::
index(int i) { return &array_[i]; }

Feat*
FBinaryArray::
find(const int id) const
{
  int top = size_;
  int bot = -1;
  int  midInd;
  for( ; ; )
    {
      if( top <= bot+1 )
	{
	  return NULL;
	}
      int mid = (top+bot)/2;
      Feat* midH = &array_[mid];
      midInd = midH->ind();
      if( id == midInd) return midH;
      else if( id < midInd) top = mid;
      else bot = mid;
    }
}

void
FTreeBinaryArray::
set(int sz) { size_ = sz; array_ = new FeatureTree[sz]; };

FeatureTree*
FTreeBinaryArray::
index(int i) { return &array_[i]; }

FeatureTree*
FTreeBinaryArray::
find(const int id) const
{
  int top = size_;
  int bot = -1;
  int  midInd;
  for( ; ; )
    {
      if( top <= bot+1 )
	{
	  return NULL;
	}
      int mid = (top+bot)/2;
      FeatureTree* midH = &array_[mid];
      midInd = midH->ind();
      if( id == midInd) return midH;
      else if( id < midInd) top = mid;
      else bot = mid;
    }
}
