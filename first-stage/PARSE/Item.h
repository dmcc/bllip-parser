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

#ifndef ITEM_H 
#define ITEM_H

#include "Wrd.h"
#include "Edge.h"
#include <map>
#include <set>
#include "AnswerTree.h"
#include "CntxArray.h"
#include "Bst.h"

class Term;
class Word;

typedef set<Edge*, less<Edge*> > EdgeSet;
typedef EdgeSet::iterator EdgeSetIter;
typedef pair<EdgeSet,BstMap> ItmGHeadInfo;
typedef map<Wrd, ItmGHeadInfo, less<Wrd> > HeadMap;
typedef map<int,HeadMap, less<int> > PosMap;
typedef HeadMap::iterator HeadIter;
typedef PosMap::iterator PosIter;

class           Item
{
public:
    Item( //const Wrd* hd,
	 const Term * _term, int _start, int _finish );
    Item() {}
    Item( const Item& );
    ~Item();
    int		    operator== (const Item& item) const;
    friend ostream& operator<< ( ostream& os, const Item& item );
    const Term *    term() const { return term_; }
    const Wrd*     word() const { return word_; }
    const Wrd*&    word() { return word_; }
    int             start() const {return start_;}
    int&            start() {return start_;}
    int             finish() const {return  finish_;}
    int&            finish() {return  finish_;}
    list<Edge*>&  needme() {return needme_;}
    list<Edge*>&  ineed() {return ineed_;}
    void            check();
    double            prob() const {return prob_;}
    double            poutside() const {return poutside_;}
    /* storeP can be used as beta for rParse */
    double            beta() const {return storeP_;}
    double&           beta() {return storeP_;}
    double            storeP() const {return storeP_;}
    double &          prob() {return prob_;}
    double &          poutside() {return poutside_;}
    double &          storeP() {return storeP_;}
    Bst&              stored(CntxArray& ca) { return bstFind(ca, stored_); }
    PosMap&          posAndheads() { return posAndheads_; }
    void            set(const Term * _term, int _start);
    void	    operator= (const Item& itm);
 private:
    int             start_;
    int             finish_;
    const Term *    term_;
    const Wrd *    word_;
    list<Edge*>  needme_;	/* A list of rules requiring a term starting
				 * at start */

    list<Edge*>  ineed_;	// needme = rules predicted by this (art) item
				// ineed = rules that predict this (art) item
    double           prob_;
    double           poutside_;
    double           storeP_;	
    BstMap           stored_;
    PosMap           posAndheads_;
};

typedef list<Item*> Items;
typedef Item *	Item_star;

#endif /* !ITEM_H */
