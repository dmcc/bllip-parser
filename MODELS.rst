Introduction to BLLIP Parser models
===================================

There are several available parsing models for BLLIP Parser. This
document is designed to help you determine which one will perform best
for your task. Each one of the parsing models discussed includes a pair
of Charniak parser and Johnson reranker models designed to work together
(this is called a ``unified parsing model``).

Finding parsing models
----------------------
If you don't already have the Python ``bllipparser`` module, run the
following in your shell::

    shell% pip install --user bllipparser

Or, if you can run ``sudo``::

    shell% sudo pip install bllipparser

Once you have ``bllipparser``, you can use the ``ModelFetcher``
functionality to list and download parsing models. To list parsing models,
run the following in your shell::

    shell% python -mbllipparser.ModelFetcher -l
    8 known unified parsing models: [uncompressed size]
    GENIA+PubMed:
        Self-trained model on GENIA treebank and approx. 200k sentences
        from PubMed [152MB]
    OntoNotes-WSJ:
        WSJ portion of OntoNotes [61MB]
    SANCL2012-Uniform:
        Self-trained model on OntoNotes-WSJ and the Google Web Treebank
        [890MB]
    WSJ:
        Wall Street Journal corpus from Penn Treebank, version 2
        ("AnyDomain" version) [52MB]
    WSJ+Gigaword:
        Self-trained model on PTB2-WSJ and approx. two million sentences
        from Gigaword (deprecated) [473MB]
    WSJ+Gigaword-v2:
        Improved self-trained model on PTB WSJ and two million sentences
        from Gigaword [435MB]
    WSJ-PTB3:
        Wall Street Journal corpus from Penn Treebank, version 3 [55MB]
    WSJ-with-AUX:
        Wall Street Journal corpus from Penn Treebank, version 2 (AUXified
        version, deprecated) [55MB]

This list may change as new parsing models are added to the list.
To download and install ``WSJ+Gigaword-v2`` (as an example), run the
following in your shell::

    % python -mbllipparser.ModelFetcher -i WSJ+Gigaword-v2

Parsing models
--------------
Depending on the text that you'd like to parse, there are different
optimal parsing models. Here are the current recommendations:

- **News text:** ``WSJ+Gigaword-v2``

- **Web text:** ``SANCL2012-Uniform``

- **Biomedical (PubMed) text:** ``GENIA+PubMed``

- **WSJ section 23 evaluations to replicate papers:** For purely supervised
  parser or parser/reranker results, use either ``WSJ-PTB3``
  (for Penn Treebank WSJ) or ``OntoNotes-WSJ`` (for the OntoNotes version
  of WSJ). Use ``WSJ+Gigaword`` to replicate self-training results, though
  ``WSJ+Gigaword-v2`` performs slightly better.

- **Everything else:** In general, it's probably best to use
  ``SANCL2012-Uniform`` or ``WSJ+Gigaword-v2`` depending on how
  well-formed your text is (``SANCL2012-Uniform`` for more informal
  web/email text).
