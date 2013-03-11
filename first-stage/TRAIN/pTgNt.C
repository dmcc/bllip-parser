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
#include "Pst.h" //???;

void setNonTermInts();

/* read through wsj training data.
   compute p(x is head of NT | pos(x) =t) and put it in pTgNt.txt
   also compute ending info for pos's ie. p(ending(x) | pos(x)).
   goes at the end of the same file */

//#define Word_p const Word*

int                 data[MAXNUMTS][MAXNUMNTS];
int numEndings = 0;

typedef map<ECString,int, less<ECString> > endMap;
endMap endData[MAXNUMTS];
int                 numTerm[MAXNUMTS];

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

const Term*
addWwData(InputTree* tree)
{
  bool okSit = true;

  if( tree->word() != ""  )
    {
      ECString wTagNm = tree->term();
      const Term* trm = Term::get(wTagNm);
      int lhsInt = trm->toInt();
      if(trm->openClass()) 
	{
	  ECString hdLexU(tree->word());
	  char temp[1024];
	  ECString hdLex(langAwareToLower(hdLexU.c_str(),temp));
	  int len = hdLex.length();
	  if(len >= 3)
	    {
	      ECString e(hdLex,len-2,2);
	      //ECString e = hdLex;
	      // if the current count for lhs and e == 0, this is new;
	      const WordInfo* wi = Pst::get(hdLex); //???;
	      if(!wi)
		{
		  assert(wi);
		}
	      if(wi->c() <= 4) 
		{
		  incrEndData(lhsInt, e);
		  numTerm[lhsInt]++;
		}
	    }
	}
      return trm;
    }
  ECString fixedTerm(tree->term());
  if(fixedTerm == "") fixedTerm = "S1";
  const Term* lhs = Term::get(fixedTerm);
  /* If we cannot recognize the term, don't abort, just warn and do not
     create a rule here or one level up. */
  if(!lhs)
    {
      lhs = Term::get("GARBAGE");
      okSit = false;
      cerr << "Garbage term: " << tree->term() << endl;
    }

  InputTrees& st = tree->subTrees();
  InputTrees::iterator  subTreeIter= st.begin();
  InputTree  *subTree;
  for( ; subTreeIter != st.end() ; subTreeIter++ )
    {
      subTree = *subTreeIter;
      addWwData(subTree);
    }
  int lhsInt = lhs->toInt();
  int k, l;
  k = Term::get(tree->headTree()->term())->toInt();
  l = lhsInt - 1 - Term::lastTagInt();
  data[k][l]++;
  return lhs;
}

int
main(int argc, char *argv[])
{
  ECArgs args( argc, argv );
  assert(args.nargs() == 1);
  ECString path(args.arg(0));
  repairPath(path);
  cerr << "At start of pTgNt" << endl;

  for(int n = 0 ; n < MAXNUMTS ; n++)
    numTerm[n] = 0;

  ECString resultsString(path);
  resultsString += "endings.txt";

  Term::init( path );  
  if(args.isset('L')) Term::Language = args.value('L');
  readHeadInfo(path);
  Pst pst(path);

  int wordCount = 0;
  int processedCount = 0;

  int i, j;
  for(i = 0 ; i < MAXNUMTS ; i++)
    for(j = 0 ; j < MAXNUMNTS ; j++)
      data[i][j] = 0;

  i = 0;
  while(cin)
    {
      if(i%10000 == 0) cerr << i << endl;
      //if(i > 1000) break;
      InputTree  parse;
      cin >> parse;
      if(!cin) break;
      if(parse.length() == 0) break;
      const Term* resTerm = addWwData(&parse);
      processedCount++;
      wordCount += parse.length();
      i++;
    }
  ofstream     resultsStream(resultsString.c_str());
  assert(resultsStream);
  int  totNt[MAXNUMTS];
  for(i = 0 ; i < MAXNUMTS ; i++) totNt[i] = 0;
  for(i = 0 ; i <= Term::lastTagInt() ; i++)
    {
      for(j = 0 ; j < (Term::lastNTInt() - Term::lastTagInt()) ; j++)
	totNt[j] += data[i][j];
    }
  resultsStream << numEndings << "\n";
  for(i = 0 ; i < MAXNUMTS ; i++)
    {
      endMap::iterator emi = endData[i].begin();
      for( ; emi != endData[i].end() ; emi++)
	{
	  ECString ending = (*emi).first;
	  int cnt = (*emi).second;
	  resultsStream << i << "\t" << ending << "\t"
			<< (float) cnt / (float) numTerm[i]
			<< endl;
	    //<< "\n";
	}
    }
  return 0;
}
