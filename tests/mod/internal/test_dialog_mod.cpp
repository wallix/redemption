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
#include "mod/internal/dialog_mod.hpp"
#include "keyboard/keymap2.hpp"
#include "test_only/front/fake_front.hpp"
#include "test_only/core/font.hpp"

RED_AUTO_TEST_CASE(TestDialogMod)
{
    ScreenInfo screen_info{800, 600, BitsPerPixel{24}};
    FakeFront front(screen_info);
    WindowListCaps window_list_caps;
    TimeBase time_base;
    TimerContainer timer_events_;
    GraphicEventContainer graphic_events_;
    ClientExecute client_execute(time_base, timer_events_, front.gd(), front, window_list_caps, false);

    Inifile ini;
    Theme theme;

    Keymap2 keymap;
    keymap.init_layout(0x040C);

    DialogMod d(ini, time_base, timer_events_, graphic_events_, front.gd(), front, screen_info.width, screen_info.height,
        Rect(0, 0, 799, 599), "Title", "Hello, World", "OK", client_execute, global_font(), theme);
    keymap.push_kevent(Keymap2::KEVENT_ENTER); // enterto validate
    d.rdp_input_scancode(0, 0, 0, 0, &keymap);

    RED_CHECK(ini.get<cfg::context::accept_message>());
}


RED_AUTO_TEST_CASE(TestDialogModReject)
{
    ScreenInfo screen_info{800, 600, BitsPerPixel{24}};
    FakeFront front(screen_info);
    WindowListCaps window_list_caps;
    TimeBase time_base;
    TimerContainer timer_events_;
    GraphicEventContainer graphic_events_;
    ClientExecute client_execute(time_base, timer_events_, front.gd(), front, window_list_caps, false);

    Inifile ini;
    Theme theme;

    Keymap2 keymap;
    keymap.init_layout(0x040C);

    DialogMod d(ini, time_base, timer_events_, graphic_events_, front.gd(), front, 800, 600, Rect(0, 0, 799, 599),
        "Title", "Hello, World", "Cancel", client_execute, global_font(), theme);
    keymap.push_kevent(Keymap2::KEVENT_ESC);
    d.rdp_input_scancode(0, 0, 0, 0, &keymap);

    RED_CHECK(!ini.get<cfg::context::accept_message>());
}

RED_AUTO_TEST_CASE(TestDialogModChallenge)
{
    ScreenInfo screen_info{800, 600, BitsPerPixel{24}};
    FakeFront front(screen_info);
    WindowListCaps window_list_caps;
    TimeBase time_base;
    TimerContainer timer_events_;
    GraphicEventContainer graphic_events_;
    ClientExecute client_execute(time_base, timer_events_, front.gd(), front, window_list_caps, false);

    Inifile ini;
    Theme theme;

    Keymap2 keymap;
    keymap.init_layout(0x040C);

    DialogMod d(ini, time_base, timer_events_, graphic_events_, front.gd(), front, 800, 600, Rect(0, 0, 799, 599),
        "Title", "Hello, World", "Cancel", client_execute, global_font(), theme, CHALLENGE_ECHO);


    bool ctrl_alt_del;

    uint16_t keyboardFlags = 0 ;
    uint16_t keyCode = 16; // key is 'a'

    keymap.event(keyboardFlags, keyCode + 1, ctrl_alt_del);
    d.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keyboardFlags, keyCode + 2, ctrl_alt_del);
    d.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keyboardFlags, keyCode, ctrl_alt_del);
    d.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keyboardFlags, keyCode, ctrl_alt_del);
    d.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keyboardFlags, keyCode, ctrl_alt_del);
    d.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keyboardFlags, keyCode, ctrl_alt_del);
    d.rdp_input_scancode(0, 0, 0, 0, &keymap);

    keymap.push_kevent(Keymap2::KEVENT_ENTER);
    d.rdp_input_scancode(0, 0, 0, 0, &keymap);

    RED_CHECK_EQUAL("zeaaaa", ini.get<cfg::context::password>());
}

RED_AUTO_TEST_CASE(TestDialogModChallenge2)
{
    ScreenInfo screen_info{1600, 1200, BitsPerPixel{24}};
    FakeFront front(screen_info);
    WindowListCaps window_list_caps;
    TimeBase time_base;
    TimerContainer timer_events_;
    GraphicEventContainer graphic_events_;
    ClientExecute client_execute(time_base, timer_events_, front.gd(), front, window_list_caps, false);

    Inifile ini;
    Theme theme;

    Keymap2 keymap;
    keymap.init_layout(0x040C);

    DialogMod d(ini, time_base, timer_events_, graphic_events_, front.gd(), front, 1600, 1200, Rect(800, 600, 799, 599),
        "Title", "Hello, World", "Cancel", client_execute, global_font(), theme, CHALLENGE_ECHO);

    bool ctrl_alt_del;

    uint16_t keyboardFlags = 0 ;
    uint16_t keyCode = 16; // key is 'a'

    keymap.event(keyboardFlags, keyCode + 1, ctrl_alt_del);
    d.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keyboardFlags, keyCode + 2, ctrl_alt_del);
    d.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keyboardFlags, keyCode, ctrl_alt_del);
    d.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keyboardFlags, keyCode, ctrl_alt_del);
    d.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keyboardFlags, keyCode, ctrl_alt_del);
    d.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keyboardFlags, keyCode, ctrl_alt_del);
    d.rdp_input_scancode(0, 0, 0, 0, &keymap);

    keymap.push_kevent(Keymap2::KEVENT_ENTER);
    d.rdp_input_scancode(0, 0, 0, 0, &keymap);

    RED_CHECK_EQUAL("zeaaaa", ini.get<cfg::context::password>());
}
