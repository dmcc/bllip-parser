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

import SWIGReranker as reranker

def dir_contents():
    print 'reranker contents:', dir(reranker)
    print

# use included reranker model
model_dir = '../../models/ec50spfinal/'
features_filename = model_dir + 'features.gz'
weights_filename = model_dir + 'cvlm-l1c10P1-weights.gz'

reranker.setOptions(100000, True)

nbest_list_text = """
2	1
-58.4143
(S1 (S (NP (DT This)) (VP (AUX is) (NP (DT an) (JJ n-best) (NN list))) (. .)))
-61.6705
(S1 (S (NP (DT This)) (VP (AUX is) (NP (DT an) (JJS n-best) (NN list))) (. .)))
""".strip()

if __name__ == "__main__":
    dir_contents()
    print "loading"
    model = reranker.RerankerModel(None, features_filename, weights_filename)
    print 'loaded model', model
    print 'model contents:', dir(model)
    print model.maxid

    print 'reading'
    nbest_list = reranker.readNBestList(nbest_list_text, False)
    print 'nbest_list', nbest_list
    print 'nbest_list contents:', dir(nbest_list)
    print 'nbest_list parses:', len(nbest_list)

    scores = model.scoreNBestList(nbest_list)
    print 'scores', scores
    print 'scores', scores[0]
    print 'scores', list(scores)
