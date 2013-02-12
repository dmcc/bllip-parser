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

#ifndef SENTREP_H
#define SENTREP_H

#include "Wrd.h"
#include "ewDciTokStrm.h"
#include <istream>
#include <list>
#include <ostream>
#include <vector>

class SentRep
{
public:

    SentRep();
    SentRep(int size); // initial size for vector to grow from
    SentRep(list<ECString> wtList); // used by wwBCTest

    // SGML layout introduces sentence with <s> and ends it with </s>.
    // <s name> ... </s> also allowed and returned as "name" parameter. 
    friend istream& operator>> (istream& is, SentRep& sr);
    friend ewDciTokStrm& operator>> (ewDciTokStrm& is, SentRep& sr);

    int length() const { return sent_.size(); }

    const Wrd& operator[] ( int index ) const { return sent_[ index ]; }
    Wrd&       operator[] ( int index )       { return sent_[ index ]; }

    const ECString& getName() const { return name_; }

  private:

    vector<Wrd> sent_;
    ECString name_;

};

ostream& operator<< (ostream& os, const SentRep& sr);

#endif /* ! SENTREP_H */
