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
#include "test_only/front/fake_front.hpp"
#include "test_only/lcg_random.hpp"
#include "test_only/transport/test_transport.hpp"
#include "test_only/core/font.hpp"

#include "acl/auth_api.hpp"
#include "acl/license_api.hpp"
#include "configs/config.hpp"
#include "utils/timebase.hpp"
#include "core/client_info.hpp"
#include "core/channels_authorizations.hpp"
#include "mod/rdp/new_mod_rdp.hpp"
#include "mod/rdp/rdp_params.hpp"
#include "mod/rdp/mod_rdp_factory.hpp"
#include "utils/theme.hpp"
#include "utils/redirection_info.hpp"
#include "configs/config.hpp"
#include "gdi/osd_api.hpp"

//#define GENERATE_TESTING_DATA
// remove # otherwise bjam add the file as a dependency (even if it's a comment !!!)
// include "core/listen.hpp"
// include "utils/netutils.hpp"
// include "system/linux/system/tls_context.hpp"
// include "transport/socket_transport.hpp"

using namespace std::chrono_literals;

namespace
{
    ClientInfo get_client_info()
    {
        ClientInfo info;
        info.build                 = 2600;
        info.keylayout             = 0x040C;
        info.console_session       = false;
        info.brush_cache_code      = 0;
        info.screen_info.bpp       = BitsPerPixel{16};
        info.screen_info.width     = 1024;
        info.screen_info.height    = 768;
        info.rdp5_performanceflags =   PERF_DISABLE_WALLPAPER
                                    | PERF_DISABLE_FULLWINDOWDRAG
                                    | PERF_DISABLE_MENUANIMATIONS;

        info.order_caps.orderSupport[TS_NEG_DSTBLT_INDEX]     = 1;
        info.order_caps.orderSupport[TS_NEG_PATBLT_INDEX]     = 1;
        info.order_caps.orderSupport[TS_NEG_SCRBLT_INDEX]     = 1;
        info.order_caps.orderSupport[TS_NEG_MEMBLT_INDEX]     = 1;
        info.order_caps.orderSupport[TS_NEG_MEM3BLT_INDEX]    = 1;
        info.order_caps.orderSupport[TS_NEG_LINETO_INDEX]     = 1;
        info.order_caps.orderSupport[TS_NEG_POLYLINE_INDEX]   = 1;
        info.order_caps.orderSupport[TS_NEG_ELLIPSE_SC_INDEX] = 1;
        info.order_caps.orderSupport[TS_NEG_GLYPH_INDEX]      = 1;

        info.order_caps.orderSupportExFlags = 0xFFFF;

        snprintf(info.hostname, sizeof(info.hostname), "CLT02");

        return info;
    }

    struct LicenseTestContext
    {
        ClientInfo info = get_client_info();
        RedirectionInfo redir_info;
        Inifile ini;

        EventManager event_manager;
        NullSessionLog session_log;

        FakeFront front{info.screen_info};

        Theme theme;
        std::array<uint8_t, 28> server_auto_reconnect_packet {};

        LicenseTestContext()
        {
            #ifdef GENERATE_TESTING_DATA
            SSL_library_init();
            #endif

            ini.set_acl<cfg::context::target_host>("10.10.44.230");
            ini.set_acl<cfg::globals::target_user>("Tester@RED");
            ini.set_acl<cfg::context::target_password>("SecureLinux$42");
        }

        void set_new_target()
        {
            const char * host = char_ptr_cast(redir_info.host);
            const char * password = char_ptr_cast(redir_info.password);
            const char * username = char_ptr_cast(redir_info.username);
            if (redir_info.dont_store_username && username[0] != 0) {
                LOG(LOG_INFO, "SrvRedir: Change target username to '%s'", username);
                ini.set_acl<cfg::globals::target_user>(username);
            }
            if (password[0] != 0) {
                LOG(LOG_INFO, "SrvRedir: Change target password");
                ini.set_acl<cfg::context::target_password>(password);
            }
            LOG(LOG_INFO, "SrvRedir: Change target host to '%s'", host);
            ini.set_acl<cfg::context::target_host>(host);
        }

