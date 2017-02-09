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

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestFont
#include "system/redemption_unit_tests.hpp"

#define LOGPRINT

#include "utils/log.hpp"
#include "core/font.hpp"

BOOST_AUTO_TEST_CASE(TestCreateFontChar)
{
    std::unique_ptr<uint8_t[]> data = std::make_unique<uint8_t[]>(8);
    int16_t offset = 0;
    int16_t baseline = 1;
    uint16_t width = 8;
    uint16_t height = 8;
    int16_t incby = 8;

    FontChar fc(data, offset, baseline, width, height, incby);

    FontChar fc(offset, baseline, width, height, incby);

}

BOOST_AUTO_TEST_CASE(TestCreateFont)
{
    {
        Font f;
        BOOST_CHECK(!f.is_loaded());
    }

    Font f(FIXTURES_PATH "/dejavu-sans-10.fv1");
    BOOST_CHECK(f.is_loaded());

    BOOST_CHECK_EQUAL("DejaVu Sans", f.name());
    BOOST_CHECK_EQUAL(1, f.style());
    BOOST_CHECK_EQUAL(10, f.size());

    BOOST_CHECK(!f.glyph_defined(31));
    BOOST_CHECK(f.glyph_defined(32));
    BOOST_CHECK(f.glyph_defined(0x4dff));
    BOOST_CHECK(!f.glyph_defined(0x4dff+1));

    BOOST_CHECK(f.glyph_defined('?'));
    BOOST_CHECK_EQUAL(f.glyph_at('?'), &f.unknown_glyph());

    BOOST_CHECK_EQUAL(&f.glyph_or_unknown(31), &f.unknown_glyph());
    BOOST_CHECK_EQUAL(f.glyph_at(31), static_cast<FontChar*>(nullptr));
    BOOST_CHECK_EQUAL(f.glyph_at(32), &f.glyph_or_unknown(32));
    BOOST_CHECK_NE(f.glyph_at(32), static_cast<FontChar*>(nullptr));
}
