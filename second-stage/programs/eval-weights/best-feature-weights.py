"""best-feature-weights.py

reads the output of the feature weight cross-validation programs
and prints out the best features and their weights."""

# 0.911719 0.917315 ['NLogP', 'Rule:0:0:0:0:0:0:0:1', 'Rule:0:1:0:0:0:0:0:1', 'Rule:0:0:1:0:0:0:0:1', 'Rule:0:0:0:1:0:0:0:1', 'Rule:0:0:0:0:2:0:0:1', 'Rule:0:0:0:0:0:2:0:1', 'Rule:0:0:0:0:2:2:0:1', 'Rule:0:1:1:0:0:0:0:1', 'Rule:0:1:0:1:0:0:0:1', 'Rule:0:1:0:0:2:0:0:1', 'Rule:0:1:0:0:0:2:0:1', 'Rule:0:1:0:0:2:2:0:1', 'Rule:1:0:1:0:0:0:0:1', 'Rule:1:0:0:1:0:0:0:1', 'Rule:1:0:0:0:2:0:0:1', 'Rule:1:0:0:0:0:2:0:1', 'Rule:1:0:0:0:2:2:0:1', 'NGram:1:0:0:1:0:0:0:1', 'NGram:2:0:0:1:0:0:0:1', 'NGram:3:0:0:1:0:0:0:1', 'NGram:1:0:0:0:1:0:0:1', 'NGram:2:0:0:0:1:0:0:1', 'NGram:3:0:0:0:1:0:0:1', 'NGram:2:0:0:0:0:1:0:1', 'NGram:3:0:0:0:0:1:0:1', 'NGram:1:0:0:0:1:1:0:1', 'NGram:2:0:0:0:1:1:0:1', 'NGram:3:0:0:0:1:1:0:1', 'NGram:1:0:0:0:2:0:0:1', 'NGram:2:0:0:0:2:0:0:1', 'NGram:3:0:0:0:2:0:0:1', 'NGram:2:0:0:0:0:2:0:1', 'NGram:3:0:0:0:0:2:0:1', 'NGram:1:0:0:0:2:2:0:1', 'NGram:2:0:0:0:2:2:0:1', 'NGram:3:0:0:0:2:2:0:1', 'NGram:1:0:0:0:2:1:0:1', 'NGram:2:0:0:0:2:1:0:1', 'NGram:3:0:0:0:2:1:0:1', 'NGram:1:0:0:0:2:2:1:1', 'NGram:2:0:0:0:2:2:1:1', 'NGram:1:0:0:0:2:2:2:1', 'NGram:2:0:0:0:2:2:2:1', 'NGram:1:0:0:1:0:0:0:0', 'NGram:2:0:0:1:0:0:0:0', 'NGram:3:0:0:1:0:0:0:0', 'NGram:1:0:0:0:1:0:0:0', 'NGram:2:0:0:0:1:0:0:0', 'NGram:3:0:0:0:1:0:0:0', 'NGram:2:0:0:0:0:1:0:0', 'NGram:3:0:0:0:0:1:0:0', 'NGram:1:0:0:0:1:1:0:0', 'NGram:2:0:0:0:1:1:0:0', 'NGram:3:0:0:0:1:1:0:0', 'NGram:1:0:0:0:2:0:0:0', 'NGram:2:0:0:0:2:0:0:0', 'NGram:3:0:0:0:2:0:0:0', 'NGram:2:0:0:0:0:2:0:0', 'NGram:3:0:0:0:0:2:0:0', 'NGram:1:0:0:0:2:2:0:0', 'NGram:2:0:0:0:2:2:0:0', 'NGram:3:0:0:0:2:2:0:0', 'NGram:1:0:0:0:2:1:0:0', 'NGram:2:0:0:0:2:1:0:0', 'NGram:3:0:0:0:2:1:0:0', 'NGram:1:0:0:0:2:2:1:0', 'NGram:2:0:0:0:2:2:1:0', 'NGram:1:0:0:0:2:2:2:0', 'NGram:2:0:0:0:2:2:2:0', 'Word:1', 'Word:2', 'WProj:0:0:1', 'Word:3', 'WProj:1:0:1', 'WProj:0:1:1', 'WProj:1:1:1', 'WProj:0:0:2', 'WProj:1:0:2', 'RightBranch', 'Heads:2:0:0:0', 'Heads:2:0:0:1', 'Heads:2:1:1:0', 'Heads:2:1:1:1', 'Heads:3:0:0:0', 'Heads:3:0:0:1', 'Heads:3:1:1:0', 'Heads:3:1:1:1', 'Heavy', 'NGramTree:2:0:1:0', 'NGramTree:2:1:1:0', 'NGramTree:2:3:1:0', 'NGramTree:3:0:1:0', 'NGramTree:3:1:1:0', 'NGramTree:3:2:1:0', 'NGramTree:2:0:0:0', 'NGramTree:2:2:0:0', 'NGramTree:2:3:0:0', 'NGramTree:3:0:0:0', 'NGramTree:3:2:0:0', 'HeadTree:0:0:0:0', 'HeadTree:0:0:0:1', 'HeadTree:1:0:0:0', 'HeadTree:1:0:0:1', 'HeadTree:1:1:0:0', 'HeadTree:1:1:0:1', 'HeadTree:0:0:1:0', 'HeadTree:0:0:1:1', 'HeadTree:1:0:1:0', 'SubjVerbAgr', 'SynSemHeads:0', 'SynSemHeads:1', 'SynSemHeads:2', 'CoPar:0', 'CoPar:1', 'CoLenPar', 'Edges:0:0:0:0:0', 'Edges:0:0:0:0:1', 'Edges:0:0:0:0:2', 'Edges:0:0:0:1:0', 'Edges:0:0:0:1:1', 'Edges:0:0:0:2:0', 'Edges:0:0:1:0:0', 'Edges:0:0:1:0:1', 'Edges:0:0:1:1:0', 'Edges:0:0:2:0:0', 'Edges:0:1:0:0:0', 'Edges:0:1:0:0:1', 'Edges:0:1:0:1:0', 'Edges:0:1:1:0:0', 'Edges:0:2:0:0:0', 'Edges:1:0:0:0:0', 'Edges:1:0:0:0:1', 'Edges:1:0:0:0:2', 'Edges:1:0:0:1:0', 'Edges:1:0:0:1:1', 'Edges:1:0:0:2:0', 'Edges:1:0:1:0:0', 'Edges:1:0:1:0:1', 'Edges:1:0:1:1:0', 'Edges:1:0:2:0:0', 'Edges:1:1:0:0:0', 'Edges:1:1:0:0:1', 'Edges:1:1:0:1:0', 'Edges:1:1:1:0:0', 'Edges:1:2:0:0:0', 'WordEdges:0:0:0:0:0', 'WordEdges:0:0:0:0:1', 'WordEdges:0:0:0:0:2', 'WordEdges:0:0:0:1:0', 'WordEdges:0:0:0:1:1', 'WordEdges:0:0:0:2:0', 'WordEdges:0:0:1:0:0', 'WordEdges:0:0:1:0:1', 'WordEdges:0:0:1:1:0', 'WordEdges:0:0:2:0:0', 'WordEdges:0:1:0:0:0', 'WordEdges:0:1:0:0:1', 'WordEdges:0:1:0:1:0', 'WordEdges:0:1:1:0:0', 'WordEdges:0:2:0:0:0', 'WordEdges:1:0:0:0:0', 'WordEdges:1:0:0:0:1', 'WordEdges:1:0:0:0:2', 'WordEdges:1:0:0:1:0', 'WordEdges:1:0:0:1:1', 'WordEdges:1:0:0:2:0', 'WordEdges:1:0:1:0:0', 'WordEdges:1:0:1:0:1', 'WordEdges:1:0:1:1:0', 'WordEdges:1:0:2:0:0', 'WordEdges:1:1:0:0:0', 'WordEdges:1:1:0:0:1', 'WordEdges:1:1:0:1:0', 'WordEdges:1:1:1:0:0', 'WordEdges:1:2:0:0:0']

