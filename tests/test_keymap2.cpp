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

}
