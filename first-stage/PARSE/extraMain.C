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

#include <sys/resource.h>
#include "extraMain.h"
#include <vector>
#include <list>
#include "Item.h"
#include "MeChart.h"
#include "headFinder.h"
#include "ClassRule.h"
#include "utils.h"

void
generalInit(ECString path)
{
  struct rlimit 	core_limits;
  core_limits.rlim_cur = 0;
  core_limits.rlim_max = 0;
  setrlimit( RLIMIT_CORE, &core_limits );

  struct rlimit stack_limits;
  stack_limits.rlim_cur = 0;
  stack_limits.rlim_max = 0;
  getrlimit( RLIMIT_STACK, &stack_limits );
  if (stack_limits.rlim_cur < stack_limits.rlim_max)
    {
      stack_limits.rlim_cur = stack_limits.rlim_max;
      setrlimit( RLIMIT_STACK, &stack_limits );
    }

  // load locale settings from the environment
  setlocale(LC_ALL, "");

  if (!endsWith(path, "/")) {
    path += "/";
  }

  Term::init( path );
  readHeadInfo(path);
  InputTree::init();
  UnitRules* ur = new UnitRules;
  ur->readData(path); 
  Bchart::unitRules = ur;
  Bchart::readTermProbs(path);
  MeChart::init(path);
  Bchart::setPosStarts();
  ChartBase::midFactor = (1.0 - (.3684 *ChartBase::endFactor))/(1.0 - .3684);
  if(Feature::isLM or Feature::useExtraConditioning) 
    ClassRule::readCRules(path);
}

InputTree*
inputTreeFromBsts(Val* at, short& pos, SentRep& sr)
{
  //cerr << "itfat " << at->trm() << " " << at->bsts().size() << endl;
  short trmInt = at->trm();
  if(trmInt >= 400)
    {
      cerr << "Bad trm int: " << trmInt << endl;
      assert(trmInt < 400);
    }
  const Term* trm = NULL;
  ECString trmString;
  if(trmInt >= 0)
    {
      trm = Term::fromInt(trmInt);
      trmString = trm->name();
    }
  ECString wrdString;
  ECString ntString;
  list<InputTree*> subtrs;
  InputTree* ans;
  short begn = pos;
  if(trm && trm->terminal_p() && at->status == TERMINALVAL)
    {
      wrdString = sr[pos].lexeme();
      pos++;
    }
  else
    {
      assert(at);
      Bsts::iterator bi = at->bsts().begin();
      int vpos = 0;
      for( ; bi != at->bsts().end() ; bi++)
        {
          Bst& sb = **bi;
          int vval = at->vec()[vpos];
          if(vval >= sb.num())
            {
              cerr << vpos << " "
                   << vval << " " << sb.num() << " " << *at << endl;
              assert(vval < sb.num());
            }
          InputTree* sit = inputTreeFromBsts(sb.nth(vval), pos,sr);
          assert(sit);
          subtrs.push_back(sit);
          vpos++;
        }
    }

  /* bestParse in MeChart creates a ficticious level of structure that
     this removes*/

  //if(!trm && !at->edge() && at->status == EXTRAVAL) return subtrs.front();
  if(!at->edge() && at->status == EXTRAVAL) return subtrs.front();

  ans = new InputTree(begn, pos, wrdString, trmString, ntString,
                      subtrs, NULL, NULL);
  
  
  /* This code inserts the position of the head word after the
     non-terminal */
  /*
  if(!trm->terminal_p())
    {
      int hp = headPosFromTree(ans);
      assert(hp >= 0);
      ans->ntInfo() = intToString(hp);
    }
  */
  InputTreesIter iti = subtrs.begin();
  for( ; iti != subtrs.end() ; iti++) (*iti)->parentSet() = ans;


  //cerr << "ITF " << *ans << endl;
  return ans;
}
