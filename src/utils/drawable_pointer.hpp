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
   Copyright (C) Wallix 2018
   Author(s): Christophe Grosjean, Poelen Jonathan
*/

#pragma once

#include "utils/bitfu.hpp"
#include "utils/image_view.hpp"
#include "utils/colors.hpp"
#include "core/RDP/rdp_pointer.hpp"

#include <cstdint>
#include <cstddef>
#include <cstring>


struct DrawablePointer
{
    enum {
          MAX_WIDTH  = 96
        , MAX_HEIGHT = 96
    };

    uint16_t width { 0 };
    uint16_t height { 0 };

    uint8_t data[MAX_WIDTH * MAX_HEIGHT * 4]; // 96 pixels per line * 96 lines * 4 bytes per pixel
    uint8_t mask[MAX_WIDTH * MAX_HEIGHT * 3]; // 96 pixels per line * 96 lines * 3 bytes per pixel

    ImageView image_data_view_data;
    ImageView image_data_view_mask;

    DrawablePointer() :
        image_data_view_data(create_img(nullptr, 0)),
        image_data_view_mask(create_img(nullptr, 0))
    {}

    DrawablePointer(Pointer const& cursor) :
        DrawablePointer()
    {
        this->set_cursor(cursor);
    }

    void set_cursor(Pointer const& cursor)
    {
        const auto dim = cursor.get_dimensions();

        this->width = dim.width;
        this->height = dim.height;

        const BitsPerPixel bits_per_pixel = cursor.get_native_xor_bpp();
        auto pointer_data = cursor.get_native_xor_mask();

        switch (bits_per_pixel) {
            case BitsPerPixel::BitsPP1: {
                const unsigned line_bytes = ::even_pad_length(::nbbytes(dim.width));
                const unsigned h = dim.height;
                auto* dest = this->data;
                for (unsigned y = 0; y < h; ++y) {
                    const uint8_t* src = pointer_data.data() + y * line_bytes;

                    unsigned char bit_count = 7;
                    const uint8_t* enddest = dest + dim.width*3;
                    while (dest < enddest) {
                        uint8_t pixel = (*src & (1 << bit_count)) ? 0xFF : 0;
                        *dest++ = pixel;
                        *dest++ = pixel;
                        *dest++ = pixel;

                        if (bit_count == 0) {
                            ++src;
                        }

                        bit_count = (bit_count - 1) & 7;
                    }
                }

                this->image_data_view_data = this->create_img(this->data,
                                                              this->width * 3,
                                                              BytesPerPixel(3));
                break;
            }

            case BitsPerPixel::BitsPP4: {
                const unsigned line_bytes = ::even_pad_length(::nbbytes(dim.width * 4));
                const unsigned h = dim.height;
                auto& palette = BGRPalette::classic_332();
                auto* dest = this->data;
                for (unsigned y = 0; y < h; ++y) {
                    const uint8_t* src = pointer_data.data() + y * line_bytes;

                    unsigned char bit_count = 7;
                    const uint8_t* enddest = dest + dim.width*3;
                    while (dest < enddest) {
                        BGRColor pixel1 = palette[*src >> 4];

                        *dest++ = pixel1.red();
                        *dest++ = pixel1.green();
                        *dest++ = pixel1.blue();

                        bit_count = (bit_count - 1) & 7;

                        BGRColor pixel2 = palette[*src & 0xf];

                        *dest++ = pixel2.red();
                        *dest++ = pixel2.green();
                        *dest++ = pixel2.blue();

                        bit_count = (bit_count - 1) & 7;

                        ++src;
                    }
                }

                this->image_data_view_data = this->create_img(this->data,
                                                              this->width * 3,
                                                              BytesPerPixel(3));
                break;
            }

            case BitsPerPixel::BitsPP8:
            case BitsPerPixel::BitsPP15:
            case BitsPerPixel::BitsPP16:
            case BitsPerPixel::BitsPP24:
            case BitsPerPixel::BitsPP32: {
                auto bytes_per_pixel = BytesPerPixel(nbbytes(underlying_cast(bits_per_pixel)));
                unsigned line_bytes = ::even_pad_length(
                    this->width * underlying_cast(bytes_per_pixel));

                ::memcpy(this->data, pointer_data.data(), pointer_data.size());
                this->image_data_view_data = this->create_img(this->data,
                                                              line_bytes,
                                                              bytes_per_pixel);
                break;
            }

            case BitsPerPixel::Unspecified:
                break;
        }

        const uint8_t* pointer_mask = cursor.get_monochrome_and_mask().data();
        const unsigned int mask_line_bytes = ::even_pad_length(::nbbytes(this->width));

        auto* mask_ptr = this->mask;
        for (unsigned int y = 0; y < this->height; ++y)
        {
            for (unsigned int x = 0; x < this->width; ++x)
            {
                const size_t index = y * mask_line_bytes * 8 + x;
                const bool bit = pointer_mask[index / 8] & (1 << (7 - (index % 8)));
                const uint8_t pixel = bit ? 0xFF : 0;
                *mask_ptr++ = pixel;
                *mask_ptr++ = pixel;
                *mask_ptr++ = pixel;
            }
        }

        /* xorMask doesn't contain alpha channel info,
           so we will skip the 4th byte on each pixel
           on reading with BytesPerPixel{3} rather than BytesPerPixel{4} */
        this->image_data_view_mask = this->create_img(this->mask, this->width * 3);
    }

private:
    ImageView create_img(uint8_t const* data,
                         unsigned line_bytes,
                         BytesPerPixel bytes_per_pixel = BytesPerPixel{3}) const noexcept
    {
        return ImageView(
            data,
            this->width,
            this->height,
            line_bytes,
            bytes_per_pixel,
            ImageView::Storage::BottomToTop,
            &BGRPalette::classic_332()
        );
    }
};  // struct DrawablePointer
