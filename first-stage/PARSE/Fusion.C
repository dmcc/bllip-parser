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

/*
 * Syntactic Parse Fusion
 *
 * This is a reimplementation of Do Kook Choe's original Python code
 * by David McClosky.
 *
 * For details see:
 *
 *      Syntactic Parse Fusion (Choe, McClosky, and Charniak, EMNLP 2015)
 */

#include <algorithm>
#include <cmath>
#include <vector>

#include "Fusion.h"
#include "SimpleAPI.h"
#include "Term.h"

const string usage = " parsing-model [flags]\n"
    "Perform syntactic parse fusion over an n-best list, for details:\n"
    "Syntactic Parse Fusion (Choe, McClosky, and Charniak, EMNLP 2015).\n"
    "\n"
    "Parser model should be a first-stage parser model, "
    "not a unified parsing model.\n"
    "   -n[nbest]     maximum number of parse trees in each n-best list to use\n"
    "                 (default: 50)\n"
    "   -t[threshold] minimum normalized score for constituents (default: 0.5)\n"
    "   -e[exponent]  exponent to raise scores to (default: 1)\n"
    "   -s[k]         n-best list includes k scores (default: 2)\n"
    "   -S[k]         use kth-score from n-best list (default: 0)\n"
    "   -h            display this menu\n"
    "\n"
    "(don't include the brackets in the flags -- there should be no space\n"
    "after each flag, e.g., \"-n30 -t0.45 -e1.1\")\n"
    "\n"
    "Each n-best list should be in this format:\n"
    "    numParses sentenceIdOrIndex\n"
    "    tree0score0 tree0score1 ... tree0scoreK\n"
    "    tree0\n"
    "    tree1score0 tree1score1 ... tree1scoreK\n"
    "    tree1\n"
    "    ...\n"
    "\n"
    "(In other words, each tree is associated with k (set by -s) scores.\n"
    "For BLLIP, k will be 1 (parser only) or 2 (parser + reranker).\n";

string formatTermNames(list<int> termIndices) {
    string names = "[";
    list<int>::const_iterator termIterator = termIndices.begin();
    for (; termIterator != termIndices.end(); termIterator++) {
        if (termIterator != termIndices.begin()) {
            names += ", ";
        }
        int termIndex = *termIterator;
        const Term* term = Term::fromInt(termIndex);
        names += term->name();
    }
    names += "]";
    return names;
}

//
// ScoredSpan
//
ostream& operator<<(ostream& os, const ScoredSpan& scoredSpan) {
    os << "ScoredSpan(terms=" << formatTermNames(scoredSpan.termIndices) <<
          ", score=" << scoredSpan.score << ")";
    return os;
}

//
// Node
//
Node::Node(int start, int end, int termIndex, float score) {
    this->start = start;
    this->end = end;
    this->termIndices.push_back(termIndex);
    this->score = score;
    this->leftChild = NULL;
    this->rightChild = NULL;
}

Node::Node(int start, int end, list<int> termIndices, float score,
     Node* leftChild, Node* rightChild) {
    this->start = start;
    this->end = end;
    this->termIndices = termIndices;
    this->score = score;
    this->leftChild = leftChild;
    this->rightChild = rightChild;
}

string Node::termNames() const {
    return formatTermNames(termIndices);
}

ostream& operator<<(ostream& os, const Node& node) {
    os << "Node(start=" << node.start << ", end=" << node.end << ", term=" <<
          node.termNames() << ", score=" << node.score;

    if (node.leftChild) {
        os << ", left=" << *(node.leftChild);
    }
    if (node.rightChild) {
        os << ", right=" << *(node.rightChild);
    }
    
    os << ")";
    return os;
}

//
// SimpleChart
//
SimpleChart::SimpleChart(int numWords) {
    this->numWords = numWords;
    this->numTerms = Term::lastNTInt();
    this->numTags = Term::lastTagInt();

    // initialize preterms
    preterms = new float**[numWords];
    for (int start = 0; start < numWords; start++) {
        preterms[start] = new float*[numWords + 1];
        for (int end = start; end < numWords + 1; end++) {
            preterms[start][end] = new float[numTerms];
            for (int term = 0; term <= numTags; term++) {
                preterms[start][end][term] = -1;
            }
        }
    }

    // initialize constits
    constits = new list<ScoredSpan>**[numWords];
    for (int start = 0; start < numWords; start++) {
        constits[start] = new list<ScoredSpan>*[numWords + 1];
        for (int end = 0; end < numWords + 1; end++) {
            constits[start][end] = NULL;
        }
    }
}

