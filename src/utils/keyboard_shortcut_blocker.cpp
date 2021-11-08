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

#include "utils/keyboard_shortcut_blocker.hpp"
#include "keyboard/keyboard_shortcut_layout.hpp"
#include "utils/log.hpp"
#include "utils/utf.hpp"
#include "utils/strutils.hpp"
#include "utils/sugar/split.hpp"
#include "utils/sugar/array_view.hpp"
#include "utils/sugar/flags.hpp"
#include "utils/ascii.hpp"

#include <vector>
#include <memory>


namespace
{
    enum class Mod
    {
        Ctrl,
        Shift,
        Alt,
        AltGr,
        Meta,
        NumLock,
        max_,
    };
} // anonymous namespace

template<>
struct utils::enum_as_flag<Mod>
{
    static constexpr std::size_t max = std::size_t( Mod::max_);
};

namespace
{
    using ModFlags = utils::flags_t<Mod>;
    struct ModPair
    {
        ModFlags mod;
        std::string_view str;
    };

    constexpr ModPair mod_infos[]{
        {Mod::Ctrl, "ctrl"},
        {Mod::Shift, "shift"},
        {Mod::Alt, "alt"},
        {Mod::AltGr, "altgr"},
        {Mod::Meta, "meta"},
        {Mod::Meta, "window"},
        {Mod::Meta, "windows"},
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
        uint8_t compressed_keycode;

        constexpr KeyNameAndKeyCode(std::string_view name, kbdtypes::KeyCode code) noexcept
        : name(name)
        , compressed_keycode(checked_int{kbdtypes::keycode_to_byte_index(code)})
        {
            assert(kbdtypes::keycode_is_compressable_to_byte(code));
        }
    };

    struct KeyNameCompare
    {
        constexpr bool operator()(std::string_view a, std::string_view b) const
        {
            if (a.size() < b.size()) return true;
            if (a.size() > b.size()) return false;
            return a < b;
        }
    };

