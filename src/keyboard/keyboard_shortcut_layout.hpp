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

#include "utils/sugar/array_view.hpp"
#include "keyboard/keylayout.hpp"
#include "keyboard/kbdtypes.hpp"


struct KeyboardShortcutLayout
{
    enum class KSMods : uint8_t
    {
        NoMods,
        Shift = 1,
        NumLock = 2,
    };

    struct ScancodeAndMods
    {
        uint8_t compressed_keycode;
        KSMods mods;

        kbdtypes::KeyCode keycode() const noexcept
        {
            return kbdtypes::KeyCode(((compressed_keycode & 0x80) << 1) | (compressed_keycode & 0x7f));
        }
    };

    static KeyboardShortcutLayout find_layout_by_id(KeyLayout::KbdId kid) noexcept;

    void log();

    explicit operator bool() const noexcept
    {
        return !cp_and_iscmods_indexes.empty();
    }

    array_view<ScancodeAndMods> find_unicode(uint16_t unicode) const noexcept;

private:
    array_view<uint16_t> cp_and_iscmods_indexes {};
};
