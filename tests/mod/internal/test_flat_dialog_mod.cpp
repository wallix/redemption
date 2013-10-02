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
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean, Meng Tan

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestFlatDialogMod
#include <boost/test/auto_unit_test.hpp>

#undef FIXTURES_PATH
#define FIXTURES_PATH "./tests/fixtures"
#undef SHARE_PATH
#define SHARE_PATH "./tests/fixtures"

#define LOGPRINT
#include "log.hpp"

#include "internal/flat_dialog_mod.hpp"
#include "../../front/fake_front.hpp"

BOOST_AUTO_TEST_CASE(TestDialogMod)
{
    ClientInfo info(1, true, true);
    info.keylayout = 0x040C;
    info.console_session = 0;
    info.brush_cache_code = 0;
    info.bpp = 24;
    info.width = 800;
    info.height = 600;

    FakeFront front(info, 0);

    Inifile             ini;

    Keymap2 keymap;
    keymap.init_layout(info.keylayout);

    FlatDialogMod d(ini, front, 800, 600, "Title", "Hello, World", "OK", 0);
    keymap.push_kevent(Keymap2::KEVENT_TAB); // move focus to OK
    d.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.push_kevent(Keymap2::KEVENT_ENTER); // enterto validate
    d.rdp_input_scancode(0, 0, 0, 0, &keymap);

    const char * res = ini.context_get_value(AUTHID_ACCEPT_MESSAGE);
    LOG(LOG_INFO, "%s\n", res);
    BOOST_CHECK(0 == strcmp("True", res));
}


BOOST_AUTO_TEST_CASE(TestDialogModReject)
{
    ClientInfo info(1, true, true);
    info.keylayout = 0x040C;
    info.console_session = 0;
    info.brush_cache_code = 0;
    info.bpp = 24;
    info.width = 800;
    info.height = 600;

    FakeFront front(info, 0);

    Inifile             ini;

    Keymap2 keymap;
    keymap.init_layout(info.keylayout);

    FlatDialogMod d(ini, front, 800, 600, "Title", "Hello, World", "Cancel", 0);
    keymap.push_kevent(Keymap2::KEVENT_ESC);
    d.rdp_input_scancode(0, 0, 0, 0, &keymap);

    const char * res = ini.context_get_value(AUTHID_ACCEPT_MESSAGE);
    LOG(LOG_INFO, "%s\n", res);
    BOOST_CHECK(0 == strcmp("False", res));
}
