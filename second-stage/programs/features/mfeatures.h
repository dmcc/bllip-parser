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

// mfeatures.h
//
// Mark Johnson, 28th Jan 2010
//
// This is a common header file for extract-spmfeatures.cc and
// best-mparses.cc.  Its job is to include the header file that
// actually defines the real features.  If you decide to define
// a new feature set, redefine the include statement in this file
// to include that file.

#pragma once

// change this if you define a new feature file
//
#include "spmfeatures.h"
