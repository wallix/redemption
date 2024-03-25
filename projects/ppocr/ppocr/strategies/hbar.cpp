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

#include "ppocr/strategies/hbar.hpp"
#include "ppocr/image/image.hpp"

#include <algorithm>

namespace ppocr { namespace strategies {

hbar::value_type hbar::load(const Image& img, const Image& /*img90*/, ctx_type& /*ctx*/)
{
    hbar::value_type n{};
    auto p = img.data();
    auto const e = img.data_end();
    bool previous_is_plain = false;
    for (; p != e; p += img.width()) {
        if (std::all_of(p, p+img.width(), is_pix_letter_fn())) {
            if (!previous_is_plain) {
                previous_is_plain = true;
                ++n;
            }
        }
        else {
            previous_is_plain = false;
        }
    }
    return n;
}

} }
