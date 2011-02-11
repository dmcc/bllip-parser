# Makefile for ~/research/rerank that uses Michael Collins' n-best trees
#
# Mark Johnson, 30th December 2004
#
# $(FEXTRACTOR) $(FEXTRACTOR-ARGS) extracts features from parse trees and
# stores them in $(FEATDIR).
#
# $(MESTIMATOR) $(MESTIMATOR-ARGS) estimates a model from these features,
# which it saves in $(FEATDIR)/$(MODELDIR)/weights. $(MDEPENDENCIES) are 
# other files which the estimator requires.
#
# $(FEATDIR)/$(MODELDIR)/weights.eval evaluates these weights on development
# data.
#
# $(FEATDIR)/$(MODELDIR)/incorrect-parses.txt prints out the incorrect parses
# and the corresponding correct parses.

# TARGET = train-weights.gz.eval # $(FEATDIR)/$(MODELDIR)/dev-parses.bz2
TARGET = traindev-weights.gz.eval

TREES = trees/mc
FEATDIR = mc-spbest-ic-s5
FEXTRACTOR = programs/features-best/extract-spfeatures
FEXTRACTOR-ARGS = -i -c -s 5
# FEXTRACTOR-ARGS = -i -c -a -C 10

# TREES = trees/mc-ss
# FEATDIR = mc-ssp-ic-s5
# FEXTRACTOR = programs/features/extract-sspfeatures
# FEXTRACTOR-ARGS = -i -c -s 5
# FEXTRACTOR-ARGS = -i -c -a -C 10

# TREES = trees/mc
# FEATDIR = mc-sp-ic-s5
# FEXTRACTOR = programs/features/extract-spfeatures
# FEXTRACTOR-ARGS = -i -c -s 5
# FEXTRACTOR-ARGS = -i -c -a -C 10

# TREES = trees/mc
# FEATDIR = mc-spbak-ic-s5
# FEXTRACTOR = programs/features/extract-spfeatures-bak
# FEXTRACTOR-ARGS = -i -c -s 5


# MODELDIR = lm-l1-Pyx1-c5
# MESTIMATOR = programs/wlle/lm
# MESTIMATOR-ARGS = -l 1 -Pyx_factor 1 -c 5 -debug 10
# MDEPENDENCIES = 

# MODELDIR = lm-l1-c12-Pyx1
# MESTIMATOR = programs/wlle/lm
# MESTIMATOR-ARGS = -l 1 -c 11.87 -Pyx_factor 1 -debug 10
# MDEPENDENCIES =

MESTIMATOR = programs/wlle/cvlm
MODELDIR = cvlm-l1-c20-Pyx1
MESTIMATOR-ARGS = -l 1 -c0 20 -ns -1 -Pyx_factor 1 -debug 10
MDEPENDENCIES =

# MESTIMATOR = programs/wlle/cvlm
# MODELDIR = cvlm-l1-c20
# MESTIMATOR-ARGS = -c0 20 -debug 10
# MDEPENDENCIES =

# MESTIMATOR = programs/wlle/cvlm
# MODELDIR = cvlm-l1-c20-p1-Pyx1
# MESTIMATOR-ARGS = -p 1 -cv -l 1 -c0 20 -debug 10  -Pyx_factor 1 
# MDEPENDENCIES = 

# MESTIMATOR = programs/wlle/cvlm
# MODELDIR = cvlm-l4-c1e-4
# MESTIMATOR-ARGS = -l 4 -c0 1e-4 -debug 10
# MDEPENDENCIES =

# MODELDIR = avper
# MESTIMATOR = programs/wlle/avper
# MESTIMATOR-ARGS = -N 10 -n 10 -d 10 -f 1
# MDEPENDENCIES =

# MODELDIR = wavper
# MESTIMATOR = programs/wlle/wavper
# MESTIMATOR-ARGS = -n 10 -d 10 -h $(FEATDIR)/train-feat.bz2
# MDEPENDENCIES = $(FEATDIR)/train-feat.bz2

# MODELDIR = cvlm-fcb2-l1-c12-ns-1-Pyx1
# MESTIMATOR = programs/wlle/cvlm
# MESTIMATOR-ARGS = -f $(FEATDIR)/train-feat-sorted.bz2 -fcb 2 -l 1 -c0 12 -ns -1 -Pyx_factor 1 -debug 10
# MDEPENDENCIES = $(FEATDIR)/train-feat-sorted.bz2

