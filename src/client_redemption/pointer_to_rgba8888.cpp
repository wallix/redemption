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
Copyright (C) Wallix 2010-2020
Author(s): Jonathan Poelen
*/

#include "client_redemption/pointer_to_rgba8888.hpp"

#include "core/RDP/rdp_pointer.hpp"
#include "utils/colors.hpp"

#include <cstring>

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

    void init_with_bpp1(uint8_t* dest, int w, int h, uint8_t const* data, uint8_t const* mask)
    {
        const int line_bytes = int(::even_pad_length(::nbbytes(unsigned(w))));

        for (int y = 0; y < h; ++y) {
            const uint8_t* src      = data + y * line_bytes;
            const uint8_t* src_mask = mask + y * line_bytes;

            unsigned char bit_count = 7;
            const uint8_t* enddest = dest + 4*w;
            while (dest < enddest) {
                uint8_t pixel = (*src & (1 << bit_count)) ? 0xFF : 0;
                *dest++ = pixel;
                *dest++ = pixel;
                *dest++ = pixel;
                *dest++ = (*src_mask & (1 << bit_count)) ? 0 : 0xFF;

                if (bit_count == 0) {
                    ++src;
                    ++src_mask;
                }

                bit_count = (bit_count - 1) & 7;
            }
        }
    }

    void init_with_bpp4(
        uint8_t* dest, int w, int h, uint8_t const* data, uint8_t const* mask,
        BGRPalette const& palette)
    {
        const int data_line_bytes = int(::even_pad_length(::nbbytes(unsigned(w * 4))));
        const int mask_line_bytes = int(::even_pad_length(::nbbytes(unsigned(w))));

        for (int y = 0; y < h; ++y) {
            const uint8_t* src      = data + y * data_line_bytes;
            const uint8_t* src_mask = mask + y * mask_line_bytes;

            unsigned char bit_count = 7;
            const uint8_t* enddest = dest + 4*w;
            while (dest < enddest) {
                BGRColor pixel1 = palette[*src >> 4];

                *dest++ = pixel1.red();
                *dest++ = pixel1.green();
                *dest++ = pixel1.blue();
                *dest++ = (*src_mask & (1 << bit_count)) ? 0 : 0xFF;

                bit_count = (bit_count - 1) & 7;

                BGRColor pixel2 = palette[*src & 0xf];

                *dest++ = pixel2.red();
                *dest++ = pixel2.green();
                *dest++ = pixel2.blue();
                *dest++ = (*src_mask & (1 << bit_count)) ? 0 : 0xFF;

                if (bit_count == 0) {
                    ++src_mask;
                }

                bit_count = (bit_count - 1) & 7;

                ++src;
            }
        }
    }

    template<class Decoder>
    void init_with_bpp8_15_16_24(
        uint8_t* dest, int w, int h, uint8_t const* data, uint8_t const* mask, Decoder decoder)
    {
        constexpr int bytes_per_pixel = ::nbbytes(unsigned(Decoder::bpp));
        const int data_line_bytes = int(::even_pad_length(unsigned(w * bytes_per_pixel)));
        const int mask_line_bytes = int(::even_pad_length(::nbbytes(unsigned(w))));

        data += (h-1) * data_line_bytes;
        mask += (h-1) * mask_line_bytes;

        for (int y = 0; y < h; ++y) {
            const uint8_t* src      = data;
            const uint8_t* src_mask = mask;

            unsigned char bit_count = 7;
            const uint8_t* enddest = dest + 4*w;
            while (dest < enddest) {
                auto raw_color = in_uint32_from_nb_bytes_le(bytes_per_pixel, src);
                BGRColor pixel = decoder(RDPColor::from(raw_color));
                src += bytes_per_pixel;

                *dest++ = pixel.red();
                *dest++ = pixel.green();
                *dest++ = pixel.blue();
                *dest++ = (*src_mask & (1 << bit_count)) ? 0 : 0xFF;

                if (bit_count == 0) {
                    ++src_mask;
                }

                bit_count = (bit_count - 1) & 7;
            }

            data -= data_line_bytes;
            mask -= mask_line_bytes;
        }
    }

    void init_with_bpp32(uint8_t* dest, int w, int h, uint8_t const* data)
    {
        const int w4 = w * 4;
        uint8_t const* src = data + (h-1) * w4;

        for (int y = 0; y < h; ++y) {
            std::memcpy(dest, src, size_t(w4));
            dest += w4;
            src -= w4;
        }
    }

    void apply_shadow(uint8_t* dest, int w, int h)
    {
        if (w <= 2 || h <= 3) {
            return ;
        }

        auto has_color = [](uint8_t const* color) {
            return color[0] || color[1] || color[2];
        };

        auto reverse_color = [=](uint8_t* dest_color, uint8_t const* color)
        {
            if (!has_color(dest_color)) {
                dest_color[0] = 255u - color[0];
                dest_color[1] = 255u - color[1];
                dest_color[2] = 255u - color[2];
                dest_color[3] = 255;
            }
        };

        const int w4 = w * 4;

        auto apply_color = [=](
            uint8_t* dest,
            auto has_top, auto has_bottom,
            auto has_left, auto has_right
        ){
            constexpr bool top = decltype(has_top)::value;
            constexpr bool bottom = decltype(has_bottom)::value;
            constexpr bool left = decltype(has_left)::value;
            constexpr bool right = decltype(has_right)::value;

            if (!has_color(dest)) {
                return;
            }

            dest[3] = 255;

            // border top
            if constexpr (top && left)     reverse_color(dest - w4 - 4, dest);
            if constexpr (top)             reverse_color(dest - w4, dest);
            if constexpr (top && right)    reverse_color(dest - w4 + 4, dest);
            // border left
            if constexpr (left)            reverse_color(dest - 4, dest);
            // border right
            if constexpr (right)           reverse_color(dest + 4, dest);
            // border bottom
            if constexpr (bottom && left)  reverse_color(dest + w4 - 4, dest);
            if constexpr (bottom)          reverse_color(dest + w4, dest);
            if constexpr (bottom && right) reverse_color(dest + w4 + 4, dest);
        };

        auto y = std::true_type{};
        auto n = std::false_type{};

        // without border top / left
        apply_color(dest, n, y, n, y);
        dest += 4;
        // without border top
        for (auto* enddest = dest + w4 - 4*2; dest < enddest; dest += 4) {
            apply_color(dest, n, y, y, y);
        }
        // without border top / right
        apply_color(dest, n, y, y, n);
        dest += 4;

        for (int i = 1; i < h-1; ++i) {
            // without border left
            apply_color(dest, y, y, n, y);
            dest += 4;
            for (auto* enddest = dest + w4 - 4*2; dest < enddest; dest += 4) {
                apply_color(dest, y, y, y, y);
            }
            // without border right
            apply_color(dest, y, y, y, n);
            dest += 4;
        }

        // without border bottom / left
        apply_color(dest, y, n, n, y);
        dest += 4;
        // without border bottom
        for (auto* enddest = dest + w4 - 4*2; dest < enddest; dest += 4) {
            apply_color(dest, y, n, y, y);
        }
        // without border bottom / right
        apply_color(dest, y, n, y, n);
        dest += 4;
    }
}

