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

#ifndef WORDINFO_H
#define WORDINFO_H

#include "ECString.h"

struct Phsgt
{
  int   term;           // indicates term 
  int   classNum;
  float prob;           // for term prob
};

class WordInfo
{
  friend class Pst;
 public:
  WordInfo() : lexeme_(), st_( 0 ), c_( 0 ), stSize_(0) { }
  int          stSize() const { return stSize_; }
  int          c() const { return c_; }
  const ECString&  lexeme() const { return lexeme_; }
  int          toInt() const { return n_; }
  // protected:
  //const Phsgt& subTerms() const { return *st_; }  //???;
  ECString  lexeme_;
  int       stSize_;
  int       c_;
  int       n_;
  Phsgt*    st_;
  bool      isVocabHole;
};

#endif /* ! WORDINFO_H */
