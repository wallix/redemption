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
#include "test_only/test_framework/compare_collection.hpp"

#include "keyboard/keymap.hpp"


#if !REDEMPTION_UNIT_TEST_FAST_CHECK
# include "utils/sugar/int_to_chars.hpp"

static ut::assertion_result test_comp_decoded(Keymap::DecodedKey a, Keymap::DecodedKey b)
{
    ut::assertion_result ar(true);

    if (REDEMPTION_UNLIKELY(a.keycode != b.keycode || a.uchars[0] != b.uchars[0] || a.uchars[1] != b.uchars[1])) {
        ar = false;

        auto put = [&](std::ostream& out, Keymap::DecodedKey const& x){
            char uchar0[] = " (x)";
            char uchar1[] = " (x)";
            auto to_ascii = [](char* s, KeyLayout2::unicode_t uni) -> char const* {
                if (' ' <= uni && uni <= '~') {
                    s[2] = char(uni);
                    return s;
                }
                return "";
            };

            out << "{.keycode=0x" << int_to_fixed_hexadecimal_upper_zchars(underlying_cast(x.keycode))
                << ", .uchar0=0x" << int_to_fixed_hexadecimal_upper_zchars(x.uchars[0])
                << to_ascii(uchar0, x.uchars[0])
                << ", .uchar1=0x" << int_to_fixed_hexadecimal_upper_zchars(x.uchars[1])
                << to_ascii(uchar1, x.uchars[1])
                << "}";
        };

        auto& out = ar.message().stream();
        out << "[";
        ut::put_data_with_diff(out, a, "!=", b, put);
        out << "]";
    }

    return ar;
}

RED_TEST_DISPATCH_COMPARISON_EQ((), (::Keymap::DecodedKey), (::Keymap::DecodedKey), ::test_comp_decoded)
#endif