    // must be sorted
    constexpr KeyNameAndKeyCode key_name_and_codes[] {
        {"f1", kbdtypes::KeyCode::F1},
        {"f2", kbdtypes::KeyCode::F2},
        {"f3", kbdtypes::KeyCode::F3},
        {"f4", kbdtypes::KeyCode::F4},
        {"f5", kbdtypes::KeyCode::F5},
        {"f6", kbdtypes::KeyCode::F6},
        {"f7", kbdtypes::KeyCode::F7},
        {"f8", kbdtypes::KeyCode::F8},
        {"f9", kbdtypes::KeyCode::F9},
        {"del", kbdtypes::KeyCode::Delete},
        {"end", kbdtypes::KeyCode::End},
        {"esc", kbdtypes::KeyCode::Esc},
        {"f10", kbdtypes::KeyCode::F10},
        {"f11", kbdtypes::KeyCode::F11},
        {"f12", kbdtypes::KeyCode::F12},
        {"tab", kbdtypes::KeyCode::Tab},
        {"home", kbdtypes::KeyCode::Home},
        {"keya", kbdtypes::KeyCode::Key_A},
        {"keyb", kbdtypes::KeyCode::Key_B},
        {"keyc", kbdtypes::KeyCode::Key_C},
        {"keyd", kbdtypes::KeyCode::Key_D},
        {"keye", kbdtypes::KeyCode::Key_E},
        {"keyf", kbdtypes::KeyCode::Key_F},
        {"keyg", kbdtypes::KeyCode::Key_G},
        {"keyh", kbdtypes::KeyCode::Key_H},
        {"keyi", kbdtypes::KeyCode::Key_I},
        {"keyj", kbdtypes::KeyCode::Key_J},
        {"keyk", kbdtypes::KeyCode::Key_K},
        {"keyl", kbdtypes::KeyCode::Key_L},
        {"keym", kbdtypes::KeyCode::Key_M},
        {"keyn", kbdtypes::KeyCode::Key_N},
        {"keyo", kbdtypes::KeyCode::Key_O},
        {"keyp", kbdtypes::KeyCode::Key_P},
        {"keyq", kbdtypes::KeyCode::Key_Q},
        {"keyr", kbdtypes::KeyCode::Key_R},
        {"keys", kbdtypes::KeyCode::Key_S},
        {"keyt", kbdtypes::KeyCode::Key_T},
        {"keyu", kbdtypes::KeyCode::Key_U},
        {"keyv", kbdtypes::KeyCode::Key_V},
        {"keyw", kbdtypes::KeyCode::Key_W},
        {"keyx", kbdtypes::KeyCode::Key_X},
        {"keyy", kbdtypes::KeyCode::Key_Y},
        {"keyz", kbdtypes::KeyCode::Key_Z},
        {"pgup", kbdtypes::KeyCode::PgUp},
        {"comma", kbdtypes::KeyCode::Comma},
        {"enter", kbdtypes::KeyCode::Enter},
        {"equal", kbdtypes::KeyCode::Equal},
        {"minus", kbdtypes::KeyCode::Minus},
        {"quote", kbdtypes::KeyCode::Quote},
        {"slash", kbdtypes::KeyCode::Slash},
        {"space", kbdtypes::KeyCode::Space},
        {"delete", kbdtypes::KeyCode::Delete},
        {"digit0", kbdtypes::KeyCode::Digit0},
        {"digit1", kbdtypes::KeyCode::Digit1},
        {"digit2", kbdtypes::KeyCode::Digit2},
        {"digit3", kbdtypes::KeyCode::Digit3},
        {"digit4", kbdtypes::KeyCode::Digit4},
        {"digit5", kbdtypes::KeyCode::Digit5},
        {"digit6", kbdtypes::KeyCode::Digit6},
        {"digit7", kbdtypes::KeyCode::Digit7},
        {"digit8", kbdtypes::KeyCode::Digit8},
        {"digit9", kbdtypes::KeyCode::Digit9},
        {"escape", kbdtypes::KeyCode::Esc},
        {"insert", kbdtypes::KeyCode::Insert},
        {"pageup", kbdtypes::KeyCode::PgUp},
        {"period", kbdtypes::KeyCode::Period},
        {"pgdown", kbdtypes::KeyCode::PgDown},
        {"numpad0", kbdtypes::KeyCode::Numpad0},
        {"numpad1", kbdtypes::KeyCode::Numpad1},
        {"numpad2", kbdtypes::KeyCode::Numpad2},
        {"numpad3", kbdtypes::KeyCode::Numpad3},
        {"numpad4", kbdtypes::KeyCode::Numpad4},
        {"numpad5", kbdtypes::KeyCode::Numpad5},
        {"numpad6", kbdtypes::KeyCode::Numpad6},
        {"numpad7", kbdtypes::KeyCode::Numpad7},
        {"numpad8", kbdtypes::KeyCode::Numpad8},
        {"numpad9", kbdtypes::KeyCode::Numpad9},
        {"pagedown", kbdtypes::KeyCode::PgDown},
        {"backquote", kbdtypes::KeyCode::Backquote},
        {"backslash", kbdtypes::KeyCode::Backslash},
        {"backspace", kbdtypes::KeyCode::Backspace},
        {"numpadAdd", kbdtypes::KeyCode::NumpadAdd},
        {"semicolon", kbdtypes::KeyCode::Semicolon},
        {"bracketleft", kbdtypes::KeyCode::BracketLeft},
        {"numpadenter", kbdtypes::KeyCode::NumpadEnter},
        {"bracketright", kbdtypes::KeyCode::BracketRight},
        {"numpaddivide", kbdtypes::KeyCode::NumpadDivide},
        {"intlbackslash", kbdtypes::KeyCode::IntlBackslash},
        {"numpaddecimal", kbdtypes::KeyCode::NumpadDecimal},
        {"numpadmultiply", kbdtypes::KeyCode::NumpadMultiply},
        {"numpadsubtract", kbdtypes::KeyCode::NumpadSubtract},
    };

    struct KeyNames
    {
        std::string_view names[std::size(key_name_and_codes)] {};

        constexpr KeyNames()
        {
            auto* p = names;
            for (auto kc : key_name_and_codes) {
                *p++ = kc.name;
            }
        }
    };

    struct KeyCodes
    {
        uint8_t compressed_keycodes[std::size(key_name_and_codes)] {};

        constexpr KeyCodes()
        {
            auto* p = compressed_keycodes;
            for (auto kc : key_name_and_codes) {
                *p++ = kc.compressed_keycode;
            }
        }
    };

    constexpr KeyNames keynames{};
    constexpr KeyCodes keycodes{};

    /// \return unsorted index + 1
    constexpr int sorted_key_name_and_codes()
    {
        auto it = std::begin(keynames.names);
        auto prev = it++;
        auto last = std::end(keynames.names);
        for (; it < last; ++it, ++prev) {
            if (!KeyNameCompare()(*prev, *it)) {
                return int(it - std::begin(keynames.names)) + 1;
            }
        }
        return 0;
    }

    template<int i>
    struct check_sorted
    {
        static_assert(i == 0);
    };

    [[maybe_unused]]
    constexpr check_sorted<sorted_key_name_and_codes()> check_sorted_x {};

