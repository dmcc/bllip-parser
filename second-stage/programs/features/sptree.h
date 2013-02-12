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

// sptree.h
//
// Mark Johnson, 9th October 2003, modified 19th November 2004

#ifndef SPTREE_H
#define SPTREE_H

#include <iostream>

#include "heads.h"
#include "sym.h"
#include "tree.h"

//! sptree_label{} adds string positions, parent and previous pointers 
//!  and syntactic and semantic head pointer to node labels.
//
class sptree_label : public tree_label {
public:
  const tree_node<sptree_label>* parent;
  const tree_node<sptree_label>* previous;
  const tree_node<sptree_label>* syntactic_headchild;
  const tree_node<sptree_label>* syntactic_lexhead;
  const tree_node<sptree_label>* semantic_headchild;
  const tree_node<sptree_label>* semantic_lexhead;
  unsigned int left, right;
  
  sptree_label(const tree_label& label) 
    : tree_label(label), parent(NULL), previous(NULL),
      syntactic_headchild(NULL), syntactic_lexhead(NULL),
      semantic_headchild(NULL), semantic_lexhead(NULL),
      left(0), right(0) { }

  bool operator==(const sptree_label& l) const {
    return cat == l.cat;
  }  // operator==

  bool operator< (const sptree_label& l) const {
    return cat < l.cat;
  }  // operator<

  bool is_syntactic_headchild() const {
    return parent != NULL && parent->label.syntactic_headchild->label == *this;
  }  // is_syntactic_headchild()
   
  bool is_semantic_headchild() const {
    return parent != NULL && parent->label.semantic_headchild->label == *this;
  }  // is_semantic_headchild()

};  // sptree_label{}

namespace EXT_NAMESPACE {
  template <> struct hash<sptree_label> {
    size_t operator()(const sptree_label& l) const {
      return hash<symbol>()(l.cat);
    }
  };  // hash<tree_label>{}
}; // namespace EXT_NAMESPACE

inline
std::ostream& operator<< (std::ostream& os, const sptree_label& l) {
  os << l.cat.c_str();
  // os << ':' << l.left << '-'<< l.right;
  // os << ':' << (l.parent ? l.parent->label.cat : l.none());
  // os << ':' << (l.previous ? l.previous->label.cat : l.none());
  // os << ':' << (l.syntactic_lexhead ? l.syntactic_lexhead->child->label.cat : l.none());
  // os << ':' << (l.semantic_lexhead ? l.semantic_lexhead->child->label.cat : l.none());
  return os;
}  // operator<<()

// sptree is the class of tree_node with string positions and
// parent pointers.
//
typedef tree_node<sptree_label> sptree;

inline symbol downcase(symbol cat) {
  std::string s(cat.string_reference());
  for (std::string::iterator it = s.begin(); it != s.end(); ++it)
    *it = tolower(*it);
  return symbol(s);
}

//! tree_sptree_helper() is a helper function that actually copies the trees.
//
template <typename label_type>
sptree* tree_sptree_helper(bool downcase_flag, const tree_node<label_type>* tp0, 
			   sptree* parent, sptree* previous, unsigned int& position)
{
  sptree* tp = new sptree(downcase_flag && tp0->is_terminal() 
			  ? downcase(tp0->label.cat) : tp0->label.cat);
  tp->label.left = position;
  tp->label.parent = parent;
  tp->label.previous = previous;

  if (tp0->child == NULL) {
    tp->child = NULL;
    ++position;
  }
  else 
    tp->child = tree_sptree_helper(downcase_flag, tp0->child, tp, NULL, position);

  tp->label.right = position;

  if (tp0->next == NULL) 
    tp->next = NULL;
  else
    tp->next = tree_sptree_helper(downcase_flag, tp0->next, parent, tp, position);

  sptree_label& label = tp->label;

  if (tp->is_nonterminal()) {
    label.syntactic_headchild = tree_syntacticHeadChild(tp);
    label.syntactic_lexhead = 
      (label.syntactic_headchild == NULL 
       ? NULL : label.syntactic_headchild->label.syntactic_lexhead);
    label.semantic_headchild = tree_semanticHeadChild(tp);
    label.semantic_lexhead = 
      (label.semantic_headchild == NULL
       ? NULL : label.semantic_headchild->label.semantic_lexhead);
  }
  else {
    label.syntactic_headchild = label.semantic_headchild = NULL;
    label.syntactic_lexhead = label.semantic_lexhead = tp->is_terminal() ? NULL : tp;
  }
  
  return tp;
}

//! tree_sptree() maps a standard tree to an sptree.  This does
//! not free tp.
//
template <typename label_type>
sptree* tree_sptree(const tree_node<label_type>* tp, bool downcase_flag=false)
{
  unsigned int position = 0;
  return tree_sptree_helper(downcase_flag, tp, NULL, NULL, position);
}

template <> tree_node<sptree_label>* copy_treeptr(const tree_node<sptree_label>* tp)
{
  return tree_sptree(tp);
}

#endif // SPTREE_H
