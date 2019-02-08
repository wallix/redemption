/*
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

Product name: redemption, a FLOSS RDP proxy
Copyright (C) Wallix 2010-2018
Author(s): Jonathan Poelen
*/

#include "redjs/image_data_from_pointer.hpp"

#include "core/RDP/rdp_pointer.hpp"
#include "utils/drawable_pointer.hpp"

#include "utils/hexdump.hpp"

namespace redjs
{

ImageData image_data_from_pointer(Pointer const& pointer)
{
    auto const dimensions = pointer.get_dimensions();

    uint8_t* pdata = new uint8_t[dimensions.width * dimensions.height * 4]{};
    ImageData img{dimensions.width, dimensions.height, std::unique_ptr<uint8_t[]>(pdata)};

    DrawablePointer drawable_pointer;
    drawable_pointer.initialize(
        dimensions.width, dimensions.height,
        pointer.get_24bits_xor_mask().data(),
        pointer.get_monochrome_and_mask().data());

    for (DrawablePointer::ContiguousPixels const & contiguous_pixels : drawable_pointer.contiguous_pixels_view()) {
        uint8_t* data = pdata + (contiguous_pixels.y * img.width() + contiguous_pixels.x) * 4;
        uint8_t const* pixel_data = contiguous_pixels.data;
        uint8_t const* pixel_end = pixel_data + contiguous_pixels.data_size;
        hexdump(pixel_data, contiguous_pixels.data_size);
        while (pixel_data < pixel_end) {
            *data++ = *pixel_data++;
            *data++ = *pixel_data++;
            *data++ = *pixel_data++;
            *data++ = 255;
        }
    }

    LOG(LOG_DEBUG, "%u %u", img.width(), img.height());
    hexdump(pdata, img.size(), img.width() * 4);

    return img;
}

}
