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
Copyright (C) Wallix 2010-2019
Author(s): Jonathan Poelen
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "redjs/image_conversions.hpp"
#include "utils/bitmap.hpp"
#include "utils/bitmap_private_data.hpp"

#include <vector>

template<class Encoder>
static Bitmap create_bmp_4x_2y(Encoder encoder)
{
    Bitmap bitmap;

    uint16_t const w = 4;
    uint16_t const h = 2;
    Bitmap::PrivateData::Data & bitmap_data = Bitmap::PrivateData
      ::initialize(bitmap, Encoder::bpp, w, h);
    uint8_t const nb_bytes = nb_bytes_per_pixel(bitmap_data.bpp());
    size_t const padding = bitmap_data.line_size() - w * nb_bytes;
    uint8_t* data = bitmap_data.get();

    auto enc = [&](uint8_t* p, BGRColor bgr){
        auto c = encoder(bgr).as_bgr().as_u32();
        for (uint8_t i = 0; i < nb_bytes; ++i) {
            *p++ = uint8_t(c);
            c >>= 8u;
        }
        return p;
    };

    data = enc(data, BGRColor(0xff0000));
    data = enc(data, BGRColor(0x00ff00));
    data = enc(data, BGRColor(0x0000ff));
    data = enc(data, BGRColor(0x123456));
    data += padding;
    data = enc(data, BGRColor(0xaa0033));
    data = enc(data, BGRColor(0x33aa00));
    data = enc(data, BGRColor(0x0033aa));
    data = enc(data, BGRColor(0x030a3a));

    return bitmap;
}

template<class Encoder>
static std::vector<uint8_t> create_image_4x_2y(Encoder encoder)
{
    std::vector<uint8_t> output_data;

    Bitmap bitmap = create_bmp_4x_2y(encoder);
    output_data.resize(bitmap.cx() * bitmap.cy() * 4);

    redjs::convert_bitmap_to_image_data(
        output_data.data(),
        bitmap.data(), bitmap.cx(), bitmap.cy(),
        bitmap.line_size(), bitmap.bpp(),
        &bitmap.palette());

    return output_data;
}

static bytes_view bmp2av(Bitmap const& bmp)
{
    return {bmp.data(), bmp.bmp_size()};
}

RED_AUTO_TEST_CASE(TestBmp16ToImageData)
{
    RED_CHECK(make_array_view(create_image_4x_2y(encode_color16())) ==
        "\x31\x00\xad\xff\x00\xaa\x31\xff\xad\x30\x00\xff\x39\x08\x00\xff"
        "\x00\x00\xff\xff\x00\xff\x00\xff\xff\x00\x00\xff\x52\x34\x10\xff"_av
    );
}

RED_AUTO_TEST_CASE(TestBmp24ToImageData)
{
    RED_CHECK(create_image_4x_2y(encode_color24()) ==
        "\xaa\x00\x33\xff\x33\xaa\x00\xff\x00\x33\xaa\xff\x03\x0a\x3a\xff"
        "\xff\x00\x00\xff\x00\xff\x00\xff\x00\x00\xff\xff\x12\x34\x56\xff"_av
    );
}

RED_AUTO_TEST_CASE(TestBmp15ToBmp16)
{
    Bitmap bmp15 = create_bmp_4x_2y(encode_color15());

    redjs::transform_bitmap15_to_bitmap16(
        const_cast<uint8_t*>(bmp15.data()), /*NOLINT*/
        bmp15.cy(), bmp15.line_size());

    RED_CHECK(bmp2av(bmp15) ==
        "\x1f\x00\xe0\x07\x00\xf8\x82\x51"
        "\x15\x30\x66\x05\x80\xa9\x40\x38"_av);
}

RED_AUTO_TEST_CASE(TestBmp8ToBmp24)
{
    Bitmap bmp8 = create_bmp_4x_2y(encode_color8());
    uint8_t buf[4*2*2];

    redjs::convert_bitmap8_to_bitmap16(
        buf, bmp8.data(),
        bmp8.cy(), bmp8.line_size(),
        BGRPalette::classic_332());

    RED_CHECK(make_array_view(buf) ==
        "\x00\xf8\xe0\x07\x1f\x00\x29\x01"
        "\x04\xa8\xa0\x05\x36\x01\x04\x00"_av);
}
