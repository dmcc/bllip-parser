# TODO: untested:
#       Tree.sd_tokens

def test_tree_errors():
    """
    First tests issue #33.

    >>> import bllipparser
    >>> bllipparser.Tree('(())')
    Traceback (most recent call last):
      File "/usr/lib/python2.7/doctest.py", line 1315, in __run
        compileflags, 1) in test.globs
      File "<doctest test_tree_errors[1]>", line 1, in <module>
        bllipparser.Tree('(())')
      File "/usr/local/lib/python2.7/dist-packages/bllipparser/RerankingParser.py", line 63, in __init__
        parser.inputTreeFromString(input_tree_or_string)
    RuntimeError: [first-stage/PARSE/InputTree.C:259]: Saw paren rather than term
    >>> bllipparser.Tree('(S1 (S (NP (DT This)) (VP (VBZ is) (NP (DT a) (ADJP (RB fairly) (JJ simple)) (NN parse) (NN tree))) (. .)))')
    Tree('(S1 (S (NP (DT This)) (VP (VBZ is) (NP (DT a) (ADJP (RB fairly) (JJ simple)) (NN parse) (NN tree))) (. .)))')
    >>> bllipparser.Tree('(S1 (S (NP (DT This)) (VP (VBZ is) (NP (DT a) (ADJP (RB fairly) (JJ simple)) (NN parse) (NN tree))) (. .)))')
    Tree('(S1 (S (NP (DT This)) (VP (VBZ is) (NP (DT a) (ADJP (RB fairly) (JJ simple)) (NN parse) (NN tree))) (. .)))')
    >>> bllipparser.Tree('(BADTOPTAG hi)')
    Traceback (most recent call last):
      File "/usr/lib/python2.7/doctest.py", line 1315, in __run
        compileflags, 1) in test.globs
      File "<doctest test_tree_errors[4]>", line 1, in <module>
        bllipparser.Tree('(BADTOPTAG hi)')
      File "/usr/local/lib/python2.7/dist-packages/bllipparser/RerankingParser.py", line 63, in __init__
        parser.inputTreeFromString(input_tree_or_string)
    RuntimeError: [first-stage/PARSE/InputTree.C:108]: Did not see legal topmost type
    >>> bllipparser.Tree('Does not start with a paren')
    Traceback (most recent call last):
      File "/usr/lib/python2.7/doctest.py", line 1315, in __run
        compileflags, 1) in test.globs
      File "<doctest test_tree_errors[5]>", line 1, in <module>
        bllipparser.Tree('Does not start with a paren')
      File "/usr/local/lib/python2.7/dist-packages/bllipparser/RerankingParser.py", line 63, in __init__
        parser.inputTreeFromString(input_tree_or_string)
    RuntimeError: [first-stage/PARSE/InputTree.C:97]: Saw 'Does' instead of open paren here.
    >>> bllipparser.Tree('(S1 eh)')
    Traceback (most recent call last):
      File "/usr/lib/python2.7/doctest.py", line 1315, in __run
        compileflags, 1) in test.globs
      File "<doctest test_tree_errors[6]>", line 1, in <module>
        bllipparser.Tree('(S1 eh)')
      File "/usr/local/lib/python2.7/dist-packages/bllipparser/RerankingParser.py", line 63, in __init__
        parser.inputTreeFromString(input_tree_or_string)
    RuntimeError: [first-stage/PARSE/InputTree.C:113]: Saw 'eh' instead of second open paren here.
    >>> bllipparser.Tree('(S1')
    Traceback (most recent call last):
      File "/usr/lib/python2.7/doctest.py", line 1315, in __run
        compileflags, 1) in test.globs
      File "<doctest test_tree_errors[7]>", line 1, in <module>
        bllipparser.Tree('(S1')
      File "/usr/local/lib/python2.7/dist-packages/bllipparser/RerankingParser.py", line 63, in __init__
        parser.inputTreeFromString(input_tree_or_string)
    RuntimeError: [first-stage/PARSE/InputTree.C:113]: Saw '' instead of second open paren here.
    >>> bllipparser.Tree('(S1 ((')
    Traceback (most recent call last):
      File "/usr/lib/python2.7/doctest.py", line 1315, in __run
        compileflags, 1) in test.globs
      File "<doctest test_tree_errors[8]>", line 1, in <module>
        bllipparser.Tree('(S1 ((')
      File "/usr/local/lib/python2.7/dist-packages/bllipparser/RerankingParser.py", line 63, in __init__
        parser.inputTreeFromString(input_tree_or_string)
    RuntimeError: [first-stage/PARSE/InputTree.C:259]: Saw paren rather than term
    >>> bllipparser.Tree('(S1 (NP')
    Traceback (most recent call last):
      File "/usr/lib/python2.7/doctest.py", line 1315, in __run
        compileflags, 1) in test.globs
      File "<doctest test_tree_errors[9]>", line 1, in <module>
        bllipparser.Tree('(S1 (NP')
      File "/usr/local/lib/python2.7/dist-packages/bllipparser/RerankingParser.py", line 63, in __init__
        parser.inputTreeFromString(input_tree_or_string)
    RuntimeError: [first-stage/PARSE/InputTree.C:158]: Unexpected end of string.
    """