# EVALWEIGHTS-ARGS = -n 0
EVALWEIGHTS-ARGS = 


# Location of various files

WSJ = /usr/local/data/Penn3/parsed/mrg/wsj

SSDIR = supersenses
SSCMD = bzcat $(SSDIR)/WSJ_FIRST_SENSE_VERBS_NOUNS.bz2

PTB = /usr/local/data/Penn3/parsed/mrg/wsj

# Top-level targets

.PHONY: top
top: $(TARGET)

.PHONY: traindev-weights.gz.eval
traindev-weights.gz.eval: $(FEATDIR)/$(MODELDIR)/traindev-weights.gz.eval 

.PHONY: train-weights.gz.eval
train-weights.gz.eval: $(FEATDIR)/$(MODELDIR)/train-weights.gz.eval 

.PHONY: gold.eval
gold.eval: $(FEATDIR)/$(MODELDIR)/gold.eval


# Feature extraction into gzip'd data files

$(FEATDIR)/train.gz $(FEATDIR)/train-dev.gz $(FEATDIR)/train-f23.gz $(FEATDIR)/train-f24.gz $(FEATDIR)/train-feat.gz: \
$(TREES)/train.bz2 $(TREES)/dev.bz2 $(TREES)/f23.bz2 $(TREES)/f24.bz2 $(FEXTRACTOR)
	mkdir -p $(FEATDIR)
	$(FEXTRACTOR) $(FEXTRACTOR-ARGS) $(TREES)/train.bz2 $(FEATDIR)/train.gz \
	                                 $(TREES)/dev.bz2 $(FEATDIR)/train-dev.gz \
	                                 $(TREES)/f23.bz2 $(FEATDIR)/train-f23.gz \
	                                 $(TREES)/f24.bz2 $(FEATDIR)/train-f24.gz \
				         | gzip > $(FEATDIR)/train-feat.gz

$(FEATDIR)/$(MODELDIR)/train-weights.gz: \
$(FEATDIR)/train.gz $(FEATDIR)/train-dev.gz $(MESTIMATOR) $(MDEPENDENCIES)
	mkdir -p $(FEATDIR)/$(MODELDIR)
	zcat $(FEATDIR)/train.gz | $(MESTIMATOR) $(MESTIMATOR-ARGS) -e $(FEATDIR)/dev.gz \
			-o $(FEATDIR)/$(MODELDIR)/train-weights | tee $(FEATDIR)/$(MODELDIR)/train-run.out
	rm -f $(FEATDIR)/$(MODELDIR)/train-weights.gz
	gzip $(FEATDIR)/$(MODELDIR)/train-weights

$(FEATDIR)/$(MODELDIR)/train-weights.gz.eval: \
$(FEATDIR)/train-dev.gz $(FEATDIR)/train-f24.gz $(FEATDIR)/$(MODELDIR)/train-weights.gz programs/eval-weights/eval-weights
	zcat $(FEATDIR)/$(MODELDIR)/train-weights.gz \
		| programs/eval-weights/eval-weights $(EVALWEIGHTS-ARGS) $(FEATDIR)/train-feat.gz $(FEATDIR)/train-dev.gz \
	        | tee $(FEATDIR)/$(MODELDIR)/train-weights.gz.eval
	zcat $(FEATDIR)/$(MODELDIR)/train-weights.gz \
		| programs/eval-weights/eval-weights $(EVALWEIGHTS-ARGS) $(FEATDIR)/train-feat.gz $(FEATDIR)/train-24.gz \
	        | tee $(FEATDIR)/$(MODELDIR)/train-weights.gz.eval

$(FEATDIR)/traindev.gz $(FEATDIR)/f23.gz $(FEATDIR)/f24.gz $(FEATDIR)/traindev-feat.gz: \
	$(TREES)/traindev.bz2 $(TREES)/f23.bz2 $(TREES)/f24.bz2 $(FEXTRACTOR)
	mkdir -p $(FEATDIR)
	$(FEXTRACTOR) $(FEXTRACTOR-ARGS) $(TREES)/traindev.bz2 $(FEATDIR)/traindev.gz \
	                                 $(TREES)/f23.bz2 $(FEATDIR)/f23.gz \
	                                 $(TREES)/f24.bz2 $(FEATDIR)/f24.gz \
				         | gzip > $(FEATDIR)/traindev-feat.gz

