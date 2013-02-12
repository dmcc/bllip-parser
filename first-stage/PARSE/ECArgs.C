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

#include "ECArgs.h"
#include <assert.h>
#include <stdio.h>
#include <iostream>
#include "utils.h"
#include <algorithm>

ECArgs::
ECArgs(int argc, char *argv[])
{
  nargs_ = 0;
  for(int i = 1 ; i < argc ; i++)
    {
      ECString arg(argv[i]);
      if(arg[0] != '-')
	{
	  argList[nargs_] = arg;
	  nargs_++;
	}
      else
	{
	  nopts_++;
	  int l = arg.length();
	  assert(l > 1);
	  ECString opt(1,arg[1]);
	  optList.push_back(opt);
	  if(l == 2) optList.push_back("");
	  else
	    {
	      ECString v(arg,2,l-2);
	      optList.push_back(v);
	    }
	}
    }
}

bool
ECArgs::
isset(char c)
{
  ECString sig = "";
  sig += c;
  list<ECString>::iterator
    oIter = find(optList.begin(), optList.end(), sig);
  bool found = (oIter != optList.end());
  return found;
}


ECString
ECArgs::
value(char c)
{
  ECString sig;
  sig += c;

  list<ECString>::iterator oIter = find(optList.begin(), optList.end(), sig);
  bool found = (oIter != optList.end());
  if(!found)
    {
      cerr << "Looking for value of on-line argument " << sig << endl;
      error("could not find value");
    }
  ++oIter;
  return *oIter;
}


