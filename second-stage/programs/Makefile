# Makefile for ~/research/rerank/programs
#
# Mark Johnson, 15th November 2005

export CFLAGS
export CXXFLAGS
export LDFLAGS

.PHONY: all
all:
	make -C eval-beam
	make -C eval-weights
	make -C features
	make -C prepare-data
	make -C wlle

.PHONY: clean
clean:
	rm -fr *~
	make -C eval-beam clean
	make -C eval-weights clean
	make -C features clean
	make -C prepare-data clean
	make -C wlle clean

.PHONY: real-clean
real-clean:
	make -C eval-beam real-clean
	make -C eval-weights real-clean
	make -C features real-clean
	make -C prepare-data real-clean
	make -C wlle real-clean
