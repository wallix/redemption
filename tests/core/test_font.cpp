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

#define RED_TEST_MODULE TestFont
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "core/font.hpp"


RED_AUTO_TEST_CASE(TestCreateFontChar)
{
    std::unique_ptr<uint8_t[]> data = std::make_unique<uint8_t[]>(8);
    int16_t offsetx = 0;
    int16_t offsety = 1;
    uint16_t width = 8;
    uint16_t height = 8;
    int16_t incby = 8;

    FontChar fc2(offsetx, offsety, width, height, incby);
    RED_CHECK_EQUAL(fc2.datasize(), 8);
    RED_CHECK_EQUAL(fc2.offsetx, offsetx);
    RED_CHECK_EQUAL(fc2.offsety, offsety);
    RED_CHECK_EQUAL(fc2.width, width);
    RED_CHECK_EQUAL(fc2.height, height);
    RED_CHECK_EQUAL(fc2.incby, incby);

}

RED_AUTO_TEST_CASE(TestCreateFont)
{
    {
        Font f;
        RED_CHECK(!f.is_loaded());
    }

    Font f(FIXTURES_PATH "/Lato-Light_16.rbf");
    RED_CHECK(f.is_loaded());

    RED_CHECK_EQUAL("Lato", f.name());
    RED_CHECK_EQUAL(1, f.style());
    RED_CHECK_EQUAL(16, f.size());

    RED_CHECK(!f.glyph_defined(31));
    RED_CHECK(f.glyph_defined(32));
    RED_CHECK(f.glyph_defined(0x4dff));
    RED_CHECK(!f.glyph_defined(0x4dff+1));

    RED_CHECK(f.glyph_defined('?'));
    RED_CHECK(f.glyph_or_unknown('?').item_compare(f.unknown_glyph()));

    FontCharView* nullptr_char = nullptr;
    RED_CHECK_EQUAL(&f.glyph_or_unknown(31), &f.unknown_glyph());
    RED_CHECK_EQUAL(f.glyph_at(31), nullptr_char);
    RED_CHECK_EQUAL(f.glyph_at(32), &f.glyph_or_unknown(32));
    RED_CHECK_NE(f.glyph_at(32), nullptr_char);
}
