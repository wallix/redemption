/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Meng Tan, Raphael Zhou
*/

#define RED_TEST_MODULE TestWidgetTestMod
#include "system/redemption_unit_tests.hpp"

#define LOGNULL

#include "configs/config.hpp"
#include "core/client_info.hpp"
#include "core/RDP/capabilities/window.hpp"
#include "core/RDP/gcc/userdata/cs_monitor.hpp"
#include "mod/internal/bouncer2_mod.hpp"
#include "mod/internal/widget_test_mod.hpp"
#include "test_only/front/fake_front.hpp"

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
    WindowListCaps window_list_caps;
    ClientExecute client_execute(front, window_list_caps, 0);

    Font font;

    Inifile ini;

    Keymap2 keymap;
    keymap.init_layout(info.keylayout);

    std::unique_ptr<mod_api> managed_mod(
        new Bouncer2Mod(front, 800, 600, font, true));

    GCC::UserData::CSMonitor cs_monitor;

    cs_monitor.monitorCount = 0;

    WidgetTestMod d(ini, front, 800, 600, Rect(0, 0, 799, 599),
        std::move(managed_mod), client_execute, cs_monitor);
    d.draw_event(100001, front);
/*
    keymap.push_kevent(Keymap2::KEVENT_ENTER); // enterto validate
    d.rdp_input_scancode(0, 0, 0, 0, &keymap);
*/

/*
    const char * res = ini.context_get_value(AUTHID_ACCEPT_MESSAGE);
    LOG(LOG_INFO, "%s\n", res);
    RED_CHECK(0 == strcmp("True", res));
*/
}
