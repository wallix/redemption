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

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestRdpClientLargePointer


#include "system/redemption_unit_tests.hpp"


// Comment the code block below to generate testing data.
#define LOGNULL
// Uncomment the code block below to generate testing data.
//#define LOGPRINT

#include "configs/config.hpp"
// Uncomment the code block below to generate testing data.
//#include "transport/socket_transport.hpp"
#include "transport/test_transport.hpp"
#include "core/client_info.hpp"
#include "mod/rdp/rdp.hpp"

#include "../front/fake_front.hpp"

RED_AUTO_TEST_CASE(TestRdpClientLargePointerDisabled)
{
    int verbose = 256;

    // Uncomment the code block below to generate testing data.
    //SocketTransport::Verbose STVerbose = SocketTransport::Verbose::dump;

    ClientInfo info;
    info.keylayout             = 0x040C;
    info.console_session       = 0;
    info.brush_cache_code      = 0;
    info.bpp                   = 16;
    info.width                 = 1024;
    info.height                = 768;
    info.rdp5_performanceflags =   PERF_DISABLE_WALLPAPER
                                 | PERF_DISABLE_FULLWINDOWDRAG
                                 | PERF_DISABLE_MENUANIMATIONS;

    // Uncomment the code block below to generate testing data.
    //SSL_library_init();

    FakeFront front(info, verbose);

    // Uncomment the code block below to generate testing data.
    //const char * name       = "RDP W2012 Target";
    //int          client_sck = ip_connect("10.10.44.27", 3389, 3, 1000);

    // Uncomment the code block below to generate testing data.
    //std::string  error_message;
    //SocketTransport     t( name
    //                     , client_sck
    //                     , "10.10.44.27"
    //                     , 3389
    //                     , STVerbose
    //                     , &error_message
    //                     );

    // Comment the code block below to generate testing data.
    #include "../fixtures/dump_large_pointer_disabled.hpp"
    TestTransport t(indata, sizeof(indata) - 1,
        outdata, sizeof(outdata) - 1, verbose);

    if (verbose > 2) {
        LOG(LOG_INFO, "--------- CREATION OF MOD ------------------------");
    }

    snprintf(info.hostname, sizeof(info.hostname), "192-168-1-100");

    Inifile ini;

    ModRDPParams mod_rdp_params( "RED\\RDUser"
                               , "SecureKurwa$42"
                               , "10.10.44.27"
                               , "192.168.1.100"
                               , 7
                               , ini.get<cfg::font>()
                               , ini.get<cfg::theme>()
                               , to_verbose_flags(511)
                               );
    mod_rdp_params.device_id                       = "device_id";
    //mod_rdp_params.enable_tls                      = true;
    mod_rdp_params.enable_nla                      = false;
    //mod_rdp_params.enable_krb                      = false;
    //mod_rdp_params.enable_clipboard                = true;
    mod_rdp_params.enable_fastpath                 = true;
    mod_rdp_params.enable_mem3blt                  = true;
    mod_rdp_params.enable_new_pointer              = true;
    //mod_rdp_params.rdp_compression                 = 0;
    //mod_rdp_params.error_message                   = nullptr;
    //mod_rdp_params.disconnect_on_logon_user_change = false;
    //mod_rdp_params.open_session_timeout            = 0;
    //mod_rdp_params.certificate_change_action       = 0;
    //mod_rdp_params.extra_orders                    = "";
    mod_rdp_params.server_redirection_support        = true;
    mod_rdp_params.large_pointer_support             = true;

    // To always get the same client random, in tests
    LCGRandom gen(0);
    LCGTime timeobj;
    NullAuthentifier authentifier;
    mod_rdp mod_(t, front, info, ini.get_ref<cfg::mod_rdp::redir_info>(),
        gen, timeobj, mod_rdp_params, authentifier);
    mod_api * mod = &mod_;

    if (verbose > 2) {
        LOG(LOG_INFO,
            "========= CREATION OF MOD DONE ====================\n\n");
    }
    RED_CHECK(t.get_status());

    RED_CHECK_EQUAL(front.info.width, 1024);
    RED_CHECK_EQUAL(front.info.height, 768);

    uint32_t count = 0;
    BackEvent_t res = BACK_EVENT_NONE;
    while (res == BACK_EVENT_NONE) {
        LOG(LOG_INFO, "===================> count = %u", count);
        if (count++ >= 70) break;
        mod->draw_event(time(nullptr), front);
    }

    //front.dump_png("trace_test_rdp_client_large_pointer_disabled_");
}

