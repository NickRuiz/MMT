//
// Created by david on 31.10.16.
//

#ifndef ILM_PHRASE_H
#define ILM_PHRASE_H

#include <vector>
#include <cstring>
#include <cassert>
#include <algorithm>

#include <mmt/sentence.h>
#include "Options.h"

using namespace std;

namespace mmt {
    namespace ilm {
        /** fixed-size, stack allocated replacement for std::vector */
        class Phrase {
        public:
            typedef wid_t * iterator;
            typedef const wid_t * const_iterator;

            Phrase() : size_(0) {}
            Phrase(size_t n) : size_(n) { memset(data_, 0, sizeof(data_)); }

            size_t size() const { return size_; }
            bool empty() const { return size_ == 0; }
            wid_t &operator[](size_t i) { return data_[i]; }
            const wid_t &operator[](size_t i) const { return data_[i]; }
            wid_t at(size_t i) const { return data_[i]; }
            bool operator==(const Phrase &other) const { return size_ == other.size_ && !memcmp(data_, other.data_, size_); }

            iterator begin() { return data_; }
            iterator end() { return data_ + size_; }
            const_iterator begin() const { return data_; }
            const_iterator end() const { return data_ + size_; }

            void push_back(wid_t w) { assert(size_ < MMT_ILM_MAX_ORDER-1); data_[size_++] = w; }
            void resize(size_t new_size) { assert(new_size <= MMT_ILM_MAX_ORDER-1); size_ = new_size; }

        private:
            size_t size_;
            wid_t data_[MMT_ILM_MAX_ORDER-1];
        };
    } // namespace ilm
} // namespace mmt

#endif //ILM_PHRASE_H
