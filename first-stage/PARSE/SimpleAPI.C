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
#include "headFinder.h"
#include "InputTree.h"
#include "Link.h"
#include "MeChart.h"
#include "Params.h"
#include "ParseStats.h"
#include "ScoreTree.h"
#include "SentRep.h"
#include "SimpleAPI.h"
#include "TimeIt.h"
#include "UnitRules.h"
#include "utils.h"
#include "Wrd.h"

int sentenceCount;
typedef pair<double,InputTree*> ScoredTree;

//
// ParserError
//
ParserError::ParserError(string msg) {
    this->description = msg;
}

ParserError::ParserError(const char *filename, int filelinenum,
                         const char *msg) {
    stringstream description;
    description << "[";
    description << filename;
    description << ":";
    description << filelinenum;
    description << "]: ";
    description << msg;

    this->description = description.str();
}

//
// LabeledSpan
//
LabeledSpan::LabeledSpan() : start(-1), end(-1), termIndex(-1) {}
LabeledSpan::LabeledSpan(int startIndex, int endIndex, string termName)
    : start(startIndex), end(endIndex) {
        assert (startIndex < endIndex);
        termIndex = Term::get(termName)->toInt();
}
int LabeledSpan::size() {
    return end - start;
}
bool LabeledSpan::disrupts(int otherStart, int otherEnd) {
    assert (otherStart < otherEnd);
    if (start == otherStart) {
        // special case -- they only intersect if end == otherEnd
        return end == otherEnd;
    }

    // canonicalize order so that A is at least as early as B
    int startB, endA, endB;
    if (start < otherStart) {
        endA = end;
        startB = otherStart;
        endB = otherEnd;
    } else { // start > otherStart
        endA = otherEnd;
        startB = start;
        endB = end;
    }
    /* now it's something like this:
            [startA ----- endA)
                [startB -?????- endB)
       where endB could be anywhere after startB and startA might
       be the same as startB. */

    // see if Bs start is within A (startB must be > startA)
    if (startB < endA) {
        // fine as long as B also ends within A
        if (endB <= endA) {
            return false;
        } else {
            return true;
        }
    }

    return false;
}
bool LabeledSpan::operator==(const LabeledSpan& other) const {
    return (start == other.start) &&
        (end == other.end) &&
        (termIndex == other.termIndex);
}
bool LabeledSpan::operator!=(const LabeledSpan& other) const {
    return !(*this == other);
}
bool LabeledSpan::operator<(const LabeledSpan& other) const {
    if (start < other.start) {
        return true;
    } else if (start == other.start) {
        if (end < other.end) {
            return true;
        } else if (end == other.end) {
            return (termIndex < other.termIndex);
        }
    }
    return false;
}
ostream& operator<<(ostream& os, const LabeledSpan& span) {
    os << "LabeledSpan(" << span.start << ", " << span.end << ", "
       << span.termIndex << ")";
    return os;
}

//
// LabeledSpans
//
LabeledSpans::LabeledSpans() {
    sorted = false;
    minSizeForParsing = 0;
}
void LabeledSpans::addConstraint(int start, int end, string term) {
    LabeledSpan sp(start, end, term);
    this->push_back(sp);
}
void LabeledSpans::spansFromTree(InputTree* tree, LabeledSpans& spans) {
    int start = tree->start();
    int finish = tree->finish();
    LabeledSpan span(start, finish, tree->term());
    spans.push_back(span);

    InputTrees subtrees = tree->subTrees();
    InputTreesIter subtreeIter = subtrees.begin();
    for (; subtreeIter != subtrees.end() ; subtreeIter++) {
        spansFromTree(*subtreeIter, spans);
    }
}

void LabeledSpans::ensureSorted() {
    if (!sorted) {
        std::sort(this->begin(), this->end());
        sorted = true;
    }
}
bool LabeledSpans::matches(InputTree* tree) {
    LabeledSpans treeSpans;
    spansFromTree(tree, treeSpans);
    treeSpans.ensureSorted();
    this->ensureSorted();

    return std::includes(treeSpans.begin(), treeSpans.end(),
        this->begin(), this->end());
}

