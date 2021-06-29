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

#include "keyboard/keymap.hpp"
#include "utils/sugar/cast.hpp"
#include "cxx/cxx.hpp"


namespace
{
    unsigned numlock_01u(unsigned key_flags) noexcept
    {
        return (key_flags >> unsigned(Keymap::KeyMods::NumLock)) & 0x1u;
    }

    unsigned capslock_01u(unsigned key_flags) noexcept
    {
        return (key_flags >> unsigned(Keymap::KeyMods::CapsLock)) & 0x1u;
    }

    unsigned ctrl_01u(unsigned key_flags, unsigned rctrl_is_ctrl) noexcept
    {
        return ( (key_flags >> unsigned(Keymap::KeyMods::LCtrl))
               | ((key_flags >> unsigned(Keymap::KeyMods::RCtrl)) & rctrl_is_ctrl)
               ) & 0x1u;
    }

    unsigned oem8_01u(unsigned key_flags, unsigned rctrl_is_ctrl) noexcept
    {
        return ((key_flags >> unsigned(Keymap::KeyMods::RCtrl)) & ~rctrl_is_ctrl) & 0x1u;
    }

    unsigned alt_01u(unsigned key_flags) noexcept
    {
        return (key_flags >> unsigned(Keymap::KeyMods::Alt)) & 0x1u;
    }

    unsigned altgr_01u(unsigned key_flags) noexcept
    {
        return (key_flags >> unsigned(Keymap::KeyMods::AltGr)) & 0x1u;
    }

    unsigned shift_01u(unsigned key_flags) noexcept
    {
        return ( (key_flags >> unsigned(Keymap::KeyMods::LShift))
               | (key_flags >> unsigned(Keymap::KeyMods::RShift))
               ) & 0x1u;
    }

    struct KEventKeymaps
    {
        using KEvent = Keymap::KEvent;
        using KEventTable = KEvent[256];

        static constexpr KEvent special = KEvent(0xff);

        KEventTable mods[3] {};

