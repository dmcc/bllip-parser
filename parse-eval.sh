#! /bin/sh
#
# Parses a section of treebank and evaluates the parser output using evalb
#
# Should be called with the treebank source files as an argument

second-stage/programs/prepare-data/ptb -c $* | parse.sh -K > parse-trees.tmp
second-stage/programs/prepare-data/ptb -e $* > gold-trees.tmp
evalb/evalb -p evalb/new.prm gold-trees.tmp parse-trees.tmp
rm parse-trees.tmp gold-trees.tmp
