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

#include "ExtPos.h"
#include "utils.h"
#include <sstream>

/*
  external pos file is of the form:

      word1 tag1 tag2 // tags for first word
      word2 tag3
      ---

  where --- is a sentence separator.
*/

void
ExtPos::
read(ifstream* isp,SentRep& sr)
{
  clear();
  assert(isp);
  istream& is=*isp;
  if(!is)return;
  int i;
  for(i=0;;i++){
    string buf;
    getline(is,buf); 
    if(buf=="---"){
      break;
    }
    if(!is){
      break;
    }
    vector<const Term*> vt;
    stringstream ss(buf);
    ECString wrd;
    ss>>wrd;  //First entry on each line is the word, not a tag.
    escapeParens(wrd);
    if (wrd != sr[i].lexeme()) {
      cerr << "wrd:            '" << wrd << "'" << endl;
      cerr << "sr[i].lexeme(): '" << sr[i].lexeme() << "'" << endl;
    }
    assert(wrd==sr[i].lexeme());
    while(ss){
      ss>>wrd;
      if (!ss) break;
      const Term* trm=Term::get(wrd);
      if (trm) {
        vt.push_back(trm);
      } else {
        cerr << "Warning: Haven't seen term '" << wrd << "' in terms.txt" << endl;
      }
    }
    push_back(vt);
  }
}

bool ExtPos::hasExtPos() {
    for (size_t i = 0; i < size(); i ++) {
        vector<const Term*> terms = operator[](i);
        if (terms.size() > 0) {
            return true;
        }
    }

    return false;
}
