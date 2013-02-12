/* can copy into npharser05/*/
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

#ifndef TERM_H
#define TERM_H

#include "ECString.h"
#include <list>
#include <map>
#include <assert.h>
#include <iostream>
#include <fstream>
#include "Feature.h"
#include "utils.h"


class Term;

typedef Term *		Term_p;
typedef const Term *	Const_Term_p;
typedef const Term     ConstTerm;

#define Terms list<ConstTerm*>
#define ConstTerms const list<ConstTerm*>
#define TermsIter list<Term*>::iterator
#define ConstTermsIter list<ConstTerm*>::const_iterator
typedef map<ECString, Term*, less<ECString> >  TermMap;

#define FINAL 3
#define COLON 8



class Term 
{
public:
    Term();			// provided only for maps.
    Term( const ECString s, int terminal, int n );
    Term( const Term& src );
    int              toInt() const { return num_; }
    const ECString& name() const {  return name_;  }
    friend ostream& operator<< ( ostream& os, const Term& t );
    friend ostream& operator>> ( istream& os, const Term& t );
    int		operator== (const Term& rhs ) const;

    int	  terminal_p() const { return terminal_p_; }
    bool   isPunc() const { return (terminal_p_ > 2) ? true : false ; }
    bool   openClass() const { return (terminal_p_ == 2) ? true : false ; }
    bool   isColon() const { return vECfind(name(), Colons);}
    bool   isFinal() const { return vECfind(name(), Finals);}
    bool   isComma() const { return terminal_p_ == 4; }
    bool   isCC() const { return (name() == "CC" || name() == "CONJP"); }
    bool   isRoot() const { return (name() == "S1"); }
    bool   isS() const ;
    bool   isParen() const {return terminal_p_ == 7;}
    bool   isNP() const {return name() == "NP";}
    bool   isVP() const {return name() == "VP";}
    bool   isOpen() const { return terminal_p_ == 5; }
    bool   isClosed() const { return terminal_p_ == 6; }
    
    static Const_Term_p get(const ECString& getname);
    static void  init(ECString & prefix);
    static const Term* fromInt(int i) 
      { assert(i < MAXNUMNTTS); return array_[i]; }
    static int  lastTagInt() { return lastTagInt_; }
    static int  lastNTInt() { return lastNTInt_; }
    static const Term* stopTerm;
    static const Term* startTerm;
    static const Term* rootTerm;
    static ECStrings Finals;
    static ECStrings Colons;
    static ECString Language;
private:
    ECString* namePtr() { return (ECString*)&name_; }
    int    	terminal_p_;
    int		num_;
    const ECString name_;
    static Term*  array_[MAXNUMNTTS];
    static TermMap termMap_ ;
    static int    lastTagInt_;
    static int    lastNTInt_;
};
  

#endif /* ! TERM_H */