def test_tree_basics():
    """
    >>> import bllipparser
    >>> tree = bllipparser.Tree('(S1 (S (NP (DT This)) (VP (VBZ is) (NP (DT a) (ADJP (RB fairly) (JJ simple)) (NN parse) (NN tree))) (. .)))')
    >>> print(tree)
    (S1 (S (NP (DT This)) (VP (VBZ is) (NP (DT a) (ADJP (RB fairly) (JJ simple)) (NN parse) (NN tree))) (. .)))
    >>> print(tree.pretty_string())
    (S1 (S (NP (DT This))
         (VP (VBZ is)
          (NP (DT a) (ADJP (RB fairly) (JJ simple)) (NN parse) (NN tree)))
         (. .)))
    >>> print(tree.tokens())
    ('This', 'is', 'a', 'fairly', 'simple', 'parse', 'tree', '.')
    >>> print(tree.tags())
    ('DT', 'VBZ', 'DT', 'RB', 'JJ', 'NN', 'NN', '.')
    >>> print(tree.tokens_and_tags())
    [('This', 'DT'), ('is', 'VBZ'), ('a', 'DT'), ('fairly', 'RB'), ('simple', 'JJ'), ('parse', 'NN'), ('tree', 'NN'), ('.', '.')]
    >>> print(tree.span())
    (0, 8)
    >>> print(str(tree.label))
    S1
    >>> tree.subtrees()
    [Tree('(S (NP (DT This)) (VP (VBZ is) (NP (DT a) (ADJP (RB fairly) (JJ simple)) (NN parse) (NN tree))) (. .))')]
    >>> tree[0] # first subtree
    Tree('(S (NP (DT This)) (VP (VBZ is) (NP (DT a) (ADJP (RB fairly) (JJ simple)) (NN parse) (NN tree))) (. .))')
    >>> tree[0].label
    'S'
    >>> tree[0][0] # first subtree of first subtree
    Tree('(NP (DT This))')
    >>> tree[0][0].label
    'NP'
    >>> tree[0][0].span()
    (0, 1)
    >>> tree[0][0].tags()
    ('DT',)
    >>> tree[0][0].tokens() # tuple of all tokens in this span
    ('This',)
    >>> tree[0][0][0]
    Tree('(DT This)')
    >>> tree[0][0][0].token
    'This'
    >>> tree[0][0][0].label
    'DT'
    >>> tree[0][0][0].is_preterminal()
    True
    >>> len(tree[0]) # number of subtrees
    3
    >>> for subtree in tree[0]:
    ...    print(subtree)
    ... 
    (NP (DT This))
    (VP (VBZ is) (NP (DT a) (ADJP (RB fairly) (JJ simple)) (NN parse) (NN tree)))
    (. .)
    >>> for subtree in tree.all_subtrees(): # all subtrees (recursive)
    ...     print('%s %s' % (subtree.is_preterminal(), subtree))
    ...
    False (S1 (S (NP (DT This)) (VP (VBZ is) (NP (DT a) (ADJP (RB fairly) (JJ simple)) (NN parse) (NN tree))) (. .)))
    False (S (NP (DT This)) (VP (VBZ is) (NP (DT a) (ADJP (RB fairly) (JJ simple)) (NN parse) (NN tree))) (. .))
    False (NP (DT This))
    True (DT This)
    False (VP (VBZ is) (NP (DT a) (ADJP (RB fairly) (JJ simple)) (NN parse) (NN tree)))
    True (VBZ is)
    False (NP (DT a) (ADJP (RB fairly) (JJ simple)) (NN parse) (NN tree))
    True (DT a)
    False (ADJP (RB fairly) (JJ simple))
    True (RB fairly)
    True (JJ simple)
    True (NN parse)
    True (NN tree)
    True (. .)
    """

