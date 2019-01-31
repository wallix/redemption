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

#include "redjs/image_data.hpp"

#include "utils/bitmap.hpp"
#include "utils/bitmap_data_allocator.hpp"
#include "utils/colors.hpp"

#include <memory>


namespace redjs
{

ImageData::ImageData(Bitmap const& bmp)
: cx(bmp.cx())
, cy(bmp.cy())
, buf(static_cast<uint8_t*>(aux_::bitmap_data_allocator.alloc(this->size())))
{
    auto init = [this, bmp](auto buf_to_color, auto dec) -> void
    {
        uint8_t * dest = this->buf.get();
        uint8_t const src_nbbytes = nb_bytes_per_pixel(dec.bpp);
        uint8_t const* src = bmp.data() + bmp.line_size() * (bmp.cy() - 1);
        size_t const step = bmp.line_size() + bmp.cx() * src_nbbytes;
        uint8_t const* end = dest + this->size();

        while (dest < end) {
            uint8_t const* endx = dest + cx * 4;
            while (dest < endx) {
                BGRColor pixel = dec(buf_to_color(src));
                if (dec.bpp == BitsPerPixel{24}) {
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
    switch (bmp.bpp()) {
        case BitsPerPixel::BitsPP8:  init(buf2col_1B, dec8{bmp.palette()}); break;
        case BitsPerPixel::BitsPP1:  init(buf2col_2B, dec15{}); break;
        case BitsPerPixel::BitsPP16: init(buf2col_2B, dec16{}); break;
        case BitsPerPixel::BitsPP24: init(buf2col_3B, dec24{}); break;
        default: assert(!"unknown bpp");
    }
}

uint8_t const* ImageData::data() const noexcept
{
    return this->buf.get();
}

unsigned ImageData::width() const noexcept
{
    return this->cx;
}

unsigned ImageData::height() const noexcept
{
    return this->cy;
}

std::size_t ImageData::size() const noexcept
{
    return static_cast<std::size_t>(cx * cy * 4);
}

void ImageData::Deleter::operator()(void* p) noexcept
{
    aux_::bitmap_data_allocator.dealloc(p);
}

}