        constexpr KEventKeymaps()
        {
            KEventTable& no_mod = mods[1];
            KEventTable& numpad_mod = mods[2];

            for (auto& k : no_mod) k = KEvent::KeyDown;
            for (auto& k : numpad_mod) k = KEvent::KeyDown;

            using KeyCode = Keymap::KeyCode;

            no_mod[int(KeyCode::Esc)] = KEvent::Esc;
            no_mod[int(KeyCode::F4)] = KEvent::F4;
            no_mod[int(KeyCode::Tab)] = special;
            no_mod[int(KeyCode::Enter)] = KEvent::Enter;
            no_mod[int(KeyCode::Delete)] = KEvent::Delete;
            no_mod[int(KeyCode::Backspace)] = KEvent::Backspace;
            no_mod[int(KeyCode::LeftArrow)] = KEvent::LeftArrow;
            no_mod[int(KeyCode::RightArrow)] = KEvent::RightArrow;
            no_mod[int(KeyCode::UpArrow)] = KEvent::UpArrow;
            no_mod[int(KeyCode::DownArrow)] = KEvent::DownArrow;
            no_mod[int(KeyCode::Home)] = KEvent::Home;
            no_mod[int(KeyCode::End)] = KEvent::End;
            no_mod[int(KeyCode::PgUp)] = KEvent::PgUp;
            no_mod[int(KeyCode::PgDown)] = KEvent::PgDown;
            no_mod[int(KeyCode::Insert)] = KEvent::Insert;
            no_mod[int(KeyCode::Numpad7)] = KEvent::Home;
            no_mod[int(KeyCode::Numpad8)] = KEvent::UpArrow;
            no_mod[int(KeyCode::Numpad9)] = KEvent::PgUp;
            no_mod[int(KeyCode::Numpad4)] = KEvent::LeftArrow;
            no_mod[int(KeyCode::Numpad6)] = KEvent::RightArrow;
            no_mod[int(KeyCode::Numpad1)] = KEvent::End;
            no_mod[int(KeyCode::Numpad2)] = KEvent::DownArrow;
            no_mod[int(KeyCode::Numpad3)] = KEvent::PgDown;
            no_mod[int(KeyCode::NumpadInsert)] = KEvent::Insert;
            no_mod[int(KeyCode::NumpadDelete)] = KEvent::Delete;
            no_mod[int(KeyCode::NumpadEnter)] = KEvent::Enter;
            no_mod[int(KeyCode::LCtrl)] = KEvent::None;
            no_mod[int(KeyCode::RCtrl)] = KEvent::None;
            no_mod[int(KeyCode::LShift)] = KEvent::None;
            no_mod[int(KeyCode::RShift)] = KEvent::None;
            no_mod[int(KeyCode::LAlt)] = KEvent::None;
            no_mod[int(KeyCode::RAlt)] = KEvent::None;
            no_mod[int(KeyCode::LWin)] = KEvent::None;
            no_mod[int(KeyCode::RWin)] = KEvent::None;
            no_mod[int(KeyCode::Apps)] = KEvent::None;
            no_mod[int(KeyCode::CapsLock)] = KEvent::None;
            no_mod[int(KeyCode::NumLock)] = KEvent::None;
            no_mod[int(KeyCode::ScrollLock)] = KEvent::None;
            no_mod[int(KeyCode::Key_X)] = special;
            no_mod[int(KeyCode::Key_C)] = special;
            no_mod[int(KeyCode::Key_V)] = special;

            numpad_mod[int(KeyCode::Esc)] = KEvent::Esc;
            numpad_mod[int(KeyCode::Tab)] = KEvent::Tab;
            numpad_mod[int(KeyCode::Enter)] = KEvent::Enter;
            numpad_mod[int(KeyCode::Delete)] = KEvent::Delete;
            numpad_mod[int(KeyCode::Backspace)] = KEvent::Backspace;
            numpad_mod[int(KeyCode::LeftArrow)] = KEvent::LeftArrow;
            numpad_mod[int(KeyCode::RightArrow)] = KEvent::RightArrow;
            numpad_mod[int(KeyCode::UpArrow)] = KEvent::UpArrow;
            numpad_mod[int(KeyCode::DownArrow)] = KEvent::DownArrow;
            numpad_mod[int(KeyCode::Home)] = KEvent::Home;
            numpad_mod[int(KeyCode::End)] = KEvent::End;
            numpad_mod[int(KeyCode::PgUp)] = KEvent::PgUp;
            numpad_mod[int(KeyCode::PgDown)] = KEvent::PgDown;
            numpad_mod[int(KeyCode::Insert)] = KEvent::Insert;
            numpad_mod[int(KeyCode::NumpadEnter)] = KEvent::Enter;
            numpad_mod[int(KeyCode::LCtrl)] = KEvent::None;
            numpad_mod[int(KeyCode::RCtrl)] = KEvent::None;
            numpad_mod[int(KeyCode::LShift)] = KEvent::None;
            numpad_mod[int(KeyCode::RShift)] = KEvent::None;
            numpad_mod[int(KeyCode::LAlt)] = KEvent::None;
            numpad_mod[int(KeyCode::RAlt)] = KEvent::None;
            numpad_mod[int(KeyCode::LWin)] = KEvent::None;
            numpad_mod[int(KeyCode::RWin)] = KEvent::None;
            numpad_mod[int(KeyCode::Apps)] = KEvent::None;
            numpad_mod[int(KeyCode::CapsLock)] = KEvent::None;
            numpad_mod[int(KeyCode::NumLock)] = KEvent::None;
            numpad_mod[int(KeyCode::ScrollLock)] = KEvent::None;
            numpad_mod[int(KeyCode::Key_X)] = special;
            numpad_mod[int(KeyCode::Key_C)] = special;
            numpad_mod[int(KeyCode::Key_V)] = special;
        }
    };

    constexpr KEventKeymaps kevent_keymaps {};
}


// KeyLayout Keymap::default_layout() noexcept
// {
//     return *KeyLayout::find_layout_by_id(KeyLayout::KbdId(0x40C));
// }

// Keymap::Keymap() noexcept
// : Keymap(default_layout())
// {}

Keymap::Keymap(KeyLayout layout) noexcept
: _keymap(layout.keymap_by_mod[0])
, _layout(layout)
{}

void Keymap::set_layout(KeyLayout new_layout) noexcept
{
    _layout = new_layout;
    _keymap = _layout.keymap_by_mod[_imods];
}

Keymap::DecodedKeys Keymap::event(uint16_t scancode_and_flags) noexcept
{
    return event(KbdFlags(scancode_and_flags & 0xff00u), Scancode(scancode_and_flags));
}

