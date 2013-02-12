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

#include "Feat.h"
#include "utils.h"
#include <ostream>

int Feat::Usage = 0;
/*
Feat::
Feat() //: cnt_(0)
{
  if(Usage == ISCALE || Usage == PARSE)
    {
      if(Usage == ISCALE)
	{
	  uVals = new float[MAXNUMFS+2];
	  for(int i = 0 ; i < MAXNUMFS+2 ; i++)
	    uVals[i] = 0;
	}
      else
	uVals = new float[1];
      g() = 1;
    }
  else uVals = NULL;
}
*/
ostream&
operator<< ( ostream& os, Feat& t )
{
  os << "{" << t.ind()/*<< "," << t.cnt()*/ << "}";
  return os;
}
