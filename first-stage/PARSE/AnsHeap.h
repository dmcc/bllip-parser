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

#ifndef ANSHEAP_H 
#define ANSHEAP_H

#define AHeapSize 100
#include "AnswerTree.h"

class           AnsTreeHeap
{
public:
  ~AnsTreeHeap();
  AnsTreeHeap() : nth(NTH), unusedPos_(0){}
  void         insert(AnsTreePair* ans);
  AnsTreePair*   pop();
  int          size() { return unusedPos_; }
  AnsTreePair*   front() { return array[0]; }
  static bool print;
private:
  void  del_(int pos);
  void  downHeap(int pos);
  bool  upheap(int pos);
  int   left_child(int par) const { return (par*2) + 1; }
  int   right_child(int par) const { return ((par*2) + 2); }
  int   parent(int child) const { return ((child-1)/2); }
  int   nth;
  int   unusedPos_;
  AnsTreePair*  array[AHeapSize];
};



#endif /* !ANSHEAP_H */
