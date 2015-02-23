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

"""Example of how to parse with arbitrary constraints. The consistent()
function can be replaced with any other predicate. Note that depending on
the strictness of your constraints, you may need to expand the n-best
list first before you find a tree that matches.

Also note that the parser natively supports span constraints via the
parse_constrained() method so this code is purely for demonstrating how
more complicated constraints could be enforced in Python."""

def parse_constrained(rrp, sentence, constraints):
    """Example of how to find the highest scoring parse in the n-best list
    match certain span constraints. constraints is a mapping from spans
    (start, end) to a list of allowed labels for that span.  Returns None
    if no parses match."""
    def consistent(tree, constraints):
        mapping = {}
        for subtree in tree.all_subtrees():
            mapping[subtree.span()] = subtree.label
        for span, allowed_labels in constraints.items():
            if mapping.get(span) not in allowed_labels:
                return False
        return True

    nbest_list = rrp.parse(sentence)
    for item in nbest_list:
        if consistent(item.ptb_parse, constraints):
            return item.ptb_parse
    else:
        return None

if __name__ == "__main__":
    # this needs to be run from the root of the repository since it has
    # a relative path to the parsing model

    from bllipparser import RerankingParser
    rrp = RerankingParser()
    rrp.load_parser_model('first-stage/DATA/EN')

    # the constraint means: there must be a VP from [1,5)
    # (i.e., left ... Falklands)
    # this encourages the parser to pick "left" as the main verb
    constraints = {(1, 5): ['VP']}
    print parse_constrained(rrp, 'British left waffles on Falklands .'.split(),
                            constraints)
    # if we parse without constraints, we get that the main verb is "waffles"
    print parse_constrained(rrp, 'British left waffles on Falklands .'.split(),
                            {})
