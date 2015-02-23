/*
 * Copyright 1999, 2005 Brown University, Providence, RI.
 *
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

#include "SimpleAPI.h"

Params params;

int main(int argc, char *argv[]) {
    ECArgs args(argc, argv);
    params.init(args);

    ECString path(args.arg(0));
    generalInit(path);

    // we don't use sentenceCount since it treeLogProb may parse the
    // sentence multiple times
    int index = 0;
    while (true) {
        if (!cin) {
            break;
        }
        InputTree correct;
        cin >> correct;
        int len = correct.length();
        if (len == 0) {
            break;
        }
        if (len > params.maxSentLen) {
            continue;
        }
        double logProb;
        try {
            logProb = treeLogProb(&correct);
        } catch (ParserError) {
            logProb = 0;
        }
        cout << index << "\t" << logProb << endl;
        index++;
    }
    return 0;
}
