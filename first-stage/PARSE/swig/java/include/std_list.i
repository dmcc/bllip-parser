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
 * This was originally swig/1.3.40/java/std_vector.i from http://www.swig.org
 * It has been adapted to work with lists instead of vectors.  The interface is incomplete and (unfortunately) does not provide iteration.
 */

%include <std_common.i>

%{
#include <list>
#include <stdexcept>
%}

namespace std {
    
    template<class T> class list {
      public:
        typedef size_t size_type;
        typedef T value_type;
        typedef const value_type& const_reference;
        list();
        size_type size() const;
        %rename(isEmpty) empty;
        bool empty() const;
        void clear();

        %rename(add) push_back;
        void push_back(const value_type& x);
        %rename(addFirst) push_front;
        void push_front(const value_type& x);

        %rename(getFirst) front;
        const_reference front() const;
        %rename(getLast) back;
        const_reference back() const;

        /* this is uses different terminology from Java's Deque interface
           since they don't return the item */
        %rename(removeFirst) pop_front;
        void pop_front();
        %rename(removeLast) pop_back;
        void pop_back();
    };

    // bool specialization
    template<> class list<bool> {
      public:
        typedef size_t size_type;
        typedef bool value_type;
        typedef bool const_reference;
        list();
        size_type size() const;
        %rename(isEmpty) empty;
        bool empty() const;
        void clear();

        %rename(add) push_back;
        void push_back(const value_type& x);
        %rename(addFirst) push_front;
        void push_front(const value_type& x);

        %rename(getFirst) front;
        const_reference front() const;
        %rename(getLast) back;
        const_reference back() const;

        /* this is uses different terminology from Java's Deque interface
           since they don't return the item */
        %rename(removeFirst) pop_front;
        void pop_front();
        %rename(removeLast) pop_front;
        void pop_back();
    };
}

%define specialize_std_list(T)
#warning "specialize_std_list - specialization for type T no longer needed"
%enddef

