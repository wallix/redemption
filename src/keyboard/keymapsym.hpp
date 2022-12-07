/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2012-2013
   Author(s): Christophe Grosjean, Dominique Lafages

   header file. KeymapSym object for keymap translation from RDP to X (VNC)
*/

#pragma once

#include "keyboard/key_mod_flags.hpp"
#include "keyboard/keymap.hpp"

#include <array>


struct KeymapSym
{
    enum class VncKeyState : uint8_t
    {
        Up = 0,
        Down = 1,
    };

    enum class IsApple : bool
    {
        No,
        Yes,
    };

    enum class IsUnix : bool
    {
        No,
        Yes,
    };

    struct Key
    {
        uint32_t keysym;
        VncKeyState down_flag;
    };

    struct Keys
    {
        Key const* begin() const noexcept { return keys.data(); }
        Key const* end() const noexcept { return keys.data() + len; }

        Key const* data() const noexcept { return keys.data(); }
        std::size_t size() const noexcept { return len; }

        void push(Key key) noexcept
        {
            keys[len] = key;
            ++len;
        }

        static constexpr std::size_t max_capacity = 12;

    private:
        friend KeymapSym;

        std::array<Key, max_capacity> keys;
        unsigned len = 0;
    };

    using KbdFlags = kbdtypes::KbdFlags;
    using Scancode = kbdtypes::Scancode;
    using KeyLocks = kbdtypes::KeyLocks;

    explicit KeymapSym(
        KeyLayout layout, kbdtypes::KeyLocks locks,
        IsApple is_apple, IsUnix is_unix,
        bool verbose) noexcept;

    Keys scancode_to_keysyms(KbdFlags flags, Scancode scancode) noexcept;

    Keys reset_mods(KeyLocks locks) noexcept;

    Keys utf16_to_keysyms(KbdFlags flag, uint16_t utf16) noexcept;

private:
    void _update_keymap() noexcept;

    KeyLayout layout_;

    sized_array_view<KeyLayout::unicode_t, 256> keymap_;
    KeyLayout::DKeyTable dkeys_ {};
    kbdtypes::KeyModFlags mods_ {};

    uint16_t previous_unicode16_ {};

    uint8_t imods_ {};

    bool is_win_;
    bool is_apple_;
    bool verbose_;
};
