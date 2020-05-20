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


namespace redjs
{

namespace
{
    bool is_empty_mask(u8_array_view av) noexcept
    {
        for (uint8_t x : av) {
            if (x != 0xff) {
                return false;
            }
        }
        return true;
    }

    void apply_reversed_color(uint8_t* data, uint8_t const* pointer_data) noexcept
    {
        if (!data[3]) {
            data[0] = 0xFFu - pointer_data[0];
            data[1] = 0xFFu - pointer_data[1];
            data[2] = 0xFFu - pointer_data[2];
            data[3] = 255;
        }
    }

    void apply_color(uint8_t* data, uint8_t const* pointer_data)
    {
        data[0] = pointer_data[0];
        data[1] = pointer_data[1];
        data[2] = pointer_data[2];
        data[3] = 255;
    }

    void apply_transparency(uint8_t* data)
    {
        // ignore invisible pixels
        // data[0] = 0;
        // data[1] = 0;
        // data[2] = 0;
        data[3] = 0;
    }
}

ImageData image_data_from_pointer(Pointer const& pointer)
{
    auto const dimensions = pointer.get_dimensions();
    auto const av_and_1byte = pointer.get_monochrome_and_mask();
    auto const av_xor_mask = pointer.get_24bits_xor_mask();
    bool const is_empty_mask = redjs::is_empty_mask(av_and_1byte);
    auto const width = dimensions.width + is_empty_mask * 2;
    auto const height = dimensions.height + is_empty_mask * 2;
    auto const d3 = dimensions.width * 3;
    auto const w4 = width * 4;
    auto const decrement_next_line = w4 * 2 - is_empty_mask * 8;

    uint8_t* pdata = is_empty_mask
        // zero initialization
        ? new uint8_t[width * height * 4]{}
        // default initialization (apply_transparency is used)
        : new uint8_t[width * height * 4];
    ImageData img{width, height, std::unique_ptr<uint8_t[]>(pdata)};
    pdata += width * height * 4 - w4 + is_empty_mask * (4 - w4);

    auto for_each_pixel = [&](auto f)
    {
        for (auto pointer_data = av_xor_mask.begin(), pointer_data_end = av_xor_mask.end();
             pointer_data < pointer_data_end;
        )
        {
            for (auto pointer_data_end_line = pointer_data + d3;
                pointer_data < pointer_data_end_line; void(pdata += 4), pointer_data += 3
            )
            {
                f(pdata, pointer_data);
            }
            // pointer_data is aligned on 2 bytes
            pointer_data += (d3 & 1);
            pdata -= decrement_next_line;
        }
    };

    if (is_empty_mask)
    {
        for_each_pixel([&](uint8_t* data, uint8_t const* pointer_data)
        {
            if (pointer_data[0] || pointer_data[1] || pointer_data[2])
            {
                apply_color(data, pointer_data);

                // border top
                apply_reversed_color(data - w4 - 4, data);
                apply_reversed_color(data - w4, data);
                apply_reversed_color(data - w4 + 4, data);
                // border left
                apply_reversed_color(data - 4, data);
                // border right
                apply_reversed_color(data + 4, data);
                // border bottom
                apply_reversed_color(data + w4 - 4, data);
                apply_reversed_color(data + w4, data);
                apply_reversed_color(data + w4 + 4, data);
            }
        });
    }
    else
    {
        unsigned i = 0;
        for_each_pixel([&](uint8_t* data, uint8_t const* pointer_data)
        {
            if (!(av_and_1byte[i / 8u] & (1u << (7u - (i % 8u)))))
            {
                apply_color(data, pointer_data);
            }
            else
            {
                apply_transparency(data);
            }
            ++i;
        });
    }

    return img;
}

}
