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

#include <math.h>
#include <fstream>
#include <sys/resource.h>
#include <iostream>
#include <unistd.h>
#include "ECArgs.h"
#include "ECString.h"
#include "utils.h"
#include "FeatIter.h"
#include "Feature.h"
#include "Term.h"

#define NUMPASSES 4
#define NUMNEWTPASSES 4

int whichInt;

FeatureTree* features;

ECString conditionedType;
int passN;
int hic = 0;

bool
badPreReqs(FeatureTree* ft, FeatureTree* parft)
{
  int fInt = ft->featureInt;
  //cerr << "ft to feat gives " << *ft << " " << fInt << endl;
  int cprf = Feature::fromInt(fInt, whichInt)->condPR;
  int pfInt = parft->featureInt;
  //cerr << "ft to feat gives " << *parft << " " << pfInt << endl;
  int cprpf = Feature::fromInt(pfInt, whichInt)->condPR;
  if(cprpf < 0) return false;
  if(cprpf == cprf) return false;
  //cerr << "badPr for " << *ft << " " << *parft << " " << fInt << " "
    //<< pfInt << endl;
  return true;
}

Feat*
parentFeat(Feat* f)
{
  FeatureTree* ft = f->toTree();
  assert(ft);
  FeatureTree* parft = ft->back;
  if(!parft) return NULL; 
  if(parft->ind == ROOTIND) return NULL;
  while(parft->ind == AUXIND) parft = parft->back;
  //if(badPreReqs(ft,parft)) return NULL;
  FeatMap::iterator fmi = parft->feats.find(f->ind());
  if(fmi == parft->feats.end()) return NULL;
  return &((*fmi).second);
}

void
initFeatVals()
{
  FeatIter fi(features); 
  Feat* f;
  for( ; fi.alive() ; fi.next() )
    {
      f = fi.curr;
      int fhij = f->cnt();
      assert(fhij > 0);
      int hij = f->toTree()->count;
      assert(hij > 0);
      assert(hij >= fhij);
      Feat* fj = parentFeat(f);
      int hj, fhj;
      if(fj)
	{
	  hj = fj->toTree()->count;
	  fhj = fj->cnt();
	}
      else
	{
	  fhj = 1;
	  hj = 1; // this sets val to fhij/hij;
	  //hj=FeatureTree::totCaboveMin[whichInt][Feature::assumedFeatVal]+1;
	}
      assert(hj > 0);
      assert(fhj > 0);
      assert(hj >= fhj);
      //float val = (float)(fhij * hj)/(float)(fhj * hij);
      float val = ((float)fhij/(float)hij);
      fi.curr->g() = val;
      //cerr << *(f->toTree()) << " " << f->ind()
	//   << " " << val << endl;
      if(!(val > 0))
	{
	  cerr << fhij << " " << hj << " " << fhj << " " << hij << endl;
	  assert(val > 0);
	}
    }
}

int
main(int argc, char *argv[])
{
   struct rlimit 	core_limits;
   core_limits.rlim_cur = 0;
   core_limits.rlim_max = 0;
   setrlimit( RLIMIT_CORE, &core_limits );

   ECArgs args( argc, argv );
   Feat::Usage = ISCALE;
   ECString path(args.arg(1));
   repairPath(path);
   Term::init(path);

   conditionedType = args.arg(0);
   cerr << "start iScale: " << conditionedType << endl;
   //Pst pst(path);

   Feature::init(path, conditionedType);
   whichInt = Feature::whichInt;
   ECString fHp(path);
   fHp += conditionedType;
   fHp += ".f";
   ifstream fHps(fHp.c_str());
   if(!fHps)
     {
       cerr << "Could not find " << fHp << endl;
       assert(fHps);
     }

   features = new FeatureTree(fHps);

   //features->subtree;
   
   initFeatVals();
   
   ECString gt(path);
   gt += conditionedType;
   gt += ".g";
   ofstream gtstream(gt.c_str());
   assert(gtstream);
   gtstream.precision(3);

   FTreeMap::iterator ftmi = features->subtree.begin();
   for( ; ftmi != features->subtree.end() ; ftmi++)
     {
       int afv = (*ftmi).first;
       (*ftmi).second->printFTree(afv, gtstream);
     }
   return 0;
}
