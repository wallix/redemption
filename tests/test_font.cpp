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
#include <boost/test/auto_unit_test.hpp>

#include "font.hpp"
#include <sstream>
#include <iostream>
#include <string>
#include <string.h>

#include "file_loc.hpp"

BOOST_AUTO_TEST_CASE(TestCreateFont)
{
    // test loading a font from default file


    openlog("xrdp", LOG_CONS | LOG_PERROR, LOG_USER);
    setlogmask(LOG_MASK(LOG_DEBUG));
    LOG(LOG_DEBUG, "reading fonts\n");

    Font f(FIXTURES_PATH "/" DEFAULT_FONT_NAME);
    BOOST_CHECK_EQUAL(std::string("DejaVu Sans"), std::string(f.name));
    BOOST_CHECK_EQUAL(1, f.style);
    BOOST_CHECK_EQUAL(10, f.size);
    BOOST_CHECK(!f.font_items[31]);
    BOOST_CHECK(f.font_items[32]);
    BOOST_CHECK(f.font_items[0x4dff]);
}
