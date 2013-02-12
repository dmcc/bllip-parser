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

// heads.h
//
// Mark Johnson, 9th November 2001, last modified 10th October 2003
//
// Contains code for syntactic and semantic head finding.
// Defined for arbitrary tree_node<> types, but only a single copy
// of the syntactic and semantic data is constructed.
//
// Most users will use the functions
//
//  tree_syntacticHeadChild()
//  tree_syntacticLexicalHead()
//  tree_semanticHeadChild()
//  tree_semanticLexicalHead()

#ifndef HEADS_H
#define HEADS_H

#include <ext/hash_map>
#include <vector>

#include "sym.h"
#include "symset.h"
#include "tree.h"

namespace heads {

  struct syntactic_data {
    typedef std::vector<symset> symsets;
    typedef ext::hash_map<symbol,const symsets*> sym_symsets;
    
    symsets adjective, conjunction, interjection, noun, preposition,
            unknown, verb;
    sym_symsets head_type;
    symset rightheaded_nominals;

    syntactic_data();   // initializer defined in heads.cc

    //! headchild() returns the head child of node t.
    //! It is defined in heads.h because it is a template function.
    //
    template <typename label_type> const tree_node<label_type>* 
    headchild(const tree_node<label_type>* t) const 
    {
      typedef tree_node<label_type> Tree;

      if (t == NULL || !t->is_nonterminal())
	return NULL;

      if (t->child->label.is_none() && t->child->next == NULL)
	return t->child;

      const Tree* head = NULL;    // search for head
      const sym_symsets::const_iterator it = head_type.find(t->label.cat);
      if (it == head_type.end()) {
	std::cerr << "heads::syntactic_data::headchild()"
	             " Error: can't find entry for category " 
		  << t->label.cat << std::endl << t << std::endl;
	abort();
      }

      const symsets& type = *it->second;
      cforeach (symsets, cats, type) {
	for (const Tree* child = t->child; child; child = child->next)
	  if (cats->contains(child->label.cat)) {
	    head = child;
	    if (type == verb || type == preposition 
		|| (type == noun && 
		    !rightheaded_nominals.contains(child->label.cat)) )
	      break;
	  }
	if (head != NULL)
	  return head;
      }

      // didn't find a head; return right-most non-punctuation preterminal

      for (const Tree* child = t->child; child; child = child->next)
	if (child->is_preterminal() && !child->is_punctuation())
	head = child;
      if (head != NULL)
	return head;

      // still no head -- return right-most non-punctuation

      for (const Tree* child = t->child; child; child = child->next)
	if (!child->is_punctuation())
	  head = child;
      if (head != NULL)
	return head;
      
      return NULL;
    }  // heads::syntactic_data::headchild()

  };  // heads::syntactic_data{}

  //! heads::syntactic() returns the static syntactic_data object.
  //! This function exists to avoid the static initialization bug.
  //
  const syntactic_data& syntactic();  // defined in heads.cc

  struct semantic_data {
    typedef std::vector<symset> symsets;
    typedef ext::hash_map<symbol,const symsets*> sym_symsets;

    symsets adjective, conjunction, interjection, noun, preposition, 
            unknown, verb;
    sym_symsets head_type;

    semantic_data();   // initializer defined in heads.cc


    template <typename label_type> const tree_node<label_type>* 
    headchild(const tree_node<label_type>* t) const 
    {
      typedef tree_node<label_type> Tree;

      if (t == NULL || !t->is_nonterminal())
	return NULL;

      if (t->child->label.is_none() && t->child->next == NULL)
	return t->child;

      const Tree* head = NULL;    // search for head
      const sym_symsets::const_iterator it = head_type.find(t->label.cat);
      if (it == head_type.end()) {
	std::cerr << "heads::semantic_data::headchild() "
                     "Error: can't find entry for category " 
		  << t->label.cat << std::endl << t << std::endl;
	abort();
      }
      const symsets& type = *it->second;
      cforeach (symsets, cats, type) {
	for (const Tree* child = t->child; child; child = child->next)
	  if (cats->contains(child->label.cat)) {
	    head = child;
	    if (type == verb || type == preposition)
	      break;
	  }
	if (head != NULL)
	  return head;
      }

      // didn't find a head; return right-most non-punctuation preterminal
      
      for (const Tree* child = t->child; child; child = child->next)
	if (child->is_preterminal() && !child->is_punctuation())
	  head = child;
      if (head != NULL)
	return head;

      // still no head -- return right-most non-punctuation
      
      for (const Tree* child = t->child; child; child = child->next)
	if (!child->is_punctuation())
	  head = child;
      if (head != NULL)
	return head;
      
      return NULL;
    }  // heads::semantic_data::headchild()

  };  // heads::semantic_data{}

  //! heads::semantic() returns the static semantic_data object.
  //! This function exists to avoid the static initialization bug.
  //
  const semantic_data& semantic();   // defined in heads.cc

}  // namespace heads


//! tree_syntacticHeadChild() returns a pointer to the
//!  child of t that is the syntactic head of t, or NULL
//!  if none exists (e.g., if t is a preterminal).
//
template <typename label_type> const tree_node<label_type>* 
tree_syntacticHeadChild(const tree_node<label_type>* t) 
{
  return heads::syntactic().headchild(t);
}  // tree_syntacticHeadChild()

//! tree_syntacticLexicalHead() returns a pointer to a
//!  preterminal node dominated by t that is its lexical
//! syntactic head.  Note that this node may be empty!
//
template <typename label_type> const tree_node<label_type>* 
tree_syntacticLexicalHead(const tree_node<label_type>* t)
{
 const tree_node<label_type>* head;
  while ((head = tree_syntacticHeadChild(t)) != NULL)
    t = head;
  return t;
}

//! tree_semanticHeadChild() returns a pointer to the
//!  child of t that is the semantic head of t, or NULL
//!  if none exists (e.g., if t is a preterminal).
//
template <typename label_type> const tree_node<label_type>* 
tree_semanticHeadChild(const tree_node<label_type>* t) 
{
  return heads::semantic().headchild(t);
}  // tree_semanticHeadChild()

//! tree_semanticLexicalHead() returns a pointer to a
//!  preterminal node dominated by t that is its lexical
//!  semantic head.  Note that this node may be empty!
//
template <typename label_type> const tree_node<label_type>* 
tree_semanticLexicalHead(const tree_node<label_type>* t)
{
 const tree_node<label_type>* head;
  while ((head = tree_semanticHeadChild(t)) != NULL)
    t = head;
  return t;
}

#endif // HEADS_H
