#! /bin/sh
# RERANKDATA=ec50-connll-ic-s5
RERANKDATA=ec50-f050902-lics5
first-stage/PARSE/parseIt -l999 -N50 -K first-stage/DATA/EN/ $* | second-stage/programs/features/best-parses -l -m 1 second-stage/features/$RERANKDATA/traindev-feat.gz second-stage/features/$RERANKDATA/cvlm-l1-c10-Pyx1-ns-1/traindev-weights.gz
