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

#ifndef MECHART_H
#define MECHART_H

#include "Bchart.h"
#include "FullHist.h"
#include "FeatureTree.h"
#include "Feature.h"
#include "Item.h"
#include "Bst.h"

class MeChart : public Bchart
{
 public:
  MeChart(SentRep & sentence,int id)
    : Bchart( sentence,id ) {}
  MeChart(SentRep & sentence,ExtPos& extpos,int id)
    : Bchart( sentence,extpos,id ){}
  double triGram();
  static void init(ECString path);
  Bst& findMapParse();
  Bst& bestParse(Item* itm, FullHist* h,Val* cat,Val* gcat,int cdir);
  Bst& bestParseGivenHead(int posInt, const Wrd& wd, Item* itm,
				 FullHist* h,ItmGHeadInfo& ighInfo,
				 Val* cat,Val* gcat);
  void  fillInHeads();
  bool  headsFromEdges(Item* itm);
  bool  headsFromEdge(Edge* e);
  Item * headItem(Edge* edge);
  void  getHt(FullHist* h, int* subfVals, int whichInt = SCALC);
  float getpHst(const ECString& hd, int t);
  Bst& recordedBP(Item* itm, FullHist* h);
  Bst& recordedBPGH(Item* itm, BstMap& atm, FullHist* h);
  float meHeadProb(int wInt, FullHist* h);
  float meProb(int val, FullHist* h, int which);
  float meRuleProb(Edge* e, FullHist* h);
  void  getRelFeats(int c, int c2, int which, Feat* relFeat[],
		    FeatureTree* fts[], FullHist* h, int facPos);

  int     ccbucket(float val, float* buckets , int sz);
  static void    initCCArrays(ECString path);
  static void    initccarray(ifstream& is, float lenArray[6][8]);
  float   ccLenProb(Edge* edge, int effend);
  void    prDp();
};

#endif
