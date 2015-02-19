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

def tree_diff(tree1, tree2, show_all=False):
    def labeled_span(subtree):
        span = subtree.span()
        return (span[0], span[1], str(subtree.label))
    tree1_spans = set(labeled_span(subtree)
        for subtree in tree1.all_subtrees())
    tree2_spans = set(labeled_span(subtree)
        for subtree in tree2.all_subtrees())

    tokens = tree1.tokens()
    assert tokens == tree2.tokens()

    for tree_span in sorted(tree1_spans | tree2_spans):
        words = ' '.join(tokens[tree_span[0]:tree_span[1]])
        marker = ' '
        if tree_span not in tree2_spans:
            marker = '-'
        elif tree_span not in tree1_spans:
            marker = '+'
        elif not show_all:
            continue
        print marker, tree_span, words

if __name__ == "__main__":
    from bllipparser import Tree

    tree1 = Tree('''(S1 (SQ (VBZ Swears) (NP (PRP she)) (VP (VBD
    recognized) (NP (PRP$ his) (NN voice)) (, ,) (SBAR (IN that) (S
    (NP (NNP Tim)) (VP (VBD fired)))) (, ,) ('' ') (S (S (NP (PRP It))
    (VP (VBZ 's) (NP (PRP$ my) (NN money)))) (CC and) (S (NP (PRP I))
    (VP (VBP want) (S (NP (PRP it)) (VP (POS '))))))) (. !)))''')

    tree2 = Tree('''(S1 (S (VP (VBZ Swears) (SBAR (SBAR (S (NP-SBJ
    (PRP she)) (VP (VBD recognized) (NP (PRP$ his) (NN voice))))) (, ,)
    (SBAR (IN that) (S (NP-SBJ (NNP Tim)) (VP (VBD fired) (, ,) ('' ')
    (S (S (NP-SBJ (PRP It)) (VP (VBZ 's) (NP-PRD (PRP$ my) (NN money))))
    (CC and) (S (NP-SBJ (PRP I)) (VP (VBP want) (NP (PRP it)))))
    ('' ')))))) (. !)))''')

    tree_diff(tree1, tree2, show_all=True)
