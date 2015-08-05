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

#include <cstddef>
#include <fstream>
#include <math.h>
#include <unistd.h>
#include <sstream>
#include <iostream>
#include <vector>
#include <list>

#include "AnsHeap.h"
#include "AnswerTree.h"
#include "Bchart.h"
#include "Bst.h"
#include "ChartBase.h"
#include "ECArgs.h"
#include "ECString.h"
#include "ewDciTokStrm.h"
#include "extraMain.h"
#include "GotIter.h"
#include "InputTree.h"
#include "Link.h"
#include "MeChart.h"
#include "Params.h"
#include "ParseStats.h"
#include "ScoreTree.h"
#include "SentRep.h"
#include "TimeIt.h"
#include "UnitRules.h"
#include "utils.h"
#include "Wrd.h"

extern int sentenceCount;
static const double log600 = log2(600.0);
typedef pair<double,InputTree*> ScoredTree;

class ParserError {
    public:
        ParserError(string msg);
        ParserError(const char *filename, int filelinenum, const char *msg);

        string description;
};

class LabeledSpan {
    public:
        int start;
        int end;
        int termIndex;

        LabeledSpan();
        LabeledSpan(int startIndex, int endIndex, string termName);
        int size();
        bool disrupts(int otherStart, int otherEnd);
        bool operator==(const LabeledSpan& other) const;
        bool operator!=(const LabeledSpan& other) const;
        bool operator<(const LabeledSpan& other) const;
        friend ostream& operator<<(ostream& os, const LabeledSpan& span);
};

class LabeledSpans: public vector<LabeledSpan> {
    public:
        int minSizeForParsing;
        bool sorted;
        LabeledSpans();
        void addConstraint(int start, int end, string term);
        static void spansFromTree(InputTree* tree, LabeledSpans& spans);

        void ensureSorted();
        bool matches(InputTree* tree);
        bool applyToChart(ChartBase* chart, int length);
        bool disrupts(int start, int end);
};

vector<ScoredTree>* parse(SentRep* sent, ExtPos& tagConstraints,
                          LabeledSpans* spanConstraints);
vector<ScoredTree>* parse(SentRep* sent);

ParseStats* getParseStats(InputTree* proposed, InputTree* gold);

double fscore(InputTree* proposed, InputTree* gold);

double treeLogProb(InputTree* tree);

void loadTermsOnly(string modelPath);
void loadHeadInfoOnly(string modelPath);

void setOptions(string language, bool caseInsensitive, int nBest,
        bool smallCorpus, double overparsing, int debug,
        float smoothPosAmount);

SentRep* tokenize(string text, int expectedTokens);
SentRep* tokenize(string text);

InputTree* inputTreeFromString(const char* str);
list<InputTree* >* inputTreesFromString(const char* str);
list<InputTree* >* inputTreesFromFile(const char* filename);
list<SentRep* >* sentRepsFromString(const char* str);
list<SentRep* >* sentRepsFromFile(const char* filename);

string asNBestList(vector<ScoredTree>& scoredTrees, string sentenceId);

string ptbEscape(string word);
string ptbUnescape(string word);
