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


namespace redjs
{

namespace
{
    bool is_empty_mask(array_view_const_u8 av) noexcept
    {
        for (uint8_t x : av) {
            if (x != 0xff) {
                return false;
            }
        }
        return true;
    }

    void apply_reversed_color(uint8_t* data, uint8_t const* pixel_data) noexcept
    {
        if (!data[3]) {
            data[0] = 0xFFu - pixel_data[0];
            data[1] = 0xFFu - pixel_data[1];
            data[2] = 0xFFu - pixel_data[2];
            data[3] = 255;
        }
    }
}

ImageData image_data_from_pointer(Pointer const& pointer)
{
    auto const dimensions = pointer.get_dimensions();

    uint8_t* pdata = new uint8_t[dimensions.width * dimensions.height * 4]{};
    ImageData img{dimensions.width, dimensions.height, std::unique_ptr<uint8_t[]>(pdata)};

    if (is_empty_mask(pointer.get_monochrome_and_mask()))
    {
        auto const w3 = dimensions.width * 3;
        auto const w4 = dimensions.width * 4;
        auto const av_xor = pointer.get_24bits_xor_mask();
        uint8_t* data = pdata + av_xor.size() / 3 * 4 - dimensions.width * 4;
        for (auto pixel_data = av_xor.begin(), pixel_data_end = av_xor.end();
             pixel_data < pixel_data_end;
        ) {
            auto pixel_data_begin_line = pixel_data;
            for (auto pixel_data_end_line = pixel_data + dimensions.width * 3;
                pixel_data < pixel_data_end_line; void(data += 4), pixel_data += 3
            ) {
                if (pixel_data[0] || pixel_data[1] || pixel_data[2]) {
                    data[0] = pixel_data[0];
                    data[1] = pixel_data[1];
                    data[2] = pixel_data[2];
                    data[3] = 255;

                    // border left
                    if (pixel_data - 3 >= pixel_data_begin_line) {
                        apply_reversed_color(data - 4, pixel_data - 3);
                    }

                    // border right
                    if (pixel_data + 3 < pixel_data_end_line) {
                        apply_reversed_color(data + 4, pixel_data + 3);
                    }

                    // border top
                    if (pixel_data - w3 >= av_xor.begin()) {
                        apply_reversed_color(data - w4, pixel_data - w3);
                        if (pixel_data - 3 >= pixel_data_begin_line) {
                            apply_reversed_color(data - 4 + w4, pixel_data - 3 - w3);
                        }
                        if (pixel_data + 3 < pixel_data_end_line) {
                            apply_reversed_color(data + 4 + w4, pixel_data + 3 - w3);
                        }
                    }

                    // border bottom
                    if (pixel_data + w3 < av_xor.end()) {
                        apply_reversed_color(data + w4, pixel_data + w3);
                        if (pixel_data - 3 >= pixel_data_begin_line) {
                            apply_reversed_color(data - 4 - w4, pixel_data - 3 + w3);
                        }
                        if (pixel_data + 3 < pixel_data_end_line) {
                            apply_reversed_color(data + 4 - w4, pixel_data + 3 + w3);
                        }
                    }
                }
            }
            data -= dimensions.width * 4 * 2;
        }
    }
    else
    {
        DrawablePointer drawable_pointer;
        drawable_pointer.initialize(
            dimensions.width, dimensions.height,
            pointer.get_24bits_xor_mask().data(),
            pointer.get_monochrome_and_mask().data());

        for (DrawablePointer::ContiguousPixels const & contiguous_pixels : drawable_pointer.contiguous_pixels_view()) {
            uint8_t* data = pdata + (contiguous_pixels.y * img.width() + contiguous_pixels.x) * 4;
            uint8_t const* pixel_data = contiguous_pixels.data;
            uint8_t const* pixel_end = pixel_data + contiguous_pixels.data_size;
            while (pixel_data < pixel_end) {
                *data++ = *pixel_data++;
                *data++ = *pixel_data++;
                *data++ = *pixel_data++;
                *data++ = 255;
            }
        }
    }

    return img;
}

}
