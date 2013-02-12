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

#include "ECString.h"
#include "ParseStats.h"
#include "utils.h"

istream&
operator >>( istream& is, ParseStats& ps )
{
  ps.readInput( is );
  return is;
}

float
ParseStats::
precision()
{
  int inGu = numInGuessed;
  if(inGu == 0)  return 0;
  float prc = (float)numCorrect/(float)inGu;
  return prc;
}

float
ParseStats::
recall()
{
  int inGo = numInGold;
  if(inGo == 0) return 0;
  float rec = (float)numCorrect/(float)inGo;
  return rec;
}

float
ParseStats::
fMeasure()
{
  float prc = precision();
  float rec = recall();

  float sm = prc+rec;
  if(sm == 0) return 0;
  return (2*prc*rec)/sm;
}

void
ParseStats::
readInput( istream& is )
{
  ECString bracket;
  is >> bracket;
  if( !is ) return;
  if( bracket != "<" ) error( "No open bracket for ParseStats" );
  is >> numInGold;
  is >> numInGuessed;
  is >> numCorrect;
  is >> bracket;
  if( bracket != ">" ) error( "No close bracket for ParseStats" );
}

ParseStats&
ParseStats::
operator+= ( const ParseStats& src )
{
  numInGold += src.numInGold;
  numInGuessed += src.numInGuessed;
  numCorrect += src.numCorrect;
  return *this;
}

void
ParseStats::
operator= ( const ParseStats& src )
{
  numInGold = src.numInGold;
  numInGuessed = src.numInGuessed;
  numCorrect = src.numCorrect;
}

ostream&
operator <<( ostream& os, const ParseStats& ps )
{
  os << "< " << ps.numInGold << " " << ps.numInGuessed << " "
     << ps.numCorrect << " >\n";
  return os;
}
