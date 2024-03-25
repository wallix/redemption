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

#include "ppocr/strategies/density.hpp"

#include "ppocr/image/image.hpp"

namespace ppocr { namespace strategies {

density::value_type density::load(const Image& img, const Image& /*img90*/, ctx_type& /*ctx*/)
{
    unsigned count = 0;
    auto first = img.data();
    auto last = img.data_end();
    for (; first != last; ++first) {
        if (is_pix_letter(*first)) {
            ++count;
        }
    }
    return count * 100 / img.area();
}

} }