$(FEATDIR)/$(MODELDIR)/traindev-weights.gz: $(FEATDIR)/traindev.gz $(FEATDIR)/f24.gz $(MESTIMATOR) $(MDEPENDENCIES)
	mkdir -p $(FEATDIR)/$(MODELDIR)
	zcat $(FEATDIR)/traindev.gz | $(MESTIMATOR) $(MESTIMATOR-ARGS) -e $(FEATDIR)/f24.gz \
			-o $(FEATDIR)/$(MODELDIR)/traindev-weights | tee $(FEATDIR)/$(MODELDIR)/traindev-run.out
	rm -f $(FEATDIR)/$(MODELDIR)/traindev-weights.gz
	gzip $(FEATDIR)/$(MODELDIR)/traindev-weights

$(FEATDIR)/$(MODELDIR)/traindev-weights.gz.eval: \
	$(FEATDIR)/f23.gz $(FEATDIR)/f24.gz  $(FEATDIR)/traindev-feat.gz \
	$(FEATDIR)/$(MODELDIR)/traindev-weights.gz programs/eval-weights/eval-weights
	zcat $(FEATDIR)/$(MODELDIR)/traindev-weights.gz \
                | programs/eval-weights/eval-weights $(EVALWEIGHTS-ARGS) $(FEATDIR)/traindev-feat.gz $(FEATDIR)/f23.gz \
                | tee $(FEATDIR)/$(MODELDIR)/traindev-weights.gz.eval
	zcat $(FEATDIR)/$(MODELDIR)/traindev-weights.gz \
		| programs/eval-weights/eval-weights $(EVALWEIGHTS-ARGS) $(FEATDIR)/traindev-feat.gz $(FEATDIR)/f24.gz \
	        | tee $(FEATDIR)/$(MODELDIR)/traindev-weights.gz.eval


# Uses bzip2, which I had fail on me for unknown reasons (when the files were very large)

$(FEATDIR)/train.bz2 $(FEATDIR)/dev.bz2 $(FEATDIR)/train-feat.bz2: $(TREES)/train.bz2 $(TREES)/dev.bz2 $(FEXTRACTOR)
	mkdir -p $(FEATDIR)
	$(FEXTRACTOR) $(FEXTRACTOR-ARGS) $(TREES)/train.bz2 $(FEATDIR)/train.bz2 \
	                                 $(TREES)/dev.bz2 $(FEATDIR)/dev.bz2 \
				         | bzip2 > $(FEATDIR)/train-feat.bz2

$(FEATDIR)/train-feat-sorted.bz2: $(FEATDIR)/train-feat.bz2
	bzcat $(FEATDIR)/train-feat.bz2 | sort -k5,5 -n | bzip2 > $(FEATDIR)/train-feat-sorted.bz2

$(FEATDIR)/$(MODELDIR)/train-weights: $(FEATDIR)/train.bz2 $(FEATDIR)/dev.bz2 $(MESTIMATOR) $(MDEPENDENCIES)
	mkdir -p $(FEATDIR)/$(MODELDIR)
	bzcat $(FEATDIR)/train.bz2 | $(MESTIMATOR) $(MESTIMATOR-ARGS) -e $(FEATDIR)/dev.bz2 \
			-o $(FEATDIR)/$(MODELDIR)/train-weights | tee $(FEATDIR)/$(MODELDIR)/train-run.out

$(FEATDIR)/$(MODELDIR)/train-weights.eval: $(FEATDIR)/$(MODELDIR)/train-weights programs/eval-weights/eval-weights
	programs/eval-weights/eval-weights $(EVALWEIGHTS-ARGS) $(FEATDIR)/train-feat.bz2 $(FEATDIR)/dev.bz2 \
	           < $(FEATDIR)/$(MODELDIR)/train-weights | tee $(FEATDIR)/$(MODELDIR)/train-weights.eval

