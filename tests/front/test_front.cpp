
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

#define RED_TEST_MODULE TestRdp
#include "system/redemption_unit_tests.hpp"

#undef RECORD_PATH
#define RECORD_PATH "/tmp/recorded"
#undef WRM_PATH
#define WRM_PATH "/tmp/recorded"
#undef HASH_PATH
#define HASH_PATH "/tmp/hash"
#undef RECORD_TMP_PATH
#define RECORD_TMP_PATH "/tmp/tmp"

// Comment the code block below to generate testing data.
#define LOGNULL
// Uncomment the code block below to generate testing data.
//#define LOGPRINT

#include "configs/config.hpp"
// Uncomment the code block below to generate testing data.
//#include "transport/socket_transport.hpp"
#include "test_only/transport/test_transport.hpp"
#include "core/client_info.hpp"
#include "mod/rdp/rdp.hpp"
#include "utils/fileutils.hpp"

#include "front/front.hpp"
#include "mod/null/null.hpp"

// Uncomment the code block below to generate testing data.
#include "core/listen.hpp"
#include "core/session.hpp"

#include "test_only/lcg_random.hpp"


namespace dump2008 {
    #include "fixtures/dump_w2008.hpp"
}

namespace dump2008_PatBlt {
    #include "fixtures/dump_w2008_PatBlt.hpp"
}


class MyFront : public Front
{
public:
    bool can_be_start_capture() override { return false; }
    bool must_be_stop_capture() override { return false; }

    MyFront( Transport & trans
            , Random & gen
            , Inifile & ini
            , CryptoContext & cctx
            , auth_api & authentifier
            , bool fp_support // If true, fast-path must be supported
            , bool mem3blt_support
            , time_t now
            , const char * server_capabilities_filename = ""
            , Transport * persistent_key_list_transport = nullptr
            )
    : Front( trans
            , gen
            , ini
            , cctx
            , authentifier
            , fp_support
            , mem3blt_support
            , now
            , server_capabilities_filename
            , persistent_key_list_transport)
    {
    }

    void clear_channels()
    {
        this->channel_list.clear_channels();
    }

    const CHANNELS::ChannelDefArray & get_channel_list(void) const override
    {
        return this->channel_list;
    }

    void send_to_channel(
        const CHANNELS::ChannelDef &,
        uint8_t const *,
        size_t ,
        size_t ,
        int ) override
    {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        LOG(LOG_INFO, "send_to_channel");
        LOG(LOG_INFO, "========================================\n");
    }
};

