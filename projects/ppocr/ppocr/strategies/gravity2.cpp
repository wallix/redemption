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

#include "ppocr/strategies/gravity2.hpp"
#include "ppocr/image/image.hpp"

#include "ppocr/strategies/utils/relationship.hpp"
#include "ppocr/strategies/utils/horizontal_gravity.hpp"

#include <ostream>
#include <istream>

#include <cassert>


namespace ppocr {namespace strategies {

static unsigned horizontal_gravity2(const Image& img)
{
    utils::TopBottom g = utils::horizontal_gravity(img);

    size_t const hby2 = img.height()/2;
    int const gimg = ((hby2 * (hby2 + 1)) / 2) * img.width();
    unsigned ret = (gimg ? (int(g.top) - int(g.bottom)) * 100 / gimg : 0) + 100;

    assert(ret <= 200);
    return ret;
}

gravity2::gravity2(const Image& img, const Image& img90)
: d(horizontal_gravity2(img) | (horizontal_gravity2(img90) << 9))
{
    static_assert(sizeof(int) > 9*3/9, "short type");
}

unsigned gravity2::relationship(const gravity2& other) const
{ return utils::mask_relationship(d, other.d, (1 << 9)-1, 9, 400); }

std::istream& operator>>(std::istream& is, gravity2& d)
{ return is >> d.d; }

std::ostream& operator<<(std::ostream& os, const gravity2& d)
{ return os << d.id(); }

} }
