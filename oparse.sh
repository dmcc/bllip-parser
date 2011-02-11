#! /bin/sh
# RERANKDATA=ec50-connll-ic-s5
# RERANKDATA=ec50-f050902-lics5
MODELDIR=second-stage/models/ec50spfinal
ESTIMATORNICKNAME=cvlm-l1c10P1
first-stage/PARSE/parseIt -l399 -N50 first-stage/DATA/EN/ $* | second-stage/programs/features/best-parses -l $MODELDIR/features.gz $MODELDIR/$ESTIMATORNICKNAME-weights.gz
