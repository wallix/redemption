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

#include "rvt/vt_emulator.hpp"
#include "rvt/utf8_decoder.hpp"

namespace rvt {

inline std::ostream & operator<<(std::ostream & out, CharacterColor const & ch_color)
{
    auto color = ch_color.color(rvt::color_table);
    return out << "Color("
        << color.red()+0 << ", "
        << color.green()+0 << ", "
        << color.blue()+0 << ")"
    ;
}

inline std::ostream & operator<<(std::ostream & out, Character const & ch)
{
    return out << "Ch("
        << ch.character << ", "
        << ch.foregroundColor << ", "
        << ch.backgroundColor << ", "
        << underlying_cast(ch.rendition)+0 << ", "
        << ch.isRealCharacter << ")"
    ;
}

}

#include <iostream>
#include <iomanip>

BOOST_AUTO_TEST_CASE(TestEmulator)
{
    rvt::VtEmulator emulator(7, 20);
    rvt::Utf8Decoder text_decoder;

    auto send_ucs = [&emulator](rvt::ucs4_char ucs) { emulator.receiveChar(ucs); };
    auto send_zstring = [&emulator, &text_decoder, send_ucs](array_view_const_char av) {
        text_decoder.decode(av.first(av.size()-1), send_ucs);
    };

    for (int i = 0; i < 10; ++i) {
        send_zstring("abc");
    }
    send_zstring("\033[0B\033[31m");
    for (int i = 0; i < 10; ++i) {
        send_zstring("abc");
    }
    send_zstring("\033[44mée\xea\xb0\x80");
    text_decoder.end_decode(send_ucs);

    BOOST_CHECK_EQUAL_RANGES(emulator.getWindowTitle(), array_view<rvt::ucs4_char>());
    send_zstring("\033]2;abc\a");
    BOOST_CHECK_EQUAL_RANGES(emulator.getWindowTitle(), cstr_array_view("abc"));
    send_zstring("\033]0;abcd\a");
    BOOST_CHECK_EQUAL_RANGES(emulator.getWindowTitle(), cstr_array_view("abcd"));

    rvt::Screen const & screen = emulator.getCurrentScreen();
    auto const & lines = screen.getScreenLines();

    BOOST_REQUIRE_EQUAL(lines.size(), 8);

    rvt::Character a_ch('a');
    rvt::Character b_ch('b');
    rvt::Character c_ch('c');

    BOOST_CHECK_EQUAL(lines[0].size(), 20);
    BOOST_CHECK_EQUAL(lines[0][0], a_ch);
    BOOST_CHECK_EQUAL(lines[0][1], b_ch);
    BOOST_CHECK_EQUAL(lines[0][2], c_ch);
    BOOST_CHECK_EQUAL(lines[0][3], a_ch);
    BOOST_CHECK_EQUAL(lines[0][4], b_ch);
    BOOST_CHECK_EQUAL(lines[0][5], c_ch);
    BOOST_CHECK_EQUAL(lines[1].size(), 10);
    BOOST_CHECK_EQUAL(lines[1][0], c_ch);
    BOOST_CHECK_EQUAL(lines[1][1], a_ch);
    BOOST_CHECK_EQUAL(lines[1][2], b_ch);
    BOOST_CHECK_EQUAL(lines[1][3], c_ch);
    BOOST_CHECK_EQUAL(lines[1][4], a_ch);

    rvt::CharacterColor fg(rvt::ColorSpace::System, 1);
    a_ch.foregroundColor = fg;
    b_ch.foregroundColor = fg;
    c_ch.foregroundColor = fg;
    rvt::Character no_ch;

    BOOST_CHECK_EQUAL(lines[2].size(), 20);
    BOOST_CHECK_EQUAL(lines[2][0], no_ch);
    BOOST_CHECK_EQUAL(lines[2][1], no_ch);
    BOOST_CHECK_EQUAL(lines[2][10], a_ch);
    BOOST_CHECK_EQUAL(lines[2][11], b_ch);
    BOOST_CHECK_EQUAL(lines[3].size(), 20);
    BOOST_CHECK_EQUAL(lines[3][0], b_ch);
    BOOST_CHECK_EQUAL(lines[3][1], c_ch);

    rvt::CharacterColor bg(rvt::ColorSpace::System, 4);
    rvt::Character ch(0, fg, bg);
    rvt::Character no_real(0, fg, bg); no_real.isRealCharacter = false;

    BOOST_CHECK_EQUAL(lines[4].size(), 4);
    BOOST_CHECK_EQUAL(lines[4][0], rvt::Character(233, fg, bg));
    BOOST_CHECK_EQUAL(lines[4][1], rvt::Character('e', fg, bg));
    BOOST_CHECK_EQUAL(lines[4][2], rvt::Character(44032, fg, bg));
    BOOST_CHECK_EQUAL(lines[4][3], no_real);

    send_zstring("e");
    text_decoder.end_decode(send_ucs);
    BOOST_CHECK_EQUAL(lines[4].size(), 5);
    BOOST_CHECK_EQUAL(lines[4][3], no_real);
    BOOST_CHECK_EQUAL(lines[4][4], rvt::Character('e', fg, bg));
    emulator.receiveChar(0x311);
    BOOST_CHECK_EQUAL(lines[4][4], rvt::Character(0, fg, bg, rvt::Rendition::ExtendedChar));
    BOOST_CHECK_EQUAL(screen.extendedCharTable().size(), 1);
    BOOST_CHECK_EQUAL_RANGES(screen.extendedCharTable()[0], utils::make_array<rvt::ucs4_char>('e', 0x311));

    BOOST_CHECK_EQUAL(lines[5].size(), 0);
}
