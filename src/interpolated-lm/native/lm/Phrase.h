//
// Created by david on 31.10.16.
//

#ifndef ILM_PHRASE_H
#define ILM_PHRASE_H

#include <vector>

#include <mmt/sentence.h>
#include "Options.h"

using namespace std;

namespace mmt {
    namespace ilm {
        // TODO: fixed size MMT_ILM_MAX_ORDER
        class Phrase : public vector<wid_t> {
        public:
            Phrase() = default;
            Phrase(size_t n) : vector<wid_t>(n) {}
        private:
            //wid_t data[MMT_ILM_MAX_ORDER];
        };
    } // namespace ilm
} // namespace mmt

#endif //ILM_PHRASE_H
