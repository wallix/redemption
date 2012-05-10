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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Unit test to keymap object
   Using lib boost functions, some tests need to be added

*/


#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestKeymap
#include <boost/test/auto_unit_test.hpp>

#include <stdint.h>

#define LOGPRINT
#include "keymap2.hpp"

BOOST_AUTO_TEST_CASE(TestKeymap)
{
    Keymap2 keymap;
    const int layout = 0x040C;
    keymap.init_layout(layout);

    BOOST_CHECK_EQUAL(false, keymap.is_shift_pressed());

    uint16_t keyboardFlags = 0 ; // key is not extended, key was up, key goes down
    uint16_t keyCode = 54 ; // key is left shift
    keymap.event(keyboardFlags, keyCode);

    BOOST_CHECK_EQUAL(true, keymap.is_shift_pressed());
    BOOST_CHECK_EQUAL(true, keymap.is_left_shift_pressed());
    BOOST_CHECK_EQUAL(false, keymap.is_right_shift_pressed());

    keyboardFlags = 0 ; // key is not extended, key was up, key goes down
    keyCode = 16 ; // key is 'A'
    keymap.event(keyboardFlags, keyCode);

    BOOST_CHECK_EQUAL(true, keymap.is_shift_pressed());
    BOOST_CHECK_EQUAL(true, keymap.is_left_shift_pressed());
    BOOST_CHECK_EQUAL(false, keymap.is_right_shift_pressed());

    uint32_t key = keymap.top_char();
    BOOST_CHECK_EQUAL('A', key);

    keyboardFlags = keymap.KBDFLAGS_DOWN|keymap.KBDFLAGS_RELEASE ; // key is not extended, key was down, key goes up
    keyCode = 54 ; // key is left shift
    keymap.event(keyboardFlags, keyCode);

    BOOST_CHECK_EQUAL(false, keymap.is_shift_pressed());
    BOOST_CHECK_EQUAL(false, keymap.is_left_shift_pressed());
    BOOST_CHECK_EQUAL(false, keymap.is_right_shift_pressed());

    // shift was released, but not A (last char down goes 'a' for autorepeat)
    key = keymap.top_char();
    BOOST_CHECK_EQUAL('A', key);

    keyboardFlags = keymap.KBDFLAGS_DOWN|keymap.KBDFLAGS_RELEASE ; // key is not extended, key was down, key goes up
    keyCode = 16 ; // key is 'A'
    keymap.event(keyboardFlags, keyCode);

    key = keymap.get_char();
    BOOST_CHECK_EQUAL('A', key);


    keyboardFlags = 0 ; // key is not extended, key was up, key goes down
    keyCode = 16 ; // key is 'A'
    keymap.event(keyboardFlags, keyCode);
    key = keymap.top_char();
    BOOST_CHECK_EQUAL('a', key);

    // CAPSLOCK Down
    // RDP_INPUT_SCANCODE time=538384316 flags=0000 param1=003a param2=0000
    BOOST_CHECK_EQUAL(false, keymap.is_caps_locked());
    keymap.event(0, 0x3A);
    BOOST_CHECK_EQUAL(true, keymap.is_caps_locked());
    key = keymap.top_char();
    BOOST_CHECK_EQUAL('a', key);

    // CAPSLOCK Up
    // RDP_INPUT_SCANCODE time=538384894 flags=c000 param1=003a param2=0000
    keymap.event(0xc000, 0x3A);
    BOOST_CHECK_EQUAL(true, keymap.is_caps_locked());
    key = keymap.get_char();
    BOOST_CHECK_EQUAL('a', key);


    // Now I hit the 'A' key on french keyboard
    keymap.event(0, 0x10);
    key = keymap.top_char();
    BOOST_CHECK_EQUAL('A', key);

    keymap.event(0xc000, 0x10); // A up
    key = keymap.get_char();
    BOOST_CHECK_EQUAL('A', key);

    BOOST_CHECK_EQUAL(true, keymap.is_caps_locked());
    keymap.event(0, 0x02);
    key = keymap.top_char();
    BOOST_CHECK_EQUAL('1', key);

    // left shift down
    keymap.event(0, 0x36);

    // left shift up
    keymap.event(0xc000, 0x36);

    // right shift down
    keymap.event(0, 0x2A);

    // right shift up
    keymap.event(0xc000, 0x2A);

    // CAPSLOCK Down
    // RDP_INPUT_SCANCODE time=538384316 flags=0000 param1=003a param2=0000
    BOOST_CHECK_EQUAL(true, keymap.is_caps_locked());
    keymap.event(0, 0x3A);
    BOOST_CHECK_EQUAL(false, keymap.is_caps_locked());
    keymap.event(0xC000, 0x3A); // capslock up
    BOOST_CHECK_EQUAL(false, keymap.is_caps_locked());

    key = keymap.get_char();
    BOOST_CHECK_EQUAL('1', key);

    // Now I hit the 'A' key on french keyboard
    keymap.event(0, 0x10);
    key = keymap.get_char();
    BOOST_CHECK_EQUAL('a', key);
    keymap.event(0xc000, 0x10); // up

    keymap.event(0, 0x02);
    key = keymap.get_char();
    BOOST_CHECK_EQUAL('&', key);
    keymap.event(0xc000, 0x02);

    // left shift down
    keymap.event(0, 54);
    keymap.event(0, 0x02);
    key = keymap.get_char();
    BOOST_CHECK_EQUAL('1', key);

    // left shift up
    keymap.event(0xc000, 54);
    keymap.event(0, 0x02);
    key = keymap.top_char();
    BOOST_CHECK_EQUAL('&', key);

    BOOST_CHECK_EQUAL(false, keymap.is_caps_locked());
    keymap.event(0, 0x3A);
    BOOST_CHECK_EQUAL(true, keymap.is_caps_locked());
    keymap.event(0xC000, 0x3A); // capslock up
    BOOST_CHECK_EQUAL(true, keymap.is_caps_locked());


    key = keymap.get_char();
    BOOST_CHECK_EQUAL('&', key);

    // Now I hit the 'A' key on french keyboard
    keymap.event(0, 0x10);
    BOOST_CHECK_EQUAL('A', keymap.get_char());
    keymap.event(0xc000, 0x10); // up

    keymap.event(0, 0x02);
    BOOST_CHECK_EQUAL('1', keymap.get_char());
    keymap.event(0xc000, 0x02);

    // left shift down
    keymap.event(0, 54);
    keymap.event(0, 0x02);
    BOOST_CHECK_EQUAL('&', keymap.get_char());

    // left shift up
    keymap.event(0xc000, 54);
    keymap.event(0, 0x02);
    BOOST_CHECK_EQUAL('1', keymap.get_char());

    // altgr down
    // RDP_INPUT_SCANCODE time=538966481 flags=0000 param1=001d param2=0000 -> CTRL
    // altgr down autorepeat
    // RDP_INPUT_SCANCODE time=538966481 flags=0100 param1=0038 param2=0000 -> ALT
    keymap.event(0x0000, 0x1d); // CTRL
    BOOST_CHECK_EQUAL(true, keymap.is_ctrl_pressed());
    keymap.event(0x0100, 0x38); // ALT Right
    BOOST_CHECK_EQUAL(true, keymap.is_right_alt_pressed());
    keymap.event(0x0000, 0x03); // Tilde
    BOOST_CHECK_EQUAL(1, keymap.nb_char_available());
    BOOST_CHECK_EQUAL('~', keymap.get_char());

    keymap.event(0xC000, 0x03); // Tilde
    keymap.event(0xC100, 0x38); // ALT Right
    BOOST_CHECK_EQUAL(false, keymap.is_right_alt_pressed());
    keymap.event(0xC000, 0x1d); // CTRL
    BOOST_CHECK_EQUAL(false, keymap.is_ctrl_pressed());


    keymap.event(0x0100, 0x35); // '/' on keypad
    BOOST_CHECK_EQUAL(1, keymap.nb_char_available());
    key = keymap.get_char();
    BOOST_CHECK_EQUAL('/', key);



    keymap.event(0xC000, 0x03); // Tilde
    keymap.event(0xC100, 0x38); // ALT Right
    BOOST_CHECK_EQUAL(false, keymap.is_right_alt_pressed());
    keymap.event(0xC000, 0x1d); // CTRL
    BOOST_CHECK_EQUAL(false, keymap.is_ctrl_pressed());


    keymap.event(0x0100, 0x35); // '/' on keypad
    BOOST_CHECK_EQUAL(1, keymap.nb_char_available());
    key = keymap.get_char();
    BOOST_CHECK_EQUAL('/', key);
    BOOST_CHECK_EQUAL(0, keymap.nb_char_available());
}

