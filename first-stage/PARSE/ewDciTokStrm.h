/*
 * Copyright 1999, Brown University, Providence, RI.
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

#ifndef EWDCI_H
#define EWDCI_H

#include <fstream>
#include "ECString.h"

/***      This is file  /pro/dpg/usl/ew/dcitokstrm/ewDciTokStrm.h         ****
****                                                                      ****
****   The code in this module is optimized to fit the peculiarities of   ****
****    wsj/text/198* .  Run any "improved" version side by side with     ****
****   this one and inspect the actual outputs, before changing this.     ***/

class ewDciTokStrm
{
  public:
    ewDciTokStrm( istream& );
    virtual ~ewDciTokStrm() {}

    ECString	read();
    int		operator!()
      {
        return savedWrd_.length() == 0 && nextWrd_.length() == 0 && !istr_;
      }
    ECString    sentenceName;
 protected:
    istream& istr_;
  private:
    virtual ECString   nextWrd2();
    ECString	savedWrd_;
    ECString	nextWrd_;
    int         parenFlag;
    int		ellipFlag;
    ECString	flush_to_sentence();
    ECString	splitAtPunc( ECString );
    int         is_stateLike( const ECString& );
};
  

#endif /* ! EWDCI_H */
