/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010-2013
   Author(s): Christophe Grosjean, Meng Tan

   Unit test to keymap object
   Using lib boost functions, some tests need to be added

*/


#define RED_TEST_MODULE TestKeymap
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "keyboard/keymap2.hpp"


RED_AUTO_TEST_CASE(TestKeymap)
{
    Keymap2 keymap;
    const int layout = 0x040C;
    keymap.init_layout(layout);

    Keymap2::DecodedKeys decoded_keys;
    bool    ctrl_alt_delete;

    RED_CHECK_EQUAL(false, keymap.is_shift_pressed());

    uint16_t keyboardFlags = 0 ; // key is not extended, key was up, key goes down
    uint16_t keyCode = 54 ; // key is right shift
    decoded_keys = keymap.event(keyboardFlags, keyCode, ctrl_alt_delete);

    RED_CHECK_EQUAL(0, decoded_keys.count);
    RED_CHECK_EQUAL(true, keymap.is_shift_pressed());
    RED_CHECK_EQUAL(false, keymap.is_left_shift_pressed());
    RED_CHECK_EQUAL(true, keymap.is_right_shift_pressed());

    keyboardFlags = 0 ; // key is not extended, key was up, key goes down
    keyCode = 16 ; // key is 'A'
    decoded_keys = keymap.event(keyboardFlags, keyCode, ctrl_alt_delete);

    RED_CHECK_EQUAL(1, decoded_keys.count);
    RED_CHECK_EQUAL('A', decoded_keys.uchars[0]);
    RED_CHECK_EQUAL(true, keymap.is_shift_pressed());
    RED_CHECK_EQUAL(false, keymap.is_left_shift_pressed());
    RED_CHECK_EQUAL(true, keymap.is_right_shift_pressed());

    uint32_t key = keymap.get_char();
    RED_CHECK_EQUAL('A', key);

    keyboardFlags = keymap.KBDFLAGS_DOWN|keymap.KBDFLAGS_RELEASE ; // key is not extended, key was down, key goes up
    keyCode = 54 ; // key is right shift
    decoded_keys = keymap.event(keyboardFlags, keyCode, ctrl_alt_delete);

    RED_CHECK_EQUAL(0, decoded_keys.count);
    RED_CHECK_EQUAL(false, keymap.is_shift_pressed());
    RED_CHECK_EQUAL(false, keymap.is_left_shift_pressed());
    RED_CHECK_EQUAL(false, keymap.is_right_shift_pressed());

    // shift was released, but not A (last char down goes 'a' for autorepeat)

    keyboardFlags = keymap.KBDFLAGS_DOWN|keymap.KBDFLAGS_RELEASE ; // key is not extended, key was down, key goes up
    keyCode = 16 ; // key is 'A'
    decoded_keys = keymap.event(keyboardFlags, keyCode, ctrl_alt_delete);

    keyboardFlags = 0 ; // key is not extended, key was up, key goes down
    keyCode = 16 ; // key is 'A'
    decoded_keys = keymap.event(keyboardFlags, keyCode, ctrl_alt_delete);
    key = keymap.get_char();
    RED_CHECK_EQUAL('a', key);

    // CAPSLOCK Down
    // RDP_INPUT_SCANCODE time=538384316 flags=0000 param1=003a param2=0000
    RED_CHECK_EQUAL(false, keymap.is_caps_locked());
    decoded_keys = keymap.event(0, 0x3A, ctrl_alt_delete);
    RED_CHECK_EQUAL(true, keymap.is_caps_locked());

    // CAPSLOCK Up
    // RDP_INPUT_SCANCODE time=538384894 flags=c000 param1=003a param2=0000
    decoded_keys = keymap.event(0xc000, 0x3A, ctrl_alt_delete);
    RED_CHECK_EQUAL(true, keymap.is_caps_locked());

    // Now I hit the 'A' key on french keyboard
    decoded_keys = keymap.event(0, 0x10, ctrl_alt_delete);
    key = keymap.get_char();
    RED_CHECK_EQUAL('A', key);

    decoded_keys = keymap.event(0xc000, 0x10, ctrl_alt_delete); // A up

    RED_CHECK_EQUAL(true, keymap.is_caps_locked());
    decoded_keys = keymap.event(0, 0x02, ctrl_alt_delete);
    key = keymap.get_char();
    RED_CHECK_EQUAL('1', key);

    // left shift down
    decoded_keys = keymap.event(0, 0x36, ctrl_alt_delete);

    // left shift up
    decoded_keys = keymap.event(0xc000, 0x36, ctrl_alt_delete);

    // right shift down
    decoded_keys = keymap.event(0, 0x2A, ctrl_alt_delete);

    // right shift up
    decoded_keys = keymap.event(0xc000, 0x2A, ctrl_alt_delete);

    // CAPSLOCK Down
    // RDP_INPUT_SCANCODE time=538384316 flags=0000 param1=003a param2=0000
    RED_CHECK_EQUAL(true, keymap.is_caps_locked());
    decoded_keys = keymap.event(0, 0x3A, ctrl_alt_delete);
    RED_CHECK_EQUAL(false, keymap.is_caps_locked());
    decoded_keys = keymap.event(0xC000, 0x3A, ctrl_alt_delete); // capslock up
    RED_CHECK_EQUAL(false, keymap.is_caps_locked());

    // Now I hit the 'A' key on french keyboard
    decoded_keys = keymap.event(0, 0x10, ctrl_alt_delete);
    key = keymap.get_char();
    RED_CHECK_EQUAL('a', key);
    decoded_keys = keymap.event(0xc000, 0x10, ctrl_alt_delete); // up

    decoded_keys = keymap.event(0, 0x02, ctrl_alt_delete);
    key = keymap.get_char();
    RED_CHECK_EQUAL('&', key);
    decoded_keys = keymap.event(0xc000, 0x02, ctrl_alt_delete);

    // left shift down
    decoded_keys = keymap.event(0, 54, ctrl_alt_delete);
    decoded_keys = keymap.event(0, 0x02, ctrl_alt_delete);
    key = keymap.get_char();
    RED_CHECK_EQUAL('1', key);

    // left shift up
    decoded_keys = keymap.event(0xc000, 54, ctrl_alt_delete);
    decoded_keys = keymap.event(0, 0x02, ctrl_alt_delete);
    key = keymap.get_char();
    RED_CHECK_EQUAL('&', key);

    RED_CHECK_EQUAL(false, keymap.is_caps_locked());
    decoded_keys = keymap.event(0, 0x3A, ctrl_alt_delete);
    RED_CHECK_EQUAL(true, keymap.is_caps_locked());
    decoded_keys = keymap.event(0xC000, 0x3A, ctrl_alt_delete); // capslock up
    RED_CHECK_EQUAL(true, keymap.is_caps_locked());


    // Now I hit the 'A' key on french keyboard
    decoded_keys = keymap.event(0, 0x10, ctrl_alt_delete);
    RED_CHECK_EQUAL('A', keymap.get_char());
    decoded_keys = keymap.event(0xc000, 0x10, ctrl_alt_delete); // up

    decoded_keys = keymap.event(0, 0x02, ctrl_alt_delete);
    RED_CHECK_EQUAL('1', keymap.get_char());
    decoded_keys = keymap.event(0xc000, 0x02, ctrl_alt_delete);

    // left shift down
    decoded_keys = keymap.event(0, 54, ctrl_alt_delete);
    decoded_keys = keymap.event(0, 0x02, ctrl_alt_delete);
    RED_CHECK_EQUAL('&', keymap.get_char());

    // left shift up
    decoded_keys = keymap.event(0xc000, 54, ctrl_alt_delete);
    decoded_keys = keymap.event(0, 0x02, ctrl_alt_delete);
    RED_CHECK_EQUAL('1', keymap.get_char());

    // altgr down
    // RDP_INPUT_SCANCODE time=538966481 flags=0000 param1=001d param2=0000 -> CTRL
    // altgr down autorepeat
    // RDP_INPUT_SCANCODE time=538966481 flags=0100 param1=0038 param2=0000 -> ALT
    decoded_keys = keymap.event(0x0000, 0x1d, ctrl_alt_delete); // CTRL
    RED_CHECK_EQUAL(true, keymap.is_ctrl_pressed());
    decoded_keys = keymap.event(0x0100, 0x38, ctrl_alt_delete); // ALT Right
    RED_CHECK_EQUAL(true, keymap.is_right_alt_pressed());
    decoded_keys = keymap.event(0x0000, 0x04, ctrl_alt_delete); // Sharp
    RED_CHECK_EQUAL(1, keymap.nb_char_available());
    RED_CHECK_EQUAL('#', keymap.get_char());

    decoded_keys = keymap.event(0xC000, 0x03, ctrl_alt_delete); // Tilde
    decoded_keys = keymap.event(0xC100, 0x38, ctrl_alt_delete); // ALT Right
    RED_CHECK_EQUAL(false, keymap.is_right_alt_pressed());
    decoded_keys = keymap.event(0xC000, 0x1d, ctrl_alt_delete); // CTRL
    RED_CHECK_EQUAL(false, keymap.is_ctrl_pressed());


    decoded_keys = keymap.event(0x0100, 0x35, ctrl_alt_delete); // '/' on keypad
    RED_CHECK_EQUAL(1, keymap.nb_char_available());
    key = keymap.get_char();
    RED_CHECK_EQUAL('/', key);

    decoded_keys = keymap.event(0xC000, 0x03, ctrl_alt_delete); // Tilde
    decoded_keys = keymap.event(0xC100, 0x38, ctrl_alt_delete); // ALT Right
    RED_CHECK_EQUAL(false, keymap.is_right_alt_pressed());
    decoded_keys = keymap.event(0xC000, 0x1d, ctrl_alt_delete); // CTRL
    RED_CHECK_EQUAL(false, keymap.is_ctrl_pressed());

    decoded_keys = keymap.event(0x0100, 0x35, ctrl_alt_delete); // '/' on keypad
    RED_CHECK_EQUAL(1, keymap.nb_char_available());
    key = keymap.get_char();
    RED_CHECK_EQUAL('/', key);
    RED_CHECK_EQUAL(0, keymap.nb_char_available());
}

