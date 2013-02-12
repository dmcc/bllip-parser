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

// custom_allocator.h
//
// Mark Johnson, 23rd April 2005, modified 6th November 2005

#ifndef CUSTOM_ALLOCATOR_H
#define CUSTOM_ALLOCATOR_H

#if ((__GNUC__ == 3 && __GNUC_MINOR__ == 4 && __GNUC_PATCHLEVEL__ == 3) || (__GNUC__ == 4 && __GNUC_MINOR__ == 0 && __GNUC_PATCHLEVEL__ >= 0))
// stop c++allocator.h from being loaded

#define _CXX_ALLOCATOR_H 1

// load the mt_allocator
#include <ext/mt_allocator.h>

#define ___glibcxx_base_allocator  __gnu_cxx::__mt_alloc
// #define ___glibcxx_base_allocator  __gnu_cxx::__pool_alloc
#endif  // g++ 3.4.3 or g++ 4.0.0

#endif  // CUSTOM_ALLOCATOR_H
