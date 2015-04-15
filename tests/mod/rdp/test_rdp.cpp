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
#define BOOST_TEST_MODULE TestRdp
#include <boost/test/auto_unit_test.hpp>

#undef SHARE_PATH
#define SHARE_PATH FIXTURES_PATH

#define LOGNULL
//#define LOGPRINT

#include "config.hpp"
// #include "socket_transport.hpp"
#include "test_transport.hpp"
#include "client_info.hpp"
#include "rdp/rdp.hpp"

#include "../../front/fake_front.hpp"

BOOST_AUTO_TEST_CASE(TestModRDPXPServer)
{
    ClientInfo info;
    info.keylayout = 0x04C;
    info.console_session = 0;
    info.brush_cache_code = 0;
    info.bpp = 24;
    info.width = 800;
    info.height = 600;
    info.rdp5_performanceflags = PERF_DISABLE_WALLPAPER;
    snprintf(info.hostname,sizeof(info.hostname),"test");
    int verbose = 511;

    FakeFront front(info, verbose);

    const char * name = "RDP XP Target";

    // int client_sck = ip_connect("10.10.47.175", 3389, 3, 1000, verbose);
    // redemption::string error_message;
    // SocketTransport t( name
    //                  , client_sck
    //                  , "10.10.47.175"
    //                  , 3389
    //                  , verbose
    //                  , &error_message
    //                  );


    #include "fixtures/dump_xp_mem3blt.hpp"
    TestTransport t(name, indata, sizeof(indata), outdata, sizeof(outdata), verbose);

    if (verbose > 2){
        LOG(LOG_INFO, "--------- CREATION OF MOD ------------------------");
    }

    Inifile ini;

    try {
        ModRDPParams mod_rdp_params( "xavier"
                                   , "SecureLinux"
                                   , "10.10.47.175"
                                   , "10.10.9.161"
                                   , 7
                                   , verbose
                                   );
        mod_rdp_params.enable_tls                      = false;
        mod_rdp_params.enable_nla                      = false;
        //mod_rdp_params.enable_krb                      = false;
        //mod_rdp_params.enable_clipboard                = true;
        mod_rdp_params.enable_fastpath                 = false;
        //mod_rdp_params.enable_mem3blt                  = true;
        //mod_rdp_params.enable_bitmap_update            = false;
        mod_rdp_params.enable_new_pointer              = false;
        //mod_rdp_params.rdp_compression                 = 0;
        //mod_rdp_params.error_message                   = NULL;
        //mod_rdp_params.disconnect_on_logon_user_change = false;
        //mod_rdp_params.open_session_timeout            = 0;
        //mod_rdp_params.certificate_change_action       = 0;
        //mod_rdp_params.extra_orders                    = "";
        mod_rdp_params.server_redirection_support        = true;

        // To always get the same client random, in tests
        LCGRandom gen(0);
        mod_rdp mod_(t, front, info, ini.mod_rdp.redir_info, gen, mod_rdp_params);
        mod_api * mod = &mod_;

        if (verbose > 2){
            LOG(LOG_INFO, "========= CREATION OF MOD DONE ====================\n\n");
        }
        BOOST_CHECK(t.get_status());
        BOOST_CHECK_EQUAL(mod->get_front_width(), 800);
        BOOST_CHECK_EQUAL(mod->get_front_height(), 600);

        uint32_t count = 0;
        BackEvent_t res = BACK_EVENT_NONE;
        while (res == BACK_EVENT_NONE){
            LOG(LOG_INFO, "=======================> count=%u", count);

            if (count++ >= 25) break;
    //        if (count == 10){
    //            front.dump_png("trace_xp_10_");
    //        }
    //        if (count == 20){
    //            front.dump_png("trace_xp_20_");
    //        }
            mod->draw_event(time(NULL));
        }
    }
    catch (const Error & e) {
        LOG(LOG_INFO, "=======================> Exception raised=%u", e.id);
    };
//    front.dump_png("trace_xp_");
}