RED_AUTO_TEST_CASE(TestKeymap)
{
    Keymap keymap;
    keymap.set_layout(*KeyLayout2::find_layout_by_id(KeyLayout2::KbdId(0x040C)));

    using KbdFlags = Keymap::KbdFlags;
    using Scancode = Keymap::Scancode;

    using DecodedKey = Keymap::DecodedKey;
    using KeyCode = Keymap::KeyCode;

    using KeyModFlags = Keymap::KeyModFlags;
    using KeyMods = Keymap::KeyMods;

    auto event = [&](uint16_t scancode_and_flags){
        keymap.event(KbdFlags(scancode_and_flags & 0xff00u), Scancode(scancode_and_flags));
        return keymap.decoded_key();
    };

    uint16_t release = checked_int(KbdFlags::Release);

    RED_CHECK_EQ(keymap.mods().as_uint(), 0);
    RED_CHECK_EQ(event(0x10 /*a*/), (DecodedKey{KeyCode(0x10), {'a'}}));
    RED_CHECK_EQ(event(0x36 /*right shift*/), (DecodedKey{KeyCode(0x36), {}}));
    RED_CHECK_EQ(keymap.mods().as_uint(), KeyModFlags(KeyMods::RShift).as_uint());
    RED_CHECK_EQ(event(0x10 /*a*/), (DecodedKey{KeyCode(0x10), {'A'}}));
    RED_CHECK(!keymap.is_ctrl_pressed());
    RED_CHECK_EQ(event(0x11d /*right ctrl*/), (DecodedKey{KeyCode(0x80 | 0x1d), {}}));
    RED_CHECK_EQ(keymap.mods().as_uint(), (KeyMods::RShift | KeyMods::RCtrl).as_uint());
    RED_CHECK(keymap.is_ctrl_pressed());
    RED_CHECK_EQ(event(0x10 /*a*/), (DecodedKey{KeyCode(0x10), {}}));
    RED_CHECK_EQ(event(release | 0x11d /*right ctrl*/), (DecodedKey{KeyCode(0x80 | 0x1d), {}}));
    RED_CHECK_EQ(keymap.mods().as_uint(), KeyModFlags(KeyMods::RShift).as_uint());
    RED_CHECK_EQ(event(0x10 /*a*/), (DecodedKey{KeyCode(0x10), {'A'}}));
    RED_CHECK_EQ(event(0x02 /*&*/), (DecodedKey{KeyCode(0x02), {'1'}}));
    RED_CHECK_EQ(event(0x2a /*left shift*/), (DecodedKey{KeyCode(0x2a), {}}));
    RED_CHECK_EQ(keymap.mods().as_uint(), (KeyMods::RShift | KeyMods::LShift).as_uint());
    RED_CHECK_EQ(event(release | 0x36 /*right shift*/), (DecodedKey{KeyCode(0x36), {}}));
    RED_CHECK_EQ(keymap.mods().as_uint(), KeyModFlags(KeyMods::LShift).as_uint());
    RED_CHECK_EQ(event(release | 0x2a /*left shift*/), (DecodedKey{KeyCode(0x2a), {}}));
    RED_CHECK_EQ(keymap.mods().as_uint(), 0);
    RED_CHECK_EQ(event(0x10 /*a*/), (DecodedKey{KeyCode(0x10), {'a'}}));
    RED_CHECK_EQ(event(0x02 /*&*/), (DecodedKey{KeyCode(0x02), {'&'}}));
    RED_CHECK_EQ(event(0x14b /*left*/), (DecodedKey{KeyCode(0x80 | 0x4b), {}}));
    RED_CHECK_EQ(keymap.mods().as_uint(), 0);

    // dead keys

    RED_CHECK_EQ(event(0x1a /*^*/), (DecodedKey{KeyCode(0x1a), {}}));
    RED_CHECK_EQ(event(0x1a /*^*/), (DecodedKey{KeyCode(0x1a), {'^', '^'}}));

    RED_CHECK_EQ(event(0x1a /*^*/), (DecodedKey{KeyCode(0x1a), {}}));
    RED_CHECK_EQ(event(0x22 /*g*/), (DecodedKey{KeyCode(0x22), {'^', 'g'}}));

    RED_CHECK_EQ(event(0x1a /*^*/), (DecodedKey{KeyCode(0x1a), {}}));
    RED_CHECK_EQ(event(0x12 /*e*/), (DecodedKey{KeyCode(0x12), {0xEA /*ê*/}}));

// }
//
// RED_AUTO_TEST_CASE(TestDeadKeys)
// {
//     Keymap keymap;
//     const int layout = 0x040C;
//     keymap.init_layout(layout);
//     RED_CHECK_EQUAL(0, keymap.nb_char_available());
//
//     Keymap::DecodedKey decoded_keys;
//     bool    ctrl_alt_delete;
//
//     decoded_keys = keymap.event(0x0000, 0x1A, ctrl_alt_delete); // '^' down dead key
//     RED_CHECK_EQUAL(0, decoded_keys.count);
//     decoded_keys = keymap.event(0xC000, 0x1A, ctrl_alt_delete); // '^' up dead key
//     RED_CHECK_EQUAL(0, decoded_keys.count);
//     RED_CHECK_EQUAL(0, keymap.nb_char_available());
//
//     decoded_keys = keymap.event(0x0000, 0x12, ctrl_alt_delete); // 'e'
//     RED_CHECK_EQUAL(1, decoded_keys.count);
//     RED_CHECK_EQUAL(0xEA, decoded_keys.uchars[0]);
//     decoded_keys = keymap.event(0xC000, 0x12, ctrl_alt_delete); // 'e'
//     RED_CHECK_EQUAL(0, decoded_keys.count);
//     RED_CHECK_EQUAL(1, keymap.nb_char_available());
//     RED_CHECK_EQUAL(0xEA, keymap.get_char()); // ê
//
//     // consecutive deadkey
//     decoded_keys = keymap.event(0x0000, 0x1A, ctrl_alt_delete); // '^' down dead key
//     RED_CHECK_EQUAL(0, decoded_keys.count);
//     decoded_keys = keymap.event(0xC000, 0x1A, ctrl_alt_delete); // '^' up dead key
//     RED_CHECK_EQUAL(0, decoded_keys.count);
//     decoded_keys = keymap.event(0x0000, 0x1A, ctrl_alt_delete); // '^' down dead key
//     RED_CHECK_EQUAL(2, decoded_keys.count);
//     RED_CHECK_EQUAL('^', decoded_keys.uchars[0]);
//     RED_CHECK_EQUAL('^', decoded_keys.uchars[1]);
//     decoded_keys = keymap.event(0xC000, 0x1A, ctrl_alt_delete); // '^' up dead key
//     RED_CHECK_EQUAL(0, decoded_keys.count);
//     RED_CHECK_EQUAL(2, keymap.nb_char_available());
//     RED_CHECK_EQUAL(0x5E, keymap.get_char()); // ^
//     RED_CHECK_EQUAL(0x5E, keymap.get_char()); // ^
//
//     // deadkey with non-comboable printed character
//     decoded_keys = keymap.event(0x0000, 0x1A, ctrl_alt_delete); // '^' down dead key
//     decoded_keys = keymap.event(0xC000, 0x1A, ctrl_alt_delete); // '^' up dead key
//     decoded_keys = keymap.event(0x0000, 0x11, ctrl_alt_delete); // 'z'
//     RED_CHECK_EQUAL(2, decoded_keys.count);
//     RED_CHECK_EQUAL('^', decoded_keys.uchars[0]);
//     RED_CHECK_EQUAL('z', decoded_keys.uchars[1]);
//     decoded_keys = keymap.event(0xC000, 0x11, ctrl_alt_delete); // 'z'
//     RED_CHECK_EQUAL(0, decoded_keys.count);
//     RED_CHECK_EQUAL(2, keymap.nb_char_available());
//     RED_CHECK_EQUAL(0x5E, keymap.get_char()); // ^
//     RED_CHECK_EQUAL(0x7A, keymap.get_char()); // z
//
//     // deadkey with special key
//     decoded_keys = keymap.event(0x0000, 0x1A, ctrl_alt_delete); // '^' down dead key
//     decoded_keys = keymap.event(0xC000, 0x1A, ctrl_alt_delete); // '^' up dead key
//     decoded_keys = keymap.event(0x0000, 0x0F, ctrl_alt_delete); // TAB
//     decoded_keys = keymap.event(0xC000, 0x0F, ctrl_alt_delete); // TAB
//     RED_CHECK_EQUAL(0, keymap.nb_char_available());
//     RED_CHECK_EQUAL(1, keymap.nb_kevent_available());
//     RED_CHECK_EQUAL(0x02, keymap.get_kevent()); // KEVENT_TAB
//     // the deadkey is still pending
//     decoded_keys = keymap.event(0x0000, 0x12, ctrl_alt_delete); // 'e'
//     decoded_keys = keymap.event(0xC000, 0x12, ctrl_alt_delete); // 'e'
//     RED_CHECK_EQUAL(1, keymap.nb_char_available());
//     RED_CHECK_EQUAL(0xEA, keymap.get_char()); // ê
//
//     // deadkey with Backspace deletes the deadkey
//     decoded_keys = keymap.event(0x0000, 0x1A, ctrl_alt_delete); // '^' down dead key
//     decoded_keys = keymap.event(0xC000, 0x1A, ctrl_alt_delete); // '^' up dead key
//     decoded_keys = keymap.event(0x0000, 0x0E, ctrl_alt_delete); // BACKSPACE
//     decoded_keys = keymap.event(0xC000, 0x0E, ctrl_alt_delete); // BACKSPACE
//     RED_CHECK_EQUAL(0, keymap.nb_char_available());
//     RED_CHECK_EQUAL(0, keymap.nb_kevent_available());
//     decoded_keys = keymap.event(0x0000, 0x12, ctrl_alt_delete); // 'e'
//     decoded_keys = keymap.event(0xC000, 0x12, ctrl_alt_delete); // 'e'
//     RED_CHECK_EQUAL(1, keymap.nb_char_available());
//     RED_CHECK_EQUAL(0x65, keymap.get_char()); // e
//
//
//     // Autorepeat
//     // RDP_INPUT_SCANCODE time=538966980 flags=4000 param1=001d param2=0000
//     // RDP_INPUT_SCANCODE time=538966980 flags=4100 param1=0038 param2=0000
//
//     // RDP_INPUT_SCANCODE time=538967136 flags=c000 param1=001d param2=0000
//     // altgr up
//     // RDP_INPUT_SCANCODE time=538967136 flags=c100 param1=0038 param2=0000
//
//     // alt down autorepeat
//     // RDP_INPUT_SCANCODE time=539107646 flags=0000 param1=0038 param2=0000
//     // alt down autorepeat
//     // RDP_INPUT_SCANCODE time=539108270 flags=4000 param1=0038 param2=0000
//     // RDP_INPUT_SCANCODE time=539108301 flags=4000 param1=0038 param2=0000
//     // RDP_INPUT_SCANCODE time=539108333 flags=4000 param1=0038 param2=0000
//     // RDP_INPUT_SCANCODE time=539108364 flags=4000 param1=0038 param2=0000
//     // RDP_INPUT_SCANCODE time=539108411 flags=4000 param1=0038 param2=0000
//     // alt up
//     // RDP_INPUT_SCANCODE time=539108426 flags=c000 param1=0038 param2=0000
//
//
//     // CTRL+ALT+@ (first line, 0 key)
//     // RDP_INPUT_SCANCODE time=539317998 flags=0000 param1=0038 param2=0000
//     // RDP_INPUT_SCANCODE time=539318014 flags=0000 param1=001d param2=0000
//     // RDP_INPUT_SCANCODE time=539318045 flags=0000 param1=000b param2=0000
//     // RDP_INPUT_SCANCODE time=539318154 flags=8000 param1=000b param2=0000
//     // RDP_INPUT_SCANCODE time=539318279 flags=c000 param1=0038 param2=0000
//     // RDP_INPUT_SCANCODE time=539318279 flags=c000 param1=001d param2=0000
//
//     // AltGr+@ (first line, 0 key)
//     // RDP_INPUT_SCANCODE time=539432674 flags=0000 param1=001d param2=0000
//     // RDP_INPUT_SCANCODE time=539432674 flags=0100 param1=0038 param2=0000
//     // RDP_INPUT_SCANCODE time=539432955 flags=0000 param1=000b param2=0000
//     // RDP_INPUT_SCANCODE time=539433049 flags=c000 param1=001d param2=0000
//     // RDP_INPUT_SCANCODE time=539433049 flags=c100 param1=0038 param2=0000
//     // RDP_INPUT_SCANCODE time=539433049 flags=8000 param1=000b param2=0000
//
//     // abcd (mapping fr) sans relacher aucune touche puis on relache dans l'ordre inverse
//     // RDP_INPUT_SCANCODE time=539592794 flags=0000 param1=0010 param2=0000
//     // RDP_INPUT_SCANCODE time=539593090 flags=0000 param1=0030 param2=0000
//     // RDP_INPUT_SCANCODE time=539593527 flags=0000 param1=002e param2=0000
//     // RDP_INPUT_SCANCODE time=539593917 flags=0000 param1=0020 param2=0000
//     // RDP_INPUT_SCANCODE time=539594182 flags=8000 param1=002e param2=0000
//     // RDP_INPUT_SCANCODE time=539594182 flags=8000 param1=0020 param2=0000
//     // RDP_INPUT_SCANCODE time=539594198 flags=8000 param1=0030 param2=0000
//     // RDP_INPUT_SCANCODE time=539594198 flags=8000 param1=0010 param2=0000
//
//     // Windows key down then up
//     // RDP_INPUT_SCANCODE time=539712088 flags=0100 param1=005b param2=0000
//     // RDP_INPUT_SCANCODE time=539712150 flags=8100 param1=005b param2=0000
//
//     // Windows key down, autorepeat, then up
//     // RDP_INPUT_SCANCODE time=539790977 flags=0100 param1=005b param2=0000
//     // RDP_INPUT_SCANCODE time=539791430 flags=0100 param1=005b param2=0000
//     // RDP_INPUT_SCANCODE time=539791461 flags=0100 param1=005b param2=0000
//     // RDP_INPUT_SCANCODE time=539791477 flags=0100 param1=005b param2=0000
//     // RDP_INPUT_SCANCODE time=539791508 flags=0100 param1=005b param2=0000
//     // RDP_INPUT_SCANCODE time=539791539 flags=0100 param1=005b param2=0000
//     // RDP_INPUT_SCANCODE time=539791570 flags=0100 param1=005b param2=0000
//     // RDP_INPUT_SCANCODE time=539791601 flags=0100 param1=005b param2=0000
//     // RDP_INPUT_SCANCODE time=539791648 flags=0100 param1=005b param2=0000
//     // RDP_INPUT_SCANCODE time=539791679 flags=0100 param1=005b param2=0000
//     // RDP_INPUT_SCANCODE time=539791711 flags=0100 param1=005b param2=0000
//     // RDP_INPUT_SCANCODE time=539791742 flags=0100 param1=005b param2=0000
//     // RDP_INPUT_SCANCODE time=539791773 flags=0100 param1=005b param2=0000
//     // RDP_INPUT_SCANCODE time=539791804 flags=0100 param1=005b param2=0000
//     // RDP_INPUT_SCANCODE time=539791820 flags=8100 param1=005b param2=0000
//
//     // dead key ^, key 'A' (french mapping)
//     // RDP_INPUT_SCANCODE time=539931659 flags=0000 param1=001a param2=0000
//     // RDP_INPUT_SCANCODE time=539931659 flags=8000 param1=001a param2=0000
//     // RDP_INPUT_SCANCODE time=539931659 flags=0000 param1=0010 param2=0000
//     // RDP_INPUT_SCANCODE time=539931690 flags=8000 param1=0010 param2=0000
//
//     // right CTRL
//     // RDP_INPUT_SCANCODE time=540142806 flags=0100 param1=001d param2=0000
//     // RDP_INPUT_SCANCODE time=540142916 flags=c100 param1=001d param2=0000
//
//     // left CTRL
//     // RDP_INPUT_SCANCODE time=540142806 flags=0000 param1=001d param2=0000
//     // RDP_INPUT_SCANCODE time=540142916 flags=c000 param1=001d param2=0000
//
//     // 'Menu' Key Up and Down
//     // RDP_INPUT_SCANCODE time=540250369 flags=0100 param1=005d param2=0000
//     // RDP_INPUT_SCANCODE time=540250884 flags=8100 param1=005d param2=0000
//
//     // Numlock down autorepeat up
//     // RDP_INPUT_SCANCODE time=540394124 flags=0000 param1=0045 param2=0000
//     // RDP_INPUT_SCANCODE time=540394608 flags=4000 param1=0045 param2=0000
//     // RDP_INPUT_SCANCODE time=540394623 flags=4000 param1=0045 param2=0000
//     // RDP_INPUT_SCANCODE time=540394654 flags=4000 param1=0045 param2=0000
//     // RDP_INPUT_SCANCODE time=540394686 flags=4000 param1=0045 param2=0000
//     // RDP_INPUT_SCANCODE time=540394717 flags=4000 param1=0045 param2=0000
//     // RDP_INPUT_SCANCODE time=540394732 flags=4000 param1=0045 param2=0000
//     // RDP_INPUT_SCANCODE time=540394764 flags=4000 param1=0045 param2=0000
//     // RDP_INPUT_SCANCODE time=540394795 flags=4000 param1=0045 param2=0000
//     // RDP_INPUT_SCANCODE time=540394826 flags=4000 param1=0045 param2=0000
//     // RDP_INPUT_SCANCODE time=540394857 flags=4000 param1=0045 param2=0000
//     // RDP_INPUT_SCANCODE time=540394873 flags=4000 param1=0045 param2=0000
//     // RDP_INPUT_SCANCODE time=540394904 flags=4000 param1=0045 param2=0000
//     // RDP_INPUT_SCANCODE time=540394935 flags=4000 param1=0045 param2=0000
//     // RDP_INPUT_SCANCODE time=540394966 flags=4000 param1=0045 param2=0000
//     // RDP_INPUT_SCANCODE time=540394982 flags=4000 param1=0045 param2=0000
//     // RDP_INPUT_SCANCODE time=540395013 flags=4000 param1=0045 param2=0000
//     // RDP_INPUT_SCANCODE time=540395044 flags=4000 param1=0045 param2=0000
//     // RDP_INPUT_SCANCODE time=540395076 flags=4000 param1=0045 param2=0000
//     // RDP_INPUT_SCANCODE time=540395091 flags=4000 param1=0045 param2=0000
//     // RDP_INPUT_SCANCODE time=540395122 flags=4000 param1=0045 param2=0000
//     // RDP_INPUT_SCANCODE time=540395154 flags=4000 param1=0045 param2=0000
//     // RDP_INPUT_SCANCODE time=540395185 flags=4000 param1=0045 param2=0000
//     // RDP_INPUT_SCANCODE time=540395200 flags=4000 param1=0045 param2=0000
//     // RDP_INPUT_SCANCODE time=540395232 flags=4000 param1=0045 param2=0000
//     // RDP_INPUT_SCANCODE time=540395232 flags=c000 param1=0045 param2=0000
//
//     // Numlock down up
//     // RDP_INPUT_SCANCODE time=540349898 flags=0000 param1=0045 param2=0000
//     // RDP_INPUT_SCANCODE time=540349991 flags=c000 param1=0045 param2=0000
//
//     // ESC Down Autorepeat Up
//     // RDP_INPUT_SCANCODE time=540563541 flags=0000 param1=0001 param2=0000
//     // RDP_INPUT_SCANCODE time=540564040 flags=0000 param1=0001 param2=0000
//     // RDP_INPUT_SCANCODE time=540564072 flags=0000 param1=0001 param2=0000
//     // RDP_INPUT_SCANCODE time=540564103 flags=0000 param1=0001 param2=0000
//     // RDP_INPUT_SCANCODE time=540564118 flags=0000 param1=0001 param2=0000
//     // RDP_INPUT_SCANCODE time=540564150 flags=0000 param1=0001 param2=0000
//     // RDP_INPUT_SCANCODE time=540564181 flags=0000 param1=0001 param2=0000
//     // RDP_INPUT_SCANCODE time=540564212 flags=0000 param1=0001 param2=0000
//     // RDP_INPUT_SCANCODE time=540564243 flags=0000 param1=0001 param2=0000
//     // RDP_INPUT_SCANCODE time=540564274 flags=0000 param1=0001 param2=0000
//     // RDP_INPUT_SCANCODE time=540564290 flags=0000 param1=0001 param2=0000
//     // RDP_INPUT_SCANCODE time=540564290 flags=8000 param1=0001 param2=0000
//
// }
//
//
// RED_AUTO_TEST_CASE(TestKeymapBuffer)
// {
//     Keymap keymap;
//     keymap.init_layout(0x040C);
//
//     Keymap::DecodedKey decoded_keys;
//     bool    ctrl_alt_delete;
//
//     RED_CHECK_EQUAL(0, keymap.nb_char_available());
//     decoded_keys = keymap.event(0, 0x10, ctrl_alt_delete);
//     RED_CHECK_EQUAL(1, keymap.nb_char_available());
//     RED_CHECK_EQUAL('a', keymap.get_char());
//     decoded_keys = keymap.event(0xc000, 0x10, ctrl_alt_delete); // up
//
//     decoded_keys = keymap.event(0x0100, 0x35, ctrl_alt_delete); // '/' on keypad
//     RED_CHECK_EQUAL(1, keymap.nb_char_available());
//     decoded_keys = keymap.event(0xc100, 0x35, ctrl_alt_delete); // '/' on keypad
//
//     RED_CHECK_EQUAL(1, keymap.nb_char_available());
//     RED_CHECK_EQUAL('/', keymap.get_char());
//     RED_CHECK_EQUAL(0, keymap.nb_char_available());
//
//     // saturating buffer
//     for(size_t i = 0; i < 10 ; i++){
//         decoded_keys = keymap.event(0, 0x10, ctrl_alt_delete);
//         RED_CHECK_EQUAL(i+1, keymap.nb_char_available());
//         decoded_keys = keymap.event(0xc000, 0x10, ctrl_alt_delete); // up
//     }
//
//     // saturating buffer
//     for(size_t i = 10; i < 20 ; i++){
//         decoded_keys = keymap.event(0, 0x11, ctrl_alt_delete);
//         RED_CHECK_EQUAL(i+1, keymap.nb_char_available());
//         decoded_keys = keymap.event(0xc000, 0x11, ctrl_alt_delete); // up
//     }
//
//     // buffer saturated
//     decoded_keys = keymap.event(0, 0x10, ctrl_alt_delete);
//     RED_CHECK_EQUAL(20, keymap.nb_char_available());
//     decoded_keys = keymap.event(0xc000, 0x10, ctrl_alt_delete); // up
//
//     // Reading back buffer
//     for(size_t i = 0; i < 10 ; i++){
//         RED_CHECK_EQUAL('a', keymap.get_char());
//         RED_CHECK_EQUAL(19 - i, keymap.nb_char_available());
//     }
//
//     // Reading back buffer
//     for(size_t i = 10; i < 20 ; i++){
//         RED_CHECK_EQUAL('z', keymap.get_char());
//         RED_CHECK_EQUAL(19 - i, keymap.nb_char_available());
//     }
//
//     RED_CHECK_EQUAL(0, keymap.nb_char_available());
//
//     // down arrow
//     decoded_keys = keymap.event(0x0100, 0x50, ctrl_alt_delete);
//     RED_CHECK_EQUAL(0, keymap.nb_char_available());
//     decoded_keys = keymap.event(0xc100, 0x50, ctrl_alt_delete);
//
//     RED_CHECK_EQUAL(0, decoded_keys.count);
// }
//
//
// RED_AUTO_TEST_CASE(TestKeyPad)
// {
//     Keymap keymap;
//     const int layout = 0x040C;
//     keymap.init_layout(layout);
//     // all lock keys are supposed to be inactive at this, point
//     RED_CHECK_EQUAL(0, keymap.key_flags);
//
//     Keymap::DecodedKey decoded_keys;
//     bool    ctrl_alt_delete;
//
//     decoded_keys = keymap.event(0x0000, 0x45, ctrl_alt_delete); // activate numlock
//     RED_CHECK_EQUAL(0, decoded_keys.count);
//     RED_CHECK_EQUAL(2, keymap.key_flags);
//     decoded_keys = keymap.event(0x0000, 0x45, ctrl_alt_delete); // desactivate numlock
//     RED_CHECK_EQUAL(0, decoded_keys.count);
//     RED_CHECK_EQUAL(0, keymap.key_flags);
//
//     decoded_keys = keymap.event(0x0000, 0x45, ctrl_alt_delete); // activate numlock
//     RED_CHECK_EQUAL(0, decoded_keys.count);
//     decoded_keys = keymap.event(0, 0x47, ctrl_alt_delete); // keypad '7' character
//     RED_CHECK_EQUAL(1, decoded_keys.count);
//     RED_CHECK_EQUAL('7', decoded_keys.uchars[0]);
//     RED_CHECK_EQUAL('7', keymap.get_char());
//
//     decoded_keys = keymap.event(0x0000, 0x45, ctrl_alt_delete); // desactivate numlock
//     RED_CHECK_EQUAL(0, decoded_keys.count);
//     decoded_keys = keymap.event(0, 0x47, ctrl_alt_delete); // keypad home character
//     RED_CHECK_EQUAL(1, decoded_keys.count);
//     RED_CHECK_EQUAL(0x2196, decoded_keys.uchars[0]);
//     RED_CHECK_EQUAL(0, keymap.get_char());
}
