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

#include "redjs/image_data_from_bitmap.hpp"

#include "red_emscripten/bind.hpp"
#include "red_emscripten/val.hpp"

#include "utils/bitmap.hpp"
#include "utils/colors.hpp"


namespace redjs
{

static void image_data_from_bitmap_impl(
    uint8_t* dest,
    uint8_t const* bmp_data, uint16_t cx, uint16_t cy,
    std::size_t line_size, BitsPerPixel bits_per_pixel,
    BGRPalette const* palette)
{
    auto init = [&](auto buf_to_color, auto dec) -> void
    {
        uint8_t const src_nbbytes = nb_bytes_per_pixel(bits_per_pixel);
        uint8_t const* src = bmp_data + line_size * (cy - 1);
        size_t const step = line_size + cx * src_nbbytes;
        uint8_t const* end = dest + cx * cy * 4;

        while (dest < end) {
            uint8_t const* endx = dest + cx * 4;
            while (dest < endx) {
                BGRColor pixel = dec(buf_to_color(src));
                if (bits_per_pixel == BitsPerPixel{24}) {
                    pixel = BGRasRGBColor(pixel);
                }
                *dest++ = pixel.red();
                *dest++ = pixel.green();
                *dest++ = pixel.blue();
                *dest++ = 255;
                src += src_nbbytes;
            }
            src -= step;
        }
    };

    auto buf2col_1B = [ ](uint8_t const * p) { return RDPColor::from(p[0]); };
    auto buf2col_2B = [=](uint8_t const * p) { return RDPColor::from(p[0] | (p[1] << 8)); };
    auto buf2col_3B = [=](uint8_t const * p) { return RDPColor::from(p[0] | (p[1] << 8) | (p[2] << 16)); };
    using namespace shortcut_decode_with_palette;
    switch (bits_per_pixel) {
        case BitsPerPixel::BitsPP8:  init(buf2col_1B, dec8{*palette}); break;
        case BitsPerPixel::BitsPP15: init(buf2col_2B, dec15{}); break;
        case BitsPerPixel::BitsPP16: init(buf2col_2B, dec16{}); break;
        case BitsPerPixel::BitsPP24: init(buf2col_3B, dec24{}); break;
        default: assert(!"unknown bpp");
    }
}

ImageData image_data_from_bitmap(Bitmap const& bmp)
{
    uint8_t* pdata = new uint8_t[bmp.cx() * bmp.cy() * 4]; /* NOLINT */
    ImageData img{bmp.cx(), bmp.cy(), std::unique_ptr<uint8_t[]>(pdata)};

    image_data_from_bitmap_impl(
        pdata,
        bmp.data(), bmp.cx(), bmp.cy(),
        bmp.line_size(), bmp.bpp(),
        &bmp.palette());

    return img;
}

}

EMSCRIPTEN_BINDINGS(image_data_func)
{
    redjs::function("loadRgbaImageFromIndex", +[](
        intptr_t idest, intptr_t idata, uint8_t bits_per_pixel,
        uint16_t w, uint16_t h, uint32_t line_size
    ) {
        auto* dest = redjs::from_memory_offset<uint8_t*>(idest);
        auto* data = redjs::from_memory_offset<uint8_t const*>(idata);
        redjs::image_data_from_bitmap_impl(
            dest, data, w, h, line_size, BitsPerPixel(bits_per_pixel),
            &BGRPalette::classic_332());
    });
}
