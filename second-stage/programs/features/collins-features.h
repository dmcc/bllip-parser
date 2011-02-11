// collins-features.h
//
// Mark Johnson, 8th October 2003
//
// Feature classes from Collins and Koo CL article, plus CollinsWords

#ifndef COLLINS_FEATURES_H
#define COLLINS_FEATURES_H

#include <algorithm>
#include <cassert>

#include "features.h"
#include "heads.h"
#include "tree.h"

//! CollinsRule{} implements Collins' "Rule", "Grandparent rule" 
//! and "Two level rule" feature classes.  
//!
//! Collin's "Rule" is implemented by CollinsRule(0, 0, none) 
//! and CollinsRule(0, 0, closed_class).
//!
//! Collins' "Grandparent rule" is implemented by 
//! CollinsRule(0, 1, none) and CollinsRule(0, 1, closed_class).
//!
//! Collin's "Two level rule" is implemented by
//! CollinsRule(1, 0, none) and CollinsRule(1, 0, closed_class).
//!
//! Identifier is CollinsRule:<nanctrees>:<nanccats>:<annotation>.
//!
class CollinsRule : public NodePathFeatureClass {
public:
  
  enum annotation_type { none, closed_class, all };

  CollinsRule(size_t nanctrees = 0,        //!< Number of ancestor local trees
	      size_t nanccats = 0,         //!< Number of ancestor categories above trees
	      annotation_type annotation = none //!< Amount of head annotation to use
	      ) : nanctrees(nanctrees), nanccats(nanccats), annotation(annotation),
		  identifier_string("CollinsRule:") {
    identifier_string += boost::lexical_cast<std::string>(nanctrees) + ":";
    identifier_string += boost::lexical_cast<std::string>(nanccats) + ":";
    identifier_string += boost::lexical_cast<std::string>(annotation);
  } // CollinsRule::CollinsRule()

  size_t nanctrees;
  size_t nanccats;
  annotation_type annotation;
  std::string identifier_string;

  template <typename FeatClass, typename Feat_Count>
  void node_featurecount(FeatClass& fc, const tree* node, 
			 Feat_Count& feat_count, const TreePtrs& ancestors) {

    static symbol childmarker("*CHILD*");

    if (ancestors.size() < nanctrees + nanccats || !node->is_nonterminal())
      return;

    Feature f;

    // push (possibly lexicalized) children
    
    for (const tree* child = node->child; child != NULL; child = child->next) {
      f.push_back(child->label.cat);
      if (annotation != none) 
	f.push_back(lexhead(child));
    }

    const tree* tp = node;    // tp is last node visited = current child of next node up
    size_t ia = ancestors.size();

    // push (possibly lexicalized) ancestor rules

    for (size_t i = 0; i < nanctrees; ++i) {
      assert(ia > 0);
      --ia;
      f.push_back(endmarker());
      for (const tree* child = ancestors[ia]->child; child != NULL; 
	   child = child->next) {
	f.push_back(child->label.cat);
	if (child == tp)
	  f.push_back(childmarker);
	if (annotation != none)
	  f.push_back(lexhead(child));
      }
      tp = ancestors[ia];
    }
	
    // push additional ancestor categories

    for (size_t i = 0; i < nanccats; ++i) {
      assert(ia > 0);
      --ia;
      f.push_back(endmarker());
      f.push_back(tp->label.cat);
      tp = ancestors[ia];
    }

    // push back final ancestor label
    
    f.push_back(tp->label.cat);

    // count the feature

    ++feat_count[f];
  }  // CollinsRule::node_featurecount()


  //! lexhead() returns the lexical head of node or endmarker()
  //
  symbol lexhead(const tree* node) {
    if (annotation == none)
      return endmarker();
    else {
      const tree* headnode = tree_semanticLexicalHead(node);
      if (headnode && (annotation == all || headnode->is_closed_class()))
	return headnode->child->label.cat;
      else
	return endmarker();
    }
  }  // CollinsRule::lexhead()

  typedef std::vector<symbol> Feature;
  typedef ext::hash_map<Feature,Id> Feature_Id; 
  Feature_Id feature_id;
  