RED_AUTO_TEST_CASE(TestDeadKeys)
{
    Keymap2 keymap;
    const int layout = 0x040C;
    keymap.init_layout(layout);
    RED_CHECK_EQUAL(0, keymap.nb_char_available());

    Keymap2::DecodedKeys decoded_keys;
    bool    ctrl_alt_delete;

    decoded_keys = keymap.event(0x0000, 0x1A, ctrl_alt_delete); // '^' down dead key
    RED_CHECK_EQUAL(0, decoded_keys.count);
    decoded_keys = keymap.event(0xC000, 0x1A, ctrl_alt_delete); // '^' up dead key
    RED_CHECK_EQUAL(0, decoded_keys.count);
    RED_CHECK_EQUAL(0, keymap.nb_char_available());

    decoded_keys = keymap.event(0x0000, 0x12, ctrl_alt_delete); // 'e'
    RED_CHECK_EQUAL(1, decoded_keys.count);
    RED_CHECK_EQUAL(0xEA, decoded_keys.uchars[0]);
    decoded_keys = keymap.event(0xC000, 0x12, ctrl_alt_delete); // 'e'
    RED_CHECK_EQUAL(0, decoded_keys.count);
    RED_CHECK_EQUAL(1, keymap.nb_char_available());
    RED_CHECK_EQUAL(0xEA, keymap.get_char()); // ê

    // consecutive deadkey
    decoded_keys = keymap.event(0x0000, 0x1A, ctrl_alt_delete); // '^' down dead key
    RED_CHECK_EQUAL(0, decoded_keys.count);
    decoded_keys = keymap.event(0xC000, 0x1A, ctrl_alt_delete); // '^' up dead key
    RED_CHECK_EQUAL(0, decoded_keys.count);
    decoded_keys = keymap.event(0x0000, 0x1A, ctrl_alt_delete); // '^' down dead key
    RED_CHECK_EQUAL(2, decoded_keys.count);
    RED_CHECK_EQUAL('^', decoded_keys.uchars[0]);
    RED_CHECK_EQUAL('^', decoded_keys.uchars[1]);
    decoded_keys = keymap.event(0xC000, 0x1A, ctrl_alt_delete); // '^' up dead key
    RED_CHECK_EQUAL(0, decoded_keys.count);
    RED_CHECK_EQUAL(2, keymap.nb_char_available());
    RED_CHECK_EQUAL(0x5E, keymap.get_char()); // ^
    RED_CHECK_EQUAL(0x5E, keymap.get_char()); // ^

    // deadkey with non-comboable printed character
    decoded_keys = keymap.event(0x0000, 0x1A, ctrl_alt_delete); // '^' down dead key
    decoded_keys = keymap.event(0xC000, 0x1A, ctrl_alt_delete); // '^' up dead key
    decoded_keys = keymap.event(0x0000, 0x11, ctrl_alt_delete); // 'z'
    RED_CHECK_EQUAL(2, decoded_keys.count);
    RED_CHECK_EQUAL('^', decoded_keys.uchars[0]);
    RED_CHECK_EQUAL('z', decoded_keys.uchars[1]);
    decoded_keys = keymap.event(0xC000, 0x11, ctrl_alt_delete); // 'z'
    RED_CHECK_EQUAL(0, decoded_keys.count);
    RED_CHECK_EQUAL(2, keymap.nb_char_available());
    RED_CHECK_EQUAL(0x5E, keymap.get_char()); // ^
    RED_CHECK_EQUAL(0x7A, keymap.get_char()); // z

    // deadkey with special key
    decoded_keys = keymap.event(0x0000, 0x1A, ctrl_alt_delete); // '^' down dead key
    decoded_keys = keymap.event(0xC000, 0x1A, ctrl_alt_delete); // '^' up dead key
    decoded_keys = keymap.event(0x0000, 0x0F, ctrl_alt_delete); // TAB
    decoded_keys = keymap.event(0xC000, 0x0F, ctrl_alt_delete); // TAB
    RED_CHECK_EQUAL(0, keymap.nb_char_available());
    RED_CHECK_EQUAL(1, keymap.nb_kevent_available());
    RED_CHECK_EQUAL(0x02, keymap.get_kevent()); // KEVENT_TAB
    // the deadkey is still pending
    decoded_keys = keymap.event(0x0000, 0x12, ctrl_alt_delete); // 'e'
    decoded_keys = keymap.event(0xC000, 0x12, ctrl_alt_delete); // 'e'
    RED_CHECK_EQUAL(1, keymap.nb_char_available());
    RED_CHECK_EQUAL(0xEA, keymap.get_char()); // ê

    // deadkey with Backspace deletes the deadkey
    decoded_keys = keymap.event(0x0000, 0x1A, ctrl_alt_delete); // '^' down dead key
    decoded_keys = keymap.event(0xC000, 0x1A, ctrl_alt_delete); // '^' up dead key
    decoded_keys = keymap.event(0x0000, 0x0E, ctrl_alt_delete); // BACKSPACE
    decoded_keys = keymap.event(0xC000, 0x0E, ctrl_alt_delete); // BACKSPACE
    RED_CHECK_EQUAL(0, keymap.nb_char_available());
    RED_CHECK_EQUAL(0, keymap.nb_kevent_available());
    decoded_keys = keymap.event(0x0000, 0x12, ctrl_alt_delete); // 'e'
    decoded_keys = keymap.event(0xC000, 0x12, ctrl_alt_delete); // 'e'
    RED_CHECK_EQUAL(1, keymap.nb_char_available());
    RED_CHECK_EQUAL(0x65, keymap.get_char()); // e


    // Autorepeat
    // RDP_INPUT_SCANCODE time=538966980 flags=4000 param1=001d param2=0000
    // RDP_INPUT_SCANCODE time=538966980 flags=4100 param1=0038 param2=0000

    // RDP_INPUT_SCANCODE time=538967136 flags=c000 param1=001d param2=0000
    // altgr up
    // RDP_INPUT_SCANCODE time=538967136 flags=c100 param1=0038 param2=0000

    // alt down autorepeat
    // RDP_INPUT_SCANCODE time=539107646 flags=0000 param1=0038 param2=0000
    // alt down autorepeat
    // RDP_INPUT_SCANCODE time=539108270 flags=4000 param1=0038 param2=0000
    // RDP_INPUT_SCANCODE time=539108301 flags=4000 param1=0038 param2=0000
    // RDP_INPUT_SCANCODE time=539108333 flags=4000 param1=0038 param2=0000
    // RDP_INPUT_SCANCODE time=539108364 flags=4000 param1=0038 param2=0000
    // RDP_INPUT_SCANCODE time=539108411 flags=4000 param1=0038 param2=0000
    // alt up
    // RDP_INPUT_SCANCODE time=539108426 flags=c000 param1=0038 param2=0000


    // CTRL+ALT+@ (first line, 0 key)
    // RDP_INPUT_SCANCODE time=539317998 flags=0000 param1=0038 param2=0000
    // RDP_INPUT_SCANCODE time=539318014 flags=0000 param1=001d param2=0000
    // RDP_INPUT_SCANCODE time=539318045 flags=0000 param1=000b param2=0000
    // RDP_INPUT_SCANCODE time=539318154 flags=8000 param1=000b param2=0000
    // RDP_INPUT_SCANCODE time=539318279 flags=c000 param1=0038 param2=0000
    // RDP_INPUT_SCANCODE time=539318279 flags=c000 param1=001d param2=0000

    // AltGr+@ (first line, 0 key)
    // RDP_INPUT_SCANCODE time=539432674 flags=0000 param1=001d param2=0000
    // RDP_INPUT_SCANCODE time=539432674 flags=0100 param1=0038 param2=0000
    // RDP_INPUT_SCANCODE time=539432955 flags=0000 param1=000b param2=0000
    // RDP_INPUT_SCANCODE time=539433049 flags=c000 param1=001d param2=0000
    // RDP_INPUT_SCANCODE time=539433049 flags=c100 param1=0038 param2=0000
    // RDP_INPUT_SCANCODE time=539433049 flags=8000 param1=000b param2=0000

    // abcd (mapping fr) sans relacher aucune touche puis on relache dans l'ordre inverse
    // RDP_INPUT_SCANCODE time=539592794 flags=0000 param1=0010 param2=0000
    // RDP_INPUT_SCANCODE time=539593090 flags=0000 param1=0030 param2=0000
    // RDP_INPUT_SCANCODE time=539593527 flags=0000 param1=002e param2=0000
    // RDP_INPUT_SCANCODE time=539593917 flags=0000 param1=0020 param2=0000
    // RDP_INPUT_SCANCODE time=539594182 flags=8000 param1=002e param2=0000
    // RDP_INPUT_SCANCODE time=539594182 flags=8000 param1=0020 param2=0000
    // RDP_INPUT_SCANCODE time=539594198 flags=8000 param1=0030 param2=0000
    // RDP_INPUT_SCANCODE time=539594198 flags=8000 param1=0010 param2=0000

    // Windows key down then up
    // RDP_INPUT_SCANCODE time=539712088 flags=0100 param1=005b param2=0000
    // RDP_INPUT_SCANCODE time=539712150 flags=8100 param1=005b param2=0000

    // Windows key down, autorepeat, then up
    // RDP_INPUT_SCANCODE time=539790977 flags=0100 param1=005b param2=0000
    // RDP_INPUT_SCANCODE time=539791430 flags=0100 param1=005b param2=0000
    // RDP_INPUT_SCANCODE time=539791461 flags=0100 param1=005b param2=0000
    // RDP_INPUT_SCANCODE time=539791477 flags=0100 param1=005b param2=0000
    // RDP_INPUT_SCANCODE time=539791508 flags=0100 param1=005b param2=0000
    // RDP_INPUT_SCANCODE time=539791539 flags=0100 param1=005b param2=0000
    // RDP_INPUT_SCANCODE time=539791570 flags=0100 param1=005b param2=0000
    // RDP_INPUT_SCANCODE time=539791601 flags=0100 param1=005b param2=0000
    // RDP_INPUT_SCANCODE time=539791648 flags=0100 param1=005b param2=0000
    // RDP_INPUT_SCANCODE time=539791679 flags=0100 param1=005b param2=0000
    // RDP_INPUT_SCANCODE time=539791711 flags=0100 param1=005b param2=0000
    // RDP_INPUT_SCANCODE time=539791742 flags=0100 param1=005b param2=0000
    // RDP_INPUT_SCANCODE time=539791773 flags=0100 param1=005b param2=0000
    // RDP_INPUT_SCANCODE time=539791804 flags=0100 param1=005b param2=0000
    // RDP_INPUT_SCANCODE time=539791820 flags=8100 param1=005b param2=0000

    // dead key ^, key 'A' (french mapping)
    // RDP_INPUT_SCANCODE time=539931659 flags=0000 param1=001a param2=0000
    // RDP_INPUT_SCANCODE time=539931659 flags=8000 param1=001a param2=0000
    // RDP_INPUT_SCANCODE time=539931659 flags=0000 param1=0010 param2=0000
    // RDP_INPUT_SCANCODE time=539931690 flags=8000 param1=0010 param2=0000

    // right CTRL
    // RDP_INPUT_SCANCODE time=540142806 flags=0100 param1=001d param2=0000
    // RDP_INPUT_SCANCODE time=540142916 flags=c100 param1=001d param2=0000

    // left CTRL
    // RDP_INPUT_SCANCODE time=540142806 flags=0000 param1=001d param2=0000
    // RDP_INPUT_SCANCODE time=540142916 flags=c000 param1=001d param2=0000

    // 'Menu' Key Up and Down
    // RDP_INPUT_SCANCODE time=540250369 flags=0100 param1=005d param2=0000
    // RDP_INPUT_SCANCODE time=540250884 flags=8100 param1=005d param2=0000

    // Numlock down autorepeat up
    // RDP_INPUT_SCANCODE time=540394124 flags=0000 param1=0045 param2=0000
    // RDP_INPUT_SCANCODE time=540394608 flags=4000 param1=0045 param2=0000
    // RDP_INPUT_SCANCODE time=540394623 flags=4000 param1=0045 param2=0000
    // RDP_INPUT_SCANCODE time=540394654 flags=4000 param1=0045 param2=0000
    // RDP_INPUT_SCANCODE time=540394686 flags=4000 param1=0045 param2=0000
    // RDP_INPUT_SCANCODE time=540394717 flags=4000 param1=0045 param2=0000
    // RDP_INPUT_SCANCODE time=540394732 flags=4000 param1=0045 param2=0000
    // RDP_INPUT_SCANCODE time=540394764 flags=4000 param1=0045 param2=0000
    // RDP_INPUT_SCANCODE time=540394795 flags=4000 param1=0045 param2=0000
    // RDP_INPUT_SCANCODE time=540394826 flags=4000 param1=0045 param2=0000
    // RDP_INPUT_SCANCODE time=540394857 flags=4000 param1=0045 param2=0000
    // RDP_INPUT_SCANCODE time=540394873 flags=4000 param1=0045 param2=0000
    // RDP_INPUT_SCANCODE time=540394904 flags=4000 param1=0045 param2=0000
    // RDP_INPUT_SCANCODE time=540394935 flags=4000 param1=0045 param2=0000
    // RDP_INPUT_SCANCODE time=540394966 flags=4000 param1=0045 param2=0000
    // RDP_INPUT_SCANCODE time=540394982 flags=4000 param1=0045 param2=0000
    // RDP_INPUT_SCANCODE time=540395013 flags=4000 param1=0045 param2=0000
    // RDP_INPUT_SCANCODE time=540395044 flags=4000 param1=0045 param2=0000
    // RDP_INPUT_SCANCODE time=540395076 flags=4000 param1=0045 param2=0000
    // RDP_INPUT_SCANCODE time=540395091 flags=4000 param1=0045 param2=0000
    // RDP_INPUT_SCANCODE time=540395122 flags=4000 param1=0045 param2=0000
    // RDP_INPUT_SCANCODE time=540395154 flags=4000 param1=0045 param2=0000
    // RDP_INPUT_SCANCODE time=540395185 flags=4000 param1=0045 param2=0000
    // RDP_INPUT_SCANCODE time=540395200 flags=4000 param1=0045 param2=0000
    // RDP_INPUT_SCANCODE time=540395232 flags=4000 param1=0045 param2=0000
    // RDP_INPUT_SCANCODE time=540395232 flags=c000 param1=0045 param2=0000

    // Numlock down up
    // RDP_INPUT_SCANCODE time=540349898 flags=0000 param1=0045 param2=0000
    // RDP_INPUT_SCANCODE time=540349991 flags=c000 param1=0045 param2=0000

    // ESC Down Autorepeat Up
    // RDP_INPUT_SCANCODE time=540563541 flags=0000 param1=0001 param2=0000
    // RDP_INPUT_SCANCODE time=540564040 flags=0000 param1=0001 param2=0000
    // RDP_INPUT_SCANCODE time=540564072 flags=0000 param1=0001 param2=0000
    // RDP_INPUT_SCANCODE time=540564103 flags=0000 param1=0001 param2=0000
    // RDP_INPUT_SCANCODE time=540564118 flags=0000 param1=0001 param2=0000
    // RDP_INPUT_SCANCODE time=540564150 flags=0000 param1=0001 param2=0000
    // RDP_INPUT_SCANCODE time=540564181 flags=0000 param1=0001 param2=0000
    // RDP_INPUT_SCANCODE time=540564212 flags=0000 param1=0001 param2=0000
    // RDP_INPUT_SCANCODE time=540564243 flags=0000 param1=0001 param2=0000
    // RDP_INPUT_SCANCODE time=540564274 flags=0000 param1=0001 param2=0000
    // RDP_INPUT_SCANCODE time=540564290 flags=0000 param1=0001 param2=0000
    // RDP_INPUT_SCANCODE time=540564290 flags=8000 param1=0001 param2=0000

}