        ModRDPParams get_mod_rdp_params()
        {
            ModRDPParams mod_rdp_params(
                ini.get<cfg::globals::target_user>().c_str(),
                ini.get<cfg::context::target_password>().c_str(),
                ini.get<cfg::context::target_host>().c_str(),
                "10.10.43.33",
                kbdtypes::KeyLocks::NoLocks,
                global_font(),
                theme,
                server_auto_reconnect_packet,
                ini.get_mutable_ref<cfg::context::close_box_extra_message>(),
                RDPVerbose(0));

            mod_rdp_params.device_id                       = "device_id";
            //mod_rdp_params.enable_tls                      = true;
            mod_rdp_params.enable_nla                      = false;
            //mod_rdp_params.enable_krb                      = false;
            //mod_rdp_params.enable_clipboard                = true;
            mod_rdp_params.enable_fastpath                 = false;
            mod_rdp_params.disabled_orders                 = TS_NEG_MEM3BLT_INDEX
                                                           | TS_NEG_DRAWNINEGRID_INDEX
                                                           | TS_NEG_MULTI_DRAWNINEGRID_INDEX
                                                           | TS_NEG_SAVEBITMAP_INDEX
                                                           | TS_NEG_MULTIDSTBLT_INDEX
                                                           | TS_NEG_MULTIPATBLT_INDEX
                                                           | TS_NEG_MULTISCRBLT_INDEX
                                                           | TS_NEG_MULTIOPAQUERECT_INDEX
                                                           | TS_NEG_FAST_INDEX_INDEX
                                                           | TS_NEG_POLYGON_SC_INDEX
                                                           | TS_NEG_POLYGON_CB_INDEX
                                                           | TS_NEG_POLYLINE_INDEX
                                                           | TS_NEG_FAST_GLYPH_INDEX
                                                           | TS_NEG_ELLIPSE_SC_INDEX
                                                           | TS_NEG_ELLIPSE_CB_INDEX;
            mod_rdp_params.enable_new_pointer              = false;
            //mod_rdp_params.rdp_compression                 = 0;
            //mod_rdp_params.error_message                   = nullptr;
            //mod_rdp_params.disconnect_on_logon_user_change = false;
            mod_rdp_params.open_session_timeout              = 5s;
            //mod_rdp_params.certificate_change_action       = 0;
            //mod_rdp_params.extra_orders                    = "";
            mod_rdp_params.large_pointer_support             = false;

            mod_rdp_params.load_balance_info = "tsv://MS Terminal Services Plugin.1.Sessions";

            return mod_rdp_params;
        }

        gdi::NullOsd osd;
        const ChannelsAuthorizations channels_authorizations{"rdpsnd_audio_output"_zv, ""_zv};
        ModRdpFactory mod_rdp_factory;
        TLSClientParams tls_client_params;
        // To always get the same client random, in tests
        LCGRandom gen;

        std::unique_ptr<mod_api> new_mod_rdp(Transport& trans, LicenseApi& license_store)
        {
            auto mod = ::new_mod_rdp(
                trans, front.gd(), osd, event_manager.get_events(),
                session_log, front, info, redir_info, gen, channels_authorizations,
                get_mod_rdp_params(), tls_client_params, license_store, ini,
                nullptr, nullptr, mod_rdp_factory);
            LOG(LOG_INFO, "--- new mod");
            return mod;
        }

#ifdef GENERATE_TESTING_DATA
        std::string error_message;
        SocketTransport socket_transport()
        {
            const char * name       = "RDP W2008 TLS Target";
            unique_fd    client_sck = ip_connect(ini.get<cfg::context::target_host>().c_str(), 3389);

            return SocketTransport(
                name,
                std::move(client_sck),
                ini.get<cfg::context::target_host>().c_str(),
                3389,
                std::chrono::seconds(1),
                SocketTransport::Verbose::dump,
                &error_message
            );
        }

