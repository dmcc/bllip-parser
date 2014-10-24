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

"""
Python frontend to the BLLIP natural language parser.

The BLLIP parser (also known as the Charniak-Johnson parser or
Brown Reranking Parser) is described in the paper `Charniak
and Johnson (Association of Computational Linguistics, 2005)
<http://aclweb.org/anthology/P/P05/P05-1022.pdf>`_.  This package
provides the BLLIP parser runtime along with a Python interface. Note
that it does not come with any parsing models but includes a model
downloader.  The primary maintenance for the parser takes place at
`GitHub <http://github.com/BLLIP/bllip-parser>`_.

We request acknowledgement in any publications that make use of this
software and any code derived from this software. Please report the
release date of the software that you are using, as this will enable
others to compare their results to yours.

References:

* Eugene Charniak and Mark Johnson. "Coarse-to-fine n-best parsing and
  MaxEnt discriminative reranking." Proceedings of the 43rd Annual Meeting
  on Association for Computational Linguistics. Association for
  Computational Linguistics, 2005.
  http://aclweb.org/anthology/P/P05/P05-1022.pdf

* Eugene Charniak. "A maximum-entropy-inspired parser." Proceedings of
  the 1st North American chapter of the Association for Computational
  Linguistics conference. Association for Computational Linguistics, 2000.
  http://aclweb.org/anthology/A/A00/A00-2018.pdf

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
    >>> rrp.load_reranker_model('/tmp/models/WSJ/reranker/features.gz', '/tmp/models/WSJ/reranker/weights.gz')

If you only want the top parse of a sentence in Penn Treebank format, use
the ``simple_parse()`` method::

    >>> rrp.simple_parse('This is simple.')
    '(S1 (S (NP (DT This)) (VP (VBZ is) (ADJP (JJ simple))) (. .)))'

If you want more information about the parse, you'll want to use the
``parse()`` method which returns an ``NBestList`` object.  The parser
produces an *n-best list* of the *n* most likely parses of the sentence
(default: *n=50*). Typically you only want the top parse, but the others
are available as well::

    >>> nbest_list = rrp.parse('This is a sentence.')

To get information about the top parse (note that the ptb_parse property
is a Tree object, described later)::

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

The parser can also be used as a tagger::

    >>> rrp.tag("Time flies while you're having fun.")
    [('Time', 'NNP'), ('flies', 'VBZ'), ('while', 'IN'), ('you', 'PRP'), ("'re", 'VBP'), ('having', 'VBG'), ('fun', 'NN'), ('.', '.')]

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

The Tree class
--------------

The parser provides a simple (immutable) Tree class which provides
information about Penn Treebank-style trees::

    >>> tree = bllipparser.Tree('(S1 (S (NP (DT This)) (VP (VBZ is) (NP (DT a) (ADJP (RB fairly) (JJ simple)) (NN parse) (NN tree))) (. .)))')
    >>> print tree
    (S1 (S (NP (DT This)) (VP (VBZ is) (NP (DT a) (ADJP (RB fairly) (JJ simple)) (NN parse) (NN tree))) (. .)))

``pretty_string()`` provides a line-wrapped stringification::

    >>> print tree.pretty_string()
    (S1 (S (NP (DT This))
         (VP (VBZ is)
          (NP (DT a) (ADJP (RB fairly) (JJ simple)) (NN parse) (NN tree)))
         (. .)))

You can obtain the tokens and tags of the tree::

    >>> print tree.tokens()
    ('This', 'is', 'a', 'fairly', 'simple', 'parse', 'tree', '.')
    >>> print tree.tags()
    ('DT', 'VBZ', 'DT', 'RB', 'JJ', 'NN', 'NN', '.')
    >>> print tree.tokens_and_tags()
    [('This', 'DT'), ('is', 'VBZ'), ('a', 'DT'), ('fairly', 'RB'), ('simple', 'JJ'), ('parse', 'NN'), ('tree', 'NN'), ('.', '.')]

Or get information about the labeled spans in the tree::

    >>> print tree.span()
    (0, 8)
    >>> print tree.label()
    S1

And finally navigate within the trees::

    >>> tree.subtrees()
    [bllipparser.RerankingParser.Tree('(S (NP (DT This)) (VP (VBZ is) (NP (DT a) (ADJP (RB fairly) (JJ simple)) (NN parse) (NN tree))) (. .))')]
    >>> tree.subtrees()[0].label()
    'S'
    >>> tree.subtrees()[0].subtrees()[0]
    bllipparser.RerankingParser.Tree('(NP (DT This))')
    >>> tree.subtrees()[0].subtrees()[0].label()
    'NP'
    >>> tree.subtrees()[0].subtrees()[0].span()
    (0, 1)
    >>> tree.subtrees()[0].subtrees()[0].tags()
    ('DT',)
    >>> tree.subtrees()[0].subtrees()[0].tokens()
    ('This',)
    >>> len(tree.subtrees()[0]) # number of subtrees
    3
    >>> for subtree in tree.subtrees()[0]:
    ...    print subtree
    ... 
    (NP (DT This))
    (VP (VBZ is) (NP (DT a) (ADJP (RB fairly) (JJ simple)) (NN parse) (NN tree)))
    (. .)
"""

from RerankingParser import RerankingParser, Tree, tokenize

__authors__ = 'Eugene Charniak, Mark Johnson, David McClosky, many others'
__license__ = 'Apache 2.0'
__version__ = '2014.08.29b'
__maintainer__ = 'David McClosky'
__email__ = 'notsoweird+pybllipparser@gmail.com'
