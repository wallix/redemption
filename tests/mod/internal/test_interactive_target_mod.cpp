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
#include "mod/internal/interactive_target_mod.hpp"
#include "keyboard/keymap.hpp"
#include "keyboard/keylayouts.hpp"
#include "test_only/front/fake_front.hpp"
#include "test_only/core/font.hpp"
#include "core/events.hpp"

// TODO "Need more tests, with or without device/login/password asking, "

RED_AUTO_TEST_CASE(TestInteractiveTargetMod)
{
    ScreenInfo screen_info{800, 600, BitsPerPixel{24}};
    FakeFront front(screen_info);
    WindowListCaps window_list_caps;
    EventContainer events;
    ClientExecute client_execute(events.get_time_base(), front.gd(), front, window_list_caps, false);

    Inifile ini;
    Theme theme;
    ini.set_acl<cfg::context::target_host>("somehost");
    ini.set_acl<cfg::globals::target_user>("someuser");
    ini.ask<cfg::context::target_password>();

    Keymap keymap(*find_layout_by_id(KeyLayout::KbdId(0x040C)));

    InteractiveTargetMod d(ini, front.gd(), front, 800, 600, Rect(0, 0, 799, 599), client_execute,
                           global_font(), theme);
    d.init();
    keymap.event(Keymap::KbdFlags(), Keymap::Scancode(0x1c)); // enter
    d.rdp_input_scancode(Keymap::KbdFlags(), Keymap::Scancode(0x1c), 0, keymap);

    RED_CHECK(ini.get<cfg::context::display_message>());
}


RED_AUTO_TEST_CASE(TestInteractiveTargetModReject)
{
    ScreenInfo screen_info{800, 600, BitsPerPixel{24}};
    FakeFront front(screen_info);
    WindowListCaps window_list_caps;
    EventContainer events;
    ClientExecute client_execute(events.get_time_base(), front.gd(), front, window_list_caps, false);

    Inifile ini;
    Theme theme;

    Keymap keymap(*find_layout_by_id(KeyLayout::KbdId(0x040C)));

    InteractiveTargetMod d(ini, front.gd(), front, screen_info.width, screen_info.height, Rect(0, 0, 799, 599),
                           client_execute, global_font(), theme);
    d.init();
    keymap.event(Keymap::KbdFlags(), Keymap::Scancode(0x01)); // esc
    d.rdp_input_scancode(Keymap::KbdFlags(), Keymap::Scancode(0x01), 0, keymap);

    RED_CHECK(not ini.get<cfg::context::display_message>());
}

RED_AUTO_TEST_CASE(TestInteractiveTargetModChallenge)
{
    ScreenInfo screen_info{800, 600, BitsPerPixel{24}};
    FakeFront front(screen_info);
    WindowListCaps window_list_caps;
    EventContainer events;
    ClientExecute client_execute(events.get_time_base(), front.gd(), front, window_list_caps, false);

    Inifile ini;
    Theme theme;
    ini.set_acl<cfg::context::target_host>("somehost");
    ini.set_acl<cfg::globals::target_user>("someuser");
    ini.ask<cfg::context::target_password>();

    InteractiveTargetMod d(ini, front.gd(), front, screen_info.width, screen_info.height, Rect(0, 0, 799, 599),
                           client_execute, global_font(), theme);
    d.init();

    Keymap keymap(*find_layout_by_id(KeyLayout::KbdId(0x040C)));

    auto rdp_input_scancode = [&](Keymap::KeyCode keycode){
        auto ukeycode = underlying_cast(keycode);
        auto scancode = Keymap::Scancode(ukeycode & 0x7F);
        auto flags = (ukeycode & 0x80) ? Keymap::KbdFlags::Extended : Keymap::KbdFlags();
        keymap.event(flags, scancode);
        d.rdp_input_scancode(flags, scancode, 0, keymap);
    };

    rdp_input_scancode(Keymap::KeyCode(0x11)); // 'z'
    rdp_input_scancode(Keymap::KeyCode(0x12)); // 'e'
    rdp_input_scancode(Keymap::KeyCode(0x10)); // 'a'
    rdp_input_scancode(Keymap::KeyCode(0x10)); // 'a'
    rdp_input_scancode(Keymap::KeyCode(0x10)); // 'a'
    rdp_input_scancode(Keymap::KeyCode(0x10)); // 'a'
    rdp_input_scancode(Keymap::KeyCode(0x1c)); // enter

    RED_CHECK_EQUAL("zeaaaa", ini.get<cfg::context::target_password>());
    RED_CHECK(ini.get<cfg::context::display_message>());
}