  virtual const char *identifier() const {
    return identifier_string.c_str();
  }  // CollinsRule::identifier()

  // These virtual functions just pass control to the static template functions
  //
  virtual void extract_features(const sentence_type& s) {
    extract_features_helper(*this, s);
  }  // CollinsRuleFeatureClass::extract_features()

  virtual Id prune_and_renumber(const size_t mincount, Id nextid,
				std::ostream& os) {
    return prune_and_renumber_helper(*this, mincount, nextid, os);
  }  // CollinsRule::prune_and_renumber()

  virtual void feature_values(const sentence_type& s, Id_Floats& p_i_v) {
    feature_values_helper(*this, s, p_i_v);
  }  // CollinsRule::feature_values()

};  // CollinsRule{}


//! CollinsWords{} is a version of CollinsRule that collects data
//! on preterminals and their terminal children.  You probably want
//! to use this with a considerable amount of parent and head annotation.
//!
//! Identifier is CollinsWords:<nanctrees>:<nanccats>:<annotation>.
//!
class CollinsWords : public NodePathFeatureClass {
public:
  
  enum annotation_type { none, closed_class, all };

  CollinsWords(size_t nanctrees = 0,        //!< Number of ancestor local trees
	       size_t nanccats = 0,         //!< Number of ancestor categories above trees
	       annotation_type annotation = none //!< Amount of head annotation to use
	       ) : nanctrees(nanctrees), nanccats(nanccats), annotation(annotation),
		   identifier_string("CollinsWords:") {
    identifier_string += boost::lexical_cast<std::string>(nanctrees) + ":";
    identifier_string += boost::lexical_cast<std::string>(nanccats) + ":";
    identifier_string += boost::lexical_cast<std::string>(annotation);
  } // CollinsWords::CollinsWords()

  size_t nanctrees;
  size_t nanccats;
  annotation_type annotation;
  std::string identifier_string;

  template <typename FeatClass, typename Feat_Count>
  void node_featurecount(FeatClass& fc, const tree* node, 
			 Feat_Count& feat_count, const TreePtrs& ancestors) {

    static symbol childmarker("*CHILD*");

    if (ancestors.size() < nanctrees + nanccats || !node->is_preterminal())
      return;

    Feature f;

    // push (possibly lexicalized) children
    
    for (const tree* child = node->child; child != NULL; child = child->next) 
      f.push_back(child->label.cat);

    const tree* tp = node;    // tp is last node visited = current child of next node up
    size_t ia = ancestors.size();

    // push (possibly lexicalized) ancestor rules

    for (size_t i = 0; i < nanctrees; ++i) {
      assert(ia > 0);
      --ia;
      f.push_back(endmarker());
      for (const tree* child = ancestors[ia]->child; child != NULL; 
	   child = child->next) {
	f.push_back(child->label.cat);
	if (child == tp)
	  f.push_back(childmarker);
	if (annotation != none)
	  f.push_back(lexhead(child));
      }
      tp = ancestors[ia];
    }
	
    // push additional ancestor categories

    for (size_t i = 0; i < nanccats; ++i) {
      assert(ia > 0);
      --ia;
      f.push_back(endmarker());
      f.push_back(tp->label.cat);
      tp = ancestors[ia];
    }

    // push back final ancestor label
    
    f.push_back(tp->label.cat);

    // count the feature

    ++feat_count[f];
  }  // CollinsWords::node_featurecount()


  //! lexhead() returns the lexical head of node or endmarker()
  //
  symbol lexhead(const tree* node) {
    if (annotation == none)
      return endmarker();
    else {
      const tree* headnode = tree_semanticLexicalHead(node);
      if (headnode && (annotation == all || headnode->is_closed_class()))
	return headnode->child->label.cat;
      else
	return endmarker();
    }
  }  // CollinsWords::lexhead()

  typedef std::vector<symbol> Feature;
  typedef ext::hash_map<Feature,Id> Feature_Id; 
  Feature_Id feature_id;
  
  virtual const char *identifier() const {
    return identifier_string.c_str();
  }  // CollinsWords::identifier()

  // These virtual functions just pass control to the static template functions
  //
  virtual void extract_features(const sentence_type& s) {
    extract_features_helper(*this, s);
  }  // CollinsWordsFeatureClass::extract_features()