$(FEATDIR)/$(MODELDIR)/dev-parses.bz2: $(FEATDIR)/$(MODELDIR)/train-weights $(TREES)/dev.bz2 $(FEATDIR)/dev.bz2 programs/eval-weights/best-indices programs/eval-weights/best-parses programs/eval-weights/pretty-print
	programs/eval-weights/best-indices $(FEATDIR)/dev.bz2 < $(FEATDIR)/$(MODELDIR)/train-weights | programs/eval-weights/best-parses $(TREES)/dev.bz2 | sort -nr -k1,1 | programs/eval-weights/pretty-print | bzip2 > $(FEATDIR)/$(MODELDIR)/dev-parses.bz2

$(FEATDIR)/traindev.bz2 $(FEATDIR)/f22.bz2 $(FEATDIR)/f23.bz2 $(FEATDIR)/f24.bz2 $(FEATDIR)/traindev-feat.bz2: $(TREES)/traindev.bz2 $(TREES)/f22.bz2 $(TREES)/f23.bz2 $(TREES)/f24.bz2 $(FEXTRACTOR)
	mkdir -p $(FEATDIR)
	$(FEXTRACTOR) $(FEXTRACTOR-ARGS) $(TREES)/traindev.bz2 $(FEATDIR)/traindev.bz2 \
					 $(TREES)/f22.bz2 $(FEATDIR)/f22.bz2 \
					 $(TREES)/f23.bz2 $(FEATDIR)/f23.bz2 \
					 $(TREES)/f24.bz2 $(FEATDIR)/f24.bz2 \
					 | bzip2 > $(FEATDIR)/traindev-feat.bz2

$(FEATDIR)/$(MODELDIR)/traindev-weights: $(FEATDIR)/traindev.bz2 $(FEATDIR)/traindev-feat.bz2 $(FEATDIR)/f24.bz2 $(MESTIMATOR)
	mkdir -p $(FEATDIR)/$(MODELDIR)
	bzcat $(FEATDIR)/traindev.bz2 | $(MESTIMATOR) $(MESTIMATOR-ARGS) -e $(FEATDIR)/f24.bz2 \
			-o $(FEATDIR)/$(MODELDIR)/traindev-weights | tee $(FEATDIR)/$(MODELDIR)/traindev-run.out

$(FEATDIR)/$(MODELDIR)/gold.eval: $(FEATDIR)/$(MODELDIR)/traindev-weights programs/eval-weights/eval-weights
	programs/eval-weights/eval-weights $(FEATDIR)/traindev-feat.bz2 $(FEATDIR)/f23.bz2 \
	           < $(FEATDIR)/$(MODELDIR)/traindev-weights | tee $(FEATDIR)/$(MODELDIR)/gold-weights.eval

# oracle-eval and oracle-score should both compute the same thing, but use
# different programs

.PHONY: oracle-eval
oracle-eval: programs/wlle/oracle $(FEATDIR)/traindev.bz2 $(FEATDIR)/dev.bz2 $(FEATDIR)/f22.bz2 $(FEATDIR)/f24.bz2
	bzcat $(FEATDIR)/traindev.bz2 | programs/wlle/oracle 
	bzcat $(FEATDIR)/dev.bz2 | programs/wlle/oracle 
	bzcat $(FEATDIR)/f22.bz2 | programs/wlle/oracle 
	bzcat $(FEATDIR)/f24.bz2 | programs/wlle/oracle 

.PHONY: oracle-score
oracle-score: programs/features/oracle-score $(TREES)/train.bz2 $(TREES)/dev.bz2
	bzcat $(TREES)/train.bz2 | programs/features/oracle-score
	bzcat $(TREES)/dev.bz2 | programs/features/oracle-score

.PHONY: eval-beam
eval-beam: programs/eval-beam/main $(TREES)/train.bz2 $(TREES)/dev.bz2
	programs/eval-beam/main $(TREES)/train.bz2
	programs/eval-beam/main $(TREES)/dev.bz2

MICHAELS-TREES = michaels-trees
MICHAELS-FCOUNTS = michaels-fcounts

# .PHONY: programs/wlle/lm
programs/wlle/lm:
	make -C programs wlle/lm

.PHONY: programs/wlle/cvlm
programs/wlle/cvlm:
	make -C programs wlle/cvlm

