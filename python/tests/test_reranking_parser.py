from __future__ import print_function
# TODO: untested:
#   NBestList.sort_by_reranker_scores, sort_by_parser_scores,
#       get_parser_best, get_reranker_best, tokens, rerank,
#       as_reranker_input
#   Sentence.sentences_from_file, Tree.trees_from_file
#   model downloading
#   RerankingParser.from_unified_model_dir, parse_tagged with
#       invalid tags, set_parser_options

def stringify_dict(d):
    """Yeah, we should really stop using doctests."""
    def format(obj):
        if isinstance(obj, float):
            return '%.2f' % obj
        else:
            return repr(obj)
    return ', '.join('%s=%s' % (k, format(v)) for (k, v) in sorted(d.items()))

def test_reranking_parser_sentences():
    """
    >>> from bllipparser import Sentence
    >>> s = Sentence('Hi there.')
    >>> s
    Sentence(['Hi', 'there', '.'])
    >>> s.tokens()
    ['Hi', 'there', '.']
    >>> len(s)
    3
    >>> sentences = Sentence.sentences_from_string('<s> Test </s>')
    >>> sentences
    [Sentence(['Test'])]
    >>> sentences[0]
    Sentence(['Test'])
    >>> sentences[0].tokens()
    ['Test']
    >>> sentences[0].tokens()[0]
    'Test'
    >>> sentences2 = Sentence.sentences_from_string('''<s> Sentence 1 </s>
    ... <s> Can't have just one. </s>
    ... <s last> The last sentence </s>
    ... <s> Just kidding. </s>''')
    >>> sentences2
    [Sentence(['Sentence', '1']), Sentence(['Can', "n't", 'have', 'just', 'one', '.']), Sentence(['The', 'last', 'sentence']), Sentence(['Just', 'kidding', '.'])]
    >>> for s in sentences2:
    ...     print(s.tokens())
    ['Sentence', '1']
    ['Can', "n't", 'have', 'just', 'one', '.']
    ['The', 'last', 'sentence']
    ['Just', 'kidding', '.']
    """

def test_reranking_parser_tokenizer():
    """
    >>> from bllipparser import tokenize
    >>> tokenize("Tokenize this sentence, please.")
    ['Tokenize', 'this', 'sentence', ',', 'please', '.']
    >>> tokenize("Whoa! What's going on here? @($*")
    ['Whoa', '!', 'What', "'s", 'going', 'on', 'here', '?', '@', '-LRB-', '$', '*']
    >>> # arguably, this is a bug as 3 should have been separated from -LSB-
    >>> tokenize("You can't do that (or can you?). [3]")
    ['You', 'can', "n't", 'do', 'that', '-LRB-', 'or', 'can', 'you', '?', '-RRB-', '.', '-LSB-3', '-RSB-']
    """

def test_reranking_parser_loading_errors():
    """
    >>> from bllipparser import RerankingParser
    >>> rrp = RerankingParser()
    >>> rrp.load_parser_model('/path/to/nowhere/hopefully')
    Traceback (most recent call last):
      File "/usr/lib/python2.7/doctest.py", line 1315, in __run
        compileflags, 1) in test.globs
      File "<doctest test_reranking_parser_loading_errors[2]>", line 1, in <module>
        rrp.load_parser_model('/path/to/nowhere/hopefully')
      File "/usr/local/lib/python2.7/dist-packages/bllipparser/RerankingParser.py", line 483, in load_parser_model
        model_dir)
    ValueError: Parser model directory '/path/to/nowhere/hopefully' does not exist.
    >>> print(rrp.check_models_loaded_or_error(False))
    Traceback (most recent call last):
      File "/usr/lib/python2.7/doctest.py", line 1315, in __run
        compileflags, 1) in test.globs
      File "<doctest test_reranking_parser_loading_errors[3]>", line 1, in <module>
        print(rrp.check_models_loaded_or_error(False))
      File "/usr/local/lib/python2.7/dist-packages/bllipparser/RerankingParser.py", line 674, in check_models_loaded_or_error
        raise ValueError("Parser model has not been loaded.")
    ValueError: Parser model has not been loaded.
    >>> rrp.load_parser_model(u'\u2602')
    Traceback (most recent call last):
      File "/usr/lib/python2.7/doctest.py", line 1315, in __run
        compileflags, 1) in test.globs
      File "<doctest test_reranking_parser_loading_errors[4]>", line 1, in <module>
        rrp.load_parser_model(u'\u2602')
      File "/usr/local/lib/python2.7/dist-packages/bllipparser/RerankingParser.py", line 480, in load_parser_model
        'string.' % model_dir)
    ValueError: Parser model directory '\u2602' must be an ASCII string.
    >>> print(rrp.check_models_loaded_or_error(False))
    Traceback (most recent call last):
      File "/usr/lib/python2.7/doctest.py", line 1315, in __run
        compileflags, 1) in test.globs
      File "<doctest test_reranking_parser_loading_errors[3]>", line 1, in <module>
        print(rrp.check_models_loaded_or_error(False))
      File "/usr/local/lib/python2.7/dist-packages/bllipparser/RerankingParser.py", line 674, in check_models_loaded_or_error
        raise ValueError("Parser model has not been loaded.")
    ValueError: Parser model has not been loaded.
    >>> rrp.load_reranker_model(u'\u2602',
    ...                          'second-stage/models/ec50spfinal/cvlm-l1c10P1-weights.gz')
    Traceback (most recent call last):
      File "/usr/lib/python2.7/doctest.py", line 1315, in __run
        compileflags, 1) in test.globs
      File "<doctest test_reranking_parser_loading_errors[6]>", line 2, in <module>
        'second-stage/models/ec50spfinal/cvlm-l1c10P1-weights.gz')
      File "/usr/local/lib/python2.7/dist-packages/bllipparser/RerankingParser.py", line 497, in load_reranker_model
        'string.' % features_filename)
    ValueError: Reranker features filename '\u2602' must be an ASCII string.
    >>> rrp.load_reranker_model('second-stage/models/ec50spfinal/features.gz',
    ...                         u'\u2602')
    Traceback (most recent call last):
      File "/usr/lib/python2.7/doctest.py", line 1315, in __run
        compileflags, 1) in test.globs
      File "<doctest test_reranking_parser_loading_errors[7]>", line 2, in <module>
        u'\u2602')
      File "/usr/local/lib/python2.7/dist-packages/bllipparser/RerankingParser.py", line 502, in load_reranker_model
        'string.' % weights_filename)
    ValueError: Reranker weights filename '\u2602' must be an ASCII string.
    >>> print(rrp.check_models_loaded_or_error(True))
    Traceback (most recent call last):
      File "/usr/lib/python2.7/doctest.py", line 1315, in __run
        compileflags, 1) in test.globs
      File "<doctest test_reranking_parser_loading_errors[3]>", line 1, in <module>
        print(rrp.check_models_loaded_or_error(False))
      File "/usr/local/lib/python2.7/dist-packages/bllipparser/RerankingParser.py", line 674, in check_models_loaded_or_error
        raise ValueError("Parser model has not been loaded.")
    ValueError: Parser model has not been loaded.
    """

