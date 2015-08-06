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

#pragma once
#include <list>

#include "InputTree.h"
#include "Term.h"

class ScoredSpan {
    public:
        list<int> termIndices; // this is a list due to unaries
        float score;

        friend ostream& operator<<(ostream& os, const ScoredSpan& scoredSpan);
};

class Node {
    public:
        int start;
        int end;
        list<int> termIndices;
        float score;
        Node* leftChild;
        Node* rightChild;

        Node(int start, int end, int termIndex, float score);
        Node(int start, int end, list<int> termIndices, float score,
             Node* leftChild, Node* rightChild);

        string termNames() const;
        friend ostream& operator<<(ostream& os, const Node& node);
};

class SimpleChart {
    public:
        SimpleChart(int numWords);
        ~SimpleChart();
        void populate(InputTree* tree, float weight);

        void prunePreterms(int start, int end);
        void pruneConstituents(int start, int end, float minScore);
        void prune(float minScore);

        void initChart();
        void fillChart();
        void addChildTrees(Node& node, InputTrees* subTrees, InputTree* parent);
        InputTrees* makeTrees(Node& node, InputTree* parent);
        InputTree* parse();

        friend ostream& operator<<(ostream& os, const SimpleChart& chart);
    protected:
        int numWords;
        int numTerms;
        int numTags;
        float*** preterms; // spanStart x spanEnd x termInteger -> score
        list<ScoredSpan>*** constits; // spanStart x spanEnd -> ScoredSpans
        list<Node>*** chart; // spanStart x spanEnd -> Nodes
        vector<string> words;
};
