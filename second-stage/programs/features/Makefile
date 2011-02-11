TARGETS = best-parses best-splhparses extract-spfeatures extract-splhfeatures extract-nfeatures oracle-score
SOURCES = best-parses.cc best-splhparses.cc extract-nfeatures.cc extract-splhfeatures.cc extract-spfeatures.cc heads.cc read-tree.l sym.cc oracle-score.cc
OBJECTS = $(patsubst %.l,%.o,$(patsubst %.c,%.o,$(SOURCES:%.cc=%.o)))

top: $(TARGETS)

parallel-tools: count-spfeatures count-nfeatures parallel-extract-nfeatures parallel-extract-spfeatures

all: top parallel-tools

extract-spfeatures: extract-spfeatures.o heads.o read-tree.o sym.o
	$(CXX) $(LDFLAGS) $^ -o $@

extract-splhfeatures: extract-splhfeatures.o heads.o read-tree.o sym.o
	$(CXX) $(LDFLAGS) $^ -o $@

extract-nfeatures: extract-nfeatures.o heads.o read-tree.o sym.o
	$(CXX) $(LDFLAGS) $^ -o $@

best-parses: best-parses.o heads.o read-tree.o sym.o
	$(CXX) $(LDFLAGS) $^ -o $@

best-splhparses: best-splhparses.o heads.o read-tree.o sym.o
	$(CXX) $(LDFLAGS) $^ -o $@

oracle-score: oracle-score.o read-tree.o sym.o
	$(CXX) $(LDFLAGS) $^ -o $@

count-spfeatures: count-spfeatures.o heads.o read-tree.o sym.o
	$(CXX) $(LDFLAGS) $^ -o $@

parallel-extract-spfeatures: parallel-extract-spfeatures.o heads.o read-tree.o sym.o
	$(CXX) $(LDFLAGS) $^ -o $@

count-nfeatures: count-nfeatures.o heads.o read-tree.o sym.o
	$(CXX) $(LDFLAGS) $^ -o $@

parallel-extract-nfeatures: parallel-extract-nfeatures.o heads.o read-tree.o sym.o
	$(CXX) $(LDFLAGS) $^ -o $@

read-tree.cc: read-tree.l
	flex -oread-tree.cc read-tree.l

.PHONY: 
clean: 
	rm -fr *.o *.d *~ core read-tree.cc 

.PHONY: real-clean
real-clean: clean 
	rm -fr $(TARGETS)

# this command tells GNU make to look for dependencies in *.d files
-include $(patsubst %.l,%.d,$(patsubst %.c,%.d,$(SOURCES:%.cc=%.d)))