  virtual Id prune_and_renumber(const size_t mincount, Id nextid,
				std::ostream& os) {
    return prune_and_renumber_helper(*this, mincount, nextid, os);
  }  // CollinsWords::prune_and_renumber()

  virtual void feature_values(const sentence_type& s, Id_Floats& p_i_v) {
    feature_values_helper(*this, s, p_i_v);
  }  // CollinsWords::feature_values()

};  // CollinsWords{}


//! CollinsBigram{} implements Collins' "Bigram", "Grandparent bigram" 
//! "Lexical bigram" and "Two level bigram" feature classes.  
//!
//! Collin's "Bigram" is implemented by CollinsBigram(0, 0, none) 
//! and CollinsBigram(0, 0, closed_class).
//!
//! Collins' "Grandparent bigram" is implemented by 
//! CollinsBigram(0, 1, none) and CollinsBigram(0, 1, closed_class).
//!
//! Collins' "Lexical bigram" is implemented by  CollinsBigram(0, 0, all)
//!
//! Collin's "Two level bigram" is implemented by
//! CollinsBigram(1, 0, none) and CollinsBigram(1, 0, closed_class).
//
class CollinsBigram : public NodePathFeatureClass {
public:

  enum annotation_type { none, closed_class, all };

  CollinsBigram(size_t nanctrees = 0,  //!< Number of ancestor local trees
		//! Number of ancestor categories above trees
		size_t nanccats = 0,   
		//! Amount of head annotation to use
		annotation_type annotation = none) 
    : nanctrees(nanctrees), nanccats(nanccats), annotation(annotation),
      identifier_string("CollinsBigram:") {
    identifier_string += boost::lexical_cast<std::string>(nanctrees) + ":";
    identifier_string += boost::lexical_cast<std::string>(nanccats) + ":";
    identifier_string += boost::lexical_cast<std::string>(annotation);
  } // CollinsBigram::CollinsBigram()

  size_t nanctrees;
  size_t nanccats;
  annotation_type annotation;
  std::string identifier_string;

  template <typename FeatClass, typename Feat_Count>
  void node_featurecount(FeatClass& fc, const tree* node, 
			 Feat_Count& feat_count, const TreePtrs& ancestors) {

    static symbol childmarker("*CHILD*");
    static symbol leftmarker("*LEFT*");
    static symbol rightmarker("*RIGHT*");

    if (ancestors.size() < nanctrees + nanccats || !node->is_nonterminal())
      return;

    Feature f;

    const tree* tp = node;    // tp is last node visited = current child of next node up
    size_t ia = ancestors.size();

    // push (possibly lexicalized) ancestor rules

    for (size_t i = 0; i < nanctrees; ++i) {
      assert(ia > 0);
      --ia;
      for (const tree* child = ancestors[ia]->child; child != NULL; 
	   child = child->next) {
	f.push_back(child->label.cat);
	if (child == tp)
	  f.push_back(childmarker);
	if (annotation != none)
	  f.push_back(lexhead(child));
      }
      f.push_back(endmarker());
      tp = ancestors[ia];
    }
	
    // push additional ancestor categories

    for (size_t i = 0; i < nanccats; ++i) {
      assert(ia > 0);
      --ia;
      f.push_back(tp->label.cat);
      f.push_back(endmarker());
      tp = ancestors[ia];
    }

    // push back final ancestor label
    
    f.push_back(tp->label.cat);
    f.push_back(endmarker());

    // now construct the bigrams

    const tree* headchild = tree_semanticHeadChild(node);
    size_t f_base_size = f.size();

    const tree* lastchild = NULL;
    symbol lastcat = endmarker();
    symbol lasthead = endmarker();
    symbol direction = leftmarker;

    for (const tree* child = node->child; child != NULL; child = child->next) {
      f.push_back(direction);
      f.push_back(lastcat);
      f.push_back(child->label.cat);
      lastchild = child;
      lastcat = child->label.cat;
      if (annotation != none) {
	symbol head = lexhead(child);
	f.push_back(lasthead);
	f.push_back(head);
	lasthead = head;
      }

      if (lastchild != headchild && child != headchild)
	++feat_count[f];        // count the feature
      else
	direction = rightmarker;

      f.resize(f_base_size);  // trim off excess elements
    }

    f.push_back(direction);
    f.push_back(lastcat);
    f.push_back(endmarker());
    if (annotation != none) {
      f.push_back(endmarker());
      f.push_back(lasthead);
    }
    ++feat_count[f];
    
  }  // CollinsBigram::node_featurecount()