    constexpr std::size_t max_name_size = [](){
        std::size_t ret = 0;
        for (ModPair mp : mod_infos) {
            ret = std::max(ret, mp.str.size());
        }
        for (auto name : keynames.names) {
            ret = std::max(ret, name.size());
        }
        return ret;
    }();

    struct ShortcutStage1
    {
        uint8_t compressed_keycode;
        uint8_t mods;

        bool operator == (ShortcutStage1 const& other) const noexcept
        {
            return compressed_keycode == other.compressed_keycode
                && mods == other.mods;
        }
    };

    void add_shortcut(std::vector<ShortcutStage1>& shortcuts, KeyboardShortcutLayout layout, chars_view shortcut)
    {
        ModFlags mods {};

        auto splitted = split_with(shortcut, '+');

        for (auto key_it = splitted.begin(); key_it != splitted.end(); ++key_it ) {
            auto key = *key_it;

            // maybe '+' key (ex: "ctrl++")
            if (REDEMPTION_UNLIKELY(key.empty())) {
                ++key_it;
                if (key_it != splitted.end() && *key.data() == '+' && key.data() + 1 == shortcut.end()) {
                    key = {key.data(), 1};
                }
                // partial value. Ex: "ctrl+" <- no value after '+'
                else {
                    break;
                }
            }

            if (REDEMPTION_UNLIKELY(key.size() > max_name_size)) {
                break;
            }

            auto mod_name = ascii_to_limited_lower<max_name_size>(key);

            auto mod = find_mod(mod_name.sv());
            if (mod != ModFlags()) {
                mods |= mod;
                continue;
            }

            if (key.end() != shortcut.end()) {
                break;
            }

            auto it = std::lower_bound(std::begin(keynames.names), std::end(keynames.names),
                mod_name.sv(), KeyNameCompare{});
            if (it != std::end(keynames.names) && *it == mod_name.sv()) {
                auto compressed_keycode = keycodes.compressed_keycodes[it - std::begin(keynames.names)];
                shortcuts.emplace_back(ShortcutStage1{compressed_keycode, mods.as_uint()});
                return;
            }

            uint8_t unicode16[4];
            if (UTF8toUTF16(mod_name.sv(), make_writable_array_view(unicode16)) == 2) {
                uint16_t unicode = static_cast<uint16_t>((unicode16[1] << 8) | unicode16[0]);
                auto av = layout.find_unicode(unicode);
                if (av.empty()) {
                    break;
                }
                for (auto sc_and_mods : av) {
                    auto newmods = mods;
                    newmods |= (underlying_cast(sc_and_mods.mods) & underlying_cast(KeyboardShortcutLayout::KSMods::Shift)) ? Mod::Shift : ModFlags();
                    newmods |= (underlying_cast(sc_and_mods.mods) & underlying_cast(KeyboardShortcutLayout::KSMods::NumLock)) ? Mod::NumLock : ModFlags();
                    shortcuts.emplace_back(ShortcutStage1{sc_and_mods.compressed_keycode, newmods.as_uint()});
                }
                return;
            }

            break;
        }

        LOG(LOG_WARNING,
            "KeyboardShortcutBlocker::add_shortcut(): Invalid shortcut \"%.*s\"",
            int(shortcut.size()), shortcut.data());
    }

    uint8_t mods_to_virtual_mods(kbdtypes::KeyModFlags mods) noexcept
    {
        using KeyMods = kbdtypes::KeyMod;

        ModFlags vmods {};
        vmods.set_if(mods.test(KeyMods::LCtrl), Mod::Ctrl);
        vmods.set_if(mods.test(KeyMods::RCtrl), Mod::Ctrl);
        vmods.set_if(mods.test(KeyMods::LShift), Mod::Shift);
        vmods.set_if(mods.test(KeyMods::RShift), Mod::Shift);
        vmods.set_if(mods.test(KeyMods::LAlt), Mod::Alt);
        vmods.set_if(mods.test(KeyMods::RAlt), Mod::AltGr);
        vmods.set_if(mods.test(KeyMods::LMeta), Mod::Meta);
        vmods.set_if(mods.test(KeyMods::RMeta), Mod::Meta);
        vmods.set_if(mods.test(KeyMods::NumLock), Mod::NumLock);
        return vmods.as_uint();
    }
} // anonymous namespace


class KeyboardShortcutBlocker::Shortcut
{
    struct Data
    {
        uint8_t index_high;
        uint8_t index_low;
        uint8_t len;
    };

    Data data;

public:
    static constexpr std::ptrdiff_t max_shortcut = 0xffff - sizeof(Data)*256 /* 256 = max range of code */;

