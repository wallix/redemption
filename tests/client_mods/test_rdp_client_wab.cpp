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

#define RED_TEST_MODULE TestRdpClientWab
#include "system/redemption_unit_tests.hpp"


// Comment the code block below to generate testing data.
// Uncomment the code block below to generate testing data.

#include "test_only/check_sig.hpp"
#include "configs/config.hpp"
// Comment the code block below to generate testing data.
#include "test_only/transport/test_transport.hpp"
#include "core/client_info.hpp"
#include "mod/rdp/rdp.hpp"

#include "test_only/lcg_random.hpp"

#include "test_only/front/fake_front.hpp"
// Uncomment the code block below to generate testing data.


RED_AUTO_TEST_CASE(TestDecodePacket)
{
    int verbose = 256;

    ClientInfo info;
    info.keylayout             = 0x040C;
    info.console_session       = 0;
    info.brush_cache_code      = 0;
    info.bpp                   = 16;
    info.width                 = 1024;
    info.height                = 768;
    info.rdp5_performanceflags =   PERF_DISABLE_WALLPAPER
                                 | PERF_DISABLE_FULLWINDOWDRAG | PERF_DISABLE_MENUANIMATIONS;

    memset(info.order_caps.orderSupport, 0xFF, sizeof(info.order_caps.orderSupport));
    info.order_caps.orderSupportExFlags = 0xFFFF;

    // Uncomment the code block below to generate testing data.
    //SSL_library_init();

    FakeFront front(info, verbose);

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
    TestTransport t(indata, sizeof(indata)-1, outdata, sizeof(outdata)-1);

    if (verbose > 2) {
        LOG(LOG_INFO, "--------- CREATION OF MOD ------------------------");
    }

    snprintf(info.hostname, sizeof(info.hostname), "192-168-1-100");

    Inifile ini;

    ModRDPParams mod_rdp_params( "x"
                               , "x"
                               , "10.10.47.154"
                               , "192.168.1.100"
                               , 7
                               , ini.get<cfg::font>()
                               , ini.get<cfg::theme>()
                               , ini.get_ref<cfg::context::server_auto_reconnect_packet>()
                               , ini.get_ref<cfg::context::close_box_extra_message>()
                               , to_verbose_flags(511)
                               );
    mod_rdp_params.device_id                       = "device_id";
    mod_rdp_params.enable_tls                      = true;
    mod_rdp_params.enable_nla                      = false;
    //mod_rdp_params.enable_krb                      = false;
    //mod_rdp_params.enable_clipboard                = true;
    mod_rdp_params.enable_fastpath                 = false;
    mod_rdp_params.enable_mem3blt                  = false;
    mod_rdp_params.enable_new_pointer              = false;
    //mod_rdp_params.rdp_compression                 = 0;
    //mod_rdp_params.error_message                   = nullptr;
    //mod_rdp_params.disconnect_on_logon_user_change = false;
    //mod_rdp_params.open_session_timeout            = 0;
    //mod_rdp_params.certificate_change_action       = 0;
    //mod_rdp_params.extra_orders                    = "";
    mod_rdp_params.large_pointer_support             = false;
    mod_rdp_params.experimental_fix_input_event_sync = false;

    // To always get the same client random, in tests
    LCGRandom gen(0);
    LCGTime timeobj;
    NullAuthentifier authentifier;
    NullReportMessage report_message;
    mod_rdp mod(t, front, info, ini.get_ref<cfg::mod_rdp::redir_info>(),
        gen, timeobj, mod_rdp_params, authentifier, report_message, ini);

    if (verbose > 2) {
        LOG(LOG_INFO, "========= CREATION OF MOD DONE ====================\n\n");
    }

    RED_CHECK_EQUAL(front.info.width, 1024);
    RED_CHECK_EQUAL(front.info.height, 768);

    time_t now = 1450864840;

    while (!mod.is_up_and_running())
            mod.draw_event(now, front);

    for (int count = 0; count < 8; ++count) {
        LOG(LOG_INFO, "===================> count = %d", count);
        mod.draw_event(time(nullptr), front);
    }

    RED_CHECK_SIG(front.gd.impl(), "\xbc\x5e\x77\xb0\x61\x27\x45\xb1\x3c\x87\xd2\x94\x59\xe7\x3e\x8d\x6c\xcc\xc3\x29");
    //front.dump_png("trace_wab_");
}
