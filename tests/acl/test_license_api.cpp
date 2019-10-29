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

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "acl/auth_api.hpp"
#include "configs/config.hpp"
#include "core/client_info.hpp"
#include "core/listen.hpp"
#include "core/report_message_api.hpp"
#include "core/set_server_redirection_target.hpp"
#include "mod/rdp/new_mod_rdp.hpp"
#include "mod/rdp/rdp_params.hpp"
#include "utils/theme.hpp"
#include "utils/netutils.hpp"
#include "system/linux/system/tls_context.hpp"
#include "test_only/front/fake_front.hpp"
#include "test_only/lcg_random.hpp"
#include "test_only/session_reactor_executor.hpp"
#include "test_only/transport/test_transport.hpp"
#include "test_only/core/font.hpp"
#include "transport/socket_transport.hpp"

#include "test_only/acl/test_license_store.hpp"

//#define GENERATE_TESTING_DATA

RED_AUTO_TEST_CASE(TestWithoutExistingLicense)
{
    ClientInfo info;
    info.keylayout             = 0x040C;
    info.console_session       = false;
    info.brush_cache_code      = 0;
    info.screen_info.bpp       = BitsPerPixel{16};
    info.screen_info.width     = 1024;
    info.screen_info.height    = 768;
    info.rdp5_performanceflags =   PERF_DISABLE_WALLPAPER
                                 | PERF_DISABLE_FULLWINDOWDRAG
                                 | PERF_DISABLE_MENUANIMATIONS;

    memset(info.order_caps.orderSupport, 0xFF, sizeof(info.order_caps.orderSupport));
    info.order_caps.orderSupportExFlags = 0xFFFF;

#ifdef GENERATE_TESTING_DATA
    // Uncomment the code block below to generate testing data.
    SSL_library_init();
#endif

    FakeFront front(info.screen_info);

    NullReportMessage report_message;

    Inifile ini;

    ini.set_acl<cfg::context::target_host>("10.10.44.230");
    ini.set_acl<cfg::globals::target_user>("Tester@RED");
    ini.set_acl<cfg::context::target_password>("SecureLinux$42");

    bool already_redirected = false;

#ifndef GENERATE_TESTING_DATA
    // Comment the code block below to generate testing data.
    #include "fixtures/test_license_api_woel_1.hpp"
    #include "fixtures/test_license_api_woel_2.hpp"
    #include "fixtures/test_license_api_license.hpp"
#endif

    for (bool do_work = true; do_work; ) {
        do_work = false;

        try {
#ifdef GENERATE_TESTING_DATA
            // Uncomment the code block below to generate testing data.
            const char * name       = "RDP W2008 TLS Target";
            unique_fd    client_sck = ip_connect(ini.get<cfg::context::target_host>().c_str(), 3389);

            // Uncomment the code block below to generate testing data.
            std::string error_message;
            SocketTransport t( name
                             , std::move(client_sck)
                             , ini.get<cfg::context::target_host>().c_str()
                             , 3389
                             , std::chrono::seconds(1)
                             , SocketTransport::Verbose::dump
                             , &error_message
                             );
#else
            // Comment the code block below to generate testing data.
            TestTransport t(
                    (already_redirected ? cstr_array_view(indata_woel_2) : cstr_array_view(indata_woel_1)),
                    (already_redirected ? cstr_array_view(outdata_woel_2) : cstr_array_view(outdata_woel_1))
                );
#endif

            snprintf(info.hostname, sizeof(info.hostname), "CLT02");

            Theme theme;

            std::array<uint8_t, 28> server_auto_reconnect_packet {};
            ModRDPParams mod_rdp_params( ini.get<cfg::globals::target_user>().c_str()
                                       , ini.get<cfg::context::target_password>().c_str()
                                       , ini.get<cfg::context::target_host>().c_str()
                                       , "10.10.43.33"
                                       , 7
                                       , global_font()
                                       , theme
                                       , server_auto_reconnect_packet
                                       , ini.get_ref<cfg::context::close_box_extra_message>()
                                       , to_verbose_flags(0)
                                       );
            mod_rdp_params.device_id                       = "device_id";
            //mod_rdp_params.enable_tls                      = true;
            mod_rdp_params.enable_nla                      = false;
            //mod_rdp_params.enable_krb                      = false;
            //mod_rdp_params.enable_clipboard                = true;
            mod_rdp_params.enable_fastpath                 = false;
            mod_rdp_params.primary_drawing_orders_support  -= TS_NEG_MEM3BLT_INDEX;
            mod_rdp_params.enable_new_pointer              = false;
            //mod_rdp_params.rdp_compression                 = 0;
            //mod_rdp_params.error_message                   = nullptr;
            //mod_rdp_params.disconnect_on_logon_user_change = false;
            //mod_rdp_params.open_session_timeout            = 0;
            //mod_rdp_params.certificate_change_action       = 0;
            //mod_rdp_params.extra_orders                    = "";
            mod_rdp_params.large_pointer_support             = false;
            mod_rdp_params.experimental_fix_input_event_sync = false;

            mod_rdp_params.load_balance_info = "tsv://MS Terminal Services Plugin.1.Sessions";

            // To always get the same client random, in tests
            LCGRandom gen(0);
            LCGTime timeobj;
            NullAuthentifier authentifier;

#ifdef GENERATE_TESTING_DATA
            CaptureLicenseStore license_store;
#else
            CompareLicenseStore license_store(license_client_name, license_version, license_scope, license_company_name,
                license_product_id, bytes_view(license_data, sizeof(license_data)));
#endif

            SessionReactor session_reactor;

            const ChannelsAuthorizations channels_authorizations{"rdpsnd_audio_output", ""};

            TLSClientParams tls_client_params;

            auto mod = new_mod_rdp(t, session_reactor, front.gd(), front, info,
                ini.get_ref<cfg::mod_rdp::redir_info>(), gen, timeobj,
                channels_authorizations, mod_rdp_params, tls_client_params, authentifier, report_message, license_store, ini,
                nullptr, nullptr);

            RED_CHECK_EQUAL(info.screen_info.width, 1024);
            RED_CHECK_EQUAL(info.screen_info.height, 768);

#ifdef GENERATE_TESTING_DATA
            // Uncomment the code block below to generate testing data.
            session_reactor.execute_timers(
                SessionReactor::EnableGraphics{true},
                [&]()->gdi::GraphicApi&{ return front.gd(); });
            unique_server_loop(unique_fd(t.get_fd()), [&](int sck)->bool {
                (void)sck;
                execute_graphics_event(session_reactor, front.gd());
                LOG(LOG_INFO, "is_up_and_running=%s", (mod->is_up_and_running() ? "Yes" : "No"));
                if (!already_redirected) {
                    return true;
                }
                return !mod->is_up_and_running();
            });
#endif

#ifndef GENERATE_TESTING_DATA
            // Comment the code block below to generate testing data.
            t.disable_remaining_error();
            execute_mod(session_reactor, *mod, front.gd(), 70);
#endif
        }
        catch(Error const & e) {
            if (e.id == ERR_RDP_SERVER_REDIR) {
                LOG(LOG_INFO, "SERVER REDIRECTION");

                set_server_redirection_target(ini, report_message);

                do_work = true;

                already_redirected = true;

                continue;
            }

            throw;
        }
    }
}

