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

#include "utils/drawable_pointer.hpp"
#include "utils/bitfu.hpp"
#include "utils/image_view.hpp"
#include "utils/colors.hpp"
#include "utils/drawable.hpp"
#include "core/RDP/rdp_pointer.hpp"

#include <cstdint>
#include <cstddef>
#include <cstring>

namespace
{
    ImageView create_img(
        uint16_t width, uint16_t height, uint8_t const* data,
        unsigned line_bytes, BytesPerPixel bytes_per_pixel
    ) noexcept
    {
        return ImageView(
            data,
            width,
            height,
            line_bytes,
            bytes_per_pixel,
            ImageView::Storage::BottomToTop,
            &BGRPalette::classic_332()
        );
    }
} // namespace anonymous

DrawablePointer::DrawablePointer()
    : image_data_view_data(create_img(0, 0, nullptr, 0, BytesPerPixel{3}))
    , image_data_view_mask(create_img(0, 0, nullptr, 0, BytesPerPixel{3}))
{
    static_assert(std::size_t(RdpPointer::DATA_SIZE) == sizeof(this->data));
    static_assert(MAX_WIDTH * MAX_HEIGHT * Drawable::Bpp == sizeof(BufferSaver));
}

DrawablePointer::DrawablePointer(RdpPointerView const& cursor)
    : DrawablePointer()
{
    this->set_cursor(cursor);
}

void DrawablePointer::set_cursor(RdpPointerView const& cursor)
{
    const auto dim = cursor.dimensions();

    this->width = dim.width;
    this->height = dim.height;
    this->hotspot_x = cursor.hotspot().x;
    this->hotspot_y = cursor.hotspot().y;

    const BitsPerPixel bits_per_pixel = cursor.xor_bits_per_pixel();
    auto pointer_data = cursor.xor_mask();

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

            this->image_data_view_data = create_img(
                this->width, this->height, this->data, this->width * 3, BytesPerPixel(3));
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

            this->image_data_view_data = create_img(
                this->width, this->height, this->data, this->width * 3, BytesPerPixel(3));
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
            this->image_data_view_data = create_img(
                this->width, this->height, this->data, line_bytes, bytes_per_pixel);
            break;
        }

        case BitsPerPixel::Unspecified:
            break;
    }

    const uint8_t* pointer_mask = cursor.and_mask().data();
    const unsigned int mask_line_bytes = RdpPointerView::compute_mask_line_size(
        this->width, BitsPerPixel(1));

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
    this->image_data_view_mask = create_img(
        this->width, this->height, this->mask, this->width * 3, BytesPerPixel{3});
}

void DrawablePointer::trace_mouse(Drawable& drawable, BufferSaver& drawable_buffer) const
{
    uint8_t* cur_psave = drawable_buffer;

    auto const x = int16_t(int(this->cursor_x) - int(this->hotspot_x));
    auto const y = int16_t(int(this->cursor_y) - int(this->hotspot_y));
    Rect const rect_pointer(x, y, this->width, this->height);
    Rect const rect_drawable(0, 0, drawable.width(), drawable.height());
    Rect const rect_intersect = rect_drawable.intersect(rect_pointer);
    if (rect_intersect.isempty()) {
        return;
    }

    for (unsigned cur_line = 0; cur_line < rect_intersect.cy; ++cur_line) {
        const uint8_t * first_byte = drawable.data(rect_intersect.x, rect_intersect.y + cur_line);
        const unsigned data_length = drawable.nbbytes_color() * rect_intersect.cx;
        ::memcpy(cur_psave, first_byte, data_length);

        cur_psave += data_length;
    }

    Rect rect_sub_view(rect_intersect.x - rect_pointer.x,
                       rect_intersect.y - rect_pointer.y,
                       rect_intersect.cx,
                       rect_intersect.cy);

    drawable.mem_blt_ex(rect_intersect,
                        this->image_data_view_mask.sub_view(rect_sub_view),
                        0,
                        0,
                        0x88);
    drawable.mem_blt_ex(rect_intersect,
                        this->image_data_view_data.sub_view(rect_sub_view),
                        0,
                        0,
                        0x66);
}

void DrawablePointer::clear_mouse(Drawable& drawable, BufferSaver const& drawable_buffer) const
{
    uint8_t const* cur_psave = drawable_buffer;

    auto const x = int16_t(int(this->cursor_x) - int(this->hotspot_x));
    auto const y = int16_t(int(this->cursor_y) - int(this->hotspot_y));
    Rect const rect_pointer(x, y, this->width, this->height);
    Rect const rect_drawable(0, 0, drawable.width(), drawable.height());
    Rect const rect_intersect = rect_drawable.intersect(rect_pointer);
    if (rect_intersect.isempty()) {
        return;
    }

    for (unsigned cur_line = 0; cur_line < rect_intersect.cy; ++cur_line) {
        uint8_t * first_byte = drawable.first_pixel(rect_intersect.x, rect_intersect.y + cur_line);
        const unsigned data_length = drawable.nbbytes_color() * rect_intersect.cx;
        ::memcpy(first_byte, cur_psave, data_length);

        cur_psave += data_length;
    }
}