RED_AUTO_TEST_CASE(TestFront)
{
    try {
        ::unlink(RECORD_PATH "/redemption.mwrm");
        ::unlink(RECORD_PATH "/redemption-000000.wrm");

        ClientInfo info;
        info.keylayout = 0x04C;
        info.console_session = 0;
        info.brush_cache_code = 0;
        info.bpp = 24;
        info.width = 800;
        info.height = 600;
        info.rdp5_performanceflags = PERF_DISABLE_WALLPAPER;
        snprintf(info.hostname,sizeof(info.hostname),"test");
        uint32_t verbose = 3;

        Inifile ini;
        ini.set<cfg::debug::front>(verbose);
        ini.set<cfg::client::persistent_disk_bitmap_cache>(false);
        ini.set<cfg::client::cache_waiting_list>(true);
        ini.set<cfg::mod_rdp::persistent_disk_bitmap_cache>(false);
        ini.set<cfg::video::png_interval>(std::chrono::seconds{300});
        ini.set<cfg::video::wrm_color_depth_selection_strategy>(ColorDepthSelectionStrategy::depth24);
        ini.set<cfg::video::wrm_compression_algorithm>(WrmCompressionAlgorithm::no_compression);

        ini.set<cfg::crypto::key0>(
            "\x00\x01\x02\x03\x04\x05\x06\x07"
            "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
            "\x10\x11\x12\x13\x14\x15\x16\x17"
            "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
        );
        ini.set<cfg::crypto::key1>(
            "\x00\x01\x02\x03\x04\x05\x06\x07"
            "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
            "\x10\x11\x12\x13\x14\x15\x16\x17"
            "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
        );


        // Uncomment the code block below to generate testing data.
        //int nodelay = 1;
        //if (-1 == setsockopt( one_shot_server.sck, IPPROTO_TCP, TCP_NODELAY
        //                    , (char *)&nodelay, sizeof(nodelay))) {
        //    LOG(LOG_INFO, "Failed to set socket TCP_NODELAY option on client socket");
        //}
        //SocketTransport front_trans( "RDP Client", one_shot_server.sck, "0.0.0.0", 0
        //                           , ini.get<cfg::debug::front,>() 0);

        time_t now = 1450864840;

        // Comment the code block below to generate testing data.
        #include "fixtures/trace_front_client.hpp"

        // Comment the code block below to generate testing data.
        GeneratorTransport front_trans(indata, sizeof(indata), verbose);

        RED_CHECK(true);

        LCGRandom gen1(0);
        CryptoContext cctx;

        const bool fastpath_support = false;
        const bool mem3blt_support  = false;

        ini.set<cfg::client::tls_support>(false);
        ini.set<cfg::client::tls_fallback_legacy>(true);
        ini.set<cfg::client::bogus_user_id>(false);
        ini.set<cfg::client::rdp_compression>(RdpCompression::none);
        ini.set<cfg::client::fast_path>(fastpath_support);
        ini.set<cfg::globals::is_rec>(true);
        ini.set<cfg::video::capture_flags>(CaptureFlags::wrm);

        NullAuthentifier authentifier;
        MyFront front( front_trans, gen1, ini , cctx, authentifier, fastpath_support, mem3blt_support
                     , now - ini.get<cfg::globals::handshake_timeout>().count());
        null_mod no_mod(front);

        front.get_event().waked_up_by_time = true;

        while (front.up_and_running == 0) {
            front.incoming(no_mod, now);

            front.get_event().waked_up_by_time = false;
        }

        LOG(LOG_INFO, "hostname=%s", front.client_info.hostname);

        // int client_sck = ip_connect("10.10.47.36", 3389, 3, 1000);
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

         RED_CHECK(true);

         ModRDPParams mod_rdp_params( "administrateur"
                                   , "S3cur3!1nux"
                                   , "10.10.47.36"
                                   , "10.10.43.33"
                                   , 2
                                   , ini.get<cfg::font>()
                                   , ini.get<cfg::theme>()
                                   , ini.get_ref<cfg::context::server_auto_reconnect_packet>()
                                   , to_verbose_flags(0)
                                   );
        mod_rdp_params.device_id                       = "device_id";
        mod_rdp_params.enable_tls                      = false;
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
        mod_rdp_params.server_redirection_support        = true;
        mod_rdp_params.verbose = to_verbose_flags(verbose);

        // To always get the same client random, in tests
        LCGRandom gen2(0);
        LCGTime timeobj;

        RED_CHECK(true);

        front.clear_channels();
        mod_rdp mod_(t, front, info, ini.get_ref<cfg::mod_rdp::redir_info>(), gen2, timeobj, mod_rdp_params, authentifier);
        mod_api * mod = &mod_;
        RED_CHECK(true);


        if (verbose > 2){
            LOG(LOG_INFO, "========= CREATION OF MOD DONE ====================\n\n");
        }
        RED_CHECK(t.get_status());
        // incoming connexion data
        RED_CHECK_EQUAL(front.client_info.width, 1024);
        RED_CHECK_EQUAL(front.client_info.height, 768);


        while (!mod->is_up_and_running())
            mod->draw_event(now, front);

        // Force Front to be up and running after Deactivation-Reactivation
        //  Sequence initiated by mod_rdp.
        front.up_and_running = 1;

        LOG(LOG_INFO, "Before Start Capture");

        front.can_be_start_capture();

        uint32_t count = 0;
        BackEvent_t res = BACK_EVENT_NONE;
        while (res == BACK_EVENT_NONE){
            LOG(LOG_INFO, "===================> count = %u", count);
            if (count++ >= 38) break;
            mod->draw_event(now, front);
            now++;
            LOG(LOG_INFO, "Calling Snapshot");
            front.periodic_snapshot();
        }

        front.must_be_stop_capture();

    //    front.dump_png("trace_w2008_");
    } catch (...) {
        // TEST Error or not error...?
        LOG(LOG_INFO, "Exiting on Exception");
    };
}

