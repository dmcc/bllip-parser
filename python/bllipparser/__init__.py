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

Basic usage:

The easiest way to construct a parser is with the load_unified_model_dir
class method. A unified model is a directory that contains two
subdirectories: parser/ and reranker/, each with the respective model
files:
>>> from bllipparser import RerankingParser, tokenize
>>> rrp = RerankingParser.load_unified_model_dir('/path/to/model/')

Parsing a single sentence and reading the result:
>>> nbest_list = rrp.parse('This is a sentence.')
>>> print len(nbest_list)
50
>>> print repr(nbest_list[0])
ScoredParse('(S1 (S (NP (DT This)) (VP (VBZ is) (NP (DT a) (NN sentence))) (. .)))', parser_score=-29.621201629004183, reranker_score=-7.9273829816098731)
>>> print nbest_list[0].parser_score
-29.621201629
>>> print nbest_list[0].reranker_score
-7.92738298161
>>> print nbest_list[0].ptb_parse
(S1 (S (NP (DT This)) (VP (VBZ is) (NP (DT a) (NN sentence))) (. .)))

Tokenization can also be specified by passing a list of strings:
>>> nbest_list = rrp.parse(['This', 'is', 'a', 'pretokenized', 'sentence', '.'])

The reranker can be disabled by setting rerank=False:
>>> nbest_list = rrp.parse('Parser only!', rerank=False)

Parsing text with existing POS tag (soft) constraints. In this example,
Token 0 ('Time') should have tag VB
Token 1 ('flies') should have tag NNS
>>> rrp.parse_tagged(['Time', 'flies'], possible_tags={0 : 'VB', 1 : 'NNS'})[0]
ScoredParse('(S1 (NP (VB Time) (NNS flies)))', parser_score=-53.94938875760073, reranker_score=-15.841407102717749)

You don't need to specify a tag for all words:
Token 0 ('Time') should have tag VB
Token 1 ('flies') is unconstrained
>>> rrp.parse_tagged(['Time', 'flies'], possible_tags={0 : 'VB'})[0]
ScoredParse('(S1 (S (VP (VB Time) (NP (VBZ flies)))))', parser_score=-54.390430751112156, reranker_score=-17.290145080887005)

You can specify multiple tags for each token:
Token 0 ('Time') should have tag VB, JJ, or NN
Token 1 ('flies') is unconstrained
>>> rrp.parse_tagged(['Time', 'flies'], possible_tags={0 : ['VB', 'JJ', 'NN']})[0]
ScoredParse('(S1 (NP (NN Time) (VBZ flies)))', parser_score=-42.82904107213723, reranker_score=-12.865900776775314)

Use this if all you want is a tokenizer:
>>> tokenize("Tokenize this sentence, please.")
['Tokenize', 'this', 'sentence', ',', 'please', '.']
"""

from RerankingParser import RerankingParser, tokenize
