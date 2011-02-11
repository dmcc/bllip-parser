// copy-sentences.cc
//
// Mark Johnson, 12th November 2003
//
// reads sentences one at a time from stdin and writes them to stdout
// with each sentence on a line of its own

#include "tree.h"

int main(int argc, char** argv) {

  tree* t;

  while ((t = readtree_root(stdin)) != NULL)
    std::cout << t << std::endl;

}
