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

    Unit test to check front-end behavior stays identical
    when connecting from mstsc (mocked up)
*/

#define RED_TEST_MODULE TestFrontMstscClientRDP50Bulk
#include "test_only/test_framework/redemption_unit_tests.hpp"

// Comment the code block below to generate testing data.
// Uncomment the code block below to generate testing data.

#include "capture/cryptofile.hpp"
#include "mod/null/null.hpp"
#include "mod/internal/test_card_mod.hpp"
#include "test_only/transport/test_transport.hpp"
#include "configs/config.hpp"
#include "front/front.hpp"
// Uncomment the code block below to generate testing data.
//include "core/listen.hpp"
//include "core/session.hpp"

#include "test_only/lcg_random.hpp"
#include "test_only/core/font.hpp"


RED_AUTO_TEST_CASE(TestIncomingConnection)
{
    // Uncomment the code block below to generate testing data.
    //// This server only support one incoming connection before closing listener
    //class ServerOnce : public Server {
    //public:
    //    int  sck;
    //    char ip_source[256];
    //
    //    ServerOnce() : sck(0) {
    //        ip_source[0] = 0;
    //    }
    //
    //    virtual Server_status start(int incoming_sck) {
    //        union {
    //            struct sockaddr s;
    //            struct sockaddr_storage ss;
    //            struct sockaddr_in s4;
    //            struct sockaddr_in6 s6;
    //        } u;
    //        unsigned int sin_size = sizeof(u);
    //        memset(&u, 0, sin_size);
    //        this->sck = accept(incoming_sck, &u.s, &sin_size);
    //        strcpy(ip_source, inet_ntoa(u.s4.sin_addr));
    //        LOG(LOG_INFO, "Incoming socket to %d (ip=%s)\n", sck, ip_source);
    //        return START_WANT_STOP;
    //    }
    //} one_shot_server;
    //Listen listener(one_shot_server, 0, 3389, true, 5); // 25 seconds to connect, or timeout
    //listener.run();

    Inifile ini;
    // ini.set<cfg::font>(Font(app_path_s(AppPath::Share) + "/sans-10.fv1"));
    ini.set<cfg::debug::front>(511);
    ini.set<cfg::client::persistent_disk_bitmap_cache>(false);
    ini.set<cfg::client::cache_waiting_list>(true);
    ini.set<cfg::mod_rdp::persistent_disk_bitmap_cache>(false);
    ini.set<cfg::video::png_interval>(std::chrono::seconds{300});
    ini.set<cfg::video::wrm_color_depth_selection_strategy>(ColorDepthSelectionStrategy::depth24);
    ini.set<cfg::video::wrm_compression_algorithm>(WrmCompressionAlgorithm::no_compression);
    ini.set<cfg::globals::experimental_enable_serializer_data_block_size_limit>(true);

    // Uncomment the code block below to generate testing data.
    //int nodelay = 1;
    //if (-1 == setsockopt( one_shot_server.sck, IPPROTO_TCP, TCP_NODELAY
    //                    , (char *)&nodelay, sizeof(nodelay))) {
    //    LOG(LOG_INFO, "Failed to set socket TCP_NODELAY option on client socket");
    //}
    //SocketTransport front_trans( "RDP Client", one_shot_server.sck, "0.0.0.0", 0
    //                           , ini.get<cfg::debug::front>(), 0);

    // Comment the code block below to generate testing data.
    #include "fixtures/trace_mstsc_client_rdp50bulk.hpp"

    // Comment the code block below to generate testing data.
    TestTransport front_trans(indata, sizeof(indata)-1, outdata, sizeof(outdata)-1);

    ini.set<cfg::client::tls_support>(true);
    ini.set<cfg::client::tls_fallback_legacy>(false);
    ini.set<cfg::client::bogus_user_id>(false);
    ini.set<cfg::client::rdp_compression>(RdpCompression::rdp5);
    ini.set<cfg::globals::large_pointer_support>(false);
    ini.set<cfg::globals::unicode_keyboard_event_support>(false);

    time_t now = 1450864840;

    LCGRandom gen(0);
    CryptoContext cctx;
    const bool fastpath_support = true;
    const bool mem3blt_support  = false;
    NullReportMessage report_message;
    SessionReactor session_reactor;
    Front front(session_reactor, front_trans, gen, ini, cctx, report_message, fastpath_support, mem3blt_support, now);
    front.ignore_rdesktop_bogus_clip = true;
    null_mod no_mod;

    while (front.up_and_running == 0) {
        front.incoming(no_mod, now);
    }

    // LOG(LOG_INFO, "hostname=%s", front.client_info.hostname);

    RED_CHECK_EQUAL(1, front.up_and_running);
    TestCardMod mod(session_reactor, front, front.client_info.screen_info.width, front.client_info.screen_info.height, global_font());
    mod.draw_event(time(nullptr), front);

    // Uncomment the code block below to generate testing data.
    //sleep(5);
    //shutdown(one_shot_server.sck, 2);
    //close(one_shot_server.sck);

    // Uncomment the code block below to generate testing data.
    //LOG(LOG_INFO, "Listener closed\n");
    //LOG( LOG_INFO, "Incoming socket %d (ip=%s)\n", one_shot_server.sck
    //   , one_shot_server.ip_source);
}
