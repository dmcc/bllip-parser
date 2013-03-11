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

# Makefile for ~/research/rerank/programs
#
# Mark Johnson, 15th November 2005

export CFLAGS
export CXXFLAGS
export LDFLAGS

.PHONY: all
all:
	$(MAKE) -C eval-beam
	$(MAKE) -C eval-weights
	$(MAKE) -C features
	$(MAKE) -C prepare-data
	$(MAKE) -C wlle

.PHONY: clean
clean:
	rm -fr *~
	$(MAKE) -C eval-beam clean
	$(MAKE) -C eval-weights clean
	$(MAKE) -C features clean
	$(MAKE) -C prepare-data clean
	$(MAKE) -C wlle clean

.PHONY: real-clean
real-clean:
	$(MAKE) -C eval-beam real-clean
	$(MAKE) -C eval-weights real-clean
	$(MAKE) -C features real-clean
	$(MAKE) -C prepare-data real-clean
	$(MAKE) -C wlle real-clean