def test_reranking_parser_basics():
    """
    >>> from bllipparser import RerankingParser
    >>> rrp = RerankingParser()
    >>> rrp.load_parser_model('first-stage/DATA/EN')
    >>> print(rrp.check_models_loaded_or_error(False))
    False
    >>> rrp.check_models_loaded_or_error(True)
    Traceback (most recent call last):
      File "/usr/lib64/python2.6/doctest.py", line 1253, in __run
        compileflags, 1) in test.globs
      File "<doctest test_reranking_parser_basics[4]>", line 1, in <module>
        rrp.check_models_loaded_or_error(True)
      File "/home/dmcclosky/local/lib/python2.6/site-packages/bllipparser/RerankingParser.py", line 589, in check_models_loaded_or_error
        raise ValueError("Reranker model has not been loaded.")
    ValueError: Reranker model has not been loaded.
    >>> rrp.load_reranker_model('second-stage/models/ec50spfinal/features.gz',
    ...                         'second-stage/models/ec50spfinal/cvlm-l1c10P1-weights.gz')
    >>> print(rrp.check_models_loaded_or_error(False))
    False
    >>> rrp.check_models_loaded_or_error(True)
    True
    >>> rrp.check_models_loaded_or_error('auto')
    True
    >>> rrp.parser_model_dir
    'first-stage/DATA/EN'
    >>> rrp.simple_parse('This is simple.')
    '(S1 (S (NP (DT This)) (VP (AUX is) (ADJP (JJ simple))) (. .)))'
    >>> nbest_list = rrp.parse('This is a sentence.')
    >>> print(str(nbest_list).strip())
    13 x
    -8.88655845608 -30.3981669701
    (S1 (S (NP (DT This)) (VP (AUX is) (NP (DT a) (NN sentence))) (. .)))
    -13.936145728 -46.4346864304
    (S1 (S (NP (NNP This)) (VP (AUX is) (NP (DT a) (NN sentence))) (. .)))
    -14.3607122818 -47.4390055933
    (S1 (S (NP (NN This)) (VP (AUX is) (NP (DT a) (NN sentence))) (. .)))
    -14.7026007585 -41.4723634172
    (S1 (S (NP (DT This)) (VP (AUX is) (S (NP (DT a) (NN sentence)))) (. .)))
    -15.3583543915 -48.567244735
    (S1 (S (DT This) (VP (AUX is) (NP (DT a) (NN sentence))) (. .)))
    -19.285724575 -56.2161267587
    (S1 (SBARQ (WHNP (DT This)) (SQ (AUX is) (NP (DT a) (NN sentence))) (. .)))
    -19.7521880305 -57.5088828776
    (S1 (S (NP (NNP This)) (VP (AUX is) (S (NP (DT a) (NN sentence)))) (. .)))
    -20.1767545843 -58.5132020405
    (S1 (S (NP (NN This)) (VP (AUX is) (S (NP (DT a) (NN sentence)))) (. .)))
    -20.2330660538 -55.5759876981
    (S1 (SBARQ (WHNP (DT This)) (SQ (VP (AUX is) (NP (DT a) (NN sentence)))) (. .)))
    -20.3467824313 -59.0747445934
    (S1 (S (ADVP (DT This)) (VP (AUX is) (NP (DT a) (NN sentence))) (. .)))
    -21.174396694 -59.6414411821
    (S1 (S (DT This) (VP (AUX is) (S (NP (DT a) (NN sentence)))) (. .)))
    -26.1628247309 -70.1489410336
    (S1 (S (ADVP (DT This)) (VP (AUX is) (S (NP (DT a) (NN sentence)))) (. .)))
    -26.7808410125 -68.4818143615
    (S1 (SBARQ (WHNP (DT This)) (SQ (VP (AUX is) (S (NP (DT a) (NN sentence))))) (. .)))
    >>> print(nbest_list[0].ptb_parse)
    (S1 (S (NP (DT This)) (VP (AUX is) (NP (DT a) (NN sentence))) (. .)))
    >>> print(nbest_list[0].parser_score)
    -30.3981669701
    >>> print(nbest_list[0].reranker_score)
    -8.88655845608
    >>> print(len(nbest_list))
    13
    >>> print(nbest_list[0].ptb_parse.log_prob())
    -30.3981669701
    >>> nbest_list[0].ptb_parse.log_prob() == nbest_list[0].parser_score
    True
    >>> print(nbest_list.fuse())
    (S1 (S (NP (DT This)) (VP (AUX is) (NP (DT a) (NN sentence))) (. .)))
    >>> print(nbest_list.fuse(use_parser_scores=True))
    (S1 (S (NP (DT This)) (VP (AUX is) (NP (DT a) (NN sentence))) (. .)))
    >>> nbest_list2 = rrp.parse(['This', 'is', 'a', 'pretokenized', 'sentence', '.'])
    >>> print(str(nbest_list2).strip())
    50 x
    -13.9140458986 -49.4538516291
    (S1 (S (NP (DT This)) (VP (AUX is) (NP (DT a) (JJ pretokenized) (NN sentence))) (. .)))
    -16.0212658926 -54.3324639691
    (S1 (S (NP (DT This)) (VP (AUX is) (NP (DT a) (VBN pretokenized) (NN sentence))) (. .)))
    -17.5114530692 -58.3751587397
    (S1 (S (NP (DT This)) (VP (AUX is) (NP (DT a) (NN pretokenized) (NN sentence))) (. .)))
    -17.6880864662 -58.7187102935
    (S1 (S (NP (DT This)) (VP (AUX is) (NP (DT a) (JJS pretokenized) (NN sentence))) (. .)))
    -17.8510752677 -60.8294536479
    (S1 (S (NP (DT This)) (VP (AUX is) (NP (DT a) (JJR pretokenized) (NN sentence))) (. .)))
    -18.9636331706 -65.4903710895
    (S1 (S (NP (NNP This)) (VP (AUX is) (NP (DT a) (JJ pretokenized) (NN sentence))) (. .)))
    -19.3881997244 -66.4946902523
    (S1 (S (NP (NN This)) (VP (AUX is) (NP (DT a) (JJ pretokenized) (NN sentence))) (. .)))
    -19.4209711415 -59.1651025727
    (S1 (S (NP (DT This)) (VP (AUX is) (NP (DT a) (ADJP (VBN pretokenized)) (NN sentence))) (. .)))
    -19.7727647746 -67.2653539515
    (S1 (S (NP (DT This)) (VP (AUX is) (NP (DT a) (VBD pretokenized) (NN sentence))) (. .)))
    -20.3192490811 -63.5531840148
    (S1 (S (NP (DT This)) (VP (AUX is) (S (NP (DT a) (JJ pretokenized) (NN sentence)))) (. .)))
    -20.4170868341 -67.622929394
    (S1 (S (DT This) (VP (AUX is) (NP (DT a) (JJ pretokenized) (NN sentence))) (. .)))
    -20.4219469891 -59.1944994701
    (S1 (S (NP (DT This)) (VP (AUX is) (NP (DT a) (ADJP (JJ pretokenized)) (NN sentence))) (. .)))
    -21.0708531645 -70.3689834294
    (S1 (S (NP (NNP This)) (VP (AUX is) (NP (DT a) (VBN pretokenized) (NN sentence))) (. .)))
    -21.4954197183 -71.3733025923
    (S1 (S (NP (NN This)) (VP (AUX is) (NP (DT a) (VBN pretokenized) (NN sentence))) (. .)))
    -21.512397545 -67.4255190551
    (S1 (S (NP (DT This)) (VP (AUX is) (DT a) (VP (VBN pretokenized) (NP (NN sentence)))) (. .)))
    -21.5209365225 -65.5850885241
    (S1 (S (NP (DT This)) (VP (AUX is) (NP (DT a) (ADJP (VBD pretokenized)) (NN sentence))) (. .)))
    -21.5396901802 -65.9911116663
    (S1 (S (NP (DT This)) (VP (AUX is) (S (NP (DT a) (VBN pretokenized) (NN sentence)))) (. .)))
    -22.1177810904 -67.5021157814
    (S1 (S (NP (DT This)) (VP (AUX is) (NP (DT a) (ADJP (JJR pretokenized)) (NN sentence))) (. .)))
    -22.2524735497 -66.9699233838
    (S1 (S (NP (DT This)) (VP (AUX is) (NP (DT a) (NX (JJ pretokenized) (NN sentence)))) (. .)))
    -22.3283866387 -71.4009340869
    (S1 (S (NP (DT This)) (VP (AUX is) (NP (DT a) (ADJP (JJS pretokenized)) (NN sentence))) (. .)))
    -22.524306828 -72.501541734
    (S1 (S (DT This) (VP (AUX is) (NP (DT a) (VBN pretokenized) (NN sentence))) (. .)))
    -22.5610403412 -74.4116782
    (S1 (S (NP (NNP This)) (VP (AUX is) (NP (DT a) (NN pretokenized) (NN sentence))) (. .)))
    -22.6931925386 -69.1503165846
    (S1 (S (NP (DT This)) (VP (AUX is) (NP (DT a) (NX (VBN pretokenized) (NN sentence)))) (. .)))
    -22.7376737381 -74.7552297539
    (S1 (S (NP (NNP This)) (VP (AUX is) (NP (DT a) (JJS pretokenized) (NN sentence))) (. .)))
    -22.8282464284 -67.9116806301
    (S1 (S (NP (DT This)) (VP (AUX is) (NP (NP (DT a)) (JJ pretokenized) (NN sentence))) (. .)))
    -22.9006625397 -76.8659731082
    (S1 (S (NP (NNP This)) (VP (AUX is) (NP (DT a) (JJR pretokenized) (NN sentence))) (. .)))
    -22.985606895 -75.4159973629
    (S1 (S (NP (NN This)) (VP (AUX is) (NP (DT a) (NN pretokenized) (NN sentence))) (. .)))
    -23.1622402947 -75.7595489168
    (S1 (S (NP (NN This)) (VP (AUX is) (NP (DT a) (JJS pretokenized) (NN sentence))) (. .)))
    -23.243264351 -71.0336954504
    (S1 (S (NP (DT This)) (VP (AUX is) (S (NP (DT a) (NN pretokenized) (NN sentence)))) (. .)))
    -23.2687569853 -75.4784919433
    (S1 (S (NP (DT This)) (VP (AUX is) (DT a) (VP (VBD pretokenized) (NP (NN sentence)))) (. .)))
    -23.3252290934 -77.8702922711
    (S1 (S (NP (NN This)) (VP (AUX is) (NP (DT a) (JJR pretokenized) (NN sentence))) (. .)))
    -23.4064078284 -71.7516211232
    (S1 (S (NP (DT This)) (VP (AUX is) (NP (NP (DT a)) (VBN pretokenized) (NN sentence))) (. .)))
    -23.4583333097 -71.3666592906
    (S1 (S (NP (DT This)) (VP (AUX is) (S (NP (DT a) (JJS pretokenized) (NN sentence)))) (. .)))
    -23.6237874662 -73.7851216229
    (S1 (S (NP (DT This)) (VP (AUX is) (S (NP (DT a) (JJR pretokenized) (NN sentence)))) (. .)))
    -24.0144940047 -76.5442365046
    (S1 (S (DT This) (VP (AUX is) (NP (DT a) (NN pretokenized) (NN sentence))) (. .)))
    -24.1911274044 -76.8877880584
    (S1 (S (DT This) (VP (AUX is) (NP (DT a) (JJS pretokenized) (NN sentence))) (. .)))
    -24.4705584135 -75.2016220331
    (S1 (S (NP (NNP This)) (VP (AUX is) (NP (DT a) (ADJP (VBN pretokenized)) (NN sentence))) (. .)))
    -24.8951249701 -76.205941196
    (S1 (S (NP (NN This)) (VP (AUX is) (NP (DT a) (ADJP (VBN pretokenized)) (NN sentence))) (. .)))
    -25.0212681665 -71.3769439695
    (S1 (S (NP (DT This)) (VP (AUX is) (S (NP (DT a) (ADJP (VBN pretokenized)) (NN sentence)))) (. .)))
    -25.1973992739 -78.1304292524
    (S1 (S (ADVP (DT This)) (VP (AUX is) (NP (DT a) (JJ pretokenized) (NN sentence))) (. .)))
    -25.4715342611 -75.2310189305
    (S1 (S (NP (NNP This)) (VP (AUX is) (NP (DT a) (ADJP (JJ pretokenized)) (NN sentence))) (. .)))
    -25.4803163686 -72.8673204341
    (S1 (S (NP (DT This)) (VP (AUX is) (NP (DT a) (NX (ADJP (JJ pretokenized)) (NN sentence)))) (. .)))
    -25.6554871258 -73.3251916009
    (S1 (S (NP (DT This)) (VP (AUX is) (NP (DT a) (NX (ADJP (VBN pretokenized)) (NN sentence)))) (. .)))
    -25.8961008149 -76.2353380934
    (S1 (S (NP (NN This)) (VP (AUX is) (NP (DT a) (ADJP (JJ pretokenized)) (NN sentence))) (. .)))
    -25.9240120798 -77.3341803376
    (S1 (S (DT This) (VP (AUX is) (NP (DT a) (ADJP (VBN pretokenized)) (NN sentence))) (. .)))
    -26.4170898917 -71.9105721867
    (S1 (S (NP (DT This)) (VP (AUX is) (S (NP (DT a) (ADJP (JJ pretokenized)) (NN sentence)))) (. .)))
    -26.9249879274 -77.3635772351
    (S1 (S (DT This) (VP (AUX is) (NP (DT a) (ADJP (JJ pretokenized)) (NN sentence))) (. .)))
    -27.1124193372 -77.7040813639
    (S1 (S (NP (DT This)) (VP (AUX is) (S (NP (DT a) (ADJP (VBD pretokenized)) (NN sentence)))) (. .)))
    -27.2836774072 -77.313649149
    (S1 (S (NP (DT This)) (VP (AUX is) (NP (NP (DT a)) (ADJP (VBN pretokenized)) (NN sentence))) (. .)))
    -28.0026072817 -76.0416349799
    (S1 (S (NP (DT This)) (VP (AUX is) (NP (NP (DT a)) (ADJP (JJ pretokenized)) (NN sentence))) (. .)))
    >>> nbest_list3 = rrp.parse('Parser only!', rerank=False, sentence_id='parser_only')
    >>> print(str(nbest_list3).strip())
    50 parser_only
    -52.57783414
    (S1 (S (VP (VB Parser) (ADVP (RB only))) (. !)))
    -53.19573267
    (S1 (FRAG (NP (NN Parser) (RB only)) (. !)))
    -54.54836523
    (S1 (FRAG (ADVP (RBR Parser) (RB only)) (. !)))
    -55.09170692
    (S1 (FRAG (NP (NN Parser)) (ADVP (RB only)) (. !)))
    -55.14038635
    (S1 (FRAG (NP (NNP Parser)) (ADVP (RB only)) (. !)))
    -57.25584872
    (S1 (FRAG (ADJP (JJR Parser)) (ADVP (RB only)) (. !)))
    -57.39656583
    (S1 (S (VP (VBP Parser) (ADVP (RB only))) (. !)))
    -57.60634106
    (S1 (S (VP (VB Parser) (ADVP (JJ only))) (. !)))
    -57.85039025
    (S1 (S (VP (VB Parser) (RB only)) (. !)))
    -57.87021346
    (S1 (FRAG (ADJP (JJ Parser)) (ADVP (RB only)) (. !)))
    -57.89165223
    (S1 (FRAG (ADVP (JJR Parser)) (RB only) (. !)))
    -58.64850061
    (S1 (FRAG (ADJP (RBR Parser) (JJ only)) (. !)))
    -58.71571915
    (S1 (FRAG (NP (NN Parser)) (ADJP (RB only)) (. !)))
    -58.75007348
    (S1 (FRAG (ADVP (RB Parser)) (RB only) (. !)))
    -58.76439858
    (S1 (FRAG (NP (NNP Parser)) (ADJP (RB only)) (. !)))
    -58.92639016
    (S1 (FRAG (ADVP (RB Parser) (RB only)) (. !)))
    -59.10118489
    (S1 (FRAG (NP (NNP Parser) (RB only)) (. !)))
    -59.42661454
    (S1 (FRAG (NP (NNP Parser)) (ADJP (JJ only)) (. !)))
    -59.59006341
    (S1 (FRAG (RB Parser) (ADVP (RB only)) (. !)))
    -59.65817632
    (S1 (FRAG (NP (NN Parser)) (ADJP (JJ only)) (. !)))
    -59.73616513
    (S1 (FRAG (ADJP (JJR Parser)) (NP (RB only)) (. !)))
    -59.93976344
    (S1 (FRAG (NP (NP (NN Parser)) (ADVP (RB only))) (. !)))
    -60.35052988
    (S1 (FRAG (ADJP (JJ Parser)) (NP (RB only)) (. !)))
    -60.38657945
    (S1 (S (VP (VB Parser) (NP (RB only))) (. !)))
    -60.57674496
    (S1 (SQ (VP (VB Parser) (ADVP (RB only))) (. !)))
    -60.62371178
    (S1 (FRAG (ADJP (NNP Parser) (JJ only)) (. !)))
    -60.63872478
    (S1 (FRAG (RB Parser) (RB only) (. !)))
    -60.68395245
    (S1 (FRAG (ADVP (JJ Parser)) (RB only) (. !)))
    -60.69738473
    (S1 (NP (NP (NNP Parser)) (NP (RB only)) (. !)))
    -60.70443033
    (S1 (NP (NN Parser) (RB only) (. !)))
    -60.75513913
    (S1 (FRAG (ADJP (RBR Parser) (RB only)) (. !)))
    -60.81313407
    (S1 (FRAG (ADVP (RBR Parser)) (RB only) (. !)))
    -60.83595554
    (S1 (FRAG (ADJP (JJR Parser) (JJ only)) (. !)))
    -60.893467
    (S1 (S (VP (VB Parser) (ADJP (RB only))) (. !)))
    -61.02350358
    (S1 (FRAG (ADVP (NN Parser)) (RB only) (. !)))
    -61.22216468
    (S1 (FRAG (NP (NP (NNP Parser)) (ADVP (RB only))) (. !)))
    -61.34291471
    (S1 (NP (NNP Parser) (RB only) (. !)))
    -61.38022269
    (S1 (FRAG (NP (JJ Parser)) (ADVP (RB only)) (. !)))
    -61.43308909
    (S1 (FRAG (ADVP (JJR Parser)) (ADVP (RB only)) (. !)))
    -61.4726006
    (S1 (FRAG (NP (NN Parser)) (NP (RB only)) (. !)))
    -61.49864523
    (S1 (FRAG (NP (JJR Parser)) (ADVP (RB only)) (. !)))
    -61.52128003
    (S1 (FRAG (NP (NNP Parser)) (NP (RB only)) (. !)))
    -61.59037588
    (S1 (S (VP (VB Parser) (ADJP (JJ only))) (. !)))
    -61.60397522
    (S1 (FRAG (JJ Parser) (ADVP (RB only)) (. !)))
    -61.67405796
    (S1 (S (VP (NN Parser) (ADVP (RB only))) (. !)))
    -61.6908843
    (S1 (FRAG (ADVP (NNP Parser)) (RB only) (. !)))
    -61.74601035
    (S1 (S (NP (NNP Parser)) (ADJP (JJ only)) (. !)))
    -61.91324518
    (S1 (FRAG (RB Parser) (ADJP (RB only)) (. !)))
    -61.94221948
    (S1 (S (ADJP (RBR Parser) (JJ only)) (. !)))
    -61.97779994
    (S1 (FRAG (ADJP (JJ Parser) (JJ only)) (. !)))
    >>> nbest_list3.fuse(use_parser_scores=True)
    Tree('(S1 (S (VP (VB Parser) (ADVP (RB only))) (. !)))')
    >>> nbest_list3.fuse(use_parser_scores=False)
    Tree('(S1 (S (VP (VB Parser) (ADVP (RB only))) (. !)))')
    >>> rrp.parse_tagged(['Time', 'flies'], possible_tags={})[0].ptb_parse
    Tree('(S1 (S (NP (NNP Time)) (VP (VBZ flies))))')
    >>> rrp.parse_tagged(['Time', 'flies'], possible_tags={0 : 'VB', 1 : 'NNS'})[0].ptb_parse
    Tree('(S1 (NP (VB Time) (NNS flies)))')
    >>> rrp.parse_tagged(['Time', 'flies'], possible_tags={0 : 'VB'})[0].ptb_parse
    Tree('(S1 (S (VP (VB Time) (NP (VBZ flies)))))')
    >>> rrp.parse_tagged(['Time', 'flies'], possible_tags={0 : ['VB', 'JJ', 'NN']})[0].ptb_parse
    Tree('(S1 (NP (NN Time) (VBZ flies)))')
    >>> stringify_dict(rrp.set_parser_options(nbest=10))
    "case_insensitive=False, debug=0, language='En', nbest=10, overparsing=21, small_corpus=True, smooth_pos=0"
    >>> nbest_list = rrp.parse('The list is smaller now.', rerank=False)
    >>> len(nbest_list)
    10
    >>> rrp.tag("Time flies while you're having fun.")
    [('Time', 'NNP'), ('flies', 'VBZ'), ('while', 'IN'), ('you', 'PRP'), ("'re", 'AUX'), ('having', 'AUXG'), ('fun', 'NN'), ('.', '.')]
    >>> rrp.tag('British left waffles on Falklands .'.split())
    [('British', 'JJ'), ('left', 'NN'), ('waffles', 'VBZ'), ('on', 'IN'), ('Falklands', 'NNP'), ('.', '.')]
    >>> from bllipparser import Sentence
    >>> Sentence('British left waffles on Falklands .').independent_tags()
    ('JJ', 'VBN', 'NNS', 'IN', 'NNP', '.')
    >>> constraints = {(1, 5) : ['VP']}
    >>> nbest_list = rrp.parse_constrained('British left waffles on Falklands .'.split(), constraints)
    >>> print(str(nbest_list).strip())
    10 x
    -25.836244321 -93.6286744642
    (S1 (S (NP (NNPS British)) (VP (VBD left) (NP (NNS waffles)) (PP (IN on) (NP (NNP Falklands)))) (. .)))
    -25.9966925705 -95.7474111377
    (S1 (S (NP (NNPS British)) (VP (VBD left) (NP (NNS waffles)) (PP (IN on) (NP (NNPS Falklands)))) (. .)))
    -26.6154733928 -93.1372330926
    (S1 (S (NP (NNPS British)) (VP (VBD left) (NP (NP (NNS waffles)) (PP (IN on) (NP (NNP Falklands))))) (. .)))
    -26.9453743621 -94.8459445679
    (S1 (S (NP (NNPS British)) (VP (VBD left) (NP (NP (NNS waffles)) (PP (IN on) (NP (NNPS Falklands))))) (. .)))
    -27.0537353446 -93.6112342559
    (S1 (S (NP (NNP British)) (VP (VBD left) (NP (NNS waffles)) (PP (IN on) (NP (NNP Falklands)))) (. .)))
    -27.3221541914 -95.7299709295
    (S1 (S (NP (NNP British)) (VP (VBD left) (NP (NNS waffles)) (PP (IN on) (NP (NNPS Falklands)))) (. .)))
    -27.9003378837 -93.1197928843
    (S1 (S (NP (NNP British)) (VP (VBD left) (NP (NP (NNS waffles)) (PP (IN on) (NP (NNP Falklands))))) (. .)))
    -28.2198807661 -95.9050765306
    (S1 (S (NP (NNS British)) (VP (VBD left) (NP (NNS waffles)) (PP (IN on) (NP (NNP Falklands)))) (. .)))
    -28.338209453 -94.8285043597
    (S1 (S (NP (NNP British)) (VP (VBD left) (NP (NP (NNS waffles)) (PP (IN on) (NP (NNPS Falklands))))) (. .)))
    -29.122754708 -95.4136351589
    (S1 (S (NP (NNS British)) (VP (VBD left) (NP (NP (NNS waffles)) (PP (IN on) (NP (NNP Falklands))))) (. .)))
    >>> nbest_list2 = rrp.parse_constrained('British left waffles on Falklands .'.split(), {})
    >>> print(str(nbest_list2).strip())
    10 x
    -25.8126695909 -90.2342444645
    (S1 (S (NP (JJ British) (NN left)) (VP (VBZ waffles) (PP (IN on) (NP (NNP Falklands)))) (. .)))
    -25.836244321 -93.6286744642
    (S1 (S (NP (NNPS British)) (VP (VBD left) (NP (NNS waffles)) (PP (IN on) (NP (NNP Falklands)))) (. .)))
    -26.0312053125 -92.352981138
    (S1 (S (NP (JJ British) (NN left)) (VP (VBZ waffles) (PP (IN on) (NP (NNPS Falklands)))) (. .)))
    -26.6154733928 -93.1372330926
    (S1 (S (NP (NNPS British)) (VP (VBD left) (NP (NP (NNS waffles)) (PP (IN on) (NP (NNP Falklands))))) (. .)))
    -26.9371121677 -93.9026623336
    (S1 (S (NP (JJ British) (NN left)) (VP (VBZ waffles) (PP (IN on) (NP (NNS Falklands)))) (. .)))
    -26.9453743621 -94.8459445679
    (S1 (S (NP (NNPS British)) (VP (VBD left) (NP (NP (NNS waffles)) (PP (IN on) (NP (NNPS Falklands))))) (. .)))
    -27.0537353446 -93.6112342559
    (S1 (S (NP (NNP British)) (VP (VBD left) (NP (NNS waffles)) (PP (IN on) (NP (NNP Falklands)))) (. .)))
    -27.3630657512 -95.1571335758
    (S1 (S (NP (NNP British) (NN left)) (VP (VBZ waffles) (PP (IN on) (NP (NNP Falklands)))) (. .)))
    -27.9003378837 -93.1197928843
    (S1 (S (NP (NNP British)) (VP (VBD left) (NP (NP (NNS waffles)) (PP (IN on) (NP (NNP Falklands))))) (. .)))
    -28.338209453 -94.8285043597
    (S1 (S (NP (NNP British)) (VP (VBD left) (NP (NP (NNS waffles)) (PP (IN on) (NP (NNPS Falklands))))) (. .)))
    >>> nbest_list3 = rrp.parse_constrained('British left waffles on Falklands .'.split(), constraints, possible_tags={1: 'VBD'})
    >>> assert str(nbest_list).strip() == str(nbest_list3).strip()
    >>> nbest_list4 = rrp.parse_constrained('British left waffles on Falklands .'.split(), constraints, possible_tags={1: 'VBZ'})
    >>> print(str(nbest_list4).strip())
    10 x
    -30.0747237573 -106.808764217
    (S1 (S (NP (NNP British)) (VP (VBZ left) (NP (NNS waffles)) (PP (IN on) (NP (NNP Falklands)))) (. .)))
    -30.4801072424 -108.927500891
    (S1 (S (NP (NNP British)) (VP (VBZ left) (NP (NNS waffles)) (PP (IN on) (NP (NNPS Falklands)))) (. .)))
    -30.5333433842 -108.948330724
    (S1 (S (NP (NNPS British)) (VP (VBZ left) (NP (NNS waffles)) (PP (IN on) (NP (NNP Falklands)))) (. .)))
    -30.8151980896 -104.805165121
    (S1 (S (NP (NNP British)) (VP (VBZ left) (NP (NP (NNS waffles)) (PP (IN on) (NP (NNP Falklands))))) (. .)))
    -31.2292881945 -106.513876597
    (S1 (S (NP (NNP British)) (VP (VBZ left) (NP (NP (NNS waffles)) (PP (IN on) (NP (NNPS Falklands))))) (. .)))
    -31.2785161465 -106.944731628
    (S1 (S (NP (NNPS British)) (VP (VBZ left) (NP (NP (NNS waffles)) (PP (IN on) (NP (NNP Falklands))))) (. .)))
    -31.5846356514 -108.653443103
    (S1 (S (NP (NNPS British)) (VP (VBZ left) (NP (NP (NNS waffles)) (PP (IN on) (NP (NNPS Falklands))))) (. .)))
    -31.7626299938 -108.627394514
    (S1 (S (NP (NNP British)) (VP (VBZ left) (NP (NP (NNS waffles)) (PP (IN on) (NP (NNS Falklands))))) (. .)))
    -33.2085090166 -107.19852478
    (S1 (S (NP (JJ British)) (VP (VBZ left) (NP (NP (NNS waffles)) (PP (IN on) (NP (NNP Falklands))))) (. .)))
    -33.5519491115 -108.907236256
    (S1 (S (NP (JJ British)) (VP (VBZ left) (NP (NP (NNS waffles)) (PP (IN on) (NP (NNPS Falklands))))) (. .)))
    >>> constraints[(1, 2)] = ['VBZ']
    >>> nbest_list5 = rrp.parse_constrained('British left waffles on Falklands .'.split(), constraints)
    >>> assert str(nbest_list4).strip() == str(nbest_list5).strip()
    >>> constraints = {(2, 4): ['NP'], (0, 1): ['VP']}
    >>> nbest_list6 = rrp.parse_constrained('British left waffles on Falklands .'.split(), constraints)
    >>> print(str(nbest_list6).strip())
    0 x
    >>> assert len(nbest_list6) == 0
    >>> constraints = {(1, 5) : 'VP'}
    >>> nbest_list7 = rrp.parse_constrained('British left waffles on Falklands .'.split(), constraints)
    >>> assert str(nbest_list).strip() == str(nbest_list7).strip()
    >>> rrp.parse('a ' * 398)
    []
    >>> rrp.parse('b ' * 399)
    Traceback (most recent call last):
      File "/usr/lib64/python2.6/doctest.py", line 1253, in __run
        compileflags, 1) in test.globs
      File "<doctest test_reranking_parser_basics[53]>", line 1, in <module>
        rrp.parse('b ' * 399)
      File "/home/dmcclosky/local/lib/python2.6/site-packages/bllipparser/RerankingParser.py", line 565, in parse
        (len(sentence), parser.max_sentence_length - 1))
    ValueError: Sentence is too long (399 tokens, must be under 399)
    >>> rrp.parse('c ' * 400)
    Traceback (most recent call last):
      File "/usr/lib64/python2.6/doctest.py", line 1253, in __run
        compileflags, 1) in test.globs
      File "<doctest test_reranking_parser_basics[53]>", line 1, in <module>
        rrp.parse('c ' * 400)
      File "/home/dmcclosky/local/lib/python2.6/site-packages/bllipparser/RerankingParser.py", line 565, in parse
        (len(sentence), parser.max_sentence_length - 1))
    ValueError: Sentence is too long (400 tokens, must be under 399)
    >>> rrp.tag('# ! ? : -', allow_failures=True)
    Traceback (most recent call last):
      File "/usr/lib64/python2.6/doctest.py", line 1253, in __run
        compileflags, 1) in test.globs
      File "<doctest test_reranking_parser_basics[56]>", line 1, in <module>
        rrp.tag('# ! ? : -', allow_failures=True)
      File "/usr/lib64/python2.6/site-packages/bllipparser/RerankingParser.py", line 697, in tag
        raise ValueError('Parse failed while tagging: %r' % text_or_tokens)
    ValueError: Parse failed while tagging: '# ! ? : -'
    >>> rrp.tag('# ! ? : -', allow_failures=False)
    [('#', '#'), ('!', '.'), ('?', '.'), (':', ':'), ('-', ':')]
    >>> stringify_dict(rrp.set_parser_options(nbest=50))
    "case_insensitive=False, debug=0, language='En', nbest=50, overparsing=21, small_corpus=True, smooth_pos=0"
    >>> complex_sentence = 'Economists are divided as to how much manufacturing strength they expect to see in September reports on industrial production and capacity utilization, also due tomorrow.'
    >>> print(rrp.simple_parse(complex_sentence))
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WHADJP (WRB how) (JJ much)) (NN manufacturing) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NNS reports)) (PP (IN on) (NP (NP (JJ industrial) (NN production)) (CC and) (NP (NP (NN capacity) (NN utilization)) (, ,) (NP (ADJP (ADVP (RB also)) (JJ due)) (NN tomorrow)))))))))))))))) (. .)))
    >>> nbest_list_complex = rrp.parse(complex_sentence)
    >>> print(str(nbest_list_complex).strip())
    50 x
    -82.7890527858 -256.862754458
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WHADJP (WRB how) (JJ much)) (NN manufacturing) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NNS reports)) (PP (IN on) (NP (NP (JJ industrial) (NN production)) (CC and) (NP (NP (NN capacity) (NN utilization)) (, ,) (NP (ADJP (ADVP (RB also)) (JJ due)) (NN tomorrow)))))))))))))))) (. .)))
    -82.9165082389 -255.862921846
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WRB how) (JJ much) (NN manufacturing) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NNS reports)) (PP (IN on) (NP (NP (JJ industrial) (NN production)) (CC and) (NP (NP (NN capacity) (NN utilization)) (, ,) (NP (ADJP (ADVP (RB also)) (JJ due)) (NN tomorrow)))))))))))))))) (. .)))
    -83.2414322173 -257.424786092
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WHADVP (WRB how) (JJ much) (NN manufacturing)) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NNS reports)) (PP (IN on) (NP (NP (NP (JJ industrial) (NN production)) (CC and) (NP (NN capacity) (NN utilization))) (, ,) (NP (ADJP (ADVP (RB also)) (JJ due)) (NN tomorrow))))))))))))))) (. .)))
    -83.3396120234 -257.449514707
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WHADJP (WRB how) (JJ much)) (NN manufacturing) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NNS reports)) (PP (IN on) (NP (NP (JJ industrial) (NN production)) (CC and) (NP (NN capacity) (NN utilization))))) (, ,) (PP (ADVP (RB also)) (RB due) (NP (NN tomorrow))))))))))))) (. .)))
    -83.3590774333 -257.207857799
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WHADVP (WRB how) (JJ much) (NN manufacturing)) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NNS reports)) (PP (IN on) (NP (NP (JJ industrial) (NN production)) (CC and) (NP (NN capacity) (NN utilization)))) (, ,) (RRC (ADVP (RB also)) (NP (JJ due) (NN tomorrow)))))))))))))) (. .)))
    -83.4670675044 -256.449682095
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WRB how) (JJ much) (NN manufacturing) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NNS reports)) (PP (IN on) (NP (NP (JJ industrial) (NN production)) (CC and) (NP (NN capacity) (NN utilization))))) (, ,) (PP (ADVP (RB also)) (RB due) (NP (NN tomorrow))))))))))))) (. .)))
    -83.5673600986 -255.564606573
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WHADVP (WRB how) (JJ much) (NN manufacturing)) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NNS reports)) (PP (IN on) (NP (NP (JJ industrial) (NN production)) (CC and) (NP (NP (NN capacity) (NN utilization)) (PRN (, ,) (ADVP (RB also)) (NP (JJ due) (NN tomorrow))))))))))))))))) (. .)))
    -83.5718465213 -256.514298268
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WHADJP (WRB how) (JJ much)) (NN manufacturing) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NNS reports)) (PP (IN on) (NP (NP (NP (JJ industrial) (NN production)) (CC and) (NP (NN capacity) (NN utilization))) (, ,) (NP (ADJP (ADVP (RB also)) (JJ due)) (NN tomorrow))))))))))))))) (. .)))
    -83.6156462512 -256.638395881
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WRB how) (JJ much) (NN manufacturing) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NNS reports)) (PP (IN on) (NP (NP (JJ industrial) (NN production)) (CC and) (NP (NN capacity) (NN utilization))))) (, ,) (NP (RB also) (JJ due) (NN tomorrow)))))))))))) (. .)))
    -83.6894917374 -256.297369974
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WHADJP (WRB how) (JJ much)) (NN manufacturing) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NNS reports)) (PP (IN on) (NP (NP (JJ industrial) (NN production)) (CC and) (NP (NN capacity) (NN utilization)))) (, ,) (RRC (ADVP (RB also)) (NP (JJ due) (NN tomorrow)))))))))))))) (. .)))
    -83.6959323955 -257.337579615
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WRB how) (JJ much) (NN manufacturing) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NNS reports)) (PP (IN on) (NP (NP (NP (JJ industrial) (NN production)) (CC and) (NP (NN capacity) (NN utilization))) (, ,) (NP (ADJP (RB also) (JJ due)) (NN tomorrow))))))))))))))) (. .)))
    -83.6993020024 -255.514465655
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WRB how) (JJ much) (NN manufacturing) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NNS reports)) (PP (IN on) (NP (NP (NP (JJ industrial) (NN production)) (CC and) (NP (NN capacity) (NN utilization))) (, ,) (NP (ADJP (ADVP (RB also)) (JJ due)) (NN tomorrow))))))))))))))) (. .)))
    -83.8169472184 -255.297537362
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WRB how) (JJ much) (NN manufacturing) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NNS reports)) (PP (IN on) (NP (NP (JJ industrial) (NN production)) (CC and) (NP (NN capacity) (NN utilization)))) (, ,) (RRC (ADVP (RB also)) (NP (JJ due) (NN tomorrow)))))))))))))) (. .)))
    -83.8977743747 -254.654118748
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WHADJP (WRB how) (JJ much)) (NN manufacturing) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NNS reports)) (PP (IN on) (NP (NP (JJ industrial) (NN production)) (CC and) (NP (NP (NN capacity) (NN utilization)) (PRN (, ,) (ADVP (RB also)) (NP (JJ due) (NN tomorrow))))))))))))))))) (. .)))
    -83.9520871226 -256.615389727
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WRB how) (JJ much) (NN manufacturing) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NNS reports)) (PP (IN on) (NP (NP (JJ industrial) (NN production)) (CC and) (NP (NN capacity) (NN utilization) (, ,) (RB also) (JJ due) (NN tomorrow))))))))))))))) (. .)))
    -84.0252298558 -253.654286136
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WRB how) (JJ much) (NN manufacturing) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NNS reports)) (PP (IN on) (NP (NP (JJ industrial) (NN production)) (CC and) (NP (NP (NN capacity) (NN utilization)) (PRN (, ,) (ADVP (RB also)) (NP (JJ due) (NN tomorrow))))))))))))))))) (. .)))
    -84.031467802 -256.699200765
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WHADJP (WRB how) (JJ much)) (NN manufacturing) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NNS reports)) (PP (IN on) (NP (NP (JJ industrial) (NN production)) (CC and) (NP (NN capacity) (NN utilization) (, ,) (ADJP (ADVP (RB also)) (JJ due)) (NN tomorrow))))))))))))))) (. .)))
    -84.0782566086 -255.763954352
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WHADVP (WRB how) (JJ much) (NN manufacturing)) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NP (NNS reports)) (PP (IN on) (NP (NP (JJ industrial) (NN production)) (CC and) (NP (NN capacity) (NN utilization))))) (, ,) (RB also) (NP (JJ due) (NN tomorrow))))))))))))) (. .)))
    -84.1589232831 -255.699368153
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WRB how) (JJ much) (NN manufacturing) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NNS reports)) (PP (IN on) (NP (NP (JJ industrial) (NN production)) (CC and) (NP (NN capacity) (NN utilization) (, ,) (ADJP (ADVP (RB also)) (JJ due)) (NN tomorrow))))))))))))))) (. .)))
    -84.207911268 -257.445803806
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WRB how) (JJ much) (NN manufacturing) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NNS reports)) (PP (IN on) (NP (NP (JJ industrial) (NN production)) (CC and) (NP (NN capacity) (NN utilization)))) (, ,) (ADVP (RB also)) (NP (JJ due) (NN tomorrow))))))))))))) (. .)))
    -84.2172200016 -256.757941126
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WRB how) (JJ much) (NN manufacturing) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NNS reports)) (PP (IN on) (NP (NP (NP (JJ industrial) (NN production)) (CC and) (NP (NN capacity) (NN utilization))) (, ,) (ADVP (RB also)) (NP (JJ due) (NN tomorrow))))))))))))))) (. .)))
    -84.4019196191 -257.117933507
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WHADVP (WRB how) (JJ much) (NN manufacturing)) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NNS reports)) (PP (IN on) (NP (NP (NP (JJ industrial) (NN production)) (CC and) (NP (NN capacity) (NN utilization))) (, ,) (RB also) (NP (JJ due) (NN tomorrow))))))))))))))) (. .)))
    -84.4086708847 -254.853466527
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WHADJP (WRB how) (JJ much)) (NN manufacturing) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NP (NNS reports)) (PP (IN on) (NP (NP (JJ industrial) (NN production)) (CC and) (NP (NN capacity) (NN utilization))))) (, ,) (RB also) (NP (JJ due) (NN tomorrow))))))))))))) (. .)))
    -84.4202693182 -257.522482005
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WRB how) (JJ much) (NN manufacturing) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NNS reports)) (PP (IN on) (NP (NP (JJ industrial) (NN production)) (CC and) (NP (NN capacity) (NN utilization) (, ,) (ADJP (RB also) (JJ due)) (NN tomorrow))))))))))))))) (. .)))
    -84.5361263658 -253.853633915
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WRB how) (JJ much) (NN manufacturing) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NP (NNS reports)) (PP (IN on) (NP (NP (JJ industrial) (NN production)) (CC and) (NP (NN capacity) (NN utilization))))) (, ,) (RB also) (NP (JJ due) (NN tomorrow))))))))))))) (. .)))
    -84.562221394 -257.037905456
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WRB how) (JJ much) (NN manufacturing) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NNS reports)) (PP (PP (IN on) (NP (NP (JJ industrial) (NN production)) (CC and) (NP (NN capacity) (NN utilization)))) (, ,) (RB also) (RB due) (NP (NN tomorrow)))))))))))))) (. .)))
    -84.7211610162 -257.10106261
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WHADVP (WRB how) (JJ much) (NN manufacturing)) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NNS reports)) (PP (IN on) (NP (NP (JJ industrial) (NN production)) (CC and) (NP (NN capacity) (NN utilization)))) (, ,) (ADVP (RB also) (NP (JJ due) (NN tomorrow)))))))))))))) (. .)))
    -84.7323339231 -256.207445682
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WHADJP (WRB how) (JJ much)) (NN manufacturing) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NNS reports)) (PP (IN on) (NP (NP (NP (JJ industrial) (NN production)) (CC and) (NP (NN capacity) (NN utilization))) (, ,) (RB also) (NP (JJ due) (NN tomorrow))))))))))))))) (. .)))
    -84.8597894042 -255.20761307
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WRB how) (JJ much) (NN manufacturing) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NNS reports)) (PP (IN on) (NP (NP (NP (JJ industrial) (NN production)) (CC and) (NP (NN capacity) (NN utilization))) (, ,) (RB also) (NP (JJ due) (NN tomorrow))))))))))))))) (. .)))
    -84.9655762188 -257.511603996
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WRB how) (JJ much) (NN manufacturing) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NNS reports)) (PP (IN on) (NP (NP (NP (JJ industrial) (NN production)) (CC and) (NP (NN capacity) (NN utilization))) (, ,) (ADVP (RB also) (JJ due)) (NP (NN tomorrow))))))))))))))) (. .)))
    -85.0357092054 -256.760509325
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WRB how) (JJ much) (NN manufacturing) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NNS reports)) (PP (IN on) (NP (NP (JJ industrial) (NX (NX (NN production)) (CC and) (NX (NN capacity) (NN utilization)))) (, ,) (NP (ADJP (ADVP (RB also)) (JJ due)) (NN tomorrow))))))))))))))) (. .)))
    -85.0404031204 -257.493696355
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WRB how) (JJ much) (NN manufacturing) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NP (NNP September) (NNS reports)) (PP (IN on) (NP (JJ industrial) (NN production) (CC and) (NN capacity))))) (NP (NN utilization) (, ,) (RB also) (JJ due) (NN tomorrow)))))))))))) (. .)))
    -85.0515753202 -256.190574785
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WHADJP (WRB how) (JJ much)) (NN manufacturing) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NNS reports)) (PP (IN on) (NP (NP (JJ industrial) (NN production)) (CC and) (NP (NN capacity) (NN utilization)))) (, ,) (ADVP (RB also) (NP (JJ due) (NN tomorrow)))))))))))))) (. .)))
    -85.1790308013 -255.190742173
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WRB how) (JJ much) (NN manufacturing) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NNS reports)) (PP (IN on) (NP (NP (JJ industrial) (NN production)) (CC and) (NP (NN capacity) (NN utilization)))) (, ,) (ADVP (RB also) (NP (JJ due) (NN tomorrow)))))))))))))) (. .)))
    -85.6747155736 -256.886955055
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WRB how) (JJ much) (NN manufacturing) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NNS reports)) (PP (IN on) (NP (NP (JJ industrial) (NN production)) (CC and) (NP (NN capacity) (NN utilization))))) (, ,) (PP (RB also) (NP (JJ due) (NN tomorrow))))))))))))) (. .)))
    -85.8137347183 -256.601249244
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (ADJP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WRB how) (JJ much) (NN manufacturing) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NNS reports)) (PP (IN on) (NP (NP (JJ industrial) (NN production)) (CC and) (NP (NP (NN capacity) (NN utilization)) (PRN (, ,) (ADVP (RB also)) (NP (JJ due) (NN tomorrow))))))))))))))))) (. .)))
    -85.8725230701 -257.453489352
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WHADJP (WRB how) (JJ much)) (NN manufacturing) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NNS reports)) (PP (IN on) (NP (NP (JJ industrial) (NX (NX (NN production)) (CC and) (NX (NN capacity) (NN utilization)))) (, ,) (RB also) (NP (JJ due) (NN tomorrow))))))))))))))) (. .)))
    -85.9999785232 -256.45365674
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WRB how) (JJ much) (NN manufacturing) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NNS reports)) (PP (IN on) (NP (NP (JJ industrial) (NX (NX (NN production)) (CC and) (NX (NN capacity) (NN utilization)))) (, ,) (RB also) (NP (JJ due) (NN tomorrow))))))))))))))) (. .)))
    -86.3246312283 -256.800597023
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (ADJP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WRB how) (JJ much) (NN manufacturing) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NP (NNS reports)) (PP (IN on) (NP (NP (JJ industrial) (NN production)) (CC and) (NP (NN capacity) (NN utilization))))) (, ,) (RB also) (NP (JJ due) (NN tomorrow))))))))))))) (. .)))
    -86.3311587192 -257.556264765
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WHADVP (WRB how) (JJ much) (NN manufacturing)) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NNS reports)) (PP (IN on) (NP (NP (JJ industrial) (NN production)) (CC and) (NP (NP (NN capacity)) (NP (NN utilization) (, ,) (ADJP (ADVP (RB also)) (JJ due)) (NN tomorrow)))))))))))))))) (. .)))
    -86.6615729953 -256.64577694
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WHADJP (WRB how) (JJ much)) (NN manufacturing) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NNS reports)) (PP (IN on) (NP (NP (JJ industrial) (NN production)) (CC and) (NP (NP (NN capacity)) (NP (NN utilization) (, ,) (ADJP (ADVP (RB also)) (JJ due)) (NN tomorrow)))))))))))))))) (. .)))
    -86.6783812339 -257.253941453
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WHADJP (WRB how) (JJ much)) (NN manufacturing) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NP (NNS reports)) (PP (IN on) (NP (JJ industrial) (NX (NX (NN production)) (CC and) (NX (NN capacity) (NN utilization)))))) (, ,) (RB also) (NP (JJ due) (NN tomorrow))))))))))))) (. .)))
    -86.7890284763 -255.645944328
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WRB how) (JJ much) (NN manufacturing) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NNS reports)) (PP (IN on) (NP (NP (JJ industrial) (NN production)) (CC and) (NP (NP (NN capacity)) (NP (NN utilization) (, ,) (ADJP (ADVP (RB also)) (JJ due)) (NN tomorrow)))))))))))))))) (. .)))
    -86.805836687 -256.254108841
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WRB how) (JJ much) (NN manufacturing) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NP (NNS reports)) (PP (IN on) (NP (JJ industrial) (NX (NX (NN production)) (CC and) (NX (NN capacity) (NN utilization)))))) (, ,) (RB also) (NP (JJ due) (NN tomorrow))))))))))))) (. .)))
    -86.8163694052 -257.300308221
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WHADVP (WRB how) (JJ much) (NN manufacturing)) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NNS reports)) (PP (IN on) (NP (NP (JJ industrial) (NN production) (CC and) (NN capacity)) (NP (NN utilization) (, ,) (ADJP (ADVP (RB also)) (JJ due)) (NN tomorrow))))))))))))))) (. .)))
    -86.8234508895 -257.469058181
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WRB how) (JJ much) (NN manufacturing) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NNS reports)) (PP (IN on) (NP (NP (JJ industrial) (NN production)) (CC and) (NP (NP (NN capacity)) (NP (NN utilization) (, ,) (ADJP (RB also) (JJ due)) (NN tomorrow)))))))))))))))) (. .)))
    -86.8276269607 -256.807156413
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WRB how) (JJ much) (NN manufacturing) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NNS reports)) (PP (IN on) (NP (NP (JJ industrial) (NX (NX (NN production)) (CC and) (NX (NN capacity) (NN utilization)))) (PRN (, ,) (ADVP (RB also)) (NP (JJ due) (NN tomorrow)))))))))))))))) (. .)))
    -87.1467837093 -256.389820397
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WHADJP (WRB how) (JJ much)) (NN manufacturing) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NNS reports)) (PP (IN on) (NP (NP (JJ industrial) (NN production) (CC and) (NN capacity)) (NP (NN utilization) (, ,) (ADJP (ADVP (RB also)) (JJ due)) (NN tomorrow))))))))))))))) (. .)))
    -87.2742391903 -255.389987785
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WRB how) (JJ much) (NN manufacturing) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NNS reports)) (PP (IN on) (NP (NP (JJ industrial) (NN production) (CC and) (NN capacity)) (NP (NN utilization) (, ,) (ADJP (ADVP (RB also)) (JJ due)) (NN tomorrow))))))))))))))) (. .)))
    -87.3086615755 -257.213101637
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WRB how) (JJ much) (NN manufacturing) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NNS reports)) (PP (IN on) (NP (NP (JJ industrial) (NN production) (CC and) (NN capacity)) (NP (NN utilization) (, ,) (ADJP (RB also) (JJ due)) (NN tomorrow))))))))))))))) (. .)))
    >>> print(nbest_list_complex.fuse())
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WRB how) (JJ much) (NN manufacturing) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NNS reports)) (PP (IN on) (NP (NP (NP (JJ industrial) (NN production)) (CC and) (NP (NN capacity) (NN utilization))) (, ,) (ADVP (RB also)) (JJ due) (NN tomorrow)))))))))))))) (. .)))
    >>> print(nbest_list_complex.fuse(use_parser_scores=True))
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WRB how) (JJ much) (NN manufacturing) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NNS reports)) (PP (IN on) (NP (NP (NP (JJ industrial) (NN production)) (CC and) (NP (NN capacity) (NN utilization))) (, ,) (ADVP (RB also)) (NP (JJ due) (NN tomorrow))))))))))))))) (. .)))
    >>> print(nbest_list_complex.fuse(num_parses=10))
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WRB how) (JJ much) (NN manufacturing) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NNS reports)) (PP (IN on) (NP (NP (NP (JJ industrial) (NN production)) (CC and) (NP (NN capacity) (NN utilization))) (, ,) (NP (ADJP (ADVP (RB also)) (JJ due)) (NN tomorrow))))))))))))))) (. .)))
    >>> print(nbest_list_complex.fuse(num_parses=10, threshold=0.75))
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WRB how) (JJ much) (NN manufacturing) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NNS reports)) (IN on) (NP (JJ industrial) (NN production)) (CC and) (NP (NN capacity) (NN utilization)) (, ,) (ADVP (RB also)) (JJ due) (NN tomorrow))))))))))) (. .)))
    >>> print(nbest_list_complex.fuse(num_parses=10, threshold=0.75, exponent=1.3))
    (S1 (S (NP (NNS Economists)) (VP (AUX are) (VP (VBN divided) (PP (IN as) (PP (TO to) (SBAR (WHNP (WRB how) (JJ much) (NN manufacturing) (NN strength)) (S (NP (PRP they)) (VP (VBP expect) (S (VP (TO to) (VP (VB see) (PP (IN in) (NP (NNP September))) (NP (NP (NNS reports)) (IN on) (NP (JJ industrial) (NN production)) (CC and) (NP (NN capacity) (NN utilization)) (, ,) (ADVP (RB also)) (JJ due) (NN tomorrow)))))))))))) (. .)))
    """

def test_tree_eval():
    """
    This is here and not in test_tree since it requires a parsing model
    to be loaded.

    >>> import bllipparser
    >>> tree1 = bllipparser.Tree('(S1 (S (NP (DT This)) (VP (AUX is) (NP (DT a) (NN sentence))) (. .)))')
    >>> tree2 = bllipparser.Tree('(S1 (S (NP (NNP This)) (VP (AUX is) (NP (DT a) (NN sentence))) (. .)))')
    >>> tree3 = bllipparser.Tree('(S1 (SBARQ (WHNP (DT This)) (SQ (VP (AUX is) (NP (DT a) (NN sentence)))) (. .)))')
    >>> eval1 = tree1.evaluate(tree2)
    >>> print(stringify_dict(eval1))
    fscore=1.00, gold=4, matched=4, precision=1.00, recall=1.00, test=4
    >>> eval2 = tree2.evaluate(tree1)
    >>> print(stringify_dict(eval2))
    fscore=1.00, gold=4, matched=4, precision=1.00, recall=1.00, test=4
    >>> eval3 = tree3.evaluate(tree1)
    >>> print(stringify_dict(eval3))
    fscore=0.44, gold=4, matched=2, precision=0.40, recall=0.50, test=5
    """