RED_AUTO_TEST_CASE(TestWithExistingLicense)
{
    ClientInfo info;
    info.keylayout             = 0x040C;
    info.console_session       = false;
    info.brush_cache_code      = 0;
    info.screen_info.bpp       = BitsPerPixel{16};
    info.screen_info.width     = 1024;
    info.screen_info.height    = 768;
    info.rdp5_performanceflags =   PERF_DISABLE_WALLPAPER
                                 | PERF_DISABLE_FULLWINDOWDRAG
                                 | PERF_DISABLE_MENUANIMATIONS;

    memset(info.order_caps.orderSupport, 0xFF, sizeof(info.order_caps.orderSupport));
    info.order_caps.orderSupportExFlags = 0xFFFF;

#ifdef GENERATE_TESTING_DATA
    // Uncomment the code block below to generate testing data.
    SSL_library_init();
#endif

    FakeFront front(info.screen_info);

    NullReportMessage report_message;

    Inifile ini;

    ini.set_acl<cfg::context::target_host>("10.10.44.230");
    ini.set_acl<cfg::globals::target_user>("Tester@RED");
    ini.set_acl<cfg::context::target_password>("SecureLinux$42");

    bool already_redirected = false;

#ifndef GENERATE_TESTING_DATA
    // Comment the code block below to generate testing data.
    #include "fixtures/test_license_api_wel_1.hpp"
    #include "fixtures/test_license_api_wel_2.hpp"
#endif
    #include "fixtures/test_license_api_license.hpp"

    for (bool do_work = true; do_work; ) {
        do_work = false;

        try {
#ifdef GENERATE_TESTING_DATA
            // Uncomment the code block below to generate testing data.
            const char * name       = "RDP W2008 TLS Target";
            unique_fd    client_sck = ip_connect(ini.get<cfg::context::target_host>().c_str(), 3389);

            // Uncomment the code block below to generate testing data.
            std::string error_message;
            SocketTransport t( name
                             , std::move(client_sck)
                             , ini.get<cfg::context::target_host>().c_str()
                             , 3389
                             , std::chrono::seconds(1)
                             , SocketTransport::Verbose::dump
                             , &error_message
                             );
#else
            // Comment the code block below to generate testing data.
            TestTransport t(
                    (already_redirected ? cstr_array_view(indata_wel_2) : cstr_array_view(indata_wel_1)),
                    (already_redirected ? cstr_array_view(outdata_wel_2) : cstr_array_view(outdata_wel_1))
                );
#endif

            snprintf(info.hostname, sizeof(info.hostname), "CLT02");

            Theme theme;

            std::array<uint8_t, 28> server_auto_reconnect_packet {};
            ModRDPParams mod_rdp_params( ini.get<cfg::globals::target_user>().c_str()
                                       , ini.get<cfg::context::target_password>().c_str()
                                       , ini.get<cfg::context::target_host>().c_str()
                                       , "10.10.43.33"
                                       , 7
                                       , global_font()
                                       , theme
                                       , server_auto_reconnect_packet
                                       , ini.get_ref<cfg::context::close_box_extra_message>()
                                       , to_verbose_flags(0)
                                       );
            mod_rdp_params.device_id                       = "device_id";
            //mod_rdp_params.enable_tls                      = true;
            mod_rdp_params.enable_nla                      = false;
            //mod_rdp_params.enable_krb                      = false;
            //mod_rdp_params.enable_clipboard                = true;
            mod_rdp_params.enable_fastpath                 = false;
            mod_rdp_params.primary_drawing_orders_support  -= TS_NEG_MEM3BLT_INDEX;
            mod_rdp_params.enable_new_pointer              = false;
            //mod_rdp_params.rdp_compression                 = 0;
            //mod_rdp_params.error_message                   = nullptr;
            //mod_rdp_params.disconnect_on_logon_user_change = false;
            //mod_rdp_params.open_session_timeout            = 0;
            //mod_rdp_params.certificate_change_action       = 0;
            //mod_rdp_params.extra_orders                    = "";
            mod_rdp_params.large_pointer_support             = false;
            mod_rdp_params.experimental_fix_input_event_sync = false;

            mod_rdp_params.load_balance_info = "tsv://MS Terminal Services Plugin.1.Sessions";

            // To always get the same client random, in tests
            LCGRandom gen(0);
            LCGTime timeobj;
            NullAuthentifier authentifier;

            ReplayLicenseStore license_store(license_client_name, license_version, license_scope, license_company_name,
                license_product_id, bytes_view(license_data, sizeof(license_data)));

            SessionReactor session_reactor;

            const ChannelsAuthorizations channels_authorizations{"rdpsnd_audio_output", ""};

            TLSClientParams tls_client_params;

            auto mod = new_mod_rdp(t, session_reactor, front.gd(), front, info,
                ini.get_ref<cfg::mod_rdp::redir_info>(), gen, timeobj,
                channels_authorizations, mod_rdp_params, tls_client_params, authentifier, report_message, license_store, ini,
                nullptr, nullptr);

            RED_CHECK_EQUAL(info.screen_info.width, 1024);
            RED_CHECK_EQUAL(info.screen_info.height, 768);

#ifdef GENERATE_TESTING_DATA
            // Uncomment the code block below to generate testing data.
            session_reactor.execute_timers(
                SessionReactor::EnableGraphics{true},
                [&]()->gdi::GraphicApi&{ return front.gd(); });
            unique_server_loop(unique_fd(t.get_fd()), [&](int sck)->bool {
                (void)sck;
                execute_graphics_event(session_reactor, front.gd());
                LOG(LOG_INFO, "is_up_and_running=%s", (mod->is_up_and_running() ? "Yes" : "No"));
                if (!already_redirected) {
                    return true;
                }
                return !mod->is_up_and_running();
            });
#endif

#ifndef GENERATE_TESTING_DATA
            // Comment the code block below to generate testing data.
            t.disable_remaining_error();
            execute_mod(session_reactor, *mod, front.gd(), 70);
#endif
        }
        catch(Error const & e) {
            if (e.id == ERR_RDP_SERVER_REDIR) {
                LOG(LOG_INFO, "SERVER REDIRECTION");

                set_server_redirection_target(ini, report_message);

                do_work = true;

                already_redirected = true;

                continue;
            }

            throw;
        }
    }
}
