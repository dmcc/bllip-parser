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

"""Miscellaneous tree functions obscure enough to not put into the Tree
class."""

def replace_leaves(source_tree, target_tree):
    """Given two Tree objects, replace all the leaves in source_tree
    with those in the target_tree Tree (admittedly a strange thing to do,
    but this does have uses)."""
    leaves = iter(target_tree.tokens())
    for subtree in source_tree.all_subtrees():
        if subtree.is_preterminal():
            subtree.token = leaves.next()

def is_prepreterminal(node):
    """Returns True iff all children of this node are preterminals."""
    subtrees = node.subtrees()
    return len(subtrees) > 0 and \
        all(subtree.is_preterminal() for subtree in subtrees)

def labeled_spans(tree):
    """Yields all labeled span in the tree: (start, end, label)."""
    for subtree in tree.all_subtrees():
        start, end = subtree.span()
        yield (start, end, subtree.label)

def strip_function_tags(tree):
    """Removes all function tags from the tree."""
    for subtree in tree.all_subtrees():
        subtree.label_suffix = ''

if __name__ == "__main__":
    from bllipparser import Tree

    tree = Tree('''(S1 (S (VP (VBZ Swears) (SBAR (SBAR (S (NP-SBJ
    (PRP she)) (VP (VBD recognized) (NP (PRP$ his) (NN voice))))) (, ,)
    (SBAR (IN that) (S (NP-SBJ (NNP Tim)) (VP (VBD fired) (, ,) ('' ')
    (S (S (NP-SBJ (PRP It)) (VP (VBZ 's) (NP-PRD (PRP$ my) (NN money))))
    (CC and) (S (NP-SBJ (PRP I)) (VP (VBP want) (NP (PRP it)))))
    ('' ')))))) (. !)))''')

    for labeled_span in labeled_spans(tree):
        print labeled_span

    print '---'
    for subtree in tree.all_subtrees():
        print is_prepreterminal(subtree), subtree
    print '---'
    strip_function_tags(tree)
    for subtree in tree.all_subtrees():
        print is_prepreterminal(subtree), subtree
