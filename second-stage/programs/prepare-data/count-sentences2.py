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

