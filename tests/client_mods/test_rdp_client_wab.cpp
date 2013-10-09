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
        sha1.update(StaticStream(data + y * len, len));
    }
    sha1.final(sig);

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
//     int          client_sck = ip_connect("10.10.47.84", 3389, 3, 1000, verbose);

//     redemption::string  error_message;
//     SocketTransport     t( name
//                          , client_sck
//                          , "10.10.47.84"
//                          , 3389
//                          , verbose
//                          , &error_message
//                          );

    #include "fixtures/dump_wab.hpp"
    TestTransport t(name, indata, sizeof(indata), outdata, sizeof(outdata), verbose);

    // To always get the same client random, in tests
    LCGRandom gen(0);

    if (verbose > 2) {
        LOG(LOG_INFO, "--------- CREATION OF MOD ------------------------");
    }
    const bool tls = false;

    snprintf(info.hostname,sizeof(info.hostname),"192-168-1-100");

    struct mod_api * mod = new mod_rdp(
        &t,
        "tester",
        "wallix",
        "192.168.1.100",
        front,
        tls,
        info,
        &gen,
        7,
        NULL,       // auth_api
        "",
        "",         // alternate_shell
        "",         // shell_working_directory
        true,       // clipboard
        false,      // fast-path support
        false,      // mem3blt support
        false,      // bitmap update support
        511,        // verbose
        false
    ); // enable new pointer

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
    "\x0d\x80\x56\x7b\x36\x90\x5a\xb2\x4c\xdb\x2c\x0a\x78\x37\xe3\x3c\xed\x18\x10\xba"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    // front.dump_png("trace_wab_");
}