        void event_loop(Transport& trans, LicenseApi& license_store)
        {
            auto mod = new_mod_rdp(trans, license_store);

            event_manager.execute_events([&](int /*sck*/)->bool {return true;}, false);

            // TODO: fix that for actual TESTING DATA GENERATION
            unique_server_loop(unique_fd(trans.get_fd()), [&](int /*sck*/)->bool {
                (void)sck;
                LOG(LOG_INFO, "is_up_and_running=%s", mod->is_up_and_running() ? "Yes" : "No");
                if (!already_redirected) {
                    return true;
                }
                return !mod->is_up_and_running();
            });
        }
#else
        void event_loop(TestTransport& trans, LicenseApi& license_store)
        {
            auto mod = new_mod_rdp(trans, license_store);

            trans.disable_remaining_error();
            event_manager.execute_events([&](int /*sck*/)->bool {return true;}, false);

            int n = 0;
            while (!event_manager.is_empty() && (++n < 70)) {
                event_manager.execute_events([&](int /*sck*/)->bool {return true;}, false);
            }
        }
#endif
    };
}

RED_AUTO_TEST_CASE(TestWithoutExistingLicense)
{
    LicenseTestContext ctx;

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
            class CaptureLicenseStore : public LicenseApi
            {
            public:
                // The functions shall return empty bytes_view to indicate the error.
                bytes_view get_license(char const* client_name, uint32_t version, char const* scope, char const* company_name, char const* product_id, writable_bytes_view out, bool enable_log) override
                {
                    (void)client_name;
                    (void)version;
                    (void)scope;
                    (void)company_name;
                    (void)product_id;
                    (void)out;
                    (void)enable_log;

                    return bytes_view { out.data(), 0 };
                }

                bool put_license(char const* client_name, uint32_t version, char const* scope, char const* company_name, char const* product_id, bytes_view in, bool enable_log) override
                {
                    (void)enable_log;

                    LOG(LOG_INFO, "/*CaptureLicenseStore */ const char license_client_name[] =");
                    hexdump_c(client_name, ::strlen(client_name));
                    LOG(LOG_INFO, "/*CaptureLicenseStore */ ;");

                    LOG(LOG_INFO, "/*CaptureLicenseStore */ uint32_t license_version = %u", version);
                    LOG(LOG_INFO, "/*CaptureLicenseStore */ ;");

                    LOG(LOG_INFO, "/*CaptureLicenseStore */ const char license_scope[] =");
                    hexdump_c(scope, ::strlen(scope));
                    LOG(LOG_INFO, "/*CaptureLicenseStore */ ;");

                    LOG(LOG_INFO, "/*CaptureLicenseStore */ const char license_company_name[] =");
                    hexdump_c(company_name, ::strlen(company_name));
                    LOG(LOG_INFO, "/*CaptureLicenseStore */ ;");

                    LOG(LOG_INFO, "/*CaptureLicenseStore */ const char license_product_id[] =");
                    hexdump_c(product_id, ::strlen(product_id));
                    LOG(LOG_INFO, "/*CaptureLicenseStore */ ;");

                    LOG(LOG_INFO, "/*CaptureLicenseStore */ const uint8_t license_data[%zu] = {", in.size());
                    hexdump_d(in);
                    LOG(LOG_INFO, "/*CaptureLicenseStore */ };");

                    return true;
                }
            } license_store;
#else
            class CompareLicenseStore : public LicenseApi
            {
            public:
                CompareLicenseStore(char const* client_name, uint32_t version, char const* scope, char const* company_name, char const* product_id, bytes_view license_data) :
                    expected_client_name(client_name),
                    expected_version(version),
                    expected_scope(scope),
                    expected_company_name(company_name),
                    expected_product_id(product_id),
                    expected_license_data(license_data) {}

                // The functions shall return empty bytes_view to indicate the error.
                bytes_view get_license(char const* client_name, uint32_t version, char const* scope, char const* company_name, char const* product_id, writable_bytes_view out, bool enable_log) override
                {
                    (void)client_name;
                    (void)version;
                    (void)scope;
                    (void)company_name;
                    (void)product_id;
                    (void)out;
                    (void)enable_log;

                    return bytes_view { out.data(), 0 };
                }

                bool put_license(char const* client_name, uint32_t version, char const* scope, char const* company_name, char const* product_id, bytes_view in, bool enable_log) override
                {
                    (void)enable_log;

                    RED_CHECK_EQ(client_name,  this->expected_client_name);
                    RED_CHECK_EQ(version,      this->expected_version);
                    RED_CHECK_EQ(scope,        this->expected_scope);
                    RED_CHECK_EQ(company_name, this->expected_company_name);
                    RED_CHECK_EQ(product_id,   this->expected_product_id);

                    RED_REQUIRE_EQ(in.size(), this->expected_license_data.size());

                    RED_CHECK(in == this->expected_license_data);

                    return true;
                }

            private:
                std::string_view expected_client_name;
                uint32_t         expected_version;
                std::string_view expected_scope;
                std::string_view expected_company_name;
                std::string_view expected_product_id;
                bytes_view       expected_license_data;
            } license_store(license_client_name, license_version, license_scope, license_company_name,
                  license_product_id, bytes_view(license_data, sizeof(license_data)));
#endif

#ifdef GENERATE_TESTING_DATA
            SocketTransport trans = ctx.socket_transport();
#else
            // Comment the code block below to generate testing data.
            TestTransport trans(
                    (already_redirected ? cstr_array_view(indata_woel_2) : cstr_array_view(indata_woel_1)),
                    (already_redirected ? cstr_array_view(outdata_woel_2) : cstr_array_view(outdata_woel_1))
                );
#endif

            ctx.event_loop(trans, license_store);
        }
        catch(Error const & e) {
            if (e.id == ERR_RDP_SERVER_REDIR) {
                LOG(LOG_INFO, "SERVER REDIRECTION");

                ctx.set_new_target();

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
    LicenseTestContext ctx;

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
            class ReplayLicenseStore : public LicenseApi
            {
            public:
                ReplayLicenseStore(char const* client_name, uint32_t version, char const* scope, char const* company_name, char const* product_id, bytes_view license_data) :
                    expected_client_name(client_name),
                    expected_version(version),
                    expected_scope(scope),
                    expected_company_name(company_name),
                    expected_product_id(product_id),
                    expected_license_data(license_data) {}

                // The functions shall return empty bytes_view to indicate the error.
                bytes_view get_license(char const* client_name, uint32_t version, char const* scope, char const* company_name, char const* product_id, writable_bytes_view out, bool enable_log) override
                {
                    (void)enable_log;

                    RED_CHECK_EQ(client_name,  this->expected_client_name);
                    RED_CHECK_EQ(version,      this->expected_version);
                    RED_CHECK_EQ(scope,        this->expected_scope);
                    RED_CHECK_EQ(company_name, this->expected_company_name);
                    RED_CHECK_EQ(product_id,   this->expected_product_id);

                    RED_REQUIRE_GE(out.size(), this->expected_license_data.size());

                    size_t const effective_license_size = std::min(out.size(), this->expected_license_data.size());

                    ::memcpy(out.data(), this->expected_license_data.data(), effective_license_size);

                    return bytes_view { out.data(), effective_license_size };
                }

                bool put_license(char const* client_name, uint32_t version, char const* scope, char const* company_name, char const* product_id, bytes_view in, bool enable_log) override
                {
                    (void)enable_log;
                    (void)in;

                    RED_CHECK_EQ(client_name,  this->expected_client_name);
                    RED_CHECK_EQ(version,      this->expected_version);
                    RED_CHECK_EQ(scope,        this->expected_scope);
                    RED_CHECK_EQ(company_name, this->expected_company_name);
                    RED_CHECK_EQ(product_id,   this->expected_product_id);

                    return true;
                }

            private:
                std::string_view expected_client_name;
                uint32_t         expected_version;
                std::string_view expected_scope;
                std::string_view expected_company_name;
                std::string_view expected_product_id;
                bytes_view       expected_license_data;
            };

            ReplayLicenseStore license_store(
                license_client_name,
                license_version,
                license_scope,
                license_company_name,
                license_product_id,
                make_array_view(license_data)
            );

#ifdef GENERATE_TESTING_DATA
            SocketTransport trans = ctx.socket_transport();
#else
            // Comment the code block below to generate testing data.
            TestTransport trans(
                    (already_redirected ? cstr_array_view(indata_wel_2) : cstr_array_view(indata_wel_1)),
                    (already_redirected ? cstr_array_view(outdata_wel_2) : cstr_array_view(outdata_wel_1))
                );
#endif

            ctx.event_loop(trans, license_store);
        }
        catch(Error const & e) {
            if (e.id == ERR_RDP_SERVER_REDIR) {
                LOG(LOG_INFO, "SERVER REDIRECTION");

                ctx.set_new_target();

                do_work = true;

                already_redirected = true;

                continue;
            }

            throw;
        }
    }
}