    Shortcut() noexcept = default;

    Shortcut(std::ptrdiff_t index, std::ptrdiff_t length) noexcept
        : data{checked_int(index >> 8), uint8_t(index), checked_int(length)}
    {
    }

    int index() const noexcept
    {
        return (data.index_high << 8) | data.index_low;
    }

    uint8_t length() const noexcept
    {
        return data.len;
    }

    static std::unique_ptr<Shortcut> create_shortcuts(
        uint8_t min_code, uint8_t max_code,
        array_view<ShortcutStage1> shortcuts)
    {
        auto const nb_shortcut = max_code - min_code + std::size_t(1);
        auto const nb_mods = shortcuts.size() / sizeof(Shortcut) + (sizeof(Shortcut) - 1);

        // Memory layout: {Shortcut..., mods...}

        auto* const refs = new Shortcut[nb_shortcut + nb_mods];
        std::unique_ptr<Shortcut> ret(refs);

        std::memset(refs, 0, nb_shortcut * sizeof(Shortcut));
        auto* const p = reinterpret_cast<uint8_t*>(refs); /* NOLINT */
        auto* mods = p + nb_shortcut * sizeof(Shortcut);
        auto* end_mods = mods;

        uint8_t previous_compressed_keycode = shortcuts.front().compressed_keycode;
        for (auto shortcut : shortcuts) {
            if (shortcut.compressed_keycode != previous_compressed_keycode) {
                refs[previous_compressed_keycode - min_code] = Shortcut(mods - p, end_mods - mods);
                previous_compressed_keycode = shortcut.compressed_keycode;
                mods = end_mods;
            }
            *end_mods++ = shortcut.mods;
        }
        refs[previous_compressed_keycode - min_code] = Shortcut(mods - p, end_mods - mods);

        return ret;
    }

    static bool is_blocked(uint8_t min_code, uint8_t max_code, Shortcut const* shortcuts, uint8_t compressed_keycode, uint8_t mods) noexcept
    {
        if (min_code <= compressed_keycode && compressed_keycode <= max_code) {
            auto& shortcut = shortcuts[compressed_keycode - min_code];
            auto* p = reinterpret_cast<uint8_t const*>(shortcuts); /* NOLINT */

            // remove NumLock when code is not a numpad lockable key
            if (compressed_keycode < 0x47 || 0x53 < compressed_keycode
             || compressed_keycode == 0x4A || compressed_keycode == 0x4E
            ) {
                mods = (ModFlags(mods) - Mod::NumLock).as_uint();
            }

            for (auto shortcut_mods : array_view{p + shortcut.index(), shortcut.length()}) {
                if (mods == shortcut_mods) {
                    return true;
                }
            }
        }
        return false;
    }
};

KeyboardShortcutBlocker::KeyboardShortcutBlocker(bool verbose) noexcept
    : verbose(verbose)
{}

KeyboardShortcutBlocker::KeyboardShortcutBlocker(KeyLayout::KbdId kid, chars_view configuration_string, bool verbose)
    : verbose(verbose)
{
    this->set_shortcuts(kid, configuration_string);
}

void KeyboardShortcutBlocker::set_shortcuts(KeyLayout::KbdId kid, chars_view configuration_string)
{
    this->shortcuts.reset();

    LOG_IF(this->verbose, LOG_INFO,
        "KeyboardShortcutBlocker::KeyboardShortcutBlocker(): "
            "KeyboardLayout=0x%X ConfigurationString=\"%.*s\"",
        kid, int(configuration_string.size()), configuration_string.data());

    if (configuration_string.empty()) {
        return;
    }

    std::vector<ShortcutStage1> shortcuts;

    auto layout = KeyboardShortcutLayout::find_layout_by_id(kid);
    if (!layout) {
        layout = KeyboardShortcutLayout::find_layout_by_id(KeyLayout::KbdId(0x00000409) /*en-US*/);
    }
    if (!layout) {
        return ;
    }

    for (auto shortcut : split_with(configuration_string, ',')) {
        if (shortcut.empty()) {
            if (shortcut.end() == configuration_string.end()) {
                break;
            }
            shortcut = ","_av;
        }
        // with "ctrl+,", shortcut="ctrl+". Change to shortcut="ctrl+,"
        // with "ctrl++,", shortcut="ctrl++" -> ok
        else if (shortcut.end() != configuration_string.end() && shortcut.back() == '+' && shortcut.size() > 2 && shortcut[shortcut.size()-2] != '+') {
            shortcut = {shortcut.data(), shortcut.size() + 1};
        }
        LOG_IF(this->verbose, LOG_INFO,
            "KeyboardShortcutBlocker::add_shortcut(): Shortcut=\"%.*s\"",
            int(shortcut.size()), shortcut.data());
        add_shortcut(shortcuts, layout, shortcut);
    }

    if (shortcuts.empty()) {
        return ;
    }

    std::sort(shortcuts.begin(), shortcuts.end(), [](auto shortcut1, auto shortcut2) noexcept {
        return shortcut1.compressed_keycode < shortcut2.compressed_keycode
            || (shortcut1.compressed_keycode == shortcut2.compressed_keycode && shortcut1.mods < shortcut2.mods);
    });
    auto first = shortcuts.begin();
    auto last = shortcuts.end();
    last = std::unique(first, last);

    if (last - first > Shortcut::max_shortcut) {
        LOG(LOG_ERR,
            "KeyboardShortcutBlocker::add_shortcut(): To many shortcut (%zu)",
            std::size_t(last - first));
        last = first + Shortcut::max_shortcut;
    }

    this->min_code = safe_int(first->compressed_keycode);
    this->max_code = safe_int((last-1)->compressed_keycode);

    this->shortcuts = Shortcut::create_shortcuts(
        this->min_code, this->max_code,
        array_view{shortcuts}.first(std::size_t(last - first))
    );
}

