The BLLIP parser (also known as the Charniak-Johnson parser or
Brown Reranking Parser) is described in the paper `Charniak
and Johnson (Association of Computational Linguistics, 2005)
<http://aclweb.org/anthology/P/P05/P05-1022.pdf>`_.  This package provides
the BLLIP parser runtime along with a Python interface. Note that it
does not come with any parsing models but includes a downloader.
The primary maintenance for the parser takes place at `GitHub
<http://github.com/BLLIP/bllip-parser>`_.

Fetching parsing models
-----------------------

Before you can parse, you'll need some parsing models.  ``ModelFetcher``
will help you download and install parsing models.  It can be invoked
from the command line. For example, this will download and install the
standard WSJ model::

    shell% python -mbllipparser.ModelFetcher -i WSJ

Run ``python -mbllipparser.ModelFetcher`` with no arguments for a full
listing of options and available parsing models. It can also be invoked
as a Python library::

    >>> from bllipparser.ModelFetcher import download_and_install_model
    >>> download_and_install_model('WSJ', '/tmp/models')
    /tmp/models/WSJ

In this case, it would download WSJ and install it to
``/tmp/models/WSJ``. Note that it returns the path to the downloaded
model.

Basic usage
-----------

The easiest way to construct a parser is with the
``from_unified_model_dir`` class method. A unified model is a directory
that contains two subdirectories: ``parser/`` and ``reranker/``, each
with the respective model files::

    >>> from bllipparser import RerankingParser, tokenize
    >>> rrp = RerankingParser.from_unified_model_dir('/path/to/model/')

This can be integrated with ModelFetcher (if the model is already
installed, ``download_and_install_model`` is a no-op)::

    >>> model_dir = download_and_install_model('WSJ', '/tmp/models')
    >>> rrp = RerankingParser.from_unified_model_dir(model_dir)

You can also load parser and reranker models manually::

    >>> rrp = RerankingParser()
    >>> rrp.load_parser_model('/tmp/models/WSJ/parser')
    >>> rrp.load_reranker_model('/tmp/models/WSJ/reranker')

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

You can also parse text with existing POS tags (these act as soft
constraints). In this example, token 0 ('Time') should have tag VB and
token 1 ('flies') should have tag NNS::

    >>> rrp.parse_tagged(['Time', 'flies'], possible_tags={0 : 'VB', 1 : 'NNS'})[0]
    ScoredParse('(S1 (NP (VB Time) (NNS flies)))', parser_score=-53.94938875760073, reranker_score=-15.841407102717749)

You don't need to specify a tag for all words: Here, token 0 ('Time') should
have tag VB and token 1 ('flies') is unconstrained::

    >>> rrp.parse_tagged(['Time', 'flies'], possible_tags={0 : 'VB'})[0]
    ScoredParse('(S1 (S (VP (VB Time) (NP (VBZ flies)))))', parser_score=-54.390430751112156, reranker_score=-17.290145080887005)

You can specify multiple tags for each token. When you do this, the
tags for a token will be used in decreasing priority. token 0 ('Time')
should have tag VB, JJ, or NN and token 1 ('flies') is unconstrained::

    >>> rrp.parse_tagged(['Time', 'flies'], possible_tags={0 : ['VB', 'JJ', 'NN']})[0]
    ScoredParse('(S1 (NP (NN Time) (VBZ flies)))', parser_score=-42.82904107213723, reranker_score=-12.865900776775314)

There are many parser options which can be adjusted (though the defaults
should work well for most cases) with ``set_parser_options``. This
will change the size of the n-best list and pick the defaults for all
other options. It returns a dictionary of the current options::

    >>> rrp.set_parser_options(nbest=10)
    {'language': 'En', 'case_insensitive': False, 'debug': 0, 'small_corpus': True, 'overparsing': 21, 'smooth_pos': 0, 'nbest': 10}
    >>> nbest_list = rrp.parse('The list is smaller now.', rerank=False)
    >>> len(nbest_list)
    10

Use this if all you want is a tokenizer::

    >>> tokenize("Tokenize this sentence, please.")
    ['Tokenize', 'this', 'sentence', ',', 'please', '.']

Parsing shell
-------------

There is an interactive shell which can help visualize a parse::

    shell% python -mbllipparser.ParsingShell /path/to/model

Once in the shell, type a sentence to have the parser parse it::

    rrp> I saw the astronomer with the telescope.
    Tokens: I saw the astronomer with the telescope .

    Parser's parse:
    (S1 (S (NP (PRP I))
         (VP (VBD saw)
          (NP (NP (DT the) (NN astronomer))
           (PP (IN with) (NP (DT the) (NN telescope)))))
         (. .)))

    Reranker's parse: (parser index 2)
    (S1 (S (NP (PRP I))
         (VP (VBD saw)
          (NP (DT the) (NN astronomer))
          (PP (IN with) (NP (DT the) (NN telescope))))
         (. .)))

If you have ``nltk`` installed, you can use its tree visualization to
see the output::

    rrp> visual Show me this parse.
    Tokens: Show me this parse .

    [graphical display of the parse appears]

There is more detailed help inside the shell under the ``help`` command.
