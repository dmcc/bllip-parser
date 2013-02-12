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
