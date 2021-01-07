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

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "capture/cryptofile.hpp"
#include "mod/null/null.hpp"
#include "mod/internal/test_card_mod.hpp"
#include "test_only/transport/test_transport.hpp"
#include "test_only/front/front_wrapper.hpp"
#include "acl/auth_api.hpp"
// Uncomment the code block below to generate testing data.
// include "core/listen.hpp"
// include "core/session.hpp"
// include "transport/socket_transport.hpp"

#include "test_only/lcg_random.hpp"
#include "test_only/core/font.hpp"
#include "core/events.hpp"
#include "utils/timebase.hpp"
#include "configs/config.hpp"


// Uncomment the code block below to generate testing data.
// include <netinet/tcp.h>

RED_AUTO_TEST_CASE(TestIncomingConnection)
{
    // Uncomment the code block below to generate testing data.
    //int port = 3389;
    //unique_fd sck_server = create_server(0, port);
    //unique_server_loop(std::move(sck_server), [&](int sck)
    //{
    //    union {
    //        struct sockaddr s;
    //        struct sockaddr_storage ss;
    //        struct sockaddr_in s4;
    //        struct sockaddr_in6 s6;
    //    } u;
    //    unsigned int sin_size = sizeof(u);
    //    memset(&u, 0, sin_size);
    //    int one_shot_server_sck = accept(sck, nullptr, nullptr);





    Inifile ini;
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
    //if (-1 == setsockopt( one_shot_server_sck, IPPROTO_TCP, TCP_NODELAY
    //                   , (char *)&nodelay, sizeof(nodelay))) {
    //   LOG(LOG_INFO, "Failed to set socket TCP_NODELAY option on client socket");
    //}
    //SocketTransport front_trans( "RDP Client", unique_fd{one_shot_server_sck}, "0.0.0.0", 0
    //                          , std::chrono::seconds(1), SocketTransport::Verbose::dump);

    // Comment the code block below to generate testing data.
    #include "fixtures/trace_mstsc_client_rdp50bulk.hpp"

    // Comment the code block below to generate testing data.
    TestTransport front_trans(cstr_array_view(indata), cstr_array_view(outdata));

    ini.set<cfg::client::tls_support>(true);
    ini.set<cfg::client::tls_fallback_legacy>(false);
    ini.set<cfg::client::bogus_user_id>(false);
    ini.set<cfg::client::rdp_compression>(RdpCompression::rdp5);
    ini.set<cfg::globals::large_pointer_support>(false);
    ini.set<cfg::globals::unicode_keyboard_event_support>(false);
    ini.set<cfg::client::disabled_orders>("4,15,16,17,18");

    LCGRandom gen;
    CryptoContext cctx;
    const bool fastpath_support = true;
    EventContainer events;
    NullSessionLog session_log;
    FrontWrapper front(events, session_log, front_trans, gen, ini, cctx, fastpath_support);
    null_mod no_mod;

    while (!front.is_up_and_running()) {
        front.incoming(no_mod);
    }

    //LOG(LOG_INFO, "hostname=%s", front.client_info.hostname);

    // TestCardMod mod(front, front.screen_info().width, front.screen_info().height, global_font());
    // mod.init();

    // Uncomment the code block below to generate testing data.
    //sleep(5);

    // Uncomment the code block below to generate testing data.
    //LOG(LOG_INFO, "Listener closed\n");





    // Uncomment the code block below to generate testing data.
    //    return true;
    //});
}
