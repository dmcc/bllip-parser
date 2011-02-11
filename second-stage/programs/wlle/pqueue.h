// pqueue.h
//
// (c) Mark Johnson, 4th August 2000
// modified (c) Mark Johnson, 29th August 2002 for gcc 3.X compatibility
// modified (c) Mark Johnson, 17th January 2003 removed "using namespace" directives
// modified (c) Mark Johnson, 3rd November 2003 added raise() and lower(),
//   made PriorityCompareType a template parameter
//
// Priority queues pair keys with priorities
//
// The elements with lowest priority values are removed first

#ifndef PQUEUE_H
#define PQUEUE_H

#include <cassert>
#include <ext/hash_map>
#include <functional>
#include <iostream>
#include <utility>
#include <vector>

#if (__GNUC__ > 3) || (__GNUC__ >= 3 && __GNUC_MINOR__ >= 1)
#define EXT_NAMESPACE __gnu_cxx
#else
#define EXT_NAMESPACE std
#endif

namespace ext = EXT_NAMESPACE;

template <class KeyType, class PriorityType, 
	  class PriorityCompareType=std::less<PriorityType> >
class pqueue
{
  struct  PI { 
    PriorityType priority; 
    size_t index; 
    PI(const PriorityType& p, size_t i) : priority(p), index(i) { }
  };

  typedef ext::hash_map<KeyType,PI> K_PI;
  typedef typename K_PI::value_type KPI;
  typedef KPI                       *KPIp;
  typedef std::vector<KPIp>         KPIps;

  K_PI  k_pi;
  KPIps i_kpip;

  size_t parent(size_t i) const { return (i-1)/2; }
  size_t left(size_t i) const { return 2*i+1; }
  size_t right(size_t i) const { return 2*i+2; }

  void swap(size_t i, size_t j) {
    assert(i < size() );
    assert(j < size() );
    KPI *pi = i_kpip[i];
    KPI *pj = i_kpip[j];
    (i_kpip[i] = pj)->second.index = i;
    (i_kpip[j] = pi)->second.index = j;
    assert(i_kpip[i]->second.index == i);
    assert(i_kpip[j]->second.index == j);
  }

  const PriorityType& priority(size_t i) { 
    assert(i < size() ); 
    return i_kpip[i]->second.priority; 
  }

  void adjust_down(size_t i) {
    size_t l = left(i);
    size_t r = right(i);
    size_t smallest = i;
    if (l < size() && PriorityCompareType()(priority(l), priority(i)))
      smallest = l;
    if (r < size() && PriorityCompareType()(priority(r), priority(smallest)))
      smallest = r;
    if (smallest != i) {
      swap(i, smallest);
      adjust_down(smallest);
    }
  }

  void adjust_up(size_t i) {
    if (i > 0) {
      size_t p = parent(i);
      if (PriorityCompareType()(priority(i), priority(p))) {
	swap(i, p);
	adjust_up(p);
      }}
  }

  void adjust(size_t i) {
    if ((i > 0) && PriorityCompareType()(priority(i), priority(parent(i))))
      adjust_up(i);
    else
      adjust_down(i);
  }

public:
  typedef KeyType             key_type;
  typedef PriorityType        priority_type;
  typedef PriorityCompareType priority_compare_type;

  // default constructor
  //
  pqueue() : k_pi(), i_kpip() { }

  // copy constructor
  //
  pqueue(const pqueue& pq) : k_pi(pq.k_pi), i_kpip(pq.i_kpip.size()) {
    assert(pq.k_pi.size() == pq.i_kpip.size());
    for (typename K_PI::iterator it = k_pi.begin(); it != k_pi.end(); ++it) {
      assert(it->second.index < i_kpip.size());
      i_kpip[it->second.index] = &(*it);
    }
    assert(k_pi.size() == i_kpip.size());
  }  // pqueue::pqueue()

  // equals
  //
  pqueue& operator= (const pqueue& pq) {
    if (&pq != this) {
      k_pi = pq.k_pi;
      i_kpip.resize(pq.i_kpip.size());
      for (typename K_PI::iterator it = k_pi.begin(); it != k_pi.end(); ++it) {
	assert(it->second.index < i_kpip.size());
	i_kpip[it->second.index] = &(*it);
      }
    }
    assert(k_pi.size() == i_kpip.size());
    return *this;
  }  // operator=()

  bool empty() const { return i_kpip.empty(); }

  size_t size() const { return i_kpip.size(); }

  const KeyType& top_key() const { assert(!i_kpip.empty()); return i_kpip[0]->first; }

