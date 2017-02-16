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

#pragma once

#include "cxx/keyword.hpp"

class TerminalEmulator;

extern "C" {

REDEMPTION_LIB_EXPORT TerminalEmulator * terminal_emulator_init(int lines, int columns);
REDEMPTION_LIB_EXPORT void terminal_emulator_deinit(TerminalEmulator *);
REDEMPTION_LIB_EXPORT int terminal_emulator_finish(TerminalEmulator *);

REDEMPTION_LIB_EXPORT int terminal_emulator_set_title(TerminalEmulator *, char const * title);
REDEMPTION_LIB_EXPORT int terminal_emulator_feed(TerminalEmulator *, char const * s, int n);
REDEMPTION_LIB_EXPORT int terminal_emulator_resize(TerminalEmulator *, int lines, int columns);
REDEMPTION_LIB_EXPORT int terminal_emulator_write(TerminalEmulator *, char const * filename);

}
