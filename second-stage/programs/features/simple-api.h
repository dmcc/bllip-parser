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

#pragma once

class RerankerError {
    public:
        std::string description;

        RerankerError(const std::string msg);
};

typedef std::vector<Float> Weights;

void setOptions(int debug, bool abs_counts);

class RerankerModel {
    public:
        Id maxid;
        FeatureClassPtrs* fcps;
        Weights* weights;

        RerankerModel(const char* feature_class,
                const char* feature_ids_filename,
                const char* feature_weights_filename);

        Weights* scoreNBestList(const sp_sentence_type& nbest_list) const;
};

sp_sentence_type* readNBestList(const std::string nbest_list, bool lowercase);
