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

#ifndef PPOCR_STRATEGIES_UTILS_HORIZONTAL_DIRECTION_HPP
#define PPOCR_STRATEGIES_UTILS_HORIZONTAL_DIRECTION_HPP

#include "ppocr/image/image.hpp"
#include "ppocr/strategies/utils/top_bottom.hpp"

namespace ppocr { namespace strategies { namespace utils {

struct horizontal_direction_fn
{
    TopBottom value() const
    {
        return top_bottom;
    }

    void compute(const Image& img);

private:
    TopBottom top_bottom;
};

inline
void horizontal_direction_fn::compute(const Image& img)
{
    top_bottom.top = 0;
    top_bottom.bottom = 0;
    auto p = img.data();
    for (auto ep = img.data({0, img.height() / 2}); p != ep; ++p) {
        if (is_pix_letter(*p)) {
            ++top_bottom.top;
        }
    }
    if (img.height() & 1) {
        p += img.width();
    }
    for (auto ep = img.data_end(); p != ep; ++p) {
        if (is_pix_letter(*p)) {
            ++top_bottom.bottom;
        }
    }
}

} } }

#endif
