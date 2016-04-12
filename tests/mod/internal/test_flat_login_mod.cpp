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
#define BOOST_TEST_MODULE TestFlatLoginMod
#include <boost/test/auto_unit_test.hpp>

#undef SHARE_PATH
#define SHARE_PATH FIXTURES_PATH

#define LOGNULL
//#define LOGPRINT

#include "mod/internal/flat_login_mod.hpp"
#include "../../front/fake_front.hpp"

BOOST_AUTO_TEST_CASE(TestDialogMod)
{
    ClientInfo info;
    info.keylayout = 0x040C;
    info.console_session = 0;
    info.brush_cache_code = 0;
    info.bpp = 24;
    info.width = 800;
    info.height = 600;

    FakeFront front(info, 0);

    Inifile ini;

    Keymap2 keymap;
    keymap.init_layout(info.keylayout);
    keymap.push_kevent(Keymap2::KEVENT_ENTER);

    FlatLoginMod d(ini, "user", "pass", front, 800, 600, static_cast<time_t>(100000));
    d.draw_event(100001);

    BOOST_CHECK_EQUAL(BACK_EVENT_NONE, d.get_event().signal);

    d.rdp_input_scancode(0, 0, 0, 0, &keymap);

    BOOST_CHECK_EQUAL(ini.get<cfg::globals::auth_user>(), "user");
    BOOST_CHECK_EQUAL(ini.get<cfg::context::password>(), "pass");
}

BOOST_AUTO_TEST_CASE(TestDialogMod1)
{
    ClientInfo info;
    info.keylayout = 0x040C;
    info.console_session = 0;
    info.brush_cache_code = 0;
    info.bpp = 24;
    info.width = 800;
    info.height = 600;

    FakeFront front(info, 0);

    Inifile ini;

    Keymap2 keymap;
    keymap.init_layout(info.keylayout);
    keymap.push_kevent(Keymap2::KEVENT_ENTER);

    FlatLoginMod d(ini, "user", "pass", front, 800, 600, static_cast<time_t>(100000));
    d.draw_event(100001);

    BOOST_CHECK_EQUAL(BACK_EVENT_NONE, d.get_event().signal);

    d.draw_event(100601);

    BOOST_CHECK_EQUAL(BACK_EVENT_STOP, d.get_event().signal);
}
