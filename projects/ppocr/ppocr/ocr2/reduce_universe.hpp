/*
* Copyright (C) 2016 Wallix
* 
* This library is free software; you can redistribute it and/or modify it under
* the terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; either version 2.1 of the License, or (at your option)
* any later version.
* 
* This library is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
* details.
* 
* You should have received a copy of the GNU Lesser General Public License along
* with this library; if not, write to the Free Software Foundation, Inc., 59
* Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#ifndef PPOCR_SRC_OCR2_REDUCE_UNIVERSE_HPP
#define PPOCR_SRC_OCR2_REDUCE_UNIVERSE_HPP

#include "probabilities.hpp"
#include "ppocr/loader2/datas_loader.hpp"
#include "ppocr/ocr2/data_indexes_by_words.hpp"

#include <algorithm>

namespace ppocr { namespace ocr2 {

template<class Strategy, class T, class Dist>
void reduce_universe_with_distance(
    ocr2::Probabilities & a,
    loader2::Data<Strategy> const & data,
    T const & value,
    Dist const & d
) {
    a.resize(std::remove_if(a.begin(), a.end(), [&](ocr2::Probability const & prop) {
        return data.get_relationship().in_dist(value, data[prop.i], d);
    }) - a.begin());
}

template<class Strategy, class T>
void reduce_universe_and_update_probability(
    ocr2::Probabilities & probabilities,
    loader2::Data<Strategy> const & data,
    T const & value,
    double prob_limit
) {
    auto it = probabilities.begin();
    for (auto & prob : probabilities) {
        auto const x = data.dist(data[prob.i], value);
        if (x >= prob_limit) {
            *it = {prob.i, prob.prob * x};
            ++it;
        }
    }
    probabilities.resize(it - probabilities.begin());
}

template<class Predicate>
void reduce_universe_by_word(
    ocr2::Probabilities & probabilities,
    ocr2::DataIndexesByWords const & data_indexes_by_words,
    Predicate predicate
) {
    auto it = probabilities.begin();
    for (auto & prob : probabilities) {
        if (data_indexes_by_words[prob.i].empty()) {
            *it = prob;
            ++it;
        }
        else {
            for (auto i : data_indexes_by_words[prob.i]) {
                if (predicate(i)) {
                    *it = prob;
                    ++it;
                    break;
                }
            }
        }
    }
    probabilities.resize(it - probabilities.begin());
}

} }

#endif
