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

#include "Item.h"
#include <iostream>

void
Item::
set(const Term * _term, int _start)
{
  term_ = _term;
  start_ = _start;
  needme_.clear();
  ineed_.clear();
  word_ = NULL;
  storeP_ = 0.0;
  stored_.clear();
  posAndheads_.clear();
}

void
Item::
operator=(const Item& itm) 
{
  term_ = itm.term();
  start_ = itm.start();
  finish_ = itm.finish();
  word_ = itm.word();
}


Item::
Item(const Item& itm) :
  start_(itm.start()),
  finish_(itm.finish()),
  term_(itm.term()),
  word_(itm.word())
{}

Item::
~Item()
{
  //cerr << "Deleting " << *this << endl;
  /*
  PosIter pi = posAndheads_.begin();
  for( ; pi != posAndheads_.end() ; pi++)
    {
      HeadMap& hm = (*pi).second;
      HeadMap::iterator hmi = hm.begin();
      for( ; hmi != hm.end() ; hmi++)
	{
	  //cerr << "HMI " << (*hmi).first << endl;
	  ItmGHeadInfo& igh = (*hmi).second;
	  BstMap& atm = igh.second;
	  BstMap::iterator atmi = atm.begin();
	  for( ; atmi != atm.end(); atmi++)
	    {
	      Bst& atp= (*atmi).second;
	      //if(atp.trees[0]) delete atp.trees[0];
	    }
	}
    }
  */
}

Item::
Item(
     const Term * _term, int _start, int _finish)
: 
  start_( _start ),
  finish_( _finish ),
  term_( _term ),
  word_( NULL ),
  needme_(),
  ineed_(),
  prob_( 1.0 ),
  poutside_( 0.0 ), 
  storeP_( 0.0 )
{}

ostream&
operator<< (ostream& os, const Item& item)
{
    os << *item.term() << "(" << item.start();
    os << ", " << item.finish();
    //os << ", " << item.head()->lexeme();
    os << ")";
    return os;
}

int
Item::
operator== (const Item& item) const
{
    return ( //head_ == item.head() &&
	    term_ == item.term()
	    && start_ == item.start()
	    && finish_ == item.finish());
}

void            
Item::
check()
{
  assert(start() < finish() || !finish());
  list<Edge*>::iterator nmIter = needme_.begin();
  list<Edge*>::iterator inIter = ineed_.begin();
  for( ; nmIter != needme_.end() ; nmIter++)
    (*nmIter)->check();
  for( ; inIter != ineed_.end() ; inIter++)
    (*inIter)->check();
}
