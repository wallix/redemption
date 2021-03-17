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
Copyright (C) Wallix 2021
Author(s): Proxies Team
*/

#include "utils/scaled_image24.hpp"
#include "utils/image_view.hpp"
#include "utils/png.hpp"


ScaledPng24::ScaledPng24(unsigned width, unsigned height)
: scaled_width{(width && height) ? (width + 3) & 0xFFCu : 0u}
, scaled_height{height}
, scaled_buffer(this->scaled_width
    ? std::make_unique<uint8_t[]>(this->scaled_width * this->scaled_height * 3)
    : std::unique_ptr<uint8_t[]>())
{
}

template<class Out>
static void ScaledPng24_dump_png24_impl(
    Out& out, const ImageView& image_view, bool bgr,
    uint8_t* scaled_buffer, unsigned scaled_width, unsigned scaled_height
)
{
    if (!scaled_buffer) {
        ::dump_png24(out, image_view, bgr);
    }
    else {
        assert(image_view.bits_per_pixel() == ImageView::BitsPerPixel::BitsPP24);
        ::scale_image24(
            scaled_buffer, image_view.data(),
            scaled_width, image_view.width(),
            scaled_height, image_view.height(),
            image_view.line_size());
        ::dump_png24(
            out, scaled_buffer,
            scaled_width, scaled_height,
            scaled_width * 3, !bgr);
    }
}

void ScaledPng24::dump_png24(Transport& trans, const ImageView& image_view, bool bgr) const
{
    ScaledPng24_dump_png24_impl(
        trans, image_view, bgr,
        this->scaled_buffer.get(), this->scaled_width, this->scaled_height
    );
}

void ScaledPng24::dump_png24(char const* filename, const ImageView& image_view, bool bgr) const
{
    ScaledPng24_dump_png24_impl(
        filename, image_view, bgr,
        this->scaled_buffer.get(), this->scaled_width, this->scaled_height
    );
}


void scale_image24(
    uint8_t *dest, const uint8_t *src,
    unsigned int dest_width, unsigned int src_width,
    unsigned int dest_height, unsigned int src_height,
    unsigned int src_rowsize
) {
    const uint32_t Bpp = 3;
    unsigned int y_pixels = dest_height;
    unsigned int y_int_part = (src_height / dest_height) * src_rowsize;
    unsigned int y_fract_part = src_height % dest_height;
    unsigned int yE = 0;
    unsigned int x_int_part = src_width / dest_width * Bpp;
    unsigned int x_fract_part = src_width % dest_width;

    while (y_pixels-- > 0) {
        unsigned int xE = 0;
        const uint8_t * x_src = src;
        unsigned int x_pixels = dest_width;
        while (x_pixels-- > 0) {
            dest[0] = x_src[2];
            dest[1] = x_src[1];
            dest[2] = x_src[0];

            dest += Bpp;
            x_src += x_int_part;
            xE += x_fract_part;
            if (xE >= dest_width) {
                xE -= dest_width;
                x_src += Bpp;
            }
        }
        src += y_int_part;
        yE += y_fract_part;
        if (yE >= dest_height) {
            yE -= dest_height;
            src += src_rowsize;
        }
    }
}
