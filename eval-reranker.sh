#! /bin/sh

# This script recompiles the reranker code, rebuilds the nbest trees
# and retrains and evaluates the reranker itself.

# You can change the flags below here

NBESTPARSERBASEDIR=first-stage
NBESTPARSERNICKNAME=ec

# NBESTPARSERBASEDIR=first-stage-Aug06
# NBESTPARSERNICKNAME=Aug06

# FEATUREEXTRACTOR=second-stage/programs/features/extract-spfeatures
# FEATUREEXTRACTORFLAGS="-l -c -i -s 5"
# FEATURESNICKNAME=spc

FEATUREEXTRACTOR=second-stage/programs/features/extract-nfeatures
FEATUREEXTRACTORFLAGS="-l -c -i -s 5 -f splh"
FEATURESNICKNAME=splh

# ESTIMATOR=second-stage/programs/wlle/cvlm
# ESTIMATORFLAGS="-l 1 -c0 10 -Pyx_factor 1 -debug 10 -ns -1"
# ESTIMATORNICKNAME=cvlm-l1c10P1-openmp

ESTIMATOR=second-stage/programs/wlle/cvlm-owlqn
ESTIMATORFLAGS="-l 1 -c 10 -F 1 -d 10 -n -1 -t 1e-7"
ESTIMATORNICKNAME=owlqn-l1c10t1e-7

# ESTIMATOR=second-stage/programs/wlle/cvlm-owlqn
# ESTIMATORFLAGS="-l 1 -p 1 -c 10 -F 1 -d 10 -n -1 -t 1e-7"
# ESTIMATORNICKNAME=owlqn-l1c10p1t1e-7

# ESTIMATOR=second-stage/programs/wlle/avper
# ESTIMATORFLAGS="-n 10 -d 0 -F 1 -N 10"
# ESTIMATORNICKNAME=avper

# ESTIMATOR=second-stage/programs/wlle/gavper
# ESTIMATORFLAGS="-a -n 10 -d 10 -F 1 -m 999999"
# ESTIMATORNICKNAME=gavper

# ESTIMATOR=second-stage/programs/wlle/hlm
# ESTIMATORFLAGS="-l 1 -c 10 -C 10000 -F 1 -d 100 -n 0 -S 7 -t 1e-7"
# ESTIMATORNICKNAME=hlm2S7


###############################################################################
#
# You shouldn't need to change anything below here
#
FLAGS="NBESTPARSERBASEDIR=$NBESTPARSERBASEDIR NBESTPARSERNICKNAME=$NBESTPARSERNICKNAME FEATUREEXTRACTOR=$FEATUREEXTRACTOR FEATURESNICKNAME=$FEATURESNICKNAME ESTIMATOR=$ESTIMATOR ESTIMATORNICKNAME=$ESTIMATORNICKNAME"

# echo make clean $FLAGS
# make clean

echo
echo make reranker $FLAGS
make reranker $FLAGS

# echo
# echo make -j 8 nbesttrain
# make -j 8 nbesttrain

# Avoid remaking the nbest parses.  Warning -- you'll recompute the features if you run this!
#
# echo
# echo make touch-nbest $FLAGS
# make touch-nbest $FLAGS

# The nonfinal version trains on sections 2-19, uses sections 20-21 as dev,
# section 22 as test1 and 24 as test2 (this is the "Collins' split")
#
echo
echo make eval-reranker VERSION=nonfinal $FLAGS FEATUREEXTRACTORFLAGS="$FEATUREEXTRACTORFLAGS" ESTIMATORFLAGS="$ESTIMATORFLAGS"
time make eval-reranker VERSION=nonfinal $FLAGS FEATUREEXTRACTORFLAGS="$FEATUREEXTRACTORFLAGS" ESTIMATORFLAGS="$ESTIMATORFLAGS"

# The final version trains on sections 2-21, uses section 24 as dev, 
# section 22 as test1 and section 23 as test2 (this is the standard PARSEVAL split)
# 
echo
echo make eval-reranker VERSION=final $FLAGS FEATUREEXTRACTORFLAGS="$FEATUREEXTRACTORFLAGS" ESTIMATORFLAGS="$ESTIMATORFLAGS"
make eval-reranker VERSION=final $FLAGS FEATUREEXTRACTORFLAGS="$FEATUREEXTRACTORFLAGS" ESTIMATORFLAGS="$ESTIMATORFLAGS"

