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

#ifndef CHARTBASE_H
#define CHARTBASE_H

#include "Edge.h"
#include "Item.h"
#include "SentRep.h"
#include "Feature.h"
#include <vector>

class InputTree;

class           ChartBase
{
public:
  ChartBase(SentRep& sentence,int id);
    virtual ~ChartBase();

    enum Err { OK, OVERFLW, FAILURE };

    // parsing functions, what the class is all about.
    virtual double  parse() = 0;

    // extracting information about the parse.
    void            set_Alphas();
    const Items&    items( int i, int j ) const
		    {   return regs[ i ][ j ];   }
    int             edgeCount() const    { return ruleiCounts_; }
    int             poppedEdgeCount() const    { return poppedEdgeCount_; }
    int             poppedEdgeCountAtS() const    { return poppedEdgeCountAtS_; }
    int             totEdgeCountAtS() const    { return totEdgeCountAtS_; }
    Item*           addtochart(const Term* trm);
    // printing information about the parse.
    const Item*     mapProbs();
    static bool finalPunc(const char* wrd);

    Item*           topS() { return get_S(); }
    int    thrdid;
    static int&	    ruleCountTimeout()  {   return ruleiCountTimeout_;   }
    static const double
		    badParse;	// error return value for parse(), crossEntropy
    SentRep&        sentence_;
    vector<vector<int> > extPos;
    int             effEnd(int pos);
    static float endFactor;
    static float midFactor;
    static int      numItemsToDelete[MAXNUMTHREADS];
    static int      itemsToDeletesize[MAXNUMTHREADS];
    static vector<Item*>    itemsToDelete[MAXNUMTHREADS];
    static bool     guided;
    void            setGuide(InputTree* tree);
protected:
    Item           *get_S() const;  
    Items           regs[MAXSENTLEN][MAXSENTLEN];
    vector<short>   guide[MAXSENTLEN][MAXSENTLEN];
    bool            inGuide(int st, int ed, int trm);
    bool            inGuide(Edge* e);
    list<Edge*>     waitingEdges[2][MAXSENTLEN];
    double          crossEntropy_;
    int             wrd_count_;
    int             poppedEdgeCount_;
    int             totEdgeCountAtS_;
    int             poppedEdgeCountAtS_;
    int             ruleiCounts_; // keeps track of how many edges have been
                                // created --- used to time out the parse
    Item*           pretermItems[4000];
    int             pretermNum;
    int		    endPos;
    static int      ruleiCountTimeout_ ; //how many rulei's before we time out.
    static int      poppedTimeout_;
    float           endFactorComp(Edge* dnrl);

private:
    void            free_chart_items(Items& itms);
    void            free_chart_itm(Item * itm);
    void            free_edges(list<Edge*>& edges);
};


#endif	/* ! CHARTBASE_H */
