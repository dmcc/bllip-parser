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


#ifndef PARAMS_H
#define PARAMS_H
      
#include "Field.h"
#include "ECArgs.h"

class Params 
{
public:

  static const int DEFAULT_SENT_LEN = 100;

    Params(): 	    
      file(0),
      maxSentLen(DEFAULT_SENT_LEN),
      stdInput_(false),
      outputData_(false),
      fileString_(),
      numString_(),
      whichSent_( 0 ),
      ofTotal_( 1 ),
      field_( 0 )
	{}

    void	    init( ECArgs& args );
    const char *    file;
    const ECString&   fileString()
		    {  return fileString_;  }
    const ECString&   numString()
		    {   return numString_;  }
    const int	    whichSent()
		    {   return whichSent_;   }
    const int	    ofTotal()
		    {   return ofTotal_;   }
    const Field&    field() const
		    {   return *field_;   }
    bool&      stdInput() { return stdInput_; }
    bool&      outputData() { return outputData_; }
    int        maxSentLen;
    ifstream*  extPosIfstream;
private:
    bool       stdInput_;
    bool       outputData_;
    ECString	    fileString_;
    ECString          numString_;
    int             whichSent_;
    int             ofTotal_;
    Field *         field_;
};

#endif /* ! PARAMS_H */
