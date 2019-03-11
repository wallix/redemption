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


#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "keyboard/keymapSym.hpp"

RED_AUTO_TEST_CASE(TestKeymapSym)
{
    KeymapSym keymap(0x040C, 0, false, false, 0);
    uint8_t downflag = 0;

    RED_CHECK_EQUAL(false, keymap.is_shift_pressed());

    uint16_t keyboardFlags = 0 ; // key is not extended, key was up, key goes down
    uint16_t keyCode = 54 ; // key is left shift
    keymap.event(keyboardFlags, keyCode);
    auto key = keymap.get_sym(downflag);
    RED_CHECK_EQUAL(0, keymap.nb_sym_available());
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

    RED_CHECK_EQUAL('A', keymap.get_sym(downflag));

    keyboardFlags = keymap.KBDFLAGS_DOWN|keymap.KBDFLAGS_RELEASE ; // key is not extended, key was down, key goes up
    keymap.event(keyboardFlags, 54); // key is left shift

    RED_CHECK_EQUAL(false, keymap.is_shift_pressed());
    RED_CHECK_EQUAL(false, keymap.is_left_shift_pressed());
    RED_CHECK_EQUAL(false, keymap.is_right_shift_pressed());

    // shift was released, but not A (last char down goes 'a' for autorepeat)
    keyboardFlags = keymap.KBDFLAGS_DOWN|keymap.KBDFLAGS_RELEASE ; // key is not extended, key was down, key goes up
    keyCode = 16 ; // key is 'A'
    keymap.event(keyboardFlags, keyCode);

    RED_CHECK_EQUAL(0xFFE2, keymap.get_sym(downflag));

    RED_CHECK_EQUAL('a', keymap.get_sym(downflag));

    keyboardFlags = 0 ; // key is not extended, key was up, key goes down
    keyCode = 0x10 ; // key is 'A'
    keymap.event(keyboardFlags, keymap.get_sym(downflag));

    // CAPSLOCK Down
    // RDP_INPUT_SCANCODE time=538384316 flags=0000 param1=003a param2=0000
    RED_CHECK_EQUAL(false, keymap.is_caps_locked());
    keymap.event(0, 0x3A);
    RED_CHECK_EQUAL(true, keymap.is_caps_locked());

    // CAPSLOCK Up
    // RDP_INPUT_SCANCODE time=538384894 flags=c000 param1=003a param2=0000
    keymap.event(0xc000, 0x3A);
    RED_CHECK_EQUAL(true, keymap.is_caps_locked());

    // Now I hit the 'A' key on french keyboard
    keymap.event(0, 0x10);
    keymap.event(0xc000, 0x10); // A up
    RED_CHECK_EQUAL(true, keymap.is_caps_locked());
    RED_CHECK_EQUAL(2, keymap.nb_sym_available());
    RED_CHECK_EQUAL('A', keymap.get_sym(downflag));
    RED_CHECK_EQUAL(1, downflag);
    RED_CHECK_EQUAL('A', keymap.get_sym(downflag));
    RED_CHECK_EQUAL(0, downflag);

    keymap.event(0, 0x02);
    RED_CHECK_EQUAL('&', keymap.get_sym(downflag));
    RED_CHECK_EQUAL(1, downflag);

    keymap.event(0, 0x36); // left shift down
    RED_CHECK_EQUAL(0xffe2, keymap.get_sym(downflag));
    RED_CHECK_EQUAL(1, downflag);

    keymap.event(0xc000, 0x36); // left shift up
    RED_CHECK_EQUAL(0xffe2, keymap.get_sym(downflag));
    RED_CHECK_EQUAL(0, downflag);

    keymap.event(0, 0x2A); // right shift down
    RED_CHECK_EQUAL(0xffe1, keymap.get_sym(downflag));
    RED_CHECK_EQUAL(1, downflag);

    keymap.event(0xc000, 0x2A); // right shift up
    RED_CHECK_EQUAL(0xffe1, keymap.get_sym(downflag));
    RED_CHECK_EQUAL(0, downflag);

    // CAPSLOCK Down
    // RDP_INPUT_SCANCODE time=538384316 flags=0000 param1=003a param2=0000
    RED_CHECK_EQUAL(true, keymap.is_caps_locked());
    keymap.event(0, 0x3A); // capslock down
    RED_CHECK_EQUAL(false, keymap.is_caps_locked());
    keymap.event(0xC000, 0x3A); // capslock up
    RED_CHECK_EQUAL(false, keymap.is_caps_locked());
    // Capse Lock do not send anything: fully managed inside proxy
    RED_CHECK_EQUAL(0, keymap.nb_sym_available());


    // Now I hit the 'A' key on french keyboard
    keymap.event(0, 0x10);
    RED_CHECK_EQUAL('a', keymap.get_sym(downflag));
    keymap.event(0xc000, 0x10); // up
    RED_CHECK_EQUAL('a', keymap.get_sym(downflag));
    RED_CHECK_EQUAL(0, keymap.nb_sym_available());
}


