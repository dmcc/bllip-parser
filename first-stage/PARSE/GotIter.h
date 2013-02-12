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

#ifndef GOTITER_H
#define GOTITER_H

#include "Edge.h"
#include "Item.h"
#include <vector>

class           GotIter
{
 public:
  GotIter(Edge* edge);
  bool     next(Item*& itm);
 private:
  Edge*        whereIam;
};

class           LeftRightGotIter
{
 public:
  LeftRightGotIter(Edge* edge);
  bool    next(Item*& itm);
  Item*   index(int i) const { assert(i < 400); return lrarray[i]; }
  int     size() const { return size_; }
  int&    pos() { return pos_; }
 private:
  void         makelrgi(Edge* edge);
  Item*        lrarray[400];
  int          pos_;
  int          size_;
};

class           MiddleOutGotIter
{
 public:
  MiddleOutGotIter(Edge* edge);
  bool    next(Item*& itm, int& pos);
  int     size() const { return size_; }
  int     dir() { return dir_; }
 private:
  void         makelrgi(Edge* edge);
  Item*        lrarray[400];
  int          pos_;
  int          size_;
  int          dir_;
  Item*        firstRight_;
};

class           SuccessorIter
{
 public:
  SuccessorIter(Edge* edge) : edge_(edge), edgeIter( edge->sucs().begin() ) {}
  bool    next(Edge*& itm);
 private:
  Edge*  edge_;
  list<Edge*>::iterator edgeIter;
};
    
class          NeedmeIter
{
 public:
  NeedmeIter(Item* itm);
  bool    next(Edge*& e);
 private:
  //int          stackptr;
  //  Edge*        stack[64000]; //??? was 32;
  vector<Edge*>    stack;
};
   
#endif	/* ! GOTITER_H */
