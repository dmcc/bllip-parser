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

#ifndef TRAINRSUTILS_h
#define TRAINRSUTILS_h
#include <fstream>
#include <sys/resource.h>
#include <iostream>
#include <unistd.h>
#include <set>
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
#include "ClassRule.h"
#include <cmath>



struct TrData 
{
  float c;
  float pm;
};




typedef set<ECString, less<ECString> > StringSet;



bool
inWordSet(int i);
void
makeSent(InputTree* tree);
void
wordsFromTree(InputTree* tree);
void
processG(bool getProb, int whichInt, int i, FeatureTree* ginfo[], TreeHist* treeh, int cVal);
void
callProcG(TreeHist* treeh);
void
gatherFfCounts(InputTree* tree, int inHpos);

void
printCounts();
void
printLambdas(ostream& res);
void
updateLambdas();

int
resetLogBases(int n);
void
zeroData();
void
pickLogBases(InputTree** trainingData, int sc);
void
lamInit();
void
goThroughSents(InputTree* trainingData[1301], int sc);
#endif