//// With ALTGr
//// AltGR down
//KeymapSym::event(keyboardFlags=0x0100, keyCode=0x0038 flags=0x00a0)
//KeymapSym::push_sym(sym=0000ffea) nbuf_sym=0
//KeymapSym::event(keyboardFlags=0x0000, keyCode=0x000b flags=0x00a0)
//KeymapSym::push_sym(sym=00000040) nbuf_sym=0
//// AltGR up
//KeymapSym::event(keyboardFlags=0x8000, keyCode=0x000b flags=0x00a0)
//KeymapSym::push_sym(sym=00000040) nbuf_sym=0

//KeymapSym::event(keyboardFlags=0x8100, keyCode=0x0038 flags=0x00a0)
//KeymapSym::push_sym(sym=0000ffea) nbuf_sym=0

//// With CTRL+Alt
//// CTRL down
//KeymapSym::event(keyboardFlags=0x0000, keyCode=0x001d flags=0x00a0)
//KeymapSym::push_sym(sym=0000ffe3) nbuf_sym=0
//// ALT down
//KeymapSym::event(keyboardFlags=0x0000, keyCode=0x0038 flags=0x00a0)
//KeymapSym::push_sym(sym=0000ffe9) nbuf_sym=0
//// 0 down
//KeymapSym::event(keyboardFlags=0x0000, keyCode=0x000b flags=0x00a0)
//KeymapSym::push_sym(sym=000000e0) nbuf_sym=0
//// 0 up
//KeymapSym::event(keyboardFlags=0x8000, keyCode=0x000b flags=0x00a0)
//// -> I get an agrave, the target server translate that to an arobas
//KeymapSym::push_sym(sym=000000e0) nbuf_sym=0

//// CTRL up
//KeymapSym::event(keyboardFlags=0x8000, keyCode=0x001d flags=0x00a0)
//KeymapSym::push_sym(sym=0000ffe3) nbuf_sym=0
//// ALT up
//KeymapSym::event(keyboardFlags=0x8000, keyCode=0x0038 flags=0x00a0)
//KeymapSym::push_sym(sym=0000ffe9) nbuf_sym=0


// KEYBOARD MAP WITH RDP SCANCODES
// -------------------------------
// +----+  +----+----+----+----+  +----+----+----+----+  +----+----+----+----+  +----+----+-------+
// | 01 |  | 3B | 3C | 3D | 3E |  | 3F | 40 | 41 | 42 |  | 43 | 44 | 57 | 58 |  | 37 | 46 | 1D+45 |
// +----+  +----+----+----+----+  +----+----+----+----+  +----+----+----+----+  +----+----+-------+
//                                     ***  keycodes suffixed by 'x' are extended ***
// +----+----+----+----+----+----+----+----+----+----+----+----+----+--------+  +----+----+----+  +--------------------+
// | 29 | 02 | 03 | 04 | 05 | 06 | 07 | 08 | 09 | 0A | 0B | 0C | 0D |   0E   |  | 52x| 47x| 49x|  | 45 | 35x| 37 | 4A  |
// +-------------------------------------------------------------------------+  +----+----+----+  +----+----+----+-----+
// |  0F  | 10 | 11 | 12 | 13 | 14 | 15 | 16 | 17 | 18 | 19 | 1A | 1B |      |  | 53x| 4Fx| 51x|  | 47 | 48 | 49 |     |
// +------------------------------------------------------------------+  1C  |  +----+----+----+  +----+----+----| 4E  |
// |  3A   | 1E | 1F | 20 | 21 | 22 | 23 | 24 | 25 | 26 | 27 | 28 | 2B |     |                    | 4B | 4C | 4D |     |
// +-------------------------------------------------------------------------+       +----+       +----+----+----+-----+
// |  2A | 56 | 2C | 2D | 2E | 2F | 30 | 31 | 32 | 33 | 34 | 35 |     36     |       | 48x|       | 4F | 50 | 51 |     |
// +-------------------------------------------------------------------------+  +----+----+----+  +---------+----| 1Cx |
// |  1D  |  5Bx | 38 |           39           |  38x  |  5Cx |  5Dx |  1Dx  |  | 4Bx| 50x| 4Dx|  |    52   | 53 |     |
// +------+------+----+------------------------+-------+------+------+-------+  +----+----+----+  +---------+----+-----+

