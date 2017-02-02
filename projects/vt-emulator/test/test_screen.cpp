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

#include "rvt/screen.hpp"


BOOST_AUTO_TEST_CASE(TestScreenCtor)
{
    rvt::Screen screen(40, 40);

    BOOST_CHECK_EQUAL(screen.bottomMargin(), 39);
    BOOST_CHECK_EQUAL(screen.topMargin(), 0);
    BOOST_CHECK_EQUAL(screen.cursorX(), 0);
    BOOST_CHECK_EQUAL(screen.cursorY(), 0);
    BOOST_CHECK_EQUAL(screen.getColumns(), 40);
    BOOST_CHECK_EQUAL(screen.getLines(), 40);
    BOOST_CHECK_EQUAL(screen.lastScrolledRegion(), Rect(0, 0, 39, 39));
    BOOST_CHECK_EQUAL(screen.scrolledLines(), -39);

    using Mode = rvt::Screen::Mode;
    BOOST_CHECK_EQUAL(screen.getMode(Mode::Cursor), true);
    BOOST_CHECK_EQUAL(screen.getMode(Mode::Wrap), true);
    BOOST_CHECK_EQUAL(screen.getMode(Mode::Origin), false);
    BOOST_CHECK_EQUAL(screen.getMode(Mode::Insert), false);
    BOOST_CHECK_EQUAL(screen.getMode(Mode::Screen), false);
    BOOST_CHECK_EQUAL(screen.getMode(Mode::NewLine), false);
}

BOOST_AUTO_TEST_CASE(TestScreenCursor)
{
    rvt::Screen screen(40, 40);
    BOOST_CHECK_EQUAL(screen.cursorX(), 0);
    BOOST_CHECK_EQUAL(screen.cursorY(), 0);

    screen.cursorDown(0);
    BOOST_CHECK_EQUAL(screen.cursorX(), 0);
    BOOST_CHECK_EQUAL(screen.cursorY(), 1);
    screen.cursorDown(0);
    BOOST_CHECK_EQUAL(screen.cursorX(), 0);
    BOOST_CHECK_EQUAL(screen.cursorY(), 2);
    screen.cursorUp(0);
    BOOST_CHECK_EQUAL(screen.cursorX(), 0);
    BOOST_CHECK_EQUAL(screen.cursorY(), 1);
    screen.cursorUp(0);
    BOOST_CHECK_EQUAL(screen.cursorX(), 0);
    BOOST_CHECK_EQUAL(screen.cursorY(), 0);

    screen.cursorDown(1);
    BOOST_CHECK_EQUAL(screen.cursorX(), 0);
    BOOST_CHECK_EQUAL(screen.cursorY(), 1);
    screen.cursorDown(1);
    BOOST_CHECK_EQUAL(screen.cursorX(), 0);
    BOOST_CHECK_EQUAL(screen.cursorY(), 2);
    screen.cursorDown(2);
    BOOST_CHECK_EQUAL(screen.cursorX(), 0);
    BOOST_CHECK_EQUAL(screen.cursorY(), 4);
    screen.cursorDown(40);
    BOOST_CHECK_EQUAL(screen.cursorX(), 0);
    BOOST_CHECK_EQUAL(screen.cursorY(), 39);
    screen.cursorUp(10);
    BOOST_CHECK_EQUAL(screen.cursorX(), 0);
    BOOST_CHECK_EQUAL(screen.cursorY(), 29);
    screen.cursorUp(40);
    BOOST_CHECK_EQUAL(screen.cursorX(), 0);
    BOOST_CHECK_EQUAL(screen.cursorY(), 0);

    screen.cursorRight(2);
    BOOST_CHECK_EQUAL(screen.cursorX(), 2);
    BOOST_CHECK_EQUAL(screen.cursorY(), 0);
    screen.cursorRight(200);
    BOOST_CHECK_EQUAL(screen.cursorX(), 39);
    BOOST_CHECK_EQUAL(screen.cursorY(), 0);
    screen.cursorLeft(3);
    BOOST_CHECK_EQUAL(screen.cursorX(), 36);
    BOOST_CHECK_EQUAL(screen.cursorY(), 0);
    screen.cursorLeft(40);
    BOOST_CHECK_EQUAL(screen.cursorX(), 0);
    BOOST_CHECK_EQUAL(screen.cursorY(), 0);
    screen.cursorRight(1);
    BOOST_CHECK_EQUAL(screen.cursorX(), 1);
    BOOST_CHECK_EQUAL(screen.cursorY(), 0);
    screen.cursorLeft(2);
    BOOST_CHECK_EQUAL(screen.cursorX(), 0);
    BOOST_CHECK_EQUAL(screen.cursorY(), 0);
}

