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

#include <memory>

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/working_directory.hpp"
#include "test_only/test_framework/check_img.hpp"
#include "test_only/front/fake_front.hpp"
#include "test_only/core/font.hpp"

#include "core/RDP/capabilities/window.hpp"
#include "RAIL/client_execute.hpp"
#include "mod/internal/close_mod.hpp"
#include "keyboard/keymap2.hpp"
#include "configs/config.hpp"
#include "gdi/graphic_api_forwarder.hpp"


#define IMG_TEST_PATH FIXTURES_PATH "/img_ref/mod/internal/close_mod/"

RED_AUTO_TEST_CASE(TestCloseMod)
{
    ScreenInfo screen_info{800, 600, BitsPerPixel{24}};
    FakeFront front(screen_info);
    WindowListCaps window_list_caps;
    EventContainer events;
    ClientExecute client_execute(events, front.gd(), front, window_list_caps, false);

    Theme theme;

    Keymap2 keymap;
    keymap.init_layout(0x040C);
    keymap.push_kevent(Keymap2::KEVENT_ESC);

    Inifile ini;

    {
        CloseMod d("message", ini, events, front.gd(), front,
            screen_info.width, screen_info.height, Rect(0, 0, 799, 599), client_execute,
            global_font_deja_vu_14(), theme, false);
        d.init();
        d.rdp_input_scancode(0, 0, 0, 0, &keymap);

        RED_CHECK(events.queue.size() == 2);
        events.execute_events(timeval{62, 0},[](int){return false;}, false);
        RED_CHECK_IMG(front, IMG_TEST_PATH "close_mod_1.png");

        RED_CHECK(events.queue.size() == 2);
        events.execute_events(timeval{581, 0},[](int){return false;}, false);
        RED_CHECK_IMG(front, IMG_TEST_PATH "close_mod_2.png");

        RED_CHECK(events.queue.size() == 2);
        events.execute_events(timeval{600, 0},[](int){return false;}, false);
        RED_CHECK(events.queue.size() == 1);

        RED_CHECK_IMG(front, IMG_TEST_PATH "close_mod_3.png");
        RED_CHECK(d.get_mod_signal() == BACK_EVENT_STOP);
    }
    // When Close mod goes out of scope remaining events should be garbaged
    RED_CHECK(events.queue.size() == 1);
    events.execute_events(timeval{600, 0},[](int){return false;}, false);
    RED_CHECK(events.queue.size() == 0);
}

RED_AUTO_TEST_CASE(TestCloseModSelector)
{
    ScreenInfo screen_info{800, 600, BitsPerPixel{24}};
    FakeFront front(screen_info);
    WindowListCaps window_list_caps;
    EventContainer events;
    ClientExecute client_execute(events, front.gd(), front, window_list_caps, false);

    Theme theme;

    Keymap2 keymap;
    keymap.init_layout(0x040C);
    keymap.push_kevent(Keymap2::KEVENT_ESC);

    Inifile ini;
    CloseMod d("message", ini, events, front.gd(), front,
        screen_info.width, screen_info.height, Rect(0, 0, 799, 599), client_execute,
        global_font_deja_vu_14(), theme, true);
    d.init();
    d.rdp_input_scancode(0, 0, 0, 0, &keymap);

    events.execute_events(timeval{1,0},[](int){return false;}, false);

    RED_CHECK_IMG(front, IMG_TEST_PATH "close_mod_selector_1.png");
}

RED_AUTO_TEST_CASE(TestCloseModRail)
{
    struct TestGd : gdi::GraphicApiForwarder<gdi::GraphicApi&>
    {
        using gdi::GraphicApiForwarder<gdi::GraphicApi&>::GraphicApiForwarder;

        void set_pointer(uint16_t cache_idx, const Pointer & cursor, gdi::GraphicApi::SetPointerMode mode) override
        {
            (void)cache_idx;
            (void)mode;
            this->last_cursor = cursor;
        }

        Pointer last_cursor;
    };

    ScreenInfo screen_info{800, 600, BitsPerPixel{24}};
    FakeFront front(screen_info);
    TestGd gd(front.gd());
    WindowListCaps window_list_caps;
    EventContainer events;
    ClientExecute client_execute(events, gd, front, window_list_caps, false);

    Theme theme;

    Keymap2 keymap;
    keymap.init_layout(0x040C);
    keymap.push_kevent(Keymap2::KEVENT_ESC);

    front.add_channel(channel_names::rail, 0, 0);
    client_execute.enable_remote_program(true);

    const Rect widget_rect = client_execute.adjust_rect(
        Rect(0, 0, screen_info.width-1u, screen_info.height-1u));

    Inifile ini;
    CloseMod d("message", ini, events, gd, front,
        screen_info.width, screen_info.height, widget_rect, client_execute,
        global_font_deja_vu_14(), theme, true);
    d.init();
    d.rdp_input_scancode(0, 0, 0, 0, &keymap);

    d.rdp_input_invalidate(Rect{ 0, 0, screen_info.width, screen_info.height });

    events.execute_events(timeval{1,0},[](int){return false;}, false);

    RED_CHECK_IMG(front, IMG_TEST_PATH "close_mod_rail.png");

    // set pointer mod
    gd.set_pointer(0, normal_pointer(), gdi::GraphicApi::SetPointerMode::Insert);

    // move to top border
    d.rdp_input_mouse(MOUSE_FLAG_MOVE, 200, 59, nullptr);
    RED_TEST((gd.last_cursor == size_NS_pointer()));

    // move to widget
    d.rdp_input_mouse(MOUSE_FLAG_MOVE, 200, 100, nullptr);
    RED_TEST((gd.last_cursor == normal_pointer()));
}
