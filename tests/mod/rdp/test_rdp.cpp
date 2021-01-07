/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Unit test to writing RDP orders to file and rereading them
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "acl/auth_api.hpp"
#include "acl/license_api.hpp"
#include "core/client_info.hpp"
#include "mod/rdp/new_mod_rdp.hpp"
#include "mod/rdp/rdp_params.hpp"
#include "mod/rdp/mod_rdp_factory.hpp"
#include "utils/theme.hpp"
#include "utils/timebase.hpp"
#include "utils/redirection_info.hpp"
#include "core/channels_authorizations.hpp"

#include "test_only/front/fake_front.hpp"
#include "test_only/lcg_random.hpp"
#include "test_only/transport/test_transport.hpp"
#include "test_only/core/font.hpp"
#include "configs/config.hpp"
#include "gdi/osd_api.hpp"


#include <chrono>

// Uncomment the code block below to generate testing data.
//#include "core/listen.hpp"
//#include "utils/netutils.hpp"

// Uncomment the code block below to generate testing data.
//#include "transport/socket_transport.hpp"

// Uncomment the code block below to generate testing data.
//#include <openssl/ssl.h>

using namespace std::chrono_literals;

RED_AUTO_TEST_CASE(TestModRDPWin2008Server)
{
    // Uncomment the code block below to generate testing data.
    //SocketTransport::Verbose STVerbose = SocketTransport::Verbose::dump;

    ClientInfo info;
    info.build = 2600;
    info.keylayout = 0x04C;
    info.console_session = false;
    info.brush_cache_code = 0;
    info.screen_info.bpp = BitsPerPixel{24};
    info.screen_info.width = 800;
    info.screen_info.height = 600;
    info.rdp5_performanceflags = PERF_DISABLE_WALLPAPER;

    snprintf(info.hostname,sizeof(info.hostname),"test");

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

    // Uncomment the code block below to generate testing data.
    //const char * name = "RDP W2008 Target";
    //auto client_sck = ip_connect("10.10.44.101", 3389);

    // Uncomment the code block below to generate testing data.
    //std::string error_message;
    //SocketTransport t( name
    //                 , std::move(client_sck)
    //                 , "10.10.44.101"
    //                 , 3389
    //                 , std::chrono::seconds(1)
    //                 , STVerbose
    //                 , nullptr);

    // Comment the code block below to generate testing data.
    #include "fixtures/dump_w2008.hpp"
    TestTransport t(cstr_array_view(indata), cstr_array_view(outdata));

    std::string close_box_extra_message;
    Theme theme;

    std::array<uint8_t, 28> server_auto_reconnect_packet {};
    ModRDPParams mod_rdp_params( "administrateur"
                               , "S3cur3!1nux$H2"
                               , "10.10.44.101"
                               , "10.10.43.33"
                               , 2
                               , global_font()
                               , theme
                               , server_auto_reconnect_packet
                               , close_box_extra_message
                               , RDPVerbose{}
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

    TLSClientParams tls_client_params;

    // To always get the same client random, in tests
    LCGRandom gen;
    NullLicenseStore license_store;
    EventContainer events;
    Inifile ini;
    NullSessionLog session_log;
    const ChannelsAuthorizations channels_authorizations{"rdpsnd_audio_output", ""};
    ModRdpFactory mod_rdp_factory;
    gdi::NullOsd osd;

    RedirectionInfo redir_info;

    auto mod = new_mod_rdp(
        t, front.gd(), osd, events, session_log,
        front, info, redir_info, gen, channels_authorizations,
        mod_rdp_params, tls_client_params, license_store,
        ini, nullptr, nullptr, mod_rdp_factory);

    RED_CHECK_EQUAL(info.screen_info.width, 800);
    RED_CHECK_EQUAL(info.screen_info.height, 600);

    events.queue[0]->alarm.fd = 0;
    events.execute_events([](int /*fd*/){ return false; }, false);
    events.set_current_time({1, 0});
    for (int count=0; count < 100 && !events.queue.empty(); ++count) {
        events.execute_events([](int){return true;}, false);
    }
}