RED_AUTO_TEST_CASE(TestRdpClientLargePointerEnabled)
{
    int verbose = 256;

    // Uncomment the code block below to generate testing data.
    //SocketTransport::Verbose STVerbose = SocketTransport::Verbose::dump;

    ClientInfo info;
    info.keylayout             = 0x040C;
    info.console_session       = 0;
    info.brush_cache_code      = 0;
    info.bpp                   = 16;
    info.width                 = 1024;
    info.height                = 768;
    info.rdp5_performanceflags =   PERF_DISABLE_WALLPAPER
                                 | PERF_DISABLE_FULLWINDOWDRAG
                                 | PERF_DISABLE_MENUANIMATIONS;

    info.large_pointer_caps.largePointerSupportFlags = LARGE_POINTER_FLAG_96x96;

    info.multi_fragment_update_caps.MaxRequestSize = 38055;

    // Uncomment the code block below to generate testing data.
    //SSL_library_init();

    FakeFront front(info, verbose);

    // Uncomment the code block below to generate testing data.
    //const char * name       = "RDP W2012 Target";
    //int          client_sck = ip_connect("10.10.44.27", 3389, 3, 1000);

    // Uncomment the code block below to generate testing data.
    //std::string  error_message;
    //SocketTransport     t( name
    //                     , client_sck
    //                     , "10.10.44.27"
    //                     , 3389
    //                     , STVerbose
    //                     , &error_message
    //                     );

    // Comment the code block below to generate testing data.
    #include "../fixtures/dump_large_pointer_enabled.hpp"
    TestTransport t(indata, sizeof(indata) - 1,
        outdata, sizeof(outdata) - 1, verbose);

    if (verbose > 2) {
        LOG(LOG_INFO, "--------- CREATION OF MOD ------------------------");
    }

    snprintf(info.hostname, sizeof(info.hostname), "192-168-1-100");

    Inifile ini;

    ModRDPParams mod_rdp_params( "RED\\RDUser"
                               , "SecureKurwa$42"
                               , "10.10.44.27"
                               , "192.168.1.100"
                               , 7
                               , ini.get<cfg::font>()
                               , ini.get<cfg::theme>()
                               , to_verbose_flags(511)
                               );
    mod_rdp_params.device_id                       = "device_id";
    //mod_rdp_params.enable_tls                      = true;
    mod_rdp_params.enable_nla                      = false;
    //mod_rdp_params.enable_krb                      = false;
    //mod_rdp_params.enable_clipboard                = true;
    mod_rdp_params.enable_fastpath                 = true;
    mod_rdp_params.enable_mem3blt                  = true;
    mod_rdp_params.enable_new_pointer              = true;
    //mod_rdp_params.rdp_compression                 = 0;
    //mod_rdp_params.error_message                   = nullptr;
    //mod_rdp_params.disconnect_on_logon_user_change = false;
    //mod_rdp_params.open_session_timeout            = 0;
    //mod_rdp_params.certificate_change_action       = 0;
    //mod_rdp_params.extra_orders                    = "";
    mod_rdp_params.server_redirection_support        = true;
    mod_rdp_params.large_pointer_support             = true;

    // To always get the same client random, in tests
    LCGRandom gen(0);
    LCGTime timeobj;
    NullAuthentifier authentifier;
    mod_rdp mod_(t, front, info, ini.get_ref<cfg::mod_rdp::redir_info>(),
        gen, timeobj, mod_rdp_params, authentifier);
    mod_api * mod = &mod_;

    if (verbose > 2) {
        LOG(LOG_INFO,
            "========= CREATION OF MOD DONE ====================\n\n");
    }
    RED_CHECK(t.get_status());

    RED_CHECK_EQUAL(front.info.width, 1024);
    RED_CHECK_EQUAL(front.info.height, 768);

    uint32_t count = 0;
    BackEvent_t res = BACK_EVENT_NONE;
    while (res == BACK_EVENT_NONE) {
        LOG(LOG_INFO, "===================> count = %u", count);
        if (count++ >= 70) break;
        mod->draw_event(time(nullptr), front);
    }

    //front.dump_png("trace_test_rdp_client_large_pointer_enabled_");
}
