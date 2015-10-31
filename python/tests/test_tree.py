# TODO: untested:
#       Tree.sd_tokens

import unittest
import bllipparser

sample_tree = '(S1 (S (NP (DT This)) (VP (VBZ is) (NP (DT a) (ADJP ' \
              '(RB fairly) (JJ simple)) (NN parse) (NN tree))) (. .)))'
sample_tree_pretty = '''(S1 (S (NP (DT This))
     (VP (VBZ is)
      (NP (DT a) (ADJP (RB fairly) (JJ simple)) (NN parse) (NN tree)))
     (. .)))'''

class TreeTests(unittest.TestCase):
    def test_tree_basics(self):
        tree = bllipparser.Tree(sample_tree)
        assert str(tree) == sample_tree
        assert tree.pretty_string() == sample_tree_pretty
        assert tree.tokens() == ('This', 'is', 'a', 'fairly', 'simple',
                                 'parse', 'tree', '.')
        assert tree.tags() == ('DT', 'VBZ', 'DT', 'RB', 'JJ', 'NN',
                               'NN', '.')
        assert tree.tokens_and_tags() == \
            [('This', 'DT'), ('is', 'VBZ'), ('a', 'DT'), ('fairly', 'RB'),
             ('simple', 'JJ'), ('parse', 'NN'), ('tree', 'NN'), ('.', '.')]
        assert tree.span() == (0, 8)
        assert tree.label == 'S1'

        subtrees = tree.subtrees()
        assert len(subtrees) == 1
        assert str(subtrees[0]) == '(S (NP (DT This)) (VP (VBZ is) (NP ' \
                                   '(DT a) (ADJP (RB fairly) (JJ simple)) ' \
                                   '(NN parse) (NN tree))) (. .))'
        assert subtrees[0].label == 'S'
        assert str(subtrees[0][0]) == '(NP (DT This))'
        assert subtrees[0][0].label == 'NP'
        assert subtrees[0][0].span() == (0, 1)
        assert subtrees[0][0].tags() == ('DT',)
        assert subtrees[0][0].tokens() == ('This',)
        assert str(subtrees[0][0][0]) == '(DT This)'
        assert subtrees[0][0][0].token == 'This'
        assert subtrees[0][0][0].label == 'DT'
        assert tree[0][0][0].is_preterminal()
        assert len(tree[0]) == 3

        subtrees = iter(tree[0])
        assert str(next(subtrees)) == '(NP (DT This))'
        assert str(next(subtrees)) == '(VP (VBZ is) (NP (DT a) (ADJP ' \
                                      '(RB fairly) (JJ simple)) (NN parse) ' \
                                      '(NN tree)))'
        assert str(next(subtrees)) == '(. .)'

        pairs = [(False, sample_tree),
                 (False, '(S (NP (DT This)) (VP (VBZ is) (NP (DT a) (ADJP '
                         '(RB fairly) (JJ simple)) (NN parse) (NN tree))) '
                         '(. .))'),
                 (False, '(NP (DT This))'),
                 (True, '(DT This)'),
                 (False, '(VP (VBZ is) (NP (DT a) (ADJP (RB fairly) (JJ '
                         'simple)) (NN parse) (NN tree)))'),
                 (True, '(VBZ is)'),
                 (False, '(NP (DT a) (ADJP (RB fairly) (JJ simple)) (NN '
                         'parse) (NN tree))'),
                 (True, '(DT a)'),
                 (False, '(ADJP (RB fairly) (JJ simple))'),
                 (True, '(RB fairly)'),
                 (True, '(JJ simple)'),
                 (True, '(NN parse)'),
                 (True, '(NN tree)'),
                 (True, '(. .)')]
        actual_pairs = [(subtree.is_preterminal(), str(subtree))
                        for subtree in tree.all_subtrees()]
        assert pairs == actual_pairs

    def test_tree_errors(self):
        # test issue #33
        self.assertRaises(RuntimeError, bllipparser.Tree, '(())')
        # make sure we can still load good trees after an error
        bllipparser.Tree(sample_tree)
        bllipparser.Tree(sample_tree)
        self.assertRaises(RuntimeError, bllipparser.Tree, '(BADTOPTAG hi)')
        self.assertRaises(RuntimeError, bllipparser.Tree,
                          'Does not start with a paren')
        self.assertRaises(RuntimeError, bllipparser.Tree, '(S1 eh)')
        self.assertRaises(RuntimeError, bllipparser.Tree, '(S1')
        self.assertRaises(RuntimeError, bllipparser.Tree, '(S1 ((')
        self.assertRaises(RuntimeError, bllipparser.Tree, '(S1 (NP')
        bllipparser.Tree(sample_tree)

    def test_tree_trees_from_string(self):
        trees = bllipparser.Tree.trees_from_string('(S1(X    (NN junk)))')
        assert len(trees) == 1
        assert str(trees[0]) == '(S1 (X (NN junk)))'

        trees2 = bllipparser.Tree.trees_from_string('''(S1 (S (NP (DT This))
    (VP (VBZ is) (NP (DT a) (ADJP (RB fairly) (JJ simple)) (NN parse) (NN tree))) (. .)))
(S1 (X    (NN junk)))
(S1(X(NN nospace)))

(S1 (NP (DT another)
                                    (JJ junk)

        (NN tree)))
''')
        assert len(trees2) == 4
        assert str(trees2[0]) == sample_tree
        assert str(trees2[1]) == '(S1 (X (NN junk)))'
        assert str(trees2[2]) == '(S1 (X (NN nospace)))'
        assert str(trees2[3]) == '(S1 (NP (DT another) (JJ junk) (NN tree)))'

        trees3 = bllipparser.Tree.trees_from_string('')
        assert len(trees3) == 0

        trees4 = bllipparser.Tree.trees_from_string(sample_tree)
        assert len(trees4) == 1
        assert str(trees4[0]) == sample_tree

    def test_tree_modify_tree(self):
        tree = bllipparser.Tree(sample_tree)
        assert str(tree) == sample_tree
        assert str(tree[0][1][0]) == '(VBZ is)'

        tree[0][1][0].label = 'ZZZ'
        assert str(tree[0][1][0]) == '(ZZZ is)'

        tree[0][1][0].token = 'displays'
        assert str(tree[0][1][0]) == '(ZZZ displays)'
        assert str(tree) == '(S1 (S (NP (DT This)) (VP (ZZZ displays) (NP ' \
                            '(DT a) (ADJP (RB fairly) (JJ simple)) (NN ' \
                            'parse) (NN tree))) (. .)))'

        tree[0][1].label_suffix = '-SUFFIX'
        assert tree[0][1].label_suffix == '-SUFFIX'
        assert str(tree[0][1]) == '(VP-SUFFIX (ZZZ displays) (NP (DT a) ' \
                                  '(ADJP (RB fairly) (JJ simple)) (NN ' \
                                  'parse) (NN tree)))'

        tree[0][-1].token = '!'
        assert str(tree) == '(S1 (S (NP (DT This)) (VP-SUFFIX (ZZZ ' \
                            'displays) (NP (DT a) (ADJP (RB fairly) (JJ ' \
                            'simple)) (NN parse) (NN tree))) (. !)))'

        tree[0][1].label_suffix = ''
        assert str(tree) == '(S1 (S (NP (DT This)) (VP (ZZZ ' \
                            'displays) (NP (DT a) (ADJP (RB fairly) (JJ ' \
                            'simple)) (NN parse) (NN tree))) (. !)))'

        # slice testing
        pieces = tree[0][1][0:1]
        assert len(pieces) == 1
        assert str(pieces[0]) == '(ZZZ displays)'

        pieces2 = tree[0][1][-2:]
        assert len(pieces2) == 2
        assert str(pieces2[0]) == '(ZZZ displays)'
        assert str(pieces2[1]) == '(NP (DT a) (ADJP (RB fairly) (JJ ' \
                                  'simple)) (NN parse) (NN tree))'
