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

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/transport/test_transport.hpp"
#include "test_only/front/front_wrapper.hpp"
#include "test_only/lcg_random.hpp"
#include "test_only/core/font.hpp"

// Comment the code block below to generate testing data.
// Uncomment the code block below to generate testing data.

#include "acl/auth_api.hpp"
#include "acl/license_api.hpp"
#include "capture/cryptofile.hpp"
#include "configs/config.hpp"
// Uncomment the code block below to generate testing data.
//include "transport/socket_transport.hpp"
#include "core/client_info.hpp"
#include "utils/theme.hpp"
#include "utils/redirection_info.hpp"

#include "mod/null/null.hpp"
#include "mod/rdp/new_mod_rdp.hpp"
#include "mod/rdp/rdp_params.hpp"
#include "mod/rdp/mod_rdp_factory.hpp"
#include "acl/auth_api.hpp"
#include "core/channel_list.hpp"
#include "core/events.hpp"
#include "utils/timebase.hpp"
#include "core/channels_authorizations.hpp"
#include "gdi/osd_api.hpp"


namespace dump2008 {
    #include "fixtures/dump_w2008.hpp"
} // namespace dump2008

// namespace dump2008_PatBlt {
//     include "fixtures/dump_w2008_PatBlt.hpp"
// } // namespace dump2008_PatBlt


class MyFront : public FrontWrapper
{
public:
    bool can_be_start_capture(bool /*force_capture*/, SessionLogApi& /*session_log*/) override { return false; }
    bool must_be_stop_capture() override { return false; }
    void must_flush_capture() override {}

    using FrontWrapper::FrontWrapper;

    void send_to_channel(
        const CHANNELS::ChannelDef & /*channel*/,
        bytes_view /*chunk_data*/,
        std::size_t /*total_length*/,
        int  /*flags*/) override
    {
    }
};

struct FrontTransport : GeneratorTransport
{
    using GeneratorTransport::GeneratorTransport;

    void do_send(const uint8_t * const /*buffer*/, size_t /*len*/) override
    {
        // TEST test
    }
};


