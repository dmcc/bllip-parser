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

#include "Link.h"
#include "InputTree.h"
#include "Term.h"

Link*
Link::
do_link(int tint, bool& ans)
{
  LinksIter li = links_.begin();
  for( ; li != links_.end() ; li++)
    {
      Link* slink = (*li);
      if(slink->key() == tint)
	{
	  ans = false;
	  return slink;
	}
    }
  ans = true;
  Link* nlink = new Link(tint);
  //cerr << "LN " << tint << endl;
  links_.push_back(nlink);
  return nlink;
}

Link*
Link::
is_unique(InputTree* tree, bool& ans, int& cnt)
{
  //cerr << "IU " << *tree << endl;
  Link* nlink;
  const Term* trm = Term::get(tree->term());
  nlink = do_link(trm->toInt(), ans);
  if(trm->terminal_p())
    {
      cnt++;
      return nlink;
    }
  InputTreesIter iti = tree->subTrees().begin();
  for( ; iti != tree->subTrees().end() ; iti++)
    {
      nlink = nlink->is_unique((*iti), ans,cnt);
    }
  nlink = nlink->do_link(DUMMYVAL, ans);
  return nlink;
}
    
      
      
