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

#define RED_TEST_MODULE TestFlatLoginMod
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "configs/config.hpp"
#include "core/RDP/capabilities/window.hpp"
#include "RAIL/client_execute.hpp"
#include "mod/internal/flat_login_mod.hpp"
#include "keyboard/keymap2.hpp"
#include "test_only/front/fake_front.hpp"
#include "test_only/core/font.hpp"


RED_AUTO_TEST_CASE(TestDialogMod)
{
    ScreenInfo screen_info{BitsPerPixel{24}, 800, 600};
    FakeFront front(screen_info);
    WindowListCaps window_list_caps;
    SessionReactor session_reactor;
    ClientExecute client_execute(session_reactor, front, window_list_caps, 0);

    Inifile ini;
    Theme theme;

    Keymap2 keymap;
    keymap.init_layout(0x040C);
    keymap.push_kevent(Keymap2::KEVENT_ENTER);

    FlatLoginMod d(ini, session_reactor, "user", "pass", front, screen_info.width, screen_info.height,
        Rect(0, 0, 799, 599), static_cast<time_t>(100000), client_execute, global_font(), theme);
    d.draw_event(100001, front);

// TODO    RED_CHECK_EQUAL(BACK_EVENT_NONE, d.get_event().signal);

    d.rdp_input_scancode(0, 0, 0, 0, &keymap);

    RED_CHECK_EQUAL(ini.get<cfg::globals::auth_user>(), "user");
    RED_CHECK_EQUAL(ini.get<cfg::context::password>(), "pass");
}

RED_AUTO_TEST_CASE(TestDialogMod1)
{
    ScreenInfo screen_info{BitsPerPixel{24}, 800, 600};
    FakeFront front(screen_info);
    WindowListCaps window_list_caps;
    SessionReactor session_reactor;
    ClientExecute client_execute(session_reactor, front, window_list_caps, 0);

    Inifile ini;
    Theme theme;

    Keymap2 keymap;
    keymap.init_layout(0x040C);
    keymap.push_kevent(Keymap2::KEVENT_ENTER);

    FlatLoginMod d(ini, session_reactor, "user", "pass", front, screen_info.width, screen_info.height,
        Rect(0, 0, 799, 599), static_cast<time_t>(100000), client_execute, global_font(), theme);
    d.draw_event(100001, front);

// TODO    RED_CHECK_EQUAL(BACK_EVENT_NONE, d.get_event().signal);

    d.draw_event(100601, front);

// TODO    RED_CHECK_EQUAL(BACK_EVENT_STOP, d.get_event().signal);
}

RED_AUTO_TEST_CASE(TestDialogMod2)
{
    ScreenInfo screen_info{BitsPerPixel{24}, 2048, 1536};
    FakeFront front(screen_info);
    WindowListCaps window_list_caps;
    SessionReactor session_reactor;
    ClientExecute client_execute(session_reactor, front, window_list_caps, 0);

    Inifile ini;
    Theme theme;

    Keymap2 keymap;
    keymap.init_layout(0x040C);
    keymap.push_kevent(Keymap2::KEVENT_ENTER);

    FlatLoginMod d(ini, session_reactor, "user", "pass", front, screen_info.width, screen_info.height,
        Rect(1024, 768, 1023, 767), static_cast<time_t>(100000), client_execute, global_font(), theme);
    d.draw_event(100001, front);

// TODO    RED_CHECK_EQUAL(BACK_EVENT_NONE, d.get_event().signal);

    d.draw_event(100601, front);

// TODO    RED_CHECK_EQUAL(BACK_EVENT_STOP, d.get_event().signal);
}