BOOST_AUTO_TEST_CASE(TestScreenInsert)
{
    auto to_string = [](rvt::Screen & screen) {
        int nlines = screen.getLines();
        int ncolumns = screen.getColumns();
        std::string s(nlines * (ncolumns + 2 + 1), '?');
        auto p = s.begin();
        auto & screen_lines = screen.getScreenLines();
        for (int i{}; i < nlines; ++i) {
            auto & lines = screen_lines[i];
            *p++ = '[';
            int effective_ncolumns(lines.size());
            int j = 0;
            for (; j < std::min(effective_ncolumns, ncolumns); ++j) {
                *p++ = lines[j].character;
            }
            p = std::fill_n(p, ncolumns - j, ' ');
            *p++ = ']';
            *p++ = '\n';
        }
        return s;
    };
    rvt::Screen screen(4, 8);

    BOOST_CHECK_EQUAL(to_string(screen), "[        ]\n[        ]\n[        ]\n[        ]\n");

    using Mode = rvt::Screen::Mode;

    screen.displayCharacter('a');
    BOOST_CHECK_EQUAL(to_string(screen), "[a       ]\n[        ]\n[        ]\n[        ]\n");
    screen.displayCharacter('b');
    BOOST_CHECK_EQUAL(to_string(screen), "[ab      ]\n[        ]\n[        ]\n[        ]\n");
    screen.displayCharacter('c');
    BOOST_CHECK_EQUAL(to_string(screen), "[abc     ]\n[        ]\n[        ]\n[        ]\n");
    screen.cursorDown(1);
    screen.displayCharacter('d');
    BOOST_CHECK_EQUAL(to_string(screen), "[abc     ]\n[   d    ]\n[        ]\n[        ]\n");
    screen.cursorLeft(1);
    BOOST_CHECK_EQUAL(to_string(screen), "[abc     ]\n[   d    ]\n[        ]\n[        ]\n");
    screen.displayCharacter('e');
    BOOST_CHECK_EQUAL(to_string(screen), "[abc     ]\n[   e    ]\n[        ]\n[        ]\n");
    screen.setCursorX(0);
    screen.displayCharacter('f');
    BOOST_CHECK_EQUAL(to_string(screen), "[abc     ]\n[f  e    ]\n[        ]\n[        ]\n");
    screen.setMode(Mode::Insert);
    screen.displayCharacter('g');
    BOOST_CHECK_EQUAL(to_string(screen), "[abc     ]\n[fg  e   ]\n[        ]\n[        ]\n");
    screen.displayCharacter('h');
    BOOST_CHECK_EQUAL(to_string(screen), "[abc     ]\n[fgh  e  ]\n[        ]\n[        ]\n");
    screen.displayCharacter('i');
    BOOST_CHECK_EQUAL(to_string(screen), "[abc     ]\n[fghi  e ]\n[        ]\n[        ]\n");
    screen.displayCharacter('j');
    BOOST_CHECK_EQUAL(to_string(screen), "[abc     ]\n[fghij  e]\n[        ]\n[        ]\n");
    screen.displayCharacter('k');
    BOOST_CHECK_EQUAL(to_string(screen), "[abc     ]\n[fghijk  ]\n[        ]\n[        ]\n");
    screen.displayCharacter('l');
    BOOST_CHECK_EQUAL(to_string(screen), "[abc     ]\n[fghijkl ]\n[        ]\n[        ]\n");
    screen.displayCharacter('m');
    BOOST_CHECK_EQUAL(to_string(screen), "[abc     ]\n[fghijklm]\n[        ]\n[        ]\n");
    screen.displayCharacter('n');
    BOOST_CHECK_EQUAL(to_string(screen), "[abc     ]\n[fghijklm]\n[n       ]\n[        ]\n");
    screen.displayCharacter('o');
    BOOST_CHECK_EQUAL(to_string(screen), "[abc     ]\n[fghijklm]\n[no      ]\n[        ]\n");
    screen.resetMode(Mode::Wrap);
    screen.cursorRight(5);
    screen.displayCharacter('p');
    BOOST_CHECK_EQUAL(to_string(screen), "[abc     ]\n[fghijklm]\n[no     p]\n[        ]\n");
    screen.displayCharacter('q');
    BOOST_CHECK_EQUAL(to_string(screen), "[abc     ]\n[fghijklm]\n[no     q]\n[        ]\n");
    screen.setMode(Mode::Wrap);
    screen.displayCharacter('r');
    BOOST_CHECK_EQUAL(to_string(screen), "[abc     ]\n[fghijklm]\n[no     q]\n[r       ]\n");
    screen.newLine();
    BOOST_CHECK_EQUAL(to_string(screen), "[fghijklm]\n[no     q]\n[r       ]\n[        ]\n");
    screen.newLine();
    BOOST_CHECK_EQUAL(to_string(screen), "[no     q]\n[r       ]\n[        ]\n[        ]\n");
    screen.displayCharacter('s');
    BOOST_CHECK_EQUAL(to_string(screen), "[no     q]\n[r       ]\n[        ]\n[ s      ]\n");
    screen.resetMode(Mode::NewLine);
    screen.newLine();
    screen.displayCharacter('t');
    BOOST_CHECK_EQUAL(to_string(screen), "[r       ]\n[        ]\n[ s      ]\n[  t     ]\n");
}
