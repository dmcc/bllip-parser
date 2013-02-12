/*
 * Copyright 2005 Brown University, Providence, RI.
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

#ifndef VALHEAP_H 
#define VALHEAP_H


class Val;
typedef vector<Val*> Vals;
typedef Vals::iterator ValsIter;
class           ValHeap
{
public:
  ~ValHeap();
  ValHeap() : unusedPos_(0){}
  void         push(Val* ans);
  Val*   pop();
  int          size() { return unusedPos_; }
  static bool print;
  Val*   index(int i) { return array[i]; }
private:
  void  del_(int pos);
  void  downHeap(int pos);
  bool  upheap(int pos);
  int   left_child(int par) const { return (par*2) + 1; }
  int   right_child(int par) const { return ((par*2) + 2); }
  int   parent(int child) const { return ((child-1)/2); }
  int   unusedPos_;
  Vals  array;
};



#endif /* !VALHEAP_H */
