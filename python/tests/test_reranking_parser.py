# TODO: untested:
#   NBestList.sort_by_reranker_scores, sort_by_parser_scores,
#       get_parser_best, get_reranker_best, tokens, rerank,
#       as_reranker_input
#   Sentence.sentences_from_file, Tree.trees_from_file
#   model downloading
#   RerankingParser.from_unified_model_dir, parse_tagged with
#       invalid tags, set_parser_options

def test_reranking_parser_sentences():
    """
    >>> from bllipparser import Sentence
    >>> s = Sentence('Hi there.')
    >>> s
    bllipparser.RerankingParser.Sentence(['Hi', 'there', '.'])
    >>> s.tokens()
    ['Hi', 'there', '.']
    >>> len(s)
    3
    >>> sentences = Sentence.sentences_from_string('<s> Test </s>')
    >>> sentences
    [bllipparser.RerankingParser.Sentence(['Test'])]
    >>> sentences[0]
    bllipparser.RerankingParser.Sentence(['Test'])
    >>> sentences[0].tokens()
    ['Test']
    >>> sentences[0].tokens()[0]
    'Test'
    >>> sentences2 = Sentence.sentences_from_string('''<s> Sentence 1 </s>
    ... <s> Can't have just one. </s>
    ... <s last> The last sentence </s>
    ... <s> Just kidding. </s>''')
    >>> sentences2
    [bllipparser.RerankingParser.Sentence(['Sentence', '1']), bllipparser.RerankingParser.Sentence(['Can', "n't", 'have', 'just', 'one', '.']), bllipparser.RerankingParser.Sentence(['The', 'last', 'sentence']), bllipparser.RerankingParser.Sentence(['Just', 'kidding', '.'])]
    >>> for s in sentences2:
    ...     print s.tokens()
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

def test_reranking_parser_basics():
    """
    >>> from bllipparser import RerankingParser
    >>> rrp = RerankingParser()
    >>> rrp.load_parser_model('first-stage/DATA/EN')
    >>> rrp.load_reranker_model('second-stage/models/ec50spfinal/features.gz',
    ...                         'second-stage/models/ec50spfinal/cvlm-l1c10P1-weights.gz')
    >>> rrp.parser_model_dir
    'first-stage/DATA/EN'
    >>> rrp.simple_parse('This is simple.')
    '(S1 (S (NP (DT This)) (VP (AUX is) (ADJP (JJ simple))) (. .)))'
    >>> nbest_list = rrp.parse('This is a sentence.')
    >>> print str(nbest_list).strip()
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
    >>> print repr(nbest_list[0])
    ScoredParse('(S1 (S (NP (DT This)) (VP (AUX is) (NP (DT a) (NN sentence))) (. .)))', parser_score=-30.398166970084645, reranker_score=-8.8865584560790545)
    >>> print nbest_list[0].ptb_parse
    (S1 (S (NP (DT This)) (VP (AUX is) (NP (DT a) (NN sentence))) (. .)))
    >>> print nbest_list[0].parser_score
    -30.3981669701
    >>> print nbest_list[0].reranker_score
    -8.88655845608
    >>> print len(nbest_list)
    13
    >>> nbest_list2 = rrp.parse(['This', 'is', 'a', 'pretokenized', 'sentence', '.'])
    >>> print str(nbest_list2).strip()
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
    >>> print str(nbest_list3).strip()
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
    >>> rrp.parse_tagged(['Time', 'flies'], possible_tags={})[0]
    ScoredParse('(S1 (S (NP (NNP Time)) (VP (VBZ flies))))', parser_score=-39.010948025063293, reranker_score=-10.642253171985915)
    >>> rrp.parse_tagged(['Time', 'flies'], possible_tags={0 : 'VB', 1 : 'NNS'})[0]
    ScoredParse('(S1 (NP (VB Time) (NNS flies)))', parser_score=-53.688744922676996, reranker_score=-15.064250357300528)
    >>> rrp.parse_tagged(['Time', 'flies'], possible_tags={0 : 'VB'})[0]
    ScoredParse('(S1 (S (VP (VB Time) (NP (VBZ flies)))))', parser_score=-53.241983495572825, reranker_score=-16.44603321313846)
    >>> rrp.parse_tagged(['Time', 'flies'], possible_tags={0 : ['VB', 'JJ', 'NN']})
    [ScoredParse('(S1 (NP (NN Time) (VBZ flies)))', parser_score=-41.799276750707349, reranker_score=-12.290348005904244), ScoredParse('(S1 (S (NP (NN Time)) (VP (VBZ flies))))', parser_score=-43.713432522271532, reranker_score=-13.061501758756112), ScoredParse('(S1 (FRAG (NP (NN Time)) (VP (VBZ flies))))', parser_score=-45.030128088700536, reranker_score=-14.771569695561784), ScoredParse('(S1 (NP (NP (NN Time)) (SBAR (S (VP (VBZ flies))))))', parser_score=-48.363308438597869, reranker_score=-15.139140564917611), ScoredParse('(S1 (NP (NP (NN Time)) (VP (VBZ flies))))', parser_score=-54.188659798409425, reranker_score=-15.537890766473778), ScoredParse('(S1 (NP (JJ Time) (VBZ flies)))', parser_score=-57.591972795009568, reranker_score=-15.88126406487681), ScoredParse('(S1 (FRAG (NP (NN Time) (VBZ flies))))', parser_score=-47.609110399076968, reranker_score=-15.976749434482407), ScoredParse('(S1 (NP (NN Time) (SBAR (S (VP (VBZ flies))))))', parser_score=-51.237835960503837, reranker_score=-16.158826030118739), ScoredParse('(S1 (FRAG (NP (NN Time)) (S (VP (VBZ flies)))))', parser_score=-47.320971445503858, reranker_score=-16.397963304069904), ScoredParse('(S1 (S (VP (VB Time) (NP (VBZ flies)))))', parser_score=-53.241983495572825, reranker_score=-16.44603321313846), ScoredParse('(S1 (FRAG (NP (NP (NN Time)) (SBAR (S (VP (VBZ flies)))))))', parser_score=-48.400117951579844, reranker_score=-16.623763976961417), ScoredParse('(S1 (S (VP (NN Time) (VP (VBZ flies)))))', parser_score=-51.462884906143714, reranker_score=-16.695185269741167), ScoredParse('(S1 (S (NP (VB Time)) (VP (VBZ flies))))', parser_score=-53.189140561020679, reranker_score=-16.720568824101335), ScoredParse('(S1 (VP (VB Time) (NP (VBZ flies))))', parser_score=-54.648302647300845, reranker_score=-16.976082226117118), ScoredParse('(S1 (S (NP (NP (NN Time)) (SBAR (S (VP (VBZ flies)))))))', parser_score=-48.58200284575706, reranker_score=-17.080097699803304), ScoredParse('(S1 (S (NP (JJ Time)) (VP (VBZ flies))))', parser_score=-56.162684713629119, reranker_score=-17.159492275854973), ScoredParse('(S1 (NP (NN Time) (S (VP (VBZ flies)))))', parser_score=-55.54108474281648, reranker_score=-17.202077602352922), ScoredParse('(S1 (S (ADVP (NN Time)) (VP (VBZ flies))))', parser_score=-50.715370545327424, reranker_score=-17.348012021431988), ScoredParse('(S1 (S (VP (NN Time) (NP (VBZ flies)))))', parser_score=-54.406980695205654, reranker_score=-17.511441562331648), ScoredParse('(S1 (S (ADJP (JJ Time)) (VP (VBZ flies))))', parser_score=-57.350979878268113, reranker_score=-17.720692686988237), ScoredParse('(S1 (FRAG (NP (NP (NN Time)) (VP (VBZ flies)))))', parser_score=-56.620064253613215, reranker_score=-17.777273001066742), ScoredParse('(S1 (S (NP (NN Time) (VBZ flies))))', parser_score=-52.233137616611593, reranker_score=-17.878821143104197), ScoredParse('(S1 (FRAG (VP (NN Time) (VP (VBZ flies)))))', parser_score=-53.446303213126768, reranker_score=-18.098260950978485), ScoredParse('(S1 (NP (NP (VB Time)) (VP (VBZ flies))))', parser_score=-59.536204000134695, reranker_score=-18.175189312924005), ScoredParse('(S1 (S (VP (VB Time) (VP (VBZ flies)))))', parser_score=-58.374138550442545, reranker_score=-18.347782906440006), ScoredParse('(S1 (S (VP (NN Time) (S (VP (VBZ flies))))))', parser_score=-55.461343468990314, reranker_score=-18.418080898900527), ScoredParse('(S1 (FRAG (S (NP (NN Time)) (VP (VBZ flies)))))', parser_score=-53.687449574227813, reranker_score=-18.470861676679654), ScoredParse('(S1 (S (VP (VB Time) (SBAR (S (VP (VBZ flies)))))))', parser_score=-54.264099191474024, reranker_score=-18.495469047355858), ScoredParse('(S1 (S (ADJP (NN Time) (S (VP (VBZ flies))))))', parser_score=-52.823665366261849, reranker_score=-18.637269048689451), ScoredParse('(S1 (VP (NN Time) (NP (VBZ flies))))', parser_score=-59.36712223843476, reranker_score=-18.686108380765404), ScoredParse('(S1 (S (VP (VB Time) (S (VP (VBZ flies))))))', parser_score=-57.834663624120637, reranker_score=-18.757393068810185), ScoredParse('(S1 (S (VP (ADVP (NN Time)) (VBZ flies))))', parser_score=-52.149064292582104, reranker_score=-18.911220009763934), ScoredParse('(S1 (FRAG (VP (VB Time) (NP (VBZ flies)))))', parser_score=-58.156778377657858, reranker_score=-19.017879810791761), ScoredParse('(S1 (S (S (NP (NN Time))) (VP (VBZ flies))))', parser_score=-53.658777416948432, reranker_score=-19.102375852958001), ScoredParse('(S1 (NP (NP (VB Time)) (SBAR (S (VP (VBZ flies))))))', parser_score=-59.021606356675655, reranker_score=-19.141928396961156), ScoredParse('(S1 (S (VP (NN Time) (SBAR (S (VP (VBZ flies)))))))', parser_score=-54.940965391950201, reranker_score=-19.157554783588044), ScoredParse('(S1 (S (NP (NP (NN Time)) (VP (VBZ flies)))))', parser_score=-59.328773475995142, reranker_score=-19.189850371029841), ScoredParse('(S1 (FRAG (ADJP (NN Time) (S (VP (VBZ flies))))))', parser_score=-52.889107334336742, reranker_score=-19.19517170709419), ScoredParse('(S1 (S (ADVP (JJ Time)) (VP (VBZ flies))))', parser_score=-58.032706810762406, reranker_score=-19.252547770330068), ScoredParse('(S1 (VP (VB Time) (S (VP (VBZ flies)))))', parser_score=-59.412712708823911, reranker_score=-19.277564425622955), ScoredParse('(S1 (FRAG (VP (ADVP (NN Time)) (VBZ flies))))', parser_score=-52.542021922149438, reranker_score=-19.372767083887499), ScoredParse('(S1 (S (S (ADJP (JJ Time))) (VP (VBZ flies))))', parser_score=-59.595990898156209, reranker_score=-19.442376990927869), ScoredParse('(S1 (FRAG (ADJP (JJ Time)) (S (VP (VBZ flies)))))', parser_score=-58.203554356941297, reranker_score=-19.509295668849127), ScoredParse('(S1 (VP (VB Time) (SBAR (S (VP (VBZ flies))))))', parser_score=-56.927554229724798, reranker_score=-19.541880177158077), ScoredParse('(S1 (FRAG (VP (NN Time) (NP (VBZ flies)))))', parser_score=-58.650853878582339, reranker_score=-19.556031335582261), ScoredParse('(S1 (FRAG (NP (NN Time) (SBAR (S (VP (VBZ flies)))))))', parser_score=-57.678063257696081, reranker_score=-19.801118379535033), ScoredParse('(S1 (FRAG (S (VP (NN Time) (VP (VBZ flies))))))', parser_score=-57.037775623775175, reranker_score=-19.943300352182188), ScoredParse('(S1 (S (VP (ADVP (NN Time)) (VP (VBZ flies)))))', parser_score=-59.452255293251611, reranker_score=-20.875148959584919), ScoredParse('(S1 (FRAG (VP (ADVP (NN Time)) (VP (VBZ flies)))))', parser_score=-59.577013382772229, reranker_score=-21.337894458276729), ScoredParse('(S1 (FRAG (S (VP (ADVP (NN Time)) (VBZ flies)))))', parser_score=-57.064705447641735, reranker_score=-21.974790706423903)]
    >>> rrp.set_parser_options(nbest=10)
    {'language': 'En', 'case_insensitive': False, 'debug': 0, 'small_corpus': True, 'overparsing': 21, 'smooth_pos': 0, 'nbest': 10}
    >>> nbest_list = rrp.parse('The list is smaller now.', rerank=False)
    >>> len(nbest_list)
    10
    >>> rrp.tag("Time flies while you're having fun.")
    [('Time', 'NNP'), ('flies', 'VBZ'), ('while', 'IN'), ('you', 'PRP'), ("'re", 'AUX'), ('having', 'AUXG'), ('fun', 'NN'), ('.', '.')]
    >>> rrp.tag('British left waffles on Falklands .'.split())
    [('British', 'JJ'), ('left', 'NN'), ('waffles', 'VBZ'), ('on', 'IN'), ('Falklands', 'NNP'), ('.', '.')]
    """
