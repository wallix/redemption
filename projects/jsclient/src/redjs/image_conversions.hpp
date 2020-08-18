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

#include <cstdint>
#include <cstddef>

class BGRPalette;
enum class BitsPerPixel : uint8_t;

namespace redjs
{
    void convert_bitmap_to_image_data(
        uint8_t* dest,
        uint8_t const* bmp_data, uint16_t cx, uint16_t cy,
        std::size_t line_size, BitsPerPixel bits_per_pixel,
        BGRPalette const* palette);

    void transform_bitmap15_to_bitmap16(
        uint8_t* bmp_data, uint16_t cy, std::size_t line_size);

    void convert_bitmap8_to_bitmap16(
        uint8_t* dest,
        uint8_t const* bmp_data, uint16_t cy,
        std::size_t line_size, BGRPalette const& palette);
}
