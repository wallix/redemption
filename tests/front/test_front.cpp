/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestRdp
#include <boost/test/auto_unit_test.hpp>

#undef SHARE_PATH
#define SHARE_PATH FIXTURES_PATH
#undef RECORD_PATH
#define RECORD_PATH "/tmp/"
#undef WRM_PATH
#define WRM_PATH "/tmp/"

//#define LOGNULL
#define LOGPRINT

#include "config.hpp"
//#include "socket_transport.hpp"
#include "test_transport.hpp"
#include "client_info.hpp"
#include "rdp/rdp.hpp"

#include "front.hpp"
#include "null/null.hpp"
#include "config_spec.hpp"

namespace dump2008 {
    #include "fixtures/dump_w2008.hpp"
}


BOOST_AUTO_TEST_CASE(TestFront)
{
    try {
        ClientInfo info;
        info.keylayout = 0x04C;
        info.console_session = 0;
        info.brush_cache_code = 0;
        info.bpp = 24;
        info.width = 800;
        info.height = 600;
        info.rdp5_performanceflags = PERF_DISABLE_WALLPAPER;
        snprintf(info.hostname,sizeof(info.hostname),"test");
        uint32_t verbose = 511;

        Inifile ini;
        ini.set<cfg::debug::front>(511);
        ini.set<cfg::client::persistent_disk_bitmap_cache>(false);
        ini.set<cfg::client::cache_waiting_list>(true);
        ini.set<cfg::mod_rdp::persistent_disk_bitmap_cache>(false);
        ini.set<cfg::video::png_interval>(3000);
        ini.set<cfg::video::wrm_color_depth_selection_strategy>(0);
        ini.set<cfg::video::wrm_compression_algorithm>(0);

        // Uncomment the code block below to generate testing data.
        //int nodelay = 1;
        //if (-1 == setsockopt( one_shot_server.sck, IPPROTO_TCP, TCP_NODELAY
        //                    , (char *)&nodelay, sizeof(nodelay))) {
        //    LOG(LOG_INFO, "Failed to set socket TCP_NODELAY option on client socket");
        //}
        //SocketTransport front_trans( "RDP Client", one_shot_server.sck, "0.0.0.0", 0
        //                           , ini.get<cfg::debug::front,>() 0);

        LCGRandom gen1(0);

        // Comment the code block below to generate testing data.
        #include "fixtures/trace_front_client.hpp"

        // Comment the code block below to generate testing data.
        GeneratorTransport front_trans(indata, sizeof(indata), verbose);

        BOOST_CHECK(true);

        const bool fastpath_support = false;
        const bool mem3blt_support  = false;

        ini.set<cfg::client::tls_support>(false);
        ini.set<cfg::client::tls_fallback_legacy>(true);
        ini.set<cfg::client::bogus_user_id>(false);
        ini.set<cfg::client::rdp_compression>(0);
        ini.set<cfg::client::fast_path>(fastpath_support);
        ini.set<cfg::globals::movie>(true);
        ini.set<cfg::video::capture_flags>(configs::CaptureFlags::wrm);


        Front front( front_trans, SHARE_PATH "/" DEFAULT_FONT_NAME, gen1, ini
                   , fastpath_support, mem3blt_support);
        null_mod no_mod(front);

        while (front.up_and_running == 0) {
            front.incoming(no_mod);
        }

        LOG(LOG_INFO, "hostname=%s", front.client_info.hostname);

        const char * name = "RDP W2008 Target";

        // int client_sck = ip_connect("10.10.47.36", 3389, 3, 1000, verbose);
        // std::string error_message;
        // SocketTransport t( name
        //                  , client_sck
        //                  , "10.10.47.36"
        //                  , 3389
        //                  , verbose
        //                  , &error_message
        //                  );

        GeneratorTransport t(dump2008::indata, sizeof(dump2008::indata), verbose);

        if (verbose > 2){
            LOG(LOG_INFO, "--------- CREATION OF MOD ------------------------");
        }

         BOOST_CHECK(true);

         ModRDPParams mod_rdp_params( "administrateur"
                                   , "S3cur3!1nux"
                                   , "10.10.47.36"
                                   , "10.10.43.33"
                                   , 2
                                   , 0
                                   );
        mod_rdp_params.device_id                       = "device_id";
        mod_rdp_params.enable_tls                      = false;
        mod_rdp_params.enable_nla                      = false;
        //mod_rdp_params.enable_krb                      = false;
        //mod_rdp_params.enable_clipboard                = true;
        mod_rdp_params.enable_fastpath                 = false;
        mod_rdp_params.enable_mem3blt                  = false;
        mod_rdp_params.enable_bitmap_update            = true;
        mod_rdp_params.enable_new_pointer              = false;
        //mod_rdp_params.rdp_compression                 = 0;
        //mod_rdp_params.error_message                   = nullptr;
        //mod_rdp_params.disconnect_on_logon_user_change = false;
        //mod_rdp_params.open_session_timeout            = 0;
        //mod_rdp_params.certificate_change_action       = 0;
        //mod_rdp_params.extra_orders                    = "";
        mod_rdp_params.server_redirection_support        = true;
        mod_rdp_params.verbose = verbose;

        // To always get the same client random, in tests
        LCGRandom gen2(0);

        BOOST_CHECK(true);
        mod_rdp mod_(t, front, info, ini.get_ref<cfg::mod_rdp::redir_info>(), gen2, mod_rdp_params);
        mod_api * mod = &mod_;
         BOOST_CHECK(true);


        if (verbose > 2){
            LOG(LOG_INFO, "========= CREATION OF MOD DONE ====================\n\n");
        }
        BOOST_CHECK(t.get_status());
        BOOST_CHECK_EQUAL(mod->get_front_width(), 800);
        BOOST_CHECK_EQUAL(mod->get_front_height(), 600);

        LOG(LOG_INFO, "Before Start Capture");

        NullAuthentifier blackhole;
        front.start_capture(800, 600, ini, &blackhole);

        uint32_t count = 0;
        BackEvent_t res = BACK_EVENT_NONE;
        while (res == BACK_EVENT_NONE){
            LOG(LOG_INFO, "===================> count = %u", count);
            if (count++ >= 38) break;
            mod->draw_event(time(nullptr));
            LOG(LOG_INFO, "Calling Snapshot");
            front.periodic_snapshot();
        }

        front.stop_capture();

    //    front.dump_png("trace_w2008_");
    } catch (...) {
        LOG(LOG_INFO, "Exiting on Exception");
    };
}

