    Copyright 1999, 2000, 2001, 2005, 2006 Brown University, Providence, RI.

    Licensed under the Apache License, Version 2.0 (the "License"); you may
    not use this file except in compliance with the License.  You may obtain
    a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
    WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
    License for the specific language governing permissions and limitations
    under the License.

======================================================================

Basic Usage
-----------
The parser (found in the subdirectory ``PARSE``) expects sentences
delimited by ``<s>`` ... ``</s>``, and outputs the parsed versions in
Penn treebank style.  The ``<s>`` and ``</s>`` must be separated by
spaces from all other text.  So if the input is::

    <s> (``He'll work at the factory.'') </s>

the output will be (to ``stdout``)::

    (S1 (PRN (-LRB- -LRB-) (S (`` ``) (NP (PRP He)) (VP (MD 'll) (VP (VB work) (PP (IN at) (NP (DT the) (NN factory))))) (. .) ('' '')) (-RRB- -RRB-)))

If you want to make it slightly easier for humans to read, use the
command line argument ``-P`` (pretty print), in which case you will get::

    (S1 (PRN (-LRB- -LRB-)
         (S (`` ``)
          (NP (PRP He))
          (VP (MD 'll)
           (VP (VB work) (PP (IN at) (NP (DT the) (NN factory)))))
          (. .)
          ('' ''))
         (-RRB- -RRB-)))

The parser will take input from either ``stdin``, or, if given the name
of a file, from that file.  So in the latter case the call to the parser
would be::

    shell> parseIt /path/to/model/dir/ /path/to/file/with/sentences

For example::

    shell> parseIt ../DATA/EN/ input-sentences.sgml

(Note that as the parser is currently distributed with three separate
``DATA`` directories, one each for English, Chinese, and English
Language Modeling.  The distributed parsing model is trained on an
``AUX``-ified version of Wall Street Journal.  More models (including
a non-``AUX``-ified version) can be obtained with the ``ModelFetcher``
module in the Python library.)

As indicated above, the parser will first tokenize the input.  If you do
*not* want to to tokenize (for some reason you are handing it pretokenized
input, as you would do if you were testing it's performance on the
treebank), give it a ``-K`` option and pass space separated tokens::

    <s> ( `` He 'll work at the factory . '' ) </s>

Compilation instructions
------------------------
The easiest way is to run ``make`` in the top-level ``Makefile`` which
will build the whole system.  To just build the parser, run ``make
PARSE``.  To only build the training tools for the first-stage parser,
run ``make TRAIN``.

*n*-best Parsing
----------------
The parser can produce *n*-best parses.  So if you want the 50 highest
scoring parses rather than just the highest scoring one, just add ``-N50``
to the command line.

In *n*-best mode the output format is slightly different::

    number-of-parses sentence-indicator-string
    logProb(parse1)
    parse1
    
    logProb(parse2)
    parse2

etc.

The sentence indicator string will typically just a sentence number.
However, if the input to the parser is of the form (with this exact
spacing)::

    <s sentence-id > ... </s>

then the ``sentence-id`` provided will be used instead.  This is useful
if, e.g., you want to know where article boundaries are.

Other options
-------------
The ``-S`` flag tells the parser to remain silent when it cannot parse
a sentence (it just goes on to the next one).

The parser can now parse Chinese.  It requires that the Chinese characters
already be grouped into words.  Assuming you have trained on the Chinese
Treebank from LDC (see the ``README`` for the ``TRAIN`` programs), you
tell the parser to be expecting Chinese by giving it the command line
option ``-LCh``.  (The default is English, which is also be specified by
``-LEn``.)  The files you need to train Chinese are in ``DATA/CH/``.

By default, the parser will skip any sentence consisting of more 100
tokens.  To change this to 200 you give it the command line argument
``-l200``.

The parser is set to be case sensitive.  To make it case insensitive
add the command line flag ``-C``.

Currently there are various array sizes that make 400 the absolute
maximum sentence length.  To allow for longer sentences change (in
``Feature.h``)::

    #define MAXSENTLEN 400

Similarly to allow for a larger dictionary of words from training,
increase::

    #define MAXNUMWORDS 500000

To see debugging information give it the on-line argument ``-d<number>``
where the ``<number>`` is > 10.  As the numbers get larger, the verbosity
of the information increases.

Training
--------
The subdirectory ``TRAIN`` contains the programs used to collect the
statistics the parser requires from treebank data.  As the parser comes
with the statistics it needs you will only need this if you want to try
experiments with the parser on more (or less, or different) treebank data.
For more information see the ``README`` file in ``TRAIN``.

Language Modeling
-----------------
To use the parser as the language model described in Charniak (ACL 2001)
you must first retrain the data using the settings found in ``DATA/LM/``.

Then give ``parseIt`` a ``-M`` command line argument.  If the data
is from speech, and thus all one case, also use the case-insensitive
(``-C``) flag.

The output in ``-M`` mode is of the form::

    log-grammar-probability log-trigram-probability log-mixed-probability
    parse

Again, if the data is from speech and has a limited vocabulary, it will
often be the case that the parser will have a very difficult time finding
a parse because of incorrect words (or, in simulated speech output, the
presence of "unk" the unknown word replacement), and there will be many
parses with equally bad probabilities.  In such cases the pruning that
keeps memory in bounds for 50-best parsing fails.  So just use 1-best,
or maybe 10-best.

Faster Parsing
--------------
The default speed/accuracy setting should give you the results in the
published papers.  It is, however, easy to get faster parsing at the
expense of some accuracy.  So a command line argument of ``-T50``
costs you about a percent of parsing accuracy, but rather than 1.4
sentences/second [editor's note: your mileage may vary] you will get
better than 6 sentences/second. (The default is ``-T210``.)

Multi-threaded version
----------------------
[Update 2013] **Using more than one thread is not currently recommended
as there appear to be thread safety issues.**

``parseIt`` is multithreaded.  It currently defaults to using a single
thread. To change this, use the command line argument, ``-t4`` to have
it use, e,g, 4 threads.  To change the maximum number of threads, change
the following line in ``Features.h`` and recompile ``parseIt``::

    #define MAXNUMTHREADS [maximum number of threads]

The original non-threaded ``parseIt`` is available as ``oparseIt``
(has fewer features/bugfixes than parseIt). However, ``parseIt`` with
a single thread should be safe to run.

``evalTree``
------------
``evalTree`` takes Penn Treebank parse trees from ``stdin``, and outputs
to ``stdout``::

    sentence-number log2(parse-tree-probability)

for each tree, one per line. ``evalTree`` can be run using the following::

    shell> evalTree /path/to/model/dir/

If the tree is assigned zero probability it returns 0 for the log2
probability.

For reasons that would take us too far afield, about 13% of the time it
returns a probability that is too high.  If you want to be warned when
it is doing this, give ``evalTree`` a ``-W`` command line argument and
the output will have an ``!`` at the end of the line when this happens.

Parsing from tagged input
-------------------------
If you have tags from an external part-of-speech tagger or lexicon,
you can now strongly encourage the parser to use these tags.  This can
now be done using a command such as the following::

    shell> parseIt -K -Einput.tags /path/to/model/dir/ input.sgml

where ``input.sgml`` looks something like this::

    <s> This is a test sentence . </s>

and ``input.tags`` looks something like this::

    This DT
    is VBZ
    a DT
    test NN
    sentence NN
    . .
    ---

Each token is given a list of zero or more tags and sentences are
separated by ``---`` (three hyphens).  Tokens and tags are whitespace
delimited.  If a token is given zero tags, the standard tagging mechanism
will be employed for tagging that token.  If a token is given multiple
tags, they will each be considered.

Note that the tokenization must match exactly between these files (tokens
are space-separated in ``input.sgml``).  To ensure that tokenization
matches, you should pretokenize your input and supply the ``-K`` flag.

Frequently confusing errors
---------------------------
a.  If parser provides no output at all

    This is most likely caused by not having spaces around the ``<s>``
    and ``</s>`` brackets, i.e.,::

        <s>This is a test sentence.</s>

    instead of::

        <s> This is a test sentence. </s>

b.  When retraining: ``Couldn't find term: _____
    pSgT: InputTree.C:206: InputTree* InputTree::newParse(std::istream&,
    int&, InputTree*): Assertion `Term::get(trm)' failed.``

    This means the training data contains an unknown term (phrasal or
    part of speech type). You'll need to add the appropriate entry to
    ``terms.txt`` in the model you're training. See the ``README`` in
    ``TRAIN`` for more details.

If you're still stuck, check the other ``README`` files then consider
filing a bug at https://github.com/BLLIP/bllip-parser/issues