Keymap::DecodedKeys Keymap::event(KbdFlags flags, Scancode scancode) noexcept
{
    assert(uint8_t(scancode) <= 0x7fu);

    _decoded_key = {kbdtypes::to_keycode(flags, scancode), flags, {}};
    uint8_t keycode {underlying_cast(_decoded_key.keycode)};

    switch (keycode)
    {
        // Lock keys

        case uint8_t(KeyCode::CapsLock):
            if (!(underlying_cast(flags) & underlying_cast(KbdFlags::Release))) {
                _key_flags ^= 1u << unsigned(KeyMods::CapsLock);
            }
            break;
        case uint8_t(KeyCode::NumLock):
            if (!(underlying_cast(flags) & underlying_cast(KbdFlags::Release))) {
                _key_flags ^= 1u << unsigned(KeyMods::NumLock);
            }
            break;
        case uint8_t(KeyCode::ScrollLock):
            if (!(underlying_cast(flags) & underlying_cast(KbdFlags::Release))) {
                _key_flags ^= 1u << unsigned(KeyMods::ScrollLock);
            }
            return _decoded_key;

        // Modifier keys

        case uint8_t(KeyCode::LCtrl):
            if (!(underlying_cast(flags) & underlying_cast(KbdFlags::Extended1))) {
                _key_flags ^= 1u << unsigned(KeyMods::LCtrl);
            }
            break;
        case uint8_t(KeyCode::RCtrl):  _key_flags ^= 1u << unsigned(KeyMods::RCtrl); break;
        case uint8_t(KeyCode::LShift): _key_flags ^= 1u << unsigned(KeyMods::LShift); break;
        case uint8_t(KeyCode::RShift): _key_flags ^= 1u << unsigned(KeyMods::RShift); break;
        case uint8_t(KeyCode::LAlt):   _key_flags ^= 1u << unsigned(KeyMods::Alt); break;
        case uint8_t(KeyCode::RAlt):   _key_flags ^= 1u << unsigned(KeyMods::AltGr); break;

        default:
            if (!(underlying_cast(flags) & underlying_cast(KbdFlags::Release))) {
                auto unicode = _keymap[keycode];

                if (REDEMPTION_UNLIKELY(_dkeys)) {
                    if (auto unicode2 = _dkeys.find_composition(unicode)) {
                        _decoded_key.uchars[0] = unicode2;
                    }
                    // Windows(c) behavior for backspace following a Deadkey
                    else if (unicode && KeyCode(keycode) != KeyCode::Backspace) {
                        _decoded_key.uchars[0] = _dkeys.accent();
                        _decoded_key.uchars[1] = unicode_t(unicode & ~KeyLayout::DK);
                    }
                    _dkeys = {};
                }
                else if (REDEMPTION_UNLIKELY(unicode & KeyLayout::DK)) {
                    _dkeys = _layout.dkeymap_by_mod[_imods][keycode];
                }
                else {
                    _decoded_key.uchars[0] = unicode;
                }
            }
            return _decoded_key;
    }

    _update_keymap();

    return _decoded_key;
}

void Keymap::_update_keymap() noexcept
{
    auto rctrl_is_ctrl = unsigned(_layout.right_ctrl_is_ctrl);

    auto numlock = numlock_01u(_key_flags);
    auto capslock = capslock_01u(_key_flags);
    auto ctrl = ctrl_01u(_key_flags, rctrl_is_ctrl);
    auto altgr = altgr_01u(_key_flags) | ctrl;
    auto oem8 = oem8_01u(_key_flags, rctrl_is_ctrl);
    auto alt = alt_01u(_key_flags);
    auto shift = shift_01u(_key_flags);

    _imods = checked_int(0u
            | (shift << KeyLayout::Mods::Shift)
            | (ctrl << KeyLayout::Mods::Control)
            | (alt << KeyLayout::Mods::Menu)
            // enable Ctrl and Alt when AltGr
            | (altgr << KeyLayout::Mods::Control)
            | (altgr << KeyLayout::Mods::Menu)
            | (oem8 << KeyLayout::Mods::OEM_8)
            | (numlock << KeyLayout::Mods::NumLock)
            | (capslock << KeyLayout::Mods::CapsLock)
            );
    _keymap = _layout.keymap_by_mod[_imods];
}

