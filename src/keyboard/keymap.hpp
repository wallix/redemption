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

#include "keyboard/keylayout2.hpp"
#include "utils/sugar/cast.hpp"
#include "utils/sugar/flags.hpp"
#include "cxx/cxx.hpp"

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
        max_
    };
}

template<>
struct utils::enum_as_flag<detail::KeyModsIndex>
{
    static constexpr std::size_t max = std::size_t(detail::KeyModsIndex::max_);
};

struct Keymap
{
// The TS_KEYBOARD_EVENT structure is a standard T.128 Keyboard Event (see [T128] section
// 8.18.2). RDP keyboard input is restricted to keyboard scancodes, unlike the code-point or virtual
// codes supported in T.128 (a scancode is an 8-bit value specifying a key location on the keyboard).
// The server accepts a scancode value and translates it into the correct character depending on the
// language locale and keyboard layout used in the session.

// keyboardFlags (2 bytes): A 16-bit, unsigned integer. The flags describing the keyboard event.

// +--------------------------+------------------------------------------------+
// | 0x0100 KBDFLAGS_EXTENDED | The keystroke message contains an extended     |
// |                          | scancode. For enhanced 101-key and 102-key     |
// |                          | keyboards, extended keys include "he right     |
// |                          | ALT and right CTRL keys on the main section    |
// |                          | of the keyboard; the INS, DEL, HOME, END,      |
// |                          | PAGE UP, PAGE DOWN and ARROW keys in the       |
// |                          | clusters to the left of the numeric keypad;    |
// |                          | and the Divide ("/") and ENTER keys in the     |
// |                          | numeric keypad.                                |
// +--------------------------+------------------------------------------------+
// | 0x4000 KBDFLAGS_DOWN     | Indicates that the key was down prior to this  |
// |                          | event.                                         |
// +--------------------------+------------------------------------------------+
// | 0x8000 KBDFLAGS_RELEASE  | The absence of this flag indicates a key-down  |
// |                          | event, while its presence indicates a          |
// |                          | key-release event.                             |
// +--------------------------+------------------------------------------------+
    enum class KbdFlags : uint16_t
    {
        Extended = 0x0100,
        // Down     = 0x4000,
        Release  = 0x8000,
    };

    enum class KeyCode : uint8_t
    {
        Esc = 0x01,
        F11 = 0x57,
        F12 = 0x58,
        // LCtrl = 0x1D,
        // RCtrl = 0x1D | 0x80,
        // LShift = 0x2A,
        // RShift = 0x36,
        // LAlt = 0x38,
        // RAlt = 0x38 | 0x80,
        // LWin = 0x5B | 0x80,
        // RWin = 0x5C | 0x80,
        // Apps = 0x5D | 0x80,
        // CapsLock = 0x3A,
        // NumLock = 0x45,
        // ScrollLock = 0x46,
        UpArrow = 0x48 | 0x80,
        LeftArrow = 0x4B | 0x80,
        RightArrow = 0x4D | 0x80,
        DownArrow = 0x50 | 0x80,
        Home = 0x47 | 0x80,
        End = 0x4F | 0x80,
        PgUp = 0x49 | 0x80,
        PgDown = 0x51 | 0x80,
        Insert = 0x52 | 0x80,
        Delete = 0x53 | 0x80,
        Enter = 0x1C,
        Tab = 0x0F,
        Backspace = 0x0E,
        NumpadInsert = 0x52,
        NumpadDelete = 0x53,
        NumpadEnter = 0x1C | 0x80,

        Key_X = 0x2D,
        Key_C = 0x2E,
        Key_V = 0x2F,
    };

    enum class Scancode : uint8_t;

    KeyLayout2 layout;
    uint8_t imods {};
    KeyLayout2::DKeyTable dkeys {};

    using unicode_t = KeyLayout2::unicode_t;

    struct DecodedKey
    {
        KeyCode keycode;
        // 2 unicode chars when bad dead key
        std::array<unicode_t, 2> uchars;
    };

    DecodedKey _decoded_key {};

    DecodedKey decoded_key() const
    {
        return _decoded_key;
    }

//     struct KeyStates
//     {
//         // key states 0 up 1 down (0..127 plain keys, 128..255 extended keys)
//         uint64_t keys_down[4];
//
//         void set(uint8_t pos, uint64_t x) noexcept
//         {
//             assert(x == 1 || x == 0);
//             this->keys_down[pos / 64] = (this->keys_down[pos / 64] & (uint64_t(1) << (pos % 64)))
//                                       | (x << (pos % 64));
//         }
//
//         uint64_t get(uint8_t pos) const noexcept
//         {
//             return (this->keys_down[pos / 64] >> (pos % 64)) & 0x1;
//         }
//
//         uint64_t get();
//     };
//
//     KeyStates keys_down;

    static array_view<KeyLayout2> keylayouts() noexcept;

    static KeyLayout2 default_layout() noexcept
    {
        return *KeyLayout2::find_layout_by_id(KeyLayout2::KbdId(0x40C));
    }

    explicit Keymap(bool verbose = false) noexcept
    : layout(default_layout())
    , verbose(verbose)
    {}

    void set_layout(KeyLayout2 new_layout) noexcept
    {
        this->layout = new_layout;
    }

    using KeyMods = detail::KeyModsIndex;
    using KeyModFlags = utils::flags_t<KeyMods>;

    unsigned _key_flags = 0;

    void event(uint16_t scancode_and_flags) noexcept
    {
        event(KbdFlags(scancode_and_flags & 0xff00u), Scancode(scancode_and_flags));
    }

