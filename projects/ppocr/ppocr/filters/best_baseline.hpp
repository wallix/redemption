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

#ifndef PPOCR_SRC_FILTERS_BEST_BASELINE_HPP
#define PPOCR_SRC_FILTERS_BEST_BASELINE_HPP

#include <type_traits>
#include <algorithm>


namespace ppocr { namespace filters {

template<class RandomIt>
inline unsigned best_baseline(RandomIt first, RandomIt last)
{
    if (first == last) {
        return ~0u;
    }

    auto max_value = *std::max_element(first, last);
    static_assert(std::is_same_v<decltype(max_value), unsigned>);

    std::vector<unsigned> ys(max_value + 1, 0);
    for (; first != last; ++first) {
        ++ys[*first];
    }

    return static_cast<unsigned>(std::max_element(ys.begin(), ys.end()) - ys.begin());
}

} }

#endif
