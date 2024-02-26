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

#include <algorithm>
#include <map>


namespace ppocr { namespace filters {

using std::size_t;

template<class FwIt>
typename std::iterator_traits<FwIt>::value_type
best_baseline(FwIt first, FwIt last)
{
    using value_type = typename std::iterator_traits<FwIt>::value_type;
    if (first == last) {
        return ~value_type{};
    }
    std::map<value_type, size_t> ys;
    for (; first != last; ++first) {
        ++ys[*first];
    }
    using cP = std::pair<value_type, size_t> const;
    return std::max_element(ys.begin(), ys.end(), [](cP & a, cP & b) {
        return a.second < b.second;
    })->first;
}

} }

#endif
