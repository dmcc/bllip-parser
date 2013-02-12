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
 
#include "AnswerTree.h"
#include "ECString.h"

AnsTreeStr&
atpFind(CntxArray& ca, AnsTreeMap& atm)
{
  AnsTreeMap::iterator atpi = atm.find(ca);
  if(atpi == atm.end())
    {
      AnsTreeStr atp;
      atm[ca] = atp;
      return atm[ca];
    }
  else return (*atpi).second;
}

