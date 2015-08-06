#! /bin/sh
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

# This is an example of how to run syntactic parse fusion in the pipeline.
# You may want to change the fusion parameters for other parsing models.

PARSERMODEL=first-stage/DATA/EN/
MODELDIR=second-stage/models/ec50spfinal
ESTIMATORNICKNAME=cvlm-l1c10P1
first-stage/PARSE/parseIt -l399 -N50 $PARSERMODEL $* | second-stage/programs/features/best-parses -l $MODELDIR/features.gz -m1 $MODELDIR/$ESTIMATORNICKNAME-weights.gz | first-stage/PARSE/fusion $PARSERMODEL -n40 -t0.44 -e1.2
