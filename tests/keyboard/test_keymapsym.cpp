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

                out << "{" << (key.down_flag == KeymapSym::VncKeyState::Down ? "down" : "up")
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

    using Scancode = kbdtypes::Scancode;
    using KbdFlags = kbdtypes::KbdFlags;
    using KeyLocks = kbdtypes::KeyLocks;

    const auto down = kbdtypes::KbdFlags();
    const auto release = kbdtypes::KbdFlags::Release;
    const auto extended = kbdtypes::KbdFlags::Extended;
    const auto extended1 = kbdtypes::KbdFlags::Extended1;

    KeymapSym::Key stUp(uint32_t uc) { return {uc, KeymapSym::VncKeyState::Up}; }
    KeymapSym::Key stDown(uint32_t uc) { return {uc, KeymapSym::VncKeyState::Down}; }

    template<class... Key>
    Keysyms ksyms(Key... key)
    {
        KeymapSym::Keys keys;
        (..., keys.push(key));
        return {keys};
    }

    auto* layout_fr = find_layout_by_id(KeyLayout::KbdId(0x040C));
}

namespace
{
    struct KeymapSymTest
    {
        KeymapSymTest(KeymapSym::IsApple is_apple, KeymapSym::IsUnix is_unix)
        : keymapSym(*layout_fr, KeyLocks(), is_apple, is_unix, false)
        {}

        Keysyms utf16(KeymapSym::KbdFlags flag, uint16_t utf16)
        {
            return Keysyms{keymapSym.utf16_to_keysyms(flag, utf16)};
        }

        Keysyms scancode(KbdFlags flags, Scancode sc)
        {
            return Keysyms{keymapSym.scancode_to_keysyms(flags, sc)};
        }

        Keysyms reset_mods()
        {
            return Keysyms{keymapSym.reset_mods({})};
        }

    private:
        KeymapSym keymapSym;
    };
} // anonymous namespace

RED_AUTO_TEST_CASE(TestKeymapSymTounicode)
{
    KeymapSymTest keymap(KeymapSym::IsApple::No, KeymapSym::IsUnix::No);

    // null
    RED_CHECK(keymap.utf16(down, 0) == ksyms());
    RED_CHECK(keymap.utf16(release, 0) == ksyms());

    // a
    RED_CHECK(keymap.utf16(down, 'a') == ksyms(stDown('a')));
    RED_CHECK(keymap.utf16(release, 'a') == ksyms(stUp('a')));

    // é
    RED_CHECK(keymap.utf16(down, eacute) == ksyms(stDown(0x01000000 | eacute)));
    RED_CHECK(keymap.utf16(release, eacute) == ksyms(stUp(0x01000000 | eacute)));

    // 🚀 (rocket)
    RED_CHECK(keymap.utf16(down, 0xd83d) == ksyms());
    RED_CHECK(keymap.utf16(down, 0xde80) == ksyms(stDown(0x01000000 | 0x1f680)));
    RED_CHECK(keymap.utf16(release, 0xd83d) == ksyms());
    RED_CHECK(keymap.utf16(release, 0xde80) == ksyms(stUp(0x01000000 | 0x1f680)));

    // a (again)
    RED_CHECK(keymap.utf16(down, 'a') == ksyms(stDown('a')));
    RED_CHECK(keymap.utf16(release, 'a') == ksyms(stUp('a')));

    // shift + altgr + 🚀 (rocket)
    RED_CHECK(keymap.scancode(down, Scancode::LShift) == ksyms(stDown(0xffe1)));
    RED_CHECK(keymap.scancode(down | extended, Scancode::LAlt) == ksyms(stDown(0xffea)));
    RED_CHECK(keymap.utf16(down, 0xd83d) == ksyms());
    RED_CHECK(keymap.utf16(down, 0xde80) == ksyms(stUp(0xffea), stDown(0x01000000 | 0x1f680), stDown(0xffea)));
    RED_CHECK(keymap.utf16(release, 0xd83d) == ksyms());
    RED_CHECK(keymap.utf16(release, 0xde80) == ksyms(stUp(0xffea), stUp(0x01000000 | 0x1f680), stDown(0xffea)));
    RED_CHECK(keymap.scancode(release | extended, Scancode::LAlt) == ksyms(stUp(0xffea)));
    RED_CHECK(keymap.scancode(release, Scancode::LShift) == ksyms(stUp(0xffe1)));
}

