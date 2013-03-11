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

#include <iostream>
#include <fstream>

#include "ECArgs.h"
#include "Term.h"
#include "utils.h"
#include "InputTree.h"
#include "headFinder.h"
#include "headFinderCh.h"
#include "UnitRules.h"
#include "Feature.h"

extern bool okFoldSent(int sntNum, int fld, int fOp);
int foldOp = 0;

void setNonTermInts();

/* read through wsj training data.
   compute p(x is head of NT | pos(x) =t) and put it in pTgNt.txt */


typedef map< int, int, less<int> > PosD;
typedef map<ECString, PosD, less<ECString> > WordMap;
WordMap wordMap;
int                 numTerm[MAXNUMNTS];

void
incrWordData(int lhsInt, ECString wupper)
{
  char temp[1024];
  ECString w(langAwareToLower(wupper.c_str(), temp));
  numTerm[lhsInt]++;
  WordMap::iterator wmi = wordMap.find(w);
  if(wmi == wordMap.end())
    {
      wordMap[w][lhsInt] = 1;
      return;
    }
  PosD& posd = (*wmi).second;
  PosD::iterator pdi = posd.find(lhsInt);
  if(pdi == posd.end())
    {
      posd[lhsInt] = 1;
    }
  else
    (*pdi).second++;
}


void
addWwData(InputTree* tree)
{
  InputTrees& st = tree->subTrees();
  InputTrees::iterator  subTreeIter= st.begin();
  InputTree  *subTree;
  for( ; subTreeIter != st.end() ; subTreeIter++ )
    {
      subTree = *subTreeIter;
      addWwData(subTree);
    }
  if( tree->word() != ""  )
    {
      ECString w = tree->word();
      const Term* trm = Term::get(tree->term());
      assert(trm);
      int trmInt = trm->toInt();
      incrWordData(trmInt, w);
    }
}

int
main(int argc, char *argv[])
{
  ECArgs args( argc, argv );
  assert(args.nargs() == 1);
  ECString path(args.arg(0));
  repairPath(path);
  cerr << "At start of pSgT" << endl;

  for(int n = 0 ; n < MAXNUMNTS ; n++)
    numTerm[n] = 0;

  Term::init( path );
  if(args.isset('L')) Term::Language = args.value('L');
  readHeadInfo(path);

  int sentenceCount = 0;

  ECString s1lex("^^");
  ECString s1nm("S1");
  int s1Int = Term::get(s1nm)->toInt();
	
  UnitRules ur;
  ur.init();
  while(cin)
    {
      if(sentenceCount%10000 == 0) cerr << sentenceCount << endl;

      InputTree  parse;
      cin >> parse;
      //cerr << parse << endl;
      if(!cin) break;
      int len = parse.length();
      if(len == 0) break;
      else if(len == -1) continue;
      EcSPairs wtList;
      parse.make(wtList); 
      InputTree* par;
      par = &parse;

      addWwData(par);
      incrWordData(s1Int, s1lex);
      ur.gatherData(par);
      sentenceCount++;
    }
  ECString resultsString(path);
  resultsString += "pSgT.txt";
  ofstream     resultsStream(resultsString.c_str());
  assert(resultsStream);

  resultsStream << "       \n";  //leave space for number of words;
  resultsStream.precision(3);
  ECString lastWord;
  WordMap::iterator wmi = wordMap.begin();
  resultsStream << wordMap.size() << "\n\n";
  for( ; wmi != wordMap.end() ; wmi++)
    {
      ECString w = (*wmi).first;
      resultsStream << w << "\t";
      PosD& posd = (*wmi).second;
      PosD::iterator pdi = posd.begin();
      int count = 0;
      for( ; pdi != posd.end(); pdi++)
	{
	  int posInt = (*pdi).first;
	  int c = (*pdi).second;
	  count += c;
	  float p = (float)c/(float)numTerm[posInt];
	  resultsStream << posInt << " " << p << " ";
	}
      resultsStream << "| " << count << "\n";
    }
  ur.setData(path);
  cerr << "Number of sentences = " << sentenceCount << endl;
  return 0;
}
