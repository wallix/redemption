/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *""
 * Product name: redemption, a FLOSS RDP proxy
 * Copyright (C) Wallix 2015
 *
 * free RDP client main program
 *
 */

#include "acl/auth_api.hpp"
#include "acl/license_api.hpp"
#include "configs/config.hpp"
#include "core/client_info.hpp"
#include "core/channels_authorizations.hpp"
#include "client_redemption/client_front.hpp"
#include "mod/rdp/new_mod_rdp.hpp"
#include "mod/rdp/rdp_params.hpp"
#include "mod/rdp/mod_rdp_factory.hpp"
#include "mod/vnc/new_mod_vnc.hpp"
#include "program_options/program_options.hpp"
#include "transport/recorder_transport.hpp"
#include "transport/socket_transport.hpp"
#include "utils/cfgloader.hpp"
#include "utils/fileutils.hpp"
#include "utils/fixed_random.hpp"
#include "utils/genrandom.hpp"
#include "utils/netutils.hpp"
#include "utils/redemption_info_version.hpp"
#include "utils/redirection_info.hpp"
#include "utils/theme.hpp"
#include "acl/sesman.hpp"
#include "acl/gd_provider.hpp"
#include "system/scoped_ssl_init.hpp"
#include "core/events.hpp"

#include <iostream>
#include <string>
#include <optional>

#include <openssl/ssl.h>


