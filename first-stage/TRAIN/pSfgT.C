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

#include <iostream>
//#include <stdiostream.h>
#include <fstream>

#include "ECArgs.h"
#include "ECString.h"
#include "Term.h"
#include "utils.h"
#include "InputTree.h"
#include <map>
#include "headFinder.h"
#include "Pst.h" //???;



/* read through wsj training data.
      also compute sufix info for pos's ie. p(ending(x) | pos(x)).
   goes at the end of the same file */

//#define Word_p const Word*

//int                 data[60][30];
int numEndings = 0;

typedef map<ECString,int, less<ECString> > endMap;
endMap endData[140];
int                 numTerm[140];

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
	  if(len >= 4)
	    {
	      ECString e=lastCharacter(hdLex);
	      // if the current count for lhs and e == 0, this is new;
	      //cout<<hdLex<<endl;
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
	}//if openClass
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
  /*
  int lhsInt = lhs->toInt();
  int k, l;
  k = Term::get(tree->headTree()->term())->toInt();
  l = lhsInt - 1 - Term::lastTagInt();
  data[k][l]++;
  return lhs;*/
}

int
main(int argc, char *argv[])
{
  ECArgs args( argc, argv );
  assert(args.nargs() == 1);
  ECString path(args.arg(0));
  repairPath(path);
  cerr << "At start of pSfgt" << endl;

  for(int n = 0 ; n < 140 ; n++)
    numTerm[n] = 0;

  ECString resultsString(path);
  resultsString += "endings.txt";

  Term::init( path );
  if(args.isset('L')) Term::Language = args.value('L');
  readHeadInfo(path);
  Pst pst(path); //???;

  int wordCount = 0;
  int processedCount = 0;

  /*int i, j;
  for(i = 0 ; i < 60 ; i++)
    for(j = 0 ; j < 30 ; j++)
      data[i][j] = 0;
  */
  int i = 0;
  while(cin)
    {
      if(i++%5000 == 1) cerr << i << endl;
      InputTree  parse;
      cin >> parse;
      if(!cin) break;
      if(parse.length() == 0 && cin) continue;
      if(parse.length()==0 ||!cin) break;
      addWwData(&parse);
      processedCount++;
      wordCount += parse.length();
    }
  ofstream     resultsStream(resultsString.c_str());
  assert(resultsStream);
  /*int  totNt[30];
  for(i = 0 ; i < 30 ; i++) totNt[i] = 0;
  for(i = 0 ; i <= Term::lastTagInt() ; i++)
    {
      for(j = 0 ; j < (Term::lastNTInt() - Term::lastTagInt()) ; j++)
	totNt[j] += data[i][j];
    }
    */
  resultsStream << numEndings << "\n";

  for(i = 0 ; i < 140 ; i++)
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
  cout<<"totol sentence:"<<processedCount<<endl;
  cout<<"total suffix:"<<numEndings<<endl;

  return 0;
}
