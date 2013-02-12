// Licensed under the Apache License, Version 2.0 (the "License"); you may
// not use this file except in compliance with the License.  You may obtain
// a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
// License for the specific language governing permissions and limitations
// under the License.

// pretty-print.cc
//
// Mark Johnson, 17th March 2004

#include "custom_allocator.h"

#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

#include "sym.h"
#include "tree.h"

const char usage[] =
"pretty-print < best-parses.txt > pretty-print.txt\n"
"\n"
"pretty-print formats the parse trees so it is somewhat easier to see how\n"
"the trees differ.\n";

typedef precrec_type::edge edge;
typedef precrec_type::edges edges;

typedef unsigned int size_type;

symbol relabel(symbol s) {
  return symbol("!" + std::string(s) + "!");
}

size_type relabel(tree* t, const edges& es,
		  size_type left = 0, bool nonrootnode = false) {
  assert(t != NULL);
  assert(t->child != NULL);

  if (t->is_punctuation() || t->is_none())
    return left;                        // ignore punctuation and empty nodes
  
  if (t->is_preterminal())
    return left+1;                      // preterminal node
  
  size_type right = left;
  for (tree* c = t->child; c; c = c->next) 
    right = relabel(c, es, right, true);

  if (nonrootnode && right > left)       // ignore root node
    if (es.count(edge(left, right, precrec_type::relabel_category(t->label.cat))) == 0) 
      t->label.cat = relabel(t->label.cat);
    
  return right;
}

typedef std::vector<std::string> strings;

void segment_line(const char* cp, strings& ss)
{
  ss.resize(1);
  ss[0].clear();
  while (*cp != '\0' && *cp != '\n') {
    if (*cp == '\t') 
      ss.resize(ss.size()+1);
    else 
      ss[ss.size()-1].push_back(*cp);
    ++cp;
  }
}

int main(int argc, char* argv[]) {

  if (argc != 1) {
    std::cerr << usage << std::endl;
    exit(EXIT_FAILURE);
  }

  int ns = 0;
  char line[4096];
  while (fgets(line, 4096, stdin) != NULL) {
    strings fields;
    segment_line(line, fields);
    assert(fields.size() == 7);
    double best_fscore, max_weight_fscore; 
    int nread, n;
    nread = sscanf(fields[1].c_str(), " %d ", &n);
    assert(nread == 1);
    tree* max_weight_parse = readtree(fields[2].c_str());
    nread = sscanf(fields[3].c_str(), " %lf ", &max_weight_fscore);
    assert(nread == 1);
    tree* best_parse = readtree(fields[4].c_str());
    if (best_parse == NULL) {
      std::cerr << "pretty-print: best_parse = NULL, line = " << line << std::endl;
      exit(EXIT_FAILURE);
    }
    nread = sscanf(fields[5].c_str(), " %lf ", &best_fscore);
    tree* gold = readtree(fields[6].c_str());
    if (gold == NULL) {
      std::cerr << "pretty-print: gold = NULL, line = " << line << std::endl;
      exit(EXIT_FAILURE);
    }
    precrec_type::edges max_weight_edges(max_weight_parse), 
      best_edges(best_parse), gold_edges(gold);
    relabel(max_weight_parse, gold_edges);
    relabel(best_parse, gold_edges);
    relabel(gold, max_weight_edges);
    std::cout << ++ns << ": original sentence number = " << n 
	      << ", maximum weight parse (f-score = " << max_weight_fscore  
	      << ")\n" << std::endl;
    display_tree(std::cout, max_weight_parse);
    std::cout << "\n\nhighest scoring parse (f-score = " << best_fscore 
	      << ")" << std::endl;
    if (max_weight_fscore < best_fscore && best_fscore < 1.0) {
      std::cout << '\n';
      display_tree(std::cout, best_parse);
      std::cout << std::endl;
    }
    if (max_weight_fscore < 1.0) {
      std::cout << "\ngold tree:\n" << std::endl;
      display_tree(std::cout, gold);
      std::cout << std::endl;
    }
    std::cout << "\n----------------------------------------------------\n" << std::endl;
  }
} // main()
