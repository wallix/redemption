/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2016
*   Author(s): Jonathan Poelen
*/


#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestRect
#include "system/redemption_unit_tests.hpp"

#include "rvt/utf8_decoder.hpp"

BOOST_AUTO_TEST_CASE(TestCharsets)
{
    struct Accu {
        std::vector<rvt::ucs4_char> v;
        void operator()(rvt::ucs4_char ucs) { v.push_back(ucs); }
    };
    rvt::Utf8Decoder decoder;

    BOOST_CHECK_EQUAL_RANGES(
        decoder.decode(cstr_array_view("abcd"), Accu()).v,
        utils::make_array<rvt::ucs4_char>('a', 'b', 'c', 'd')
    );
    BOOST_CHECK_EQUAL_RANGES(
        decoder.end_decode(Accu()).v,
        utils::make_array<rvt::ucs4_char>()
    );

    BOOST_CHECK_EQUAL_RANGES(
        decoder.decode(cstr_array_view("abc"), Accu()).v,
        utils::make_array<rvt::ucs4_char>()
    );
    BOOST_CHECK_EQUAL_RANGES(
        decoder.end_decode(Accu()).v,
        utils::make_array<rvt::ucs4_char>('a', 'b', 'c')
    );

    BOOST_CHECK_EQUAL_RANGES(
        decoder.decode(cstr_array_view("\xea\xb0\x80"), Accu()).v,
        utils::make_array<rvt::ucs4_char>()
    );
    BOOST_CHECK_EQUAL_RANGES(
        decoder.end_decode(Accu()).v,
        utils::make_array<rvt::ucs4_char>(0xac00u)
    );

    // utf8 format error

    BOOST_CHECK_EQUAL_RANGES(
        decoder.decode(cstr_array_view("\xea\xb0""a\xea""a\x80"), Accu()).v,
        utils::make_array<rvt::ucs4_char>(rvt::replacement_character, rvt::replacement_character, 'a', rvt::replacement_character)
    );
    BOOST_CHECK_EQUAL_RANGES(
        decoder.end_decode(Accu()).v,
        utils::make_array<rvt::ucs4_char>('a', rvt::replacement_character)
    );

    BOOST_CHECK_EQUAL_RANGES(
        decoder.decode(cstr_array_view("\xfa\xb0\x80""ab"), Accu()).v,
        utils::make_array<rvt::ucs4_char>(rvt::replacement_character, rvt::replacement_character, rvt::replacement_character, 'a')
    );
    BOOST_CHECK_EQUAL_RANGES(
        decoder.end_decode(Accu()).v,
        utils::make_array<rvt::ucs4_char>('b')
    );
}
