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

#include <cassert>
#include <cstdlib>
#include <vector>
#include <string>

#include <sstream>
#include <iostream>

#include "popen.h"
#include "sp-data.h"
#include "features.h"

#include "simple-api.h"

// externed variables
int debug_level = 0;
bool absolute_counts = true;
bool collect_correct = false;
bool collect_incorrect = false;

RerankerError::RerankerError(const std::string msg) {
    this->description = msg;
}

void setOptions(int debug, bool abs_counts) {
    debug_level = debug;
    absolute_counts = abs_counts;
}

RerankerModel::RerankerModel(const char* feature_class,
        const char* feature_ids_filename,
        const char* feature_weights_filename) {
    fcps = new FeatureClassPtrs(feature_class);

    if (!std::ifstream(feature_ids_filename).good()) {
        throw RerankerError("Can't open feature IDs file.");
    }
    izstream fdin(feature_ids_filename);
    maxid = fcps->read_feature_ids(fdin);

    izstream fwin(feature_weights_filename);
    if (!std::ifstream(feature_weights_filename).good()) {
        throw RerankerError("Can't open feature weights file.");
    }

    weights = new Weights(maxid + 1);
    Id id;
    Float weight;
    while (fwin >> id >> "=" >> weight) {
        assert(id <= maxid);
        assert((*weights)[id] == 0);
        (*weights)[id] = weight;
    }
}

Weights*
RerankerModel::scoreNBestList(const sp_sentence_type& nbest_list) const {
    Id_Floats p_i_v(nbest_list.nparses());
    cforeach (FeatureClassPtrs, it, *fcps)
        (*it)->feature_values(nbest_list, p_i_v);

    Weights* parse_scores = new Weights();

    for (size_type i = 0; i < nbest_list.nparses(); ++i) {
        const Id_Float& i_v = p_i_v[i];

        Float w = 0;
        cforeach (Id_Float, ivit, i_v) {
            assert(ivit->first < weights->size());
            w += ivit->second * (*weights)[ivit->first];
        }
        parse_scores->push_back(w);
    }

    return parse_scores;
}

sp_sentence_type* readNBestList(const std::string nbest_list, bool lowercase) {
    std::stringstream text(nbest_list);
    sp_sentence_type* s = new sp_sentence_type();
    s->read(text, lowercase);

    return s;
}
