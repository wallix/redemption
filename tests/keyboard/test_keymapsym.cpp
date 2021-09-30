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


#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "keyboard/keymapsym.hpp"
#include "keyboard/keylayouts.hpp"


namespace
{

struct Keysyms
{
    KeymapSym::Keys keys;

    bool operator == (Keysyms const& other) const noexcept
    {
        if (keys.size() == other.keys.size()) {
            auto p = other.keys.begin();
            for (auto key : keys) {
                if (key.down_flag != p->down_flag || key.keysym != p->keysym) {
                    return false;
                }
                ++p;
            }
            return true;
        }

        return false;
    }
};

}

#if !REDEMPTION_UNIT_TEST_FAST_CHECK
# include "utils/sugar/int_to_chars.hpp"
# include "test_only/test_framework/compare_collection.hpp"

namespace
{

static ut::assertion_result test_comp_keysyms(Keysyms const& a, Keysyms const& b)
{
    ut::assertion_result ar(true);

    if (REDEMPTION_UNLIKELY(!(a == b))) {
        ar = false;

        auto put = [&](std::ostream& out, array_view<KeymapSym::Key> keys){
            for (auto key : keys) {

                char uchar[] = " (x)";
                auto to_ascii = [](char* s, uint32_t uni) -> char const* {
                    if (' ' <= uni && uni <= '~') {
                        s[2] = char(uni);
                        return s;
                    }
                    return "";
                };

                out << "{" << (key.down_flag == KeymapSym::VncDownFlag::Down ? "down" : "up")
                    << ", .ksym=0x" << int_to_fixed_hexadecimal_upper_zchars(key.keysym)
                    << to_ascii(uchar, key.keysym) << "},";
            }
        };

        auto& out = ar.message().stream();
        out << "[";
        ut::put_data_with_diff(out, a.keys, "!=", b.keys, put);
        out << "]";
    }

    return ar;
}

}

RED_TEST_DISPATCH_COMPARISON_EQ((), (::Keysyms), (::Keysyms), ::test_comp_keysyms)
#endif

namespace
{
    constexpr uint16_t eacute = 0xa9c3 /* é */;

    using KeyCode = kbdtypes::KeyCode;
    using Scancode = kbdtypes::Scancode;
    using KbdFlags = kbdtypes::KbdFlags;
    using Mod = kbdtypes::KeyMod;
    using Mods = kbdtypes::KeyModFlags;
    using KeyLocks = kbdtypes::KeyLocks;
    using DecodedKeys = Keymap::DecodedKeys;

    const auto down = kbdtypes::KbdFlags();
    const auto release = kbdtypes::KbdFlags::Release;
    const auto extended = kbdtypes::KbdFlags::Extended;
    const auto extended1 = kbdtypes::KbdFlags::Extended1;

    KeymapSym::Key vnc_up(uint32_t uc) { return {uc, KeymapSym::VncDownFlag::Up}; }
    KeymapSym::Key vnc_down(uint32_t uc) { return {uc, KeymapSym::VncDownFlag::Down}; }

    template<class... Key>
    Keysyms ksyms(Key... key)
    {
        KeymapSym::Keys keys;
        (..., keys.push(key));
        return {keys};
    };

    auto* layout_fr = find_layout_by_id(KeyLayout::KbdId(0x040C));
}


