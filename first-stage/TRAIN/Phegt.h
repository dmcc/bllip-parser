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

#ifndef PHEGT_H
#define PHEGT_H

#include "ECString.h"

class Phegt
{
 public:
  Phegt() : t(-1), p(0){}
  Phegt(int t1, ECString& es) : t(t1), p(0)
    {
      e[0] = es[0];
      e[1] = es[1];
    }
  friend int operator== ( const Phegt& l, const Phegt& r)
    { return (l.greaterThan(r) == 0);}
  friend int operator> ( const Phegt& l, const Phegt& r)
    {return (l.greaterThan(r) > 0);}
  friend int operator< ( const Phegt& l, const Phegt& r)
    {return (l.greaterThan(r) < 0);}
  int t;
  char e[2];
  float p;
  int greaterThan(const Phegt& r) const;
  int greaterThan(int t, const char* e) const;
};


#endif /* ! PHEGT_H */
