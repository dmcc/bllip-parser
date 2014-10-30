Overview
--------
The ``TRAIN`` directory includes the programs needed to train the parser
(by reading in in treebank data and collecting the needed probabilities).
While many of the source files have the same names as those of the
parser, often they are slightly different and thus this directory must
be kept separate.

The shell script ``trainParser`` runs the various programs needed to
train the parser/language model. Run it with no arguments to get a usage
statement. For the English parser, usage is::

    shell> trainParser -parser [data directory] [training corpus] [development corpus]

For the English language model, use ``-lm`` instead of ``-parser``.
For Chinese, add the ``-Ch`` flag after ``-parser``.

The train and dev corpora should be in Penn Treebank format (similar to
parser output). Training data is not provided with the parser.

Files created during training will be written to "data directory".
Importantly, the training code (and parser) also expect certain static
files to be here that are **not** created during training. As such,
the easiest way to setup everything correctly is to make a copy of the
appropriate data directory that is distributed with the parser:

* ``DATA/EN``: English parser (trained on PTB III WSJ corpus)
* ``DATA/LM``: English language model
* ``DATA/CH``: Chinese parser (trained on LDC Chinese Treebank)

and point ``trainParser`` at your copied directory.

Some additional notes on training:

* The file ``terms.txt`` must contain a list of all part-of-speech and
  bracket labels found in the train and dev corpus, so you may need to
  add additional labels to it for your corpora.  See details of its file
  format below.

* All such labels found in the dev corpus must also be present in the
  train corpus.

* As coded, only the first 1000 sentences of the dev corpus are used
  (i.e., if your dev corpus is longer than this, the additional sentences
  will be ignored). This is intended to avoid over-fitting to the dev
  corpus. To change this behavior, modify the main sentence processing
  loop in ``trainRs.C``.

* To get the effect of combining multiple corpora with different
  weights, one means is to simply make multiple copies of each corpus
  (e.g., train = 3 x WSJ + 2 x Brown). If you do this with the language
  model, however, note you will break Knesser-Ney smoothing since it
  will never see any token occurring only once.

Info on parameter files
-----------------------
Below is a brief and **incomplete** description of the parameter
files used by the parser and/or its training code (purpose, format,
interpretation, etc.).  Additional info will be added based on need and
as time allows.

The following files are static and required for training::

    -rw-r--r--  1 ec fac  32 May 26 14:17 bugFix.txt
    -rw-r--r--  1 ec fac 258 May 23 16:54 featInfo.h
    -rw-r--r--  1 ec fac 411 May 23 16:54 featInfo.l
    -rw-r--r--  1 ec fac  94 May 23 16:54 featInfo.lm
    -rw-r--r--  1 ec fac 298 May 23 16:54 featInfo.m
    -rw-r--r--  1 ec fac 405 May 23 16:54 featInfo.r
    -rw-r--r--  1 ec fac  91 May 23 16:54 featInfo.rm
    -rw-r--r--  1 ec fac  65 May 23 16:54 featInfo.ru
    -rw-r--r--  1 ec fac 112 May 23 16:54 featInfo.s
    -rw-r--r--  1 ec fac 138 May 23 16:54 featInfo.t
    -rw-r--r--  1 ec fac  58 May 23 16:54 featInfo.tt
    -rw-r--r--  1 ec fac 181 May 23 16:54 featInfo.u
    -rw-r--r--  1 ec fac 553 May 23 16:54 headInfo.txt
    -rw-r--r--  1 ec fac 609 May 23 16:54 terms.txt

* ``bugFix.txt`` includes shards of sentences which are necessary to cover
  very unlikely combinations which the training data do not cover::

     ( (FRAG (NP (NN Task) (# #)) (. .)))

* ``featInfo.*`` tell the data collection programs exactly
  what features to attend to (see ``treeHistSf.h`` for a long comment
  block with features names / IDs). The order is consistent with the
  conditioning order of the prob model in the paper:

      - ``h``: head
      - ``u``: terminal POS
      - ``t``: pre-terminal POS
      - ``c``: punctuation
      - ``v``: parent POS
      - ``m``: grandparent POS
      - ``i``: parent term

* ``headInfo.txt`` states which children categories like to be the
  heads of which parent categories (note that the Chinese headfinder
  has a different format).

      - ``1`` = 1st choice, ``2`` = 2nd choice, etc.
      - ``ADJP JJ``: if current POS is ``ADJP``, head is right-most ``JJ``

* ``terms.txt`` tells the parser all the pre-terminal and phrasal
  categories and their type::

      0: constituent types
      1: closed-class, non-punctuation POS
      2: open-class POS
      3: sentence-final punctuation (period, exclamation/question mark)
      4: comma
      5: open quotation
      6: close quotation
      7: open/close parentheses
      8: colon / semi-colon 

Files created during training
-----------------------------

``endings.txt``: statistics for guessing POS of unknown words by 2-letter
suffix::

    * col 1: POS
    * col 2: suffix
    * col 3: P(suffix|POS) (e.g., all rows with POS 3 sum to 1)

``pUgT.txt`` ("probability of unknown given text"): is also used in the
unknown word model::

    * col 1: POS
    * col 2: P(unknown|POS)
    * col 3: P(capitalized|POS)
    * col 4: P(contains hypen|POS)

``*.g``: (extracted statistics from training trees according to the
corresponding ``featInfo.*`` file, see below)

``*.lambdas``: (backoff coefficients, see below)

``unitRules.txt``: provides an ordering of unary production rules

Rough training procedure
------------------------
Extract vocabulary (``pSgT.txt``), unknown word statistics
(``endings.txt``, ``pUgT.txt``, ``nttCounts.txt``), and information
about unary rules (``unitRules.txt``).

For each feature, run:

1. ``rCounts`` - get counts of features (reads train trees, writes ``.ff``
   files)
2. ``selFeats`` - prune features (reads ``.ff`` files, writes ``.f``
   files)
3. ``iScale`` - normalize pruned features (reads ``.f`` files, writes
   ``.g`` files)
4. ``trainRs`` - tune backoff coefficients from dev data (reads dev trees,
   writes ``.lambdas``)

``*.f`` and ``*.ff`` files are not needed for parsing and are deleted.
