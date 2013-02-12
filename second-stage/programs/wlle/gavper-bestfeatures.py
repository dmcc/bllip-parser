#! /bin/env python
# Licensed under the Apache License, Version 2.0 (the "License"); you may
# not use this file except in compliance with the License.  You may obtain
# a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
# License for the specific language governing permissions and limitations
# under the License.

usage = """%prog -- extract best scoring combinations of features
from gavper output

Version of 20th July, 2008

(c) Mark Johnson

usage: %prog [options]

"""

import optparse, re, sys

features_rex = re.compile(r"\n# Regularization classes: \((.*)\)\n")
results_rex = re.compile(r"\n(\d+)\s+([0-9.]+)\s+([0-9.]+)\s+\(([0-9. ]+)\)")

def score12(s1, s2):
    return (s1 + s2)/2.0

def score1(s1, s2):
    return s1

def score2(s1, s2):
    return s2

def bestfeatures(scorefn, features, results):
    best_result = None
    best_score = -1
    for result in results:
        s = scorefn(result[0], result[1])
        if s > best_score:
            best_score = s
            best_result = result
    print "best score =", best_score
    nfeatures = 0
    print "features =",
    for w,f in zip(best_result[2], features):
        if w != 0:
            nfeatures += 1
            if w == 1:
                print f,
            else:
                print "(%s,%s)"%(f,w),
    print
    print "nfeatures =", nfeatures
    
if __name__ == "__main__":
    txt = sys.stdin.read()
    features_mo = features_rex.search(txt)
    if features_mo == None:
        sys.stderr.write("Can't find features list")
        sys.exit(1)
    features = list(features_mo.group(1).split())
    results = [[float(result_mo.group(2)), float(result_mo.group(3)), [float(w) for w in result_mo.group(4).split()]]
               for result_mo in results_rex.finditer(txt)]
    print
    print "Dev scores,",
    bestfeatures(score1, features, results)
    print
    print "Test1 scores,",
    bestfeatures(score2, features, results)
    print
    print "Average Dev and Test1 scores,",
    bestfeatures(score12, features, results)
    print
