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

#include "ppocr/ocr2/filter_by_font.hpp"

#include <map>

unsigned ppocr::ocr2::filter_by_font(ppocr::ocr2::ambiguous_t & ambiguous)
{
    std::map<unsigned, unsigned> fonts;
    for (view_ref_list & vec : ambiguous) {
        if (vec.size() == 1) {
            ++fonts[vec[0].get().font];
        }
    }
    if (!fonts.empty()) {
        using cP = decltype(fonts)::value_type const;
        auto font_it = std::max_element(fonts.begin(), fonts.end(), [](cP & a, cP & b) {
            return a.second < b.second;
        });
        if (fonts.size() == 1 || font_it->second >= ambiguous.size()/2) {
            for (view_ref_list & vec : ambiguous) {
                if (vec.size() >= 2) {
                    auto pos = std::find_if(
                        vec.begin(), vec.end(),
                        [&](View const & view) { return view.font == font_it->first; }
                    );
                    vec = {(pos != vec.end()) ? *pos : vec[0]};
                }
            }
        }
        return font_it->first;
    }
    return ~0u;
}
