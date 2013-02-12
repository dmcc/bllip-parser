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

#ifndef TIMEIT_H
#define TIMEIT_H

#include <time.h>
#include "Bchart.h"

class TimeIt
{
 public:
  TimeIt() :
    totEdges(0),
    totPoppedEdges(0),
    totAccessTime(0),
    totParseTime(0),
    totSemParseTime(0)
      {}
  void befSent();
  void betweenSent(Bchart* chart);
  void aftSent();
  void finish(int totSents);
  int    totEdges;
  int    totPoppedEdges;
  double totAccessTime;
  double totParseTime;
  double totSemParseTime;
  clock_t lastTime;
  clock_t currTime;
  double lastTimeSec;
  double currTimeSec;
  double elapsedTime;
};

#endif /* ! TIMEIT_H */
