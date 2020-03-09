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

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "core/error.hpp"
#include "utils/hexdump.hpp"
#include "core/RDP/capabilities/bitmapcodecs.hpp"


RED_AUTO_TEST_CASE(TestBitmapCodecCaps_emit)
{
    StaticOutStream<1024> out_stream;
    Emit_CS_BitmapCodecCaps cap;
    cap.emit(out_stream);

    InStream in_stream(out_stream.get_bytes());
    uint16_t capstype = in_stream.in_uint16_le();
    uint16_t capslen = in_stream.in_uint16_le();
    RED_CHECK_EQUAL(CAPSETTYPE_BITMAP_CODECS, capstype);
    RED_CHECK_EQUAL(5, capslen);

    Recv_CS_BitmapCodecCaps cap2;
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

    InStream in_stream(data);

    uint16_t capstype = in_stream.in_uint16_le();
    uint16_t capslen = in_stream.in_uint16_le();
    RED_CHECK_EQUAL(CAPSETTYPE_BITMAP_CODECS, capstype);
    RED_CHECK_EQUAL(0x60, capslen);

    Recv_SC_BitmapCodecCaps caps;
    caps.recv(in_stream, capslen);
}

// RFXClntCaps()
// RFXClntCaps::recv()
// ◢ In src/core/RDP/capabilities/bitmapcodecs.hpp:566
// we want capsetType == CLY_CAPSET
// Create Error: Exception ERR_MCS_PDU_TRUNCATED no: 5029
// ◢ In src/core/session.cpp:333
// Proxy data processing raised error 5029 : Exception ERR_MCS_PDU_TRUNCATED
// Session::Client Session Disconnected

RED_AUTO_TEST_CASE(TestBitmapCodecCaps_recv_CS_BitmapCodecsCaps)
{
    uint8_t data[] = {
    /* 0000 */ 0x1d, 0x00, // capset bitmap codec capabilities
               163, 0x00, // 163 bytes long

              0x03, // 4 bitmap codecs to follow

               // CODEC_GUID_NSCODEC
              0xb9, 0x1b, 0x8d, 0xca, 0x0f, 0x00, 0x4f, 0x15,
              0x58, 0x9f, 0xae, 0x2d, 0x1a, 0x87, 0xe2, 0xd6,
              0x01, // codec Id
              0x03, 0x00, // propertyLen to follow = 3
              0x01, 0x01, 0x03,

              // CODEC_GUID_IMAGE_REMOTEFX
              0xd4, 0xcc, 0x44, 0x27, 0x8a, 0x9d, 0x74, 0x4e,
              0x80, 0x3c, 0x0e, 0xcb, 0xee, 0xa1, 0x9c, 0x54,
              0x05, // codecId
              0x31, 0x00, // propertyLen to follow = 49
              0x31, 0x00, 0x00, 0x00, // length
              0x01, 0x00, 0x00, 0x00, // captureFlags
              0x25, 0x00, 0x00, 0x00, // capsLength
              // TS_RFX_CAPS
              0xc0, 0xcb, // blockType CBY_CAPS
              0x08, 0x00, 0x00, 0x00, // blockLen
              0x01, 0x00, // numCapsets
              0xc1, 0xcb, // blockType CBY_CAPSET
              0x1d, 0x00, 0x00, 0x00, // blockLen
              0x01,
              0xc0, 0xcf, // capsetType
              0x02, 0x00, // numIcaps
              0x08, 0x00, // icapLen
              // TS_RFX_ICAP
              0x00, 0x01, // CLW_VERSION_1_0
              0x40, 0x00, // CT_TILE_64x64 (0x0040)
              0x02, // CODEC_MODE
              0x01, // colConvBits=CLW_COL_CONV_ICT (0x1)
              0x01, // transFormBits=CLW_XFORM_DWT_53_A (0x1)
              0x01, // entropyBits CLW_ENTROPY_RLGR1
              // TS_RFX_ICAP
              0x00, 0x01, // CLW_VERSION_1_0
              0x40, 0x00, // CT_TILE_64x64 (0x0040)
              0x02, // CODEC_MODE
              0x01, // colConvBits=CLW_COL_CONV_ICT (0x1)
              0x01, // transFormBits=CLW_XFORM_DWT_53_A (0x1)
              0x04, // entropyBits CLW_ENTROPY_RLGR3

              // CODEC_GUID_REMOTEFX
              0x12, 0x2f, 0x77, 0x76, 0x72, 0xbd, 0x63, 0x44,
              0xaf, 0xb3, 0xb7, 0x3c, 0x9c, 0x6f, 0x78, 0x86,
              0x03,
              0x31, 0x00,
              0x31, 0x00, 0x00, 0x00, // length
              0x01, 0x00, 0x00, 0x00, // captureFlags
              0x25, 0x00, 0x00, 0x00, // capsLen
              // TS_RFX_CAPS
              0xc0, 0xcb, // blockType CBY_CAPS
              0x08, 0x00, 0x00, 0x00, // blockLen
              0x01, 0x00, // numCapset
              0xc1, 0xcb,  // blockType CBY_CAPSET 
              0x1d, 0x00, 0x00, 0x00, // blockLen
              0x01, // codecId
              0xc0, 0xcf,  // capsetType
              0x02, 0x00,  // numIcaps
              0x08, 0x00,  // icap.len
              // TS_RFX_ICAP
              0x00, 0x01, // CLW_VERSION_1_0
              0x40, 0x00, // CT_TILE_64x64 (0x0040)
              0x02, // CODEC_MODE
              0x01, // colConvBits=CLW_COL_CONV_ICT (0x1)
              0x01, // transFormBits=CLW_XFORM_DWT_53_A (0x1)
              0x01, // entropyBits CLW_ENTROPY_RLGR1
              // TS_RFX_ICAP
              0x00, 0x01, // CLW_VERSION_1_0
              0x40, 0x00, // CT_TILE_64x64 (0x0040)
              0x02, // CODEC_MODE
              0x01, // colConvBits=CLW_COL_CONV_ICT (0x1)
              0x01, // transFormBits=CLW_XFORM_DWT_53_A (0x1)
              0x04, // entropyBits CLW_ENTROPY_RLGR3

              0x00, 0x00, 0x00, 0x00,
    };


    InStream in_stream(data);

    uint16_t capstype = in_stream.in_uint16_le();
    uint16_t capslen = in_stream.in_uint16_le();
    RED_CHECK_EQUAL(CAPSETTYPE_BITMAP_CODECS, capstype);
    RED_CHECK_EQUAL(163, capslen);

    Recv_CS_BitmapCodecCaps caps;
    caps.recv(in_stream, capslen);
}
