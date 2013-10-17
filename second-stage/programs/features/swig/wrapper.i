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

// vi: syntax=cpp
%module SWIGReranker

/* SWIG includes */
%include "std_except.i"
%include "std_vector.i"
%include "std_string.i"
%include "exception.i"

#ifdef SWIGPYTHON
// Python-ify some of these names
%rename(__str__) toString;
%rename(__len__) nparses;
#endif

#ifdef SWIGJAVA
%rename(size) nparses;
#endif

%rename(NBestList) sp_sentence_type;

%exception {
    try {
        $action
    } catch (RerankerError re) {
        SWIG_exception(SWIG_RuntimeError, re.description.c_str());
    }
}

%newobject readNBestList;
%newobject scoreNBestList;

%inline {
    #include <cstddef>

    class RerankerError {
        public:
            const std::string description;
            RerankerError(const std::string msg);
    };

    typedef unsigned int size_type;
    typedef size_type Id;

    typedef double Float; // potentially confusing, but this is the notation
    typedef std::vector<Float> Weights;

    struct sp_sentence_type {
        size_t nparses() const;
    };
    sp_sentence_type* readNBestList(const std::string nbest_list, bool lowercase);

    class RerankerModel {
        public:
            Id maxid;

            RerankerModel(const char* feature_class,
                    const char* feature_ids_filename,
                    const char* feature_weights_filename);
            Weights* scoreNBestList(const sp_sentence_type& nbest_list) const;
    };

    void setOptions(int debug, bool abs_counts);
}

%template(Weights) std::vector<Float>;
