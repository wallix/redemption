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

#ifndef PPOCR_UTILS_IMAGE_COMPARE_HPP
#define PPOCR_UTILS_IMAGE_COMPARE_HPP

#include <functional>

namespace ppocr {

class Image;

int image_compare(Image const & a, Image const & b);

}

namespace std {
    template<>
    struct less< ::ppocr::Image>
    {
        constexpr less() noexcept {}

        bool operator()(::ppocr::Image const & a, ::ppocr::Image const & b) const
        { return ::ppocr::image_compare(a, b) < 0; }
    };
}

namespace ppocr {

using image_less = std::less<Image>;

}

#endif
