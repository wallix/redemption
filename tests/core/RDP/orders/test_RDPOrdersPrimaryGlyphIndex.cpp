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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean

   Unit test to RDP Orders coder/decoder
   Using lib boost functions for testing
*/

#define RED_TEST_MODULE TestOrderGlyphIndex
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "core/RDP/orders/RDPOrdersPrimaryGlyphIndex.hpp"

RED_TEST_DONT_PRINT_LOG_VALUE(RDPBrush)

RED_AUTO_TEST_CASE(TestGlyphIndex)
{
    using namespace RDP;

    {
        StaticOutStream<1000> out_stream;
        // TODO " actual data is much more complex  than a text  we should create a specialized object to store  serialize and replay it. This should be done after the RDP layer include cache management primitives"

        const uint8_t empty[1] = {0};
        RDPOrderCommon state_common(0, Rect(0, 0, 0, 0));
        RDPGlyphIndex statecmd(0, 0, 0, 0, RDPColor{}, RDPColor{}, Rect(), Rect(), RDPBrush(), 0, 0, 0, empty);
        RDPOrderCommon newcommon(GLYPHINDEX, Rect(5, 0, 800, 600));
        const uint8_t brush_extra[] = {1,2,3,4,5,6,7};
        const uint8_t greeting[] = { 'H','e','l','l','o',',',' ','W','o','r','l','d' };
        RDPGlyphIndex newcmd(1, 0x20, 1, 4,
                             encode_color24()(BGRColor{0x112233}),
                             encode_color24()(BGRColor{0x445566}),
                             Rect(1,2,40,60),
                             Rect(3,4,50,70),
                             RDPBrush(3, 4, 0x03, 0xDD, brush_extra),
                             5, 6,
                             12, greeting);

        newcmd.emit(out_stream, newcommon, state_common, statecmd);

        RED_CHECK_EQUAL(static_cast<uint8_t>(GLYPHINDEX), newcommon.order);
        RED_CHECK_EQUAL(Rect(5, 0, 800, 600), newcommon.clip);

        // TODO " DELTA is disabled because it does not works with rdesktop"
        uint8_t datas[] = {
            CHANGE | STANDARD | BOUNDS,
            GLYPHINDEX,
            0xff, 0xff, 0x3f, // fields
            0x1c, 0x05, 0x24, 0x03, 0x57, 0x02, // bounds
            0x01, // font cache_id
            0x20, // flags
            0x01, // charinc
            0x04, // f_op_redundant
            0x33, 0x22, 0x11, // back_color
            0x66, 0x55, 0x44, // fore_color
            0x01, 0x00, 0x02, 0x00, 0x28, 0x00, 0x3d, 0x00, // bk rect
            0x03, 0x00, 0x04, 0x00, 0x34, 0x00, 0x49, 0x00, // op rect
            0x03, 0x04, 0x03, 0xdd, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, // brush
            0x05, 0x00, 0x06, 0x00,// glyph_x, glyph_y
            // 12, "Hello, World"
            0x0c, 0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64,
        };

        RED_CHECK_MEM(out_stream.get_bytes(), make_array_view(datas));
        InStream in_stream(out_stream.get_bytes());

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = in_stream.in_uint8();
        RED_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(in_stream, control);

        RED_CHECK_EQUAL(static_cast<uint8_t>(0x0D), header.control);
        RED_CHECK_EQUAL(static_cast<uint32_t>(0x3fffff), header.fields);
        RED_CHECK_EQUAL(static_cast<uint8_t>(GLYPHINDEX), common_cmd.order);
        RED_CHECK_EQUAL(Rect(5, 0, 800, 600), common_cmd.clip);

        RDPGlyphIndex cmd = statecmd;
        cmd.receive(in_stream, header);

        if (!(RDPBrush(3, 4, 0x03, 0xDD, brush_extra)
                == cmd.brush)){
            RED_CHECK_EQUAL(true, false);
        }
        // TODO " actual data is much more complex than a text  we should create a specialized object to store  serialize and replay it. This should be done after the RDP layer include cache management primitives"

        decltype(out_stream) out_stream2;
        cmd.emit(out_stream2, newcommon, state_common, statecmd);
        RED_CHECK_MEM(out_stream.get_bytes(), out_stream2.get_bytes());
    }
}

