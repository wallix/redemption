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

#define UNIT_TEST_MODULE TestInteractiveTargetMod
#include "system/redemption_unit_tests.hpp"


#define LOGNULL

#include "mod/internal/interactive_target_mod.hpp"
#include "../../front/fake_front.hpp"

// TODO "Need more tests, with or without device/login/password asking, "

RED_AUTO_TEST_CASE(TestDialogMod)
{
    ClientInfo info;
    info.keylayout = 0x040C;
    info.console_session = 0;
    info.brush_cache_code = 0;
    info.bpp = 24;
    info.width = 800;
    info.height = 600;

    FakeFront front(info, 0);
    ClientExecute client_execute(front, 0);

    Inifile             ini;
    ini.set_acl<cfg::context::target_host>("somehost");
    ini.set_acl<cfg::globals::target_user>("someuser");
    ini.ask<cfg::context::target_password>();

    Keymap2 keymap;
    keymap.init_layout(info.keylayout);

    InteractiveTargetMod d(ini, front, 800, 600, Rect(0, 0, 799, 599), client_execute);
    keymap.push_kevent(Keymap2::KEVENT_ENTER); // enter to validate
    d.rdp_input_scancode(0, 0, 0, 0, &keymap);

    RED_CHECK_EQUAL(true, ini.get<cfg::context::display_message>());
}


RED_AUTO_TEST_CASE(TestDialogModReject)
{
    ClientInfo info;
    info.keylayout = 0x040C;
    info.console_session = 0;
    info.brush_cache_code = 0;
    info.bpp = 24;
    info.width = 800;
    info.height = 600;

    FakeFront front(info, 0);
    ClientExecute client_execute(front, 0);

    Inifile             ini;

    Keymap2 keymap;
    keymap.init_layout(info.keylayout);

    InteractiveTargetMod d(ini, front, 800, 600, Rect(0, 0, 799, 599), client_execute);
    keymap.push_kevent(Keymap2::KEVENT_ESC);
    d.rdp_input_scancode(0, 0, 0, 0, &keymap);

    RED_CHECK_EQUAL(false, ini.get<cfg::context::display_message>());
}

RED_AUTO_TEST_CASE(TestDialogModChallenge)
{
    ClientInfo info;
    info.keylayout = 0x040C;
    info.console_session = 0;
    info.brush_cache_code = 0;
    info.bpp = 24;
    info.width = 800;
    info.height = 600;

    FakeFront front(info, 0);
    ClientExecute client_execute(front, 0);

    Inifile ini;
    ini.set_acl<cfg::context::target_host>("somehost");
    ini.set_acl<cfg::globals::target_user>("someuser");
    ini.ask<cfg::context::target_password>();

    Keymap2 keymap;
    keymap.init_layout(info.keylayout);

    InteractiveTargetMod d(ini, front, 800, 600, Rect(0, 0, 799, 599), client_execute);

    bool    ctrl_alt_del;

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

    RED_CHECK_EQUAL("zeaaaa", ini.get<cfg::context::target_password>());
    RED_CHECK_EQUAL(true, ini.get<cfg::context::display_message>());
}

RED_AUTO_TEST_CASE(TestDialogModChallenge2)
{
    ClientInfo info;
    info.keylayout = 0x040C;
    info.console_session = 0;
    info.brush_cache_code = 0;
    info.bpp = 24;
    info.width = 1600;
    info.height = 1200;

    FakeFront front(info, 0);
    ClientExecute client_execute(front, 0);

    Inifile ini;
    ini.set_acl<cfg::context::target_host>("somehost");
    ini.set_acl<cfg::globals::target_user>("someuser");
    ini.ask<cfg::context::target_password>();

    Keymap2 keymap;
    keymap.init_layout(info.keylayout);

    InteractiveTargetMod d(ini, front, 1600, 1200, Rect(800, 600, 799, 599), client_execute);

    bool    ctrl_alt_del;

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

    RED_CHECK_EQUAL("zeaaaa", ini.get<cfg::context::target_password>());
    RED_CHECK_EQUAL(true, ini.get<cfg::context::display_message>());
}