RED_AUTO_TEST_CASE(TestKeymapSymTounicode)
{
    KeymapSym keymap(*layout_fr, KeyLocks(), KeymapSym::IsApple::No, KeymapSym::IsUnix::No, false);

    auto utf16_to_keysyms = [&keymap](KeymapSym::KbdFlags flag, uint16_t utf16) {
        return Keysyms{keymap.utf16_to_keysyms(flag, utf16)};
    };

    auto scancode_to_keysyms = [&keymap](KbdFlags flags, Scancode sc) {
        return Keysyms{keymap.scancode_to_keysyms(flags, sc)};
    };

    // null
    RED_CHECK(utf16_to_keysyms(down, 0) == ksyms());
    RED_CHECK(utf16_to_keysyms(release, 0) == ksyms());

    // a
    RED_CHECK(utf16_to_keysyms(down, 'a') == ksyms(vnc_down('a')));
    RED_CHECK(utf16_to_keysyms(release, 'a') == ksyms(vnc_up('a')));

    // é
    RED_CHECK(utf16_to_keysyms(down, eacute) == ksyms(vnc_down(0x01000000 | eacute)));
    RED_CHECK(utf16_to_keysyms(release, eacute) == ksyms(vnc_up(0x01000000 | eacute)));

    // 🚀 (rocket)
    RED_CHECK(utf16_to_keysyms(down, 0xd83d) == ksyms());
    RED_CHECK(utf16_to_keysyms(down, 0xde80) == ksyms(vnc_down(0x01000000 | 0x1f680)));
    RED_CHECK(utf16_to_keysyms(release, 0xd83d) == ksyms());
    RED_CHECK(utf16_to_keysyms(release, 0xde80) == ksyms(vnc_up(0x01000000 | 0x1f680)));

    // a (again)
    RED_CHECK(utf16_to_keysyms(down, 'a') == ksyms(vnc_down('a')));
    RED_CHECK(utf16_to_keysyms(release, 'a') == ksyms(vnc_up('a')));

    // shift + altgr + 🚀 (rocket)
    RED_CHECK(scancode_to_keysyms(down, Scancode::LShift) == ksyms(vnc_down(0xffe1)));
    RED_CHECK(scancode_to_keysyms(down | extended, Scancode::LAlt) == ksyms(vnc_down(0xffea)));
    RED_CHECK(utf16_to_keysyms(down, 0xd83d) == ksyms());
    RED_CHECK(utf16_to_keysyms(down, 0xde80) == ksyms(vnc_up(0xffea), vnc_down(0x01000000 | 0x1f680), vnc_down(0xffea)));
    RED_CHECK(utf16_to_keysyms(release, 0xd83d) == ksyms());
    RED_CHECK(utf16_to_keysyms(release, 0xde80) == ksyms(vnc_up(0xffea), vnc_up(0x01000000 | 0x1f680), vnc_down(0xffea)));
    RED_CHECK(scancode_to_keysyms(release | extended, Scancode::LAlt) == ksyms(vnc_up(0xffea)));
    RED_CHECK(scancode_to_keysyms(release, Scancode::LShift) == ksyms(vnc_up(0xffe1)));
}

RED_AUTO_TEST_CASE(TestKeymapSym)
{
    KeymapSym keymap(*layout_fr, KeyLocks(), KeymapSym::IsApple::No, KeymapSym::IsUnix::No, false);

    auto scancode_to_keysyms = [&keymap](KbdFlags flags, Scancode sc) {
        return Keysyms{keymap.scancode_to_keysyms(flags, sc)};
    };

    // a
    RED_CHECK(scancode_to_keysyms(down, Scancode(0x10)) == ksyms(vnc_down('a')));
    RED_CHECK(scancode_to_keysyms(release, Scancode(0x10)) == ksyms(vnc_up('a')));

    // shift + a
    RED_CHECK(scancode_to_keysyms(down, Scancode::LShift) == ksyms(vnc_down(0xffe1)));
    RED_CHECK(scancode_to_keysyms(down, Scancode(0x10)) == ksyms(vnc_down('A')));
    RED_CHECK(scancode_to_keysyms(release, Scancode(0x10)) == ksyms(vnc_up('A')));
    RED_CHECK(scancode_to_keysyms(release, Scancode::LShift) == ksyms(vnc_up(0xffe1)));

    // é
    RED_CHECK(scancode_to_keysyms(down, Scancode(0x03)) == ksyms(vnc_down(0xe9)));
    RED_CHECK(scancode_to_keysyms(release, Scancode(0x03)) == ksyms(vnc_up(0xe9)));

    const uint16_t euro = 0x20ac /* € */;

    // altgr + e (remove altgr, push €)
    RED_CHECK(scancode_to_keysyms(down | extended, Scancode::LAlt) == ksyms(vnc_down(0xffea)));
    RED_CHECK(scancode_to_keysyms(down, Scancode(0x12))
        == ksyms(vnc_up(0xffea), vnc_down(0x01000000 | euro), vnc_down(0xffea)));
    RED_CHECK(scancode_to_keysyms(release, Scancode(0x12))
        == ksyms(vnc_up(0xffea), vnc_up(0x01000000 | euro), vnc_down(0xffea)));
    RED_CHECK(scancode_to_keysyms(release | extended, Scancode::LAlt) == ksyms(vnc_up(0xffea)));

    // ctrl + alt + e (remove ctrl+alt, push €)
    RED_CHECK(scancode_to_keysyms(down, Scancode::LCtrl) == ksyms(vnc_down(0xffe3)));
    RED_CHECK(scancode_to_keysyms(down, Scancode::LAlt) == ksyms(vnc_down(0xffe9)));
    RED_CHECK(scancode_to_keysyms(down, Scancode(0x12))
        == ksyms(vnc_up(0xffe9), vnc_up(0xffe3), vnc_down(0x01000000 | euro), vnc_down(0xffe9), vnc_down(0xffe3)));
    RED_CHECK(scancode_to_keysyms(release, Scancode(0x12))
        == ksyms(vnc_up(0xffe9), vnc_up(0xffe3), vnc_up(0x01000000 | euro), vnc_down(0xffe9), vnc_down(0xffe3)));
    RED_CHECK(scancode_to_keysyms(release, Scancode::LCtrl) == ksyms(vnc_up(0xffe3)));
    RED_CHECK(scancode_to_keysyms(release, Scancode::LAlt) == ksyms(vnc_up(0xffe9)));

    // dead key
    RED_CHECK(scancode_to_keysyms(down, Scancode(0x1a)) == ksyms());
    RED_CHECK(scancode_to_keysyms(release, Scancode(0x1a)) == ksyms());
    RED_CHECK(scancode_to_keysyms(down, Scancode(0x1a)) == ksyms(vnc_down('^'), vnc_up('^'), vnc_down('^')));
    RED_CHECK(scancode_to_keysyms(release, Scancode(0x1a)) == ksyms(vnc_up('^')));

    // end
    RED_CHECK(scancode_to_keysyms(down | extended, Scancode(0x4F)) == ksyms(vnc_down(0xff57)));
    RED_CHECK(scancode_to_keysyms(release | extended, Scancode(0x4F)) == ksyms(vnc_up(0xff57)));

    // ctrl+alt+end -> ctrl+alt+del
    RED_CHECK(scancode_to_keysyms(down, Scancode::LCtrl) == ksyms(vnc_down(0xffe3)));
    RED_CHECK(scancode_to_keysyms(down, Scancode::LAlt) == ksyms(vnc_down(0xffe9)));
    RED_CHECK(scancode_to_keysyms(down | extended, Scancode(0x4F)) == ksyms(vnc_down(0xffff)));
    RED_CHECK(scancode_to_keysyms(release | extended, Scancode(0x4F)) == ksyms(vnc_up(0xffff)));
    RED_CHECK(scancode_to_keysyms(release, Scancode::LCtrl) == ksyms(vnc_up(0xffe3)));
    RED_CHECK(scancode_to_keysyms(release, Scancode::LAlt) == ksyms(vnc_up(0xffe9)));

    // pause
    RED_CHECK(scancode_to_keysyms(down | extended1, Scancode::LCtrl) == ksyms(vnc_down(0xff13)));
    // 0x0000ff7f (numlocks) is acceptable ?
    RED_CHECK(scancode_to_keysyms(down, Scancode(0x45)) == ksyms(vnc_down(0x0000ff7f)));
    RED_CHECK(scancode_to_keysyms(release, Scancode(0x45)) == ksyms(vnc_up(0x0000ff7f)));
    RED_CHECK(scancode_to_keysyms(release | extended1, Scancode::LCtrl) == ksyms(vnc_up(0xff13)));
}

