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

// vi: syntax=cpp
%module SWIGParser

using namespace std;

/* SWIG includes */
%include "std_except.i"
%include "std_vector.i"
%include "std_string.i"
%include "exception.i"

#ifdef SWIGPYTHON
%include "std_list.i"
// make default stringification work in Python even though we use Java
// names here
%rename(__str__) toString;
%rename(__len__) length;
#endif
#ifdef SWIGJAVA
%include "swig/java/include/std_list.i"
#endif

%include "std_pair.i"
#include <assert.h>

typedef std::string ECString;

%{
    #include <cstddef>
    #include <fstream>
    #include <math.h>
    #include <unistd.h>
    #include <sstream>
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
    #include <iostream>
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

    int sentenceCount;
    static const double log600 = log2(600.0);

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

    class ParserError {
        public:
            const char* description;

            ParserError(string msg) {
                this->description = msg.c_str();
            }

            ParserError(const char *filename, int filelinenum, const char *msg) {
                stringstream description;
                description << "[";
                description << filename;
                description << ":";
                description << filelinenum;
                description << "]: ";
                description << msg;

                this->description = description.str().c_str();
            }
    };
%}

%exception {
    try {
        $action
    } catch (ParserError pe) {
        SWIG_exception(SWIG_RuntimeError, pe.description);
    }
}

%newobject parse;
%newobject tokenize;
%newobject inputTreeFromString;
%newobject inputTreesFromString;
%newobject inputTreesFromFile;
%newobject sentRepsFromString;
%newobject sentRepsFromFile;
%newobject asNBestList;

