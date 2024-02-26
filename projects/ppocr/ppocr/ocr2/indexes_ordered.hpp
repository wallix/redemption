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

#ifndef PPOCR_SRC_OCR2_INDEXES_ORDERED_HPP
#define PPOCR_SRC_OCR2_INDEXES_ORDERED_HPP

#include "ppocr/utils/range_iterator.hpp"

#include <memory>
#include <numeric>
#include <algorithm>

namespace ppocr { namespace ocr2 {

struct IndexesOrdered
{
    IndexesOrdered() = default;
    IndexesOrdered(IndexesOrdered &&) = default;
    IndexesOrdered(IndexesOrdered const &) = delete;

    template<class Cont>
    IndexesOrdered(Cont const & data)
    : indexes_(new unsigned[data.size()])
    {
        std::iota(this->indexes_.get(), this->indexes_.get()+ data.size(), 0u);
        std::sort(this->indexes_.get(), this->indexes_.get()+ data.size(), [&](unsigned i1, unsigned i2) {
            return data[i1] < data[i2];
        });
    }

    using iterator = unsigned const *;
    using const_iterator = iterator;

    iterator begin() const { return this->indexes_.get(); }

private:
    std::unique_ptr<unsigned[]> indexes_;
};


template<class Data, class T>
::ppocr::range_iterator<typename IndexesOrdered::iterator>
range_bounds(Data const & data, IndexesOrdered const & indexes, T const & start, T const & stop) {
    auto first = std::lower_bound(
        indexes.begin(),
        indexes.begin() + data.size(),
        start,
        [&](unsigned i, T const & x) {
            return data[i] < x;
        }
    );
    return {first, std::upper_bound(
        first,
        indexes.begin() + data.size(),
        stop,
        [&](T const & x, unsigned i) {
            return x < data[i];
        }
    )};
}

} }

#endif