// Apply these constraints to a chart. Returns true iff parsing
// should be done in guided mode (if the minSizeForParsing is too
// high, the constraints won't actually be applied and you shouldn't
// do guided parsing)
bool LabeledSpans::applyToChart(ChartBase* chart, int length) {
    if (length < minSizeForParsing) {
        return false;
    }
    vector<LabeledSpan>::iterator spanIterator = this->begin();
    for (; spanIterator != this->end(); spanIterator++) {
        LabeledSpan span = *spanIterator;
        if (span.size() >= minSizeForParsing) {
            chart->addConstraint(span.start, span.end, span.termIndex);
        }
    }
    // leave all other spans that don't "disrupt" one of
    // our real constraints as open
    for (int i = 0; i < length; i++) {
        for (int j = i + 1; j <= length; j++) {
            if (disrupts(i, j)) {
                // don't post any constraints
                continue;
            }

            // if it's a word span, can be all tags +
            // phrasal types
            int tagStart = 0;
            int tagEnd = Term::lastNTInt();
            if (j != i + 1) {
                // if not a word span, can only be a phrasal type
                // (lastTag to lastNT)
                tagStart = Term::lastTagInt();
            }
            for (int k = tagStart; k <= tagEnd; k++) {
                chart->addConstraint(i, j, k);
            }
        }
    }
    return true;
}
bool LabeledSpans::disrupts(int start, int end) {
    vector<LabeledSpan>::iterator spanIterator = this->begin();
    for (; spanIterator != this->end(); spanIterator++) {
        LabeledSpan span = *spanIterator;
        if (span.size() < minSizeForParsing) {
            continue;
        }
        if (span.disrupts(start, end)) {
            return true;
        }
    }
    return false;
}

//
// Helper methods
//

vector<ScoredTree>* parse(SentRep* sent, ExtPos& tagConstraints,
                          LabeledSpans* spanConstraints) {
    if (sent->length() > MAXSENTLEN) {
        throw ParserError("Sentence is longer than maximum supported sentence length.");
    }

    vector<ScoredTree>* scoredTrees = new vector<ScoredTree>();

    MeChart* chart = new MeChart(*sent, tagConstraints, 0);
    if (spanConstraints) {
        ChartBase::guided = spanConstraints->applyToChart(chart,
                                                          sent->length());
    } else {
        ChartBase::guided = false;
    }
    chart->parse();
    Item* topS = chart->topS();
    if (!topS) {
        delete chart;
        delete scoredTrees;
        throw ParserError("Parse failed: !topS");
    }

    chart->set_Alphas();
    Bst& bst = chart->findMapParse();

    if (bst.empty()) {
        delete chart;
        delete scoredTrees;
        throw ParserError("Parse failed: chart->findMapParse().empty()");
    }

    // decode unique parses
    Link diffs(0);
    int numVersions = 0;
    for ( ; ; numVersions++) {
        short pos = 0;
        Val *v = bst.next(numVersions);
        if (!v) {
            break;
        }
        double vp = v->prob();
        if (vp == 0 || isnan(vp) || isinf(vp)) {
            break;
        }
        InputTree *mapparse = inputTreeFromBsts(v, pos, *sent);
        bool uniqueAndValidParse;
        int length = 0;
        diffs.is_unique(mapparse, uniqueAndValidParse, length);
        if (length != sent->length()) {
            cerr << "Bad length parse for: " << *sent << endl;
            cerr << *mapparse << endl;
            assert (length == sent->length());
        }
        if (uniqueAndValidParse && spanConstraints) {
            uniqueAndValidParse = spanConstraints->matches(mapparse);
        }
        if (uniqueAndValidParse) {
            // this strange bit is our underflow protection system
            double prob = log2(v->prob()) - (mapparse->length() * log600);
            ScoredTree scoredTree(prob, mapparse);
            scoredTrees->push_back(scoredTree);
        } else {
            delete mapparse;
        }
        if (scoredTrees->size() >= Bchart::Nth) {
            break;
        }
        if (numVersions > 20000) {
            break;
        }
    }

    delete chart;
    sentenceCount++;
    return scoredTrees;
}

// parse a sentence with no external POS tag or phrasal constraints
vector<ScoredTree>* parse(SentRep* sent) {
    ExtPos extPos;
    return parse(sent, extPos, NULL);
}

// compute labeled bracket statistics between two trees
ParseStats* getParseStats(InputTree* proposed, InputTree* gold) {
    ScoreTree st;
    vector<ECString> poslist;
    gold->makePosList(poslist);
    st.setEquivInts(poslist);

    ParseStats* stats = new ParseStats();
    st.recordGold(gold, *stats);
    st.precisionRecall(proposed, *stats);
    return stats;
}

// compute the f-score between two trees
double fscore(InputTree* proposed, InputTree* gold) {
    ParseStats* stats = getParseStats(proposed, gold);
    double result = stats->fMeasure();
    delete stats;
    return result;
}

