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
#include "ECString.h"
#include "Term.h"
#include "utils.h"
#include "InputTree.h"
#include <map>
#include "headFinder.h"
#include "Pst.h" 
#include "string.h"

extern bool okFoldSent(int sntNum, int fld, int fOp);
int foldOp = 0;

int posDenoms[MAXNUMTS];
int posUCounts[MAXNUMTS];
int posDashCounts[MAXNUMTS];
int posCounts[MAXNUMTS];
int totCounts[MAXNUMTS];
int posCapCounts[MAXNUMTS];

void setNonTermInts();

/* read through wsj training data.
   compute p(x is head of NT | pos(x) =t) and put it in pTgNt.txt */

InputTree* curSent;

int numEndings = 0;

typedef map<ECString,int, less<ECString> > endMap;
endMap endData[MAXNUMTS];
int                 numTerm[MAXNUMNTS];

void
incrEndData(int lhsInt, ECString e)
{
  endMap::iterator emi = endData[lhsInt].find(e);
  if(emi == endData[lhsInt].end())
    {
      endData[lhsInt][e] = 1;
      numEndings++;
    }
  else
    {
      (*emi).second++;
    }

}

void
addWwData(InputTree* tree)
{
  ECString wTagNm = tree->term();
  const Term* trm = Term::get(wTagNm);
  int lhsInt = trm->toInt();
  totCounts[lhsInt]++;
  if( tree->word() != ""  )
    {
      ECString hdLexU(tree->word());
      char temp[1024];
      ECString hdLex(langAwareToLower(hdLexU.c_str(),temp));
      int len = hdLex.length();
      const WordInfo* wi = Pst::get(hdLex); //???;
      if (!wi)
          cerr << "Couldn't find entry for word '" << hdLex << 
                  "' in pSgT.txt" << endl;

      assert(wi);
      /* Ignore words very close to start of sentence, those
	 that are of length 1, and those who's capitalization is
	 ambiguous. */
      if(tree->start() >= 2 && len > 1
	 &&!(hdLex[0] != hdLexU[0] && hdLex[1] != hdLexU[1]))
	{
	  posCounts[lhsInt]++;
	  if(hdLex[0] != hdLexU[0] && hdLex[1] == hdLexU[1])
	    {
	      posCapCounts[lhsInt]++;
	    }
	}
      posDenoms[lhsInt]++;
      if(wi->c() <= 2)
	{
	  posUCounts[lhsInt]++;
	  const char* hyppos =  strpbrk(hdLex.c_str(), "-");
	  if(hyppos) posDashCounts[lhsInt]++;
	}
      return;
    }
  InputTrees& st = tree->subTrees();
  InputTrees::iterator  subTreeIter= st.begin();
  InputTree  *subTree;
  for( ; subTreeIter != st.end() ; subTreeIter++ )
    {
      subTree = *subTreeIter;
      addWwData(subTree);
    }
}

int
main(int argc, char *argv[])
{
  ECArgs args( argc, argv );
  ECString path(args.arg(0));
  repairPath(path);
  cerr << "At start of pUgT" << endl;

  Term::init( path );  
  if(args.isset('L')) Term::Language = args.value('L');
  readHeadInfo(path);
  Pst pst(path);

  int sentenceCount = 0;

  int i, j;
  for(i = 0 ; i < MAXNUMTS ; i++)
    {
      posCounts[i] = 0;
      posCapCounts[i] = 0;
      posDenoms[i] = 0;
      posUCounts[i] = 0;
      posDashCounts[i] = 0;
    }
  for(i = 0 ; i < MAXNUMTS ; i++) totCounts[i] = 0;

  i = 0;
  for( ; ; )
    {
      if(i++%10000 == 1) cerr << i << endl;
      //if(i > 1000) break;
      InputTree  parse;
      cin >> parse;
      //cerr << parse << endl;
      if(parse.length() == 0) break;
      if(!cin) break;
      curSent = &parse;
      addWwData(&parse);
      sentenceCount++;
    }

  ECString resultsString(path);
  resultsString += "pUgT.txt";
  ofstream     resultsStream(resultsString.c_str());
  assert(resultsStream);
  /* we print out p(unknown|tag)    p(Capital|tag)   p(hasDash|tag, unknown)
     note for Capital the denom is different because we ignore the first
     two words of the sentence */
  int nm = Term::lastTagInt()+1;
  for(i = 0 ; i < nm ; i++)
    {
      resultsStream << i << "\t";
      float pugt = 0;
      float pudenom = (float)posDenoms[i];
      if(pudenom > 0) pugt = (float)posUCounts[i]/pudenom;
      resultsStream << pugt << "\t";
      if(posCounts[i] == 0) resultsStream << 0 << "\t";
      else
	resultsStream << (float) posCapCounts[i]/ (float)posCounts[i] << "\t";
      if(posUCounts[i] == 0) resultsStream << 0;
      else resultsStream << (float)posDashCounts[i]/posUCounts[i] ;
      resultsStream << endl;
    }
  ECString resultsString2(path);
  resultsString2 += "nttCounts.txt";
  ofstream     resultsStream2(resultsString2.c_str());
  assert(resultsStream2);
  for(i = 0 ; i <= Term::lastNTInt() ; i++)
    {
      resultsStream2 << i << "\t";
      resultsStream2 << totCounts[i] << "\n";
    }
  return 0;
}
