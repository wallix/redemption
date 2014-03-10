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
#define BOOST_TEST_MODULE TestRdpClientWab
#include <boost/test/auto_unit_test.hpp>
#include <errno.h>
#include <algorithm>

#define LOGNULL
//#define LOGPRINT
#include "test_orders.hpp"

#include "stream.hpp"
#include "transport.hpp"
#include "sockettransport.hpp"
#include "testtransport.hpp"
#include "RDP/x224.hpp"
#include "RDP/mcs.hpp"
#include "RDP/sec.hpp"
#include "wait_obj.hpp"
#include "RDP/RDPGraphicDevice.hpp"
#include "channel_list.hpp"
#include "front_api.hpp"
#include "client_info.hpp"
#include "rdp/rdp.hpp"
#include "ssl_calls.hpp"
#include "png.hpp"
#include "RDP/RDPDrawable.hpp"
#include "staticcapture.hpp"

#include "../front/fake_front.hpp"

inline bool check_sig(const uint8_t* data, std::size_t height, uint32_t len,
                      char * message, const char * shasig)
{
    uint8_t sig[20];
    SslSha1 sha1;
    for (size_t y = 0; y < static_cast<size_t>(height); y++){
        sha1.update(data + y * len, len);
    }
    sha1.final(sig, 20);

    if (memcmp(shasig, sig, 20)){
        sprintf(message, "Expected signature: \""
        "\\x%.2x\\x%.2x\\x%.2x\\x%.2x"
        "\\x%.2x\\x%.2x\\x%.2x\\x%.2x"
        "\\x%.2x\\x%.2x\\x%.2x\\x%.2x"
        "\\x%.2x\\x%.2x\\x%.2x\\x%.2x"
        "\\x%.2x\\x%.2x\\x%.2x\\x%.2x\"",
        sig[ 0], sig[ 1], sig[ 2], sig[ 3],
        sig[ 4], sig[ 5], sig[ 6], sig[ 7],
        sig[ 8], sig[ 9], sig[10], sig[11],
        sig[12], sig[13], sig[14], sig[15],
        sig[16], sig[17], sig[18], sig[19]);
        return false;
    }
    return true;
}

inline bool check_sig(Drawable & data, char * message, const char * shasig)
{
    return check_sig(data.data, data.height, data.rowsize, message, shasig);
}

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

    const char * name       = "RDP Wab Target";

    // int                 client_sck = ip_connect("10.10.47.32", 3389, 3, 1000, verbose);
    // redemption::string  error_message;
    // SocketTransport     t( name
    //                      , client_sck
    //                      , "10.10.47.32"
    //                      , 3389
    //                      , verbose
    //                      , &error_message
    //                      );

    #include "fixtures/dump_wab.hpp"
    TestTransport t(name, indata, sizeof(indata), outdata, sizeof(outdata), verbose);

    if (verbose > 2) {
        LOG(LOG_INFO, "--------- CREATION OF MOD ------------------------");
    }
//    const bool tls = false;

    snprintf(info.hostname, sizeof(info.hostname), "192-168-1-100");

    ModRDPParams mod_rdp_param( "x"
                              , "x"
                              , "10.10.47.32"
                              , "192.168.1.100"
                              , 7
                              , 511
                              );
    mod_rdp_param.enable_tls                      = false;
    mod_rdp_param.enable_nla                      = false;
    //mod_rdp_param.enable_krb                      = false;
    //mod_rdp_param.enable_clipboard                = true;
    mod_rdp_param.enable_fastpath                 = false;
    mod_rdp_param.enable_mem3blt                  = false;
    //mod_rdp_param.enable_bitmap_update            = false;
    mod_rdp_param.enable_new_pointer              = false;
    //mod_rdp_param.rdp_compression                 = 0;
    //mod_rdp_param.error_message                   = NULL;
    //mod_rdp_param.disconnect_on_logon_user_change = false;
    //mod_rdp_param.open_session_timeout            = 0;
    //mod_rdp_param.certificate_change_action       = 0;
    //mod_rdp_param.extra_orders                    = "";

    // To always get the same client random, in tests
    LCGRandom gen(0);
    struct mod_api * mod = new mod_rdp(&t, front, info, gen, mod_rdp_param);

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
        if (count++ >= 12) break;
        mod->draw_event(time(NULL));
    }

    char message[1024];
    if (!check_sig(front.gd.drawable, message,
    "\x8b\xf9\x8e\x38\x8f\x8e\x56\x49\x5a\x85\x40\x90\xfe\xa2\xf6\xd0\xa6\x11\x38\x3b"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    //front.dump_png("trace_wab_");
}
