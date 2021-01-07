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

#include "acl/license_api.hpp"
#include "configs/config.hpp"
#include "core/client_info.hpp"
#include "core/channels_authorizations.hpp"
#include "client_redemption/client_front.hpp"
#include "mod/rdp/new_mod_rdp.hpp"
#include "mod/rdp/rdp_params.hpp"
#include "mod/rdp/mod_rdp_factory.hpp"
#include "mod/vnc/new_mod_vnc.hpp"
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
#include "utils/cli.hpp"
#include "utils/cli_chrono.hpp"
#include "system/scoped_ssl_init.hpp"
#include "core/events.hpp"
#include "gdi/osd_api.hpp"

#include <iostream>
#include <string>
#include <optional>


int main(int argc, char** argv)
{
    uint64_t verbose = 0;
    std::string target_device;
    int port = 0;

    std::chrono::milliseconds inactivity_time_ms {1000};
    std::chrono::milliseconds max_time_ms {inactivity_time_ms * 5};
    std::string screen_output;
    std::string record_output;

    std::string username;
    std::string password;

    std::string load_balance_info;
    std::string ini_file;

    int cert_check = 0;
    bool is_vnc = false;
    bool use_LCGRandom = false;

    auto positive_duration_location = [](std::chrono::milliseconds& ms){
        return cli::arg_location(ms, [](std::chrono::milliseconds& ms){
            return ms < std::chrono::milliseconds::zero() ? cli::Res::BadFormat : cli::Res::Ok;
        });
    };

    auto const options = cli::options(
        cli::option('h', "help").help("produce help message")
            .parser(cli::help()),

        cli::option('v', "version").help("show software version")
            .parser(cli::quit([]{
                std::cout << redemption_info_version() << std::endl;
            })),

        cli::option('t', "target-device")
            .parser(cli::arg_location(target_device)).argname("<address>"),

        cli::option('u', "username")
            .parser(cli::arg_location(username)).argname("<username>"),

        cli::option('p', "password")
            .parser(cli::password_location(argv, password)),

        cli::option('P', "port")
            .parser(cli::arg_location(port)).argname("<port>"),

        cli::option('a', "inactivity-time").help("milliseconds inactivity before sreenshot")
            .parser(positive_duration_location(inactivity_time_ms)),

        cli::option('m', "max-time").help("maximum milliseconds before sreenshot")
            .parser(positive_duration_location(max_time_ms)),

        cli::option('s', "screen-output").help("png screenshot path")
            .parser(cli::arg_location(screen_output)).argname("<path>"),

        cli::option('r', "record-path").help("dump socket path")
            .parser(cli::arg_location(record_output)).argname("<path>"),

        cli::option('V', "vnc").help("enable vnc instead of rdp")
            .parser(cli::on_off_location(is_vnc)),

        cli::option('l', "lcg").help("use LCGRandom")
            .parser(cli::on_off_location(use_LCGRandom)),

        cli::option('b', "load-balance-info")
            .parser(cli::arg_location(load_balance_info)).argname("<data>"),

        cli::option('n', "ini").help("load ini filename")
            .parser(cli::arg_location(ini_file)).argname("<path>"),

        cli::option('c', "cert-check").help("\n"
            "    0 = fails if certificates doesn't match or miss.\n"
            "    1 = fails if certificate doesn't match, succeed if no known certificate\n"
            "    2 = succeed if certificates exists (not checked), fails if missing.\n"
            "    3 = always succeed.")
            .parser(cli::arg_location(cert_check)).argname("<number>"),

        cli::option("verbose")
            .parser(cli::arg_location(verbose)).argname("<verbosity>")
    );

    auto const cli_result = cli::parse(options, argc, argv);
    switch (cli_result.res) {
        case cli::Res::Ok:
            break;
        case cli::Res::Exit:
            return 0;
        case cli::Res::Help:
            std::cout << "Usage: rdpclient [options]\n\n";
            cli::print_help(options, std::cout);
            return 0;
        case cli::Res::BadFormat:
        case cli::Res::BadOption:
        case cli::Res::NotOption:
        case cli::Res::StopParsing:
            std::cerr << "Bad " << (cli_result.res == cli::Res::BadFormat ? "format" : "option") << " at parameter " << cli_result.opti;
            if (cli_result.opti < cli_result.argc) {
                std::cerr << " (" << cli_result.argv[cli_result.opti] << ")";
            }
            std::cerr << "\n";
            return 1;
    }

    int const target_port = port ? port : (is_vnc ? 5900 : 3389);

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
    client_info.build = 420;
    if (is_vnc) {
        client_info.keylayout = 0x04C;
        client_info.console_session = false;
        client_info.brush_cache_code = 0;
    }

    SocketTransport mod_trans(
        is_vnc ? "VNC Server" : "RDP Server", std::move(sck), target_device.c_str(),
        target_port, std::chrono::seconds(1),
        safe_cast<SocketTransport::Verbose>(uint32_t(verbose >> 32)), nullptr);

    ScopedSslInit scoped_ssl;

    ClientFront front(client_info.screen_info, verbose & 0x8000'0000);
    EventContainer events;
    events.set_current_time(tvtime());

    auto run = [&](auto create_mod){
        std::optional<RecorderTransport> recorder_trans;
        Transport* trans = &mod_trans;
        if (!record_output.empty()) {
            RecorderTransport& recorder = recorder_trans.emplace(
                mod_trans, events.time_base, record_output.c_str());
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
        return run_test_client(is_vnc ? "VNC" : "RDP", events, *mod,
            inactivity_time_ms, max_time_ms, screen_output);
    };

    Inifile ini;
    if (!ini_file.empty()) {
        configuration_load(ini.configuration_holder(), ini_file.c_str());
    }

    NullSessionLog session_log;

    UdevRandom system_gen;
    FixedRandom lcg_gen;
    NullLicenseStore licensestore;
    RedirectionInfo redir_info;

    if (is_vnc) {
        return run([&](Transport& trans){
            return new_mod_vnc(
                trans
              , gdi::null_gd()
              , events
              , session_log
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

    bool const use_system_obj = record_output.empty() && !use_LCGRandom;

    const ChannelsAuthorizations channels_authorizations("*", std::string{});
    gdi::NullOsd osd;

    auto run_rdp = [&]{
        ModRdpFactory mod_rdp_factory;
        return run([&](Transport& trans){
            using RandomRef = Random&;
            return new_mod_rdp(
                trans,
                gdi::null_gd(),
                osd,
                events,
                session_log,
                front, client_info, redir_info,
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
        session_log.report("SERVER_REDIRECTION", message.c_str());
    }

    return run_rdp() ? 2 : 0;
}