RED_AUTO_TEST_CASE(TestKeymapBuffer)
{
    Keymap2 keymap;
    keymap.init_layout(0x040C);

    Keymap2::DecodedKeys decoded_keys;
    bool    ctrl_alt_delete;

    RED_CHECK_EQUAL(0, keymap.nb_char_available());
    decoded_keys = keymap.event(0, 0x10, ctrl_alt_delete);
    RED_CHECK_EQUAL(1, keymap.nb_char_available());
    RED_CHECK_EQUAL('a', keymap.get_char());
    decoded_keys = keymap.event(0xc000, 0x10, ctrl_alt_delete); // up

    decoded_keys = keymap.event(0x0100, 0x35, ctrl_alt_delete); // '/' on keypad
    RED_CHECK_EQUAL(1, keymap.nb_char_available());
    decoded_keys = keymap.event(0xc100, 0x35, ctrl_alt_delete); // '/' on keypad

    RED_CHECK_EQUAL(1, keymap.nb_char_available());
    RED_CHECK_EQUAL('/', keymap.get_char());
    RED_CHECK_EQUAL(0, keymap.nb_char_available());

    // saturating buffer
    for(size_t i = 0; i < 10 ; i++){
        decoded_keys = keymap.event(0, 0x10, ctrl_alt_delete);
        RED_CHECK_EQUAL(i+1, keymap.nb_char_available());
        decoded_keys = keymap.event(0xc000, 0x10, ctrl_alt_delete); // up
    }

    // saturating buffer
    for(size_t i = 10; i < 20 ; i++){
        decoded_keys = keymap.event(0, 0x11, ctrl_alt_delete);
        RED_CHECK_EQUAL(i+1, keymap.nb_char_available());
        decoded_keys = keymap.event(0xc000, 0x11, ctrl_alt_delete); // up
    }

    // buffer saturated
    decoded_keys = keymap.event(0, 0x10, ctrl_alt_delete);
    RED_CHECK_EQUAL(20, keymap.nb_char_available());
    decoded_keys = keymap.event(0xc000, 0x10, ctrl_alt_delete); // up

    // Reading back buffer
    for(size_t i = 0; i < 10 ; i++){
        RED_CHECK_EQUAL('a', keymap.get_char());
        RED_CHECK_EQUAL(19 - i, keymap.nb_char_available());
    }

    // Reading back buffer
    for(size_t i = 10; i < 20 ; i++){
        RED_CHECK_EQUAL('z', keymap.get_char());
        RED_CHECK_EQUAL(19 - i, keymap.nb_char_available());
    }

    RED_CHECK_EQUAL(0, keymap.nb_char_available());

    // down arrow
    decoded_keys = keymap.event(0x0100, 0x50, ctrl_alt_delete);
    RED_CHECK_EQUAL(0, keymap.nb_char_available());
    decoded_keys = keymap.event(0xc100, 0x50, ctrl_alt_delete);

    RED_CHECK_EQUAL(0, decoded_keys.count);
}


