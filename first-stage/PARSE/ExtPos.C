
#include "ExtPos.h"
#include <sstream>

/*
  external pos file is of form

  tag1 tag2 //tages for first word 
  tag3
  ---
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
    string wrd;
    ss>>wrd;  //First entry on each line is the word, not a tag.
    assert(wrd==sr[i].lexeme());
    while(ss){
      ss>>wrd;
      if (!ss) break;
      const Term* trm=Term::get(wrd);
      vt.push_back(trm);
    }
    push_back(vt);
  }
}
