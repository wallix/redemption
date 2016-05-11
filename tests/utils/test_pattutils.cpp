/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2014
    Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestPattUtils
#include "system/redemption_unit_tests.hpp"

//#define LOGNULL
#define LOGPRINT

#include "utils/pattutils.hpp"

BOOST_AUTO_TEST_CASE(TestKbdPattern)
{
    BOOST_CHECK(!contains_kbd_pattern(""));

    BOOST_CHECK(!contains_kbd_pattern("AT"));

    BOOST_CHECK(!contains_kbd_pattern("Bloc-notes"));

    BOOST_CHECK(contains_kbd_pattern("$kbd:gpedit"));

    BOOST_CHECK(contains_kbd_pattern(" $kbd:gpedit\x01" "AT"));

    BOOST_CHECK(contains_kbd_pattern(" $kbd:kill\x01 " "AT "));

    BOOST_CHECK(contains_kbd_pattern("AT\x01$kbd:kill"));

    BOOST_CHECK(!contains_kbd_pattern("$ocr:Bloc-notes"));

    BOOST_CHECK(contains_kbd_pattern("$ocr-kbd:cmd"));

    BOOST_CHECK(contains_kbd_pattern("$kbd-ocr:cmd"));
}

BOOST_AUTO_TEST_CASE(TestOcrPattern)
{
    BOOST_CHECK(!contains_ocr_pattern(""));

    BOOST_CHECK(contains_ocr_pattern("AT"));

    BOOST_CHECK(contains_ocr_pattern("Bloc-notes"));

    BOOST_CHECK(contains_ocr_pattern("$ocr:Bloc-notes"));

    BOOST_CHECK(contains_ocr_pattern("$ocr:Bloc-notes\x01" "AT"));

    BOOST_CHECK(contains_ocr_pattern("$kbd:kill\x01" " AT"));

    BOOST_CHECK(contains_ocr_pattern(" AT\x01$kbd:kill"));

    BOOST_CHECK(!contains_ocr_pattern("$kbd:kill"));

    BOOST_CHECK(contains_ocr_pattern("$ocr-kbd:cmd"));

    BOOST_CHECK(contains_ocr_pattern("$kbd-ocr:cmd"));
}

BOOST_AUTO_TEST_CASE(TestKbdOrOcrPattern)
{
    BOOST_CHECK(!contains_kbd_or_ocr_pattern(""));

    BOOST_CHECK(contains_kbd_or_ocr_pattern("AT"));

    BOOST_CHECK(contains_kbd_or_ocr_pattern("Bloc-notes"));

    BOOST_CHECK(contains_kbd_or_ocr_pattern("$kbd:gpedit"));

    BOOST_CHECK(contains_kbd_or_ocr_pattern(" $kbd:gpedit\x01" "AT"));

    BOOST_CHECK(contains_kbd_or_ocr_pattern(" $kbd:kill\x01 " "AT "));

    BOOST_CHECK(contains_kbd_or_ocr_pattern("AT\x01$kbd:kill"));

    BOOST_CHECK(contains_kbd_or_ocr_pattern("$ocr:Bloc-notes"));

    BOOST_CHECK(contains_kbd_or_ocr_pattern("$ocr-kbd:cmd"));

    BOOST_CHECK(contains_kbd_or_ocr_pattern("$kbd-ocr:cmd"));

    BOOST_CHECK(!contains_kbd_or_ocr_pattern("$ocm:10.10.46.0/24:3389"));
}
