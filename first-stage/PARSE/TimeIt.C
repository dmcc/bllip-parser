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

#include "TimeIt.h"
#include <fstream>

void
TimeIt::
befSent()
{
  lastTime = clock();
  currTime = clock();
  lastTimeSec = (double)lastTime/(double)CLOCKS_PER_SEC;
  currTimeSec = (double)currTime/(double)CLOCKS_PER_SEC;
  elapsedTime = currTimeSec - lastTimeSec;
  if(elapsedTime < 0) elapsedTime += 2147;
  totAccessTime += elapsedTime;
  lastTime = currTime;
}

void
TimeIt::
betweenSent(Bchart* chart)
{
  currTime = clock();
  lastTimeSec = (double)lastTime/(double)CLOCKS_PER_SEC;
  currTimeSec = (double)currTime/(double)CLOCKS_PER_SEC;
  elapsedTime = currTimeSec - lastTimeSec;
  if(elapsedTime < 0) elapsedTime += 2147;
  cerr << "Parsing time = " << elapsedTime
       << "\tEdges created = " << chart->totEdgeCountAtS()
       << "\tEdges popped = " << chart->poppedEdgeCountAtS() << endl;
  totParseTime += elapsedTime;
  //totEdges += chart->totEdgeCountAtS();
  //totPoppedEdges += chart->poppedEdgeCountAtS();
  totEdges += chart->totEdgeCountAtS();
  totPoppedEdges += chart->poppedEdgeCountAtS();
  lastTime = clock();
}


void
TimeIt::
aftSent()
{
  currTime = clock();
  lastTimeSec = (double)lastTime/(double)CLOCKS_PER_SEC;
  currTimeSec = (double)currTime/(double)CLOCKS_PER_SEC;
  elapsedTime = currTimeSec - lastTimeSec;
  if(elapsedTime < 0) elapsedTime += 2147;
  cerr << "Sem Parsing time = " << elapsedTime << endl;
  totSemParseTime += elapsedTime;
}

void
TimeIt::
finish(int totSents)
{
  cout << "Av access time = " << totAccessTime/totSents
       << "\t Av parse time = "
       << totParseTime/totSents
       << "\t Av stats time = "
       << totSemParseTime/totSents
       << "\nAv edges created = "
       << (float)totEdges/totSents
       << "\tAv edges popped = "
       << (float)totPoppedEdges/totSents
       << endl;
}
