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

#ifndef PPOCR_SRC_OCR2_SORT_PROBABILITIES_HPP
#define PPOCR_SRC_OCR2_SORT_PROBABILITIES_HPP

#include "probabilities.hpp"

#include <algorithm>


namespace ppocr { namespace ocr2 {

struct EqProbByViews {
    std::vector<unsigned> const & id_views;
    bool operator()(ppocr::ocr2::Probability const & a, ppocr::ocr2::Probability const & b) const {
        return id_views[a.i] == id_views[b.i];
    }
};

struct LtProbByViews {
    std::vector<unsigned> const & id_views;
    bool operator()(ppocr::ocr2::Probability const & a, ppocr::ocr2::Probability const & b) const {
        if (id_views[a.i] == id_views[b.i]) {
            return a.prob > b.prob;
        }
        return id_views[a.i] < id_views[b.i];
    }
};

inline void unique_copy_by_views(
    ppocr::ocr2::Probabilities & out,
    ppocr::ocr2::Probabilities const & probabilities,
    std::vector<unsigned> const & id_views
) {
    out.resize(std::unique_copy(
        probabilities.begin(),
        probabilities.end(),
        out.begin(),
        ppocr::ocr2::EqProbByViews{id_views}
    ) - out.begin());
}

inline void unique_by_views(ppocr::ocr2::Probabilities & probabilities, std::vector<unsigned> const & id_views
) {
    probabilities.resize(
        std::unique(probabilities.begin(), probabilities.end(), EqProbByViews{id_views})
      - probabilities.begin()
    );
}

inline void sort_by_views(ppocr::ocr2::Probabilities & probabilities, std::vector<unsigned> const & id_views) {
    std::sort(probabilities.begin(), probabilities.end(), LtProbByViews{id_views});
}

inline void sort_by_prop(ppocr::ocr2::Probabilities & probabilities) {
    std::sort(probabilities.begin(), probabilities.end(), ppocr::ocr2::GtProb{});
}

} }

#endif