BOOST_AUTO_TEST_CASE(TestModRDPWin2008Server)
{
    ClientInfo info;
    info.keylayout = 0x04C;
    info.console_session = 0;
    info.brush_cache_code = 0;
    info.bpp = 24;
    info.width = 800;
    info.height = 600;
    info.rdp5_performanceflags = PERF_DISABLE_WALLPAPER;
    snprintf(info.hostname,sizeof(info.hostname),"test");
    int verbose = 511;

    FakeFront front(info, verbose);

    const char * name = "RDP W2008 Target";

    // int client_sck = ip_connect("10.10.46.78", 3389, 3, 1000, verbose);
    // redemption::string error_message;
    // SocketTransport t( name
    //                  , client_sck
    //                  , "10.10.46.78"
    //                  , 3389
    //                  , verbose
    //                  , &error_message
    //                  );

    #include "fixtures/dump_w2008.hpp"
    TestTransport t(name, indata, sizeof(indata), outdata, sizeof(outdata), verbose);

    if (verbose > 2){
        LOG(LOG_INFO, "--------- CREATION OF MOD ------------------------");
    }

    Inifile ini;

    ModRDPParams mod_rdp_params( "administrateur@qa"
                               , "S3cur3!1nux"
                               , "10.10.46.78"
                               , "10.10.9.161"
                               , 2
                               , 0
                               );
    mod_rdp_params.enable_tls                      = false;
    mod_rdp_params.enable_nla                      = false;
    //mod_rdp_params.enable_krb                      = false;
    //mod_rdp_params.enable_clipboard                = true;
    mod_rdp_params.enable_fastpath                 = false;
    mod_rdp_params.enable_mem3blt                  = false;
    mod_rdp_params.enable_bitmap_update            = true;
    mod_rdp_params.enable_new_pointer              = false;
    //mod_rdp_params.rdp_compression                 = 0;
    //mod_rdp_params.error_message                   = NULL;
    //mod_rdp_params.disconnect_on_logon_user_change = false;
    //mod_rdp_params.open_session_timeout            = 0;
    //mod_rdp_params.certificate_change_action       = 0;
    //mod_rdp_params.extra_orders                    = "";
    mod_rdp_params.server_redirection_support        = true;

    // To always get the same client random, in tests
    LCGRandom gen(0);
    mod_rdp mod_(t, front, info, ini.mod_rdp.redir_info, gen, mod_rdp_params);
    mod_api * mod = &mod_;

    if (verbose > 2){
        LOG(LOG_INFO, "========= CREATION OF MOD DONE ====================\n\n");
    }
    BOOST_CHECK(t.get_status());
    BOOST_CHECK_EQUAL(mod->get_front_width(), 800);
    BOOST_CHECK_EQUAL(mod->get_front_height(), 600);

    uint32_t count = 0;
    BackEvent_t res = BACK_EVENT_NONE;
    while (res == BACK_EVENT_NONE){
        LOG(LOG_INFO, "===================> count = %u", count);
        if (count++ >= 38) break;
        mod->draw_event(time(NULL));
    }

//    front.dump_png("trace_w2008_");
}

