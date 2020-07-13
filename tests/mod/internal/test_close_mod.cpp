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
#include "core/font.hpp"

#include "acl/gd_provider.hpp"
#include "core/front_api.hpp"
#include "utils/image_data_view.hpp"
#include "core/channel_list.hpp"
#include "core/app_path.hpp"

#include "core/RDP/capabilities/window.hpp"
#include "RAIL/client_execute.hpp"
#include "mod/internal/close_mod.hpp"
#include "keyboard/keymap2.hpp"

#include "utils/png.hpp"
#include "utils/sugar/cast.hpp"

#include "test_only/check_sig.hpp"
#include "test_only/gdi/test_graphic.hpp"
#include "configs/config.hpp"
#include "core/events.hpp"

class FakeFront : public FrontAPI
{
public:
    FakeFront(ScreenInfo& screen_info) :
        data{screen_info, TestGraphic(screen_info.width, screen_info.height), {}}
    {}
    ~FakeFront() = default;

    bool can_be_start_capture() override { return false; }
    bool must_be_stop_capture() override { return false; }
    bool is_capture_in_progress() const override { return false; }
    const CHANNELS::ChannelDefArray & get_channel_list() const override { return data.cl; }
    void send_to_channel( const CHANNELS::ChannelDef & /*channel*/, bytes_view /*chunk_data*/
                        , std::size_t /*total_length*/, int /*flags*/) override {}
    ResizeResult server_resize(ScreenInfo screen_server) override
    {
        this->data.gd.resize(screen_server.width, screen_server.height);
        this->data.screen_info.bpp = screen_server.bpp;
        return ResizeResult::done;
    }
    void update_pointer_position(uint16_t /*x*/, uint16_t /*y*/) override {}
    operator ConstImageDataView() const { return this->data.gd; }
    gdi::GraphicApi& gd() noexcept { return this->data.gd; }

private:
    struct D
    {
        ScreenInfo& screen_info;
        TestGraphic gd;
        CHANNELS::ChannelDefArray cl;
    } data;
};


RED_AUTO_TEST_CASE(TestCloseMod)
{
    ScreenInfo screen_info{800, 600, BitsPerPixel{24}};
    FakeFront front(screen_info);
    GdForwarder gd_forwarder(front.gd());
    WindowListCaps window_list_caps;
    TimeBase time_base({0,0});
    EventContainer events;
    ClientExecute client_execute(time_base, events, front.gd(), front, window_list_caps, false);

    Theme theme;

    Keymap2 keymap;
    keymap.init_layout(0x040C);
    keymap.push_kevent(Keymap2::KEVENT_ESC);

    Font glyphs = Font(app_path(AppPath::DefaultFontFile), false);

    InifileWrapper ini;

    Inifile ini2;
    ini2.set<cfg::context::auth_error_message>("toto");
    RED_CHECK(!ini2.get<cfg::context::auth_error_message>().empty());
    auto & tmp = ini2.get_mutable_ref<cfg::context::auth_error_message>();
    ini2.set<cfg::context::auth_error_message>("toto");
    tmp = "";
    RED_CHECK(ini2.get<cfg::context::auth_error_message>().empty());

    {
        CloseMod d("message", ini.get_ini(), time_base, events, gd_forwarder, front,
            screen_info.width, screen_info.height, Rect(0, 0, 799, 599), client_execute,
            glyphs, theme, false);
        d.init();
        d.rdp_input_scancode(0, 0, 0, 0, &keymap);

        RED_CHECK(events.queue.size() == 2);
        events.execute_events(timeval{62, 0},[](int){return false;});
    //    ::dump_png24("TestCloseMod.png", ConstImageDataView(front), true);
        RED_CHECK_SIG(ConstImageDataView(front),
            "\x11\xe6\x7f\xdb\x20\x5a\x01\x1e\x74\x58\xf3\xb8\x44\xe9\xeb\x51\x30\xd0\x73\x3d");

        RED_CHECK(events.queue.size() == 2);
        events.execute_events(timeval{580, 0},[](int){return false;});
    //    ::dump_png24("TestCloseMod.png", ConstImageDataView(front), true);
        RED_CHECK_SIG(ConstImageDataView(front),
            "\xf1\x93\xd8\x9f\x7a\x00\x14\x8b\x42\xd8\x4b\x70\x4d\x7c\x96\xdc\x7f\x92\xb2\xe0");

        RED_CHECK(events.queue.size() == 2);
        events.execute_events(timeval{600, 0},[](int){return false;});
        RED_CHECK(events.queue.size() == 1);

//        ::dump_png24("TestCloseModFin.png", ConstImageDataView(front), true);
        RED_CHECK_SIG(ConstImageDataView(front),
            "\x3d\x73\xd1\xa3\x50\x06\x05\x5e\x37\xba\xab\x46\xb0\x37\x78\x53\xd6\xdf\xd3\x7c");
        RED_CHECK(d.get_mod_signal() == BACK_EVENT_STOP);
    }
    // When Close mod goes out of scope remaining events should be garbaged
    RED_CHECK(events.queue.size() == 1);
    events.execute_events(timeval{600, 0},[](int){return false;});
    RED_CHECK(events.queue.size() == 0);
}

RED_AUTO_TEST_CASE(TestCloseModSelector)
{
    ScreenInfo screen_info{800, 600, BitsPerPixel{24}};
    FakeFront front(screen_info);
    GdForwarder gd_forwarder(front.gd());
    WindowListCaps window_list_caps;
    TimeBase time_base({0,0});
    EventContainer events;
    ClientExecute client_execute(time_base, events, front.gd(), front, window_list_caps, false);

    Theme theme;

    Keymap2 keymap;
    keymap.init_layout(0x040C);
    keymap.push_kevent(Keymap2::KEVENT_ESC);

    Font glyphs = Font(app_path(AppPath::DefaultFontFile), false);

    InifileWrapper ini;
    CloseMod d("message", ini.get_ini(), time_base, events, gd_forwarder, front,
        screen_info.width, screen_info.height, Rect(0, 0, 799, 599), client_execute,
        glyphs, theme, true);
    d.init();
    d.rdp_input_scancode(0, 0, 0, 0, &keymap);

    events.execute_events(timeval{1,0},[](int){return false;});

    // ::dump_png24("TestCloseModSelector1.png", ConstImageDataView(front), true);
    RED_CHECK_SIG(ConstImageDataView(front),
        "\x95\xd0\x6e\x6e\xae\xdf\xa0\x68\xcb\x7b\x3d\x2d\x84\x07\x59\xa1\xb6\xdb\x30\xb8");
}
