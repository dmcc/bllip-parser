The BLLIP parser (also known as the Charniak-Johnson parser or
Brown Reranking Parser) is described in the paper `Charniak
and Johnson (Association of Computational Linguistics, 2005)
<http://aclweb.org/anthology/P/P05/P05-1022.pdf>`_.  This code
provides a Python interface to the parser. Note that it does
not contain any parsing models which must be downloaded
separately (for example, `WSJ self-trained parsing model
<http://cs.brown.edu/~dmcc/selftraining/selftrained.tar.gz>`_).
The primary maintenance for the parser takes place at `GitHub
<http://github.com/BLLIP/bllip-parser>`_.

Basic usage
-----------

The easiest way to construct a parser is with the
``load_unified_model_dir`` class method. A unified model is a directory
that contains two subdirectories: ``parser/`` and ``reranker/``, each
with the respective model files::

    >>> from bllipparser import RerankingParser, tokenize
    >>> rrp = RerankingParser.load_unified_model_dir('/path/to/model/')

Parsing a single sentence and reading information about the top parse
with ``parse()``. The parser produces an *n-best list* of the *n* most
likely parses of the sentence (default: *n=50*). Typically you only want
the top parse, but the others are available as well::

    >>> nbest_list = rrp.parse('This is a sentence.')

Getting information about the top parse::

    >>> print repr(nbest_list[0])
    ScoredParse('(S1 (S (NP (DT This)) (VP (VBZ is) (NP (DT a) (NN sentence))) (. .)))', parser_score=-29.621201629004183, reranker_score=-7.9273829816098731)
    >>> print nbest_list[0].ptb_parse
    (S1 (S (NP (DT This)) (VP (VBZ is) (NP (DT a) (NN sentence))) (. .)))
    >>> print nbest_list[0].parser_score
    -29.621201629
    >>> print nbest_list[0].reranker_score
    -7.92738298161
    >>> print len(nbest_list)
    50

If you have an existing tokenizer, tokenization can also be specified
by passing a list of strings::

    >>> nbest_list = rrp.parse(['This', 'is', 'a', 'pretokenized', 'sentence', '.'])

The reranker can be disabled by setting ``rerank=False``::

    >>> nbest_list = rrp.parse('Parser only!', rerank=False)

Parsing text with existing POS tag (soft) constraints. In this example,
token 0 ('Time') should have tag VB and token 1 ('flies') should have
tag NNS::

    >>> rrp.parse_tagged(['Time', 'flies'], possible_tags={0 : 'VB', 1 : 'NNS'})[0]
    ScoredParse('(S1 (NP (VB Time) (NNS flies)))', parser_score=-53.94938875760073, reranker_score=-15.841407102717749)

You don't need to specify a tag for all words: token 0 ('Time') should
have tag VB and token 1 ('flies') is unconstrained::

    >>> rrp.parse_tagged(['Time', 'flies'], possible_tags={0 : 'VB'})[0]
    ScoredParse('(S1 (S (VP (VB Time) (NP (VBZ flies)))))', parser_score=-54.390430751112156, reranker_score=-17.290145080887005)

You can specify multiple tags for each token: token 0 ('Time') should
have tag VB, JJ, or NN and token 1 ('flies') is unconstrained::

    >>> rrp.parse_tagged(['Time', 'flies'], possible_tags={0 : ['VB', 'JJ', 'NN']})[0]
    ScoredParse('(S1 (NP (NN Time) (VBZ flies)))', parser_score=-42.82904107213723, reranker_score=-12.865900776775314)

Use this if all you want is a tokenizer::

    >>> tokenize("Tokenize this sentence, please.")
    ['Tokenize', 'this', 'sentence', ',', 'please', '.']