BOOST_AUTO_TEST_CASE(TestModRDPW2003Server)
{
    ClientInfo info;
    info.keylayout = 0x04C;
    info.console_session = 0;
    info.brush_cache_code = 0;
    info.bpp = 24;
    info.width = 800;
    info.height = 600;
    info.rdp5_performanceflags = PERF_DISABLE_WALLPAPER;
    snprintf(info.hostname,sizeof(info.hostname),"test");
    int verbose = 511;

    FakeFront front(info, verbose);

    const char * name = "RDP W2003 Target";

    // int client_sck = ip_connect("10.10.47.205", 3389, 3, 1000, verbose);
    // redemption::string error_message;
    // SocketTransport t( name
    //                  , client_sck
    //                  , "10.10.47.205"
    //                  , 3389
    //                  , verbose
    //                  , &error_message
    //                  );


    #include "fixtures/dump_w2003_mem3blt.hpp"
    TestTransport t(name, indata, sizeof(indata), outdata, sizeof(outdata), verbose);

    if (verbose > 2){
        LOG(LOG_INFO, "--------- CREATION OF MOD ------------------------");
    }

    Inifile ini;

    ModRDPParams mod_rdp_params( "administrateur"
                               , "SecureLinux"
                               , "10.10.47.205"
                               , "0.0.0.0"
                               , 2
                               , verbose
                               );
    mod_rdp_params.enable_tls                      = false;
    mod_rdp_params.enable_nla                      = false;
    //mod_rdp_params.enable_krb                      = false;
    //mod_rdp_params.enable_clipboard                = true;
    mod_rdp_params.enable_fastpath                 = false;
    //mod_rdp_params.enable_mem3blt                  = true;
    //mod_rdp_params.enable_bitmap_update            = false;
    mod_rdp_params.enable_new_pointer              = false;
    //mod_rdp_params.rdp_compression                 = 0;
    //mod_rdp_params.error_message                   = NULL;
    //mod_rdp_params.disconnect_on_logon_user_change = false;
    //mod_rdp_params.open_session_timeout            = 0;
    //mod_rdp_params.certificate_change_action       = 0;
    //mod_rdp_params.extra_orders                    = "";
    mod_rdp_params.server_redirection_support        = true;

    // To always get the same client random, in tests
    LCGRandom gen(0);
    mod_rdp mod_(t, front, info, ini.mod_rdp.redir_info, gen, mod_rdp_params);
    mod_api * mod = &mod_;

    if (verbose > 2){
        LOG(LOG_INFO, "========= CREATION OF MOD DONE ====================\n\n");
    }

    BOOST_CHECK(t.get_status());
    BOOST_CHECK_EQUAL(mod->get_front_width(), 800);
    BOOST_CHECK_EQUAL(mod->get_front_height(), 600);

    uint32_t count = 0;
    BackEvent_t res = BACK_EVENT_NONE;
    while (res == BACK_EVENT_NONE){
        LOG(LOG_INFO, "=======================> count=%u", count);

        if (count++ >= 25) break;
//        if (count == 10){
//            front.dump_png("trace_w2003_10_");
//        }
//        if (count == 20){
//            front.dump_png("trace_w2003_20_");
//        }
        mod->draw_event(time(NULL));
    }


//    front.dump_png("trace_w2003_");
}