int main(int argc, char** argv)
{
    uint64_t verbose = 0;
    std::string target_device;
    int target_port = 3389;

    unsigned inactivity_time_ms = 1000u;
    unsigned max_time_ms = 5u * inactivity_time_ms;
    std::string screen_output;
    std::string record_output;

    std::string username;
    std::string password;

    std::string load_balance_info;
    std::string ini_file;

    int cert_check;

    /* Program options */
    namespace po = program_options;
    po::options_description desc({
        {'v', "version",""},
        {'h', "help","produce help message"},
        {'t', "target-device", &target_device, "target device"},
        {'u', "username", &username, "username"},
        {'p', "password", &password, "password"},
        {'P', "port", &target_port, "port"},
        {'a', "inactivity-time", &inactivity_time_ms, "milliseconds inactivity before sreenshot"},
        {'m', "max-time", &max_time_ms, "maximum milliseconds before sreenshot"},
        {'s', "screen-output", &screen_output, "png screenshot path"},
        {'r', "record-path", &record_output, "dump socket path"},
        {'V', "vnc", "dump socket path"},
        {'l', "lcg", "use LCGRandom"},
        {'b', "load-balance-info", &load_balance_info, ""},
        {'n', "ini", &ini_file, "load ini filename"},
        {'c', "cert-check", &cert_check,
            "0 = fails if certificates doesn't match or miss.\n"
            "1 = fails if certificate doesn't match, succeed if no known certificate\n"
            "2 = succeed if certificates exists (not checked), fails if missing.\n"
            "3 = always succeed.\n"
        },
        {"verbose", &verbose, "verbose"},
    });

    auto options = po::parse_command_line(argc, argv, desc);

    if (options.count("help") > 0) {
        std::cout <<
            "\n"
            "ReDemPtion stand alone RDP Client. " << redemption_info_version() << ".\n"
            "Copyright (C) Wallix 2010-2018.\n"
            "\n"
            "Usage: " << argv[0] << " [options]\n\n"
            << desc << std::endl
        ;
        return 0;
    }

    if (options.count("version") > 0) {
        std::cout << redemption_info_version() << std::endl;
        return 0;
    }

    bool const is_vnc = options.count("vnc");

    if (is_vnc && !options.count("port")) {
        target_port = 5900;
    }

    openlog("rdpclient", LOG_CONS | LOG_PERROR, LOG_USER);

    /* SocketTransport mod_trans */
    auto sck = ip_connect(target_device.c_str(), target_port);
    if (!sck.is_open()) {
        return 1;
    }

    ClientInfo client_info;
    client_info.screen_info.width = 800;
    client_info.screen_info.height = 600;
    client_info.screen_info.bpp = BitsPerPixel{24};
    if (is_vnc) {
        client_info.keylayout = 0x04C;
        client_info.console_session = false;
        client_info.brush_cache_code = 0;
        client_info.build = 420;
    }

    SocketTransport mod_trans(
        is_vnc ? "VNC Server" : "RDP Server", std::move(sck), target_device.c_str(),
        target_port, std::chrono::seconds(1),
        safe_cast<SocketTransport::Verbose>(uint32_t(verbose >> 32)), nullptr);

    ScopedSslInit scoped_ssl;

    ClientFront front(client_info.screen_info, verbose & 0x8000'0000);
    TimeBase time_base(tvtime());
    EventContainer events;

    auto run = [&](auto create_mod){
        std::optional<RecorderTransport> recorder_trans;
        Transport* trans = &mod_trans;
        if (!record_output.empty()) {
            RecorderTransport& recorder = recorder_trans.emplace(
                mod_trans, time_base, record_output.c_str());
            if (ini_file.empty()) {
                recorder.add_info({});
            }
            else {
                std::string contents;
                auto error = append_file_contents(ini_file, contents);
                if (bool(error)) {
                    int errnum = errno;
                    LOG(LOG_ERR, "failed to read %s: %s", ini_file, strerror(errnum));
                    throw Error(ERR_TRANSPORT_READ_FAILED, errnum);
                }
                recorder.add_info(contents);
            }
            for (auto cstr : make_array_view(argv+1, argv+argc)) {
                recorder.add_info({cstr, strlen(cstr)});
            }
            trans = &recorder;
        }
        auto mod = create_mod(*trans);
        using Ms = std::chrono::milliseconds;
        return run_test_client(is_vnc ? "VNC" : "RDP", events, *mod,
            Ms(inactivity_time_ms), Ms(max_time_ms), screen_output);
    };

    Inifile ini;
    if (!ini_file.empty()) {
        configuration_load(ini.configuration_holder(), ini_file.c_str());
    }

    Sesman sesman(ini, time_base);

    UdevRandom system_gen;
    FixedRandom lcg_gen;
    NullLicenseStore licensestore;
    RedirectionInfo redir_info;
    GdForwarder gd_forwarder(gdi::null_gd());

    if (is_vnc) {
        return run([&](Transport& trans){
            return new_mod_vnc(
                trans
              , time_base
              , gd_forwarder
              , events
              , sesman
              , username.c_str()
              , password.c_str()
              , front
              , 800
              , 600
              , 0x04C         /* keylayout */
              , 0             /* key_flags */
              , true          /* clipboard */
              , true          /* clipboard */
              , "16, 2, 0, 1,-239"    /* encodings: Raw,CopyRect,Cursor pseudo-encoding */
              , false
              , false          /*remove_server_alt_state_for_char*/
              , true           /* support Cursor Pseudo-Encoding */
              , nullptr
              , safe_cast<VNCVerbose>(uint32_t(verbose))
              | VNCVerbose::connection
              | VNCVerbose::basic_trace
              , nullptr);
        }) ? 1 : 0;
    }

    ini.set<cfg::mod_rdp::server_redirection_support>(true);
    std::array<unsigned char, 28> server_auto_reconnect_packet;
    std::string close_box_extra_message;
    Theme theme;
    Font font;

    ModRDPParams mod_rdp_params(
        username.c_str()
        , password.c_str()
        , target_device.c_str()
        , "0.0.0.0"   // client ip is silenced
        , /*front.keymap.key_flags*/ 0
        , font
        , theme
        , server_auto_reconnect_packet
        , close_box_extra_message
        , RDPVerbose(verbose));

    mod_rdp_params.device_id                  = "device_id";
    mod_rdp_params.enable_tls                 = true;
    mod_rdp_params.enable_nla                 = true;
    mod_rdp_params.enable_fastpath            = true;
    mod_rdp_params.enable_new_pointer         = true;
    mod_rdp_params.enable_glyph_cache         = true;
    mod_rdp_params.file_system_params.enable_rdpdr_data_analysis = false;
    mod_rdp_params.load_balance_info          = load_balance_info.c_str();
    mod_rdp_params.server_cert_check          = static_cast<ServerCertCheck>(cert_check);

    TLSClientParams tls_client_params;

    if (verbose > 128) {
        mod_rdp_params.log();
    }

    bool const use_system_obj = record_output.empty() && !options.count("lcg");

    const ChannelsAuthorizations channels_authorizations("*", std::string{});

    auto run_rdp = [&]{
        ModRdpFactory mod_rdp_factory;
        return run([&](Transport& trans){
            using RandomRef = Random&;
            return new_mod_rdp(
                trans,
                time_base,
                gd_forwarder,
                events,
                sesman,
                gdi::null_gd(), front, client_info, redir_info,
                use_system_obj ? RandomRef(system_gen) : lcg_gen,
                channels_authorizations, mod_rdp_params, tls_client_params, licensestore,
                ini, nullptr, nullptr, mod_rdp_factory);
        });
    };

    int eid = run_rdp();

    if (ERR_RDP_SERVER_REDIR != eid) {
        return eid  ? 1 : 0;
    }

    {
        // SET new target in ini
        const char * host = char_ptr_cast(redir_info.host);
        const char * password = char_ptr_cast(redir_info.password);
        const char * username = char_ptr_cast(redir_info.username);
        const char * change_user = "";
        if (redir_info.dont_store_username && username[0] != 0) {
            LOG(LOG_INFO, "SrvRedir: Change target username to '%s'", username);
            ini.set_acl<cfg::globals::target_user>(username);
            change_user = username;
        }
        if (password[0] != 0) {
            LOG(LOG_INFO, "SrvRedir: Change target password");
            ini.set_acl<cfg::context::target_password>(password);
        }
        LOG(LOG_INFO, "SrvRedir: Change target host to '%s'", host);
        ini.set_acl<cfg::context::target_host>(host);
        auto message = str_concat(change_user, '@', host);
        sesman.report("SERVER_REDIRECTION", message.c_str());
    }

    return run_rdp() ? 2 : 0;
}
