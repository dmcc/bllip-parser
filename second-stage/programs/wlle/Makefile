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

SOURCES = avper.cc cvlm-owlqn.cc hlm.cc gavper.cc lm.cc lmdata.c oracle.cc wavper.cc wlle.cc OWLQN.cpp TerminationCriterion.cpp # cvlm.cc
TARGETS = avper gavper oracle # cvlm lm oracle wavper cvlm-owlqn hlm
OBJECTS = $(patsubst %.cpp,%.o,$(patsubst %.l,%.o,$(patsubst %.c,%.o,$(SOURCES:%.cc=%.o))))

all: $(TARGETS)

lm: liblmdata.a lm.o
	$(CXX) $(LDFLAGS) lm.o liblmdata.a $(TAO_PETSC_LIBS) -o lm

lm-owlqn: lm-owlqn.o OWLQN.o TerminationCriterion.o liblmdata.a
	$(CXX) $(LDFLAGS) $^ -o lm-owlqn

cvlm: liblmdata.a cvlm.o
	$(CXX) $(LDFLAGS) cvlm.o liblmdata.a $(TAO_PETSC_LIBS) -o cvlm

cvlm-nolib: cvlm.o lmdata.o
	$(CXX) $(LDFLAGS) cvlm.o lmdata.o $(TAO_PETSC_LIBS) -o cvlm-nolib

cvlm-owlqn: cvlm-owlqn.o OWLQN.o TerminationCriterion.o liblmdata.a
	$(CXX) $(LDFLAGS) $^ -o cvlm-owlqn

hlm: hlm.o OWLQN.o TerminationCriterion.o liblmdata.a
	$(CXX) $(LDFLAGS) $^ -o $@

lnne: libdata.a lnne.o 
	$(CXX) $(LDFLAGS) lnne.o libdata.a $(TAO_PETSC_LIBS) -o lnne

avper: avper.o liblmdata.a
	$(CXX) $(LDFLAGS) $^ -o $@ 

gavper: gavper.o liblmdata.a 
	$(CXX) $(LDFLAGS) $^ -o $@ 

wavper: wavper.o liblmdata.a
	$(CXX) $(LDFLAGS) $^ -o $@ 

wlle: libdata.a wlle.o
	$(CXX) $(LDFLAGS) wlle.o libdata.a $(TAO_PETSC_LIBS) -o wlle

cvwlle: libdata.a cvwlle.o
	$(CXX) $(LDFLAGS) cvwlle.o libdata.a $(TAO_PETSC_LIBS) -o cvwlle

oracle: liblmdata.a oracle.o
	$(CXX) $(LDFLAGS) oracle.o liblmdata.a -o oracle

libdata.a: data.o
	ar rcv libdata.a data.o; ranlib libdata.a

liblmdata.a: lmdata.o
	ar rcv liblmdata.a lmdata.o; ranlib liblmdata.a

# TAO stuff

# PETSC_DIR = /usr/local/share/petsc
# PETSC_ARCH = linux
# TAO_DIR = /usr/local/share/tao
# BOPT = O_c++

TAO_PETSC_INCLUDE = -I${PETSC_DIR} -I${PETSC_DIR}/bmake/$(PETSC_ARCH) -I${PETSC_DIR}/include -I${PETSC_DIR}/include/mpiuni -I${TAO_DIR} -I${TAO_DIR}/include -I${PETSC_DIR}/$(PETSC_ARCH)/include

# TAO_PETSC_LIBS = -L${TAO_DIR}/lib/${PETSC_ARCH} -ltaopetsc -ltao -Wl,-rpath,${TAO_DIR}/lib/${PETSC_ARCH} -Wl,-rpath,${PETSC_DIR}/lib/${PETSC_ARCH} -L${PETSC_DIR}/lib/${PETSC_ARCH} -lpetscsnes -lpetscksp -lpetscdm -lpetscmat -lpetscvec -lpetsc   -Wl,-rpath,${PETSC_DIR}/lib/${PETSC_ARCH} -L${PETSC_DIR}/lib/${PETSC_ARCH} -lmpiuni -L${LLAPACK_DIR} -llapack -lblas -lm -lstdc++ -lgcc_s
# TAO_PETSC_LIBS = -L${TAO_DIR}/lib/${PETSC_ARCH} -ltaopetsc -ltao -Wl,-rpath,${TAO_DIR}/lib/${PETSC_ARCH} -Wl,-rpath,${PETSC_DIR}/lib/${PETSC_ARCH} -L${PETSC_DIR}/lib/${PETSC_ARCH} -lpetscsnes -lpetscksp -lpetscdm -lpetscmat -lpetscvec -lpetsc   -Wl,-rpath,${PETSC_DIR}/lib/${PETSC_ARCH} -L${PETSC_DIR}/lib/${PETSC_ARCH} -lmpiuni -llapack -lblas -lm -lstdc++ -lgcc_s
# TAO_PETSC_LIBS = -L${TAO_DIR}/lib/${PETSC_ARCH} -ltaopetsc -ltao -Wl,-rpath,${TAO_DIR}/lib/${PETSC_ARCH} -Wl,-rpath,${PETSC_DIR}/lib/${PETSC_ARCH} -L${PETSC_DIR}/lib/${PETSC_ARCH} -lpetscsnes -lpetscksp -lpetscdm -lpetscmat -lpetscvec -lpetsc -Wl,-rpath,${PETSC_DIR}/lib/${PETSC_ARCH} -L${PETSC_DIR}/lib/${PETSC_ARCH} -lmpiuni -lm -lstdc++ -lgcc_s
TAO_PETSC_LIBS = -L${TAO_DIR}/lib/${PETSC_ARCH} -ltaopetsc -ltao -Wl,-rpath,${TAO_DIR}/lib/${PETSC_ARCH} -Wl,-rpath,${PETSC_DIR}/lib/${PETSC_ARCH} -L${PETSC_DIR}/lib/${PETSC_ARCH} -L${PETSC_DIR}/${PETSC_ARCH}/lib -lpetscsnes -lpetscksp -lpetscdm -lpetscmat -lpetscvec -lpetsc -Wl,-rpath,${PETSC_DIR}/lib/${PETSC_ARCH} -Wl,-rpath,${PETSC_DIR}/${PETSC_ARCH}/lib -L${PETSC_DIR}/lib/${PETSC_ARCH} -lmpiuni -lm -lstdc++ -lgcc_s

# end of TAO stuff

# Compilation help: you may need to remove -march=native on older compilers.
GCCFLAGS=-march=native -mfpmath=sse -msse2 -mmmx
FOPENMP=-fopenmp

CC=gcc
CFLAGS=-MMD -O6 -ffast-math -fstrict-aliasing -Wall -finline-functions $(GCCFLAGS) $(FOPENMP)
LDFLAGS=$(FOPENMP)
CXXFLAGS=${CFLAGS} ${TAO_PETSC_INCLUDE} -Wno-deprecated

# FOPENMP=
# CFLAGS=-MMD -O1 -g $(GCCFLAGS) $(FOPENMP)
# LDFLAGS=-g $(FOPENMP)
# CXXFLAGS=${CFLAGS} ${TAO_PETSC_INCLUDE} -Wno-deprecated

.PHONY: real-clean
real-clean: clean

.PHONY: clean
clean:
	rm -fr *.a *.d *.o *~ $(TARGETS)

# this command tells GNU make to look for dependencies in *.d files
-include $(patsubst %.cpp,%.d,$(patsubst %.l,%.d,$(patsubst %.c,%.d,$(SOURCES:%.cc=%.d))))
