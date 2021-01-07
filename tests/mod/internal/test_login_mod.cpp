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

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "configs/config.hpp"
#include "core/RDP/capabilities/window.hpp"
#include "RAIL/client_execute.hpp"
#include "mod/internal/login_mod.hpp"
#include "gdi/graphic_api.hpp"
#include "keyboard/keymap2.hpp"
#include "test_only/front/fake_front.hpp"
#include "test_only/core/font.hpp"
#include "core/events.hpp"

RED_AUTO_TEST_CASE(TestLoginMod)
{
    ScreenInfo screen_info{800, 600, BitsPerPixel{24}};
    FakeFront front(screen_info);
    WindowListCaps window_list_caps;
    EventContainer events;
    ClientExecute client_execute(events, front.gd(), front, window_list_caps, false);

    Inifile ini;
    Theme theme;

    Keymap2 keymap;
    keymap.init_layout(0x040C);
    keymap.push_kevent(Keymap2::KEVENT_ENTER);

    RED_CHECK_NE(ini.get<cfg::globals::auth_user>(), "user");
    RED_CHECK_NE(ini.get<cfg::context::password>(), "pass");

    LoginMod d(ini, events, "user", "pass", front.gd(), front, screen_info.width, screen_info.height,
        Rect(0, 0, 799, 599), client_execute, global_font(), theme);
    d.init();

    d.rdp_input_scancode(0, 0, 0, 0, &keymap);

    RED_CHECK_EQUAL(ini.get<cfg::globals::auth_user>(), "user");
    RED_CHECK_EQUAL(ini.get<cfg::context::password>(), "pass");
}

RED_AUTO_TEST_CASE(TestLoginMod2)
{
    ScreenInfo screen_info{2048, 1536, BitsPerPixel{24}};
    FakeFront front(screen_info);
    WindowListCaps window_list_caps;
    EventContainer events;
    ClientExecute client_execute(events, front.gd(), front, window_list_caps, false);

    Inifile ini;
    Theme theme;

    Keymap2 keymap;
    keymap.init_layout(0x040C);
    keymap.push_kevent(Keymap2::KEVENT_ENTER);

    ini.set<cfg::globals::authentication_timeout>(std::chrono::seconds(1));

    LoginMod d(ini, events, "user", "pass", front.gd(), front, screen_info.width, screen_info.height,
        Rect(1024, 768, 1023, 767), client_execute, global_font(), theme);
    d.init();

    events.execute_events(timeval{0,0},[](int){return false;}, false);
    RED_CHECK_EQUAL(BACK_EVENT_NONE, d.get_mod_signal());

    events.execute_events(timeval{2,1},[](int){return false;}, false);
    RED_CHECK_EQUAL(BACK_EVENT_STOP, d.get_mod_signal());
}
