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
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <thread>

void write_file(rvt::VtEmulator & emulator);

// script -f >(./bin/terminal_browser)
// while [ 1 ] ; do a2h < /tmp/rawdisk/output_term > /tmp/rawdisk/output.html ; sleep 2 ; done

int main()
{
    rvt::VtEmulator emulator;
    rvt::Utf8Decoder decoder;

    auto send_ucs = [&emulator](rvt::ucs4_char ucs) {
//         switch (uc) {
//             case '\n': std::cout << "\\n"; break;
//             case '\t': std::cout << "\\t"; break;
//             case '\v': std::cout << "\\v"; break;
//             case '\f': std::cout << "\\f"; break;
//             case 033 : std::cout << "\\e"; break;
//             case '\b': std::cout << "\\b"; break;
//             default : {
//                 char utf8_ch[4];
//                 std::size_t const n = ucs4_to_utf8(uc, utf8_ch);
//                 std::cout.write(utf8_ch, static_cast<std::streamsize>(n));
//             }
//         }
//         std::cout << "\n--------------------------------------------\n";
        emulator.receiveChar(ucs);
//         std::this_thread::sleep_for(std::chrono::milliseconds{4});
        write_file(emulator);
    };

    std::string line;
    while (std::getline(std::cin, line)) {
        line += "\n";
        decoder.decode(line, send_ucs);
    }
    decoder.end_decode(send_ucs);
}

void write_file(rvt::VtEmulator & emulator)
{
    std::ofstream out("/tmp/rawdisk/output_term");
    //auto & out = std::cout;

    rvt::Screen const & screen = emulator.getCurrentScreen();

    auto print_uc = [&out](rvt::ucs4_char uc) {
        char utf8_ch[4];
        std::size_t const n = ucs4_to_utf8(uc, utf8_ch);
        out.write(utf8_ch, static_cast<std::streamsize>(n));
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

    auto print_color = [&out](char const * cmd, rvt::CharacterColor const & ch_color) {
        auto color = ch_color.color(rvt::color_table);
        out << cmd << (color.red()+0) << ";" << (color.green()+0) << ";" << (color.blue()+0);
    };
    auto print_mode = [&out](char const * cmd, rvt::Character const & ch, rvt::Rendition r) {
        if (bool(ch.rendition & r)) {
            out << cmd;
        }
    };

    // Format
    //@{
    rvt::Rendition previous_rendition = rvt::Rendition::Default;
    rvt::CharacterColor previous_fg(rvt::ColorSpace::Default, 0);
    rvt::CharacterColor previous_bg(rvt::ColorSpace::Default, 1);
    //@}
    out << "\033[0";
    print_color(";38;2;", previous_fg);
    print_color(";48;2;", previous_bg);
    out << "m";
    for (auto const & line : screen.getScreenLines()) {
        for (rvt::Character const & ch : line) {
            if (!ch.isRealCharacter) {
                continue;
            }
            if (ch.backgroundColor != previous_bg
             || ch.foregroundColor != previous_fg
             || ch.rendition != previous_rendition
            ) {
                out << "\033[0";
                print_mode(";1", ch, rvt::Rendition::Bold);
                print_mode(";3", ch, rvt::Rendition::Italic);
                print_mode(";4", ch, rvt::Rendition::Underline);
                print_mode(";5", ch, rvt::Rendition::Blink);
                print_mode(";7", ch, rvt::Rendition::Reverse);
                print_color(";38;2;", ch.foregroundColor);
                print_color(";48;2;", ch.backgroundColor);
                out << "m";
                previous_bg = ch.backgroundColor;
                previous_fg = ch.foregroundColor;
                previous_rendition = ch.rendition;
            }
            print_ch(ch);
        }
        out << '\n';
    }
}
