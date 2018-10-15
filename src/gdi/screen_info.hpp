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

#pragma once

#include "utils/sugar/std_stream_proto.hpp"
#include "utils/sugar/cast.hpp"

#include <cstdint>
#include <cassert>


enum class BitsPerPixel : uint8_t {
    BitsPP1 = 1,
    BitsPP4 = 4,
    BitsPP8 = 8,
    BitsPP15 = 15,
    BitsPP16 = 16,
    BitsPP24 = 24,
    BitsPP32 = 32,
};
enum class BytesPerPixel : uint8_t;
// enum class Width : uint16_t;
// enum class Height : uint16_t;

REDEMPTION_OSTREAM(out_name, BitsPerPixel bpp) { return out_name << underlying_cast(bpp); }
REDEMPTION_OSTREAM(out_name, BytesPerPixel Bpp) { return out_name << underlying_cast(Bpp); }
// REDEMPTION_OSTREAM(out_name, Width width) { return out_name << underlying_cast(width); }
// REDEMPTION_OSTREAM(out_name, Height height) { return out_name << underlying_cast(height); }


constexpr bool is_normalized_bits_per_pixel(BitsPerPixel nb_bits) noexcept
{
    return nb_bits == BitsPerPixel{8}
        || nb_bits == BitsPerPixel{15}
        || nb_bits == BitsPerPixel{16}
        || nb_bits == BitsPerPixel{24}
        || nb_bits == BitsPerPixel{32}
    ;
}

constexpr bool is_normalized_bytes_per_pixel(BytesPerPixel nb_bytes) noexcept
{
    return nb_bytes == BytesPerPixel{1}
        || nb_bytes == BytesPerPixel{2}
        || nb_bytes == BytesPerPixel{3}
        || nb_bytes == BytesPerPixel{4}
    ;
}

constexpr BytesPerPixel to_bytes_per_pixel(BitsPerPixel nb_bits) noexcept
{
    assert(is_normalized_bits_per_pixel(nb_bits));
    return BytesPerPixel((uint8_t(nb_bits) + 7) / 8);
}

constexpr BitsPerPixel to_bits_per_pixel(BytesPerPixel nb_bytes) noexcept
{
    assert(is_normalized_bytes_per_pixel(nb_bytes));
    return BitsPerPixel(uint8_t(nb_bytes) * 8);
}

constexpr uint8_t nb_bytes_per_pixel(BytesPerPixel nb_bits) noexcept
{
    return uint8_t(nb_bits);
}

constexpr uint8_t nb_bytes_per_pixel(BitsPerPixel nb_bits) noexcept
{
    return uint8_t(to_bytes_per_pixel(nb_bits));
}

constexpr uint8_t nb_bits_per_pixel(BitsPerPixel nb_bytes) noexcept
{
    return uint8_t(nb_bytes);
}

constexpr uint8_t nb_bits_per_pixel(BytesPerPixel nb_bytes) noexcept
{
    return uint8_t(to_bits_per_pixel(nb_bytes));
}

struct ScreenInfo
{
    BitsPerPixel bpp {};
    // Width width {};
    // Height height {};
    uint16_t width {};
    uint16_t height {};
};
