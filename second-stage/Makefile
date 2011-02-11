# Makefile for BLLIP/reranking-parser/second-stage
#
# Mark Johnson, 15th November 2005
#

export CFLAGS
export CXXFLAGS
export LDFLAGS

.PHONY: all
all:
	$(MAKE) -C programs

.PHONY: clean
clean:
	rm -fr *~ nbest features eval
	$(MAKE) -C programs clean

.PHONY: train-clean
train-clean:
	rm -fr nbest
	rm -fr features
	rm -fr eval

.PHONY: real-clean
real-clean:
	rm -fr models
	$(MAKE) -C programs real-clean
