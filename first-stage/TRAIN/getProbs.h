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

#ifndef GETPROBS_H
#define GETPROBS_H

#include <fstream>
#include <sys/resource.h>
#include <iostream>
#include <unistd.h>
#include <set>
#include <math.h>
#include "ClassRule.h"
#include "ECArgs.h"
#include "Feature.h"
#include "FeatureTree.h"
#include "InputTree.h"
#include "headFinder.h"
#include "treeHistSf.h"
#include "Pst.h"
#include "Smoother.h"
#include "TreeHist.h"
#include "Term.h"

#include "trainRsUtils.h"


class getProbs{

 public:
  static void init(ECString path);

};
float getProb(InputTree* tree, int pos, int whichInt);
void initGetProb(string path);
 

#endif
