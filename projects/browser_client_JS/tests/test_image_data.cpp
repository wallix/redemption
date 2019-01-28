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

#define RED_TEST_MODULE TestImageData
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "redjs/image_data.hpp"
#include "utils/bitmap.hpp"
#include "utils/bitmap_private_data.hpp"

template<class Encoder>
redjs::ImageData create_image_4x_2y(Encoder encoder)
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
        auto c = encoder(bgr).as_bgr().to_u32();
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

    redjs::ImageData img(bitmap);

    RED_CHECK(img.width() == bitmap.cx());
    RED_CHECK(img.height() == bitmap.cy());
    RED_CHECK(img.size() == bitmap.cx()*bitmap.cy()*4u);

    return img;
}

RED_AUTO_TEST_CASE(TestImageData16)
{
    redjs::ImageData img = create_image_4x_2y(encode_color16());

    RED_CHECK_MEM_C(make_array_view(img.data(), img.size()),
        "\x31\x00\xad\xff\x00\xaa\x31\xff\xad\x30\x00\xff\x39\x08\x00\xff"
        "\x00\x00\xff\xff\x00\xff\x00\xff\xff\x00\x00\xff\x52\x34\x10\xff"
    );
}

RED_AUTO_TEST_CASE(TestImageData24)
{
    redjs::ImageData img = create_image_4x_2y(encode_color24());

    RED_CHECK_MEM_C(make_array_view(img.data(), img.size()),
        "\xaa\x00\x33\xff\x33\xaa\x00\xff\x00\x33\xaa\xff\x03\x0a\x3a\xff"
        "\xff\x00\x00\xff\x00\xff\x00\xff\x00\x00\xff\xff\x12\x34\x56\xff"
    );
}
