/*
 * Copyright 1997, Brown University, Providence, RI.
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

#include "Phegt.h"

int
Phegt::
greaterThan(const Phegt& r) const
{
  const char* re = r.e;
  int rt = r.t;
  return greaterThan(rt, re);
}

int
Phegt::
greaterThan(int rt, const char* re) const
{
  int ans = 0;
  if(t < rt) ans = -1;
  else if(t > rt) ans = 1;
  else if(e[0] < re[0]) ans = -1;
  else if(e[0] > re[0]) ans = 1;
  else if(e[1] < re[1]) ans = -1;
  else if(e[1] > re[1]) ans = 1;
  return ans;
}