// searches for target in scoredTrees, returns the logProb of the
// corresponding tree or 1 if the tree isn't found.
double findMatchingTreeLogProb(vector<ScoredTree>* scoredTrees,
                               InputTree* target) {
    // find a tree that matches the input tree -- this is needed since
    // it's possible to find trees which fit all the constraints but
    // have extra constituents in them
    for (size_t i = 0; i < scoredTrees->size(); i++) {
        ScoredTree scoredTree = (*scoredTrees)[i];
        InputTree* parse = scoredTree.second;
        if (fscore(parse, target) == 1) {
            return scoredTree.first;
        }
    }
    return 1;
}

// get the lob probability of an existing tree against the current
// model. This is essentially what the evalTree command line tool
// does.
double treeLogProb(InputTree* tree) {
    // this operates in one or two passes (depending on whether the first
    // pass succeeds). the first pass uses constraints at chart filling
    // time as well as decoding time. for various reasons "too far afield",
    // this doesn't always find a parse matching the input tree.  for this
    // reason, when we detect this, we reparse the sentence ignoring unary
    // spans. in both cases, we need to filter the returned n-best list since
    // there may be more probable parses which fits all the constraints.

    // change some parameters to speed up parsing for the first pass
    float origTimeFactor = Bchart::timeFactor;
    int origNBest = Bchart::Nth;
    Bchart::timeFactor = 3;
    Bchart::Nth = 5;

    // make SentRep for the words in the tree
    list<ECString> tokenList;
    tree->make(tokenList);
    SentRep sentRep(tokenList);

    // make LabeledSpans for the spans in the tree
    LabeledSpans treeSpans;
    LabeledSpans::spansFromTree(tree, treeSpans);

    ExtPos extPos; // empty POS constraints
    vector<ScoredTree>* scoredTrees = NULL;

    // seems that only two passes are required -- the first pass uses
    // all constraints, the second one ignores length-1 spans
    int minSizes[] = {0, 2};
    for (int minSizeIndex = 0; minSizeIndex < 2; minSizeIndex++) {
        treeSpans.minSizeForParsing = minSizes[minSizeIndex];
        try {
            scoredTrees = parse(&sentRep, extPos, &treeSpans);
        } catch (ParserError) {
            // ignore it time since we may retry
        }

        // restore old parameters
        Bchart::timeFactor = origTimeFactor;
        Bchart::Nth = origNBest;

        // see if any of the parses returned mtach
        if (scoredTrees != NULL) {
            double logProb = findMatchingTreeLogProb(scoredTrees, tree);

            // free the InputTrees
            for (size_t i = 0; i < scoredTrees->size(); i++) {
                ScoredTree scoredTree = (*scoredTrees)[i];
                delete scoredTree.second;
            }
            delete scoredTrees;

            if (logProb != 1) {
                return logProb;
            }
        }
    }

    throw ParserError("Parse failed: no parses even with limited constraints");
}

/* Initialize only the terms from a model. This can be used by tools
   that need a parsing model for POS tag information but don't want to
   load a complete parser model. */
void loadTermsOnly(string modelPath) {
    modelPath = sanitizePath(modelPath);
    Term::init(modelPath);
}

/* Initialize only the terms from a model. This can be used by tools
   that need a parsing model for POS tag information but don't want to
   load a complete parser model. */
void loadHeadInfoOnly(string modelPath) {
    modelPath = sanitizePath(modelPath);
    readHeadInfo(modelPath);
}

/* Set options in the parser */
void setOptions(string language, bool caseInsensitive, int nBest,
        bool smallCorpus, double overparsing, int debug,
        float smoothPosAmount) {
    Bchart::caseInsensitive = caseInsensitive;
    Bchart::Nth = nBest;
    Bchart::smallCorpus = smallCorpus;
    Bchart::timeFactor = overparsing;
    Bchart::printDebug() = debug;
    Term::Language = language;
    Bchart::smoothPosAmount = smoothPosAmount;
}

/* Tokenizes the text and returns a SentRep with the tokens in it.
   expectedTokens is an estimate of the number of tokens in the sentence.
   It's not bad if you're wrong since this is just used to preallocate
   a vector of words. */
SentRep* tokenize(string text, int expectedTokens) {
    istringstream* inputstream = new istringstream(text);
    ewDciTokStrm* tokStream = new ewDciTokStrm(*inputstream);
    // not sure why we need an extra read here, but the first word is null
    // otherwise
    tokStream->read();

    SentRep* srp = new SentRep(expectedTokens);
    *tokStream >> *srp;

    delete inputstream;
    delete tokStream;
    return srp;
}