  const PriorityType& top_priority() const { assert(!i_kpip.empty()); return i_kpip[0]->second.priority; }

  //! set() sets key to priority.
  //
  bool set(const KeyType& key, const PriorityType& priority) {
    typedef typename K_PI::iterator K_PIi;
    std::pair<K_PIi,bool> r = k_pi.insert(std::make_pair(key,PI(priority,k_pi.size())));
    if (r.second) {
      i_kpip.push_back(&*r.first);   // key is newly inserted
      assert(k_pi.size() == i_kpip.size());
      adjust_up(r.first->second.index);
    }
    else {
      r.first->second.priority = priority;
      adjust(r.first->second.index);
    }
    assert(k_pi.size() == i_kpip.size());
    return r.second;
  }  // pqueue::set()

  //! lower() changes the priority assigned to key if the key's current priority
  //! is greater than priority, or the key is not in the priority queue, and
  //! returns true if key was already present, false otherwise.
  //
  bool lower(const KeyType& key, const PriorityType& priority) {
    typedef typename K_PI::iterator K_PIi;
    std::pair<K_PIi,bool> r = k_pi.insert(std::make_pair(key,PI(priority,size())));
    if (r.second) {
      i_kpip.push_back(&*r.first);   // key is newly inserted
      assert(k_pi.size() == i_kpip.size());
      adjust_up(r.first->second.index);
    }
    else {
      if (PriorityCompareType()(priority, r.first->second.priority)) {
	r.first->second.priority = priority;
	adjust(r.first->second.index);
      }
    }
    assert(k_pi.size() == i_kpip.size());
    return r.second;
  }  // pqueue::lower()  

  //! raise() changes the priority assigned to key if the key's current priority
  //! is less than priority, or the key is not in the priority queue, and
  //! returns true if key was already present, false otherwise.
  //
  bool raise(const KeyType& key, const PriorityType& priority) {
    typedef typename K_PI::iterator K_PIi;
    std::pair<K_PIi,bool> r = k_pi.insert(std::make_pair(key,PI(priority,size())));
    if (r.second) {
      i_kpip.push_back(&*r.first);   // key is newly inserted
      assert(k_pi.size() == i_kpip.size());
      adjust_up(r.first->second.index);
    }
    else {
      if (PriorityCompareType()(r.first->second.priority, priority)) {
	r.first->second.priority = priority;
	adjust(r.first->second.index);
      }
    }
    assert(k_pi.size() == i_kpip.size());
    return r.second;
  }  // pqueue::raise()  

  //! pop() deletes the lowest scoring element from the priority queue
  //
  void pop() {
    assert(!empty());
    assert(k_pi.size() == i_kpip.size());
    swap(0, size()-1);
    KeyType k = i_kpip[i_kpip.size()-1]->first;
    size_t nerased = k_pi.erase(k);
    if (nerased != 1)
      std::cerr << "## pqueue.h nerased = " << nerased << ", k_pi.size() = " 
		<< k_pi.size() << ", i_kpip.size() = " << i_kpip.size() 
		<< ", k = " << k << std::endl;
    assert(nerased == 1);
    i_kpip.pop_back();
    adjust_down(0);
    assert(k_pi.size() == i_kpip.size());
  }  // pqueue::pop()

  //! max_size() pops elements off the priority queue until its size
  //! is no greater than n
  //
  void max_size(size_t n) {
    while (size() > n)
      pop();
  }  // pqueue::max_size()

  //! clear() removes all of the elements from a priority queue
  //
  void clear() {
    i_kpip.clear();
    k_pi.clear();
  }  // pqueue::clear()


  //! consistent() does some very simple error checking on the priority queue.
  //
  bool consistent() {
    if (i_kpip.size() != k_pi.size()) {
      std::cerr << "## pqueue.h Error: i_kpip.size() = " << i_kpip.size() 
		<< " !=  k_pi.size() = " << k_pi.size() << std::endl;
      return false;
    }
    for (size_t i = 0; i < i_kpip.size(); ++i) {
      if (i != i_kpip[i]->second.index) {
	std::cerr << "## pqueue.h Error: i = " << i 
		  << " !=  i_kpip[i]->second.index = " << i_kpip[i]->second.index << std::endl;
	return false;
      }
      if (i > 0 && PriorityCompareType()(priority(i), priority(parent(i)))) {
	std::cerr << "## pqueue.h Error: priority of item " << i 
		  << " is lower than priority of item " << parent(i) << std::endl;
	return false;
      }
    }
    return true;
  }  // pqueue::consistent()

};  // pqueue{}

#endif // PQUEUE_H
