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
   Copyright (C) Wallix 2010-2013
   Author(s): Christophe Grosjean, Dominique Lafages, Raphael Zhou,
              Meng Tan

   header file. Keymap2 object, used to manage key stroke events
*/

#pragma once

#include "keyboard/keylayout.hpp"
#include "keyboard/kbdtypes.hpp"
#include "utils/sugar/flags.hpp"


namespace detail
{
    enum class  KeyModsIndex : unsigned
    {
        LCtrl,
        RCtrl,
        LShift,
        RShift,
        // LWin,
        // RWin,
        Alt,
        AltGr,
        NumLock,
        CapsLock,
        ScrollLock,
        max_
    };
} // namespace detail

template<>
struct utils::enum_as_flag<detail::KeyModsIndex>
{
    static constexpr std::size_t max = std::size_t(detail::KeyModsIndex::max_);
};

struct Keymap
{
    using KbdFlags = kbdtypes::KbdFlags;
    using Scancode = kbdtypes::Scancode;
    using KeyCode = kbdtypes::KeyCode;
    using KeyLocks = kbdtypes::KeyLocks;

    enum class KEvent : uint8_t
    {
        None,
        KeyDown,
        F4,
        Tab,
        BackTab,
        Enter,
        Esc,
        Delete,
        Backspace,
        LeftArrow,
        RightArrow,
        UpArrow,
        DownArrow,
        Home,
        End,
        PgUp,
        PgDown,
        Insert,
        Cut,
        Copy,
        Paste,
    };

    using KeyMods = detail::KeyModsIndex;
    using KeyModFlags = utils::flags_t<KeyMods>;

    using unicode_t = KeyLayout::unicode_t;

    struct DecodedKeys
    {
        KeyCode keycode;
        KbdFlags flags;
        // 2 unicode chars when bad dead key
        std::array<unicode_t, 2> uchars;

        bool has_char() const noexcept
        {
            return uchars[0];
        }
    };


    explicit Keymap(KeyLayout layout) noexcept;

    DecodedKeys event(uint16_t scancode_and_flags) noexcept;
    DecodedKeys event(KbdFlags flags, Scancode scancode) noexcept;

    void reset_decoded_keys() noexcept
    {
        _decoded_key = {};
    }

    DecodedKeys last_decoded_keys() const noexcept
    {
        return _decoded_key;
    }

    KEvent last_kevent() const noexcept;

    bool is_tsk_switch_shortcut() const noexcept;
    bool is_app_switching_shortcut() const noexcept;

    bool is_alt_pressed() const noexcept;
    bool is_ctrl_pressed() const noexcept;
    bool is_shift_pressed() const noexcept;

    void reset_mods(KeyLocks locks) noexcept;

    void set_locks(KeyLocks locks) noexcept;

    void set_layout(KeyLayout new_layout) noexcept;

    KeyLayout const& layout() const noexcept
    {
        return _layout;
    }

    KeyLocks locks() const noexcept;

    KeyModFlags mods() const noexcept
    {
        return KeyModFlags(_key_flags);
    }

private:
    void _update_keymap() noexcept;

    DecodedKeys _decoded_key {};
    unsigned _key_flags = 0;

    sized_array_view<unicode_t, 256> _keymap;
    uint8_t _imods {};
    KeyLayout::DKeyTable _dkeys {};

    KeyLayout _layout;
}; // END STRUCT - Keymap2
