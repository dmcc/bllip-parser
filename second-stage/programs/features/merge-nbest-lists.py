#! /usr/bin/env python
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

usage = """%prog Version of 23rd November 2009

(c) Mark Johnson

Merge the n-best outputs of two or more n-best parsers

usage: %prog [options]"""

import gzip, itertools, optparse, re, sys

def readPetrov(inf):
    """A generator that reads the Petrov Berkeley n-best parses for a sentence"""
    probtrees = []
    for line in inf:
        if line == "\n":
            yield probtrees
            probtrees = []
        else:
            splits = line.split(None, 1)
            prob = splits[0]
            if prob == "-Infinity":
                continue
            # prob = float(prob)
            tree = splits[1].rstrip()
            assert(tree[-2:] == ' )')
            tree = tree[:-2]+')'
            probtrees.append((prob,tree))

parseid = None

def readCharniak1(inf):
    """Reads the Charniak Brown n-best parses for the next sentence"""
    global parseid
    line = inf.readline()
    if line == "":
        return None
    splits = line.split()
    nparses = int(splits[0])
    parseid = splits[1]
    probtrees = []
    for iparse in xrange(nparses):
        splits = inf.readline().split(None, 1)
        prob = splits[0]
        # prob = float(prob)
        tree = inf.readline().rstrip()
        assert(tree[:4] == '(S1 ')
        tree = '( '+tree[4:]
        probtrees.append((prob,tree))
    line = inf.readline()
    assert(line == '\n')
    return probtrees

def readCharniak(inf):
    """A generator that reads the Charniak Brown n-best parses for a sentence"""
    parsetrees = readCharniak1(inf)
    while parsetrees:
        yield parsetrees
        parsetrees = readCharniak1(inf)

tree_terminals_rex = re.compile(r"[(][^ \t\n\r\f\v()]+ +([^ \t\n\r\f\v()]+)[)]")

def tree_terminals(tree):
    """returns the terminals of tree"""
    return ' '.join((mo.group(1) for mo in tree_terminals_rex.finditer(tree)))

def readnbest(generators):
    """a generator returning the union of the n-best trees produced by generators"""
    ngenerators = len(generators)
    for nbests in itertools.izip_longest(*generators):
        terminals = None
        tree_probs = {}
        for i, probtrees in enumerate(nbests):
            assert(probtrees != None)
            for prob,tree in probtrees:
                terminals0 = tree_terminals(tree)
                if terminals:
                    assert(terminals == terminals0)
                else:
                    terminals = terminals0
                if tree in tree_probs:
                    tree_probs[tree][i] = prob
                else:
                    # probs = ngenerators*[0]
                    probs = ngenerators*['0']
                    probs[i] = prob
                    tree_probs[tree] = probs
        yield parseid,tree_probs

if __name__ == '__main__':
    parser = optparse.OptionParser(usage=usage)

    parser.add_option("-p", "--Petrov-gzip", dest="Petrov_gzip", type="str", action="append", default=[],
                      help="gzipped file to read Berkeley-format n-best parses from")
    parser.add_option("-c", "--Charniak-gzip", dest="Charniak_gzip", type="str", action="append", default=[],
                      help="gzipped file to read Brown-format n-best parses from")
     
    (options,args) = parser.parse_args()

    Petrovgens = [readPetrov(gzip.open(fn,'rb')) for fn in options.Petrov_gzip]
    Charniakgens = [readCharniak(gzip.open(fn,'rb')) for fn in options.Charniak_gzip]
    
    for parseid, tree_probs in readnbest(Charniakgens+Petrovgens):
        print "%s\t%s"%(len(tree_probs), parseid)
        for tree,probs in tree_probs.iteritems():
            print ' '.join(probs)
            print tree
        print



        