/* Tokenizes the text and returns a SentRep with the tokens in it.
   Uses the approximate average length of an (English) sentence
   (+1 for a space) to estimate the number of tokens in the text. */
SentRep* tokenize(string text) {
    return tokenize(text, text.length() / 6);
}

InputTree* inputTreeFromString(const char* str) {
    stringstream inputstream;
    inputstream << str;
    InputTree* tree = new InputTree(inputstream);
    return tree;
}

// NOTE: this leaks! the list is handed off to the higher-level
// language (Python, Java) but the InputTree objects in it are never
// freed.  the current workaround is to make sure that we acquire()
// the pointers in the higher-level language.
list<InputTree* >* inputTreesFromString(const char* str) {
    stringstream inputstream;
    inputstream << str;
    list<InputTree* >* trees = new list<InputTree* >();

    while (inputstream) {
        InputTree* tree = new InputTree();
        inputstream >> *tree;
        // returns an empty tree when the stream is finished
        if (!tree->length()) {
            delete tree;
            break;
        }
        trees->push_back(tree);
    }

    return trees;
}

// NOTE: this leaks. see inputTreesFromString
list<InputTree* >* inputTreesFromFile(const char* filename) {
    ifstream filestream(filename);
    list<InputTree* >* trees = new list<InputTree* >();

    while (filestream) {
        InputTree* tree = new InputTree();
        filestream >> *tree;
        // returns an empty tree when the stream is finished
        if (!tree->length()) {
            delete tree;
            break;
        }
        trees->push_back(tree);
    }

    return trees;
}

// NOTE: this leaks. see inputTreesFromString
list<SentRep* >* sentRepsFromString(const char* str) {
    stringstream inputstream;
    inputstream << str;
    list<SentRep* >* sentReps = new list<SentRep* >();
    ewDciTokStrm tokStream(inputstream);

    while (true) {
        SentRep* sentRep = new SentRep();
        tokStream >> *sentRep;
        if (!sentRep->length()) {
            delete sentRep;
            break;
        }
        sentReps->push_back(sentRep);
    }

    return sentReps;
}

// NOTE: this leaks. see inputTreesFromString
list<SentRep* >* sentRepsFromFile(const char* filename) {
    ifstream filestream(filename);
    list<SentRep* >* sentReps = new list<SentRep* >();
    ewDciTokStrm tokStream(filestream);

    while (true) {
        SentRep* sentRep = new SentRep();
        tokStream >> *sentRep;
        if (!sentRep->length()) {
            delete sentRep;
            break;
        }
        sentReps->push_back(sentRep);
    }

    return sentReps;
}

/* Returns a string suitable for use with read_nbest_list() in
   the reranker. Ideally, we'd convert directly from ScoredTree to
   the reranker's equivalent structure. */
string asNBestList(vector<ScoredTree>& scoredTrees, string sentenceId) {
    stringstream nbest_list;
    nbest_list.precision(10);
    nbest_list << scoredTrees.size() << " " << sentenceId << endl;
    for (size_t i = 0; i < scoredTrees.size(); i++) {
        ScoredTree scoredTree = scoredTrees[i];
        nbest_list << scoredTree.first << endl;
        scoredTree.second->printproper(nbest_list);
        nbest_list << endl;
    }

    return nbest_list.str();
}

// Apply PTB escaping to a string ("(" becomes "-LRB-", etc.)
string ptbEscape(string word) {
    escapeParens(word);
    return word;
}

// Reverse PTB escaping to a string ("-LRB-" restored as "(", etc.)
string ptbUnescape(string word) {
    unescapeParens(word);
    return word;
}

// getPOS() is adapted from parseIt.C
// Helper function to return the string name of the most likely part
// of speech for a specific word in a chart.
static const ECString& getPOS(Wrd& w, MeChart* chart) {
    list <float>&wpl = chart->wordPlist(&w, w.loc());
    list <float>::iterator wpli = wpl.begin();
    float max = -1.0;
    int termInt = (int)max;
    for (; wpli != wpl.end(); wpli++) {
        int term = (int)(*wpli);
        wpli++;
        // p*(pos|w) = argmax(pos){ p(w|pos) * p(pos) }
        double prob = *wpli * chart->pT(term);
        if (prob > max) {
            termInt = term;
            max = prob;
        }
    }
    const Term *nxtTerm = Term::fromInt(termInt);
    return nxtTerm->name();
}
