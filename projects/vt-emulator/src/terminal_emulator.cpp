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

#include "terminal_emulator.hpp"

#include "rvt/character_color.hpp"
#include "rvt/vt_emulator.hpp"
#include "rvt/utf8_decoder.hpp"
#include "rvt/json_rendering.hpp"

#include "utils/fdbuf.hpp"

#include <cerrno>
#include <cstdlib>

#include <unistd.h> // unlink
#include <stdio.h> // rename
#include <fcntl.h> // O_* flags


struct TerminalEmulator
{
    rvt::VtEmulator emulator;
    rvt::Utf8Decoder decoder;

    TerminalEmulator(int lines, int columns)
    : emulator(lines, columns)
    {}
};

extern "C" {

#define return_if(x) do { if (x) { return -1; } } while (0)
#define return_errno_if(x) do { if (x) { return errno ? errno : -1; } } while (0)

TerminalEmulator * terminal_emulator_init(int lines, int columns)
{
    return new TerminalEmulator(lines, columns);
}

void terminal_emulator_deinit(TerminalEmulator * emu)
{
    delete emu;
}

int terminal_emulator_feed(TerminalEmulator * emu, char const * s, int n)
{
    return_if(!emu);

    auto send_fn = [emu](rvt::ucs4_char ucs) { emu->emulator.receiveChar(ucs); };
    emu->decoder.decode(const_bytes_array(s, std::max(n, 0)), send_fn);
    return 0;
}

int terminal_emulator_finish(TerminalEmulator * emu)
{
    return_if(!emu);

    auto send_fn = [emu](rvt::ucs4_char ucs) { emu->emulator.receiveChar(ucs); };
    emu->decoder.end_decode(send_fn);
    return 0;
}

int terminal_emulator_resize(TerminalEmulator * emu, int lines, int columns)
{
    return_if(!emu);

    emu->emulator.setScreenSize(lines, columns);
    return 0;
}

int terminal_emulator_write(TerminalEmulator * emu, char const * filename)
{
    return_if(!emu);

    std::string const out = rvt::json_rendering(
        emu->emulator.getWindowTitle(),
        emu->emulator.getCurrentScreen(),
        rvt::color_table
    );

    char tmpfilename[4096];
    tmpfilename[0] = 0;
    int n = std::snprintf(tmpfilename, utils::size(tmpfilename) - 1, "%s-terermu-XXXXXX.tmp", filename);
    tmpfilename[n < 0 ? 0 : n] = 0;

    io::posix::fdbuf f;
    return_errno_if(f.open(tmpfilename, O_WRONLY | O_CREAT, 0444) < 0);

    std::streamsize sz = f.write_all(out.c_str(), out.size());
    return_errno_if(sz < 0);

    f.close();

    if (rename(tmpfilename, filename)) {
        unlink(tmpfilename);
        return errno;
    }

    return 0;
}

}
