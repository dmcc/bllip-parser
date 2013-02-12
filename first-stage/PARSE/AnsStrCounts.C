/*
 * Copyright 2005 Brown University, Providence, RI.
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

class AnsStrCounts
{
 public:
  AnsStrCounts()
  {
    for(int i = 0 ; i < 11 ; i++)
      numWords[i] = numCounts[i] = numSents[i] = 0;
  }
  void updateCounts(int len)
  {
    int buk = len/10;
    numWords[buk] += len;
    numCounts[buk] += AnsTreeStr::numCreated;
    numSents[buk]++;
    AnsTreeStr::numCreated = 0;
  }
  int numWords[11];
  int numCounts[11];
  int numSents[11];
  void showCounts();
};

void
AnsStrCounts::
showCounts()
{
  for(int i = 0 ; i < 11 ; i++)
    {
      cerr << i << "\t";
      if(numWords[i] != 0)
	{
	  float cps = (float)numCounts[i]/(float)numSents[i];
	  float wps = (float)numWords[i]/(float)numSents[i];
	  cerr << numSents[i] << "\t"<< (float)numCounts[i]
	       << "\t" << wps
	       << "\t" << cps
	       << "\t" << 10.0*wps*sqrt(wps);
	}
      cerr << endl;
    }
}
