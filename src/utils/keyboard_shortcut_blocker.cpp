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

#include "utils/keyboard_shortcut_blocker.hpp"
#include "keyboard/scancode/unicode_to_scancode.hpp"
#include "keyboard/kbdtypes.hpp"
#include "utils/log.hpp"
#include "utils/utf.hpp"
#include "utils/strutils.hpp"
#include "utils/sugar/split.hpp"
#include "utils/sugar/array_view.hpp"
#include "utils/sugar/flags.hpp"
#include "utils/ascii.hpp"


namespace
{
    enum class Mods
    {
        Ctrl,
        Shift,
        Alt,
        AltGr,
        Meta,
        max_,
    };
}

template<>
struct utils::enum_as_flag<Mods>
{
    static constexpr std::size_t max = std::size_t(Mods::max_);
};

namespace
{
    using ModFlags = utils::flags_t<Mods>;
    struct ModPair
    {
        ModFlags mod;
        std::string_view str;
    };

    constexpr ModPair mod_infos[]{
        {Mods::Ctrl, "ctrl"},
        {Mods::Shift, "shift"},
        {Mods::Alt, "alt"},
        {Mods::AltGr, "altgr"},
        {Mods::Meta, "meta"},
        {Mods::Meta, "windows"},
    };

    constexpr ModFlags find_mod(std::string_view mod_name)
    {
        for (ModPair mp : mod_infos) {
            if (mp.str == mod_name) {
                return mp.mod;
            }
        }
        return ModFlags();
    }

    struct KeyNameAndKeyCode
    {
        std::string_view name;
        kbdtypes::KeyCode code;
    };

    constexpr KeyNameAndKeyCode key_name_and_codes[] {
        {"backspace", kbdtypes::KeyCode::Backspace},
        {"del", kbdtypes::KeyCode::Delete},
        {"end", kbdtypes::KeyCode::End},
        {"enter", kbdtypes::KeyCode::Enter},
        {"esc", kbdtypes::KeyCode::Esc},
        {"f1", kbdtypes::KeyCode::F1},
        {"f2", kbdtypes::KeyCode::F2},
        {"f3", kbdtypes::KeyCode::F3},
        {"f4", kbdtypes::KeyCode::F4},
        {"f5", kbdtypes::KeyCode::F5},
        {"f6", kbdtypes::KeyCode::F6},
        {"f7", kbdtypes::KeyCode::F7},
        {"f8", kbdtypes::KeyCode::F8},
        {"f9", kbdtypes::KeyCode::F9},
        {"f10", kbdtypes::KeyCode::F10},
        {"f11", kbdtypes::KeyCode::F11},
        {"f12", kbdtypes::KeyCode::F12},
        {"home", kbdtypes::KeyCode::Home},
        {"insert", kbdtypes::KeyCode::Insert},
        {"pgdown", kbdtypes::KeyCode::PgDown},
        {"pgup", kbdtypes::KeyCode::PgUp},
        {"space", kbdtypes::KeyCode::Space},
        {"tab", kbdtypes::KeyCode::Tab},
    };

    constexpr std::size_t max_name_size = [](){
        std::size_t ret = 0;
        for (ModPair mp : mod_infos) {
            ret = std::max(ret, mp.str.size());
        }
        for (auto key : key_name_and_codes) {
            ret = std::max(ret, key.name.size());
        }
        return ret;
    }();

