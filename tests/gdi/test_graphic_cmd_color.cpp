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


#define RED_TEST_MODULE TestGdCmdConverter
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "core/RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "gdi/graphic_cmd_color.hpp"
#include "utils/colors.hpp"

RED_AUTO_TEST_CASE(TestGdCmdConverter)
{
    encode_color15 enc;
    decode_color16 dec;
    BGRColor raw_color(0x8ba93c);
    RDPColor color16 = enc(raw_color);
    RDPOpaqueRect opaque_rect({}, color16);

    RED_CHECK_EQ(gdi::GraphicCmdColor::is_encodable_cmd_color(opaque_rect).value, true);
    RED_CHECK_EQ(gdi::GraphicCmdColor::is_encodable_cmd_color(RDPMemBlt{0, {}, 0, 0, 0, 0}).value, false);

    auto c = enc(dec(color16));
    RED_CHECK_NE(opaque_rect.color, c);
    gdi::GraphicCmdColor::encode_cmd_color([=](RDPColor c){ return enc(dec(c)); }, opaque_rect);
    RED_CHECK_EQ(opaque_rect.color, c);
}
