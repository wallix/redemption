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
#define BOOST_TEST_MODULE TestRdpClientTLSW2008
#include <boost/test/auto_unit_test.hpp>
#include <errno.h>
#include <algorithm>

#define LOGNULL
//#define LOGPRINT
#include "test_orders.hpp"

#include "stream.hpp"
#include "transport.hpp"
#include "testtransport.hpp"
#include "RDP/x224.hpp"
#include "RDP/mcs.hpp"
#include "RDP/sec.hpp"
#include "wait_obj.hpp"
#include "RDP/RDPGraphicDevice.hpp"
#include "channel_list.hpp"
#include "client_info.hpp"
#include "rdp/rdp.hpp"
#include "ssl_calls.hpp"
#include "png.hpp"
#include "RDP/RDPDrawable.hpp"
#include "staticcapture.hpp"

#include "../front/fake_front.hpp"

BOOST_AUTO_TEST_CASE(TestDecodePacket)
{
    int verbose = 256;

    ClientInfo info(1, true, true);
    info.keylayout             = 0x040C;
    info.console_session       = 0;
    info.brush_cache_code      = 0;
    info.bpp                   = 16;
    info.width                 = 1024;
    info.height                = 768;
    info.rdp5_performanceflags =   PERF_DISABLE_WALLPAPER
                                 | PERF_DISABLE_FULLWINDOWDRAG | PERF_DISABLE_MENUANIMATIONS;


    FakeFront front(info, verbose);

    const char * name       = "RDP W2008 TLS Target";
//     int          client_sck = ip_connect("192.168.1.50", 3389, 3, 1000, verbose);

//     redemption::string  error_message;
//     SocketTransport     t( name
//                          , client_sck
//                          , "192.168.1.50"
//                          , 3389
//                          , verbose
//                          , &error_message
//                          );

    #include "fixtures/dump_TLSw2008.hpp"
    TestTransport t(name, indata, sizeof(indata), outdata, sizeof(outdata), verbose);

    if (verbose > 2) {
        LOG(LOG_INFO, "--------- CREATION OF MOD ------------------------");
    }

    snprintf(info.hostname, sizeof(info.hostname), "192-168-1-100");

    ModRDPParams mod_rdp_params( "administrateur@qa"
                               , "S3cur3!1nux"
                               , "192.168.1.50"
                               , "192.168.1.100"
                               , 7
                               , 511
                               );
    //mod_rdp_params.enable_tls                      = true;
    mod_rdp_params.enable_nla                      = false;
    //mod_rdp_params.enable_krb                      = false;
    //mod_rdp_params.enable_clipboard                = true;
    mod_rdp_params.enable_fastpath                 = false;
    mod_rdp_params.enable_mem3blt                  = false;
    //mod_rdp_params.enable_bitmap_update            = false;
    mod_rdp_params.enable_new_pointer              = false;
    //mod_rdp_params.rdp_compression                 = 0;
    //mod_rdp_params.error_message                   = NULL;
    //mod_rdp_params.disconnect_on_logon_user_change = false;
    //mod_rdp_params.open_session_timeout            = 0;
    //mod_rdp_params.certificate_change_action       = 0;
    //mod_rdp_params.extra_orders                    = "";

    // To always get the same client random, in tests
    LCGRandom gen(0);
    struct mod_api * mod = new mod_rdp(&t, front, info, gen, mod_rdp_params);

    if (verbose > 2) {
        LOG(LOG_INFO, "========= CREATION OF MOD DONE ====================\n\n");
    }
    BOOST_CHECK(t.get_status());

    BOOST_CHECK_EQUAL(mod->front_width,  1024);
    BOOST_CHECK_EQUAL(mod->front_height, 768);

    uint32_t count = 0;
    BackEvent_t res = BACK_EVENT_NONE;
    while (res == BACK_EVENT_NONE) {
        LOG(LOG_INFO, "===================> count = %u", count);
        if (count++ >= 40) break;
        mod->draw_event(time(NULL));
    }

//    front.dump_png("trace_w2008_tls_");
}
