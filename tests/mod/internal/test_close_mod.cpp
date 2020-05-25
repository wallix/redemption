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
#include "test_only/check_sig.hpp"
#include "test_only/gdi/test_graphic.hpp"
#include "core/font.hpp"

#include "acl/gd_provider.hpp"
#include "core/front_api.hpp"
#include "utils/image_data_view.hpp"
#include "core/channel_list.hpp"
#include "core/app_path.hpp"

#include "configs/config.hpp"
#include "core/RDP/capabilities/window.hpp"
#include "RAIL/client_execute.hpp"
#include "mod/internal/close_mod.hpp"
#include "keyboard/keymap2.hpp"

#include "utils/png.hpp"
#include "utils/sugar/cast.hpp"


class FakeFront : public FrontAPI
{
public:
    FakeFront(ScreenInfo& screen_info) :
        data{screen_info, TestGraphic(screen_info.width, screen_info.height), {}}
    {}
    ~FakeFront() = default;

    bool can_be_start_capture(SesmanInterface & /*sesman*/) override { return false; }
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
    GdForwarder<gdi::GraphicApi> gd_forwarder(front.gd());
    WindowListCaps window_list_caps;
    TimeBase time_base;
    TimerContainer timer_events_;
    ClientExecute client_execute(time_base, timer_events_, front.gd(), front, window_list_caps, false);

    Inifile ini;
    Theme theme;

    Keymap2 keymap;
    keymap.init_layout(0x040C);
    keymap.push_kevent(Keymap2::KEVENT_ESC);

    Font glyphs = Font(app_path(AppPath::DefaultFontFile), ini.get<cfg::globals::spark_view_specific_glyph_width>());

    CloseMod d("message", ini, time_base, timer_events_, gd_forwarder, front, screen_info.width, screen_info.height, Rect(0, 0, 799, 599), client_execute, glyphs, theme, false);
    d.init();
    d.rdp_input_scancode(0, 0, 0, 0, &keymap);

    auto end_tv = time_base.get_current_time();
    end_tv = {end_tv.tv_sec+1, end_tv.tv_usec};
    timer_events_.exec_timer(end_tv);

    ::dump_png24("TestCloseMod.png", ConstImageDataView(front), true);
    RED_CHECK_SIG(ConstImageDataView(front),
    "\x51\x50\xc4\xea\x5a\xb5\x0f\x12\x91\xe5\x2f\xd4\xd3\x83\x43\x71\x3b\xcd\x34\xe0");
}

RED_AUTO_TEST_CASE(TestCloseModSelector)
{
    ScreenInfo screen_info{800, 600, BitsPerPixel{24}};
    FakeFront front(screen_info);
    GdForwarder<gdi::GraphicApi> gd_forwarder(front.gd());
    WindowListCaps window_list_caps;
    TimeBase time_base;
    TimerContainer timer_events_;
    ClientExecute client_execute(time_base, timer_events_, front.gd(), front, window_list_caps, false);

    Inifile ini;
    Theme theme;

    Keymap2 keymap;
    keymap.init_layout(0x040C);
    keymap.push_kevent(Keymap2::KEVENT_ESC);

    Font glyphs = Font(app_path(AppPath::DefaultFontFile), ini.get<cfg::globals::spark_view_specific_glyph_width>());

    CloseMod d("message", ini, time_base, timer_events_, gd_forwarder, front, screen_info.width, screen_info.height, Rect(0, 0, 799, 599), client_execute, glyphs, theme, true);
    d.init();
    d.rdp_input_scancode(0, 0, 0, 0, &keymap);

    timeval tv1{1, 0};
    timer_events_.exec_timer(tv1);
    ::dump_png24("TestCloseModSelector1.png", ConstImageDataView(front), true);
    RED_CHECK_SIG(ConstImageDataView(front),
    "\x95\xd0\x6e\x6e\xae\xdf\xa0\x68\xcb\x7b\x3d\x2d\x84\x07\x59\xa1\xb6\xdb\x30\xb8");

}
