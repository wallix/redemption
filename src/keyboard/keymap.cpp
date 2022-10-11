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
    using KeyMod = kbdtypes::KeyMod;

    unsigned numlock_01u(kbdtypes::KeyModFlags mods) noexcept
    {
        return mods.test_as_uint(KeyMod::NumLock);
    }

    unsigned capslock_01u(kbdtypes::KeyModFlags mods) noexcept
    {
        return mods.test_as_uint(KeyMod::CapsLock);
    }

    unsigned ctrl_01u(kbdtypes::KeyModFlags mods, unsigned rctrl_is_ctrl) noexcept
    {
        return mods.test_as_uint(KeyMod::LCtrl)
             | (mods.test_as_uint(KeyMod::RCtrl) & rctrl_is_ctrl);
    }

    unsigned oem8_01u(kbdtypes::KeyModFlags mods, unsigned rctrl_is_ctrl) noexcept
    {
        return mods.test_as_uint(KeyMod::RCtrl) & ~rctrl_is_ctrl;
    }

    unsigned alt_01u(kbdtypes::KeyModFlags mods) noexcept
    {
        return mods.test_as_uint(KeyMod::LAlt);
    }

    unsigned altgr_01u(kbdtypes::KeyModFlags mods) noexcept
    {
        return mods.test_as_uint(KeyMod::RAlt);
    }

    unsigned shift_01u(kbdtypes::KeyModFlags mods) noexcept
    {
        return mods.test_as_uint(KeyMod::LShift)
             | mods.test_as_uint(KeyMod::RShift);
    }

    inline bool sharing_shortcut(kbdtypes::KeyCode shortcut,
                                 Keymap::DecodedKeys const& decoded_key,
                                 kbdtypes::KeyModFlags mods) noexcept
    {
        return decoded_key.keycode == shortcut
            && !bool(decoded_key.flags & kbdtypes::KbdFlags::Release)
            && (mods.test_as_uint(KeyMod::LCtrl)
              | mods.test_as_uint(KeyMod::LShift)
              | mods.test_as_uint(KeyMod::LAlt)
            );
    }

    struct KEventKeymaps
    {
        using KEvent = Keymap::KEvent;
        using KEventTable = KEvent[256 * 2];

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
            no_mod[int(KeyCode::Numpad0)] = KEvent::Insert;
            no_mod[int(KeyCode::NumpadDecimal)] = KEvent::Delete;
            no_mod[int(KeyCode::NumpadEnter)] = KEvent::Enter;
            no_mod[int(KeyCode::LCtrl)] = KEvent::Ctrl;
            no_mod[int(KeyCode::RCtrl)] = KEvent::Ctrl;
            no_mod[int(KeyCode::LShift)] = KEvent::Shift;
            no_mod[int(KeyCode::RShift)] = KEvent::Shift;
            no_mod[int(KeyCode::LAlt)] = KEvent::None;
            no_mod[int(KeyCode::RAlt)] = KEvent::None;
            no_mod[int(KeyCode::LWin)] = KEvent::None;
            no_mod[int(KeyCode::RWin)] = KEvent::None;
            no_mod[int(KeyCode::ContextMenu)] = KEvent::None;
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
            numpad_mod[int(KeyCode::LCtrl)] = KEvent::Ctrl;
            numpad_mod[int(KeyCode::RCtrl)] = KEvent::Ctrl;
            numpad_mod[int(KeyCode::LShift)] = KEvent::Shift;
            numpad_mod[int(KeyCode::RShift)] = KEvent::Shift;
            numpad_mod[int(KeyCode::LAlt)] = KEvent::None;
            numpad_mod[int(KeyCode::RAlt)] = KEvent::None;
            numpad_mod[int(KeyCode::LWin)] = KEvent::None;
            numpad_mod[int(KeyCode::RWin)] = KEvent::None;
            numpad_mod[int(KeyCode::ContextMenu)] = KEvent::None;
            numpad_mod[int(KeyCode::CapsLock)] = KEvent::None;
            numpad_mod[int(KeyCode::NumLock)] = KEvent::None;
            numpad_mod[int(KeyCode::ScrollLock)] = KEvent::None;
            numpad_mod[int(KeyCode::Key_X)] = special;
            numpad_mod[int(KeyCode::Key_C)] = special;
            numpad_mod[int(KeyCode::Key_V)] = special;
        }
    };

    constexpr KEventKeymaps kevent_keymaps {};
} // anonymous namespace


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
    _decoded_key = {kbdtypes::to_keycode(flags, scancode), flags, {}};

    auto set_mod = [&](KeyMod mod){
        _key_mods.update(flags, mod);
        _update_keymap();
    };

    auto set_locks_mod = [&](KeyMod mod){
        if (!bool(flags & KbdFlags::Release)) {
            _key_mods.flip(mod);
            _update_keymap();
        }
    };

    switch (underlying_cast(_decoded_key.keycode))
    {
        // Lock keys
        case underlying_cast(KeyCode::CapsLock):   set_locks_mod(KeyMod::CapsLock); break;
        case underlying_cast(KeyCode::NumLock):    set_locks_mod(KeyMod::NumLock); break;
        case underlying_cast(KeyCode::ScrollLock): set_locks_mod(KeyMod::ScrollLock); break;

        // Modifier keys
        case underlying_cast(KeyCode::LCtrl):  set_mod(KeyMod::LCtrl); break;
        case underlying_cast(KeyCode::RCtrl):  set_mod(KeyMod::RCtrl); break;
        case underlying_cast(KeyCode::LShift): set_mod(KeyMod::LShift); break;
        case underlying_cast(KeyCode::RShift): set_mod(KeyMod::RShift); break;
        case underlying_cast(KeyCode::LAlt):   set_mod(KeyMod::LAlt); break;
        case underlying_cast(KeyCode::RAlt):   set_mod(KeyMod::RAlt); break;

        default:
            if (!(underlying_cast(flags) & underlying_cast(KbdFlags::Release))
             && keycode_is_compressable_to_byte(_decoded_key.keycode)
            ) {
                const std::size_t keymap_index = keycode_to_byte_index(_decoded_key.keycode);
                auto unicode = _keymap[keymap_index];

                if (REDEMPTION_UNLIKELY(_dkeys)) {
                    if (auto unicode2 = _dkeys.find_composition(unicode)) {
                        _decoded_key.uchars[0] = unicode2;
                    }
                    // Windows(c) behavior for backspace following a Deadkey
                    else if (unicode && _decoded_key.keycode != KeyCode::Backspace) {
                        _decoded_key.uchars[0] = _dkeys.accent();
                        _decoded_key.uchars[1] = unicode_t(unicode & ~KeyLayout::DK);
                    }
                    _dkeys = {};
                }
                else if (REDEMPTION_UNLIKELY(unicode & KeyLayout::DK)) {
                    _dkeys = _layout.dkeymap_by_mod[_imods][keymap_index];
                }
                else {
                    _decoded_key.uchars[0] = unicode;
                }
            }
    }

    return _decoded_key;
}