redclient::RGBA8888Image redclient::pointer_to_rgba8888(RdpPointerView const& pointer)
{
    auto const dimensions = pointer.dimensions();
    auto const width = dimensions.width;
    auto const height = dimensions.height;
    auto const source = pointer.xor_mask();
    auto const mask = pointer.and_mask();
    auto* pdata = new uint8_t[width * height * 4]; /*NOLINT*/
    RGBA8888Image img{width, height, std::unique_ptr<uint8_t[]>(pdata)};

    auto* xor_mask = source.data();
    auto* and_mask = mask.data();

    switch (pointer.xor_bits_per_pixel()) {
        case BitsPerPixel::BitsPP1:
            init_with_bpp1(pdata, width, height, xor_mask, and_mask);
            break;

        case BitsPerPixel::BitsPP4:
            init_with_bpp4(pdata, width, height, xor_mask, and_mask,
                BGRPalette::classic_332());
            break;

        case BitsPerPixel::BitsPP8:
            init_with_bpp8_15_16_24(pdata, width, height, xor_mask, and_mask,
                decode_color8_with_palette{BGRPalette::classic_332()});
            break;

        case BitsPerPixel::BitsPP15:
            init_with_bpp8_15_16_24(pdata, width, height, xor_mask, and_mask,
                decode_color15{});
            break;

        case BitsPerPixel::BitsPP16:
            init_with_bpp8_15_16_24(pdata, width, height, xor_mask, and_mask,
                decode_color16{});
            break;

        case BitsPerPixel::BitsPP24:
            init_with_bpp8_15_16_24(pdata, width, height, xor_mask, and_mask,
                decode_color24{});
            break;

        case BitsPerPixel::BitsPP32:
            init_with_bpp32(pdata, width, height, xor_mask);
            break;

        case BitsPerPixel::Unspecified:
            break;
    }

    if (is_empty_mask(mask)) {
        apply_shadow(pdata, width, height);
    }

    return img;
}
