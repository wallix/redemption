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

BOOST_AUTO_TEST_CASE(Test_uc_to_utf8) {
    uint8_t utf8_ch[4]{};

    for (uint8_t c = 0; c <= 126; ++c) {
        BOOST_CHECK_EQUAL(1, rvt::unsafe_ucs4_to_utf8(c, utf8_ch));
        BOOST_CHECK_EQUAL(c, utf8_ch[0]);
    }

    BOOST_CHECK_EQUAL(2, rvt::unsafe_ucs4_to_utf8(0xa2, utf8_ch));
    BOOST_CHECK_EQUAL(0xc2, utf8_ch[0]);
    BOOST_CHECK_EQUAL(0xa2, utf8_ch[1]);

    BOOST_CHECK_EQUAL(3, rvt::unsafe_ucs4_to_utf8(0xac00, utf8_ch));
    BOOST_CHECK_EQUAL(0xea, utf8_ch[0]);
    BOOST_CHECK_EQUAL(0xb0, utf8_ch[1]);
    BOOST_CHECK_EQUAL(0x80, utf8_ch[2]);

    BOOST_CHECK_EQUAL(4, rvt::unsafe_ucs4_to_utf8(0x10348, utf8_ch));
    BOOST_CHECK_EQUAL(0xf0, utf8_ch[0]);
    BOOST_CHECK_EQUAL(0x90, utf8_ch[1]);
    BOOST_CHECK_EQUAL(0x8d, utf8_ch[2]);
    BOOST_CHECK_EQUAL(0x88, utf8_ch[3]);
}

BOOST_AUTO_TEST_CASE(TestUtf8Decoder)
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
        utils::make_array<rvt::ucs4_char>('a', 'b', 'c')
    );
    BOOST_CHECK_EQUAL_RANGES(
        decoder.end_decode(Accu()).v,
        utils::make_array<rvt::ucs4_char>()
    );

    BOOST_CHECK_EQUAL_RANGES(
        decoder.decode(cstr_array_view("\xea\xb0\x80"), Accu()).v,
        utils::make_array<rvt::ucs4_char>(0xac00u)
    );
    BOOST_CHECK_EQUAL_RANGES(
        decoder.end_decode(Accu()).v,
        utils::make_array<rvt::ucs4_char>()
    );

    BOOST_CHECK_EQUAL_RANGES(
        decoder.decode(cstr_array_view("\xb7p\xc3\xc7"), Accu()).v,
        utils::make_array<rvt::ucs4_char>(0xb7, 'p', 0xc3, 0xc7)
    );
    BOOST_CHECK_EQUAL_RANGES(
        decoder.end_decode(Accu()).v,
        utils::make_array<rvt::ucs4_char>()
    );

    BOOST_CHECK_EQUAL_RANGES(
        decoder.decode(cstr_array_view("\xea\xb0"), Accu()).v,
        utils::make_array<rvt::ucs4_char>()
    );
    BOOST_CHECK_EQUAL_RANGES(
        decoder.decode(cstr_array_view("\x80"), Accu()).v,
        utils::make_array<rvt::ucs4_char>(0xac00u)
    );
    BOOST_CHECK_EQUAL_RANGES(
        decoder.end_decode(Accu()).v,
        utils::make_array<rvt::ucs4_char>()
    );

    // utf8 format error

    BOOST_CHECK_EQUAL_RANGES(
        decoder.decode(cstr_array_view("\xea\xb0"), Accu()).v,
        utils::make_array<rvt::ucs4_char>()
    );
    BOOST_CHECK_EQUAL_RANGES(
        decoder.end_decode(Accu()).v,
        utils::make_array<rvt::ucs4_char>(0xea, 0xb0)
    );

    BOOST_CHECK_EQUAL_RANGES(
        decoder.decode(cstr_array_view("\xea\xb0""a\xea""a\x80"), Accu()).v,
        utils::make_array<rvt::ucs4_char>(0xea, 0xb0, 'a', 0xea, 'a', 0x80)
    );
    BOOST_CHECK_EQUAL_RANGES(
        decoder.end_decode(Accu()).v,
        utils::make_array<rvt::ucs4_char>()
    );

    BOOST_CHECK_EQUAL_RANGES(
        decoder.decode(cstr_array_view("\xfa\xb0\x80""ab"), Accu()).v,
        utils::make_array<rvt::ucs4_char>(0xfa, 0xb0, 0x80, 'a', 'b')
    );
    BOOST_CHECK_EQUAL_RANGES(
        decoder.end_decode(Accu()).v,
        utils::make_array<rvt::ucs4_char>()
    );
}
