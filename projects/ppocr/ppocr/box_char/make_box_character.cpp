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

#include "ppocr/box_char/box.hpp"
#include "ppocr/box_char/make_box_character.hpp"
#include "ppocr/image/image.hpp"

namespace ppocr {

namespace utils {
    bool horizontal_empty(Pixel const * d, unsigned w) {
        for (auto e = d+w; d != e; ++d) {
            if (is_pix_letter(*d)) {
                return false;
            }
        }
        return true;
    }

    bool vertical_empty(Pixel const * d, Bounds const & bnd) {
        for (auto e = d + bnd.area(); d != e; d += bnd.width()) {
            if (is_pix_letter(*d)) {
                return false;
            }
        }
        return true;
    }
}


Box make_box_character(Image const & image, Index const & idx, Bounds const & bnd)
{
    unsigned x = idx.x();

    auto d = image.data({x, idx.y()});
    for (; x < bnd.width(); ++x) {
        if (!utils::vertical_empty(d, bnd)) {
            break;
        }
        ++d;
    }

    unsigned w = x;

    while (w + 1 < bnd.width()) {
        ++w;
        if ([&image](Pixel const * d, unsigned w, unsigned h) -> bool {
            for (auto e = d+w*h; d != e; d += w) {
                if (is_pix_letter(*d) && (
                    (d+1 != e && is_pix_letter(*(d+1)))
                 || (d-w+1 >= image.data() && is_pix_letter(*(d-w+1)))
                 || (d+w+1 < e && is_pix_letter(*(d+w+1)))
                )) {
                    return false;
                }
            }
            return true;
        }(d, bnd.width(), bnd.height())) {
            break;
        }
        ++d;
    }
    w -= x;

    unsigned y = idx.y();

    d = image.data({x, y});
    for (; y < bnd.height(); ++y) {
        if (!utils::horizontal_empty(d, w)) {
            break;
        }
        d += bnd.width();
    }

    unsigned h = bnd.height();

    d = image.data() + image.to_size_t({x, h});
    while (--h > y) {
        d -= bnd.width();
        if (!utils::horizontal_empty(d, w)) {
            break;
        }
    }
    h -= y;

    ++h;
    return {{x, y}, {w, h}};
}

}
