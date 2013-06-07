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

#/////////////////////////////////////////////////////////////////////
# makefile for Charniak parser TRAIN dir
#/////////////////////////////////////////////////////////////////////

CFLAGS = -fPIC -O3 -Wall
# CFLAGS = -fPIC -g -Wall

default: all

# this rule automatically makes our dependency files.
# run "make Makefile.dep" if you add any files or change dependencies.
Makefile.dep:
	$(CXX) -MM *.C > Makefile.dep

# include the automatically generated dependency files
-include Makefile.dep

#---------------------------------------------------------------------
# Build Training programs

.C.o:
	$(CXX) $(CFLAGS) -c $<
 
RCOUNTS_OBJS = \
	ClassRule.o \
	ECArgs.o \
	EmpNums.o \
	Feat.o \
	Feature.o \
	FeatureTree.o \
	InputTree.o \
	Pst.o \
	Phegt.o \
	Term.o \
	auxify.o \
	ccInd.o \
	headFinder.o \
	headFinderCh.o \
	treeHistSf.o \
	utils.o \
	rCounts.o
rCounts: $(RCOUNTS_OBJS)
	$(CXX) $(CFLAGS) $(RCOUNTS_OBJS) -o rCounts 

ISCALE_OBJS = \
	ECArgs.o \
	Feat.o \
	Feature.o \
	FeatureTree.o \
	FeatIter.o \
	FeatTreeIter.o \
	Phegt.o \
	Term.o \
	utils.o \
	iScale.o
iScale: $(ISCALE_OBJS)
	$(CXX) $(CFLAGS) $(ISCALE_OBJS) -o iScale 
 
SELFEATS_OBJS = \
	ECArgs.o \
	Feat.o \
	Feature.o \
	FeatureTree.o \
	FeatIter.o \
	FeatTreeIter.o \
	Pst.o \
	Phegt.o \
	Term.o \
	utils.o \
	selFeats.o
selFeats: $(SELFEATS_OBJS)
	$(CXX) $(CFLAGS) $(SELFEATS_OBJS) -o selFeats 

 
TRAINRS_OBJS = \
	trainRsUtils.o \
	ClassRule.o \
	ECArgs.o \
	EmpNums.o \
	Feat.o \
	Feature.o \
	FeatureTree.o \
	InputTree.o \
	Pst.o \
	Phegt.o \
	Smoother.o \
	Term.o \
	auxify.o \
	ccInd.o \
	headFinder.o \
	headFinderCh.o \
	treeHistSf.o \
	utils.o \
	trainRs.o
trainRs: $(TRAINRS_OBJS)
	$(CXX) $(CFLAGS) $(TRAINRS_OBJS) -o trainRs 

 
KN3COUNTS_OBJS = \
	ClassRule.o \
	ECArgs.o \
	EmpNums.o \
	Feat.o \
	Feature.o \
	FeatIter.o \
	FeatTreeIter.o \
	FeatureTree.o \
	InputTree.o \
	Pst.o \
	Phegt.o \
	Term.o \
	auxify.o \
	ccInd.o \
	headFinder.o \
	headFinderCh.o \
	treeHistSf.o \
	utils.o \
	kn3Counts.o
kn3Counts: $(KN3COUNTS_OBJS)
	$(CXX) $(CFLAGS) $(KN3COUNTS_OBJS) -o kn3Counts 

AUXIT_OBJS = \
	BrownIter.o \
	EmpNums.o \
	ECArgs.o \
	InputTree.o \
	Term.o \
	headFinder.o \
	headFinderCh.o \
	utils.o \
	auxify.o \
	auxIt.o
auxIt: $(AUXIT_OBJS)
	$(CXX) $(AUXIT_OBJS) -o auxIt

PSGT_OBJS = \
	ECArgs.o \
	EmpNums.o \
	InputTree.o \
	Term.o \
	auxify.o \
	headFinder.o \
	headFinderCh.o \
	utils.o \
	UnitRules.o \
	pSgT.o
pSgT: $(PSGT_OBJS)
	$(CXX) $(PSGT_OBJS) -o pSgT


PTGNT_OBJS = \
	ECArgs.o \
	EmpNums.o \
	InputTree.o \
	Pst.o \
	Phegt.o \
	Term.o \
	headFinder.o \
	headFinderCh.o \
	utils.o \
	pTgNt.o
pTgNt: $(PTGNT_OBJS)
	$(CXX) $(PTGNT_OBJS) -o pTgNt

PSFGT_OBJS = \
	ECArgs.o \
	EmpNums.o \
	InputTree.o \
	Pst.o \
	Phegt.o \
	Term.o \
	headFinder.o \
	headFinderCh.o \
	utils.o \
	pSfgT.o
pSfgT: $(PSFGT_OBJS)
	$(CXX) $(PSFGT_OBJS) -o pSfgT

PUGT_OBJS = \
	ECArgs.o \
	EmpNums.o \
	InputTree.o \
	Pst.o \
	Phegt.o \
	Term.o \
	auxify.o \
	headFinder.o \
	headFinderCh.o \
	utils.o \
	pUgT.o
pUgT: $(PUGT_OBJS)
	$(CXX) $(PUGT_OBJS) -o pUgT

GETPROBS_OBJS = \
	ClassRule.o \
	ECArgs.o \
	EmpNums.o \
	Feat.o \
	Feature.o \
	FeatureTree.o \
	InputTree.o \
	Pst.o \
	Phegt.o \
	Smoother.o \
	Term.o \
	auxify.o \
	ccInd.o \
	headFinder.o \
	headFinderCh.o \
	treeHistSf.o \
	utils.o \
	trainRsUtils.o \
	getProbs.o
getProbs:$(GETPROBS_OBJS)
	$(CXX) $(CFLAGS) $(GETPROBS_OBJS) -o getProbs 

all: rCounts selFeats iScale trainRs pSgT pTgNt pUgT kn3Counts pSfgT 

clean: 
	rm -f *.o rCounts selFeats iScale trainRs pSgT pTgNt pUgT kn3Counts pSfgT

.PHONY: real-clean
real-clean: clean
	rm -f *.d Makefile.dep TAGS tags
