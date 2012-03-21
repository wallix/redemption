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

   uint32_t key = keymap.last_char_down();
   BOOST_CHECK_EQUAL('A', key);

   keyboardFlags = keymap.KBDFLAGS_DOWN|keymap.KBDFLAGS_RELEASE ; // key is not extended, key was down, key goes up
   keyCode = 54 ; // key is left shift
   keymap.event(keyboardFlags, keyCode);

   BOOST_CHECK_EQUAL(false, keymap.is_shift_pressed());
   BOOST_CHECK_EQUAL(false, keymap.is_left_shift_pressed());
   BOOST_CHECK_EQUAL(false, keymap.is_right_shift_pressed());

   // shift was released, but not A (last char down goes 'a' for autorepeat)
   key = keymap.last_char_down();
   BOOST_CHECK_EQUAL('A', key);

   keyboardFlags = keymap.KBDFLAGS_DOWN|keymap.KBDFLAGS_RELEASE ; // key is not extended, key was down, key goes up
   keyCode = 16 ; // key is 'A'
   keymap.event(keyboardFlags, keyCode);

   key = keymap.last_char_down();
   BOOST_CHECK_EQUAL(0, key);


   keyboardFlags = 0 ; // key is not extended, key was up, key goes down
   keyCode = 16 ; // key is 'A'
   keymap.event(keyboardFlags, keyCode);
   key = keymap.last_char_down();
   BOOST_CHECK_EQUAL('a', key);

    // CAPSLOCK Down
    // RDP_INPUT_SCANCODE time=538384316 flags=0000 param1=003a param2=0000

    // CAPSLOCK Down Autorepeat
    // RDP_INPUT_SCANCODE time=538384831 flags=4000 param1=003a param2=0000
    // RDP_INPUT_SCANCODE time=538384847 flags=4000 param1=003a param2=0000
    // RDP_INPUT_SCANCODE time=538384878 flags=4000 param1=003a param2=0000

    // CAPSLOCK Up
    // RDP_INPUT_SCANCODE time=538384894 flags=c000 param1=003a param2=0000

    // 'a' down
    //RDP_INPUT_SCANCODE time=538779077 flags=0000 param1=0010 param2=0000

    // 'a' down autorepeat
    //RDP_INPUT_SCANCODE time=538779545 flags=0000 param1=0010 param2=0000
    //RDP_INPUT_SCANCODE time=538779576 flags=0000 param1=0010 param2=0000
    //RDP_INPUT_SCANCODE time=538779607 flags=0000 param1=0010 param2=0000
    //RDP_INPUT_SCANCODE time=538779639 flags=0000 param1=0010 param2=0000
    //RDP_INPUT_SCANCODE time=538779654 flags=0000 param1=0010 param2=0000
    //RDP_INPUT_SCANCODE time=538779685 flags=0000 param1=0010 param2=0000
    //RDP_INPUT_SCANCODE time=538779717 flags=0000 param1=0010 param2=0000
    //RDP_INPUT_SCANCODE time=538779732 flags=0000 param1=0010 param2=0000
    //RDP_INPUT_SCANCODE time=538779763 flags=0000 param1=0010 param2=0000
    //RDP_INPUT_SCANCODE time=538779795 flags=0000 param1=0010 param2=0000
    //RDP_INPUT_SCANCODE time=538779826 flags=0000 param1=0010 param2=0000
    //RDP_INPUT_SCANCODE time=538779857 flags=0000 param1=0010 param2=0000

    // 'a' UP
    //RDP_INPUT_SCANCODE time=538779873 flags=8000 param1=0010 param2=0000


    // altgr down
    // RDP_INPUT_SCANCODE time=538966481 flags=0000 param1=001d param2=0000 -> CTRL
    // altgr down autorepeat
    // RDP_INPUT_SCANCODE time=538966481 flags=0100 param1=0038 param2=0000 -> ALT
    // RDP_INPUT_SCANCODE time=538966980 flags=4000 param1=001d param2=0000
    // RDP_INPUT_SCANCODE time=538966980 flags=4100 param1=0038 param2=0000
    // RDP_INPUT_SCANCODE time=538967027 flags=4000 param1=001d param2=0000
    // RDP_INPUT_SCANCODE time=538967027 flags=4100 param1=0038 param2=0000
    // RDP_INPUT_SCANCODE time=538967043 flags=4000 param1=001d param2=0000
    // RDP_INPUT_SCANCODE time=538967043 flags=4100 param1=0038 param2=0000
    // RDP_INPUT_SCANCODE time=538967074 flags=4000 param1=001d param2=0000
    // RDP_INPUT_SCANCODE time=538967074 flags=4100 param1=0038 param2=0000
    // RDP_INPUT_SCANCODE time=538967105 flags=4000 param1=001d param2=0000
    // RDP_INPUT_SCANCODE time=538967105 flags=4100 param1=0038 param2=0000
    // RDP_INPUT_SCANCODE time=538967136 flags=4000 param1=001d param2=0000
    // RDP_INPUT_SCANCODE time=538967136 flags=4100 param1=0038 param2=0000
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
