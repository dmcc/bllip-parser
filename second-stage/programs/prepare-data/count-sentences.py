#! /usr/bin/env python
#
# Writes out the number of sentences in a Collin's scored.gz file

import os, string, sys
                  
def count_scored(scored_in):
    old_scored_id = ''
    lineno = 0
    nsentences = 0
    while 1:
        scored_line = scored_in.readline()
        lineno = lineno+1
        if len(scored_line) == 0:
            nsentences = nsentences+1
            break
        scored_fields = string.split(scored_line, None, 2)
        if old_scored_id != scored_fields[0] and old_scored_id != '':
            nsentences = nsentences+1
            old_scored_id = scored_fields[0]
    return nsentences

if __name__ == '__main__':
    print "The file contains %d sentences" % count_scored(sys.stdin)