import re, sys

fnames_re = re.compile(r"Regularization classes: \(([^\)]+)\)")
result_re = re.compile(r"([0-9]+)\s+([\.0-9]+)\s+([\.0-9]+)\s+\(([^\)]+)\)")

def score(score1, score2):
    """combines the scores for the two cross-validation sets
    into a single score"""
    return score1*score1 + score2*score2


def read_data(inf):
    fnames = []
    results = []
    for line in inf:
        fnames_mo = fnames_re.search(line)
        if fnames_mo:
            fnames = fnames_mo.group(1).split()
        result_mo = result_re.search(line)
        if result_mo:
            score1 = float(result_mo.group(2))
            score2 = float(result_mo.group(3))
            score12 = score(score1, score2)
            weights = [int(w) for w in result_mo.group(4).split()]
            results.append((score12,score1,score2,weights))
    results.sort(reverse=True)
    return (fnames,results)

def select_fnames(fnames0, weights):
    fnames = []
    for (fname,weight) in zip(fnames0,weights):
        if weights != 0:
            fnames.append(fname)
    return fnames
    

if __name__ == "__main__":
    if len(sys.argv) == 1:
        (feats,results) = read_data(sys.stdin)
    else:
        (feats,results) = read_data(file(sys.argv[1], "rU"))
    # print feats
    result = results[0]
    print "// dev1 score = %s, dev2 score = %s" % (result[1], result[2])
    for (f,w) in zip(feats, result[3]):
        if w == 0:
            print "//\t\t- %s" % f
        else:
            print "//\t+ %s" % f

    

    