RED_AUTO_TEST_CASE(TestKeymapMacOS)
{
    KeymapSym keymap(*layout_fr, KeyLocks(), KeymapSym::IsApple::Yes, KeymapSym::IsUnix::No, false);

    auto scancode_to_keysyms = [&keymap](KbdFlags flags, Scancode sc) {
        return Keysyms{keymap.scancode_to_keysyms(flags, sc)};
    };

    // a
    RED_CHECK(scancode_to_keysyms(down, Scancode(0x10)) == ksyms(vnc_down('q')));
    RED_CHECK(scancode_to_keysyms(release, Scancode(0x10)) == ksyms(vnc_up('q')));

    // shift + a
    RED_CHECK(scancode_to_keysyms(down, Scancode::LShift) == ksyms(vnc_down(0xffe1)));
    RED_CHECK(scancode_to_keysyms(down, Scancode(0x10)) == ksyms(vnc_down('Q')));
    RED_CHECK(scancode_to_keysyms(release, Scancode(0x10)) == ksyms(vnc_up('Q')));
    RED_CHECK(scancode_to_keysyms(release, Scancode::LShift) == ksyms(vnc_up(0xffe1)));

    // ctrl
    RED_CHECK(scancode_to_keysyms(down, Scancode::LCtrl) == ksyms(vnc_down(0xffe3)));
    RED_CHECK(scancode_to_keysyms(release, Scancode::LCtrl) == ksyms(vnc_up(0xffe3)));

    // altgr + e
    RED_CHECK(scancode_to_keysyms(down | extended, Scancode::LAlt) == ksyms(vnc_down(0xffea)));
    RED_CHECK(scancode_to_keysyms(down, Scancode(0x12)) == ksyms(vnc_down('e')));
    RED_CHECK(scancode_to_keysyms(release, Scancode(0x12)) == ksyms(vnc_up('e')));
    RED_CHECK(scancode_to_keysyms(release | extended, Scancode::LAlt) == ksyms(vnc_up(0xffea)));
}


