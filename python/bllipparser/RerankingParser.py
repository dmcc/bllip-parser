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

"""Higher-level python frontend to the BLLIP reranking parser. Wraps the
lower-level (SWIG-generated) CharniakParser and JohnsonReranker modules
so you don't need to interact with them directly."""

import os.path
import CharniakParser as parser
import JohnsonReranker as reranker

class ScoredParse:
    """Represents a single parse and its associated parser probability
    and reranker score."""
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
        return "%s(%r, parser_score=%r, reranker_score=%r)" % \
            (self.__class__.__name__, str(self.ptb_parse), 
             self.parser_score, self.reranker_score)

class Sentence:
    """Represents a single sentence as input to the parser. You should
    not typically need to construct this object directly."""
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
    """Represents an n-best list of parses of the same sentence."""
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
    def sort_by_parser_scores(self):
        self.parses.sort(key=lambda parse: -parse.parser_score)
    def get_parser_best(self):
        """Get the best parse in this n-best list according to the parser."""
        if len(self.parses):
            return min(self, key=lambda parse: parse.parser_rank)
        else:
            return None
    def get_reranker_best(self):
        """Get the best parse in this n-best list according to the reranker."""
        return min(self, key=lambda parse: parse.reranker_rank)
    def get_tokens(self):
        """Get the tokens of this sentence as a sequence of strings."""
        return self._sentrep.get_tokens()
    def rerank(self, reranker, lowercase=True):
        """Rerank this n-best list according to a reranker model. reranker
        can be a RerankingParser or RerankerModel."""
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
        """Represent the n-best list in a similar output format to the
        command-line parser and reranker."""
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
        """Convert the n-best list to an internal structure used as input
        to the reranker.  You shouldn't typically need to call this."""
        return reranker.readNBestList(str(self), lowercase)

class RerankingParser:
    """Wraps the Charniak parser and Johnson reranker into a single
    object. In general, the RerankingParser is not thread safe."""
    def __init__(self):
        """Create an empty reranking parser. You'll need to call
        load_parsing_model() at minimum and load_reranker_model() if
        you're using the reranker. See also the load_unified_model_dir()
        classmethod which will take care of calling both of these
        for you."""
        self._parser_model_loaded = False
        self.parser_model_dir = None
        self.reranker_model = None
        self._parser_thread_slot = parser.ThreadSlot()
        self.unified_model_dir = None

    def __repr__(self):
        if self.unified_model_dir:
            return "%s(unified_model_dir=%r)" % \
                (self.__class__.__name__, self.unified_model_dir)
        else:
            return "%s(parser_model_dir=%r, reranker_model=%r)" % \
                (self.__class__.__name__, self.parser_model_dir,
                 self.reranker_model)

    def load_parsing_model(self, model_dir, language='En',
                           case_insensitive=False, nbest=50, small_corpus=True,
                           overparsing=21, debug=0, smoothPos=0):
        """Load the parsing model from model_dir and set parsing
        options. In general, the default options should suffice. Note
        that the parser does not allow loading multiple models within
        the same process."""
        if self._parser_model_loaded:
            raise ValueError('Parser is already loaded and can only be loaded once.')
        if not os.path.exists(model_dir):
            raise ValueError('Parser model directory %r does not exist.' % model_dir)
        self._parser_model_loaded = True
        parser.loadModel(model_dir)
        self.parser_model_dir = model_dir
        parser.setOptions(language, case_insensitive, nbest, small_corpus,
                          overparsing, debug, smoothPos)

    def load_reranker_model(self, features_filename, weights_filename,
                            feature_class=None):
        """Load the reranker model from its feature and weights files. A feature
        class may optionally be specified."""
        if not os.path.exists(features_filename):
            raise ValueError('Reranker features filename %r does not exist.' % \
                features_filename)
        if not os.path.exists(weights_filename):
            raise ValueError('Reranker weights filename %r does not exist.' % \
                weights_filename)
        self.reranker_model = reranker.RerankerModel(feature_class,
                                                     features_filename,
                                                     weights_filename)

    def parse(self, sentence, rerank=True, max_sentence_length=399):
        """Parse some text or tokens and return an NBestList with the
        results.  sentence can be a string or a sequence.  If it is a
        string, it will be tokenized.  If rerank is True, we will rerank
        the n-best list."""
        self.check_loaded_models(rerank)

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
        self.check_loaded_models(rerank)

        ext_pos = parser.ExtPos()
        for index in range(len(tokens)):
            tags = possible_tags.get(index, [])
            if isinstance(tags, basestring):
                tags = [tags]
            ext_pos.addTagConstraints(parser.VectorString(tags))

        sentence = Sentence(tokens)
        parses = parser.parse(sentence.sentrep, ext_pos,
            self._parser_thread_slot)
        nbest_list = NBestList(sentence, parses)
        if rerank:
            nbest_list.rerank(self)
        return nbest_list

    def check_loaded_models(self, rerank):
        if not self._parser_model_loaded:
            raise ValueError("Parser model has not been loaded.")
        if rerank and not self.reranker_model:
            raise ValueError("Reranker model has not been loaded.")

    @classmethod
    def load_unified_model_dir(this_class, model_dir, parsing_options=None,
        reranker_options=None):
        """Create a RerankingParser from a unified parsing model on disk.
        A unified parsing model should have the following filesystem structure:
        
        parser/
            Charniak parser model: should contain pSgT.txt, *.g files,
            and various others
        reranker/
            features.gz -- features for reranker
            weights.gz -- corresponding weights of those features
        """
        parsing_options = parsing_options or {}
        reranker_options = reranker_options or {}
        rrp = this_class()
        rrp.load_parsing_model(model_dir + '/parser/', **parsing_options)

        reranker_model_dir = model_dir + '/reranker/'
        features_filename = reranker_model_dir + 'features.gz'
        weights_filename = reranker_model_dir + 'weights.gz'

        rrp.load_reranker_model(features_filename, weights_filename,
            **reranker_options)
        rrp.unified_model_dir = model_dir
        return rrp

def tokenize(text, max_sentence_length=399):
    """Helper method to tokenize a string. Note that most methods accept
    untokenized text so you shouldn't need to run this if you intend
    to parse this text. Returns a list of string tokens. If the text is
    longer than max_sentence_length tokens, it will be truncated."""
    sentence = Sentence(text)
    return sentence.get_tokens()