BOOST_AUTO_TEST_CASE(TestModRDPW2000Server)
{
    ClientInfo info;
    info.keylayout = 0x04C;
    info.console_session = 0;
    info.brush_cache_code = 0;
    info.bpp = 24;
    info.width = 800;
    info.height = 600;
    info.rdp5_performanceflags = PERF_DISABLE_WALLPAPER;
    snprintf(info.hostname,sizeof(info.hostname),"test");
    int verbose = 256;

    FakeFront front(info, verbose);

    const char * name = "RDP W2000 Target";

    // int client_sck = ip_connect("10.10.47.39", 3389, 3, 1000, verbose);
    // redemption::string error_message;
    // SocketTransport t( name
    //                  , client_sck
    //                  , "10.10.47.39"
    //                  , 3389
    //                  , verbose
    //                  , &error_message
    //                  );

    #include "fixtures/dump_w2000_mem3blt.hpp"
    TestTransport t(name, indata, sizeof(indata), outdata, sizeof(outdata), verbose);

    if (verbose > 2){
        LOG(LOG_INFO, "--------- CREATION OF MOD ------------------------");
    }

    Inifile ini;

    ModRDPParams mod_rdp_params( "administrateur"
                               , "SecureLinux"
                               , "10.10.47.39"
                               , "0.0.0.0"
                               , 2
                               , 0
                               );
    mod_rdp_params.enable_tls                      = false;
    mod_rdp_params.enable_nla                      = false;
    //mod_rdp_params.enable_krb                      = false;
    //mod_rdp_params.enable_clipboard                = true;
    mod_rdp_params.enable_fastpath                 = false;
    //mod_rdp_params.enable_mem3blt                  = true;
    //mod_rdp_params.enable_bitmap_update            = false;
    mod_rdp_params.enable_new_pointer              = false;
    //mod_rdp_params.rdp_compression                 = 0;
    //mod_rdp_params.error_message                   = NULL;
    //mod_rdp_params.disconnect_on_logon_user_change = false;
    //mod_rdp_params.open_session_timeout            = 0;
    //mod_rdp_params.certificate_change_action       = 0;
    //mod_rdp_params.extra_orders                    = "";
    mod_rdp_params.server_redirection_support        = true;

    // To always get the same client random, in tests
    LCGRandom gen(0);
    mod_rdp mod_(t, front, info, ini.mod_rdp.redir_info, gen, mod_rdp_params);
    mod_api * mod = &mod_;

    if (verbose > 2){
        LOG(LOG_INFO, "========= CREATION OF MOD DONE ====================\n\n");
    }

    BOOST_CHECK(t.get_status());
    BOOST_CHECK_EQUAL(mod->get_front_width(), 800);
    BOOST_CHECK_EQUAL(mod->get_front_height(), 600);

    uint32_t count = 0;
    BackEvent_t res = BACK_EVENT_NONE;
    while (res == BACK_EVENT_NONE){
        LOG(LOG_INFO, "=======================> count=%u", count);

        if (count++ >= 25) break;
//        if (count == 10){
//            front.dump_png("trace_w2000_10_");
//        }
//        if (count == 20){
//            front.dump_png("trace_w2000_20_");
//        }
        mod->draw_event(time(NULL));
    }

//    front.dump_png("trace_w2000_");
}

