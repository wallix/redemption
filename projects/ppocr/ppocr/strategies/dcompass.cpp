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

#include "ppocr/strategies/dcompass.hpp"
#include "ppocr/image/image.hpp"

#include "ppocr/strategies/utils/relationship.hpp"
#include "ppocr/strategies/utils/cardinal_direction_io.hpp"

#include <ostream>
#include <istream>


namespace ppocr { namespace strategies {

static unsigned count_dcompass(Bounds const & bnd, Pixel const * p, Pixel const * ep, bool is_top)
{
    unsigned d = 0;
    size_t ih = 0;
    size_t const wdiv2 = bnd.w()/2;
    for (; p != ep; p += bnd.w(), ++ih) {
        size_t x = wdiv2 - bnd.w() / (!is_top ? bnd.h() - ih : 1 + ih) / 2;
        auto leftp = p + x;
        auto rightp = p + bnd.w() - x;
        for (; leftp != rightp; ++leftp) {
            if (is_pix_letter(*leftp)) {
                ++d;
            }
        }
    }
    return d;
}

static int horizontal_dcompass(const Image& img)
{
    Bounds const bnd(img.width(), img.height() / 2);
    auto p = img.data();
    auto ep = img.data({0, bnd.h()});
    long const top = count_dcompass(bnd, p, ep, true);
    p = ep;
    if (img.height() & 1) {
        p += img.width();
    }
    long const bottom = count_dcompass(bnd, p, img.data_end(), false);

    return top < bottom ? 1 : top > bottom ? 3 : 2;
}

dcompass::dcompass(const Image& img, const Image& img90)
: d(static_cast<cardinal_direction>(horizontal_dcompass(img) | horizontal_dcompass(img90) << 2))
{}

unsigned dcompass::relationship(const dcompass& other) const
{ return utils::cardinal_relationship(d, other.d); }

std::istream& operator>>(std::istream& is, dcompass& d)
{ return is >> d.d; }

std::ostream& operator<<(std::ostream& os, const dcompass& d)
{ return os << d.direction(); }

} }