RED_AUTO_TEST_CASE(TestFrontGlyph24Bitmap)
{
   const uint8_t bytes_data[] = "\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60"
                                "\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f"
                                "\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08" "\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60"
                                "\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f"
                                "\x08\x60\x1f\x08\xff\xff\xff\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08"
                                "\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\xff\xff\xff\xff\xff\xff\x60" "\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f"
                                "\x08\xff\xff\xff\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08" "\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\xff\xff\xff\x60\x1f\x08\x60"  "\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\xff\xff" "\xff\xff\xff\xff\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08"
                                "\x60\x1f\x08\x60\x1f\x08\xff\xff\xff\x60\x1f\x08\x60\x1f\x08\x60" "\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\xff\xff" "\xff\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08" "\x60\x1f\x08\xff\xff\xff\xff\xff\xff\x60\x1f\x08\x60\x1f\x08\x60" "\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\xff\xff\xff\x60\x1f" "\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08" "\x60\x1f\x08\xff\xff\xff\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60" "\x1f\x08\x60\x1f\x08\x60\x1f\x08\xff\xff\xff\xff\xff\xff\x60\x1f" "\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08"
                                "\xff\xff\xff\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60" "\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f" "\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08" "\xff\xff\xff\xff\xff\xff\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\xff" "\xff\xff\xff\xff\xff\xff\xff\xff\x60\x1f\x08\x60\x1f\x08\x60\x1f" "\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08" "\x60\x1f\x08\x60\x1f\x08\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" "\xff\xff\xff\xff\xff\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f" "\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08" "\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\xff" "\xff\xff\x60\x1f\x08\xff\xff\xff\x60\x1f\x08\x60\x1f\x08\x60\x1f" "\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08" "\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\xff\xff\xff\x60\x1f\x08\xff" "\xff\xff\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f" "\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08" "\x60\x1f\x08\x60\x1f\x08\xff\xff\xff\xff\xff\xff\x60\x1f\x08\x60" "\x1f\x08\x60\x1f\x08\xff\xff\xff\x60\x1f\x08\x60\x1f\x08\x60\x1f" "\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08" "\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60" "\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f" "\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08" "\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60" "\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f"  "\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08"
                                "\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60"
                                "\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f"
                                "\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08\x60\x1f\x08";

    const uint8_t bits_data[] = "\x00\x00\x00\x08\x18\x10\x10\x30\x20\x20\x60\x40\x40\xc0\x80\x00"
                                "\xc7\x00\x3e\x00\x05\x00\x14\x00\x31\x00\x00\x00\x00\x00\x00\x00";

    int16_t offset = 0;
    int16_t baseline = 2;
    uint16_t width = 16;
    uint16_t height = 16;
    int16_t incby = 16;

    FontChar fc(offset, baseline, width, height, incby);

    for (int i = 0; i < 32; i++) {
        fc.data[i] = reinterpret_cast<uint8_t>(bits_data[i]);
    }

    Front::GlyphTo24Bitmap g24b(fc, BGRColor{ 96,  31,   8}, BGRColor{255, 255, 255});

    std::string const out_data(reinterpret_cast<char *>(g24b.raw_data), 256*3);
    std::string const expected(reinterpret_cast<const char *>(bytes_data), 256*3);
    RED_CHECK_EQUAL(expected, out_data);
}

