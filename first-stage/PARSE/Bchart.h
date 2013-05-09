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

#ifndef BCHART_H
#define BCHART_H

#include "ChartBase.h"
#include "EdgeHeap.h"
#include "Wrd.h"
#include "FullHist.h"
#include "UnitRules.h"
#include "ExtPos.h"

#define Termstar const Term*

struct Wwegt  
{
  int t;
  ECString e;
  float p;
};

class Bchart;

/* WordAndPresence stores the integer associated with a word and whether
   the word is a "hole" or not (false = hole).  ("hole"s are in the
   vocabulary for the purposes of unified vocabulary indexing. */
typedef pair<int, bool> WordAndPresence;

class           Bchart : public ChartBase
{
public:
  Bchart(SentRep& sentence,int id);
  Bchart(SentRep& sentence,ExtPos& extPos,int id);
    virtual ~Bchart();
    virtual double  parse();
    static int&      printDebug() { return printDebug_; }
    static bool      printDebug(int val) { return val < printDebug_; }
    static void     readTermProbs(ECString& path);
    static void     makepUgT(ECString path);
    static void     readpUgT(ECString path);
    int      wtoInt(ECString& str);
    int     extraTime; //if no parse is found on regular time;
    static  Item*    dummyItem;
    static float timeFactor;
    float    denomProbs[MAXSENTLEN];  
    void            check();
    static void     setPosStarts();
    ECString intToW(int n);
    bool prned();
    bool issprn(Edge* e);
    static map< ECString, WordAndPresence, less<ECString> > wordMap;
    static ECString invWordMap[MAXNUMWORDS];
  static int lastKnownWord;
  static int lastWord[MAXNUMTHREADS];
  static map<ECString, int> newWordMap[MAXNUMTHREADS];
  static vector<ECString> newWords[MAXNUMTHREADS];
  static UnitRules*  unitRules; 
  static bool caseInsensitive;
  static bool tokenize;
  static int Nth;
  static bool prettyPrint;
  static bool silent;
  static bool smallCorpus;
  static float smoothPosAmount;
  static const char *HEADWORD_S1;
  Item*   edgesFromTree(InputTree* tree);
  void  set_Betas();

  // 06/01/06 ML: made these methods public for access by parseIt.C
  // in getting at least POS tags when parsing fails.
  list<float>& wordPlist(Wrd* word, int word_num);
  static float& pT(int val)
    {
      if (val < 0 || val >= MAXNUMNTTS)
	{
	  cerr << "Bad val = " << val << endl;
	  assert(val >= 0 && val < MAXNUMTS);
	}
      return pT_[val];
    }
  int depth;
  Val* curVal;
  int curDir;
  Val* gcurVal;
  ExtPos extraPos;
 protected:
  /* this block of functions are only used/defined in rParse */
    Wrd*  add_word(const Term* trm, int st, ECString wrdStr);
    Item* add_item(int b, const Term* trmNm, int wrd);
    Item* add_item2(int b, const Term* trm, int wInt, ECString wrdstr);
    Item* addToChart(const Term* trm);;
    Item* in_chart(int b, const Term* trm, bool& wasThere);
    Item* in_chartT(int b, const Term* trm);
    Edge* add_edge(Item* lhs, Items& rhs);
    void   computeEdgeBeta(Item* itm, Edge* edge);
    void   propagateItemBeta(Item* itm, double quant);
    int   headPosFromItems(Item* lhs, Items& rhs);
    void  readItem(istream& str, int& b, const Term*& trm);
    void  store_word(Wrd* wrd);
    Wrd*  find_word(int wint, int st);
    void  assignRProb(Edge* edge);
    double compute_Betas();
    bool  compute_Beta(Item* itm);
    double compute_EdgeBeta(Edge* edge);
    InputTree* lookUpPhrase(Item* lhs, ECString phrase);
    void  newWord(ECString wrdstr, int wInt, Item* ans);
    Edge*  procPhrasal(Item* lhs, ECString phrase);
    bool   procPhrase(Item* lhs, InputTree* tree);
    void   rPendFactor();
    /* end of block */

    void            add_reg_item(Item * itm);
    void            addFinishedEdge(Edge* newEdge);
    void            add_starter_edges(Item* itm);
    float           meEdgeProb(const Term* trm, Edge* edge, int whichInt);
    float           meFHProb(const Term* trm, FullHist& fh, int whichInt);
    static int printDebug_;

    void            extend_rule(Edge* rule, Item * itm, int right);
    void            already_there_extention(int i, int start, int right,
					    Edge* edge);
    void            add_edge(Edge* rli, int left);
    void            put_in_reg(Item * item);
    void            addWordsToKeylist( );
    Item           *in_chart(const Wrd* hd, const Term * trm,
			     int start, int finish);
    bool            repeatRule(Edge* edge);

    void            redoP(Edge* edge, double probRatio);
    void            redoP(Item *item, double probDiff);

    float           computeMerit(Edge* edge, int whichCalc);

    void  initDenom();
    double  psktt(Wrd* shU, int t);
    double  pCapgt(const Wrd* shU, int t);
    float   pHst(int w, int t);
    double  psutt(const Wrd* shU, int t);
    float   pegt(ECString& sh, int t);
    void    getpHst(const ECString& hd, int t);
    double pHypgt(const ECString& shU, int t);
    static float&  pHcapgt(int i) { return pHcapgt_[i]; }
    static float&  pHhypgt(int i) { return pHhypgt_[i]; }
    static float&  pHugt(int i) { return pHugt_[i]; }

    int     bucket(float val, int whichInt, int whichFt);
    int     bucket(float val);
    int    greaterThan(Wwegt& wwegt, ECString e, int t);
    float  pHegt(ECString& es, int t);
    float  computepTgT(int t1,int t2);
    void   addToDemerits(Edge* edge);
    static Item*    stops[MAXSENTLEN];
    EdgeHeap*       heap;
    int             alreadyPoppedNum;
    Edge*           alreadyPopped[450000]; //was 350000;
    static int&     posStarts(int i, int j);
    static int      posStarts_[MAXNUMNTTS][MAXNUMNTS];
  int     curDemerits_[MAXSENTLEN][MAXSENTLEN];

  static int egtSize_;
  static float bucketLims[14];
  static float pT_[MAXNUMNTTS];
  static float pHcapgt_[MAXNUMTS];
  static float pHhypgt_[MAXNUMTS];
  static float pHugt_[MAXNUMTS];

  static Wwegt* pHegt_;
  list<float> wordPlists[MAXSENTLEN];
};

#endif	/* ! BCHART_H */

