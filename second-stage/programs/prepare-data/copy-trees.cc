// Licensed under the Apache License, Version 2.0 (the "License"); you may
// not use this file except in compliance with the License.  You may obtain
// a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
// License for the specific language governing permissions and limitations
// under the License.

// copy-trees.cc
//
// Mark Johnson, 12th November 2003
//
// reads trees one at a time from stdin and writes them to stdout
// with each sentence on a line of its own

#include "tree.h"

int main(int argc, char** argv) {

  tree* t;

  while ((t = readtree_root(stdin)) != NULL)
    std::cout << t << std::endl;

}
