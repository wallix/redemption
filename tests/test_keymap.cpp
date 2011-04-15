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

   Unit test to keymap file
   Using lib boost functions, some tests need to be added

*/


#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_MODULE TestKeymap
#include <boost/test/auto_unit_test.hpp>

#include "keymap.hpp"
#include <sstream>
#include <iostream>
#include <string>
#include <string.h>


BOOST_AUTO_TEST_CASE(TestKeymapFromFile)
{
    // test we can read from a file (and not only from a stream)
    Keymap test_keymap(FIXTURES_PATH "/km-0409.ini");
}

BOOST_AUTO_TEST_CASE(TestKeymapFromLine)
{
    std::stringstream test_stream(
    "[shift]\n"
    "Key8=0:0\n"
    "Key9=65307:27\n"
    "Key10=38:38\n"
    "Key11=233:233\n"
    "Key12=34:34\n"
    "Key13=39:39\n"
    "Key14=40:40\n"
    "Key15=45:45\n"
    "Key16=232:232\n"
    "Key17=95:95\n"
    "Key18=231:231\n"
    "Key19=224:224\n"
    "Key20=41:41\n"
    "Key21=61:61\n"
    "Key22=65288:8\n"
    "Key23=65289:9\n"
    "[noshift]\n"
    "Key8=0:0\n"
    "Key9=65307:27\n"
    "Key10=38:38\n"
    "Key11=233:233\n"
    "Key12=34:34\n"
    "[shiftcapslock]\n"
    "Key8=0:0\n"
    "Key9=65307:27\n"
    "Key10=38:38\n"
    "Key11=233:233\n"
    "Key12=34:34\n"
    );

    Keymap keymap(test_stream);
}


BOOST_AUTO_TEST_CASE(TestKeymapConfig1)
{
    //cheking key params values
    std::stringstream test_stream(
    "[shift]\n"
    "Key8=0:0\n"
    "Key9=65307:27\n"
    "Key10=38:38\n"
    "Key11=233:233\n"
    "Key12=34:34\n"
    "Key13=39:39\n"
    "Key14=40:40\n"
    "Key15=45:45\n"
    "[shiftcapslock]\n"
    "Key8=0:0\n"
    "Key9=10:15\n"
    "Key10=98:85\n"
    "Key11=233:233\n"
    "Key12=34:34\n"
    );

    Keymap test_keymap(test_stream);

    BOOST_CHECK_EQUAL(0, test_keymap.keys_shift[8].chr);
    BOOST_CHECK_EQUAL(0, test_keymap.keys_shift[8].sym);

    BOOST_CHECK_EQUAL(27, test_keymap.keys_shift[9].chr);
    BOOST_CHECK_EQUAL(65307, test_keymap.keys_shift[9].sym);

    BOOST_CHECK_EQUAL(38, test_keymap.keys_shift[10].chr);
    BOOST_CHECK_EQUAL(38, test_keymap.keys_shift[10].sym);

    BOOST_CHECK_EQUAL(0, test_keymap.keys_shiftcapslock[8].chr);
    BOOST_CHECK_EQUAL(0, test_keymap.keys_shiftcapslock[8].sym);

    BOOST_CHECK_EQUAL(15, test_keymap.keys_shiftcapslock[9].chr);
    BOOST_CHECK_EQUAL(10, test_keymap.keys_shiftcapslock[9].sym);

    BOOST_CHECK_EQUAL(85, test_keymap.keys_shiftcapslock[10].chr);
    BOOST_CHECK_EQUAL(98, test_keymap.keys_shiftcapslock[10].sym);
}

BOOST_AUTO_TEST_CASE(TestKeymapConfig2)
{
    //cheking key params values by default

    std::stringstream test_stream(
    "[shift]\n"
    "Key8=0:0\n"
    "Key9=65307:27\n"
    "Key10=38:38\n"
    "Key11=233:233\n"
    "Key12=34:34\n"
    "Key13=39:39\n"
    "Key14=40:40\n"
    "Key15=45:45\n"
    "[shiftcapslock]\n"
    "Key8=0:0\n"
    "Key9=10:15\n"
    "Key10=98:85\n"
    "Key11=233:233\n"
    "Key12=34:34\n"
    );

    Keymap test_keymap(test_stream);

    BOOST_CHECK_EQUAL(0, test_keymap.keys_noshift[8].chr);
    BOOST_CHECK_EQUAL(0, test_keymap.keys_noshift[8].sym);

    BOOST_CHECK_EQUAL(223, test_keymap.keys_altgr[39].chr);
    BOOST_CHECK_EQUAL(223, test_keymap.keys_altgr[39].sym);

    BOOST_CHECK_EQUAL(0, test_keymap.keys_shift[75].chr);
    BOOST_CHECK_EQUAL(65478, test_keymap.keys_shift[75].sym);

    BOOST_CHECK_EQUAL(0, test_keymap.keys_shiftcapslock[97].chr);
    BOOST_CHECK_EQUAL(0, test_keymap.keys_shiftcapslock[97].sym);

    BOOST_CHECK_EQUAL(0, test_keymap.keys_capslock[110].chr);
    BOOST_CHECK_EQUAL(65299, test_keymap.keys_capslock[110].sym);
}
