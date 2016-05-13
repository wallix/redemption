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
#include "utils/range.hpp"

#include <array>


using Color4 = std::array<unsigned, 4>;

struct RngByBpp
{
    unsigned igds[4] {0, 1, 2, 3};
    Color4 & colors;

    RngByBpp(Color4 & colors) : colors(colors) {}

    using iterator = unsigned const *;

    range<iterator> rng8() const { return {&igds[0], &igds[1]}; }
    range<iterator> rng15() const { return {&igds[1], &igds[2]}; }
    range<iterator> rng16() const { return {&igds[2], &igds[3]}; }
    range<iterator> rng24() const { return {&igds[3], &igds[4]}; }
    range<iterator> rng_all() const { BOOST_CHECK(false); return {&igds[0], &igds[0]}; }

    void apply(unsigned igd, const RDPOpaqueRect& cmd, const Rect&) const {
        this->colors[igd] = cmd.color;
    }
};

template<bool has_enc15, bool has_enc16, bool has_enc24, class Dec>
Color4 get_colors(Dec dec, uint32_t color) {
    Color4 colors {};
    RngByBpp rng_by_bpp{colors};
    gdi::GraphicCmdColorDistributor<RngByBpp, Dec, false, has_enc15, has_enc16, has_enc24>{
        rng_by_bpp, dec
    }(RDPOpaqueRect({}, color), Rect{});
    return colors;
}


BOOST_AUTO_TEST_CASE(TestGdCmdConverter)
{
    BOOST_CHECK_EQUAL(gdi::GraphicCmdColor::is_encodable_cmd_color_trait<RDPOpaqueRect>::value, true);
    BOOST_CHECK_EQUAL(gdi::GraphicCmdColor::is_encodable_cmd_color_trait<GlyphCache>::value, false);

    decode_color15_opaquerect dec15;
    decode_color16_opaquerect dec16;
    decode_color24_opaquerect dec24;

    encode_color15 enc15;
    encode_color16 enc16;
    encode_color24 enc24;

    Color4
    colors = get_colors<1, 0, 0>(dec15, 0x0000fb);
    BOOST_CHECK_EQUAL(colors[0], 0);
    BOOST_CHECK_EQUAL(colors[1], 0x0000fb);
    BOOST_CHECK_EQUAL(colors[2], 0);
    BOOST_CHECK_EQUAL(colors[3], 0);

    colors = get_colors<0, 1, 0>(dec16, 0x0000fb);
    BOOST_CHECK_EQUAL(colors[0], 0);
    BOOST_CHECK_EQUAL(colors[1], 0);
    BOOST_CHECK_EQUAL(colors[2], 0x0000fb);
    BOOST_CHECK_EQUAL(colors[3], 0);

    colors = get_colors<0, 0, 1>(dec24, 0x0000fb);
    BOOST_CHECK_EQUAL(colors[0], 0);
    BOOST_CHECK_EQUAL(colors[1], 0);
    BOOST_CHECK_EQUAL(colors[2], 0);
    BOOST_CHECK_EQUAL(colors[3], 0x0000fb);

    colors = get_colors<1, 1, 1>(dec24, 0x0000fb);
    BOOST_CHECK_EQUAL(colors[0], 0);
    BOOST_CHECK_EQUAL(colors[1], enc15(0x0000fb));
    BOOST_CHECK_EQUAL(colors[2], enc16(0x0000fb));
    BOOST_CHECK_EQUAL(colors[3], 0x0000fb);

    colors = get_colors<1, 1, 1>(dec16, 0x0000fb);
    BOOST_CHECK_EQUAL(colors[0], 0);
    BOOST_CHECK_EQUAL(colors[1], enc15(dec16(0x0000fb)));
    BOOST_CHECK_EQUAL(colors[2], 0x0000fb);
    BOOST_CHECK_EQUAL(colors[3], enc24(dec16(0x0000fb)));

    colors = get_colors<0, 1, 1>(dec16, 0x0000fb);
    BOOST_CHECK_EQUAL(colors[0], 0);
    BOOST_CHECK_EQUAL(colors[1], 0);
    BOOST_CHECK_EQUAL(colors[2], 0x0000fb);
    BOOST_CHECK_EQUAL(colors[3], enc24(dec16(0x0000fb)));
}