    kbdtypes::KeyCode str_to_key_code(KeyLayout::KbdId keyboardLayout, chars_view str)
    {
        uint16_t final_key_u16;
        writable_bytes_view final_key_u16_as_u8{reinterpret_cast<uint8_t*>(&final_key_u16), sizeof(final_key_u16)};

        // is ascii
        if (str.size() == 1 && str[0] < 127) {
            final_key_u16 = static_cast<uint8_t>(ascii_to_lower(str[0]));
        }
        // TODO use UTF8toUnicodeIterator and UTF32ToUTF16(UTF32Tolower(uint32_t))
        else if (const std::size_t n = UTF8toUTF16(str, final_key_u16_as_u8); n == 2) {
            UTF16Lower(final_key_u16_as_u8.data(), 2);
        }
        else {
            LOG(LOG_ERR, "KeyboardShortcutBlocker::Shortcut::Shortcut(): Failed to convert UTF-8 string to UTF-16 string!");
            return kbdtypes::KeyCode();
        }

        using namespace scancode;

        ScancodeSeq scancode_seq = unicode_to_scancode(keyboardLayout, final_key_u16);
        array_view<Scancode16bits> scancodes = scancode_seq.scancodes();

        if (scancodes.size() ==  1) {
            return kbdtypes::KeyCode(scancodes[0]);
        }

        if (scancodes.empty())
        {
            LOG(LOG_ERR, "KeyboardShortcutBlocker::Shortcut::Shortcut(): No scancode sequence for unicode=%d", final_key_u16);
        }
        else if (scancodes.size() != 1)
        {
            LOG(LOG_ERR, "KeyboardShortcutBlocker::Shortcut::Shortcut(): Too much scancode sequence for unicode=%d", final_key_u16);
        }

        return kbdtypes::KeyCode();
    }
}


class KeyboardShortcutBlocker::Shortcut
{
    ModFlags mods;
    kbdtypes::KeyCode keycode;

public:
    Shortcut(ModFlags mods, kbdtypes::KeyCode keycode, bool verbose)
    : mods(mods)
    , keycode(keycode)
    {
        LOG_IF(verbose, LOG_INFO,
            "KeyboardShortcutBlocker::Shortcut::Shortcut(): "
                "Ctrl=%s Alt=%s AltGr=%s Shift=%s Meta=%s keycode=0x%04X",
            mods.test(Mods::Ctrl) ? "Yes" : "No",
            mods.test(Mods::Alt) ? "Yes" : "No",
            mods.test(Mods::AltGr) ? "Yes" : "No",
            mods.test(Mods::Shift) ? "Yes" : "No",
            mods.test(Mods::Meta) ? "Yes" : "No",
            underlying_cast(keycode));
    }

    bool scancode_must_be_blocked(ModFlags mods, KbdFlags keyboardFlags, kbdtypes::KeyCode keycode, bool verbose) const
    {
        if (mods == this->mods && keycode == this->keycode) {
            LOG_IF(verbose, LOG_INFO,
                "KeyboardShortcutBlocker::Shortcut::scancode_must_be_blocked(): Key-%s event. Shortcut=\"%s%s%s%s%s %04X\"",
                bool(keyboardFlags & kbdtypes::KbdFlags::Release) ? "release" : "down",
                mods.test(Mods::Ctrl) ? "Ctrl+" : "",
                mods.test(Mods::Alt) ? "Alt+" : "",
                mods.test(Mods::AltGr) ? "AltGr+" : "",
                mods.test(Mods::Shift) ? "Shift+" : "",
                mods.test(Mods::Meta) ? "Meta+" : "",
                keycode);

            return true;
        }

        return false;
    }
};

KeyboardShortcutBlocker::KeyboardShortcutBlocker(KeyLayout::KbdId keyboardLayout, chars_view configuration_string, bool verbose)
    : verbose(verbose)
{
    LOG_IF(this->verbose, LOG_INFO,
        "KeyboardShortcutBlocker::KeyboardShortcutBlocker(): "
            "KeyboardLayout=0x%X ConfigurationString=\"%.*s\"",
        keyboardLayout, int(configuration_string.size()), configuration_string.data());

    for (auto shortcut : split_with(configuration_string, ',')) {
        this->add_shortcut(keyboardLayout, shortcut);
    }
}

KeyboardShortcutBlocker::~KeyboardShortcutBlocker() = default;

