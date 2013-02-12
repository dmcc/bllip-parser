/*
 * Copyright 1997, Brown University, Providence, RI.
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

#ifndef PST_H
#define PST_H
#include <list>
#include "WordInfo.h"
#include "Phegt.h"
#include "Feature.h"

class Pst
{
 public:
  Pst(ECString& path);
  float&  pHugt(int i) { return pHugt_[i]; }
  float&  pHhypgt(int i) { return pHhypgt_[i]; }
  float&  pHcapgt(int i) { return pHcapgt_[i]; }
  float&  pHtgnt(int i, int j);
  float   pHegt(ECString& es, int t);
  float   pegt(ECString& sh, int t);
  void    readTermProbs(ECString& pTString, ECString& pUString);
  float           pHst( const ECString& s, int trm) const
    {
      const Phsgt* sh = useSubTermC( s, trm ); 
      return( sh ? sh->prob : 0 );
    }

  int             ch(ECString& s)
    {
      const WordInfo* sh = useHeadC( s ); 
      return( sh ? sh->c() : 0 );
    }  
  static int& classNum(int wnum, int pos);
  static int classNm(int wnum, int pos);
  list<double> wordPlistConstruct(const ECString& head, int word_num);
  static int  hSize() { return hSize_; }
  static const WordInfo*  get(const ECString& wd)
    { return useHeadC(wd); }
  static const ECString& fromInt(int i);
  static void classReader(const ECString& path);
// protected:
  void  readPhsgt(ECString& path);
  const Phsgt*         useSubTermC(const ECString& head,
				   const int trm) const;
  static const WordInfo*      useHeadC(const ECString& head);
  double  pstt(ECString& shU, int t, int word_num);
  double  psutt(const ECString& shU, int t, int word_num);
  double  psktt(const ECString& shU, int t, int word_num);
  double  pCapgt(const ECString& shU, int t, int word_num);
  double  pHypgt(const ECString& shU, int t);
  int   egtSize_;
  static int   numClasses_[50];  //unused at the moment
  static int   classOffsets_[50]; //unused at the moment
  int   nttCounts_[MAXNUMNTTS]; 
  float pHugt_[MAXNUMNTS];
  float pHcapgt_[MAXNUMTS];
  float pHhypgt_[MAXNUMTS];
  float pHtgnt_[MAXNUMTS][MAXNUMNTS]; 
  Phegt *pHegt_;
  static int       hSize_;
  static WordInfo* h_;
  };

#endif /* ! PST_H */



