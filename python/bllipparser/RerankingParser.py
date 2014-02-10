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

from os.path import exists, join
import CharniakParser as parser
import JohnsonReranker as reranker

class ScoredParse:
    """Represents a single parse and its associated parser
    probability and reranker score. Note that ptb_parse is actually
    a CharniakParser.InputTree rather than a string (str()ing it will
    return the actual PTB parse."""
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
            # text_or_tokens is a sequence -- need to make sure that each
            # element is a string to avoid crashing
            text_or_tokens = map(str, text_or_tokens)
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
        """Delegate everything else to our list of ScoredParse objects."""
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
        to the reranker. You shouldn't typically need to call this."""
        return reranker.readNBestList(str(self), lowercase)

class RerankingParser:
    """Wraps the Charniak parser and Johnson reranker into a single
    object. Note that RerankingParser is not thread safe."""
    def __init__(self):
        """Create an empty reranking parser. You'll need to call
        load_parser_model() at minimum and load_reranker_model() if
        you're using the reranker. See also the from_unified_model_dir()
        classmethod which will take care of calling both of these
        for you."""
        self._parser_model_loaded = False
        self.parser_model_dir = None
        self.parser_options = {}
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

    def load_parser_model(self, model_dir, **parser_options):
        """Load the parsing model from model_dir and set parsing
        options. In general, the default options should suffice but see
        the set_parser_options() method for details. Note that the parser
        does not allow loading multiple models within the same process
        (calling this function twice will raise a RuntimeError)."""
        if self._parser_model_loaded:
            raise RuntimeError('Parser is already loaded and can only be loaded once.')
        if not exists(model_dir):
            raise ValueError('Parser model directory %r does not exist.' % model_dir)
        self._parser_model_loaded = True
        self.parser_model_dir = model_dir
        parser.loadModel(model_dir)
        self.set_parser_options(**parser_options)

    def load_reranker_model(self, features_filename, weights_filename,
                            feature_class=None):
        """Load the reranker model from its feature and weights files. A feature
        class may optionally be specified."""
        if not exists(features_filename):
            raise ValueError('Reranker features filename %r does not exist.' % \
                features_filename)
        if not exists(weights_filename):
            raise ValueError('Reranker weights filename %r does not exist.' % \
                weights_filename)
        self.reranker_model = reranker.RerankerModel(feature_class,
                                                     features_filename,
                                                     weights_filename)

    def parse(self, sentence, rerank='auto', max_sentence_length=399):
        """Parse some text or tokens and return an NBestList with the
        results. sentence can be a string or a sequence. If it is a
        string, it will be tokenized. If rerank is True, we will rerank
        the n-best list, if False the reranker will not be used. rerank
        can also be set to 'auto' which will only rerank if a reranker
        model is loaded."""
        rerank = self._check_loaded_models(rerank)

        sentence = Sentence(sentence, max_sentence_length)
        try:
            parses = parser.parse(sentence.sentrep, self._parser_thread_slot)
        except RuntimeError:
            parses = []
        nbest_list = NBestList(sentence, parses)
        if rerank:
            nbest_list.rerank(self)
        return nbest_list

    def parse_tagged(self, tokens, possible_tags, rerank='auto'):
        """Parse some pre-tagged, pre-tokenized text. tokens must be a
        sequence of strings. possible_tags is map from token indices
        to possible POS tags (strings). Tokens without an entry in
        possible_tags will be unconstrained by POS. POS tags must be
        in the terms.txt file in the parsing model or else you will get
        a ValueError. If rerank is True, we will rerank the n-best list,
        if False the reranker will not be used. rerank can also be set to
        'auto' which will only rerank if a reranker model is loaded."""
        rerank = self._check_loaded_models(rerank)
        if isinstance(tokens, basestring):
            raise ValueError("tokens must be a sequence, not a string.")

        ext_pos = parser.ExtPos()
        for index in range(len(tokens)):
            tags = possible_tags.get(index, [])
            if isinstance(tags, basestring):
                tags = [tags]
            tags = map(str, tags)
            valid_tags = ext_pos.addTagConstraints(parser.VectorString(tags))
            if not valid_tags:
                # at least one of the tags is bad -- find out which ones
                # and throw a ValueError
                self._find_bad_tag_and_raise_error(tags)

        sentence = Sentence(tokens)
        parses = parser.parse(sentence.sentrep, ext_pos,
            self._parser_thread_slot)
        nbest_list = NBestList(sentence, parses)
        if rerank:
            nbest_list.rerank(self)
        return nbest_list

    def _find_bad_tag_and_raise_error(self, tags):
        ext_pos = parser.ExtPos()
        bad_tags = set()
        for tag in set(tags):
            good_tag = ext_pos.addTagConstraints(parser.VectorString([tag]))
            if not good_tag:
                bad_tags.add(tag)

        raise ValueError("Invalid POS tags (not present in the parser's terms.txt file): %s" % ', '.join(sorted(bad_tags)))

    def _check_loaded_models(self, rerank):
        """Given a reranking mode (True, False, 'auto') determines
        whether we have the appropriately loaded models. Also returns
        whether the reranker should be used (essentially resolves the
        value of rerank if rerank='auto')."""
        if not self._parser_model_loaded:
            raise ValueError("Parser model has not been loaded.")
        if rerank == True and not self.reranker_model:
            raise ValueError("Reranker model has not been loaded.")
        if rerank == 'auto':
            return bool(self.reranker_model)
        else:
            return rerank

    def set_parser_options(self, language='En', case_insensitive=False,
        nbest=50, small_corpus=True, overparsing=21, debug=0, smooth_pos=0):
        """Set options for the parser. Note that this is called
        automatically by load_parser_model() so you should only need to
        call this to update the parsing options. The method returns a
        dictionary of the new options.

        The options are as follows: language is a string describing
        the language. Currently, it can be one of En (English), Ch
        (Chinese), or Ar (Arabic). case_insensitive will make the parser
        ignore capitalization. nbest is the maximum size of the n-best
        list. small_corpus=True enables additional smoothing (originally
        intended for training from small corpora, but helpful in many
        situations). overparsing determines how much more time the parser
        will spend on a sentence relative to the time it took to find the
        first possible complete parse. This affects the speed/accuracy
        tradeoff. debug takes a non-negative integer. Setting it higher
        than 0 will cause the parser to print debug messages (surprising,
        no?). Setting smooth_pos to a number higher than 0 will cause the
        parser to assign that value as the probability of seeing a known
        word in a new part-of-speech (one never seen in training)."""
        if not self._parser_model_loaded:
            raise RuntimeError('Parser must already be loaded (call load_parser_model() first)')

        parser.setOptions(language, case_insensitive, nbest, small_corpus,
            overparsing, debug, smooth_pos)
        self.parser_options = {
            'language': language,
            'case_insensitive': case_insensitive,
            'nbest': nbest,
            'small_corpus': small_corpus,
            'overparsing': overparsing,
            'debug': debug,
            'smooth_pos': smooth_pos
        }
        return self.parser_options

    @classmethod
    def load_unified_model_dir(this_class, *args, **kwargs):
        """Deprecated. Use from_unified_model_dir() instead as this
        method will eventually disappear."""
        import warnings
        warnings.warn('BllipParser.load_parser_model() method is deprecated now, use BllipParser.from_unified_model_dir() instead.')
        return this_class.from_unified_model_dir(*args, **kwargs)

    @classmethod
    def from_unified_model_dir(this_class, model_dir, parsing_options=None,
        reranker_options=None):
        """Create a RerankingParser from a unified parsing model on disk.
        A unified parsing model should have the following filesystem
        structure:
        
        parser/
            Charniak parser model: should contain pSgT.txt, *.g files
            among others
        reranker/
            features.gz or features.bz2 -- features for reranker
            weights.gz or weights.bz2 -- corresponding weights of those
            features
        """
        parsing_options = parsing_options or {}
        reranker_options = reranker_options or {}
        (parser_model_dir, reranker_features_filename,
         reranker_weights_filename) = get_unified_model_parameters(model_dir)

        rrp = this_class()
        if parser_model_dir:
            rrp.load_parser_model(parser_model_dir, **parsing_options)
        if reranker_features_filename and reranker_weights_filename:
            rrp.load_reranker_model(reranker_features_filename,
                reranker_weights_filename, **reranker_options)

        rrp.unified_model_dir = model_dir
        return rrp