RED_AUTO_TEST_CASE(TestFront2)
{
    auto test = [&]{
        ::unlink(RECORD_PATH "/redemption.mwrm");
        ::unlink(RECORD_PATH "/redemption-000000.wrm");

        ClientInfo info;
        info.keylayout = 0x04C;
        info.console_session = 0;
        info.brush_cache_code = 0;
        info.bpp = 24;
        info.width = 800;
        info.height = 600;
        info.rdp5_performanceflags = PERF_DISABLE_WALLPAPER;
        snprintf(info.hostname,sizeof(info.hostname),"test");
        uint32_t verbose = 3;

        Inifile ini;
        ini.set<cfg::debug::front>(verbose);
        ini.set<cfg::client::persistent_disk_bitmap_cache>(false);
        ini.set<cfg::client::cache_waiting_list>(true);
        ini.set<cfg::mod_rdp::persistent_disk_bitmap_cache>(false);
        ini.set<cfg::video::png_interval>(std::chrono::seconds{300});
        ini.set<cfg::video::wrm_color_depth_selection_strategy>(ColorDepthSelectionStrategy::depth24);
        ini.set<cfg::video::wrm_compression_algorithm>(WrmCompressionAlgorithm::no_compression);

        ini.set<cfg::crypto::key0>(
            "\x00\x01\x02\x03\x04\x05\x06\x07"
            "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
            "\x10\x11\x12\x13\x14\x15\x16\x17"
            "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
        );
        ini.set<cfg::crypto::key1>(
            "\x00\x01\x02\x03\x04\x05\x06\x07"
            "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
            "\x10\x11\x12\x13\x14\x15\x16\x17"
            "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
        );


        // Uncomment the code block below to generate testing data.
        //int nodelay = 1;
        //if (-1 == setsockopt( one_shot_server.sck, IPPROTO_TCP, TCP_NODELAY
        //                    , (char *)&nodelay, sizeof(nodelay))) {
        //    LOG(LOG_INFO, "Failed to set socket TCP_NODELAY option on client socket");
        //}
        //SocketTransport front_trans( "RDP Client", one_shot_server.sck, "0.0.0.0", 0
        //                           , ini.get<cfg::debug::front,>() 0);

        time_t now = 1450864840;

        // Comment the code block below to generate testing data.
        #include "fixtures/trace_front_client.hpp"

        // Comment the code block below to generate testing data.
        GeneratorTransport front_trans(indata, sizeof(indata), verbose);

        RED_CHECK(true);

        LCGRandom gen1(0);
        CryptoContext cctx;
        const bool fastpath_support = false;
        const bool mem3blt_support  = false;

        ini.set<cfg::client::tls_support>(false);
        ini.set<cfg::client::tls_fallback_legacy>(true);
        ini.set<cfg::client::bogus_user_id>(false);
        ini.set<cfg::client::rdp_compression>(RdpCompression::none);
        ini.set<cfg::client::fast_path>(fastpath_support);
        ini.set<cfg::globals::is_rec>(true);
        ini.set<cfg::video::capture_flags>(CaptureFlags::wrm);

        NullAuthentifier authentifier;
        MyFront front( front_trans, gen1, ini
                     , cctx, authentifier, fastpath_support, mem3blt_support
                     , now - ini.get<cfg::globals::handshake_timeout>().count() - 1);
        null_mod no_mod(front);

        front.get_event().waked_up_by_time = true;

        while (front.up_and_running == 0) {
            front.incoming(no_mod, now);

            front.get_event().waked_up_by_time = false;
        }

        LOG(LOG_INFO, "hostname=%s", front.client_info.hostname);

        // int client_sck = ip_connect("10.10.47.36", 3389, 3, 1000);
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

         RED_CHECK(true);

         ModRDPParams mod_rdp_params( "administrateur"
                                   , "S3cur3!1nux"
                                   , "10.10.47.36"
                                   , "10.10.43.33"
                                   , 2
                                   , ini.get<cfg::font>()
                                   , ini.get<cfg::theme>()
                                   , ini.get_ref<cfg::context::server_auto_reconnect_packet>()
                                   , to_verbose_flags(0)
                                   );
        mod_rdp_params.device_id                       = "device_id";
        mod_rdp_params.enable_tls                      = false;
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
        mod_rdp_params.server_redirection_support        = true;
        mod_rdp_params.verbose = to_verbose_flags(verbose);

        // To always get the same client random, in tests
        LCGRandom gen2(0);
        LCGTime timeobj;

        RED_CHECK(true);

        front.clear_channels();
        mod_rdp mod_(t, front, info, ini.get_ref<cfg::mod_rdp::redir_info>(), gen2, timeobj, mod_rdp_params, authentifier);
        mod_api * mod = &mod_;
         RED_CHECK(true);


        if (verbose > 2){
            LOG(LOG_INFO, "========= CREATION OF MOD DONE ====================\n\n");
        }
        RED_CHECK(t.get_status());
        RED_CHECK_EQUAL(front.client_info.width, 800);
        RED_CHECK_EQUAL(front.client_info.height, 600);

        while (!mod->is_up_and_running())
            mod->draw_event(now, front);

        // Force Front to be up and running after Deactivation-Reactivation
        //  Sequence initiated by mod_rdp.
        front.up_and_running = 1;

        LOG(LOG_INFO, "Before Start Capture");

        front.can_be_start_capture();

        uint32_t count = 0;
        BackEvent_t res = BACK_EVENT_NONE;
        while (res == BACK_EVENT_NONE){
            LOG(LOG_INFO, "===================> count = %u", count);
            if (count++ >= 38) break;
            mod->draw_event(now, front);
            now++;
            LOG(LOG_INFO, "Calling Snapshot");
            front.periodic_snapshot();
        }

        front.must_be_stop_capture();

    //    front.dump_png("trace_w2008_");
    };
    RED_CHECK_EXCEPTION_ERROR_ID(test(), ERR_RDP_HANDSHAKE_TIMEOUT);
}

