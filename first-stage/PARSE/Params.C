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

#include "Params.h"
#include "Bchart.h"
#include "CntxArray.h"
#include "ClassRule.h"
#include "Feature.h"
#include "string.h"

void
Params::
init( ECArgs& args ) 
{  
   fileString_ = args.arg(0);

   if( args.nargs() > 2 || args.nargs() == 0 )	// require path name 
     error( "Needs 1 or 2 args." );

   if(args.isset('M'))
     {
       Feature::setLM();
       CntxArray::sz = 6;
     }
   if(args.isset('X'))
     {
       Feature::setExtraConditioning();
       CntxArray::sz = 6;
     }
   if(args.isset('N'))
     {
       Bchart::Nth = atoi(args.value('N').c_str());
     }
   if(args.isset('s')) Bchart::smallCorpus = true;
   if(args.isset('S')) Bchart::silent = true;
   if(args.isset('P')) Bchart::prettyPrint = true;
   if(args.isset('C')) Bchart::caseInsensitive = true;
   if(args.isset('K')) Bchart::tokenize = false;
   if(args.isset('E')){
     string nm=args.value('E');
     extPosIfstream=new ifstream(nm.c_str());
     assert(extPosIfstream);
   }
   if(args.isset('p'))
     {
       float smoothPosAmount = atof(args.value('p').c_str());
       assert(smoothPosAmount >= 0);
       assert(smoothPosAmount <= 1);
       Bchart::smoothPosAmount = smoothPosAmount;
     }
   if(args.isset('T'))
     {
       int fac = atoi(args.value('T').c_str());
       float ffac = (float)fac;
       ffac /= 10;
       Bchart::timeFactor = ffac;
     }
   if(args.isset('l'))
     {
       maxSentLen = atoi(args.value('l').c_str());
       if (maxSentLen > MAXSENTLEN)
	 {
	   cerr << "\nMaximum sentence length allowed is " << MAXSENTLEN;
	   cerr << "; using this value.\n\n";
	   maxSentLen = MAXSENTLEN;
	 }
     }
   if( args.isset('d') )
     {
       int lev = atoi(args.value('d').c_str());
       Bchart::printDebug() = lev;
     }
   if (args.isset('L')) {
       Term::Language = args.value('L');
       if (!(Term::Language == "En" || Term::Language == "Ch" || Term::Language == "Ar"))
            error("Language (-L) must be one of En, Ch, or Ar.");
       if (Term::Language == "Ar")
            Bchart::tokenize = false;
   }

  if( args.isset('n') )
    {
      char etemp[16];
      strcpy(etemp,args.value('n').c_str());
      char *	temp = strchr( etemp, '/' );
      if( !temp )
	error( "No terminal '/' found in '-n' argument" );
      *temp = '\0';
      ofTotal_ = atoi( ++temp );
      char *	mask = new char[ ofTotal_ ];
      for( int i = 0; i < ofTotal_; i++ )
	mask[ i ] = 0;
      // fill in mask with valid numbers;
      ECString tmp2 = etemp;
      numString_ = tmp2;		// meaningful id for this process;
      whichSent_ = atoi(tmp2.c_str());
      mask[ whichSent_ ] = 1;
      field_ = new Field( ofTotal_, mask );
    }
    else
    {
	static char mask[1] = { 1 };
	field_ = new Field( 1, mask );
    }
}