  //! lexhead() returns the lexical head of node or endmarker()
  //
  symbol lexhead(const tree* node) {
    if (node == NULL || annotation == none)
      return endmarker();
    else {
      const tree* headnode = tree_semanticLexicalHead(node);
      if (headnode && (annotation == all || headnode->is_closed_class()))
	return headnode->child->label.cat;
      else
	return endmarker();
    }
  }  // CollinsBigram::lexhead()

  typedef std::vector<symbol> Feature;
  typedef ext::hash_map<Feature,Id> Feature_Id; 
  Feature_Id feature_id;
  
  virtual const char *identifier() const {
    return identifier_string.c_str();
  }  // CollinsBigram::identifier()

  // These virtual functions just pass control to the static template functions
  //
  virtual void extract_features(const sentence_type& s) {
    extract_features_helper(*this, s);
  }  // CollinsBigram::extract_features()

  virtual Id prune_and_renumber(const size_t mincount, Id nextid,
				std::ostream& os) {
    return prune_and_renumber_helper(*this, mincount, nextid, os);
  }  // CollinsBigram::prune_and_renumber()

  virtual void feature_values(const sentence_type& s, Id_Floats& p_i_v) {
    feature_values_helper(*this, s, p_i_v);
  }  // CollinsBigram::feature_values()

};  // CollinsBigram{}


//! The CollinsNGram{} are fragments of local trees, possibly
//! with ancestor annotation.
//!
//! All contiguous subsequences of children categories are collected
//! of lengths fraglen (including endmarker).  Heads are marked.
//!
//! Collins Trigram feature class is implemented by
//! CollinsNGram(0, 3, none) and 
//! CollinsNGram(0, 3, closed_class)
//
class CollinsNGram : public NodePathFeatureClass {
public:

  enum annotation_type { none, closed_class, all };

  CollinsNGram(size_t nanccats = 0,   //!< number of ancestors
	       size_t fraglen = 3,    //!< n-gram length
	       //! Amount of head annotation to use
	       annotation_type annotation = none)
    : nanccats(nanccats), fraglen(fraglen), annotation(annotation),
      identifier_string("CollinsNGram:") 
  { 
    identifier_string += boost::lexical_cast<std::string>(fraglen);
    identifier_string += ':';
    identifier_string += boost::lexical_cast<std::string>(nanccats);
    identifier_string += ':';
    identifier_string += boost::lexical_cast<std::string>(annotation);
  }  // CollinsNGram::CollinsNGramFeatureClass()

  //! The identifier is CollinsNGram:<fraglen>:<nanccats>:<annotation>
  //   
  virtual const char* identifier() const {
    return identifier_string.c_str();
  }  // CollinsNGram::identifier()

  size_t nanccats;                  //!< number of ancestors
  size_t fraglen;                   //!< fragment length
  annotation_type annotation;
  std::string identifier_string;

  template <typename FeatClass, typename Feat_Count>
  void node_featurecount(FeatClass& fc, const tree* node, 
			 Feat_Count& feat_count, const TreePtrs& ancestors) {

    static symbol headmarker("*HEAD*");

    if (ancestors.size() < nanccats || !node->is_nonterminal())
      return;

    size_t nchildren = 0;
    for (const tree* child = node->child; child != NULL; child = child->next)
      ++nchildren;

    if (nchildren < fraglen)
      return;

    const tree* headchild = tree_semanticHeadChild(node);

    typedef std::vector<const tree*> Tptrs;
    Tptrs children;

    children.push_back(NULL);
    for (const tree* child = node->child; child != NULL; child = child->next) 
      children.push_back(child);
    children.push_back(NULL);

    for (size_t start = 0; start+fraglen <= children.size(); ++start) {
      Feature f;
      f.push_back(node->label.cat);      // push parent's category
   
      for (size_t i0 = 1; i0 <= nanccats; ++i0) { // push ancestors' category
	size_t i = ancestors.size() - i0;
	f.push_back(ancestors[i]->label.cat);
      }

      for (size_t pos = start; pos < start+fraglen; ++pos) {
	const tree* child = children[pos];
	if (child != NULL) {
	  f.push_back(child->label.cat);  // push child's category
	  if (child == headchild)
	    f.push_back(headmarker);
	  if (annotation != none) 
	    f.push_back(lexhead(child));
	}
	else
	  f.push_back(endmarker());
      }
      ++feat_count[f];
    }
  }  // CollinsNGram::node_featurecount()
 