SimpleChart::~SimpleChart() {
    for (int start = 0; start < numWords; start++) {
        for (int end = start; end < numWords + 1; end++) {
            delete[] preterms[start][end];
        }
        delete[] preterms[start];
    }
    delete[] preterms;

    for (int start = 0; start < numWords; start++) {
        for (int end = 0; end < numWords + 1; end++) {
            delete constits[start][end];
        }
        delete[] constits[start];
    }
    delete[] constits;

    for (int start = 0; start < numWords; start++) {
        for (int end = start + 1; end < numWords + 1; end++) {
            delete chart[start][end];
        }
        delete[] chart[start];
    }
    delete[] chart;
}

void SimpleChart::populate(InputTree* tree, float score) {
    assert (score >= 0);
    if (words.empty()) {
        list<string> wordList;
        tree->make(wordList);
        words.reserve(wordList.size());
        words.insert(words.end(), wordList.begin(), wordList.end());
    }

    // tree-specific constituents
    ScoredSpan*** treeConstits = new ScoredSpan**[numWords];
    for (int start = 0; start < numWords; start++) {
        treeConstits[start] = new ScoredSpan*[numWords + 1];
        for (int end = start + 1; end < numWords + 1; end++) {
            treeConstits[start][end] = NULL;
        }
    }

    LabeledSpans treeSpans;
    LabeledSpans::spansFromTree(tree, treeSpans);
    vector<LabeledSpan>::iterator spanIterator = treeSpans.begin();
    for (; spanIterator != treeSpans.end(); spanIterator++) {
        LabeledSpan span = *spanIterator;
        if (span.termIndex <= numTags) {
            // preterminal
            float value = preterms[span.start][span.end][span.termIndex];
            if (value == -1) {
                value = score;
            } else {
                value += score;
            }
            preterms[span.start][span.end][span.termIndex] = value;
        } else {
            // constituent
            ScoredSpan* scoredSpan;
            if (treeConstits[span.start][span.end] == NULL) {
                scoredSpan = new ScoredSpan();
                treeConstits[span.start][span.end] = scoredSpan;
                scoredSpan->score = score;
            } else {
                scoredSpan = treeConstits[span.start][span.end];
                // score doesn't change here since it's set once per tree
            }
            scoredSpan->termIndices.push_back(span.termIndex);
        }
    }

    // merge treeConstits into constits
    for (int start = 0; start < numWords; start++) {
        for (int end = start + 1; end < numWords + 1; end++) {
            if (treeConstits[start][end]) {
                ScoredSpan* treeConstit = treeConstits[start][end];

                if (!constits[start][end]) {
                    list<ScoredSpan>* scoredSpans = new list<ScoredSpan>();
                    scoredSpans->push_back(*treeConstit);
                    constits[start][end] = scoredSpans;
                    // don't need to do the search in this case
                    continue;
                }
                
                bool found = false;
                list<ScoredSpan>* scoredSpans = constits[start][end];
                list<ScoredSpan>::iterator spanIterator =
                    scoredSpans->begin();
                for (; spanIterator != scoredSpans->end(); spanIterator++) {
                    if (spanIterator->termIndices == treeConstit->termIndices) {
                        spanIterator->score += treeConstit->score;
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    constits[start][end]->push_back(*treeConstit);
                }
            }
        }
    }

    // cleanup treeConstits
    for (int start = 0; start < numWords; start++) {
        for (int end = start + 1; end < numWords + 1; end++) {
            delete treeConstits[start][end];
        }
        delete[] treeConstits[start];
    }
    delete[] treeConstits;
}

void SimpleChart::prunePreterms(int start, int end) {
    float bestScore = -1;
    int bestTermIndex = -1;
    // find preterm with span [start,end] with highest score
    for (int term = 0; term <= numTags; term++) {
        float score = preterms[start][end][term];
        if (score > bestScore) {
            bestScore = score;
            bestTermIndex = term;
        }
    }

    if (bestScore == -1) {
        return;
    }

    // unlike pruneConstituents, we keep all preterminals even if
    // they're below minScore
    for (int term = 0; term <= numTags; term++) {
        if (term != bestTermIndex) {
            preterms[start][end][term] = -1;
        } else {
            // convert scores to logspace (add 100 to reduce underflow)
            preterms[start][end][term] = log(bestScore) + 100;
        }
    }
}

void SimpleChart::pruneConstituents(int start, int end, float minScore) {
    if (!constits[start][end]) {
        return;
    }
    list<ScoredSpan>* scoredSpans = constits[start][end];

    // find highest scoring span
    list<ScoredSpan>::iterator spanIterator = scoredSpans->begin();
    float bestScore = -1;
    ScoredSpan* bestSpan = NULL;
    for (; spanIterator != scoredSpans->end(); spanIterator++) {
        if (spanIterator->score > bestScore) {
            bestScore = spanIterator->score;
            bestSpan = &(*spanIterator);
        }
    }

    // if the score isn't high enough, prune everything for this span
    if (bestScore < minScore) {
        bestScore = -1;
        bestSpan = NULL;
    }

    // do another pass and erase everything except the bestSpan
    spanIterator = scoredSpans->begin();
    while (spanIterator != scoredSpans->end()) {
        if (bestSpan != &(*spanIterator)) {
            spanIterator = scoredSpans->erase(spanIterator);
        } else {
            // converted scores to logspace (add 100 to reduce underflow)
            spanIterator->score = log(bestScore) + 100;
            spanIterator++;
        }
    }

    if (bestSpan == NULL) {
        delete constits[start][end];
        constits[start][end] = NULL;
    }
}

void SimpleChart::prune(float minScore) {
    for (int start = 0; start < numWords; start++) {
        prunePreterms(start, start + 1);
        for (int end = start + 1; end < numWords + 1; end++) {
            pruneConstituents(start, end, minScore);
        }
    }
}

void SimpleChart::initChart() {
    chart = new list<Node>**[numWords];
    for (int start = 0; start < numWords; start++) {
        chart[start] = new list<Node>*[numWords + 1];
        for (int end = start + 1; end < numWords + 1; end++) {
            list<Node>* nodes = new list<Node>();
            chart[start][end] = nodes;
        }

        // transfer best preterminals from preterms to chart
        // constits should be have pruned by now, so anything that's left
        // is the best of its class
        int end = start + 1;
        for (int term = 0; term <= numTags; term++) {
            float pretermScore = preterms[start][end][term];
            if (pretermScore == -1) {
                continue;
            }
            Node pretermNode(start, end, term, pretermScore);
            chart[start][end]->push_back(pretermNode);

            // transfer best span-1 phrasal constit to chart if there is one
            if (!constits[start][end]) {
                continue;
            }
            list<ScoredSpan>* scoredSpans = constits[start][end];
            assert (scoredSpans->size() == 1);
            ScoredSpan constitSpan = scoredSpans->front();
            list<int> termIndices = constitSpan.termIndices;
            Node constitNode(start, end, termIndices,
                             pretermScore + constitSpan.score,
                             &(chart[start][end]->back()),
                             NULL);
            chart[start][end]->push_back(constitNode);
        }
    }
}

void SimpleChart::fillChart() {
    for (int end = 1; end < numWords + 1; end++) {
        for (int start = end - 1; start >= 0; start--) {
            float bestScore = -1;
            int bestMid = -1;
            for (int mid = start + 1; mid < end; mid++) {
                // see if there are nodes from [start, mid] and [mid, end]
                // in the chart.
                if (chart[start][mid]->empty() || chart[mid][end]->empty()) {
                    continue;
                }

                // optionally use the span from [start, end] in constits
                float newScore = 0;
                if (constits[start][end]) {
                    newScore = constits[start][end]->back().score;
                }

                Node left = chart[start][mid]->back();
                Node right = chart[mid][end]->back();
                newScore += left.score + right.score;

                // if this is the case, we can make a new chart node
                // from [start, end]
                if (newScore > bestScore) {
                    bestScore = newScore;
                    bestMid = mid;
                }
            }

            if (bestScore != -1) {
                Node* left = &(chart[start][bestMid]->back());
                Node* right = &(chart[bestMid][end]->back());
                list<int> bestTerms;
                if (constits[start][end]) {
                    ScoredSpan span = constits[start][end]->back();
                    bestTerms = span.termIndices;
                }
                Node newNode(start, end, bestTerms, bestScore, left, right);
                chart[start][end]->push_back(newNode);
            }
        }
    }
}

/*
 * For a given Node, add the trees from its most direct left and right
 * children to subTrees.
 */
void SimpleChart::addChildTrees(Node& node, InputTrees* subTrees,
                                InputTree* parent) {
    if (node.leftChild) {
        InputTrees* leftTrees = makeTrees(*(node.leftChild), parent);
        subTrees->insert(subTrees->end(), leftTrees->begin(),
                         leftTrees->end());
        delete leftTrees;
    }
    if (node.rightChild) {
        InputTrees* rightTrees = makeTrees(*(node.rightChild), parent);
        subTrees->insert(subTrees->end(), rightTrees->begin(),
                         rightTrees->end());
        delete rightTrees;
    }
}

/*
 * Build a list of InputTrees from this Node. The reason that this is
 * a list rather than a single InputTree is that the Node could be virtual
 * (i.e., no terms on it) in which case it does not create a single subtree
 * but a series of fragments.
 */
InputTrees* SimpleChart::makeTrees(Node& node, InputTree* parent) {
    if (node.termIndices.empty()) {
        InputTrees* children = new InputTrees();
        addChildTrees(node, children, parent);
        return children;
    }

    // get first term from node and make its root InputTree
    list<int>::const_iterator termIterator = node.termIndices.begin();
    int termIndex = *termIterator;
    const string termName = Term::fromInt(termIndex)->name();

    InputTrees topSubTrees;

    // if it's a preterminal, set the word
    string word = "";
    if (termIndex < Term::lastTagInt()) {
        word = words[node.start];
    }
    InputTree* root = new InputTree(node.start, node.end, word, termName, "",
                                    topSubTrees, parent, NULL);
    InputTree* top = root;

    // iterate over the remaining terms in this node (for nodes with unaries)
    termIterator++;
    for (; termIterator != node.termIndices.end(); termIterator++) {
        int termIndex = *termIterator;
        const string childTermName = Term::fromInt(termIndex)->name();

        InputTrees childSubTrees;
        InputTree* child = new InputTree(node.start, node.end, "",
                                         childTermName, "",
                                         childSubTrees, top, NULL);
        topSubTrees.push_back(child);
        top->subTrees() = topSubTrees;

        topSubTrees = childSubTrees;
        top = child;
    }

    addChildTrees(node, &topSubTrees, top);
    top->subTrees() = topSubTrees;

    InputTrees* trees = new InputTrees();
    trees->push_back(root);
    return trees;
}

/*
 * Returns an InputTree fused from the n-best list.
 *
 * Note that the headTree is currently left null for these trees since
 * we do not assume a head finder.
 */
InputTree* SimpleChart::parse() {
    initChart();
    fillChart();

    if (chart[0][numWords]->back().termIndices.size() == 0) {
        // top node is virtual (no terms), meaning that the parse failed
        return NULL;
    }
    InputTrees* trees = makeTrees(chart[0][numWords]->back(), NULL);
    InputTree* tree = trees->back();
    delete trees;
    return tree;
}

ostream& operator<<(ostream& os, const SimpleChart& chart) {
    os << "SimpleChart(" << chart.numWords << "):\n";
    os << "preterms and constituents:\n";
    for (int start = 0; start < chart.numWords; start++) {
        for (int end = start + 1; end < chart.numWords + 1; end++) {
            // preterminals
            for (int termIndex = 0; termIndex <= chart.numTags; termIndex++) {
                float score = chart.preterms[start][end][termIndex];
                if (score == -1) {
                    continue;
                }
                const Term* term = Term::fromInt(termIndex);
                os << "\t" << start << " -> " << end << " ["
                   << term->name() << "] = " << score << "\n";
            }

            // constituents
            if (chart.constits[start][end]) {
                list<ScoredSpan>* scoredSpans = chart.constits[start][end];
                list<ScoredSpan>::iterator spanIterator =
                    scoredSpans->begin();
                for (; spanIterator != scoredSpans->end(); spanIterator++) {
                    ScoredSpan span = *spanIterator;
                    os << "\t" << start << " -> " << end << " "
                       << span << "\n";
                }
            }
        }
    }
    os << "chart:\n";
    for (int start = 0; start < chart.numWords; start++) {
        for (int end = start + 1; end < chart.numWords + 1; end++) {
            if (chart.chart[start][end] == NULL) {
                continue;
            }
            list<Node>* nodes = chart.chart[start][end];
            list<Node>::iterator nodeIterator =
                nodes->begin();
            for (; nodeIterator != nodes->end(); nodeIterator++) {
                Node node = *nodeIterator;
                os << "\t" << start << " -> " << end << " " << node
                   << "\n";
            }
        }
    }

    return os;
}

void printUsage(string programName, string errorMessage="") {
    cerr << programName << usage;
    if (errorMessage.length()) {
        cerr << "\nError: " << errorMessage << endl;
    }
}

int main(int argc, char *argv[]) {
    ECArgs args(argc, argv);

    if (args.isset('h')) {
        printUsage(argv[0]);
        return 0;
    }

    if (args.nargs() == 1) {
        loadTermsOnly(args.arg(0));
    } else {
        printUsage(argv[0], "Must provide a parser model as first argument");
        return 1;
    }
    int numScores = 2, scoreToUse = 0, numParsesToUse = 50;
    double threshold = 0.5, exponent = 1;
    if (args.isset('s')) {
        numScores = atoi(args.value('s').c_str());
        if (numScores < 1) {
            printUsage(argv[0], "-s: Need at least one score");
            return 1;
        }
    }
    if (args.isset('S')) {
        scoreToUse = atoi(args.value('S').c_str());
        if (numScores < 0) {
            printUsage(argv[0], "-S: Score to use must be positive");
            return 1;
        } else if (scoreToUse >= numScores) {
            printUsage(argv[0], "-S: Can't be higher than number of scores - 1");
            return 1;
        }
    }
    if (args.isset('n')) {
        numParsesToUse = atoi(args.value('n').c_str());
        if (numParsesToUse < 1) {
            printUsage(argv[0], "-n: Number of parses to use must be positive");
            return 1;
        }
    }
    if (args.isset('t')) {
        threshold = atof(args.value('t').c_str());
    }
    if (args.isset('e')) {
        exponent = atof(args.value('e').c_str());
    }

    while (true) {
        SimpleChart* simpleChart = NULL;
        string sentenceId;
        int numParses;
        cin >> numParses;
        cin >> sentenceId;
        if (sentenceId == "") {
            break;
        }

        vector<double> scores(numParses); // log probs
        vector<InputTree*> trees(numParses);

        // read n-best list
        double highestScore = 0;
        for (int parseIndex = 0; parseIndex < numParses; parseIndex++) {
            double tempScore, score = 0;
            // read scores
            for (int scoreIndex = 0; scoreIndex < numScores; scoreIndex++) {
                cin >> tempScore;
                if (scoreIndex == scoreToUse) {
                    score = tempScore * exponent;
                }
            }
            InputTree* tree = new InputTree();
            cin >> *tree;

            scores[parseIndex] = score;
            trees[parseIndex] = tree;

            if (parseIndex == 0) {
                highestScore = score;
                simpleChart = new SimpleChart(tree->length());
            } else {
                highestScore = max(score, highestScore);
            }
        }
        numParses = min(numParses, numParsesToUse);

        /* sum probs stored as log probs in a (more) numerically stable
         * fashion, see:
         *
         *   http://blog.smola.org/post/987977550/log-probabilities-semirings-and-floating-point
         */
        double scoreDiffExpSum = 0;
        for (int parseIndex = 0; parseIndex < numParses; parseIndex++) {
            double score = scores[parseIndex];
            scoreDiffExpSum += exp(score - highestScore);
        }

        for (int parseIndex = 0; parseIndex < numParses; parseIndex++) {
            double score = scores[parseIndex];
            double scoreNormalized = exp(score - highestScore) / scoreDiffExpSum;
            InputTree* tree = trees[parseIndex];
            simpleChart->populate(tree, scoreNormalized);
        }

        simpleChart->prune(threshold);
        InputTree* tree = simpleChart->parse();
        if (tree && tree->term() == "S1") {
            tree->printproper(cout);
            cout << endl;
        } else {
            // parse failed, print out the original top tree
            if (trees.size() > 0) {
                trees[0]->printproper(cout);
                cout << endl;
            }
        }
        delete simpleChart;
        delete tree;
        for (int parseIndex = 0; parseIndex < numParses; parseIndex++) {
            delete trees[parseIndex];
        }
    }

    return 0;
}