RED_AUTO_TEST_CASE(TestFront)
{
    ClientInfo info;
    info.keylayout = 0x04C;
    info.console_session = false;
    info.brush_cache_code = 0;
    info.screen_info.bpp = BitsPerPixel{24};
    info.screen_info.width = 800;
    info.screen_info.height = 600;
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

    ini.set<cfg::crypto::encryption_key>(
        "\x00\x01\x02\x03\x04\x05\x06\x07"
        "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
        "\x10\x11\x12\x13\x14\x15\x16\x17"
        "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
    );
    ini.set<cfg::crypto::sign_key>(
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

    // Comment the code block below to generate testing data.
    #include "fixtures/trace_front_client.hpp"

    // Comment the code block below to generate testing data.
    FrontTransport front_trans(cstr_array_view(indata));

    LCGRandom gen1;
    CryptoContext cctx;

    const bool fastpath_support = false;

    ini.set<cfg::client::tls_support>(false);
    ini.set<cfg::client::tls_fallback_legacy>(true);
    ini.set<cfg::client::bogus_user_id>(false);
    ini.set<cfg::client::rdp_compression>(RdpCompression::none);
    ini.set<cfg::client::fast_path>(fastpath_support);
    ini.set<cfg::globals::is_rec>(true);
    ini.set<cfg::video::capture_flags>(CaptureFlags::wrm);
    ini.set<cfg::globals::handshake_timeout>(std::chrono::seconds::zero());

    EventContainer events;
    NullSessionLog session_log;

    RED_TEST_PASSPOINT();

    MyFront front(
        events, session_log, front_trans, gen1, ini , cctx,
        fastpath_support);
    null_mod no_mod;

    gdi::NullOsd osd;

    while (!front.is_up_and_running()) {
        front.incoming(no_mod);
        RED_CHECK(events.queue.empty());
    }

    // LOG(LOG_INFO, "hostname=%s", front.client_info.hostname);

    // int client_sck = ip_connect("10.10.47.36", 3389, 3, 1000);
    // std::string error_message;
    // SocketTransport t( name
    //                  , client_sck
    //                  , "10.10.47.36"
    //                  , 3389
    //                  , verbose
    //                  , &error_message
    //                  );

    FrontTransport t(cstr_array_view(dump2008::indata));

    Theme theme;

    std::array<uint8_t, 28> server_auto_reconnect_packet {};
    ModRDPParams mod_rdp_params( "administrateur"
                                , "S3cur3!1nux"
                                , "10.10.47.36"
                                , "10.10.43.33"
                                , 2
                                , global_font()
                                , theme
                                , server_auto_reconnect_packet
                                , ini.get_mutable_ref<cfg::context::close_box_extra_message>()
                                , RDPVerbose(0)
                                );
    mod_rdp_params.device_id                       = "device_id";
    mod_rdp_params.enable_tls                      = false;
    mod_rdp_params.enable_nla                      = false;
    //mod_rdp_params.enable_krb                      = false;
    //mod_rdp_params.enable_clipboard                = true;
    mod_rdp_params.enable_fastpath                 = false;
    mod_rdp_params.disabled_orders                 = TS_NEG_MEM3BLT_INDEX | TS_NEG_DRAWNINEGRID_INDEX | TS_NEG_MULTI_DRAWNINEGRID_INDEX |
                                                     TS_NEG_SAVEBITMAP_INDEX | TS_NEG_MULTIDSTBLT_INDEX | TS_NEG_MULTIPATBLT_INDEX |
                                                     TS_NEG_MULTISCRBLT_INDEX | TS_NEG_MULTIOPAQUERECT_INDEX | TS_NEG_FAST_INDEX_INDEX |
                                                     TS_NEG_POLYGON_SC_INDEX | TS_NEG_POLYGON_CB_INDEX | TS_NEG_POLYLINE_INDEX |
                                                     TS_NEG_FAST_GLYPH_INDEX | TS_NEG_ELLIPSE_SC_INDEX | TS_NEG_ELLIPSE_CB_INDEX;
    mod_rdp_params.enable_new_pointer              = false;
    //mod_rdp_params.rdp_compression                 = 0;
    //mod_rdp_params.error_message                   = nullptr;
    //mod_rdp_params.disconnect_on_logon_user_change = false;
    mod_rdp_params.open_session_timeout            = 5s;
    //mod_rdp_params.certificate_change_action       = 0;
    //mod_rdp_params.extra_orders                    = "";
    mod_rdp_params.verbose = RDPVerbose(verbose);

    // To always get the same client random, in tests
    LCGRandom gen2;

    NullLicenseStore license_store;
    class RDPMetrics * metrics = nullptr;
    const ChannelsAuthorizations channels_authorizations{};
    ModRdpFactory mod_rdp_factory;

    FileValidatorService * file_validator_service = nullptr;

    TLSClientParams tls_client_params;
    RedirectionInfo redir_info;

    auto mod = new_mod_rdp(
        t, front.gd(), osd, events, session_log, front, info, redir_info,
        gen2, channels_authorizations, mod_rdp_params, tls_client_params,
        license_store, ini, metrics,
        file_validator_service, mod_rdp_factory);

    // incoming connexion data
    RED_CHECK_EQUAL(front.screen_info().width, 1024);
    RED_CHECK_EQUAL(front.screen_info().height, 768);

    // Force Front to be up and running after Deactivation-Reactivation
    //  Sequence initiated by mod_rdp.

    RED_TEST_PASSPOINT();

    int count = 0;
    int n = 38;
    events.queue[0]->alarm.fd = 0;
    for (; count < n && !events.queue.empty(); ++count) {
        events.execute_events(timeval{1,0},[](int){return true;}, false);
    }

    RED_CHECK_EQ(count, n);
//    front.dump_png("trace_w2008_");
}

RED_AUTO_TEST_CASE(TestFront2)
{
    ClientInfo info;
    info.keylayout = 0x04C;
    info.console_session = false;
    info.brush_cache_code = 0;
    info.screen_info.bpp = BitsPerPixel{24};
    info.screen_info.width = 800;
    info.screen_info.height = 600;
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

    ini.set<cfg::crypto::encryption_key>(
        "\x00\x01\x02\x03\x04\x05\x06\x07"
        "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
        "\x10\x11\x12\x13\x14\x15\x16\x17"
        "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
    );
    ini.set<cfg::crypto::sign_key>(
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

    // Comment the code block below to generate testing data.
    #include "fixtures/trace_front_client.hpp"

    // Comment the code block below to generate testing data.
    GeneratorTransport front_trans(cstr_array_view(indata));
    front_trans.disable_remaining_error();

    LCGRandom gen1;
    CryptoContext cctx;
    const bool fastpath_support = false;

    ini.set<cfg::client::tls_support>(false);
    ini.set<cfg::client::tls_fallback_legacy>(true);
    ini.set<cfg::client::bogus_user_id>(false);
    ini.set<cfg::client::rdp_compression>(RdpCompression::none);
    ini.set<cfg::client::fast_path>(fastpath_support);
    ini.set<cfg::globals::is_rec>(true);
    ini.set<cfg::video::capture_flags>(CaptureFlags::wrm);

    EventContainer events;

    RED_TEST_PASSPOINT();
    NullSessionLog session_log;
    MyFront front(events, session_log, front_trans, gen1, ini
                 , cctx, fastpath_support);
    null_mod no_mod;

//    RED_TEST_PASSPOINT();

//    RED_REQUIRE(!events_.is_empty());
//    events.set_current_time({ini.get<cfg::globals::handshake_timeout>().count(), 0});

//    RED_CHECK_EXCEPTION_ERROR_ID(fn(), ERR_RDP_HANDSHAKE_TIMEOUT);

    // LOG(LOG_INFO, "hostname=%s", front.client_info.hostname);
    //
    // // int client_sck = ip_connect("10.10.47.36", 3389, 3, 1000);
    // // std::string error_message;
    // // SocketTransport t( name
    // //                  , client_sck
    // //                  , "10.10.47.36"
    // //                  , 3389
    // //                  , verbose
    // //                  , &error_message
    // //                  );
    //
    // GeneratorTransport t(cstr_array_view(dump2008::indata));
    //
    // if (verbose > 2){
    //     LOG(LOG_INFO, "--------- CREATION OF MOD ------------------------");
    // }
    //
    // ModRDPParams mod_rdp_params( "administrateur"
    //                            , "S3cur3!1nux"
    //                            , "10.10.47.36"
    //                            , "10.10.43.33"
    //                            , 2
    //                            , ini.get<cfg::font>()
    //                            , ini.get<cfg::theme>()
    //                            , ini.get_mutable_ref<cfg::context::server_auto_reconnect_packet>()
    //                            , ini.get_mutable_ref<cfg::context::close_box_extra_message>()
    //                            , RDPVerbose(0)
    //                            );
    // mod_rdp_params.device_id                       = "device_id";
    // mod_rdp_params.enable_tls                      = false;
    // mod_rdp_params.enable_nla                      = false;
    // //mod_rdp_params.enable_krb                      = false;
    // //mod_rdp_params.enable_clipboard                = true;
    // mod_rdp_params.enable_fastpath                 = false;
    // mod_rdp_params.enable_mem3blt                  = false;
    // mod_rdp_params.enable_new_pointer              = false;
    // //mod_rdp_params.rdp_compression                 = 0;
    // //mod_rdp_params.error_message                   = nullptr;
    // //mod_rdp_params.disconnect_on_logon_user_change = false;
    // //mod_rdp_params.open_session_timeout            = 5s;
    // //mod_rdp_params.certificate_change_action       = 0;
    // //mod_rdp_params.extra_orders                    = "";
    // mod_rdp_params.verbose = RDPVerbose(verbose);
    //
    // // To always get the same client random, in tests
    // LCGRandom gen2;
    //
    // front.clear_channels();
    //
    // auto mod = new_mod_rdp(t, front, front, info, ini.get_mutable_ref<cfg::mod_rdp::redir_info>(), gen2, mod_rdp_params, report_message, ini, nullptr);
    //
    // if (verbose > 2){
    //     LOG(LOG_INFO, "========= CREATION OF MOD DONE ====================\n\n");
    // }
    // RED_CHECK_EQUAL(front.client_info.width, 800);
    // RED_CHECK_EQUAL(front.client_info.height, 600);
    //
    // while (!mod->is_up_and_running())
    //     mod->draw_event(now, front);
    //
    // // Force Front to be up and running after Deactivation-Reactivation
    // //  Sequence initiated by mod_rdp.
    // front.up_and_running = 1;
    //
    // LOG(LOG_INFO, "Before Start Capture");
    //
    // front.can_be_start_capture(false);
    //
    // uint32_t count = 0;
    // BackEvent_t res = BACK_EVENT_NONE;
    // while (res == BACK_EVENT_NONE){
    //     LOG(LOG_INFO, "===================> count = %u", count);
    //     if (count++ >= 38) break;
    //     mod->draw_event(now, front);
    //     now++;
    //     LOG(LOG_INFO, "Calling Snapshot");
    //     front.periodic_snapshot();
    // }
    //
    // front.must_be_stop_capture();

//    front.dump_png("trace_w2008_");
}

/*
RED_AUTO_TEST_CASE(TestFront3)
{
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
    Sesman & sesman;

    Inifile ini;
    ini.set<cfg::debug::front>(verbose);
    ini.set<cfg::client::persistent_disk_bitmap_cache>(false);
    ini.set<cfg::client::cache_waiting_list>(true);
    ini.set<cfg::mod_rdp::persistent_disk_bitmap_cache>(false);
    ini.set<cfg::video::png_interval>(std::chrono::seconds{300});
    ini.set<cfg::video::wrm_color_depth_selection_strategy>(ColorDepthSelectionStrategy::depth24);
    ini.set<cfg::video::wrm_compression_algorithm>(WrmCompressionAlgorithm::no_compression);

    ini.set<cfg::crypto::encryption_key>(
        "\x00\x01\x02\x03\x04\x05\x06\x07"
        "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
        "\x10\x11\x12\x13\x14\x15\x16\x17"
        "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
    );
    ini.set<cfg::crypto::sign_key>(
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
    // GeneratorTransport front_trans(cstr_array_view(indata), verbose);
    TestTransport front_trans(cstr_array_view(indata), cstr_array_view(outdata));

    LCGRandom gen1;
    CryptoContext cctx;
    const bool fastpath_support = false;

    ini.set<cfg::client::tls_support>(false);
    ini.set<cfg::client::tls_fallback_legacy>(true);
    ini.set<cfg::client::bogus_user_id>(false);
    ini.set<cfg::client::rdp_compression>(RdpCompression::none);
    ini.set<cfg::client::fast_path>(fastpath_support);
    ini.set<cfg::globals::is_rec>(true);
    ini.set<cfg::video::capture_flags>(CaptureFlags::wrm);

    NullAuthentifier report_message;
    MyFront front( front_trans, gen1, ini
                 , cctx, report_message, fastpath_support
                 , now - ini.get<cfg::globals::handshake_timeout>().count());
    null_mod no_mod;

    front.get_event().set_trigger_time(wait_obj::NOW);
    bool is_set = front.get_event().is_set();

    while (front.up_and_running == 0) {
        front.incoming(no_mod, sesman);

        front.get_event().reset_trigger_time();
        RED_CHECK(!front.get_event().is_waked_up_by_time());
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

    GeneratorTransport t(cstr_array_view(dump2008_PatBlt::indata));

    if (verbose > 2){
        LOG(LOG_INFO, "--------- CREATION OF MOD ------------------------");
    }

    ModRDPParams mod_rdp_params( "administrateur"
                               , "S3cur3!1nux"
                               , "10.10.47.36"
                               , "10.10.43.33"
                               , 2
                               , ini.get<cfg::font>()
                               , ini.get<cfg::theme>()
                               , ini.get_mutable_ref<cfg::context::server_auto_reconnect_packet>()
                               , ini.get_mutable_ref<cfg::context::close_box_extra_message>()
                               , RDPVerbose(0)
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
    //mod_rdp_params.open_session_timeout            = 5s;
    //mod_rdp_params.certificate_change_action       = 0;
    //mod_rdp_params.extra_orders                    = "";
    mod_rdp_params.verbose = RDPVerbose(verbose);

    // To always get the same client random, in tests
    LCGRandom gen2;

    front.clear_channels();

    mod_rdp mod(t, front, front, info, ini.get_mutable_ref<cfg::mod_rdp::redir_info>(), gen2, mod_rdp_params, report_message, ini);


    if (verbose > 2){
        LOG(LOG_INFO, "========= CREATION OF MOD DONE ====================\n\n");
    }
    RED_CHECK_EQUAL(front.client_info.width, 1024);
    RED_CHECK_EQUAL(front.client_info.height, 768);

    // Force Front to be up and running after Deactivation-Reactivation
    //  Sequence initiated by mod_rdp.
    front.up_and_running = 1;

    LOG(LOG_INFO, "Before Start Capture");

    front.can_be_start_capture(false);

    uint32_t count = 0;
    BackEvent_t res = BACK_EVENT_NONE;
    while (res == BACK_EVENT_NONE){
        LOG(LOG_INFO, "===================> count = %u", count);
        if (count++ >= 46) break;
        mod.draw_event(now, front);
        now++;
        LOG(LOG_INFO, "Calling Snapshot");
        front.periodic_snapshot();
    }

    front.must_be_stop_capture();

    // front.dump_png("trace_w2008_");
}
*/
