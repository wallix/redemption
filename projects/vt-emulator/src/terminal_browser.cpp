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

#include "terminal_emulator.hpp"

#include <iostream>
#include <memory>

#include <cstring>


struct TerminalEmulatorDeleter
{
    void operator()(TerminalEmulator * p) noexcept
    { terminal_emulator_deinit(p); }
};

int main(int ac, char ** av)
{
    std::unique_ptr<TerminalEmulator, TerminalEmulatorDeleter> uptr{terminal_emulator_init(68, 117, 1)};

    auto filename = ac > 1 ? av[1] : "screen.json";
    char c;
    int n = 0;
    auto emu = uptr.get();
    #define PError(x) do { if (int err = (x)) std::cerr << (err < 0 ? "internal error" : strerror(err)) << std::endl; } while (0)
    while (std::cin.get(c)) {
        PError(terminal_emulator_feed(emu, &c, 1));

        if (c == '\n' || ++n == 100) {
            n = 0;
            PError(terminal_emulator_write(emu, filename));
        }
    }
    PError(terminal_emulator_finish(emu));
    PError(terminal_emulator_write(emu, filename));
}
