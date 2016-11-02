/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Unit test for bitmap class (mostly tests of compression/decompression)

*/


#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestGdCmdConverter
#include "system/redemption_unit_tests.hpp"

#define LOGNULL

#include "gdi/graphic_cmd_color_converter.hpp"
#include "utils/sugar/range.hpp"

#include <array>

using GraphicsColor = gdi::BlackoutGraphic;

using Color4 = std::array<uint32_t, 4>;

inline Color4 get_colors(
    gdi::GraphicDepth order_depth, uint32_t color,
    GraphicsColor const & gd1, GraphicsColor const & gd2, GraphicsColor const & gd3
) {
    Color4 colors {{}};
    using GdRef = std::reference_wrapper<GraphicsColor const>;
    GdRef graphics[3] {gd1, gd2, gd3, };
    gdi::RngByBpp<GdRef*> rng_by_bpp{order_depth, std::begin(graphics), std::end(graphics)};
    gdi::draw_cmd_color_convert(
        [&](GraphicsColor const & gdc, const RDPOpaqueRect& cmd, const Rect&) {
            colors[gdc.order_depth().id()-1] = cmd.color;
        }, order_depth, rng_by_bpp, RDPOpaqueRect({}, color), Rect{}
    );
    return colors;
}


BOOST_AUTO_TEST_CASE(TestGdCmdConverter)
{
    BOOST_CHECK_EQUAL(gdi::GraphicCmdColor::is_encodable_cmd_color(RDPOpaqueRect({}, 0)).value, true);
    BOOST_CHECK_EQUAL(gdi::GraphicCmdColor::is_encodable_cmd_color(GlyphCache{}).value, false);

    gdi::GraphicDepth const depth15 = gdi::GraphicDepth::depth15();
    gdi::GraphicDepth const depth16 = gdi::GraphicDepth::depth16();
    gdi::GraphicDepth const depth24 = gdi::GraphicDepth::depth24();

    decode_color15_opaquerect dec15;
    decode_color16_opaquerect dec16;
    decode_color24_opaquerect dec24;

    encode_color15 enc15;
    encode_color16 enc16;
    encode_color24 enc24;

    Color4
    colors = get_colors(depth15, 0x0000fb, depth15, depth15, depth15);
    BOOST_CHECK_EQUAL(colors[0], 0);
    BOOST_CHECK_EQUAL(colors[1], 0x0000fb);
    BOOST_CHECK_EQUAL(colors[2], 0);
    BOOST_CHECK_EQUAL(colors[3], 0);

    colors = get_colors(depth16, 0x0000fb, depth16, depth16, depth16);
    BOOST_CHECK_EQUAL(colors[0], 0);
    BOOST_CHECK_EQUAL(colors[1], 0);
    BOOST_CHECK_EQUAL(colors[2], 0x0000fb);
    BOOST_CHECK_EQUAL(colors[3], 0);

    colors = get_colors(depth24, 0x0000fb, depth24, depth24, depth24);
    BOOST_CHECK_EQUAL(colors[0], 0);
    BOOST_CHECK_EQUAL(colors[1], 0);
    BOOST_CHECK_EQUAL(colors[2], 0);
    BOOST_CHECK_EQUAL(colors[3], 0x0000fb);


    colors = get_colors(depth15, 0x0000fb, depth24, depth24, depth24);
    BOOST_CHECK_EQUAL(colors[0], 0);
    BOOST_CHECK_EQUAL(colors[1], 0);
    BOOST_CHECK_EQUAL(colors[2], 0);
    BOOST_CHECK_EQUAL(colors[3], enc24(dec15(0x0000fb)));

    colors = get_colors(depth16, 0x0000fb, depth24, depth24, depth24);
    BOOST_CHECK_EQUAL(colors[0], 0);
    BOOST_CHECK_EQUAL(colors[1], 0);
    BOOST_CHECK_EQUAL(colors[2], 0);
    BOOST_CHECK_EQUAL(colors[3], enc24(dec16(0x0000fb)));

    colors = get_colors(depth24, 0x0000fb, depth15, depth15, depth15);
    BOOST_CHECK_EQUAL(colors[0], 0);
    BOOST_CHECK_EQUAL(colors[1], enc15(dec24(0x0000fb)));
    BOOST_CHECK_EQUAL(colors[2], 0);
    BOOST_CHECK_EQUAL(colors[3], 0);

    colors = get_colors(depth24, 0x0000fb, depth16, depth16, depth16);
    BOOST_CHECK_EQUAL(colors[0], 0);
    BOOST_CHECK_EQUAL(colors[1], 0);
    BOOST_CHECK_EQUAL(colors[2], enc16(dec24(0x0000fb)));
    BOOST_CHECK_EQUAL(colors[3], 0);


    colors = get_colors(depth15, 0x0000fb, depth15, depth16, depth24);
    BOOST_CHECK_EQUAL(colors[0], 0);
    BOOST_CHECK_EQUAL(colors[1], 0x0000fb);
    BOOST_CHECK_EQUAL(colors[2], enc16(dec15(0x0000fb)));
    BOOST_CHECK_EQUAL(colors[3], enc24(dec15(0x0000fb)));

    colors = get_colors(depth16, 0x0000fb, depth15, depth16, depth24);
    BOOST_CHECK_EQUAL(colors[0], 0);
    BOOST_CHECK_EQUAL(colors[1], enc15(dec16(0x0000fb)));
    BOOST_CHECK_EQUAL(colors[2], 0x0000fb);
    BOOST_CHECK_EQUAL(colors[3], enc24(dec16(0x0000fb)));

    colors = get_colors(depth24, 0x0000fb, depth15, depth16, depth24);
    BOOST_CHECK_EQUAL(colors[0], 0);
    BOOST_CHECK_EQUAL(colors[1], enc15(dec24(0x0000fb)));
    BOOST_CHECK_EQUAL(colors[2], enc16(dec24(0x0000fb)));
    BOOST_CHECK_EQUAL(colors[3], 0x0000fb);


    colors = get_colors(depth24, 0x0000fb, depth16, depth16, depth24);
    BOOST_CHECK_EQUAL(colors[0], 0);
    BOOST_CHECK_EQUAL(colors[1], 0);
    BOOST_CHECK_EQUAL(colors[2], enc16(dec24(0x0000fb)));
    BOOST_CHECK_EQUAL(colors[3], enc24(dec24(0x0000fb)));
}
