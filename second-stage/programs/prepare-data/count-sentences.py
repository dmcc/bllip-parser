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

