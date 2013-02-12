/*
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

#ifndef LINKH
#define LINKH

#include <vector>
#include "ECString.h"

class InputTree;

#define DUMMYVAL 999

class Link;
typedef  vector<Link*> Links;
typedef  Links::iterator LinksIter;

class Link
{
 public:
  Link(short key): key_(key){}
  ~Link()
    {
      LinksIter li = links_.begin();
      for( ; li != links_.end() ; li++) delete (*li);
    }
  Link* is_unique(InputTree* tree, bool& ans, int& cnt);
  short key() const { return key_; }
 private:
  Link* do_link(int tint, bool& ans);
  short key_;
  Links links_;
};

#endif