RED_AUTO_TEST_CASE(TestKeyPad)
{
    Keymap2 keymap;
    const int layout = 0x040C;
    keymap.init_layout(layout);
    // all lock keys are supposed to be inactive at this, point
    RED_CHECK_EQUAL(0, keymap.key_flags);

    Keymap2::DecodedKeys decoded_keys;
    bool    ctrl_alt_delete;

    decoded_keys = keymap.event(0x0000, 0x45, ctrl_alt_delete); // activate numlock
    RED_CHECK_EQUAL(0, decoded_keys.count);
    RED_CHECK_EQUAL(2, keymap.key_flags);
    decoded_keys = keymap.event(0x0000, 0x45, ctrl_alt_delete); // desactivate numlock
    RED_CHECK_EQUAL(0, decoded_keys.count);
    RED_CHECK_EQUAL(0, keymap.key_flags);

    decoded_keys = keymap.event(0x0000, 0x45, ctrl_alt_delete); // activate numlock
    RED_CHECK_EQUAL(0, decoded_keys.count);
    decoded_keys = keymap.event(0, 0x47, ctrl_alt_delete); // keypad '7' character
    RED_CHECK_EQUAL(1, decoded_keys.count);
    RED_CHECK_EQUAL('7', decoded_keys.uchars[0]);
    RED_CHECK_EQUAL('7', keymap.get_char());

    decoded_keys = keymap.event(0x0000, 0x45, ctrl_alt_delete); // desactivate numlock
    RED_CHECK_EQUAL(0, decoded_keys.count);
    decoded_keys = keymap.event(0, 0x47, ctrl_alt_delete); // keypad home character
    RED_CHECK_EQUAL(1, decoded_keys.count);
    RED_CHECK_EQUAL(0x2196, decoded_keys.uchars[0]);
    RED_CHECK_EQUAL(0, keymap.get_char());
}
