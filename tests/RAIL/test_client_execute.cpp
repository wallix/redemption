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
#include "keyboard/keymap2.hpp"

#include "utils/png.hpp"
#include "utils/sugar/cast.hpp"

#include "test_only/check_sig.hpp"
#include "test_only/gdi/test_graphic.hpp"
#include "test_only/acl/sesman_wrapper.hpp" // for InifileWrapper
#include "test_only/core/font.hpp" // for global_font()
#include "configs/config.hpp"
#include "core/events.hpp"
#include "core/RDP/remote_programs.hpp"
#include "mod/internal/login_mod.hpp"
#include "test_only/acl/sesman_wrapper.hpp" // for InifileWrapper
#include "core/session_reactor.hpp"
#include "core/RDP/gcc/userdata/cs_core.hpp"
#include "core/RDP/gcc/userdata/cs_monitor.hpp"
#include "core/client_info.hpp"

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


RED_AUTO_TEST_CASE(TestClientExecute)
{
    ScreenInfo screen_info{1024, 768, BitsPerPixel{24}};
    FakeFront front(screen_info);

    ClientInfo client_info;
    GCC::UserData::CSMonitor cs_monitor;
    GCC::UserData::CSCore cs_core;
    client_info.screen_info = screen_info;
    client_info.cs_monitor = cs_monitor;
    client_info.cs_core = cs_core;

    GdForwarder<gdi::GraphicApi> gd_forwarder(front.gd());
    WindowListCaps window_list_caps;
    TimeBase time_base({0,0});
    EventContainer events;
    ClientExecute client_execute(time_base, events, front.gd(), front, window_list_caps, true);

    TimerContainer timer_events_;
    Theme theme;

    Keymap2 keymap;
    keymap.init_layout(0x040C);
    keymap.push_kevent(Keymap2::KEVENT_ESC);

    Font glyphs = Font(app_path(AppPath::DefaultFontFile), false);

    Inifile ini;
    LoginMod d(ini, time_base, timer_events_, "user", "pass", front.gd(), front,
        screen_info.width, screen_info.height, client_execute.adjust_rect(client_info.get_widget_rect()), client_execute, global_font(), theme);
    d.init();
}

RED_AUTO_TEST_CASE(TestProcessClientExecutePdu)
{
// (.........|.|.W.A.B.R.e.m.o.t.e.A.p.p..F
    uint8_t buf[] = {0x01, 0x00, 0x28, 0x00, 0x08, 0x00, 0x1c, 0x00,
                   0x00, 0x00, 0x00, 0x00, 0x7c, 0x00, 0x7c, 0x00,
                   0x57, 0x00, 0x41, 0x00, 0x42, 0x00, 0x52, 0x00,
                   0x65, 0x00, 0x6d, 0x00, 0x6f, 0x00, 0x74, 0x00,
                   0x65, 0x00, 0x41, 0x00, 0x70, 0x00, 0x70, 0x00};

    InStream chunk{buf};
    auto order_type = chunk.in_uint16_le();
    RED_CHECK(order_type == TS_RAIL_ORDER_EXEC);
    auto order_length = chunk.in_uint16_le();
    RED_CHECK(order_length == 0x0028);

    ClientExecutePDU cepdu;
    cepdu.receive(chunk);

    WindowsExecuteShellParams params = cepdu.get_client_execute();
    RED_CHECK(params.flags == 8);
    RED_CHECK(params.exe_or_file == std::string("||WABRemoteApp"));
    RED_CHECK(params.working_dir == std::string(""));
    RED_CHECK(params.arguments == std::string(""));
}

RED_AUTO_TEST_CASE(TestProcessClientActivatePDU)
{
    uint8_t buf[] = {0x02, 0x00, 0x09, 0x00, 0x40, 0x9c, 0x00, 0x00, 0x00,};

    InStream chunk{buf};
    auto order_type = chunk.in_uint16_le();
    RED_CHECK(order_type == TS_RAIL_ORDER_ACTIVATE);
    auto order_length = chunk.in_uint16_le();
    RED_CHECK(order_length == 0x0009);


}

RED_AUTO_TEST_CASE(TestProcessClientSystemParametersUpdatePDU)
{
    uint8_t buf[] = {0x03, 0x00, 0x12, 0x00, 0x43, 0x00, 0x00, 0x00,
                   0x7e, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
                   0x00, 0x00};

    InStream chunk{buf};
    auto order_type = chunk.in_uint16_le();
    RED_CHECK(order_type == TS_RAIL_ORDER_SYSPARAM);
    auto order_length = chunk.in_uint16_le();
    RED_CHECK(order_length == 0x0012);

}

// TODO: test for TS_RAIL_ORDER_HANDSHAKE

RED_AUTO_TEST_CASE(TestProcessClientHandshakePdu)
{
    uint8_t buf[] = {0x05, 0x00, 0x08, 0x00, 0xba, 0x47, 0x00, 0x00, 0x00,};

    InStream chunk{buf};
    auto order_type = chunk.in_uint16_le();
    RED_CHECK(order_type == TS_RAIL_ORDER_HANDSHAKE);
    auto order_length = chunk.in_uint16_le();
    RED_CHECK(order_length == 0x0008);


}

// TODO: test for TS_RAIL_ORDER_NOTIFY_EVENT

// TODO: test for TS_RAIL_ORDER_WINDOW_MOVE

// TODO: test for TS_RAIL_ORDER_LOCALMOVESIZE

// TODO: test for TS_RAIL_ORDER_MINMAXINFO


RED_AUTO_TEST_CASE(TestProcessClientInformationPdu)
{
    uint8_t buf[] = {0x0b, 0x00, // order type 0x000b
                     0x08, 0x00, // order length 0x0008
                     0xe5, 0x01,
                     0x00, 0x00, // order Flags 0x1e5
                     };

    InStream chunk{buf};
    auto order_type = chunk.in_uint16_le();
    RED_CHECK(order_type == TS_RAIL_ORDER_CLIENTSTATUS);
    auto order_length = chunk.in_uint16_le();
    RED_CHECK(order_length == 0x0008);

    ClientInformationPDU cipdu;
    cipdu.receive(chunk);

    RED_CHECK(cipdu.get_flags() == (0x100
                                  | TS_RAIL_CLIENTSTATUS_HIGH_DPI_ICONS_SUPPORTED
                                  | TS_RAIL_CLIENTSTATUS_APPBAR_REMOTING_SUPPORTED
                                  | TS_RAIL_CLIENTSTATUS_POWER_DISPLAY_REQUEST_SUPPORTED
                                  | TS_RAIL_CLIENTSTATUS_ALLOWLOCALMOVESIZE
//                                  | TS_RAIL_CLIENTSTATUS_AUTORECONNECT
                                  | TS_RAIL_CLIENTSTATUS_ZORDER_SYNC));
}

// TODO: test for TS_RAIL_ORDER_SYSMENU

// TODO: test for TS_RAIL_ORDER_LANGBARINFO