  //! lexhead() returns the lexical head of node or endmarker()
  //
  symbol lexhead(const tree* node) {
    if (node == NULL || annotation == none)
      return endmarker();
    else {
      const tree* headnode = tree_semanticLexicalHead(node);
      if (headnode && (annotation == all || headnode->is_closed_class()))
	return headnode->child->label.cat;
      else
	return endmarker();
    }
  }  // CollinsBigram::lexhead()

  typedef std::vector<symbol> Feature;
  typedef ext::hash_map<Feature,Id> Feature_Id; 
  Feature_Id feature_id;

  // These virtual functions just pass control to the static template functions
  //
  virtual void extract_features(const sentence_type& s) {
    extract_features_helper(*this, s);
  }  // CollinsNGram::extract_features()

  virtual Id prune_and_renumber(const size_t mincount, Id nextid,
				std::ostream& os) {
    return prune_and_renumber_helper(*this, mincount, nextid, os);
  }  // CollinsNGram::prune_and_renumber()

  virtual void feature_values(const sentence_type& s, Id_Floats& p_i_v) {
    feature_values_helper(*this, s, p_i_v);
  }  // CollinsNGram::feature_values()

};  // CollinsNGram{}


//! CollinsHeadMod{} implements Collins' Head-Modifiers feature class.
//!
//! Collin's "Head-Modifier" is implemented by CollinsHeadMod(0, 1, none).
//
class CollinsHeadMod : public NodePathFeatureClass {
public:

  enum annotation_type { none, closed_class, all };

  CollinsHeadMod(size_t nanccats = 0, annotation_type annotation=none) 
    : nanccats(nanccats), annotation(annotation),
      identifier_string("CollinsHeadMod:") {
    identifier_string += boost::lexical_cast<std::string>(nanccats) + ':';
    identifier_string += boost::lexical_cast<std::string>(annotation);
  } // CollinsHeadMod::CollinsHeadMod()

  size_t nanctrees;
  size_t nanccats;
  annotation_type annotation;
  std::string identifier_string;

  template <typename FeatClass, typename Feat_Count>
  void node_featurecount(FeatClass& fc, const tree* node, 
			 Feat_Count& feat_count, const TreePtrs& ancestors) {

    static symbol adjacentmarker("*ADJACENT*");
    static symbol leftmarker("*LEFT*");
    static symbol rightmarker("*RIGHT*");

   if (ancestors.size() < nanccats || !node->is_nonterminal())
      return;

    const tree* headchild = tree_semanticHeadChild(node);
    if (headchild == NULL)
      return;

    Feature f;

    // annotate head if required 

    if (annotation != none) 
      f.push_back(lexhead(headchild));

    // tp is last node visited = current child of next node up

    const tree* tp = node; 
    size_t ia = ancestors.size();
	
    // push ancestor categories

    for (size_t i = 0; i < nanccats; ++i) {
      assert(ia > 0);
      --ia;
      f.push_back(tp->label.cat);
      tp = ancestors[ia];
    }

    // push back final ancestor label
    
    f.push_back(tp->label.cat);
    f.push_back(headchild->label.cat);
    if (annotation != none)
      f.push_back(lexhead(headchild));

    size_t f_base_size = f.size();

    const tree* lastchild = NULL;
    symbol direction = leftmarker;

    for (const tree* child = node->child; child != NULL; child = child->next) {
      if (child == headchild) 
	direction = rightmarker;
      else {
	f.resize(f_base_size);  // trim off excess elements
	f.push_back(direction);
	if (child->next == headchild || lastchild == headchild)
	  f.push_back(adjacentmarker);
	
	f.push_back(child->label.cat);
	if (annotation != none) 
	  f.push_back(lexhead(child));
	++feat_count[f];        // count the feature
      }

      lastchild = child;
    }
  }  // CollinsHeadMod::node_featurecount()