.PHONY: programs/wlle/avper
programs/wlle/avper:
	make -C programs wlle/avper

.PHONY: programs/wlle/cvwlle
programs/wlle/cvwlle:
	make -C programs wlle/cvwlle

.PHONY: programs/wlle/lnne
programs/wlle/lnne:
	make -C programs wlle/lnne

.PHONY: programs/wlle/oracle
programs/wlle/oracle:
	make -C programs wlle/oracle

.PHONY: programs/wlle/wavper
programs/wlle/wavper:
	make -C programs wlle/wavper

.PHONY: programs/wlle/wlle
programs/wlle/wlle:
	make -C programs wlle/wlle

.PHONY: programs/eval-weights/eval
programs/eval-weights/eval:
	make -C programs eval-weights/eval

.PHONY: programs/eval-weights/eval-weights
programs/eval-weights/eval-weights:
	make -C programs eval-weights/eval-weights

.PHONY: programs/eval-weights/best-indices
programs/eval-weights/best-indices:
	make -C programs eval-weights/best-indices

.PHONY: programs/eval-weights/best-parses
programs/eval-weights/best-parses:
	make -C programs eval-weights/best-parses

.PHONY: programs/eval-weights/pretty-print
programs/eval-weights/pretty-print:
	make -C programs eval-weights/pretty-print

programs/features/extract-features:
	make -C programs features/extract-features

programs/features/extract-spfeatures:
	make -C programs features/extract-spfeatures

programs/features/extract-sspfeatures:
	make -C programs features/extract-sspfeatures

programs/features/extract-spfeatures-ec:
	make -C programs features/extract-spfeatures-ec

programs/features/extract-rule-features:
	make -C programs features/extract-rule-features

programs/features/extract-spfeatures-nomaxcountsub:
	make -C programs features/extract-spfeatures-nomaxcountsub

programs/features/gold-spfeatures:
	make -C programs features/gold-spfeatures

# The prepare-data program needs to know the number of sentences in each
# data file before beginning, as it writes that information at the top
# of each data file.  count-sentences provides that information
#
.PHONY: count-new-sentences
count-new-sentences: programs/prepare-data/count-new-sentences.py
	zcat mc/new-data/allparses.gz | python -O programs/prepare-data/count-new-sentences.py 
	zcat mc/new-data/dev.testParsed.gz | python -O programs/prepare-data/count-new-sentences.py 
	zcat mc/new-data/sec22.nbest.2.gz | python -O programs/prepare-data/count-new-sentences.py 
	zcat mc/new-data/sec23.merged.gz | python -O programs/prepare-data/count-new-sentences.py 
	zcat mc/new-data/sec24.nbest.2.gz | python -O programs/prepare-data/count-new-sentences.py 


.PHONY: count-sentences
count-sentences: programs/prepare-data/count-sentences.py
	zcat mc/data/scored.gz | python -O programs/prepare-data/count-sentences.py 
	zcat mc/data/devscored.gz | python -O programs/prepare-data/count-sentences.py
	zcat mc/data/devscored.gz | python -O programs/prepare-data/count-sentences.py

PARSEXF = mc/scripts/raisePunc_proc_pout.pl


# The following builds the Collins training and development trees

trees/mc/train.bz2: $(PARSEXF) programs/prepare-data/prepare-new-data mc/new-data/allparses.gz mc/new-data/wsj02-21 
	mkdir -p trees
	mkdir -p trees/mc
	programs/prepare-data/prepare-new-data 0 36112 "cat mc/new-data/wsj02-21" "zcat mc/new-data/allparses.gz | $(PARSEXF)" | bzip2 -cz > trees/mc/train.bz2

trees/mc/dev.bz2: mc/new-data/dev.testParsed.gz $(PARSEXF) programs/prepare-data/prepare-new-data
	mkdir -p trees
	mkdir -p trees/mc
	programs/prepare-data/prepare-new-data 36112 3720 "cat mc/new-data/wsj02-21" "zcat mc/new-data/dev.testParsed.gz | $(PARSEXF)"  | bzip2 -cz > trees/mc/dev.bz2

