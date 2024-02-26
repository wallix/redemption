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

#ifndef PPOCR_MAKE_BOX_CHARACTER_HPP
#define PPOCR_MAKE_BOX_CHARACTER_HPP

#include "ppocr/box_char/box.hpp"
#include "ppocr/image/pixel.hpp"

namespace ppocr {

class Image;
class Bounds;

namespace utils {
    bool horizontal_empty(Pixel const * d, std::size_t w);
    bool vertical_empty(Pixel const * d, Bounds const & bnd);
}

Box make_box_character(Image const & image, Index const & idx, Bounds const & bnd);

}

#endif
