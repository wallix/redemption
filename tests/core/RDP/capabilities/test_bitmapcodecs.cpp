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
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean
*/

#define RED_TEST_MODULE TestBitmapCodecs
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "core/error.hpp"
#include "utils/hexdump.hpp"
#include "core/RDP/capabilities/bitmapcodecs.hpp"


RED_AUTO_TEST_CASE(TestBitmapCodecCaps_emit)
{
    StaticOutStream<1024> out_stream;
    BitmapCodecCaps cap(true);
    cap.emit(out_stream);

    InStream in_stream(out_stream.get_bytes());
    uint16_t capstype = in_stream.in_uint16_le();
    uint16_t capslen = in_stream.in_uint16_le();
    RED_CHECK_EQUAL(CAPSETTYPE_BITMAP_CODECS, capstype);
    RED_CHECK_EQUAL(5, capslen);

    BitmapCodecCaps cap2(true);
    cap2.recv(in_stream, CAPLEN_BITMAP_CODECS_CAPS);
}

RED_AUTO_TEST_CASE(TestBitmapCodecCaps_recv)
{
    uint8_t data[] = { 
    /* 0000 */ 0x1d, 0x00, // capset bitmap codec capabilities
               0x60, 0x00, // 96 bytes long
               0x04,       // 4 bitmap codecs to follow
               // CODEC_GUID_NSCODEC
               0xb9, 0x1b, 0x8d, 0xca, 0x0f, 0x00, 0x4f, 0x15,
               0x58, 0x9f, 0xae, 0x2d, 0x1a, 0x87, 0xe2, 0xd6,
               0x00,       // codecID=0
               0x03, 0x00, // property len to follow = 3 bytes
               0x01, 0x01, 0x03, 
               // CODEC_GUID_REMOTEFX
               0x12, 0x2f, 0x77, 0x76, 0x72, 0xbd, 0x63, 0x44,
               0xaf, 0xb3, 0xb7, 0x3c, 0x9c, 0x6f, 0x78, 0x86,
               0x00,       // codecID=0
               0x04, 0x00, // property len to follow = 4 bytes
               0x00, 0x00, 0x00, 0x00,
               // CODEC_REMOTEFX
               0xa6, 0x51, 0x43, 0x9c, 0x35, 0x35, 0xae, 0x42,
               0x91, 0x0c, 0xcd, 0xfc, 0xe5, 0x76, 0x0b, 0x58,
               0x00,       // codecID=0
               0x04, 0x00, // property len to follow = 4 bytes
               0x00, 0x00, 0x00, 0x00,
               // CODEC_GUID_IMAGE_REMOTEFX
               0xd4, 0xcc, 0x44, 0x27, 0x8a, 0x9d, 0x74, 0x4e,
               0x80, 0x3c, 0x0e, 0xcb, 0xee, 0xa1, 0x9c, 0x54,
               0x00,       // codecID=0
               0x04, 0x00, // property len to follow = 4 bytes
               0x00, 0x00, 0x00, 0x00,

    // this is junk to make a stream containing more data than mere BitmapCodecCaps
    /* 0020 */ 0xbd, 0x63, 0x44, 0xaf, 0xb3, 0xb7, 0x3c, 0x9c, 0x6f, 0x78, 0x86, 0x00, 0x04, 0x00, 0x00, 0x00,  // .cD...<.ox......
    /* 0030 */ 0x00, 0x00, 0xa6, 0x51, 0x43, 0x9c, 0x35, 0x35, 0xae, 0x42, 0x91, 0x0c, 0xcd, 0xfc, 0xe5, 0x76,  // ...QC.55.B.....v
    /* 0040 */ 0x0b, 0x58, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0xd4, 0xcc, 0x44, 0x27, 0x8a, 0x9d, 0x74,  // .X.........D'..t
    /* 0050 */ 0x4e, 0x80, 0x3c, 0x0e, 0xcb, 0xee, 0xa1, 0x9c, 0x54, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00,  // N.<.....T.......

    };

 
    InStream in_stream(data, sizeof(data));
 
    uint16_t capstype = in_stream.in_uint16_le();
    uint16_t capslen = in_stream.in_uint16_le();
    RED_CHECK_EQUAL(CAPSETTYPE_BITMAP_CODECS, capstype);
    RED_CHECK_EQUAL(0x60, capslen);
 
    BitmapCodecCaps caps(false);
    caps.recv(in_stream, capslen);
}
