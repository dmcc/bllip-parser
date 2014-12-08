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

#ifndef INPUTTREE_H
#define INPUTTREE_H

#include <list>
#include "ECString.h"
#include "utils.h"
#include "ParseStats.h"
#include "SentRep.h"
#include <set>
#include <vector>

class InputTree;
typedef list<InputTree*> InputTrees;
typedef InputTrees::iterator InputTreesIter;
typedef InputTrees::const_iterator ConstInputTreesIter;
typedef pair<ECString,ECString> EcSPair;
typedef list<EcSPair> EcSPairs;
typedef EcSPairs::iterator EcSPairsIter;
bool scorePunctuation( const ECString trmString );

class  InputTree
{
 public:
  InputTree(InputTree* p);
  InputTree(istream& is);
  InputTree() : start_(0), finish_(0), word_(""), term_(""), parent_(NULL) {}
  InputTree(int s, int f, const ECString w, const ECString t, const ECString n,
	    InputTrees& subT, InputTree* par, InputTree* headTr)
    : start_(s), finish_(f), word_(w), term_(t), ntInfo_(n), num_(-1),
      subTrees_(subT), parent_(par), headTree_(headTr){}
  InputTree(const ECString w, int i)
    : start_(i), finish_(i+1), word_(w), term_(w), ntInfo_(""),num_(-1),
      parent_(NULL), headTree_(NULL){}
  ~InputTree();

  friend istream& operator >>( istream& is, InputTree& parse );
  friend ostream& operator <<( ostream& os, const InputTree& parse );
  void        printproper( ostream& os ) const;
  short       num() const { return num_; }
  short&      num() { return num_; }
  short       start() const { return start_; }
  short       length() const { return (finish() - start()); }
  short       finish() const { return finish_; }
  const ECString word() const { return word_; }  
  ECString& word() { return word_; }
  const ECString term() const { return term_; }
  ECString& term() { return term_; }
  const ECString ntInfo() const { return ntInfo_; }
  ECString& ntInfo() { return ntInfo_; }
  const ECString head() { return headTree_->word(); }
  const ECString hTag() { return headTree_->term(); }
  InputTrees& subTrees() { return subTrees_; }
  InputTree*& headTree() { return headTree_; }
  InputTree*  parent() { return parent_; }
  InputTree*&  parentSet() { return parent_; }
  void   recordGold( ParseStats& parseStats);
  void   precisionRecall( ParseStats& parseStats );
  bool   lexact2();

  void        make(list<ECString>& str);
  void        makePosList(vector<ECString>& str);
  static int  pageWidth;     
  static ECString tempword[400];
  static int      tempwordnum;
  static void   init();

  bool   ccChild();
  bool   ccTree();

 protected:
  void        readParse(istream& is);
  InputTree*     newParse(istream& is, int& strt, InputTree* par);
  ECString&  readNext( istream& is );
  void        parseTerm(istream& is, ECString& a, ECString& b,int& n);
  void        prettyPrint(ostream& os, int start, bool startLine) const;
  int         spaceNeeded() const;
  
  short       start_;
  short       finish_;
  ECString   word_;
  ECString   term_;
  ECString   ntInfo_;
  short       num_;
  InputTrees  subTrees_;
  InputTree*  parent_;
  InputTree*  headTree_;
};

InputTree* ithInputTree(int i, const list<InputTree*> l);

#endif /* ! INPUTTREE_H */
