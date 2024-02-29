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

#include "ppocr/image/image_from_string.hpp"

#include <cassert>

namespace ppocr {

Image image_from_string(const Bounds& bnd, const char * pix_data)
{
    assert(pix_data[bnd.area()] == 0 && (pix_data[bnd.area()-1] == '-' || pix_data[bnd.area()-1] == 'x'));

    return Image(bnd, [pix_data, &bnd]{
        unsigned const area = bnd.area();
        PtrImageData data(new Pixel[area]);
        std::copy(pix_data, pix_data+area, data.get());
        return data;
    }());
}

}
