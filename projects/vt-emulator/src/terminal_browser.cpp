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
#include "rvt/json_rendering.hpp"

#include <iostream>
#include <fstream>


int main(int ac, char ** av)
{
    rvt::VtEmulator emulator(68, 117);
    rvt::Utf8Decoder decoder;

    auto send_ucs = [&emulator](rvt::ucs4_char ucs) {
        emulator.receiveChar(ucs);
    };

    auto filename = ac > 1 ? av[1] : "screen.json";
    auto write_file = [&emulator, filename]() {
        std::ofstream out(filename);
        rvt::json_rendering(emulator.getWindowTitle(), emulator.getCurrentScreen(), rvt::color_table, out);
    };

    char c;
    int n = 0;
    while (std::cin.get(c)) {
        decoder.decode(bytes_array(&c, 1), send_ucs);

        if (c == '\n' || ++n == 100) {
            n = 0;
            write_file();
        }
    }
    decoder.end_decode(send_ucs);
    write_file();
}
