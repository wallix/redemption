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

#include "redjs/image_conversions.hpp"

#include "red_emscripten/bind.hpp"
#include "red_emscripten/val.hpp"

#include "utils/bitmap.hpp"
#include "utils/colors.hpp"
#include "utils/pixel_conversion.hpp"


namespace redjs
{

void convert_bitmap_to_image_data(
    uint8_t* dest,
    uint8_t const* bmp_data, uint16_t cx, uint16_t cy,
    std::size_t line_size, BitsPerPixel bits_per_pixel,
    BGRPalette const* palette)
{
    auto convert = [&](auto buf_to_color, auto dec) -> void
    {
        ptrdiff_t const src_nbbytes = nb_bytes_per_pixel(bits_per_pixel);
        ptrdiff_t const step = ptrdiff_t(line_size + std::size_t(cx * src_nbbytes));
        uint8_t const* src = bmp_data + line_size * (cy - 1);
        uint8_t const* end = bmp_data + cx * src_nbbytes;

        for (;;) {
            for (uint8_t const* endx = src + cx * src_nbbytes
              ; src < endx; src += src_nbbytes
            ) {
                BGRColor pixel = dec(buf_to_color(src));
                *dest++ = pixel.red();
                *dest++ = pixel.green();
                *dest++ = pixel.blue();
                *dest++ = 255;
            }

            if (src == end) {
                break;
            }

            src -= step;
        }
    };

    namespace fns = pixel_conversion_fns;
    using namespace shortcut_decode_with_palette;
    auto dec24_revert = [](RDPColor c) { return BGRasRGBColor(dec24{}(c)); };
    switch (bits_per_pixel) {
        case BitsPerPixel::BitsPP8:  convert(fns::buf2col_1B, dec8{*palette}); break;
        case BitsPerPixel::BitsPP15: convert(fns::buf2col_2B, dec15{}); break;
        case BitsPerPixel::BitsPP16: convert(fns::buf2col_2B, dec16{}); break;
        case BitsPerPixel::BitsPP24: convert(fns::buf2col_3B, dec24_revert); break;
        default: assert(!"unknown bpp");
    }
}

void convert_bitmap8_to_bitmap16(
    uint8_t* dest,
    uint8_t const* bmp_data, uint16_t cy,
    std::size_t line_size, BGRPalette const& palette)
{
    namespace fns = pixel_conversion_fns;
    using namespace shortcut_encode;

    auto color_to_buf = fns::col2buf_3B;
    auto enc = enc16{};

    auto* end = bmp_data + cy * line_size;
    for (; bmp_data != end; ++bmp_data, dest += 2) {
        color_to_buf(enc(palette[*bmp_data]), dest);
    }
}

void transform_bitmap15_to_bitmap16(uint8_t* bmp_data, uint16_t cy, std::size_t line_size)
{
    namespace fns = pixel_conversion_fns;
    using namespace shortcut_decode_with_palette;
    using namespace shortcut_encode;

    auto buf_to_color = fns::buf2col_2B;
    auto color_to_buf = fns::col2buf_2B;
    auto dec = dec15{};
    auto enc = enc16{};

    auto* end = bmp_data + cy * line_size;
    for (; bmp_data != end; bmp_data += 2) {
        color_to_buf(enc(dec(buf_to_color(bmp_data))), bmp_data);
    }
}

}

EMSCRIPTEN_BINDINGS(image_data_func)
{
    redjs::function("convertBmpToImageData", +[](
        intptr_t idest, intptr_t idata, uint8_t bits_per_pixel,
        uint16_t w, uint16_t h, uint32_t line_size
    ) {
        auto* dest = redjs::from_memory_offset<uint8_t*>(idest);
        auto* data = redjs::from_memory_offset<uint8_t const*>(idata);
        redjs::convert_bitmap_to_image_data(
            dest, data, w, h, line_size, BitsPerPixel(bits_per_pixel),
            &BGRPalette::classic_332());
    });

    redjs::function("convertBmp8ToBmp16", +[](
        intptr_t idest, intptr_t idata,
        uint16_t h, uint32_t line_size
    ) {
        auto* dest = redjs::from_memory_offset<uint8_t*>(idest);
        auto* data = redjs::from_memory_offset<uint8_t const*>(idata);
        redjs::convert_bitmap8_to_bitmap16(dest, data, h, line_size, BGRPalette::classic_332());
    });

    redjs::function("transformBmp15ToBmp16", +[](
        intptr_t idata, uint16_t h, uint32_t line_size
    ) {
        auto* data = redjs::from_memory_offset<uint8_t*>(idata);
        redjs::transform_bitmap15_to_bitmap16(data, h, line_size);
    });
}
