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

#ifndef PPOCR_SRC_STRATEGIES_UTILS_DIAGONAL_VERTICAL_GRAVITY_HPP
#define PPOCR_SRC_STRATEGIES_UTILS_DIAGONAL_VERTICAL_GRAVITY_HPP

#include "ppocr/image/image.hpp"
#include "ppocr/strategies/utils/top_bottom.hpp"

namespace ppocr { namespace strategies { namespace utils {

namespace details_ {
    inline unsigned count_diagonal_vertical_gravity(
        Bounds const & bnd, Pixel const * p, Pixel const * ep, bool is_top
    ) {
        unsigned g = 0;
        unsigned coef = is_top ? bnd.height() : 1;
        unsigned ih = 0;
        unsigned const wdiv2 = bnd.width()/2;
        for (; p != ep; p += bnd.width(), ++ih) {
            unsigned x = wdiv2 - bnd.width() / (!is_top ? bnd.height() - ih : 1 + ih) / 2;
            auto leftp = p + x;
            auto rightp = p + bnd.width() - x;
            for (; leftp != rightp; ++leftp) {
                if (is_pix_letter(*leftp)) {
                    g += coef;
                }
            }
            (is_top ? --coef : ++coef);
        }
        return g;
    }
}

inline unsigned diagonal_vertical_gravity_area(const Image& img)
{
    Bounds const bnd(img.width(), img.height() / 2);
    auto p = img.data();
    auto ep = img.data({0, bnd.height()});

    unsigned area = 0;
    unsigned ih = 0;
    unsigned const wdiv2 = bnd.width()/2;
    for (; p != ep; p += bnd.width(), ++ih) {
        unsigned x = wdiv2 - bnd.width() / (bnd.height() - ih) / 2;
        area += (bnd.width() - x*2) * (ih+1);
    }
    return area * 2;
}

struct diagonal_vertical_gravity_fn
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
void diagonal_vertical_gravity_fn::compute(const Image& img)
{
    Bounds const bnd(img.width(), img.height() / 2);
    auto p = img.data();
    auto ep = img.data({0, bnd.height()});
    top_bottom.top = details_::count_diagonal_vertical_gravity(bnd, p, ep, true);

    p = ep;
    if (img.height() & 1) {
        p += img.width();
    }
    top_bottom.bottom = details_::count_diagonal_vertical_gravity(bnd, p, img.data_end(), false);
}

} } }

#endif
