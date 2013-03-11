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

#include"trainRsUtils.h"
extern int pass;
extern int whichInt;
extern int sentenceCount;
extern ECString conditionedType;
extern FeatureTree* tRoot;
extern bool procGSwitch;
extern StringSet wordSet;


int
main(int argc, char *argv[])
{

   struct rlimit 	core_limits;
   core_limits.rlim_cur = 0;
   core_limits.rlim_max = 0;
   setrlimit( RLIMIT_CORE, &core_limits );

   ECArgs args( argc, argv );
   assert(args.nargs() == 2);
   conditionedType = args.arg(0);
   cerr << "start trainRs: " << conditionedType << endl;
  
   ECString path(args.arg(1));
   repairPath(path);
   if(args.isset('M')) Feature::setLM();
   if(args.isset('L')) Term::Language = args.value('L');

   Term::init(path);

   readHeadInfo(path);
   Pst pst(path);
   if(Feature::isLM) ClassRule::readCRules(path);

   addSubFeatureFns();
   Feature::init(path, conditionedType); 

   whichInt = Feature::whichInt;
   int ceFunInt = Feature::conditionedFeatureInt[Feature::whichInt];
   Feature::conditionedEvent
     = SubFeature::Funs[ceFunInt];

   Feat::Usage = PARSE;
   ECString ftstr(path);
   ftstr += conditionedType;
   ftstr += ".g";
   ifstream fts(ftstr.c_str());
   if(!fts)
     {
       cerr << "Could not find " << ftstr << endl;
       assert(fts);
     }
   tRoot = new FeatureTree(fts); //puts it in root;

   cout.precision(3);
   cerr.precision(3);

   lamInit();

   InputTree* trainingData[1001];
   int usedCount = 0;
   sentenceCount = 0;
   for( ;  ; sentenceCount++)
     {
       // cerr << "sentence number " << sentenceCount << ", used " << usedCount << endl;
       if(0&&sentenceCount%1000==1)
	 {
	    cerr << conditionedType << ".tr "
	      << sentenceCount << endl;
	 }
       if(usedCount >= 1000) break;
       InputTree*     correct = new InputTree;  
       cin >> (*correct);

       if(correct->length() == 0) break;
       if(!cin) break;
       EcSPairs wtList;
       correct->make(wtList); 
       InputTree* par;
       par = correct;
       trainingData[usedCount++] = par;
     }
   if(Feature::isLM) pickLogBases(trainingData,sentenceCount);
   procGSwitch = true;
   for(pass = 0 ; pass < 10 ; pass++)
     {
       if(pass%2 == 1) cout << "Pass " << pass << endl;
       goThroughSents(trainingData, sentenceCount);
       updateLambdas();
       //printLambdas(cout);
       zeroData();
     }
   ECString resS(path);
   resS += conditionedType;
   resS += ".lambdas";
   ofstream res(resS.c_str());
   res.precision(3);
   printLambdas(res);
   printLambdas(cout);
   cout << "Total params = " << FeatureTree::totParams << endl;

}
