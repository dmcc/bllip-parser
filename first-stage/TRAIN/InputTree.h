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
  InputTree(istream& is);
  InputTree() : start_(0), finish_(0), parent_(NULL) {}
  InputTree(int s, int f, const ECString w, const ECString t, const ECString n,
	    InputTrees& subT, InputTree* par, InputTree* headTr)
    : start_(s), finish_(f), word_(w), term_(t), ntInfo_(n),
      subTrees_(subT), parent_(par), headTree_(headTr),
      num_(""), traceTree_(NULL){}
  ~InputTree();

  friend istream& operator >>( istream& is, InputTree& parse );
  friend ostream& operator <<( ostream& os, const InputTree& parse );
  int         start() const { return start_; }
  int         length() const { return (finish() - start()); }
  int         finish() const { return finish_; }
  const ECString word() const { return word_; }  
  ECString&      word() { return word_; }  
  const ECString term() const { return term_; }
  ECString&      term() { return term_; }
  const ECString ntInfo() const { return ntInfo_; }
  ECString&      ntInfo() { return ntInfo_; }
  const ECString fTag() const { return fTag_; }
  ECString&      fTag() { return fTag_; }
  const ECString fTag2() const { return fTag2_; }
  ECString&      fTag2() { return fTag2_; }
  const ECString head() { return headTree_->word(); }
  const ECString hTag() { return headTree_->term(); }
  ECString       neInfo() const { return neInfo_; }
  ECString&      neInfo() { return neInfo_; }
  InputTrees& subTrees() { return subTrees_; }
  InputTree*& headTree() { return headTree_; }
  InputTree*& traceTree() { return traceTree_; }
  InputTree*  traceTree() const { return traceTree_; }
  InputTree*  parent() { return parent_; }
  InputTree*&  parentSet() { return parent_; }
  ECString       num() const { return num_; }
  ECString&      num() { return num_; }
  int          isEmpty();
  int          isUnaryEmpty();
  void        make(EcSPairs& str);
  bool        isCodeTree();
  void        readParse(istream& is);
  static bool readCW(istream& is);
  bool        ccTree();
  bool        ccChild();
  static int  pageWidth;     
  void prettyPrintWithHead(ostream& os) const;
 protected:
  InputTree*     newParse(istream& is, int& strt, InputTree* par);
  ECString&  readNext( istream& is );
  void        parseTerm(istream& is, ECString& a, ECString& b, ECString& c,
			ECString& f2, ECString& n);
  void        printproper( ostream& os, bool withhead=false ) const;
  void        prettyPrint(ostream& os, int start, bool startLine, bool withhead=false) const;
  int         spaceNeeded() const;
  void        flushConstit(istream& is);
  InputTree*  fixNPBifNecessary(InputTree* nextTree, ECString trm);
  
  InputTree*  parent_;
  int         start_;
  int         finish_;
  ECString   word_;
  ECString   term_;
  ECString   fTag_;
  ECString   fTag2_;
  ECString   ntInfo_;
  ECString      num_;
  ECString   neInfo_;
  InputTree*  traceTree_;
  InputTree*  headTree_;
  InputTrees  subTrees_;
};

InputTree* ithInputTree(int i, const list<InputTree*> l);
ECString     numSuffex(ECString str);
int        okFTag(ECString nc);

#endif /* ! INPUTTREE_H */