def tokenize(text, max_sentence_length=399):
    """Helper method to tokenize a string. Note that most methods accept
    untokenized text so you shouldn't need to run this if you intend
    to parse this text. Returns a list of string tokens. If the text is
    longer than max_sentence_length tokens, it will be truncated."""
    sentence = Sentence(text)
    return sentence.get_tokens()

def get_unified_model_parameters(model_dir):
    """Determine the actual parser and reranker model filesystem entries
    for a unified parsing model. Returns a triple:

    (parser_model_dir, reranker_features_filename,
     reranker_weights_filename)

    Any of these can be None if that part of the model is not present
    on disk (though, if you have only one of the reranker model files,
    the reranker will not be loaded).

    A unified parsing model should have the following filesystem structure:

    parser/
        Charniak parser model: should contain pSgT.txt, *.g files
        among others
    reranker/
        features.gz or features.bz2 -- features for reranker
        weights.gz or weights.bz2 -- corresponding weights of those
        features
    """
    if not exists(model_dir):
        raise IOError("Model directory %r does not exist" % model_dir)

    parser_model_dir = join(model_dir, 'parser')
    if not exists(parser_model_dir):
        parser_model_dir = None
    reranker_model_dir = join(model_dir, 'reranker')

    def get_reranker_model_filename(name):
        filename = join(reranker_model_dir, '%s.gz' % name)
        if not exists(filename):
            # try bz2 version
            filename = join(reranker_model_dir, '%s.bz2' % name)
        if not exists(filename):
            filename = None
        return filename

    features_filename = get_reranker_model_filename('features')
    weights_filename = get_reranker_model_filename('weights')
    return (parser_model_dir, features_filename, weights_filename)
