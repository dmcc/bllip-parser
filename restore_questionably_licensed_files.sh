#!/bin/sh
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

(cd second-stage/programs/wlle && \
    wget -nc "http://bitbucket.org/bllip/bllip-parser/raw/79845e3f6d7b27289bbdc36cf962d315331e8f93/second-stage/programs/wlle/OWLQN.cpp" && \
    wget -nc "http://bitbucket.org/bllip/bllip-parser/raw/79845e3f6d7b27289bbdc36cf962d315331e8f93/second-stage/programs/wlle/OWLQN.h" && \
    wget -nc "http://bitbucket.org/bllip/bllip-parser/raw/79845e3f6d7b27289bbdc36cf962d315331e8f93/second-stage/programs/wlle/TerminationCriterion.cpp" && \
    wget -nc "http://bitbucket.org/bllip/bllip-parser/raw/79845e3f6d7b27289bbdc36cf962d315331e8f93/second-stage/programs/wlle/TerminationCriterion.h" && \
    wget -nc "http://bitbucket.org/bllip/bllip-parser/raw/79845e3f6d7b27289bbdc36cf962d315331e8f93/second-stage/programs/wlle/cvlm-owlqn.cc" && \
    wget -nc "http://bitbucket.org/bllip/bllip-parser/raw/79845e3f6d7b27289bbdc36cf962d315331e8f93/second-stage/programs/wlle/lm-owlqn.cc" && \
    wget -nc "http://bitbucket.org/bllip/bllip-parser/raw/79845e3f6d7b27289bbdc36cf962d315331e8f93/second-stage/programs/wlle/hlm.cc")

(cd second-stage/programs/eval-weights && \
    wget -nc "http://bitbucket.org/bllip/bllip-parser/raw/79845e3f6d7b27289bbdc36cf962d315331e8f93/second-stage/programs/eval-weights/cephes.c" && \
    wget -nc "http://bitbucket.org/bllip/bllip-parser/raw/79845e3f6d7b27289bbdc36cf962d315331e8f93/second-stage/programs/eval-weights/cephes.h" && \
    wget -nc "http://bitbucket.org/bllip/bllip-parser/raw/79845e3f6d7b27289bbdc36cf962d315331e8f93/second-stage/programs/eval-weights/compare-models.cc")