  //! lexhead() returns the lexical head of node or endmarker()
  //
  symbol lexhead(const tree* node) {
    if (node == NULL || annotation == none)
      return endmarker();
    else {
      const tree* headnode = tree_semanticLexicalHead(node);
      if (headnode && (annotation == all || headnode->is_closed_class()))
	return headnode->child->label.cat;
      else
	return endmarker();
    }
  }  // CollinsHeadMod::lexhead()

  typedef std::vector<symbol> Feature;
  typedef ext::hash_map<Feature,Id> Feature_Id; 
  Feature_Id feature_id;
  
  virtual const char *identifier() const {
    return identifier_string.c_str();
  }  // CollinsHeadMod::identifier()

  // These virtual functions just pass control to the static template functions
  //
  virtual void extract_features(const sentence_type& s) {
    extract_features_helper(*this, s);
  }  // CollinsHeadModFeatureClass::extract_features()

  virtual Id prune_and_renumber(const size_t mincount, Id nextid,
				std::ostream& os) {
    return prune_and_renumber_helper(*this, mincount, nextid, os);
  }  // CollinsHeadMod::prune_and_renumber()

  virtual void feature_values(const sentence_type& s, Id_Floats& p_i_v) {
    feature_values_helper(*this, s, p_i_v);
  }  // CollinsHeadMod::feature_values()

};  // CollinsHeadMod{}



//! CollinsPP{} implements Collins' "PP" feature classes.  
//!
//! Collin's "PP" is implemented by CollinsPP(false) 
//! and CollinsPP(true).
//
class CollinsPP : public NodePathFeatureClass {
public:

  CollinsPP(bool lexicalize_governor = false  //!< lexicalize PP's governor
	    ) : lexicalize_governor(lexicalize_governor),
		identifier_string("CollinsPP:") {
    identifier_string += boost::lexical_cast<std::string>(lexicalize_governor);
  } // CollinsPP::CollinsPP()

  //! The identifier is CollinsPP:<lexicalize_governor>
  //
  virtual const char *identifier() const {
    return identifier_string.c_str();
  }  // CollinsPP::identifier()

  bool lexicalize_governor;
  std::string identifier_string;

  template <typename FeatClass, typename Feat_Count>
  void node_featurecount(FeatClass& fc, const tree* node, 
			 Feat_Count& feat_count, const TreePtrs& ancestors) {

    static symbol pp("PP");

    if (node->label.cat != pp)
      return;

    assert(ancestors.size() >= 1);

    Feature f;

    const tree* parent = ancestors[ancestors.size()-1];
    f.push_back(parent->label.cat);
    
    const tree* head = tree_semanticHeadChild(parent);
    f.push_back(head->label.cat);

    if (lexicalize_governor) {
      const tree* lexhead = tree_semanticLexicalHead(head);
      if (lexhead)
	f.push_back(lexhead->child->label.cat);
    }

    f.push_back(node->label.cat);
    head = tree_syntacticHeadChild(node);
    f.push_back(lexhead(head));
    
    const tree* complement = head->next;
    if (complement != NULL) {
      f.push_back(complement->label.cat);
      f.push_back(lexhead(complement));
    }
    else {
      f.push_back(endmarker());
      f.push_back(endmarker());
    }

    ++feat_count[f];
  }  // CollinsPP::node_featurecount()

  //! lexhead() returns the lexical head of node or endmarker()
  //
  static symbol lexhead(const tree* node) {
    if (node == NULL)
      return endmarker();
    else {
      const tree* headnode = tree_semanticLexicalHead(node);
      if (headnode != NULL)
	return headnode->child->label.cat;
      else
	return endmarker();
    }
  }  // CollinsPP::lexhead()

  typedef std::vector<symbol> Feature;
  typedef ext::hash_map<Feature,Id> Feature_Id; 
  Feature_Id feature_id;
  
