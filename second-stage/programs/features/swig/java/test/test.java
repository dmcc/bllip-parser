/*
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.  You may obtain
 * a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

import java.util.ArrayList;
import java.util.List;

public class test {
    static {
        System.loadLibrary("SWIGReranker");
    }

    public static void main(String argv[]) throws Exception {
        System.out.println("top of main");
        SWIGReranker.setOptions(0, true);
        String modelDir = "../../models/ec50spfinal/";
        String featuresFilename = modelDir + "features.gz";
        String weightsFilename = modelDir + "cvlm-l1c10P1-weights.gz";
        System.out.println("loading");
        RerankerModel reranker = new RerankerModel(null, featuresFilename, weightsFilename);
        System.out.println("number of features: " + reranker.getMaxid());
        System.out.println("loaded");

        String nbestListText = "2	1\n-58.4143\n(S1 (S (NP (DT This)) (VP (AUX is) (NP (DT an) (JJ n-best) (NN list))) (. .)))\n-61.6705\n(S1 (S (NP (DT This)) (VP (AUX is) (NP (DT an) (JJS n-best) (NN list))) (. .)))\n";

        NBestList nbestList = SWIGReranker.readNBestList(nbestListText, false);
        Weights scores = reranker.scoreNBestList(nbestList);
        for (int i = 0; i < scores.size(); i++) {
            System.out.format("score %d: %s\n", i, scores.get(i));
        }
    }
}
