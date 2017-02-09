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
*   Author(s): Jonathan Poelen;
*
*   Based on Konsole, an X terminal
*/

#include "rvt/character.hpp"
#include "rvt/charsets.hpp"
#include "rvt/screen.hpp"
#include "rvt/vt_emulator.hpp"
#include "rvt/utf8_decoder.hpp"

#include <iostream>
#include <iomanip>

int main()
{
    rvt::VtEmulator emulator(40, 40);

    for (int i = 0; i < 20; ++i) {
        emulator.receiveChar('a');
        emulator.receiveChar('b');
        emulator.receiveChar('c');
    }
    emulator.receiveChar('\033');
    emulator.receiveChar('[');
    emulator.receiveChar('0');
    emulator.receiveChar('B');
    emulator.receiveChar('\033');
    emulator.receiveChar('[');
    emulator.receiveChar('3');
    emulator.receiveChar('1');
    emulator.receiveChar('m');
    for (int i = 0; i < 20; ++i) {
        emulator.receiveChar('a');
        emulator.receiveChar('b');
        emulator.receiveChar('c');
    }
    emulator.receiveChar('\033');
    emulator.receiveChar('[');
    emulator.receiveChar('4');
    emulator.receiveChar('4');
    emulator.receiveChar('m');
    emulator.receiveChar(UTF8toUnicodeIterator("é").code());
    emulator.receiveChar('e');
    emulator.receiveChar(0x311);
    emulator.receiveChar(0xac00);


    rvt::Screen const & screen = emulator.getCurrentScreen();

    auto print_uc = [](rvt::ucs4_char uc) {
        char utf8_ch[4];
        std::size_t const n = ucs4_to_utf8(uc, utf8_ch);
        std::cout.write(utf8_ch, static_cast<std::streamsize>(n));
    };

    auto print_ch = [&screen, print_uc](rvt::Character const & ch) {
        if (ch.isRealCharacter) {
            if (ch.is_extended()) {
                for (rvt::ucs4_char uc : screen.extendedCharTable()[ch.character]) {
                    print_uc(uc);
                }
            }
            else {
                print_uc(ch.character);
            }
        }
    };

    int i = 0;
    for (auto const & line : screen.getScreenLines()) {
        std::cout << std::setw(4) << ++i << " ";
        for (rvt::Character const & ch : line) {
            print_ch(ch);
        }
        std::cout << '\n';
    }

    auto print_color = [](char const * cmd, rvt::CharacterColor const & ch_color) {
        auto color = ch_color.color(rvt::color_table);
        std::cout << cmd << (color.red()+0) << ";" << (color.green()+0) << ";" << (color.blue()+0);
    };
    auto print_mode = [](char const * cmd, rvt::Character const & ch, rvt::Rendition r) {
        if (bool(ch.rendition & r)) {
            std::cout << cmd;
        }
    };

    // Format
    //@{
    rvt::Rendition previous_rendition = rvt::Rendition::Default;
    rvt::CharacterColor previous_fg(rvt::ColorSpace::Default, 0);
    rvt::CharacterColor previous_bg(rvt::ColorSpace::Default, 1);
    //@}
    std::cout << "\033[0";
    print_color(";38;2;", previous_fg);
    print_color(";48;2;", previous_bg);
    std::cout << "m";
    for (auto const & line : screen.getScreenLines()) {
        for (rvt::Character const & ch : line) {
            if (!ch.isRealCharacter) {
                continue;
            }
            if (ch.backgroundColor != previous_bg
             || ch.foregroundColor != previous_fg
             || ch.rendition != previous_rendition
            ) {
                std::cout << "\033[0";
                print_mode(";1", ch, rvt::Rendition::Bold);
                print_mode(";3", ch, rvt::Rendition::Italic);
                print_mode(";4", ch, rvt::Rendition::Underline);
                print_mode(";5", ch, rvt::Rendition::Blink);
                print_mode(";7", ch, rvt::Rendition::Reverse);
                print_color(";38;2;", ch.foregroundColor);
                print_color(";48;2;", ch.backgroundColor);
                std::cout << "m";
                previous_bg = ch.backgroundColor;
                previous_fg = ch.foregroundColor;
                previous_rendition = ch.rendition;
            }
            print_ch(ch);
        }
        std::cout << '\n';
    }
}