RED_AUTO_TEST_CASE(TestKeymapSym)
{
    KeymapSymTest keymap(KeymapSym::IsApple::No, KeymapSym::IsUnix::No);

    // a
    RED_CHECK(keymap.scancode(down, Scancode(0x10)) == ksyms(stDown('a')));
    RED_CHECK(keymap.scancode(release, Scancode(0x10)) == ksyms(stUp('a')));

    // shift + a
    RED_CHECK(keymap.scancode(down, Scancode::LShift) == ksyms(stDown(0xffe1)));
    RED_CHECK(keymap.scancode(down, Scancode(0x10)) == ksyms(stDown('A')));
    RED_CHECK(keymap.scancode(release, Scancode(0x10)) == ksyms(stUp('A')));
    RED_CHECK(keymap.scancode(release, Scancode::LShift) == ksyms(stUp(0xffe1)));

    // é
    RED_CHECK(keymap.scancode(down, Scancode(0x03)) == ksyms(stDown(0xe9)));
    RED_CHECK(keymap.scancode(release, Scancode(0x03)) == ksyms(stUp(0xe9)));

    const uint16_t euro = 0x20ac /* € */;

    // altgr + e (remove altgr, push €)
    RED_CHECK(keymap.scancode(down | extended, Scancode::LAlt) == ksyms(stDown(0xffea)));
    RED_CHECK(keymap.scancode(down, Scancode(0x12))
        == ksyms(stUp(0xffea), stDown(0x01000000 | euro), stDown(0xffea)));
    RED_CHECK(keymap.scancode(release, Scancode(0x12))
        == ksyms(stUp(0xffea), stUp(0x01000000 | euro), stDown(0xffea)));
    RED_CHECK(keymap.scancode(release | extended, Scancode::LAlt) == ksyms(stUp(0xffea)));

    // ctrl + alt + e (remove ctrl+alt, push €)
    RED_CHECK(keymap.scancode(down, Scancode::LCtrl) == ksyms(stDown(0xffe3)));
    RED_CHECK(keymap.scancode(down, Scancode::LAlt) == ksyms(stDown(0xffe9)));
    RED_CHECK(keymap.scancode(down, Scancode(0x12))
        == ksyms(stUp(0xffe9), stUp(0xffe3), stDown(0x01000000 | euro), stDown(0xffe9), stDown(0xffe3)));
    RED_CHECK(keymap.scancode(release, Scancode(0x12))
        == ksyms(stUp(0xffe9), stUp(0xffe3), stUp(0x01000000 | euro), stDown(0xffe9), stDown(0xffe3)));
    RED_CHECK(keymap.scancode(release, Scancode::LCtrl) == ksyms(stUp(0xffe3)));
    RED_CHECK(keymap.scancode(release, Scancode::LAlt) == ksyms(stUp(0xffe9)));

    // dead key
    RED_CHECK(keymap.scancode(down, Scancode(0x1a)) == ksyms());
    RED_CHECK(keymap.scancode(release, Scancode(0x1a)) == ksyms());
    RED_CHECK(keymap.scancode(down, Scancode(0x1a)) == ksyms(stDown('^'), stUp('^'), stDown('^')));
    RED_CHECK(keymap.scancode(release, Scancode(0x1a)) == ksyms(stUp('^')));

    // end
    RED_CHECK(keymap.scancode(down | extended, Scancode(0x4F)) == ksyms(stDown(0xff57)));
    RED_CHECK(keymap.scancode(release | extended, Scancode(0x4F)) == ksyms(stUp(0xff57)));

    // ctrl+alt+end -> ctrl+alt+del
    RED_CHECK(keymap.scancode(down, Scancode::LCtrl) == ksyms(stDown(0xffe3)));
    RED_CHECK(keymap.scancode(down, Scancode::LAlt) == ksyms(stDown(0xffe9)));
    RED_CHECK(keymap.scancode(down | extended, Scancode(0x4F)) == ksyms(stDown(0xffff)));
    RED_CHECK(keymap.scancode(release | extended, Scancode(0x4F)) == ksyms(stUp(0xffff)));
    RED_CHECK(keymap.scancode(release, Scancode::LCtrl) == ksyms(stUp(0xffe3)));
    RED_CHECK(keymap.scancode(release, Scancode::LAlt) == ksyms(stUp(0xffe9)));

    // pause
    RED_CHECK(keymap.scancode(down | extended1, Scancode::LCtrl) == ksyms(stDown(0xff13)));
    // 0x0000ff7f (numlocks) is acceptable ?
    RED_CHECK(keymap.scancode(down, Scancode(0x45)) == ksyms(stDown(0x0000ff7f)));
    RED_CHECK(keymap.scancode(release, Scancode(0x45)) == ksyms(stUp(0x0000ff7f)));
    RED_CHECK(keymap.scancode(release | extended1, Scancode::LCtrl) == ksyms(stUp(0xff13)));
}

RED_AUTO_TEST_CASE(TestKeymapSymMacOS)
{
    KeymapSymTest keymap(KeymapSym::IsApple::Yes, KeymapSym::IsUnix::No);

    // a
    RED_CHECK(keymap.scancode(down, Scancode(0x10)) == ksyms(stDown('q')));
    RED_CHECK(keymap.scancode(release, Scancode(0x10)) == ksyms(stUp('q')));

    // shift + a
    RED_CHECK(keymap.scancode(down, Scancode::LShift) == ksyms(stDown(0xffe1)));
    RED_CHECK(keymap.scancode(down, Scancode(0x10)) == ksyms(stDown('Q')));
    RED_CHECK(keymap.scancode(release, Scancode(0x10)) == ksyms(stUp('Q')));
    RED_CHECK(keymap.scancode(release, Scancode::LShift) == ksyms(stUp(0xffe1)));

    // ctrl
    RED_CHECK(keymap.scancode(down, Scancode::LCtrl) == ksyms(stDown(0xffe3)));
    RED_CHECK(keymap.scancode(release, Scancode::LCtrl) == ksyms(stUp(0xffe3)));

    // altgr + e
    RED_CHECK(keymap.scancode(down | extended, Scancode::LAlt) == ksyms(stDown(0xffea)));
    RED_CHECK(keymap.scancode(down, Scancode(0x12)) == ksyms(stDown('e')));
    RED_CHECK(keymap.scancode(release, Scancode(0x12)) == ksyms(stUp('e')));
    RED_CHECK(keymap.scancode(release | extended, Scancode::LAlt) == ksyms(stUp(0xffea)));
}

RED_AUTO_TEST_CASE(TestKeymapSymResetMods)
{
    KeymapSymTest keymap(KeymapSym::IsApple::Yes, KeymapSym::IsUnix::No);

    // shift
    RED_CHECK(keymap.scancode(down, Scancode::LShift) == ksyms(stDown(0xffe1)));
    RED_CHECK(keymap.reset_mods() == ksyms(stUp(0xffe1)));
}