def test_tree_modify_tree():
    """
    >>> import bllipparser
    >>> tree = bllipparser.Tree('(S1 (S (NP (DT This)) (VP (VBZ is) (NP (DT a) (ADJP (RB fairly) (JJ simple)) (NN parse) (NN tree))) (. .)))')
    >>> tree
    Tree('(S1 (S (NP (DT This)) (VP (VBZ is) (NP (DT a) (ADJP (RB fairly) (JJ simple)) (NN parse) (NN tree))) (. .)))')
    >>> tree[0][1][0]
    Tree('(VBZ is)')
    >>> tree[0][1][0].label = 'ZZZ'
    >>> tree[0][1][0]
    Tree('(ZZZ is)')
    >>> tree[0][1][0].token = 'displays'
    >>> tree[0][1][0]
    Tree('(ZZZ displays)')
    >>> tree
    Tree('(S1 (S (NP (DT This)) (VP (ZZZ displays) (NP (DT a) (ADJP (RB fairly) (JJ simple)) (NN parse) (NN tree))) (. .)))')
    >>> tree[0][1].label_suffix = '-SUFFIX'
    >>> tree[0][1].label_suffix
    '-SUFFIX'
    >>> tree[0][1]
    Tree('(VP-SUFFIX (ZZZ displays) (NP (DT a) (ADJP (RB fairly) (JJ simple)) (NN parse) (NN tree)))')
    >>> tree[0][1][0:1]
    [Tree('(ZZZ displays)')]
    >>> tree[0][1][-2:]
    [Tree('(ZZZ displays)'), Tree('(NP (DT a) (ADJP (RB fairly) (JJ simple)) (NN parse) (NN tree))')]
    """

def test_tree_trees_from_string():
    """
    >>> import bllipparser
    >>> trees = bllipparser.Tree.trees_from_string('(S1(X    (NN junk)))')
    >>> trees
    [Tree('(S1 (X (NN junk)))')]
    >>> trees = bllipparser.Tree.trees_from_string('''
    ... (S1 (S (NP (DT This))
    ...     (VP (VBZ is) (NP (DT a) (ADJP (RB fairly) (JJ simple)) (NN parse) (NN tree))) (. .)))
    ... (S1 (X    (NN junk)))
    ... (S1(X(NN nospace)))
    ... 
    ... (S1 (NP (DT another)
    ...                                     (JJ junk)
    ... 
    ...         (NN tree)))
    ... ''')
    ...
    >>> trees
    [Tree('(S1 (S (NP (DT This)) (VP (VBZ is) (NP (DT a) (ADJP (RB fairly) (JJ simple)) (NN parse) (NN tree))) (. .)))'), Tree('(S1 (X (NN junk)))'), Tree('(S1 (X (NN nospace)))'), Tree('(S1 (NP (DT another) (JJ junk) (NN tree)))')]
    """
