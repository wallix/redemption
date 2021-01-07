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
    Author(s): Christophe Grosjean, Javier Caverni, Raphael Zhou
    Based on xrdp Copyright (C) Jay Sorg 2004-2010

    Unit test to writing RDP orders to file and rereading them
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/check_img.hpp"

#include "test_only/front/fake_front.hpp"
#include "test_only/lcg_random.hpp"
#include "test_only/transport/test_transport.hpp"
#include "test_only/core/font.hpp"

#include "acl/auth_api.hpp"
#include "acl/license_api.hpp"
#include "core/client_info.hpp"
#include "utils/timebase.hpp"
#include "acl/auth_api.hpp"
#include "core/channels_authorizations.hpp"
#include "mod/rdp/new_mod_rdp.hpp"
#include "mod/rdp/rdp_params.hpp"
#include "mod/rdp/mod_rdp_factory.hpp"
#include "utils/theme.hpp"
#include "utils/redirection_info.hpp"
#include "configs/config.hpp"
#include "gdi/osd_api.hpp"



RED_AUTO_TEST_CASE(TestDecodePacket)
{
    ClientInfo info;
    info.build                 = 2600;
    info.keylayout             = 0x040C;
    info.console_session       = false;
    info.brush_cache_code      = 0;
    info.screen_info.bpp       = BitsPerPixel{16};
    info.screen_info.width     = 1024;
    info.screen_info.height    = 768;
    info.rdp5_performanceflags =   PERF_DISABLE_WALLPAPER
                                 | PERF_DISABLE_FULLWINDOWDRAG | PERF_DISABLE_MENUANIMATIONS;

    info.order_caps.orderSupport[TS_NEG_DSTBLT_INDEX]     = 1;
    info.order_caps.orderSupport[TS_NEG_PATBLT_INDEX]     = 1;
    info.order_caps.orderSupport[TS_NEG_SCRBLT_INDEX]     = 1;
    info.order_caps.orderSupport[TS_NEG_MEMBLT_INDEX]     = 1;
    info.order_caps.orderSupport[TS_NEG_MEM3BLT_INDEX]    = 1;
    info.order_caps.orderSupport[TS_NEG_LINETO_INDEX]     = 1;
    info.order_caps.orderSupport[TS_NEG_POLYLINE_INDEX]   = 1;
    info.order_caps.orderSupport[TS_NEG_ELLIPSE_SC_INDEX] = 1;
    info.order_caps.orderSupport[TS_NEG_GLYPH_INDEX]      = 1;

    info.order_caps.orderSupportExFlags = 0xFFFF;

    // Uncomment the code block below to generate testing data.
    //SSL_library_init();

    FakeFront front(info.screen_info);

    // const char * name = "RDP Wab Target";
    // Uncomment the code block below to generate testing data.
    //int             client_sck = ip_connect("10.10.47.154", 3389, 3, 1000);
    //std::string     error_message;
    //SocketTransport t( name
    //                 , client_sck
    //                 , "10.10.47.154"
    //                 , 3389
    //                 , verbose
    //                 , &error_message
    //                 );

    // Comment the code block below to generate testing data.
    #include "fixtures/dump_wab.hpp"
    TestTransport t(cstr_array_view(indata), cstr_array_view(outdata));

    snprintf(info.hostname, sizeof(info.hostname), "192-168-1-100");

    std::string close_box_extra_message;

    std::array<uint8_t, 28> server_auto_reconnect_packet {};
    Theme theme;
    ModRDPParams mod_rdp_params( "x"
                               , "x"
                               , "10.10.47.154"
                               , "192.168.1.100"
                               , 7
                               , global_font()
                               , theme
                               , server_auto_reconnect_packet
                               , close_box_extra_message
                               , RDPVerbose(0)
                               );
    mod_rdp_params.device_id                       = "device_id";
    mod_rdp_params.enable_tls                      = true;
    mod_rdp_params.enable_nla                      = false;
    //mod_rdp_params.enable_krb                      = false;
    //mod_rdp_params.enable_clipboard                = true;
    mod_rdp_params.enable_fastpath                 = false;
    mod_rdp_params.disabled_orders                 = TS_NEG_MEM3BLT_INDEX | TS_NEG_DRAWNINEGRID_INDEX | TS_NEG_MULTI_DRAWNINEGRID_INDEX |
                                                     TS_NEG_SAVEBITMAP_INDEX | TS_NEG_MULTIDSTBLT_INDEX | TS_NEG_MULTIPATBLT_INDEX |
                                                     TS_NEG_MULTISCRBLT_INDEX | TS_NEG_MULTIOPAQUERECT_INDEX | TS_NEG_FAST_INDEX_INDEX |
                                                     TS_NEG_POLYGON_SC_INDEX | TS_NEG_POLYGON_CB_INDEX | TS_NEG_POLYLINE_INDEX |
                                                     TS_NEG_FAST_GLYPH_INDEX | TS_NEG_ELLIPSE_SC_INDEX | TS_NEG_ELLIPSE_CB_INDEX;
    mod_rdp_params.enable_new_pointer              = false;
    //mod_rdp_params.rdp_compression                 = 0;
    //mod_rdp_params.error_message                   = nullptr;
    //mod_rdp_params.disconnect_on_logon_user_change = false;
    mod_rdp_params.open_session_timeout            = 5s;
    //mod_rdp_params.certificate_change_action       = 0;
    //mod_rdp_params.extra_orders                    = "";
    mod_rdp_params.large_pointer_support             = false;
    mod_rdp_params.experimental_fix_input_event_sync = false;

    // To always get the same client random, in tests
    LCGRandom gen;
    NullLicenseStore license_store;
    gdi::NullOsd osd;
    EventContainer events;
    Inifile ini;
    NullSessionLog session_log;
    RedirectionInfo redir_info;

    const ChannelsAuthorizations channels_authorizations{"rdpsnd_audio_output", ""};
    ModRdpFactory mod_rdp_factory;

    TLSClientParams tls_client_params;

    auto mod = new_mod_rdp(
        t, front.gd(), osd, events, session_log, front, info, redir_info, gen,
        channels_authorizations, mod_rdp_params, tls_client_params,
        license_store, ini, nullptr, nullptr, mod_rdp_factory);

    RED_CHECK_EQUAL(info.screen_info.width, 1024);
    RED_CHECK_EQUAL(info.screen_info.height, 768);

    events.execute_events([](int){return false;}, false);

    int n = 10;
    int count = 0;
    events.queue[0]->alarm.fd = 0;
    events.set_current_time({1, 0});
    for (; count < n && !events.queue.empty(); ++count) {
        events.execute_events([](int){return true;}, false);
    }

    RED_CHECK_EQ(count, n);

    RED_CHECK_IMG(front, FIXTURES_PATH "/img_ref/client_mods/client_wab.png");
}
