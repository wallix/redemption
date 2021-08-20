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
#include "utils/sugar/algostring.hpp"
#include "utils/sugar/splitter.hpp"
#include "utils/sugar/array_view.hpp"
#include "utils/utf.hpp"
#include "utils/sugar/flags.hpp"


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

    constexpr std::size_t max_mod_name_size_fn()
    {
        std::size_t ret = 0;
        for (ModPair mp : mod_infos) {
            ret = std::max(ret, mp.str.size());
        }
        return ret;
    }

    constexpr std::size_t max_mod_name_size = max_mod_name_size_fn();

    struct ascii_lower_convertor
    {
        constexpr ascii_lower_convertor()
        {
            for (std::size_t i = 0; i < std::size(table); ++i) {
                table[i] = char(i);
            }
            for (int i = 'A'; i < 'Z'; ++i) {
                table[i] = char(i - 'A' + 'a');
            }
        }

        char operator()(char c) const
        {
            return table[int(c)];
        }

        char table[256] {};
    };

    constexpr ascii_lower_convertor to_ascii_lower {};

    kbdtypes::KeyCode str_to_key_code(uint32_t keyboardLayout, chars_view str)
    {
        uint16_t final_key_u16;
        writable_bytes_view final_key_u16_as_u8{reinterpret_cast<uint8_t*>(&final_key_u16), sizeof(final_key_u16)};

        if (str.size() == 1 && str[0] < 127) {
            final_key_u16 = static_cast<uint8_t>(to_ascii_lower(str[0]));
        }
        else if (const std::size_t n = UTF8toUTF16(str, final_key_u16_as_u8); n == 2) {
            UTF16Lower(final_key_u16_as_u8.data(), n);
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

KeyboardShortcutBlocker::KeyboardShortcutBlocker(uint32_t keyboardLayout, chars_view configuration_string, bool verbose)
    : verbose(verbose)
{
    LOG_IF(this->verbose, LOG_INFO,
        "KeyboardShortcutBlocker::KeyboardShortcutBlocker(): "
            "KeyboardLayout=0x%X ConfigurationString=\"%.*s\"",
        keyboardLayout, int(configuration_string.size()), configuration_string.data());

    for (auto r : make_splitter(configuration_string, ',')) {
        this->add_shortcut(keyboardLayout, {r.begin(), r.end()});
    }
}

KeyboardShortcutBlocker::~KeyboardShortcutBlocker() = default;

void KeyboardShortcutBlocker::add_shortcut(uint32_t keyboardLayout, chars_view shortcut)
{
    LOG_IF(this->verbose, LOG_INFO,
        "KeyboardShortcutBlocker::add_shortcut(): KeyboardLayout=0x%X Shortcut=\"%.*s\"",
        keyboardLayout, int(shortcut.size()), shortcut.data());

    ModFlags mods {};

    for (auto r : make_splitter(shortcut, '+')) {
        auto trimmed = trim(r);
        if (trimmed.empty()) {
            continue;
        }

        constexpr std::string_view enter = "enter";
        constexpr std::size_t buff_size = std::max(max_mod_name_size, enter.size());

        if (trimmed.size() > buff_size) {
            break;
        }

        // convert to lower case
        char buff[buff_size];
        char *p = buff;
        for (char c : trimmed) {
            *p = to_ascii_lower(c);
            ++p;
        }
        std::string_view mod_name{buff, p};

        auto mod = find_mod(mod_name);
        if (mod != ModFlags()) {
            mods |= mod;
            continue;
        }

        kbdtypes::KeyCode keycode = (mod_name == enter)
            ? kbdtypes::KeyCode::Enter
            : str_to_key_code(keyboardLayout, {trimmed.begin(), trimmed.end()});

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