bool KeyboardShortcutBlocker::has_shortcut() const noexcept
{
    return bool(this->shortcuts);
}

KeyboardShortcutBlocker::~KeyboardShortcutBlocker() = default;

void KeyboardShortcutBlocker::sync_locks(KeyboardShortcutBlocker::KeyLocks key_locks) noexcept
{
    this->mods.sync_locks(key_locks);
    this->virtual_mods = mods_to_virtual_mods(this->mods);
}

bool KeyboardShortcutBlocker::scancode_must_be_blocked(KbdFlags keyboardFlags, Scancode scancode) noexcept
{
    if (!this->has_shortcut()) {
        return false;
    }

    using KeyMods = kbdtypes::KeyMod;

    auto set_mod = [&](KeyMods mod){
        this->mods.update(keyboardFlags, mod);
        this->virtual_mods = mods_to_virtual_mods(this->mods);
    };

    using KeyCode = kbdtypes::KeyCode;

    const auto keycode = kbdtypes::to_keycode(keyboardFlags, scancode);

    switch (underlying_cast(keycode))
    {
        case underlying_cast(KeyCode::NumLock):
            if (!bool(keyboardFlags & KbdFlags::Release)) {
                this->mods.flip(KeyMods::NumLock);
                this->virtual_mods = mods_to_virtual_mods(this->mods);
            }
            break;

        case underlying_cast(KeyCode::LCtrl):  set_mod(KeyMods::LCtrl); break;
        case underlying_cast(KeyCode::RCtrl):  set_mod(KeyMods::RCtrl); break;
        case underlying_cast(KeyCode::LShift): set_mod(KeyMods::LShift); break;
        case underlying_cast(KeyCode::RShift): set_mod(KeyMods::RShift); break;
        case underlying_cast(KeyCode::LAlt):   set_mod(KeyMods::LAlt); break;
        case underlying_cast(KeyCode::RAlt):   set_mod(KeyMods::RAlt); break;
        case underlying_cast(KeyCode::LWin):   set_mod(KeyMods::LMeta); break;
        case underlying_cast(KeyCode::RWin):   set_mod(KeyMods::RMeta); break;
        default: {
            LOG_IF(this->verbose, LOG_INFO,
                "KeyboardShortcutBlocker::scancode_must_be_blocked(): "
                    "Ctrl=%s Alt=%s AltGr=%s Shift=%s Meta=%s NumLock=%s keyboardFlags=0x%04X scancode=0x%02X",
                ModFlags(this->virtual_mods).test( Mod::Ctrl) ? "Yes" : "No",
                ModFlags(this->virtual_mods).test( Mod::Alt) ? "Yes" : "No",
                ModFlags(this->virtual_mods).test( Mod::AltGr) ? "Yes" : "No",
                ModFlags(this->virtual_mods).test( Mod::Shift) ? "Yes" : "No",
                ModFlags(this->virtual_mods).test( Mod::Meta) ? "Yes" : "No",
                ModFlags(this->virtual_mods).test( Mod::NumLock) ? "Yes" : "No",
                keyboardFlags, scancode);

            return kbdtypes::keycode_is_compressable_to_byte(keycode)
                && Shortcut::is_blocked(
                    this->min_code, this->max_code, this->shortcuts.get(),
                    checked_int{kbdtypes::keycode_to_byte_index(keycode)},
                    this->virtual_mods
                );
        }
    }

    return false;
}