trees/mc/traindev.bz2: mc/new-data/allparses.gz mc/new-data/dev.testParsed.gz $(PARSEXF) programs/prepare-data/prepare-new-data
	mkdir -p trees
	mkdir -p trees/mc
	programs/prepare-data/prepare-new-data 0 39832 "cat mc/new-data/wsj02-21" "zcat mc/new-data/allparses.gz mc/new-data/dev.testParsed.gz | $(PARSEXF)"  | bzip2 -cz > trees/mc/traindev.bz2

trees/mc/f22.bz2: mc/new-data/sec22.nbest.2.gz
	mkdir -p trees
	mkdir -p trees/mc
	programs/prepare-data/prepare-new-data 0 1700 "cat $(WSJ)/22/wsj*.mrg | programs/prepare-data/copy-trees" "zcat mc/new-data/sec22.nbest.2.gz | $(PARSEXF)"  | bzip2 -cz > trees/mc/f22.bz2

trees/mc/f23.bz2: mc/new-data/sec23.merged.gz
	mkdir -p trees
	mkdir -p trees/mc
	programs/prepare-data/prepare-new-data 0 2416 "cat $(WSJ)/23/wsj*.mrg | programs/prepare-data/copy-trees" "zcat mc/new-data/sec23.merged.gz | $(PARSEXF)"  | bzip2 -cz > trees/mc/f23.bz2

trees/mc/f24.bz2: mc/new-data/sec24.nbest.2.gz
	mkdir -p trees
	mkdir -p trees/mc
	programs/prepare-data/prepare-new-data 0 1346 "cat $(WSJ)/24/wsj*.mrg | programs/prepare-data/copy-trees" "zcat mc/new-data/sec24.nbest.2.gz | $(PARSEXF)"  | bzip2 -cz > trees/mc/f24.bz2


# The following builds the Collins SS annotated training and development trees

trees/mc-ss/train.bz2 trees/mc-ss/dev.bz2 trees/mc-ss/traindev.bz2: $(PARSEXF) programs/prepare-data/copy-trees-ss programs/prepare-data/prepare-new-data mc/new-data/allparses.gz mc/new-data/dev.testParsed.gz
	mkdir -p trees
	mkdir -p trees/mc-ss
	$(SSCMD) | grep "^wj " | programs/prepare-data/copy-trees-ss $(PTB) '(cd $(PTB); ls 0[2-9]/*.mrg 1*/*.mrg 2[01]/*.mrg)' > train.tmp
	programs/prepare-data/prepare-new-data 0 36112 "cat train.tmp" "zcat mc/new-data/allparses.gz | $(PARSEXF)" | bzip2 -cz > trees/mc-ss/train.bz2
	programs/prepare-data/prepare-new-data 36112 3720 "cat train.tmp" "zcat mc/new-data/dev.testParsed.gz | $(PARSEXF)"  | bzip2 -cz > trees/mc-ss/dev.bz2
	programs/prepare-data/prepare-new-data 0 39832 "cat train.tmp" "zcat mc/new-data/allparses.gz mc/new-data/dev.testParsed.gz | $(PARSEXF)"  | bzip2 -cz > trees/mc-ss/traindev.bz2
	rm train.tmp

trees/mc-ss/f22.bz2: mc/new-data/sec22.nbest.2.gz
	mkdir -p trees
	mkdir -p trees-ss
	$(SSCMD) | grep "^wj " | programs/prepare-data/copy-trees-ss $(PTB) '(cd $(PTB); ls 22/*.mrg)' > f22.tmp
	programs/prepare-data/prepare-new-data 0 1700 "cat f22.tmp" "zcat mc/new-data/sec22.nbest.2.gz | $(PARSEXF)"  | bzip2 -cz > trees/mc-ss/f22.bz2
	rm f22.tmp

trees/mc-ss/f23.bz2: mc/new-data/sec23.merged.gz
	mkdir -p trees
	mkdir -p trees/mc-ss
	$(SSCMD) | grep "^wj " | programs/prepare-data/copy-trees-ss $(PTB) '(cd $(PTB); ls 23/*.mrg)' > f23.tmp
	programs/prepare-data/prepare-new-data 0 2416 "cat f23.tmp" "zcat mc/new-data/sec23.merged.gz | $(PARSEXF)"  | bzip2 -cz > trees/mc-ss/f23.bz2
	rm f23.tmp

