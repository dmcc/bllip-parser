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

if __name__ == "__main__":
    # needs to be run from the root of the repository for the parser
    # model path below to work

    from bllipparser import RerankingParser, Tree

    rrp = RerankingParser()
    rrp.load_parser_model('first-stage/DATA/EN', heads_only=True)

    tree1 = Tree('''(S1 (SQ (VBZ Swears) (NP (PRP she)) (VP (VBD
    recognized) (NP (PRP$ his) (NN voice)) (, ,) (SBAR (IN that) (S
    (NP (NNP Tim)) (VP (VBD fired)))) (, ,) ('' ') (S (S (NP (PRP It))
    (VP (VBZ 's) (NP (PRP$ my) (NN money)))) (CC and) (S (NP (PRP I))
    (VP (VBP want) (S (NP (PRP it)) (VP (POS '))))))) (. !)))''')

    head = tree1.head()
    print 'head word of sentence:', head.token
    print 'head tree of sentence:', head
    print

    # print all syntactic dependencies
    for goveror, dependent in tree1.dependencies():
        print 'dependency: %s -> %s' % (goveror.token, dependent.token)
    print

    # demo of how to lexicalize a tree by adding the headword to the
    # label of the tree
    for subtree in tree1.all_subtrees():
        subtree.label += '-' + subtree.head().token
    print 'lexicalized tree:'
    print tree1.pretty_string()
