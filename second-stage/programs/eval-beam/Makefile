SOURCES = main.cc heads.cc sym.cc
OBJECTS = $(patsubst %.l,%.o,$(patsubst %.c,%.o,$(SOURCES:%.cc=%.o)))

main: heads.o main.o sym.o
	$(CXX) $(LDFLAGS) $^ -o $@

read-tree.cc: read-tree.l
	flex -oread-tree.cc read-tree.l

.PHONY: 
clean: 
	rm -fr *.o *.d core main read-tree.cc 

.PHONY: real-clean
real-clean: clean
	rm -fr *~ main

# this command tells GNU make to look for dependencies in *.d files
-include $(patsubst %.l,%.d,$(patsubst %.c,%.d,$(SOURCES:%.cc=%.d)))
