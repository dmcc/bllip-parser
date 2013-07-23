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

import sys

# this makes this work without modifications to PYTHONPATH in the swig/
# or the base directory
sys.path.extend(['../first-stage/PARSE/swig/python/lib',
                 '../second-stage/programs/features/swig/python/lib',
                 'first-stage/PARSE/swig/python/lib',
                 'second-stage/programs/features/swig/python/lib'])

try:
    import SWIGParser as parser
    import SWIGReranker as reranker
except ImportError:
    print "Couldn't find SWIG bindings for parser or reranker."
    print "Please run 'make swig-python' first."
    print
    raise

class ScoredParse:
    def __init__(self, ptb_parse, parser_score=None, reranker_score=None,
                 parser_rank=None, reranker_rank=None):
        self.ptb_parse = ptb_parse
        self.parser_score = parser_score
        self.parser_rank = parser_rank
        self.reranker_score = reranker_score
        self.reranker_rank = reranker_rank
    def __str__(self):
        return "%s %s %s" % \
            (self.parser_score, self.reranker_score, self.ptb_parse)
    def __repr__(self):
        return "%s(%r, %r, %r)" % (self.__class__.__name__,
                                   str(self.ptb_parse), self.parser_score,
                                   self.reranker_score)

class Sentence:
    def __init__(self, text_or_tokens, max_sentence_length=399):
        if isinstance(text_or_tokens, Sentence):
            self.sentrep = text_or_tokens.sentrep
        elif isinstance(text_or_tokens, basestring):
            self.sentrep = parser.tokenize('<s> ' + text_or_tokens + ' </s>',
                                           max_sentence_length)
        else:
            self.sentrep = parser.SentRep(text_or_tokens)
    def get_tokens(self):
        tokens = []
        for index in range(len(self.sentrep)):
            tokens.append(self.sentrep.getWord(index).lexeme())
        return tokens

class NBestList:
    def __init__(self, sentrep, parses):
        # we keep this around since it's our key to converting our input
        # to the reranker's format (see __str__())
        self._parses = parses
        self._sentrep = sentrep
        self.parses = []
        for index, (score, parse) in enumerate(parses):
            scored_parse = ScoredParse(parse, score, parser_rank=index)
            self.parses.append(scored_parse)
        self._reranked = False

    def __getattr__(self, key):
        """Defer anything unimplemented to our list of ScoredParse objects."""
        return getattr(self.parses, key)

    def sort_by_reranker_scores(self):
        self.parses.sort(key=lambda parse: -parse.reranker_score)
    def get_parser_best(self):
        if len(self.parses):
            return min(self, key=lambda parse: parse.parser_rank)
        else:
            return None
    def get_reranker_best(self):
        return min(self, key=lambda parse: parse.reranker_rank)
    def get_tokens(self):
        return self._sentrep.get_tokens()
    def rerank(self, reranker, lowercase=True):
        """reranker can be a RerankingParser or RerankerModel."""
        assert reranker
        if not self.parses:
            self._reranked = True
            return
        if isinstance(reranker, RerankingParser):
            reranker = reranker.reranker_model
        reranker_input = self.as_reranker_input()
        scores = reranker.scoreNBestList(reranker_input)
        # this could be more efficient if needed
        for (score, nbest_list_item) in zip(scores, self.parses):
            nbest_list_item.reranker_score = score
        self.sort_by_reranker_scores()
        for index, nbest_list_item in enumerate(self.parses):
            nbest_list_item.reranker_rank = index
        self._reranked = True

    def __str__(self):
        if self._reranked:
            from cStringIO import StringIO
            combined = StringIO()
            combined .write('%d dummy\n' % len(self.parses))
            for parse in self.parses:
                combined.write('%s %s\n%s\n' % \
                    (parse.reranker_score, parse.parser_score, parse.ptb_parse))
            combined.seek(0)
            return combined.read()
        else:
            return parser.asNBestList(self._parses)
    def as_reranker_input(self, lowercase=True):
        return reranker.readNBestList(str(self), lowercase)