BOOST_AUTO_TEST_CASE(TestDeadKeys)
{
    Keymap2 keymap;
    const int layout = 0x040C;
    keymap.init_layout(layout);
    BOOST_CHECK_EQUAL(0, keymap.nb_char_available());

    keymap.event(0x0000, 0x1A); // '^' down dead key
    keymap.event(0xC000, 0x1A); // '^' up dead key
    BOOST_CHECK_EQUAL(0, keymap.nb_char_available());

    keymap.event(0x0000, 0x12); // 'e'
    keymap.event(0xC000, 0x12); // 'e'
    BOOST_CHECK_EQUAL(1, keymap.nb_char_available());
    BOOST_CHECK_EQUAL(0xEA, keymap.top_char()); // ê


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


BOOST_AUTO_TEST_CASE(TestKeymapBuffer)
{
    Keymap2 keymap;
    keymap.init_layout(0x040C);

    BOOST_CHECK_EQUAL(0, keymap.nb_char_available());
    keymap.event(0, 0x10);
    BOOST_CHECK_EQUAL(1, keymap.nb_char_available());
    BOOST_CHECK_EQUAL('a', keymap.top_char());
    keymap.event(0xc000, 0x10); // up

    keymap.event(0x0100, 0x35); // '/' on keypad
    BOOST_CHECK_EQUAL('/', keymap.top_char());
    BOOST_CHECK_EQUAL(2, keymap.nb_char_available());
    keymap.event(0xc100, 0x35); // '/' on keypad

    BOOST_CHECK_EQUAL('a', keymap.get_char());
    BOOST_CHECK_EQUAL(1, keymap.nb_char_available());
    BOOST_CHECK_EQUAL('/', keymap.get_char());
    BOOST_CHECK_EQUAL(0, keymap.nb_char_available());

    // saturating buffer
    for(size_t i = 0; i < 10 ; i++){
        keymap.event(0, 0x10);
        BOOST_CHECK_EQUAL(i+1, keymap.nb_char_available());
        BOOST_CHECK_EQUAL('a', keymap.top_char());
        keymap.event(0xc000, 0x10); // up
    }

    // saturating buffer
    for(size_t i = 10; i < 20 ; i++){
        keymap.event(0, 0x11);
        BOOST_CHECK_EQUAL(i+1, keymap.nb_char_available());
        BOOST_CHECK_EQUAL('z', keymap.top_char());
        keymap.event(0xc000, 0x11); // up
    }

    // buffer saturated
    keymap.event(0, 0x10);
    BOOST_CHECK_EQUAL(20, keymap.nb_char_available());
    keymap.event(0xc000, 0x10); // up

    // saturating buffer
    for(size_t i = 0; i < 10 ; i++){
        BOOST_CHECK_EQUAL('a', keymap.get_char());
        BOOST_CHECK_EQUAL(19 - i, keymap.nb_char_available());
    }

    for(size_t i = 10; i < 20 ; i++){
        BOOST_CHECK_EQUAL('z', keymap.get_char());
        BOOST_CHECK_EQUAL(19 - i, keymap.nb_char_available());
    }

    // down arrow
    keymap.event(0x0100, 0x50);
    BOOST_CHECK_EQUAL(0, keymap.nb_char_available());
    keymap.event(0xc100, 0x50);


}



BOOST_AUTO_TEST_CASE(TestKeyPad)
{
    Keymap2 keymap;
    const int layout = 0x040C;
    keymap.init_layout(layout);
    // all lock keys are supposed to be inactive at this, point
    BOOST_CHECK_EQUAL(0, keymap.key_flags);

    keymap.event(0x0000, 0x45); // activate numlock
    BOOST_CHECK_EQUAL(2, keymap.key_flags);
    keymap.event(0x0000, 0x45); // desactivate numlock
    BOOST_CHECK_EQUAL(0, keymap.key_flags);

    keymap.event(0x0000, 0x45); // activate numlock
    keymap.event(0, 0x47); // keypad '7' character
    BOOST_CHECK_EQUAL('7', keymap.get_char());

    keymap.event(0x0000, 0x45); // desactivate numlock
    keymap.event(0, 0x47); // keypad home character
    BOOST_CHECK_EQUAL(0, keymap.get_char());
}