RED_AUTO_TEST_CASE(TestKeymapSymEuro)
{
    KeymapSym keymap(0x040C, 0, false, false, 0);
    uint8_t downflag = 0;

    keymap.event(0x0000, 0x001d);
    RED_CHECK_EQUAL(KS_Ctrl_L, keymap.get_sym(downflag));
    RED_CHECK_EQUAL(1, downflag);

    keymap.event(0x0000, 0x0038);
    RED_CHECK_EQUAL(KS_Alt_L, keymap.get_sym(downflag));
    RED_CHECK_EQUAL(1, downflag);

    keymap.event(0x0000, 0x0012); // 'e' down

    // I get an E, which means the translation is done by target VNC server not here
    RED_CHECK_EQUAL(KS_Alt_L, keymap.get_sym(downflag)); // up
    RED_CHECK_EQUAL(0, downflag);
    RED_CHECK_EQUAL(KS_Ctrl_L, keymap.get_sym(downflag)); // up
    RED_CHECK_EQUAL(0, downflag);

    RED_CHECK_EQUAL(KS_Ctrl_L, keymap.get_sym(downflag)); // down
    RED_CHECK_EQUAL(1, downflag);
    RED_CHECK_EQUAL(KS_Alt_L, keymap.get_sym(downflag)); // down
    RED_CHECK_EQUAL(1, downflag);

    RED_CHECK_EQUAL(0x65, keymap.get_sym(downflag));
    RED_CHECK_EQUAL(1, downflag);

    keymap.event(0x8000, 0x0012); // 'e' up

    RED_CHECK_EQUAL(KS_Ctrl_L, keymap.get_sym(downflag));

    RED_CHECK_EQUAL(0, downflag); // up
    RED_CHECK_EQUAL(KS_Alt_L, keymap.get_sym(downflag));
    RED_CHECK_EQUAL(0, downflag); // up
    
    RED_CHECK_EQUAL(KS_Alt_L, keymap.get_sym(downflag));
    RED_CHECK_EQUAL(1, downflag); // down
    RED_CHECK_EQUAL(KS_Ctrl_L, keymap.get_sym(downflag));
    RED_CHECK_EQUAL(1, downflag); // down

    keymap.event(0x8000, 0x001d); // ctrl up
    RED_CHECK_EQUAL(1, keymap.nb_sym_available());

    RED_CHECK_EQUAL(KS_Ctrl_L, keymap.get_sym(downflag));
    RED_CHECK_EQUAL(0, downflag);

    keymap.event(0x8000, 0x0038); // Alt up
    RED_CHECK_EQUAL(1, keymap.nb_sym_available());
    RED_CHECK_EQUAL(KS_Alt_L, keymap.get_sym(downflag));
    RED_CHECK_EQUAL(0, downflag);

    keymap.event(0x8000, 0x0038); // Alt up
    RED_CHECK_EQUAL(1, keymap.nb_sym_available());
    RED_CHECK_EQUAL(KS_Alt_L, keymap.get_sym(downflag));
    RED_CHECK_EQUAL(0, downflag); // Alt up
}

RED_AUTO_TEST_CASE(TestKeymapSymUpUp)
{
    KeymapSym keymap(0x040C, 0, false, false, 0);
    uint8_t downflag = 0;

    keymap.event(0x8000, 0x0038); // Alt up
    RED_CHECK_EQUAL(1, keymap.nb_sym_available());
    RED_CHECK_EQUAL(KS_Alt_L, keymap.get_sym(downflag));
    RED_CHECK_EQUAL(0, downflag);

    keymap.event(0x8000, 0x0038); // Alt up
    RED_CHECK_EQUAL(1, keymap.nb_sym_available());
    RED_CHECK_EQUAL(KS_Alt_L, keymap.get_sym(downflag));
    RED_CHECK_EQUAL(0, downflag); // Alt down
}

RED_AUTO_TEST_CASE(TestKeymapSymDownDown)
{
    KeymapSym keymap(0x040C, 0, false, false, 0);
    uint8_t downflag = 0;

    keymap.event(0x0000, 0x0038); // Alt down
    RED_CHECK_EQUAL(1, keymap.nb_sym_available());
    RED_CHECK_EQUAL(KS_Alt_L, keymap.get_sym(downflag));
    RED_CHECK_EQUAL(1, downflag);

    keymap.event(0x0000, 0x0038); // Alt down
    RED_CHECK_EQUAL(1, keymap.nb_sym_available());
    RED_CHECK_EQUAL(KS_Alt_L, keymap.get_sym(downflag));
    RED_CHECK_EQUAL(1, downflag); // Alt down
}