void Keymap::_update_keymap() noexcept
{
    auto rctrl_is_ctrl = unsigned(_layout.right_ctrl_is_ctrl);

    auto numlock = numlock_01u(_key_mods);
    auto capslock = capslock_01u(_key_mods);
    auto ctrl = ctrl_01u(_key_mods, rctrl_is_ctrl);
    auto altgr = altgr_01u(_key_mods);
    auto oem8 = oem8_01u(_key_mods, rctrl_is_ctrl);
    auto alt = alt_01u(_key_mods);
    auto shift = shift_01u(_key_mods);

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
    auto shift = shift_01u(_key_mods);
    auto numlock = numlock_01u(_key_mods);

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
                return KEvent(unsigned(KEvent::Tab) + shift_01u(_key_mods));
        }
    }
    return kevent;
}

bool Keymap::is_tsk_switch_shortcut() const noexcept
{
    auto rctrl_is_ctrl = unsigned(_layout.right_ctrl_is_ctrl);
    auto ctrl = ctrl_01u(_key_mods, rctrl_is_ctrl);

    if (!ctrl) {
        return false;
    }

    // ctrl+alt+del or ctrl+shift+esc
    return (alt_01u(_key_mods) && (_decoded_key.keycode == KeyCode::Delete
                                 || _decoded_key.keycode == KeyCode::NumpadDecimal))
        || (shift_01u(_key_mods) && _decoded_key.keycode == KeyCode::Esc);
}

bool Keymap::is_app_switching_shortcut() const noexcept
{
    if (_decoded_key.keycode != KeyCode::Tab) {
        return false;
    }

    auto is_down = !((underlying_cast(_decoded_key.flags) >> 15) & 0x1u);
    if (!is_down) {
        return false;
    }

    // alt+tab, ctrl+tab, ctrl+alt+tab
    auto rctrl_is_ctrl = unsigned(_layout.right_ctrl_is_ctrl);
    return alt_01u(_key_mods) || ctrl_01u(_key_mods, rctrl_is_ctrl);
}

bool Keymap::is_session_sharing_take_control() const noexcept
{
    return sharing_shortcut(KeyCode::F9, _decoded_key, _key_mods);
}

bool Keymap::is_session_sharing_give_control() const noexcept
{
    return sharing_shortcut(KeyCode::F10, _decoded_key, _key_mods);
}

bool Keymap::is_session_sharing_common_control() const noexcept
{
    return sharing_shortcut(KeyCode::F11, _decoded_key, _key_mods);
}

bool Keymap::is_session_sharing_kill_guest() const noexcept
{
    return sharing_shortcut(KeyCode::F5, _decoded_key, _key_mods);
}

bool Keymap::is_session_sharing_toggle_graphics() const noexcept
{
    return sharing_shortcut(KeyCode::F8, _decoded_key, _key_mods);
}

bool Keymap::is_alt_pressed() const noexcept
{
    return bool(alt_01u(_key_mods));
}

bool Keymap::is_ctrl_pressed() const noexcept
{
    auto rctrl_is_ctrl = unsigned(_layout.right_ctrl_is_ctrl);
    return bool(ctrl_01u(_key_mods, rctrl_is_ctrl));
}

bool Keymap::is_shift_pressed() const noexcept
{
    return bool(shift_01u(_key_mods));
}

void Keymap::reset_mods(KeyLocks locks) noexcept
{
    _key_mods.reset();
    set_locks(locks);
}

void Keymap::set_locks(KeyLocks locks) noexcept
{
    _key_mods.sync_locks(locks);
    _update_keymap();
}

kbdtypes::KeyLocks Keymap::locks() const noexcept
{
    using U = unsigned;
    U flags = 0;
    flags |= _key_mods.test(KeyMod::NumLock) ? U(KeyLocks::NumLock) : 0u;
    flags |= _key_mods.test(KeyMod::CapsLock) ? U(KeyLocks::CapsLock) : 0u;
    // flags |= _key_mods.test(KeyMod::KanaLock) ? U(KeyLocks::KanaLock) : 0u;
    flags |= _key_mods.test(KeyMod::ScrollLock) ? U(KeyLocks::ScrollLock) : 0u;
    return kbdtypes::KeyLocks(flags);
}

Keymap::KeyModFlags Keymap::mods() const noexcept
{
    return _key_mods;
}