RED_AUTO_TEST_CASE(TestGlyphIndex2)
{
    using namespace RDP;

    {
        StaticOutStream<1000> out_stream;
        // TODO " actual data is much more complex  than a text  we should create a specialized object to store  serialize and replay it. This should be done after the RDP layer include cache management primitives"

        RDPOrderCommon state_common(RDP::PATBLT, Rect(0, 0, 1024, 768));
        RDPGlyphIndex  statecmd(0, 0, 0, 0, RDPColor{}, RDPColor{}, Rect(0, 0, 1, 1), Rect(0, 0, 1, 1), RDPBrush(), 0, 0, 0, byte_ptr_cast(""));
        RDPOrderCommon newcommon(GLYPHINDEX, Rect(0, 0, 1024, 768));
        const uint8_t nullbrush_extra[] = {0,0,0,0,0,0,0};
        const uint8_t data[] = { 0x00, 0x00, 0x01, 0x08, 0x02, 0x07, 0x03, 0x07
                               , 0x04, 0x07, 0x05, 0x05, 0x01, 0x05, 0x04, 0x07
                               , 0x03, 0x05, 0x07, 0x0a, 0x03, 0x07, 0x08, 0x0a
                               , 0x03, 0x06, 0x08, 0x07, 0x08, 0x06, 0x09, 0x06
                               , 0x0a, 0x03, 0x0b, 0x07, 0x0c, 0x0a, 0x09, 0x0b
                               , 0x0b, 0x03, 0x07, 0x07, 0x0a, 0x07, 0x0d, 0x07
                               , 0x08, 0x09};


        RDPGlyphIndex  newcmd( 7, 3, 0, 1
                             , encode_color24()(BGRColor{0x00ffff})
                             , encode_color24()(BGRColor{0x00092d})
                             , Rect(308, 155, 174, 14)
                             , Rect(0, 0, 1, 1)
                             , RDPBrush(0, 0, 0, 0, nullbrush_extra)
                             , 0x0134, 0x00a6
                             , 50
                             , data
                             );

        newcmd.emit(out_stream, newcommon, state_common, statecmd);

        RED_CHECK_EQUAL(static_cast<uint8_t>(GLYPHINDEX), newcommon.order);
        RED_CHECK_EQUAL(Rect(0, 0, 1024, 768), newcommon.clip);

        // TODO " DELTA is disabled because it does not works with rdesktop"
        uint8_t datas[] = {
              CHANGE | STANDARD
            , GLYPHINDEX
            , 0xfb, 0x03, 0x38                                  // fieldFlags
            , 0x07                                              // cacheId
            , 0x03                                              // flAccel
            , 0x01                                              // fOpRedundant
            , 0xff, 0xff, 0x00                                  // BackColor
            , 0x2d, 0x09, 0x00                                  // ForeColor
            , 0x34, 0x01, 0x9b, 0x00, 0xe1, 0x01, 0xa8, 0x00    // Bk rect
            , 0x34, 0x01, 0xa6, 0x00                            // X, Y
            , 0x32                                              // cbData
            , 0x00, 0x00, 0x01, 0x08, 0x02, 0x07, 0x03, 0x07, 0x04, 0x07, 0x05, 0x05, 0x01, 0x05, 0x04, 0x07
            , 0x03, 0x05, 0x07, 0x0a, 0x03, 0x07, 0x08, 0x0a, 0x03, 0x06, 0x08, 0x07, 0x08, 0x06, 0x09, 0x06
            , 0x0a, 0x03, 0x0b, 0x07, 0x0c, 0x0a, 0x09, 0x0b, 0x0b, 0x03, 0x07, 0x07, 0x0a, 0x07, 0x0d, 0x07
            , 0x08, 0x09
        };

        RED_CHECK_MEM(out_stream.get_bytes(), make_array_view(datas));
        InStream in_stream(out_stream.get_bytes());

        RDPOrderCommon common_cmd = state_common;
        uint8_t        control    = in_stream.in_uint8();
        RED_CHECK(control & STANDARD);

        RDPPrimaryOrderHeader header = common_cmd.receive(in_stream, control);
        RED_CHECK_EQUAL(static_cast<uint8_t>(0x09), header.control);
        RED_CHECK_EQUAL(static_cast<uint32_t>(0x3803fb), header.fields);

        RED_CHECK_EQUAL(static_cast<uint8_t>(GLYPHINDEX), common_cmd.order);
        RED_CHECK_EQUAL(Rect(0, 0, 1024, 768), common_cmd.clip);

        RDPGlyphIndex cmd = statecmd;
        cmd.receive(in_stream, header);

        RED_CHECK(RDPBrush(0, 0, 0x0, 0x0, nullbrush_extra) == cmd.brush);

        // TODO " actual data is much more complex than a text  we should create a specialized object to store  serialize and replay it. This should be done after the RDP layer include cache management primitives"
        decltype(out_stream) out_stream2;
        cmd.emit(out_stream2, newcommon, state_common, statecmd);
        RED_CHECK_MEM(out_stream.get_bytes(), out_stream2.get_bytes());
    }
}
