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