BOOST_AUTO_TEST_CASE(TestRemoveRdpdrFileSystem)
{
    uint8_t result_data[] = {
/* 0000 */ 0x72, 0x44, 0x41, 0x44, 0x03, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,  // rDAD............
/* 0010 */ 0x50, 0x52, 0x4e, 0x34, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,  // PRN4............
/* 0020 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x34, 0x00, 0x00, 0x00, 0x34, 0x00, 0x00, 0x00,  // ........4...4...
/* 0030 */ 0x00, 0x00, 0x00, 0x00, 0x43, 0x00, 0x61, 0x00, 0x6e, 0x00, 0x6f, 0x00, 0x6e, 0x00, 0x20, 0x00,  // ....C.a.n.o.n. .
/* 0040 */ 0x47, 0x00, 0x65, 0x00, 0x6e, 0x00, 0x65, 0x00, 0x72, 0x00, 0x69, 0x00, 0x63, 0x00, 0x20, 0x00,  // G.e.n.e.r.i.c. .
/* 0050 */ 0x50, 0x00, 0x43, 0x00, 0x4c, 0x00, 0x36, 0x00, 0x20, 0x00, 0x44, 0x00, 0x72, 0x00, 0x69, 0x00,  // P.C.L.6. .D.r.i.
/* 0060 */ 0x76, 0x00, 0x65, 0x00, 0x72, 0x00, 0x00, 0x00, 0x43, 0x00, 0x61, 0x00, 0x6e, 0x00, 0x6f, 0x00,  // v.e.r...C.a.n.o.
/* 0070 */ 0x6e, 0x00, 0x20, 0x00, 0x47, 0x00, 0x65, 0x00, 0x6e, 0x00, 0x65, 0x00, 0x72, 0x00, 0x69, 0x00,  // n. .G.e.n.e.r.i.
/* 0080 */ 0x63, 0x00, 0x20, 0x00, 0x50, 0x00, 0x43, 0x00, 0x4c, 0x00, 0x36, 0x00, 0x20, 0x00, 0x44, 0x00,  // c. .P.C.L.6. .D.
/* 0090 */ 0x72, 0x00, 0x69, 0x00, 0x76, 0x00, 0x65, 0x00, 0x72, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,  // r.i.v.e.r.......
/* 00a0 */ 0x03, 0x00, 0x00, 0x00, 0x50, 0x52, 0x4e, 0x33, 0x00, 0x00, 0x00, 0x00, 0xc2, 0x00, 0x00, 0x00,  // ....PRN3........
/* 00b0 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5c, 0x00, 0x00, 0x00,  // ................
/* 00c0 */ 0x4e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4d, 0x00, 0x69, 0x00, 0x63, 0x00, 0x72, 0x00,  // N.......M.i.c.r.
/* 00d0 */ 0x6f, 0x00, 0x73, 0x00, 0x6f, 0x00, 0x66, 0x00, 0x74, 0x00, 0x20, 0x00, 0x4f, 0x00, 0x66, 0x00,  // o.s.o.f.t. .O.f.
/* 00e0 */ 0x66, 0x00, 0x69, 0x00, 0x63, 0x00, 0x65, 0x00, 0x20, 0x00, 0x44, 0x00, 0x6f, 0x00, 0x63, 0x00,  // f.i.c.e. .D.o.c.
/* 00f0 */ 0x75, 0x00, 0x6d, 0x00, 0x65, 0x00, 0x6e, 0x00, 0x74, 0x00, 0x20, 0x00, 0x49, 0x00, 0x6d, 0x00,  // u.m.e.n.t. .I.m.
/* 0100 */ 0x61, 0x00, 0x67, 0x00, 0x65, 0x00, 0x20, 0x00, 0x57, 0x00, 0x72, 0x00, 0x69, 0x00, 0x74, 0x00,  // a.g.e. .W.r.i.t.
/* 0110 */ 0x65, 0x00, 0x72, 0x00, 0x20, 0x00, 0x44, 0x00, 0x72, 0x00, 0x69, 0x00, 0x76, 0x00, 0x65, 0x00,  // e.r. .D.r.i.v.e.
/* 0120 */ 0x72, 0x00, 0x00, 0x00, 0x4d, 0x00, 0x69, 0x00, 0x63, 0x00, 0x72, 0x00, 0x6f, 0x00, 0x73, 0x00,  // r...M.i.c.r.o.s.
/* 0130 */ 0x6f, 0x00, 0x66, 0x00, 0x74, 0x00, 0x20, 0x00, 0x4f, 0x00, 0x66, 0x00, 0x66, 0x00, 0x69, 0x00,  // o.f.t. .O.f.f.i.
/* 0140 */ 0x63, 0x00, 0x65, 0x00, 0x20, 0x00, 0x44, 0x00, 0x6f, 0x00, 0x63, 0x00, 0x75, 0x00, 0x6d, 0x00,  // c.e. .D.o.c.u.m.
/* 0150 */ 0x65, 0x00, 0x6e, 0x00, 0x74, 0x00, 0x20, 0x00, 0x49, 0x00, 0x6d, 0x00, 0x61, 0x00, 0x67, 0x00,  // e.n.t. .I.m.a.g.
/* 0160 */ 0x65, 0x00, 0x20, 0x00, 0x57, 0x00, 0x72, 0x00, 0x69, 0x00, 0x74, 0x00, 0x65, 0x00, 0x72, 0x00,  // e. .W.r.i.t.e.r.
/* 0170 */ 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x50, 0x52, 0x4e, 0x32, 0x00, 0x00,  // ..........PRN2..
/* 0180 */ 0x00, 0x00, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ..D.............
/* 0190 */ 0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0x00,  // ..............P.
/* 01a0 */ 0x44, 0x00, 0x46, 0x00, 0x43, 0x00, 0x72, 0x00, 0x65, 0x00, 0x61, 0x00, 0x74, 0x00, 0x6f, 0x00,  // D.F.C.r.e.a.t.o.
/* 01b0 */ 0x72, 0x00, 0x00, 0x00, 0x50, 0x00, 0x44, 0x00, 0x46, 0x00, 0x43, 0x00, 0x72, 0x00, 0x65, 0x00,  // r...P.D.F.C.r.e.
/* 01c0 */ 0x61, 0x00, 0x74, 0x00, 0x6f, 0x00, 0x72, 0x00, 0x00, 0x00,                    // a.t.o.r...
    };

    uint8_t source_data[] = {
/* 0000 */ 0x72, 0x44, 0x41, 0x44, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,  // rDAD............
/* 0010 */ 0x50, 0x52, 0x4e, 0x34, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,  // PRN4............
/* 0020 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x34, 0x00, 0x00, 0x00, 0x34, 0x00, 0x00, 0x00,  // ........4...4...
/* 0030 */ 0x00, 0x00, 0x00, 0x00, 0x43, 0x00, 0x61, 0x00, 0x6e, 0x00, 0x6f, 0x00, 0x6e, 0x00, 0x20, 0x00,  // ....C.a.n.o.n. .
/* 0040 */ 0x47, 0x00, 0x65, 0x00, 0x6e, 0x00, 0x65, 0x00, 0x72, 0x00, 0x69, 0x00, 0x63, 0x00, 0x20, 0x00,  // G.e.n.e.r.i.c. .
/* 0050 */ 0x50, 0x00, 0x43, 0x00, 0x4c, 0x00, 0x36, 0x00, 0x20, 0x00, 0x44, 0x00, 0x72, 0x00, 0x69, 0x00,  // P.C.L.6. .D.r.i.
/* 0060 */ 0x76, 0x00, 0x65, 0x00, 0x72, 0x00, 0x00, 0x00, 0x43, 0x00, 0x61, 0x00, 0x6e, 0x00, 0x6f, 0x00,  // v.e.r...C.a.n.o.
/* 0070 */ 0x6e, 0x00, 0x20, 0x00, 0x47, 0x00, 0x65, 0x00, 0x6e, 0x00, 0x65, 0x00, 0x72, 0x00, 0x69, 0x00,  // n. .G.e.n.e.r.i.
/* 0080 */ 0x63, 0x00, 0x20, 0x00, 0x50, 0x00, 0x43, 0x00, 0x4c, 0x00, 0x36, 0x00, 0x20, 0x00, 0x44, 0x00,  // c. .P.C.L.6. .D.
/* 0090 */ 0x72, 0x00, 0x69, 0x00, 0x76, 0x00, 0x65, 0x00, 0x72, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,  // r.i.v.e.r.......
/* 00a0 */ 0x03, 0x00, 0x00, 0x00, 0x50, 0x52, 0x4e, 0x33, 0x00, 0x00, 0x00, 0x00, 0xc2, 0x00, 0x00, 0x00,  // ....PRN3........
/* 00b0 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5c, 0x00, 0x00, 0x00,  // ................
/* 00c0 */ 0x4e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4d, 0x00, 0x69, 0x00, 0x63, 0x00, 0x72, 0x00,  // N.......M.i.c.r.
/* 00d0 */ 0x6f, 0x00, 0x73, 0x00, 0x6f, 0x00, 0x66, 0x00, 0x74, 0x00, 0x20, 0x00, 0x4f, 0x00, 0x66, 0x00,  // o.s.o.f.t. .O.f.
/* 00e0 */ 0x66, 0x00, 0x69, 0x00, 0x63, 0x00, 0x65, 0x00, 0x20, 0x00, 0x44, 0x00, 0x6f, 0x00, 0x63, 0x00,  // f.i.c.e. .D.o.c.
/* 00f0 */ 0x75, 0x00, 0x6d, 0x00, 0x65, 0x00, 0x6e, 0x00, 0x74, 0x00, 0x20, 0x00, 0x49, 0x00, 0x6d, 0x00,  // u.m.e.n.t. .I.m.
/* 0100 */ 0x61, 0x00, 0x67, 0x00, 0x65, 0x00, 0x20, 0x00, 0x57, 0x00, 0x72, 0x00, 0x69, 0x00, 0x74, 0x00,  // a.g.e. .W.r.i.t.
/* 0110 */ 0x65, 0x00, 0x72, 0x00, 0x20, 0x00, 0x44, 0x00, 0x72, 0x00, 0x69, 0x00, 0x76, 0x00, 0x65, 0x00,  // e.r. .D.r.i.v.e.
/* 0120 */ 0x72, 0x00, 0x00, 0x00, 0x4d, 0x00, 0x69, 0x00, 0x63, 0x00, 0x72, 0x00, 0x6f, 0x00, 0x73, 0x00,  // r...M.i.c.r.o.s.
/* 0130 */ 0x6f, 0x00, 0x66, 0x00, 0x74, 0x00, 0x20, 0x00, 0x4f, 0x00, 0x66, 0x00, 0x66, 0x00, 0x69, 0x00,  // o.f.t. .O.f.f.i.
/* 0140 */ 0x63, 0x00, 0x65, 0x00, 0x20, 0x00, 0x44, 0x00, 0x6f, 0x00, 0x63, 0x00, 0x75, 0x00, 0x6d, 0x00,  // c.e. .D.o.c.u.m.
/* 0150 */ 0x65, 0x00, 0x6e, 0x00, 0x74, 0x00, 0x20, 0x00, 0x49, 0x00, 0x6d, 0x00, 0x61, 0x00, 0x67, 0x00,  // e.n.t. .I.m.a.g.
/* 0160 */ 0x65, 0x00, 0x20, 0x00, 0x57, 0x00, 0x72, 0x00, 0x69, 0x00, 0x74, 0x00, 0x65, 0x00, 0x72, 0x00,  // e. .W.r.i.t.e.r.
/* 0170 */ 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x50, 0x52, 0x4e, 0x32, 0x00, 0x00,  // ..........PRN2..
/* 0180 */ 0x00, 0x00, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ..D.............
/* 0190 */ 0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0x00,  // ..............P.
/* 01a0 */ 0x44, 0x00, 0x46, 0x00, 0x43, 0x00, 0x72, 0x00, 0x65, 0x00, 0x61, 0x00, 0x74, 0x00, 0x6f, 0x00,  // D.F.C.r.e.a.t.o.
/* 01b0 */ 0x72, 0x00, 0x00, 0x00, 0x50, 0x00, 0x44, 0x00, 0x46, 0x00, 0x43, 0x00, 0x72, 0x00, 0x65, 0x00,  // r...P.D.F.C.r.e.
/* 01c0 */ 0x61, 0x00, 0x74, 0x00, 0x6f, 0x00, 0x72, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x01, 0x00,  // a.t.o.r.........
/* 01d0 */ 0x00, 0x00, 0x43, 0x3a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,        // ..C:..........
    };

    StaticStream source_stream(source_data, sizeof(source_data));

    source_stream.in_skip_bytes(8);    /* Component(2) + PacketId(2) + DeviceCount(4) */

    BStream result(65535);

    const uint32_t device_count = 4;

    AuthorizationChannels authorization_channels("*", "rdpdr_drive");

    FileSystemDriveManager file_system_driver_manager;

    bool     device_capability_version_02_supported = false;
    uint32_t verbose                                = 0;

    uint32_t real_device_count = mod_rdp::filter_unsupported_device(authorization_channels,
        source_stream, device_count, result, file_system_driver_manager,
        device_capability_version_02_supported,
        verbose);

    hexdump_c(result.get_data(), result.size());

    BOOST_CHECK_EQUAL(real_device_count, 3);
    BOOST_CHECK_EQUAL(sizeof(result_data), result.size());
    BOOST_CHECK_EQUAL(memcmp(result_data, result.get_data(), sizeof(result_data)), 0);
}
