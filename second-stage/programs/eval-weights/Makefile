SOURCES = best-indices.cc best-parse.cc best-parses.cc compare-models.cc data.c eval-weights.cc lmdata.c pretty-print.cc read-tree.l sym.cc
TARGETS = eval-weights # best-indices best-parse best-parses compare-models pretty-print
OBJECTS = $(patsubst %.l,%.o,$(patsubst %.c,%.o,$(SOURCES:%.cc=%.o)))

CC = gcc

all: $(TARGETS)

compare-models: lmdata.o cephes.o compare-models.o
	$(CXX) $(LDFLAGS) $^ -o $@

eval-weights: lmdata.o eval-weights.o
	$(CXX) $(LDFLAGS) $^ -o $@ 

best-indices: data.o best-indices.o
	$(CXX) $(LDFLAGS) $^ -o $@

best-parse: best-parse.o read-tree.o sym.o
	$(CXX)  $(LDFLAGS) $^ -o $@

best-parses: best-parses.o read-tree.o sym.o
	$(CXX)  $(LDFLAGS) $^ -o $@

pretty-print: pretty-print.o read-tree.o sym.o
	$(CXX) $(LDFLAGS) $^ -o $@

read-tree.o: read-tree.cc
	$(CXX) $(CXXFLAGS) -c -o read-tree.o read-tree.cc

read-tree.cc: read-tree.l
	flex -oread-tree.cc read-tree.l

.PHONY: real-clean
real-clean: clean
	rm -fr $(TARGETS)

.PHONY: clean
clean:
	rm -fr *.d *.o *~ core read-tree.cc

# this command tells GNU make to look for dependencies in *.d files
-include $(patsubst %.l,%.d,$(patsubst %.c,%.d,$(SOURCES:%.cc=%.d)))