class RerankingParser:
    def __init__(self):
        self._parser_model_loaded = False
        self.reranker_model = None
        self._parser_thread_slot = parser.ThreadSlot()

    def load_parsing_model(self, model_dir, language='En',
                           case_insensitive=False, nbest=50, small_corpus=True,
                           overparsing=21, debug=0, smoothPos=0):
        assert not self._parser_model_loaded
        self._parser_model_loaded = True
        parser.loadModel(model_dir)
        parser.setOptions(language, case_insensitive, nbest, small_corpus,
                          overparsing, debug, smoothPos)

    def parse(self, sentence, rerank=True, max_sentence_length=399):
        """Parse some text or tokens and return an NBestList with the
        results.  sentence can be a string or a sequence.  If it is a
        string, it will be tokenized.  If rerank is True, we will rerank
        the n-best list."""
        assert self._parser_model_loaded

        sentence = Sentence(sentence, max_sentence_length)
        try:
            parses = parser.parse(sentence.sentrep, self._parser_thread_slot)
        except RuntimeError:
            parses = []
        nbest_list = NBestList(sentence, parses)
        if rerank:
            nbest_list.rerank(self)
        return nbest_list

    def parse_tagged(self, tokens, possible_tags, rerank=True):
        """Parse some pre-tagged, pre-tokenized text.  tokens is a
        sequence of strings.  possible_tags is map from token indices
        to possible POS tags.  Tokens without an entry in possible_tags
        will be unconstrained by POS.  If rerank is True, we will
        rerank the n-best list."""
        assert self._parser_model_loaded

        ext_pos = parser.ExtPos()
        for index in range(len(tokens)):
            tags = possible_tags.get(index, [])
            if isinstance(tags, basestring):
                tags = [tags]
            ext_pos.addTagConstraints(parser.VectorString(tags))

        sentence = Sentence(tokens)
        parses = parser.parse(sentence.sentrep, ext_pos, self._parser_thread_slot)
        nbest_list = NBestList(sentence, parses)
        if rerank:
            nbest_list.rerank(self)
        return nbest_list

    def load_reranker_model(self, features_filename, weights_filename,
                            feature_class=None):
        self.reranker_model = reranker.RerankerModel(feature_class,
                                                     features_filename,
                                                     weights_filename)

def load_included_model():
    import os
    rrp = RerankingParser()
    if os.path.isdir('../first-stage/DATA/EN'):
        rrp.load_parsing_model('../first-stage/DATA/EN')
    else:
        rrp.load_parsing_model('first-stage/DATA/EN')

    reranker_model_dir = '../second-stage/models/ec50spfinal/'
    if not os.path.isdir(reranker_model_dir):
        reranker_model_dir = 'second-stage/models/ec50spfinal/'

    features_filename = reranker_model_dir + 'features.gz'
    weights_filename = reranker_model_dir + 'cvlm-l1c10P1-weights.gz'

    rrp.load_reranker_model(features_filename, weights_filename)

    return rrp

def load_unified_model_dir(model_dir):
    rrp = RerankingParser()
    rrp.load_parsing_model(model_dir + '/parser/')

    reranker_model_dir = model_dir + '/reranker/'
    features_filename = reranker_model_dir + 'features.gz'
    weights_filename = reranker_model_dir + 'weights.gz'

    rrp.load_reranker_model(features_filename, weights_filename)
    return rrp

if __name__ == "__main__":
    from time import time
    class timing:
        depth = 0
        depth_changes = 0
        def __init__(self, description):
            self.description = description
        def __enter__(self):
            self.start = time()

            indent = '  ' * self.__class__.depth
            print '%s%s {' % (indent, self.description)

            self.__class__.depth += 1
            self.__class__.depth_changes += 1
        def __exit__(self, exc_type, exc_value, traceback):
            elapsed = time() - self.start
            self.__class__.depth -= 1
            indent = '  ' * self.__class__.depth
            print '%s} [%.1fs]' % (indent, elapsed)

    rrp = RerankingParser()

    with timing("loading"):
        with timing("loading parsing model"):
            rrp.load_parsing_model('../first-stage/DATA/EN')

        with timing("loading reranking model"):
            reranker_model_dir = '../second-stage/models/ec50spfinal/'
            features_filename = reranker_model_dir + 'features.gz'
            weights_filename = reranker_model_dir + 'cvlm-l1c10P1-weights.gz'

            rrp.load_reranker_model(features_filename, weights_filename)

    with timing("parsing"):
        sentence = "This is the reranking parser .".split()
        nbest_list = rrp.parse(sentence)
        print nbest_list[0]
        nbest_list.rerank(rrp)
        print nbest_list[0]

    with timing("parsing"):
        sentence = "This is a much much longer sentence which we will parse using the reranking parser .".split()
        nbest_list = rrp.parse(sentence)
        print nbest_list[0]
        nbest_list.rerank(rrp)
        print nbest_list[0]

    for scored_parse in nbest_list:
        print scored_parse, scored_parse.parser_rank