    void event(KbdFlags flags, Scancode scancode) noexcept
    {
        assert(uint8_t(scancode) <= 0x7fu);

        // The scancode and its extended nature are merged in a new variable (whose most significant bit indicates the extended nature)
        uint8_t keycode = uint8_t(scancode) | ((uint16_t(flags) & 0x100u) >> 1);
//         uint64_t down = ~((uint64_t(flags) >> 15) & 0x1u);
//         this->keys_down.set(keycode, down);

        _decoded_key.keycode = KeyCode(keycode);
        _decoded_key.uchars = {};

        switch (keycode)
        {
            // Lock keys

            case uint8_t(KeyCode::CapsLock):
                if (underlying_cast(flags) & underlying_cast(KbdFlags::Release)) {
                    this->_key_flags ^= 1u << unsigned(KeyMods::CapsLock);
                }
                break;
            case uint8_t(KeyCode::NumLock):
                if (underlying_cast(flags) & underlying_cast(KbdFlags::Release)) {
                    this->_key_flags ^= 1u << unsigned(KeyMods::NumLock);
                }
                break;
            // case uint8_t(KeyCode::ScrollLock):
            //     if (underlying_cast(flags) & underlying_cast(KbdFlags::Release)){
            //         this->key_flags ^= KeyMods::ScrollLock;
            //     }
            //     break;

            // Modifier keys

            case uint8_t(KeyCode::LCtrl): this->_key_flags ^= 1u << unsigned(KeyMods::LCtrl); break;
            case uint8_t(KeyCode::RCtrl): this->_key_flags ^= 1u << unsigned(KeyMods::RCtrl); break;
            case uint8_t(KeyCode::LShift): this->_key_flags ^= 1u << unsigned(KeyMods::LShift); break;
            case uint8_t(KeyCode::RShift): this->_key_flags ^= 1u << unsigned(KeyMods::RShift); break;
            case uint8_t(KeyCode::LAlt): this->_key_flags ^= 1u << unsigned(KeyMods::Alt); break;
            case uint8_t(KeyCode::RAlt): this->_key_flags ^= 1u << unsigned(KeyMods::AltGr); break;

            default: {
                auto i = keycode & 0x7fu;
                if (!(keycode & 0x80u)) {
                    auto unicode = this->layout.keymap_by_mod[this->imods][i];

                    if (REDEMPTION_UNLIKELY(this->dkeys)) {
                        if (auto unicode2 = this->dkeys.find(unicode)) {
                            _decoded_key.uchars[0] = unicode2;
                        }
                        else {
                            _decoded_key.uchars[0] = this->dkeys.accent();
                            _decoded_key.uchars[1] = unicode_t(unicode & ~KeyLayout2::DK);
                        }
                        this->dkeys = {};
                    }
                    else if (REDEMPTION_UNLIKELY(unicode & KeyLayout2::DK)) {
                        this->dkeys = this->layout.dkeymap_by_mod[this->imods][i];
                    }
                    else {
                        _decoded_key.uchars[0] = unicode;
                    }
                }
                else {
                    _decoded_key.uchars[0] = this->layout.extended_keymap_by_mod[this->imods][i];
                }

                return ;
            }
        }

        auto rctrl_is_ctrl = unsigned(this->layout.right_ctrl_is_ctrl);

        auto numlock = (_key_flags >> unsigned(KeyMods::NumLock)) & 0x1;
        auto capslock = (_key_flags >> unsigned(KeyMods::CapsLock)) & 0x1;
        auto ctrl = ( (_key_flags >> unsigned(KeyMods::LCtrl))
                    | ((_key_flags >> unsigned(KeyMods::RCtrl)) & rctrl_is_ctrl)
                    ) & 0x1;
        auto oem8 = ((_key_flags >> unsigned(KeyMods::RCtrl)) & ~rctrl_is_ctrl) & 0x1;
        auto alt = (_key_flags >> unsigned(KeyMods::Alt)) & 0x1;
        auto altgr = ( (_key_flags >> unsigned(KeyMods::AltGr))
                     | (alt & ctrl)
                     ) & 0x1;
        auto shift = ( (_key_flags >> unsigned(KeyMods::LShift))
                     | (_key_flags >> unsigned(KeyMods::RShift))
                     ) & 0x1;

        imods = checked_int(0u
              | (shift << KeyLayout2::Mods::Shift)
              | (ctrl << KeyLayout2::Mods::Control)
              | (alt << KeyLayout2::Mods::Menu)
              // enable Ctrl and Alt when AltGr
              | (altgr << KeyLayout2::Mods::Control)
              | (altgr << KeyLayout2::Mods::Menu)
              | (oem8 << KeyLayout2::Mods::OEM_8)
              | (numlock << KeyLayout2::Mods::NumLock)
              | (capslock << KeyLayout2::Mods::CapsLock)
              );
    }

    KeyModFlags mods() const noexcept
    {
        return KeyModFlags(_key_flags).normalized();
    }

    // TODO lctrl+alt+sup (canadien) = ?
    bool is_ctrl_pressed() const noexcept
    {
        auto rctrl_is_ctrl = unsigned(this->layout.right_ctrl_is_ctrl);
        auto ctrl = ( (this->_key_flags >> unsigned(KeyMods::LCtrl))
                    | ((this->_key_flags >> unsigned(KeyMods::RCtrl)) & rctrl_is_ctrl)
                    ) & 0x1;
        return bool(ctrl);
    }

    void synchronize(uint16_t param1);

    void toggle_caps_lock(bool on);
    void toggle_num_lock(bool on);
    void toggle_scroll_lock(bool on);

    [[nodiscard]] int layout_id() const;
}; // END STRUCT - Keymap2
