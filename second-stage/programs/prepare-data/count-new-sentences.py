#! /usr/bin/env python
#
# Writes out the number of sentences in a Collin's parse file

import os, string, sys
                  
def count_parsed(parsed_in):
    old_parsed_id = ''
    lineno = 0
    nsentences = 0
    old_parsed_ident = -1
    while 1:
        parsed_line = parsed_in.readline()
        lineno = lineno+1
        if len(parsed_line) == 0:
            nsentences = nsentences+1
            break
        parsed_fields = string.split(parsed_line, None, 3)
        if old_parsed_id != parsed_fields[1]:
            if old_parsed_id != '':
                nsentences = nsentences+1
            old_parsed_id = parsed_fields[1]
            try:
                parsed_ident = int(old_parsed_id)
            except ValueError:
                print "%d: %s" % (lineno, parsed_line)
            if parsed_ident != old_parsed_ident+1:
                print "%d: parsed_ident = %d, old_parsed_ident = %d" % (lineno, parsed_ident, old_parsed_ident)
            old_parsed_ident = parsed_ident
    return nsentences

if __name__ == '__main__':
    print "The file contains %d sentences" % count_parsed(sys.stdin)