%inline{
    const int max_sentence_length = MAXSENTLEN;

    typedef pair<double,InputTree*> ScoredTree;

    /* main parsing workhorse in the wrapped world */
    vector<ScoredTree>* parse(SentRep* sent, ExtPos& tag_constraints) {
        if (sent->length() > MAXSENTLEN) {
            throw ParserError("Sentence is longer than maximum supported sentence length.");
        }

        vector<ScoredTree>* scoredTrees = new vector<ScoredTree>();

        MeChart* chart = new MeChart(*sent, tag_constraints, 0);
        chart->parse();
        Item* topS = chart->topS();
        if (!topS) {
            delete chart;
            throw ParserError("Parse failed: !topS");
        }

        chart->set_Alphas();
        Bst& bst = chart->findMapParse();

        if (bst.empty()) {
            delete chart;
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
            bool isUnique;
            int length = 0;
            diffs.is_unique(mapparse, isUnique, length);
            if (length != sent->length()) {
                cerr << "Bad length parse for: " << *sent << endl;
                cerr << *mapparse << endl;
                assert (length == sent->length());
            }
            if (isUnique) {
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

    vector<ScoredTree>* parse(SentRep* sent) {
        ExtPos extPos;
        return parse(sent, extPos);
    }

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
    // language but the InputTree objects in it are never freed.
    // the current workaround is to make sure that we acquire() the
    // pointers in the higher-level language.
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
        for (int i = 0; i < scoredTrees.size(); i++) {
            ScoredTree scoredTree = scoredTrees[i];
            nbest_list << scoredTree.first << endl;
            scoredTree.second->printproper(nbest_list);
            nbest_list << endl;
        }

        return nbest_list.str();
    }

    // overridden version of error() from utils.[Ch]
    // see weakdecls.h for how we "override" C functions
    void error(const char *filename, int filelinenum, const char *msg) {
        throw ParserError(filename, filelinenum, msg);
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
} // end %inline

namespace std {
   %template(StringList) list<string>;
   %template(SentRepList) list<SentRep*>;

   %template(ScoredTreePair) pair<double,InputTree*>;
   %template(InputTrees) list<InputTree*>;

   %template(VectorScoredTree) vector<ScoredTree>;
   %template(StringVector) vector<string>;
   %template(TermVector) vector<Term*>;
   %template(TermVectorVector) vector<vector<Term*> >;
}

// bits of header files to wrap -- some of these may not be necessary
%rename(loadModel) generalInit;
void generalInit(ECString path);

class SentRep {
    public:
        SentRep(list<ECString> wtList);
        int length();

        %rename(getWord) operator[](int);
        const Wrd& operator[] (int index);

        const ECString& getName();

        %extend {
            string toString() {
                stringstream outputstream;
                outputstream << *$self;
                string outputstring = outputstream.str();
                return outputstring;
            }

            // makeFailureTree() is adapted from makeFlat() in parseIt.C
            %newobject makeFailureTree;
            InputTree* makeFailureTree(string category) {
                MeChart* chart = new MeChart(*$self, 0);
                if ($self->length() >= MAXSENTLEN) {
                    delete chart;
                    error("Sentence is too long.");
                }
                InputTrees dummy1;
                InputTree *inner_tree = new InputTree(0, $self->length(), "",
                    category, "", dummy1, NULL, NULL);
                InputTrees dummy2;
                dummy2.push_back(inner_tree);
                InputTree *top_tree = new InputTree(0, $self->length(), "",
                    "S1", "", dummy2, NULL, NULL);
                inner_tree->parentSet() = top_tree;
                InputTrees its;
                for (int index = 0; index < $self->length(); index++) {
                    Wrd& w = (*$self)[index];
                    const ECString& pos = getPOS(w, chart);
                    InputTree *word_tree = new InputTree(index, index + 1,
                        w.lexeme(), pos, "", dummy1, inner_tree, NULL);
                    its.push_back(word_tree);
                }

                inner_tree->subTrees() = its;
                delete chart;
                return top_tree;
            }
        }
};

class InputTree {
    public:
        short num() const;
        short start() const;
        short length() const;
        short finish() const;
        const ECString word() const;
        const ECString term() const;
        const ECString ntInfo() const;
        const ECString head();
        const ECString hTag();
        list<InputTree*>& subTrees();
        InputTree* headTree();
        InputTree* parent();
        InputTree*& parentSet();

        ~InputTree();

        void        make(list<ECString>& str);
        void        makePosList(vector<ECString>& str);
        static int  pageWidth;

        %extend {
            string toString() {
                stringstream outputstream;
                $self->printproper(outputstream);
                string outputstring = outputstream.str();
                return outputstring;
            }

            string toStringPrettyPrint() {
                stringstream outputstream;
                outputstream << *$self;
                string outputstring = outputstream.str();
                return outputstring;
            }

            %newobject toSentRep;
            SentRep* toSentRep() {
                list<ECString> leaves;
                $self->make(leaves);
                return new SentRep(leaves);
            }

            %newobject getTags;
            list<ECString>* getTags() {
                vector<ECString> tags;
                $self->makePosList(tags);
                // copy it to a list
                list<ECString>* tagsList = new list<ECString>(tags.begin(),
                    tags.end());
                return tagsList;
            }

            %newobject getWords;
            list<ECString>* getWords() {
                list<ECString>* leaves = new list<ECString>();
                $self->make(*leaves);
                return leaves;
            }

            void setTerm(ECString newTerm) {
                $self->term() = newTerm;
            }
            void setNtInfo(ECString newInfo) {
                $self->ntInfo() = newInfo;
            }
            void setWord(ECString newWord) {
                $self->word() = newWord;
            }
        }
};

class ewDciTokStrm {
    public:
        ewDciTokStrm(istream&);
        ECString read();
};

class Wrd {
    public:
        const ECString& lexeme();
};

class Term {
    public:
        Term(); // provided only for maps.
        Term(const ECString s, int terminal, int n);
        int toInt();

        int terminal_p() const;
        bool isPunc() const;
        bool openClass() const;
        bool isColon() const;
        bool isFinal() const;
        bool isComma() const;
        bool isCC() const;
        bool isRoot() const;
        bool isS() const;
        bool isParen() const;
        bool isNP() const;
        bool isVP() const;
        bool isOpen() const;
        bool isClosed() const;
};

class ExtPos {
    public:
        bool hasExtPos();

        %extend {
            bool addTagConstraints(vector<string> tags) {
                vector<const Term*> constTerms;
                for (vector<Term*>::size_type i = 0; i != tags.size(); i++) {
                    string tag = tags[i];
                    const Term* term = Term::get(tag);
                    if (!term) {
                        return false;
                    }
                    constTerms.push_back(term);
                }
                $self->push_back(constTerms);
                return true;
            }

            // TODO has memory leak issue?
            vector<const Term*> getTerms(int i) {
                return $self->operator[](i);
            }

            int size() const {
                return $self->size();
            }
        }
};

class ScoreTree {
    public:
        %extend {
            %newobject score;
            ParseStats* score(InputTree* proposed, InputTree* gold) {
                vector<ECString> poslist;
                gold->makePosList(poslist);
                $self->setEquivInts(poslist);

                ParseStats* stats = new ParseStats();
                $self->recordGold(gold, *stats);
                $self->precisionRecall(proposed, *stats);
                return stats;
            }
        }
};

class ParseStats {
    public:
        ParseStats();
        int numInGold;
        int numInGuessed;
        int numCorrect;
};
