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

/* read-tree.l
 *
 * (c) Mark Johnson, 7th March 2004, modified 21st October 2004 to read null category label
 */

%option noyywrap
%option fast
%option prefix="readtree"

%{
#include "sym.h"
#include "tree.h"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <stack>

int readtree_lineno = 1;
const char* readtree_filename = NULL;

static const symbol empty_symbol("");

inline static void message(const char* s1, const char* s2) {
  fprintf(stderr, "%s:%d: %s %s\n", readtree_filename, readtree_lineno, s1, s2);
}

inline static void downcase(char *s) {
  assert(s != NULL);
  while (*s != '\0') {
    if (isupper(*s))
      *s = tolower(*s);
    ++s;
  }
}

#define YY_DECL static tree* readtree_lex(bool downcase_flag = false)

%}

/* RT  - PTB-style tree root with no label
 * RTC - tree root (with label)
 * FC  - first child subtree or terminal
 * NC  - next child
 * CAT - category from label
 * PC -  post-category junk in label
 * IND - index in label
 */

%s RT
%s RTC
%s FC
%s NC
%s CAT
%x PC
%s IND

%%

%{
  tree* root = NULL;    /* tree's root node */
  std::stack<tree*> s;  /* stack of tree node ptrs */
%}

<RT>"("                 assert(s.empty()); s.push(root = new tree); s.top()->label.cat = tree::label_type::root(); BEGIN(FC); 
<RT><<EOF>>		assert(s.empty()); return NULL;

<RTC>"("		assert(s.empty()); s.push(root = new tree); BEGIN(CAT);
<RTC><<EOF>>		assert(s.empty()); return NULL;

<FC>"("			assert(!s.empty()); s.push(s.top()->child = new tree); BEGIN(CAT); 
<FC>[^ \t\n()]+		{ assert(!s.empty()); s.push(s.top()->child = new tree); 
                          if (downcase_flag)
			   downcase(readtreetext);
			  s.top()->label.cat = symbol(readtreetext); 
			  BEGIN(NC);
                        }
<FC>")"			assert(!s.empty()); s.pop(); if (s.size() == 1) return root; BEGIN(NC);

<NC>")"			assert(!s.empty()); s.pop(); if (s.size() == 1) return root;  
<NC>"("			assert(!s.empty()); s.top() = s.top()->next = new tree; BEGIN(CAT); 

<CAT>"-NONE-"           { assert(!s.empty());  
                          s.top()->label.cat = symbol(readtreetext);
			  s.push(s.top()->child = new tree);
                         }
<CAT>[A-Z0-9$?*]+("."[^ \t\n()]+)*  assert(!s.empty()); s.top()->label.cat = symbol(readtreetext); BEGIN(PC); 
<CAT>[^A-Z0-9 \n\t()$*][^ \n\t()]*  assert(!s.empty()); s.top()->label.cat = symbol(readtreetext); BEGIN(PC);

<PC>"-"[0-9]+		/* ignore index tag */
<PC>"-"[A-Z]+		/* ignore GF tag */
<PC>([=|+-])([^ \t\n()-])+	/* ignore other tags */
<PC>.			unput(readtreetext[0]); BEGIN(FC);

[ \t]+			/* ignore spaces */
"\n"			++readtree_lineno;   /* increment line count */
.			message("Unexpected character", readtreetext); std::cerr << "Parse tree so far: " << root << '\n'; abort();

%%

tree* readtree_root(FILE* fp, bool downcase_flag)
{
  readtreein = fp;
  BEGIN(RT);
  return readtree_lex(downcase_flag);
}

tree* readtree(FILE* fp, bool downcase_flag)
{
  readtreein = fp;
  BEGIN(RTC);
  return readtree_lex(downcase_flag);
}

tree* readtree_root(const char* str, bool downcase_flag)
{
  readtree_lineno = 1;
  readtree_filename = str;
  YY_BUFFER_STATE buf = readtree_scan_string(str);
  BEGIN(RT);
  tree* t = readtree_lex(downcase_flag);
  readtree_delete_buffer(buf);
  readtree_filename = NULL;
  return t;
}

tree* readtree(const char* str, bool downcase_flag)
{
  readtree_lineno = 1;
  readtree_filename = str;
  YY_BUFFER_STATE buf = readtree_scan_string(str);
  BEGIN(RTC);
  tree* t = readtree_lex(downcase_flag);
  readtree_delete_buffer(buf);
  readtree_filename = NULL;
  return t;
}
