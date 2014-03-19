# Licensed under the Apache License, Version 2.0 (the "License"); you may
# not use this file except in compliance with the License.  You may obtain
# a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
# License for the specific language governing permissions and limitations
# under the License.

TARGETS = copy-trees-ss prepare-new-data prepare-ec-data ptb

top: $(TARGETS)

# CXX = g++
# CC = $(CXX)
# CFLAGS = -MMD -O3 -Wall -ffast-math -finline-functions -fstrict-aliasing $(GCCFLAGS)
# CFLAGS = -g -MMD -Wall


# CXXFLAGS = -I $(BOOST_DIR) $(CXXFLAGS) 
#
# debugging
#
# CFLAGS = -g -O -MMD -Wall -ffast-math -fno-inline -fstrict-aliasing -march=pentium4
# LDFLAGS = -g
#
# profiling
#
# CFLAGS = -g -pg -MMD -Wall -ffast-math -fno-default-inline -fno-inline -fstrict-aliasing -march=pentium4
# LDFLAGS = -g -pg

SOURCES = copy-trees.cc copy-trees-ss.cc prepare-data.cc prepare-ec-data.cc prepare-ec-data100.cc prepare-data-michael.cc prepare-new-data.cc ptb.cc read-tree.l sym.cc

OBJECTS = $(patsubst %.l,%.o,$(patsubst %.c,%.o,$(SOURCES:%.cc=%.o)))

copy-trees: copy-trees.o read-tree.o sym.o
	$(CXX) $(LDFLAGS) $^ -o $@

copy-trees-ss: copy-trees-ss.o read-tree.o sym.o
	$(CXX) $(LDFLAGS) $^ -o $@

prepare-ec-data: prepare-ec-data.o sym.o
	$(CXX) $(LDFLAGS) $^ -o $@

prepare-ec-data100: prepare-ec-data100.o sym.o
	$(CXX) $(LDFLAGS) $^ -o $@

prepare-new-data: prepare-new-data.o read-tree.o sym.o
	$(CXX) $(LDFLAGS) $^ -o $@

prepare-data: prepare-data.o read-tree.o sym.o
	$(CXX) $(LDFLAGS) $^ -o $@

prepare-data-michael: prepare-data-michael.o read-tree.o sym.o
	$(CXX) $(LDFLAGS) $^ -o $@

ptb: ptb.o sym.o
	$(CXX) $(LDFLAGS) $^ -o $@

read-tree.cc: read-tree.l
	flex -oread-tree.cc read-tree.l

.PHONY: 
clean: 
	rm -fr *.o *.d core read-tree.cc *~ 

.PHONY: real-clean
real-clean: clean 
	rm -fr prepare-data prepare-data-michael prepare-new-data

# this command tells GNU make to look for dependencies in *.d files
-include $(patsubst %.l,%.d,$(patsubst %.c,%.d,$(SOURCES:%.cc=%.d)))