void KeyboardShortcutBlocker::add_shortcut(KeyLayout::KbdId keyboardLayout, chars_view shortcut)
{
    LOG_IF(this->verbose, LOG_INFO,
        "KeyboardShortcutBlocker::add_shortcut(): KeyboardLayout=0x%X Shortcut=\"%.*s\"",
        underlying_cast(keyboardLayout), int(shortcut.size()), shortcut.data());

    ModFlags mods {};

    for (auto r : split_with(shortcut, '+')) {
        auto trimmed = trim(r);
        if (trimmed.empty()) {
            continue;
        }

        if (trimmed.size() > max_name_size) {
            break;
        }

        auto mod_name = ascii_to_limited_lower<max_name_size>(trimmed);

        auto mod = find_mod(mod_name.sv());
        if (mod != ModFlags()) {
            mods |= mod;
            continue;
        }

        kbdtypes::KeyCode keycode = [&]() {
            for (KeyNameAndKeyCode const& key : key_name_and_codes) {
                if (mod_name.sv() == key.name) {
                    return key.code;
                }
            }
            return str_to_key_code(keyboardLayout, trimmed);
        }();

        if (bool(keycode)) {
            this->shortcuts.emplace_back(mods, keycode, this->verbose);
            return ;
        }
    }

    LOG(LOG_WARNING,
        "KeyboardShortcutBlocker::add_shortcut(): Invalid shortcut \"%.*s\"",
        int(shortcut.size()), shortcut.data());
}

bool KeyboardShortcutBlocker::scancode_must_be_blocked(KbdFlags keyboardFlags, Scancode scancode)
{
    using KeyMod = kbdtypes::KeyMod;

    auto set_mod = [&](KeyMod mod){
        this->mods.update(keyboardFlags, mod);
        this->virtual_mods = 0;
        this->virtual_mods |= this->mods.test(KeyMod::LCtrl) << unsigned(Mods::Ctrl);
        this->virtual_mods |= this->mods.test(KeyMod::RCtrl) << unsigned(Mods::Ctrl);
        this->virtual_mods |= this->mods.test(KeyMod::LShift) << unsigned(Mods::Shift);
        this->virtual_mods |= this->mods.test(KeyMod::RShift) << unsigned(Mods::Shift);
        this->virtual_mods |= this->mods.test(KeyMod::LAlt) << unsigned(Mods::Alt);
        this->virtual_mods |= this->mods.test(KeyMod::RAlt) << unsigned(Mods::AltGr);
        this->virtual_mods |= this->mods.test(KeyMod::LMeta) << unsigned(Mods::Meta);
        this->virtual_mods |= this->mods.test(KeyMod::RMeta) << unsigned(Mods::Meta);
    };

    using KeyCode = kbdtypes::KeyCode;

    const auto keycode = kbdtypes::to_keycode(keyboardFlags, scancode);

    switch (underlying_cast(keycode))
    {
        case underlying_cast(KeyCode::LCtrl):  set_mod(KeyMod::LCtrl); break;
        case underlying_cast(KeyCode::RCtrl):  set_mod(KeyMod::RCtrl); break;
        case underlying_cast(KeyCode::LShift): set_mod(KeyMod::LShift); break;
        case underlying_cast(KeyCode::RShift): set_mod(KeyMod::RShift); break;
        case underlying_cast(KeyCode::LAlt):   set_mod(KeyMod::LAlt); break;
        case underlying_cast(KeyCode::RAlt):   set_mod(KeyMod::RAlt); break;
        case underlying_cast(KeyCode::LWin):   set_mod(KeyMod::LMeta); break;
        case underlying_cast(KeyCode::RWin):   set_mod(KeyMod::RMeta); break;
        default: {
            ModFlags mods(this->virtual_mods);

            LOG_IF(this->verbose, LOG_INFO,
                "KeyboardShortcutBlocker::scancode_must_be_blocked(): "
                    "Ctrl=%s Alt=%s AltGr=%s Shift=%s Meta=%s keyboardFlags=0x%04X scancode=0x%02X",
                mods.test(Mods::Ctrl) ? "Yes" : "No",
                mods.test(Mods::Alt) ? "Yes" : "No",
                mods.test(Mods::AltGr) ? "Yes" : "No",
                mods.test(Mods::Shift) ? "Yes" : "No",
                mods.test(Mods::Meta) ? "Yes" : "No",
                keyboardFlags, scancode);

            for (auto const& shortcut : this->shortcuts) {
                if (shortcut.scancode_must_be_blocked(mods, keyboardFlags, keycode, this->verbose)) {
                    return true;
                }
            }
            break;
        }
    }

    return false;
}
