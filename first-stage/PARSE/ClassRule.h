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

#ifndef CLASSRULE_H
#define CLASSRULE_H

#include "FullHist.h"
#include <fstream>
#include <iostream>
#include <vector>
#include "AnswerTree.h"
#include "Bst.h"

#define MCALCRULES 10

class ClassRule
{
 public:
  ClassRule(int dd, int mm, int rr, int tt)
    : t_(tt), m_(mm), rel_(rr), d_(dd) {}
  ClassRule(const ClassRule& cr)
    : t_(cr.t()), m_(cr.m()), rel_(cr.rel()), d_(cr.d()) {}
  Val* apply(FullHist* treeh);
  static void readCRules(ECString str);
  static vector<ClassRule>& getCRules(FullHist* treeh, int wh);
  friend ostream& operator<<(ostream& os, const ClassRule& cr)
    {
      os << "{"<< cr.d() << "," << cr.m() << "," << cr.rel() << "," << cr.t() << "}";
      return os;
    }
  int d() const { return d_; }
  int m() const { return m_; }
  int t() const { return t_; }
  int rel() const { return rel_; }
 private:
  int t_;
  int m_;
  int rel_;
  int d_;
  static vector<ClassRule>  rBundles2_[MAXNUMNTTS][MAXNUMNTS];
  static vector<ClassRule>  rBundles3_[MAXNUMNTTS][MAXNUMNTS];
  static vector<ClassRule>  rBundlesm_[MAXNUMNTTS][MAXNUMNTS];
};
    
typedef vector<ClassRule> CRuleBundle;

#endif /* ! CLASSRULE_H */