  // These virtual functions just pass control to the static template functions
  //
  virtual void extract_features(const sentence_type& s) {
    extract_features_helper(*this, s);
  }  // CollinsPP::extract_features()

  virtual Id prune_and_renumber(const size_t mincount, Id nextid,
				std::ostream& os) {
    return prune_and_renumber_helper(*this, mincount, nextid, os);
  }  // CollinsPPFeatureClass::prune_and_renumber()

  virtual void feature_values(const sentence_type& s, Id_Floats& p_i_v) {
    feature_values_helper(*this, s, p_i_v);
  }  // CollinsPP::feature_values()

};  // CollinsPP{}


//! CollinsDist{} implements Collins' "Distance Head-Modifiers" feature class.
//!  
//! Collins' "Distance Head-Modifiers" is implemented by
//! CollinsDist(leq), CollinsDist(eq) and CollinsDist(geq).
//
class CollinsDist : public PTsFeatureClass {
public:

  enum distance_type { leq, eq, geq };

  CollinsDist(distance_type dist = eq) 
    : dist(dist), identifier_string("CollinsDist:") {
    identifier_string += boost::lexical_cast<std::string>(dist);
  }

  //! The identifier is CollinsDist
  //
  virtual const char* identifier() const {
    return identifier_string.c_str();
  }  // CollinsDistFeatureClass::identifier()

  distance_type dist;
  std::string identifier_string;

  typedef std::vector<symbol> symbols;
  typedef std::pair<symbols,int> Feature;
  typedef ext::hash_map<Feature,Id> Feature_Id; 
  Feature_Id feature_id;
  
  template <typename FeatClass, typename Feat_Count>
  void node_featurecount(FeatClass& fc, const tree* node, 
			 Feat_Count& feat_count, 
			 const TreePtrs& preterminals,
			 const TreePtrs& ancestors,
			 size_t leftpos, size_t rightpos) {

    if (!node->is_nonterminal())
      return;

    const tree* headchild = tree_semanticHeadChild(node);
    if (headchild == NULL)
      return;
    
    int headloc = location(headchild, preterminals);
    for (const tree* child = node->child; child != NULL; child = child->next) 
      if (child != headchild && !child->is_punctuation()) {
	int deploc = location(child, preterminals);
	int distance = abs(headloc-deploc);
	Feature f;
	f.first.push_back(node->label.cat);
	f.first.push_back(headchild->label.cat);
	f.first.push_back(child->label.cat);
	f.second = distance;
	++feat_count[f];

	if (dist == leq) 
	  for (int i = 1; i < distance; ++i) {
	    f.second = i;
	    ++feat_count[f];
	  }
	
	if (dist == geq)
	  for (int i = distance+1; i <= 9; ++i) {
	    f.second = i;
	    ++feat_count[f];
	  }
      }
  }  // CollinsDistFeatureClass::node_featurecount()

  //! location() finds the location of the lexical head of the node
  //! in the vector of lexical POS words
  //
  static int location(const tree* node, const TreePtrs& preterminals) {
    assert(node != NULL);
    const tree* head = tree_semanticLexicalHead(node);
    assert(head != NULL);
    const TreePtrs::const_iterator it = 
      find(preterminals.begin(), preterminals.end(), head);
    assert(it != preterminals.end());
    return it - preterminals.begin();
  }  // CollinsDistFeatureClass::location()
  
  // These virtual functions just pass control to the static template functions
  //
  virtual void extract_features(const sentence_type& s) {
    extract_features_helper(*this, s);
  }  // CollinsDistFeatureClass::extract_features()

  virtual Id prune_and_renumber(const size_t mincount, Id nextid,
				std::ostream& os) {
    return prune_and_renumber_helper(*this, mincount, nextid, os);
  }  // CollinsDistFeatureClass::prune_and_renumber()

  virtual void feature_values(const sentence_type& s, Id_Floats& p_i_v) {
    feature_values_helper(*this, s, p_i_v);
  }  // CollinsDistFeatureClass::feature_values()

};  // CollinsDistFeatureClass{}


#endif // COLLINS_FEATURES_H
