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

#include "Smoother.h"
#include "math.h"
#include <fstream>
#include <iostream>

float Smoother::bucketLims[14] =
  {0, .003, .01, .033, .09, .33, 1.01, 2.01, 5.1, 12, 30, 80, 200, 600};


int 
Smoother::
bucket(float val)
{
  for(int i = 0 ; i < 14 ; i++)
    if(val <= bucketLims[i])
      return i;
  return 14;
}



//JT new bucket taken from getProbs
int 
Smoother::
bucket(float val, int whichInt, int whichFt)
{
  float logFac = 1.0 / log(Feature::logFacs[whichInt][whichFt]);

  float lval = logFac *log(val);

  int lvi = (int)lval;
  lvi++;
  if(lvi <= 14) return lvi;
  return 14;
}


