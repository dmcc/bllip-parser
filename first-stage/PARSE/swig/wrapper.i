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
#endif
#ifdef SWIGJAVA
%include "swig/java/include/std_list.i"
#endif
%rename(stream_extraction) operator<<; // mostly to silence a warning

%include "std_pair.i"
#include <assert.h>

// these lines included verbatim in wrapper
%{
    #include "SimpleAPI.C"
    #include "Fusion.h"
%}
typedef std::string ECString;

// general SWIG exception handler: convert ParserError to RuntimeError
%exception {
    try {
        $action
    } catch (ParserError pe) {
        SWIG_exception(SWIG_RuntimeError, pe.description.c_str());
    }
}

%newobject parse;
%newobject tokenize;
%newobject inputTreeFromString;
%newobject inputTreesFromString;
%newobject inputTreesFromFile;
%newobject sentRepsFromString;
%newobject sentRepsFromFile;
%newobject getParseStats;
%newobject asNBestList;
%newobject treeLogProb;

%inline{
    const int max_sentence_length = MAXSENTLEN;

    // overridden version of error() from utils.[Ch]
    // see weakdecls.h for how we "override" C functions
    void error(const char *filename, int filelinenum, const char *msg) {
        throw ParserError(filename, filelinenum, msg);
    }
} // end %inline

namespace std {
    %template(ScoreAndBoolean) pair<double,bool>;

    // Python doesn't need to wrap this
#ifdef SWIGJAVA
    %template(ScoredTree) pair<double,InputTree*>;
#endif

    %template(StringList) list<string>;
    %template(SentRepList) list<SentRep*>;
    %template(InputTrees) list<InputTree*>;

    %template(StringVector) vector<string>;
    %template(TermVector) vector<Term*>;
    %template(TermVectorVector) vector<vector<Term*> >;
}

// bits of header files to wrap -- some of these may not be necessary
%rename(loadModel) generalInit;
void generalInit(ECString path);

%feature("python:slot", "sq_length", functype="lenfunc") SentRep::length;
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
                ChartBase::guided = false;
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

%feature("python:slot", "sq_length", functype="lenfunc") InputTree::length;
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
        Term();
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
        float fMeasure();
        float precision();
        float recall();
};

%include "SimpleAPI.h"
%include "Fusion.h"

namespace std {
    %template(VectorLabeledSpan) vector<LabeledSpan>;
    %template(VectorScoredTree) vector<ScoredTree>;
}