Keymap::KEvent Keymap::last_kevent() const noexcept
{
    auto shift = shift_01u(_key_flags);
    auto numlock = numlock_01u(_key_flags);

    auto down = ~(underlying_cast(_decoded_key.flags) >> 15) & 0x1u;

    // numpad + shift = no_mod
    auto kevent = kevent_keymaps.mods[down + (numlock & ~shift & down)][underlying_cast(_decoded_key.keycode)];
    if (REDEMPTION_UNLIKELY(kevent == KEventKeymaps::special)) {
        switch (underlying_cast(_decoded_key.keycode)) {
            case underlying_cast(KeyCode::Key_X):
            case underlying_cast(KeyCode::Key_C):
            case underlying_cast(KeyCode::Key_V):
                return is_ctrl_pressed()
                    ? KEvent(int(_decoded_key.keycode) - int(KeyCode::Key_X) + int(KEvent::Cut))
                    : KEvent::KeyDown;

            case underlying_cast(KeyCode::Tab):
                return KEvent(unsigned(KEvent::Tab) + shift_01u(_key_flags));
        }
    }
    return kevent;
}

bool Keymap::is_tsk_switch_shortcut() const noexcept
{
    auto rctrl_is_ctrl = unsigned(_layout.right_ctrl_is_ctrl);
    auto ctrl = ctrl_01u(_key_flags, rctrl_is_ctrl);
    // ctrl+alt/altgr+del or ctrl+shift+esc
    return ((ctrl & shift_01u(_key_flags)) && _decoded_key.keycode == KeyCode::Esc)
        || ((ctrl & (altgr_01u(_key_flags) | alt_01u(_key_flags)))
            && (_decoded_key.keycode == KeyCode::Delete
             || _decoded_key.keycode == KeyCode::NumpadDelete));
}

bool Keymap::is_app_switching_shortcut() const noexcept
{
    if (_decoded_key.keycode != KeyCode::Tab) {
        return false;
    }

    auto rctrl_is_ctrl = unsigned(_layout.right_ctrl_is_ctrl);
    auto ctrl = ctrl_01u(_key_flags, rctrl_is_ctrl);
    auto alt = alt_01u(_key_flags);
    return bool(ctrl ^ alt);
}

bool Keymap::is_alt_pressed() const noexcept
{
    return bool(alt_01u(_key_flags));
}

bool Keymap::is_ctrl_pressed() const noexcept
{
    auto rctrl_is_ctrl = unsigned(_layout.right_ctrl_is_ctrl);
    return bool(ctrl_01u(_key_flags, rctrl_is_ctrl));
}

bool Keymap::is_shift_pressed() const noexcept
{
    return bool(shift_01u(_key_flags));
}

void Keymap::reset_mods(KeyLocks locks) noexcept
{
    _key_flags = 0;
    set_locks(locks);
}

void Keymap::set_locks(KeyLocks locks) noexcept
{
    using U = unsigned;
    U mask = (1u << U(KeyMods::NumLock))
           | (1u << U(KeyMods::CapsLock))
           // | (1u << U(KeyMods::KanaLock))
           | (1u << U(KeyMods::ScrollLock))
           ;
    _key_flags &= ~mask;
    _key_flags |= bool(locks & KeyLocks::NumLock) ? (1u << U(KeyMods::NumLock)) : U();
    _key_flags |= bool(locks & KeyLocks::CapsLock) ? (1u << U(KeyMods::CapsLock)) : U();
    // _key_flags |= bool(locks & KeyLocks::KanaLock) ? (1u << U(KeyMods::KanaLock)) : U();
    _key_flags |= bool(locks & KeyLocks::ScrollLock) ? (1u << U(KeyMods::ScrollLock)) : U();

    _update_keymap();
}

kbdtypes::KeyLocks Keymap::locks() const noexcept
{
    using U = unsigned;
    U flags = 0;
    flags |= (_key_flags & (1u << U(KeyMods::NumLock))) ? U(KeyLocks::NumLock) : 0;
    flags |= (_key_flags & (1u << U(KeyMods::CapsLock))) ? U(KeyLocks::CapsLock) : 0;
    // flags |= (_key_flags & (1u << U(KeyMods::KanaLock))) ? U(KeyLocks::KanaLock) : 0;
    flags |= (_key_flags & (1u << U(KeyMods::ScrollLock))) ? U(KeyLocks::ScrollLock) : 0;
    return kbdtypes::KeyLocks(flags);
}
