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

import SWIGParser
import fileinput

if __name__ == "__main__":
    from test import initialize, display_parses
    initialize(n=50)
    for line in fileinput.input():
        line = line.strip()

        print line
        tree = SWIGParser.inputTreeFromString('(S1 ' + line + ')')
        print tree
        sentence = tree.toSentRep()
        print sentence
        parses = SWIGParser.parse(sentence)
        print len(parses), 'parses'
        if not parses:
            raise 'failed'
        display_parses(parses)
        print 'example failure tree', sentence.makeFailureTree('Xyz')
        print