/*
RED_AUTO_TEST_CASE(TestFront3)
{
    try {
        ::unlink(RECORD_PATH "/redemption.mwrm");
        ::unlink(RECORD_PATH "/redemption-000000.wrm");

        ClientInfo info;
        info.keylayout = 0x04C;
        info.console_session = 0;
        info.brush_cache_code = 0;
        info.bpp = 24;
        info.width = 1024;
        info.height = 768;
        info.rdp5_performanceflags = PERF_DISABLE_WALLPAPER;
        snprintf(info.hostname,sizeof(info.hostname),"test");
        uint32_t verbose = 3;

        Inifile ini;
        ini.set<cfg::debug::front>(verbose);
        ini.set<cfg::client::persistent_disk_bitmap_cache>(false);
        ini.set<cfg::client::cache_waiting_list>(true);
        ini.set<cfg::mod_rdp::persistent_disk_bitmap_cache>(false);
        ini.set<cfg::video::png_interval>(std::chrono::seconds{300});
        ini.set<cfg::video::wrm_color_depth_selection_strategy>(ColorDepthSelectionStrategy::depth24);
        ini.set<cfg::video::wrm_compression_algorithm>(WrmCompressionAlgorithm::no_compression);

        ini.set<cfg::crypto::key0>(
            "\x00\x01\x02\x03\x04\x05\x06\x07"
            "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
            "\x10\x11\x12\x13\x14\x15\x16\x17"
            "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
        );
        ini.set<cfg::crypto::key1>(
            "\x00\x01\x02\x03\x04\x05\x06\x07"
            "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
            "\x10\x11\x12\x13\x14\x15\x16\x17"
            "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
        );


        // Uncomment the code block below to generate testing data.
        //int nodelay = 1;
        //if (-1 == setsockopt( one_shot_server.sck, IPPROTO_TCP, TCP_NODELAY
        //                    , (char *)&nodelay, sizeof(nodelay))) {
        //    LOG(LOG_INFO, "Failed to set socket TCP_NODELAY option on client socket");
        //}
        //SocketTransport front_trans( "RDP Client", one_shot_server.sck, "0.0.0.0", 0
        //                           , ini.get<cfg::debug::front,>() 0);

        time_t now = 1450864840;

        // Comment the code block below to generate testing data.
        #include "fixtures/trace_front_client_patblt.hpp"

        // Comment the code block below to generate testing data.
        // GeneratorTransport front_trans(indata, sizeof(indata), verbose);
        TestTransport front_trans("Front", indata, sizeof(indata), outdata, sizeof(outdata) - 1, verbose);

        RED_CHECK(true);

        LCGRandom gen1(0);
        CryptoContext cctx;
        const bool fastpath_support = false;
        const bool mem3blt_support  = false;

        ini.set<cfg::client::tls_support>(false);
        ini.set<cfg::client::tls_fallback_legacy>(true);
        ini.set<cfg::client::bogus_user_id>(false);
        ini.set<cfg::client::rdp_compression>(RdpCompression::none);
        ini.set<cfg::client::fast_path>(fastpath_support);
        ini.set<cfg::globals::is_rec>(true);
        ini.set<cfg::video::capture_flags>(CaptureFlags::wrm);

        NullAuthentifier authentifier;

        class MyFront : public Front
        {
            public:

            MyFront( Transport & trans
                   , const char * default_font_name // SHARE_PATH "/" DEFAULT_FONT_NAME
                   , Random & gen
                   , Inifile & ini
                   , CryptoContext & cctx
                   , auth_api * authentifier
                   , bool fp_support // If true, fast-path must be supported
                   , bool mem3blt_support
                   , time_t now
                   , const char * server_capabilities_filename = ""
                   , Transport * persistent_key_list_transport = nullptr
                   )
            : Front( trans
                   , default_font_name
                   , gen
                   , ini
                   , cctx
                   , authentifier
                   , fp_support
                   , mem3blt_support
                   , now
                   , server_capabilities_filename
                   , persistent_key_list_transport)
                {
                }

                void clear_channels()
                {
                    this->channel_list.clear_channels();
                }

                const CHANNELS::ChannelDefArray & get_channel_list(void) const override
                {
                    return this->channel_list;
                }

                void send_to_channel(
                    const CHANNELS::ChannelDef & channel,
                    uint8_t const * data,
                    size_t length,
                    size_t chunk_size,
                    int flags) override
                {
                    LOG(LOG_INFO, "--------- FRONT ------------------------");
                    LOG(LOG_INFO, "send_to_channel");
                    LOG(LOG_INFO, "========================================\n");
                }
        };

        NullAuthentifier authentifier;

        MyFront front( front_trans, SHARE_PATH "/" DEFAULT_FONT_NAME, gen1, ini
                     , cctx, authentifier, fastpath_support, mem3blt_support
                     , now - ini.get<cfg::globals::handshake_timeout>());
        null_mod no_mod(front);

        front.get_event().waked_up_by_time = true;

        while (front.up_and_running == 0) {
            front.incoming(no_mod, now);

            front.get_event().waked_up_by_time = false;
        }

        LOG(LOG_INFO, "hostname=%s", front.client_info.hostname);

        // int client_sck = ip_connect("10.10.47.36", 3389, 3, 1000);
        // std::string error_message;
        // SocketTransport t( name
        //                  , client_sck
        //                  , "10.10.47.36"
        //                  , 3389
        //                  , verbose
        //                  , &error_message
        //                  );

        GeneratorTransport t(dump2008_PatBlt::indata, sizeof(dump2008_PatBlt::indata), verbose);

        if (verbose > 2){
            LOG(LOG_INFO, "--------- CREATION OF MOD ------------------------");
        }

         RED_CHECK(true);

         ModRDPParams mod_rdp_params( "administrateur"
                                   , "S3cur3!1nux"
                                   , "10.10.47.36"
                                   , "10.10.43.33"
                                   , 2
                                   , ini.get<cfg::font>()
                                   , ini.get<cfg::theme>()
                                   , ini.get_ref<cfg::context::server_auto_reconnect_packet>()
                                   , 0
                                   );
        mod_rdp_params.device_id                       = "device_id";
        mod_rdp_params.enable_tls                      = false;
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
        mod_rdp_params.server_redirection_support        = true;
        mod_rdp_params.verbose = to_verbose_flags(verbose);

        // To always get the same client random, in tests
        LCGRandom gen2(0);

        RED_CHECK(true);

        front.clear_channels();
        mod_rdp mod_(t, front, info, ini.get_ref<cfg::mod_rdp::redir_info>(), gen2, mod_rdp_params, authentifier);
        mod_api * mod = &mod_;
         RED_CHECK(true);


        if (verbose > 2){
            LOG(LOG_INFO, "========= CREATION OF MOD DONE ====================\n\n");
        }
        RED_CHECK(t.get_status());
        RED_CHECK_EQUAL(mod->get_front_width(), 1024);
        RED_CHECK_EQUAL(mod->get_front_height(), 768);

        // Force Front to be up and running after Deactivation-Reactivation
        //  Sequence initiated by mod_rdp.
        front.up_and_running = 1;

        LOG(LOG_INFO, "Before Start Capture");

        front.can_be_start_capture();

        uint32_t count = 0;
        BackEvent_t res = BACK_EVENT_NONE;
        while (res == BACK_EVENT_NONE){
            LOG(LOG_INFO, "===================> count = %u", count);
            if (count++ >= 46) break;
            mod->draw_event(now, front);
            now++;
            LOG(LOG_INFO, "Calling Snapshot");
            front.periodic_snapshot();
        }

        front.stop_capture();

    //    front.dump_png("trace_w2008_");
    } catch (...) {
        LOG(LOG_INFO, "Exiting on Exception");
    };
}
*/
