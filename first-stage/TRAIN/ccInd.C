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

/* this can be copied over to nparser05/TRAIN */
#include "ccInd.h"
#include "InputTree.h"
#include "Term.h"
#include "Feature.h"

int
ccIndFromTree(InputTree* tree)
{
  InputTreesIter  subTreeIter = tree->subTrees().begin();
  ECString trmNm = tree->term();
  bool sawComma = false;
  bool sawColen = false;
  bool sawCC = false;
  bool sawOTHNT = false;
  int numTrm = 0;
  int pos = 0;
  const Term* trm = Term::get(trmNm);
  int tint = trm->toInt();
  /*Change next line to indicate which non-terminals get specially
    marked to indicate that they are conjoined together */
  if(!trm->isNP() && !trm->isS() && !trm->isVP()) return tint;
  for( ; subTreeIter != tree->subTrees().end() ; subTreeIter++ )
    {
      InputTree* subTree = *subTreeIter;
      ECString strmNm = subTree->term();
      const Term* strm = Term::get(strmNm);
      if(pos != 0 && strm->isCC()) sawCC = true;
      else if(strmNm == trmNm) numTrm++;
      else if(pos != 0 && strm->isComma()) sawComma = true;
      else if(pos != 0 && strm->isColon()) sawColen = true;
      else if(!strm->terminal_p()) sawOTHNT = true;
      pos++;
    }
  if(trmNm == "NP" && numTrm == 2 && !sawCC) return Term::lastNTInt()+1;
  if((sawComma || sawColen || sawCC) && numTrm >= 2) return tint+Term::lastNTInt();
  return tint;
}
