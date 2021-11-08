/*
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

Product name: redemption, a FLOSS RDP proxy
Copyright (C) Wallix 2021
Author(s): Proxies Team
*/

#pragma once

#include "keyboard/kbdtypes.hpp"
#include "keyboard/key_mod_flags.hpp"
#include "keyboard/keylayout.hpp"

#include <memory>
#include <cstdint>

class KeyboardShortcutBlocker
{
public:
    using Scancode = kbdtypes::Scancode;
    using KbdFlags = kbdtypes::KbdFlags;
    using KeyLocks = kbdtypes::KeyLocks;

    KeyboardShortcutBlocker(bool verbose) noexcept;
    KeyboardShortcutBlocker(KeyLayout::KbdId kid, chars_view configuration_string, bool verbose);

    ~KeyboardShortcutBlocker();

    /**
     * configuration_string format:
     * mod = "shift" | "meta" | "ctrl" | "alt" | "altgr" | "numlocks"
     * scancode = "numpad8" | "keya" | "bracketright" | .... (note: js keyname)
     * keyname = scancode | letter
     * shortcut = ( mod "+" )* keyname
     * shortcuts = shortcut ( "," shortcut )*
     */
    void set_shortcuts(KeyLayout::KbdId kid, chars_view configuration_string);

    bool has_shortcut() const noexcept;

    bool scancode_must_be_blocked(KbdFlags keyboardFlags, Scancode scancode) noexcept;
    void sync_locks(KeyLocks key_locks) noexcept;

private:
    struct Shortcut;

    std::unique_ptr<Shortcut> shortcuts;

    kbdtypes::KeyModFlags mods {};
    uint8_t virtual_mods {};

    uint8_t min_code = 0;
    uint8_t max_code = 0;

    bool verbose;
};
