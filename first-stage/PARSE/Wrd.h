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

#ifndef WRD_H 
#define WRD_H

#include "ECString.h"
#include <iostream>
#include "ewDciTokStrm.h"
#include "utils.h"

class Wrd;
class Wrd
{
public:
  friend class SentRep;
  Wrd() : lexeme_(""), loc_(-1) {}
  Wrd(const Wrd& wrd) : lexeme_(wrd.lexeme()),loc_(wrd.loc()),wInt_(wrd.toInt()) {}
  Wrd(ECString lx, int ps) : lexeme_(lx), loc_(ps) {}
  const ECString&  lexeme() const { return lexeme_; }
  ECString&  lexeme() { return lexeme_; }
  friend ewDciTokStrm& operator>>(ewDciTokStrm& is, Wrd& w)
    {
      w.lexeme_ = is.read();
      return is;
    }
  friend int operator<(const Wrd& w1, const Wrd& w2)
    { return w1.lexeme_ < w2.lexeme_; }
  friend istream& operator>>(istream& is, Wrd& w)
    {
      ECString lx;
      is >> lx;
      escapeParens(w.lexeme_);
      return is;
    }
  friend ostream& operator<<(ostream& os, const Wrd& w)
    {
      os << w.lexeme_;
      return os;
    }
  void operator=(const Wrd& wr)
    {
      lexeme_ = wr.lexeme();
      loc_ = wr.loc();
    }

  void setLoc(int l) { loc_ = l; }
  int loc() const { return loc_; }
  int toInt() const { return wInt_; }
  int& toInt() { return wInt_; }

private:
  ECString lexeme_;
  int loc_;
  int wInt_;
};

#endif
