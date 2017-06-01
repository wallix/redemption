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

#define RED_TEST_MODULE TestFlatWabCloseMod
#include "system/redemption_unit_tests.hpp"

#define LOGNULL
//#define LOGPRINT

#include "configs/config.hpp"
#include "core/RDP/capabilities/window.hpp"
#include "mod/internal/client_execute.hpp"
#include "mod/internal/flat_wab_close_mod.hpp"
#include "test_only/front/fake_front.hpp"

RED_AUTO_TEST_CASE(TestWabCloseMod)
{
    ClientInfo info;
    info.keylayout = 0x040C;
    info.console_session = 0;
    info.brush_cache_code = 0;
    info.bpp = 24;
    info.width = 800;
    info.height = 600;

    FakeFront front(info, 0);
    WindowListCaps window_list_caps;
    ClientExecute client_execute(front, window_list_caps, 0);

    Inifile ini;

    Keymap2 keymap;
    keymap.init_layout(info.keylayout);
    keymap.push_kevent(Keymap2::KEVENT_ESC);

    FlatWabCloseMod d(ini, front, 800, 600, Rect(0, 0, 799, 599), static_cast<time_t>(100000), client_execute, true);
    d.draw_event(100001, front);
    d.rdp_input_scancode(0, 0, 0, 0, &keymap);
}

RED_AUTO_TEST_CASE(TestWabCloseMod2)
{
    ClientInfo info;
    info.keylayout = 0x040C;
    info.console_session = 0;
    info.brush_cache_code = 0;
    info.bpp = 24;
    info.width = 2048;
    info.height = 1536;

    FakeFront front(info, 0);
    WindowListCaps window_list_caps;
    ClientExecute client_execute(front, window_list_caps, 0);

    Inifile ini;

    Keymap2 keymap;
    keymap.init_layout(info.keylayout);
    keymap.push_kevent(Keymap2::KEVENT_ESC);

    FlatWabCloseMod d(ini, front, 2048, 1536, Rect(1024, 768, 1023, 767), static_cast<time_t>(100000), client_execute, true);
    d.draw_event(100001, front);
    d.rdp_input_scancode(0, 0, 0, 0, &keymap);
}
