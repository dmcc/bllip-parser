#! /usr/bin/env python
#
# Writes out the number of sentences in Collin's new allparses.gz file

import os, string, sys
                  
def count_ids(ids_in):
    old_id = ''
    lineno = 0
    nsentences = 0
    while 1:
        line = ids_in.readline()
        lineno = lineno+1
        if len(line) == 0:
            nsentences = nsentences+1
            break
        fields = string.split(line, None, 3)
        if old_id != fields[1]:
            if old_id != '':
            	nsentences = nsentences+1
            old_id = fields[1]
    return nsentences

if __name__ == '__main__':
    print "The file contains %d sentences" % count_ids(sys.stdin)

