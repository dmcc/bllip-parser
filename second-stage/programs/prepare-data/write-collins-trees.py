#! /usr/bin/env python -O
#
# Writes out a file with format
#
# <N_GOLD_CONSTITUENTS> <N_TREES>
# <PREC_REC> <LOG_PROB> <COND_PROB> <TREE>

import os, string

train_trees_source = 'zcat ../rerankdata/data/parsed.gz | ../rerankdata/scripts/proc_pout.prl'
train_stats_source = 'zcat ../rerankdata/data/scored.gz'
train_out = 'train.dat'

dev_trees_source = 'zcat ../rerankdata/data/devparsed.gz | ../rerankdata/scripts/proc_pout.prl'
dev_stats_source = 'zcat ../rerankdata/data/devscored.gz'
dev_out = 'dev.dat'

class Tree:
    def __init__(self, tree0, id0, logprob0, score1, normscore1, logprob1, condprob1, best1):
        self.tree = tree0
        self.id = int(id0)
        self.logprob = float(logprob0)
        self.precrec = float(normscore1)
        self.condprob = float(condprob1)
        if self.precrec > 0:
            self.n_gold_constituents = float(score1)/self.precrec
        else:
            self.n_gold_constituents = -1
    def __cmp__(t1,t2):
        return cmp(t2.precrec,t1.precrec) or cmp(t2.logprob,t1.logprob)
    
def write_trees(out, trees):
    assert len(trees)>0
    if len(trees) > 1:
        trees.sort()
        out.write("%d %d\n" % (trees[0].n_gold_constituents,len(trees)))
        for tree in trees:
            out.write("%g %g %g %s" % (tree.precrec,tree.logprob,tree.condprob,tree.tree))
        out.flush()

                  
def transform_collins(trees_in_command, stats_in_command, out_file):
    trees_in = os.popen(trees_in_command, 'r')
    stats_in = os.popen(stats_in_command, 'r')
    out = open(out_file, 'w')
    old_parsed_id = ''
    old_scored_id = ''
    parse_treestats = {}
    lineno = 0
    nsentences = 0
    while 1:
        tree_line = trees_in.readline()
        stats_line = stats_in.readline()
        lineno = lineno+1
        if len(tree_line) == 0:
            nsentences = nsentences+1
            write_trees(out, parse_treestats.values())
            assert len(stats_line) == 0, "lineno = %d, empty tree_line, stats_line = %s" % (lineno, stats_line)
            break
        assert len(stats_line) != 0, "lineno = %d, empty stats_line, tree_line = %s" % (lineno, tree_line)
        tree_fields = string.split(tree_line, None, 8)
        stats_fields = string.split(stats_line, None, 7)
        assert tree_fields[0] == 'ID', "lineno = %d, tree_line = %s" % (lineno, tree_line)
        assert tree_fields[2] == 'PROB', "lineno = %d, tree_line = %s" % (lineno, tree_line)
        assert tree_fields[4] == 'RANK', "lineno = %d, tree_line = %s" % (lineno, tree_line)
        assert tree_fields[6] == 'EDGE', "lineno = %d, tree_line = %s" % (lineno, tree_line)
        if old_parsed_id != tree_fields[1] or old_scored_id != stats_fields[0]:
            assert old_parsed_id != tree_fields[1] and old_scored_id != stats_fields[0], "lineno = %d, old_parsed_id = %s, old_stats_id = %s, tree_line = %s, stats_line = %s" % (lineno,old_parsed_id,old_stats_id,tree_line,stats_line)
            if old_parsed_id != '':
                nsentences = nsentences+1
                write_trees(out, parse_treestats.values())
                parse_treestats.clear()
            old_parsed_id = tree_fields[1]
            old_scored_id = stats_fields[0]
        parse = tree_fields[8]
        logprob = float(tree_fields[3])
        if (not parse_treestats.has_key(parse)) or parse_treestats[parse].logprob < logprob:
            parse_treestats[parse] = Tree(parse, tree_fields[1], logprob,
                                          stats_fields[2], stats_fields[3], stats_fields[4], stats_fields[6], stats_fields[7])
    return nsentences


if __name__ == '__main__':
    ns = transform_collins(train_trees_source, train_stats_source, train_out)
    print "There are %d sentences in the training data" % ns
    ns = transform_collins(dev_trees_source, dev_stats_source, dev_out)
    print "There are %d sentences in the development data" % ns