trees/mc-ss/f24.bz2: mc/new-data/sec24.nbest.2.gz
	mkdir -p trees
	mkdir -p trees/mc-ss
	$(SSCMD) | grep "^wj " | programs/prepare-data/copy-trees-ss $(PTB) '(cd $(PTB); ls 24/*.mrg)' > f24.tmp
	programs/prepare-data/prepare-new-data 0 1346 "cat f24.tmp" "zcat mc/new-data/sec24.nbest.2.gz | $(PARSEXF)"  | bzip2 -cz > trees/mc-ss/f24.bz2
	rm f24.tmp


################################################################################################################

programs/prepare-data/copy-trees:
	$(MAKE) -C programs prepare-data/copy-trees

programs/prepare-data/prepare-new-data:
	$(MAKE) -C programs prepare-data/prepare-new-data

programs/prepare-data/prepare-data:
	$(MAKE) -C programs prepare-data/prepare-data

.PHONY: match-ptb
match-ptb: programs/prepare-data/match-ptb
	zcat mc/new-data/allparses.gz | head  -n 1 | cut -d " " -f 9- | $(PARSEXF) | programs/prepare-data/match-ptb "$(WSJ)/*/*.mrg"
	zcat mc/new-data/allparses.gz | tail  -n 1 | cut -d " " -f 9- | $(PARSEXF) | programs/prepare-data/match-ptb "$(WSJ)/*/*.mrg"
	zcat mc/new-data/dev.testParsed.gz | head  -n 1 | cut -d " " -f 9- | $(PARSEXF) | programs/prepare-data/match-ptb "$(WSJ)/*/*.mrg"
	zcat mc/new-data/dev.testParsed.gz | tail  -n 1 | cut -d " " -f 9- | $(PARSEXF) | programs/prepare-data/match-ptb "$(WSJ)/*/*.mrg"

programs/prepare-data/match-ptb:
	$(MAKE) -C programs prepare-data/match-ptb

# The following builds Michael's feature counts

$(MICHAELS-FCOUNTS)/train.bz2 $(MICHAELS-FCOUNTS)/dev.bz2 $(MICHAELS-FCOUNTS)/feat.bz2: $(MICHAELS-TREES)/train.bz2 $(MICHAELS-TREES)/dev.bz2 $(EXTRACTOR)
	mkdir -p $(MICHAELS-FCOUNTS)
	$(EXTRACTOR) $(EXTRACTOR-ARGS) $(MICHAELS-TREES)/train.bz2 $(MICHAELS-TREES)/dev.bz2 $(MICHAELS-FCOUNTS)/train.bz2 $(MICHAELS-FCOUNTS)/dev.bz2 | bzip2 > $(MICHAELS-FCOUNTS)/feat.bz2

# The following builds Michael's training and development trees

$(MICHAELS-TREES)/train.bz2: mc/data/parsed.gz $(PARSEXF) mc/data/scored.gz programs/prepare-data/prepare-data-michael
	mkdir -p $(MICHAELS-TREES)
	programs/prepare-data/prepare-data-michael "$(WSJ)/*/*.mrg" 35540 "zcat mc/data/parsed.gz | $(PARSEXF)" "zcat mc/data/scored.gz" | bzip2 -cz > $(MICHAELS-TREES)/train.bz2

$(MICHAELS-TREES)/dev.bz2: mc/data/devparsed.gz $(PARSEXF) mc/data/devscored.gz programs/prepare-data/prepare-data-michael
	mkdir -p $(MICHAELS-TREES)
	programs/prepare-data/prepare-data-michael "$(WSJ)/*/*.mrg" 3676 "zcat mc/data/devparsed.gz | $(PARSEXF)" "zcat mc/data/devscored.gz" | bzip2 -cz > $(MICHAELS-TREES)/dev.bz2

programs/prepare-data/prepare-data-michael:
	$(MAKE) -C programs prepare-data/prepare-data-michael


.PHONY: clean
clean:
	rm -f *~
	$(MAKE) -C programs clean

.PHONY: real-clean
real-clean: clean
	rm -fr trees trees-ss $(FCOUNTS) $(MICHAELS-TREES) $(MICHAELS-FCOUNTS)
	$(MAKE) -C programs real-clean
