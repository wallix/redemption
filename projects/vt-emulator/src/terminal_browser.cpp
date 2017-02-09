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

#include "rvt/character_color.hpp"
#include "rvt/vt_emulator.hpp"
#include "rvt/utf8_decoder.hpp"
#include "rvt/ansi_rendering.hpp"

#include <iostream>
#include <fstream>
// #include <chrono>
// #include <thread>

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

        std::ofstream out("/tmp/rawdisk/output_term");
        //auto & out = std::cout;
        rvt::ansi_rendering(emulator.getCurrentScreen(), rvt::color_table, out);
    };

    std::string line;
    while (std::getline(std::cin, line)) {
        line += "\n";
        decoder.decode(line, send_ucs);
    }
    decoder.end_decode(send_ucs);
}
