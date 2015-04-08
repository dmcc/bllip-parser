#!/usr/bin/env python
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

# Tool for merging extracted features in "cvlm" format and their
# feature mappings.

from itertools import izip

from bllipparser.RerankerFeatureCorpus import RerankerFeatureCorpus, \
    FeatureMapping
from waterworks.Files import possibly_compressed_file
from waterworks.Sequences import display_index_every_K_items
from waterworks.Tools import initialize

class Merger:
    def __init__(self, corpus_filename1, corpus_filename2, mapping1, mapping2,
                 merged_corpus_filename, merged_mapping_filename,
                 verbose=True, warn_only=False):
        initialize(self, locals())
        self.read_mappings()
        self.make_merged_mapping()
        self.make_merged_feature_values()
    def read_mappings(self):
        if self.verbose:
            print 'Reading mapping 1 (%s)' % self.mapping1
        self.mapping1 = FeatureMapping.mapping_from_filename(self.mapping1)
        if self.verbose:
            print 'Mapping 1: %d features.' % len(self.mapping1)
            print 'Reading mapping 2 (%s)' % self.mapping2
        self.mapping2 = FeatureMapping.mapping_from_filename(self.mapping2)
        if self.verbose:
            print 'Mapping 2: %d features.' % len(self.mapping2)
    def make_merged_mapping(self):
        self.merged_mapping = FeatureMapping(self.mapping1)
        # amount that we shift all the feature indices in corpus2 by
        self.offset = len(self.mapping1)
        for index, name in self.mapping2.items():
            new_index = index + self.offset
            self.merged_mapping[new_index] = name
        if self.verbose:
            print 'Merged mapping: %d features.' % len(self.merged_mapping)
        self.merged_mapping.write(self.merged_mapping_filename)
    def make_merged_feature_values(self):
        def warn_or_error(message):
            if self.warn_only:
                print "Warning:", message
            else:
                raise ValueError(message)

        self.corpus1 = RerankerFeatureCorpus(self.corpus_filename1)
        self.corpus2 = RerankerFeatureCorpus(self.corpus_filename2)

        if len(self.corpus1) != len(self.corpus2):
            warn_or_error("Corpus 1 has %d sentences, corpus 2 has %d." %
                          (len(self.corpus1), len(self.corpus2)))

        merged_corpus = possibly_compressed_file(self.merged_corpus_filename,
                                                 'w')
        merged_corpus.write(self.corpus1.cvlm_format_header())
        sentence_iter = izip(self.corpus1, self.corpus2)
        if self.verbose:
            print 'Transforming corpora (%d sentences)' % len(self.corpus1)
            sentence_iter = display_index_every_K_items(sentence_iter, 50,
                                                        format='Sentence %s\n')
        for sentence_index, (sentence1, sentence2) in enumerate(sentence_iter):
            if len(sentence1) != len(sentence2):
                warn_or_error("Sentence %d: Corpus 1 has %d parses, corpus "
                              "2 has %d." % (sentence_index, len(sentence1),
                                             len(sentence2)))
                
            if sentence1.gold_brackets != sentence2.gold_brackets:
                warn_or_error("Sentence %d: Corpus 1 has %d gold brackets, "
                              "corpus 2 has %d." % (sentence_index,
                                                    sentence1.gold_brackets,
                                                    sentence2.gold_brackets))

            parse_iter = enumerate(izip(sentence1, sentence2))
            for parse_index, (parse1, parse2) in parse_iter:
                if parse1.proposed_brackets != parse2.proposed_brackets:
                    warn_or_error("Sentence %d, parse %d: Corpus 1 has %d "
                                  "proposed brackets, corpus 2 has %d." %
                                  (sentence_index, parse_index,
                                   parse1.proposed_brackets,
                                   parse2.proposed_brackets))
                if parse1.matched_brackets != parse2.matched_brackets:
                    warn_or_error("Sentence %d, parse %d: Corpus 1 has %d "
                                  "matched brackets, corpus 2 has %d." %
                                  (sentence_index, parse_index,
                                   parse1.matched_brackets,
                                   parse2.matched_brackets))

                # add all features from parse2 to parse1 (after remapping)
                features = parse1.features
                for index, value in parse2.features.items():
                    features[index + self.offset] = value
            merged_corpus.write(sentence1.cvlm_format())

if __name__ == "__main__":
    import optparse

    parser = optparse.OptionParser(usage="""%prog [flags]

Tool for merging extracted features in "cvlm" format and their
feature mappings. All arguments are flags, no positional arguments
are allowed.""")
    parser.add_option('-q', '--quiet', dest='verbose', action='store_false',
        default=True, help="Don't print anything unless there are errors.")
    parser.add_option('-w', '--warn-only', action='store_true',
        help="Ignore mismatches (print a warning instead). If mismatches are "
             "big enough, this may cause bad output or crashes.")

    input_group = optparse.OptionGroup(parser, 'Input')
    parser.add_option_group(input_group)
    input_group.add_option('-c', '--corpus1', metavar='FILE',
        dest='corpus_filename1', help='Features counts (1st corpus)')
    input_group.add_option('-m', '--mapping1', metavar='FILE',
        help='Feature mapping (1st corpus)')
    input_group.add_option('-C', '--corpus2', metavar='FILE',
        dest='corpus_filename2', help='Features counts (2nd corpus)')
    input_group.add_option('-M', '--mapping2', metavar='FILE',
        help='Feature mapping (2nd corpus)')

    output_group = optparse.OptionGroup(parser, 'Output')
    parser.add_option_group(output_group)
    output_group.add_option('-o', '--merged-corpus', metavar='FILE',
        dest='merged_corpus_filename',
        help='Filename for merged feature counts')
    output_group.add_option('-O', '--merged-mapping', metavar='FILE',
        dest='merged_mapping_filename',
        help='Filename for merged feature mapping')
    opts, args = parser.parse_args()
    assert not args # everything should be passed as a flag
    merger = Merger(**vars(opts))
