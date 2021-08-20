/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2021
    Author(s): Christophe Grosjean, Jonathan Poelen,
               Meng Tan, Raphael Zhou
*/

#pragma once

#include <vector>
#include <cstdint>

#include "keyboard/kbdtypes.hpp"
#include "utils/sugar/array_view.hpp"


class KeyboardShortcutBlocker
{
public:
    using Scancode = kbdtypes::Scancode;
    using KbdFlags = kbdtypes::KbdFlags;

    KeyboardShortcutBlocker(uint32_t keyboardLayout, chars_view configuration_string, bool verbose);
    ~KeyboardShortcutBlocker();

    bool scancode_must_be_blocked(KbdFlags keyboardFlags, Scancode scancode);

private:
    void add_shortcut(uint32_t keyboardLayout, chars_view shortcut);

    class Shortcut;

    std::vector<Shortcut> shortcuts;

    kbdtypes::KeyModFlags mods {};
    uint8_t virtual_mods {};

    const bool verbose;
};
