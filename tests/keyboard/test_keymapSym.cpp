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
   Author(s): Christophe Grosjean

   Unit test to keymap object
   Using lib boost functions, some tests need to be added

*/


#define RED_TEST_MODULE TestKeymapSym
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "keyboard/keymapSym.hpp"

RED_AUTO_TEST_CASE(TestKeymapSym)
{
    KeymapSym keymap;
    const int layout = 0x040C;
    keymap.init_layout_sym(layout);

    RED_CHECK_EQUAL(false, keymap.is_shift_pressed());

    uint16_t keyboardFlags = 0 ; // key is not extended, key was up, key goes down
    uint16_t keyCode = 54 ; // key is left shift
    keymap.event(keyboardFlags, keyCode);
    RED_CHECK_EQUAL(1, keymap.nb_sym_available());
    uint32_t key = keymap.get_sym();
    RED_CHECK_EQUAL(0xffe2, key); // Shift_R

    RED_CHECK_EQUAL(true, keymap.is_shift_pressed());
    RED_CHECK_EQUAL(true, keymap.is_left_shift_pressed());
    RED_CHECK_EQUAL(false, keymap.is_right_shift_pressed());

    keyboardFlags = 0 ; // key is not extended, key was up, key goes down
    keyCode = 16 ; // key is 'A'
    keymap.event(keyboardFlags, keyCode);

//    RED_CHECK_EQUAL(true, keymap.is_shift_pressed());
//    RED_CHECK_EQUAL(true, keymap.is_left_shift_pressed());
//    RED_CHECK_EQUAL(false, keymap.is_right_shift_pressed());

    key = keymap.get_sym();
    RED_CHECK_EQUAL('A', key);

    keyboardFlags = keymap.KBDFLAGS_DOWN|keymap.KBDFLAGS_RELEASE ; // key is not extended, key was down, key goes up
    keyCode = 54 ; // key is left shift
    keymap.event(keyboardFlags, keyCode);

    RED_CHECK_EQUAL(false, keymap.is_shift_pressed());
    RED_CHECK_EQUAL(false, keymap.is_left_shift_pressed());
    RED_CHECK_EQUAL(false, keymap.is_right_shift_pressed());

    // shift was released, but not A (last char down goes 'a' for autorepeat)
    keyboardFlags = keymap.KBDFLAGS_DOWN|keymap.KBDFLAGS_RELEASE ; // key is not extended, key was down, key goes up
    keyCode = 16 ; // key is 'A'
    keymap.event(keyboardFlags, keyCode);

    key = keymap.get_sym();
    RED_CHECK_EQUAL(0xffe2, key);

    key = keymap.get_sym();
    RED_CHECK_EQUAL('a', key);

    keyboardFlags = 0 ; // key is not extended, key was up, key goes down
    keyCode = 0x10 ; // key is 'A'
    keymap.event(keyboardFlags, keyCode);

    // CAPSLOCK Down
    // RDP_INPUT_SCANCODE time=538384316 flags=0000 param1=003a param2=0000
    RED_CHECK_EQUAL(false, keymap.is_caps_locked());
    keymap.event(0, 0x3A);
    RED_CHECK_EQUAL(true, keymap.is_caps_locked());

    // CAPSLOCK Up
    // RDP_INPUT_SCANCODE time=538384894 flags=c000 param1=003a param2=0000
    keymap.event(0xc000, 0x3A);
    RED_CHECK_EQUAL(true, keymap.is_caps_locked());
    key = keymap.get_sym();
    RED_CHECK_EQUAL('a', key);


    // Now I hit the 'A' key on french keyboard
    keymap.event(0, 0x10);
    keymap.event(0xc000, 0x10); // A up
    key = keymap.get_sym();
    RED_CHECK_EQUAL('A', key);
    RED_CHECK_EQUAL(true, keymap.is_caps_locked());

    RED_CHECK_EQUAL(1, keymap.nb_sym_available());

    keymap.event(0, 0x02);
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
    RED_CHECK_EQUAL(true, keymap.is_caps_locked());
    keymap.event(0, 0x3A);
    RED_CHECK_EQUAL(false, keymap.is_caps_locked());
    keymap.event(0xC000, 0x3A); // capslock up
    RED_CHECK_EQUAL(false, keymap.is_caps_locked());

    RED_CHECK_EQUAL(6, keymap.nb_sym_available());
    RED_CHECK_EQUAL('A', keymap.get_sym());
    RED_CHECK_EQUAL('&', keymap.get_sym());
    RED_CHECK_EQUAL(0xffe2, keymap.get_sym());
    RED_CHECK_EQUAL(0xffe2, keymap.get_sym());
    RED_CHECK_EQUAL(0xffe1, keymap.get_sym());
    RED_CHECK_EQUAL(0xffe1, keymap.get_sym());

    // Now I hit the 'A' key on french keyboard
    keymap.event(0, 0x10);
    RED_CHECK_EQUAL('a', keymap.get_sym());
    keymap.event(0xc000, 0x10); // up
    RED_CHECK_EQUAL('a', keymap.get_sym());
}


