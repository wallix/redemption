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

#ifndef PPOCR_SRC_OCR2_DATA_INDEXES_ORDERED_HPP
#define PPOCR_SRC_OCR2_DATA_INDEXES_ORDERED_HPP

#include "ppocr/ocr2/indexes_ordered.hpp"
#include "ppocr/loader2/datas_loader.hpp"

namespace ppocr { namespace ocr2 {

template<class LoaderStrategy>
struct DataIndexesOrdered
{
    using Data = ppocr::loader2::Data<LoaderStrategy>;
    static_assert(Data::is_contiguous::value, "is not contiguous");

    DataIndexesOrdered(Data const & data)
    : indexes_ordered(data)
    {}

    template<class... Ts>
    DataIndexesOrdered(ppocr::loader2::Datas<Ts...> const & datas)
    : indexes_ordered(datas.template get<LoaderStrategy>())
    {}

    ::ppocr::range_iterator<typename ppocr::ocr2::IndexesOrdered::iterator>
    get_range(Data const & data, unsigned x, unsigned d) const {
        return ppocr::ocr2::range_bounds(
            data,
            this->indexes_ordered,
            x > d ? x-d : 0u,
            x + d
        );
    }

    template<class... Ts>
    ::ppocr::range_iterator<typename ppocr::ocr2::IndexesOrdered::iterator>
    get_range(ppocr::loader2::Datas<Ts...> const & datas, unsigned x, unsigned d) const {
        return this->get_range(datas.template get<LoaderStrategy>(), x, d);
    }

private:
    IndexesOrdered const indexes_ordered;
};

} }

#endif
