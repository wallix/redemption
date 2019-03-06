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
#include "system/redemption_unit_tests.hpp"

#include "utils/log.hpp"

#include "keyboard/keymapSym.hpp"

enum ScancodeState {
    DOWN = 0,
    UP   = 0x8000,
    DOWN_EXT = 0x0100,
    UP_EXT   = 0x8100
};

//enum PersistantKeys {
//    None      = 0,
//    ShiftL    = 0x1,
//    ShiftR    = 0x8000,
//};


struct check_key_events {
    ScancodeState flags;   // input keyboard flags
    uint8_t  code;         // key scancode, or 0 if there is another keysym to read from previous keys
    uint16_t rflags;       // expected keymap flags (persistant flags)
    uint32_t rkey;         // expected generated key (or 0 if no key to send, 
                           // like for deadkeys or char keys up)
    
};


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


RED_AUTO_TEST_CASE(TestKeymapSymLetterA_FR)
{

    KeymapSym keymap;
    // TODO: use symbolic names for layouts, below 0x040C is french layout
    keymap.init_layout_sym(0x040C);

    check_key_events keys[] = {
        {DOWN, 16, 0, 'a'},
        {UP,   16, 0, 'a'},
    }; 
    for (auto k: keys){
//        LOG(LOG_INFO, "%u", k.code);
        keymap.event(k.flags, k.code);
        RED_CHECK_EQUAL(k.rflags, keymap.key_flags);
        RED_CHECK_EQUAL(k.rkey, keymap.get_sym());
    }
    RED_CHECK_EQUAL(0, keymap.nb_sym_available());
}


RED_AUTO_TEST_CASE(TestKeymapSymShiftA_FR)
{

    KeymapSym keymap;
    // TODO: use symbolic names for layouts, below 0x040C is french layout
    keymap.init_layout_sym(0x040C);

    // SHIFT_L + a
    check_key_events keys[] = {
        {DOWN, 0x36, 0, 0xffe2}, 
        {DOWN, 16, 0, 'A'},
        {UP,   16, 0, 'A'},
        {UP,   0x36, 0, 0xffe2},
    }; 
    for (auto k: keys){
//        LOG(LOG_INFO, "%u", k.code);
        keymap.event(k.flags, k.code);
        RED_CHECK_EQUAL(k.rflags, keymap.key_flags);
        RED_CHECK_EQUAL(k.rkey, keymap.get_sym());
    }
    RED_CHECK_EQUAL(0, keymap.nb_sym_available());
}

RED_AUTO_TEST_CASE(TestKeymapSymCAPSLOCK_FR)
{

    KeymapSym keymap;
    // TODO: use symbolic names for layouts, below 0x040C is french layout
    keymap.init_layout_sym(0x040C);

    // Le CAPSLOCK n'est pas transmis mais affecte le rang clavier
    // est-ce que c'est bien ce qu'il fut faire ?

    check_key_events keys[] = {
        {DOWN, 0x3A, 4, 0},  // CAPSLOCK ON
        {UP,   0x3A, 4, 0},
        {DOWN, 16,   4, 'A'}, // A
        {UP,   16,   4, 'A'},
        {DOWN, 0x2A, 4, 0xffe1}, // SHIFT + A
        {DOWN, 16, 4, 'a'},
        {UP,   16, 4, 'a'},
        {UP,   0x2A, 4, 0xffe1},
        {DOWN, 0x3A, 0, 0},   // CAPSLOCK OFF
        {UP,   0x3A, 0, 0},
    };
    size_t i = 0;
    for (auto k: keys){
        i++;
        BOOST_TEST_CONTEXT("Loop " << i << ": " 
                   << std::hex << static_cast<uint16_t>(k.flags) << ", "
                   << std::dec << +k.code << ", " 
                   << std::hex << k.rflags << ", " << k.rkey){
            keymap.event(k.flags, k.code);
            RED_CHECK_EQUAL(k.rflags, keymap.key_flags);
            RED_CHECK_EQUAL(k.rkey, keymap.get_sym());
        }
    }
    RED_CHECK_EQUAL(0, keymap.nb_sym_available());
}

//    keymap.event(0, 0x02);
//    // left shift down
//    keymap.event(0, 0x36);
//    // left shift up
//    keymap.event(0xc000, 0x36);
//    // right shift down
//    keymap.event(0, 0x2A);
//    // right shift up
//    keymap.event(0xc000, 0x2A);

//    // CAPSLOCK Down
//    // RDP_INPUT_SCANCODE time=538384316 flags=0000 param1=003a param2=0000
//    RED_CHECK_EQUAL(true, keymap.is_caps_locked());
//    keymap.event(0, 0x3A);
//    RED_CHECK_EQUAL(false, keymap.is_caps_locked());
//    keymap.event(0xC000, 0x3A); // capslock up
//    RED_CHECK_EQUAL(false, keymap.is_caps_locked());

//    RED_CHECK_EQUAL(6, keymap.nb_sym_available());
//    RED_CHECK_EQUAL('A', keymap.get_sym());
//    RED_CHECK_EQUAL('&', keymap.get_sym());
//    RED_CHECK_EQUAL(0xffe2, keymap.get_sym());
//    RED_CHECK_EQUAL(0xffe2, keymap.get_sym());
//    RED_CHECK_EQUAL(0xffe1, keymap.get_sym());
//    RED_CHECK_EQUAL(0xffe1, keymap.get_sym());

//    // Now I hit the 'A' key on french keyboard
//    keymap.event(0, 0x10);
//    RED_CHECK_EQUAL('a', keymap.get_sym());
//    keymap.event(0xc000, 0x10); // up
//    RED_CHECK_EQUAL('a', keymap.get_sym());



