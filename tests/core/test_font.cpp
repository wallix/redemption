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

   Unit test to fonts file
   Using lib boost functions, some tests need to be added

*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "core/font.hpp"

using namespace std::string_view_literals;


RED_AUTO_TEST_CASE(TestCreateFontChar)
{
    std::unique_ptr<uint8_t[]> data = std::make_unique<uint8_t[]>(8);
    int16_t offsetx = 0;
    int16_t offsety = 1;
    uint16_t width = 8;
    uint16_t height = 8;
    int16_t incby = 8;

    RDPFontChar fc2(offsetx, offsety, width, height, incby);
    RED_CHECK_EQUAL(fc2.datasize(), 8);
    RED_CHECK_EQUAL(fc2.offsetx, offsetx);
    RED_CHECK_EQUAL(fc2.offsety, offsety);
    RED_CHECK_EQUAL(fc2.width, width);
    RED_CHECK_EQUAL(fc2.height, height);
    RED_CHECK_EQUAL(fc2.incby, incby);

}

RED_AUTO_TEST_CASE(TestCreateFont)
{
    RED_CHECK(!FontData().is_loaded());

    FontData data(FIXTURES_PATH "/dejavu_14.rbf2");
    RED_CHECK(data.is_loaded());

    Font f = data.font();

    RED_CHECK(!f.item(31).is_valid);
    RED_CHECK(f.item(' ').is_valid);
    RED_CHECK(f.item('?').is_valid);
    RED_CHECK(f.item(0x4dff).is_valid);
    RED_CHECK(!f.item(0x30dff).is_valid);
    RED_CHECK(&f.item(31).view == &f.unknown_glyph());
    RED_CHECK(&f.item(0x30dff).view == &f.unknown_glyph());
}
