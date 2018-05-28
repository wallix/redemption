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
 *
 * Product name: redemption, a FLOSS RDP proxy
 * Copyright (C) Wallix 2015
 *
 * free RDP client main program
 *
 */

#include "main/version.hpp"

#include "configs/config.hpp"
#include "core/client_info.hpp"
#include "front/client_front.hpp"
#include "mod/mod_api.hpp"
#include "mod/rdp/rdp.hpp"
#include "mod/rdp/rdp_params.hpp"
#include "mod/vnc/vnc.hpp"
#include "program_options/program_options.hpp"
#include "transport/recorder_transport.hpp"
#include "transport/socket_transport.hpp"
#include "utils/genrandom.hpp"
#include "utils/netutils.hpp"
#include "utils/redirection_info.hpp"
#include "test_only/fixed_random.hpp"

#include <iostream>
#include <string>
#include <optional>

#include <openssl/ssl.h>


int main(int argc, char** argv)
{
    RedirectionInfo redir_info;
    uint64_t verbose = 0;
    std::string target_device;
    int target_port = 3389;
    int nbretry = 3;
    int retry_delai_ms = 1000;

    unsigned inactivity_time_ms = 1000u;
    unsigned max_time_ms = 5u * inactivity_time_ms;
    std::string screen_output;
    std::string record_output;

    std::string username;
    std::string password;

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
        {'l', "lcg", "use LCGRandom and LCGTime"},
        {"verbose", &verbose, "verbose"},
    });

    auto options = po::parse_command_line(argc, argv, desc);

    if (options.count("help") > 0) {
        std::cout <<
            "\n"
            "ReDemPtion stand alone RDP Client " << VERSION << ".\n"
            "Copyright (C) Wallix 2010-2018.\n"
            "\n"
            "Usage: " << argv[0] << " [options]\n\n"
            << desc << std::endl
        ;
        return 0;
    }

    if (options.count("version") > 0) {
        std::cout << VERSION << std::endl;
        return 0;
    }

    bool const is_vnc = options.count("vnc");

    if (is_vnc && !options.count("port")) {
        target_port = 5900;
    }

    openlog("rdpclient", LOG_CONS | LOG_PERROR, LOG_USER);

    /* SocketTransport mod_trans */
    auto sck = ip_connect(target_device.c_str(), target_port, nbretry, retry_delai_ms);
    if (!sck.is_open()) {
        return 1;
    }

    ClientInfo client_info;
    client_info.width = 800;
    client_info.height = 600;
    client_info.bpp = 24;
    if (is_vnc) {
        client_info.keylayout = 0x04C;
        client_info.console_session = 0;
        client_info.brush_cache_code = 0;
        client_info.build = 420;
    }

    SocketTransport mod_trans(
        is_vnc ? "VNC Server" : "RDP Server", std::move(sck), target_device.c_str(),
        target_port, std::chrono::seconds(1), to_verbose_flags(verbose), nullptr);

    SSL_library_init();

    ClientFront front(client_info, verbose);
    NullReportMessage report_message;
    SessionReactor session_reactor;

    auto run = [&](auto create_mod){
        std::optional<RecorderTransport> recorder_trans;
        Transport* trans = &mod_trans;
        if (!record_output.empty()) {
            trans = &recorder_trans.emplace(mod_trans, record_output.c_str());
        }
        auto mod = create_mod(*trans);
        using Ms = std::chrono::milliseconds;
        return run_test_client(
            is_vnc ? "VNC" : "RDP", session_reactor, mod, front,
            Ms(inactivity_time_ms), Ms(max_time_ms), screen_output);
    };

    if (is_vnc) {
        return run([&](Transport& trans){
            return mod_vnc(
                trans
              , session_reactor
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
              , mod_vnc::ClipboardEncodingType::UTF8
              , VncBogusClipboardInfiniteLoop::delayed
              , report_message
              , false
              , nullptr
              , to_verbose_flags(verbose) | VNCVerbose::connection | VNCVerbose::basic_trace);
        });
    }
    else {
        Inifile ini;

        ModRDPParams mod_rdp_params(
            username.c_str()
          , password.c_str()
          , target_device.c_str()
          , "0.0.0.0"   // client ip is silenced
          , /*front.keymap.key_flags*/ 0
          , ini.get<cfg::font>()
          , ini.get<cfg::theme>()
          , ini.get_ref<cfg::context::server_auto_reconnect_packet>()
          , ini.get_ref<cfg::context::close_box_extra_message>()
          , to_verbose_flags(verbose));

        if (verbose > 128) {
            mod_rdp_params.log();
        }

        UdevRandom system_gen;
        TimeSystem system_timeobj;
        FixedRandom lcg_gen;
        LCGTime lcg_timeobj;
        NullAuthentifier authentifier;

        bool const use_system_obj = record_output.empty() && !options.count("lcg");

        return run([&](Transport& trans){
            using TimeObjRef = TimeObj&;
            using RandomRef = Random&;
            return mod_rdp(
                trans, session_reactor, front, client_info, redir_info,
                use_system_obj ? RandomRef(system_gen) : lcg_gen,
                use_system_obj ? TimeObjRef(system_timeobj) : lcg_timeobj,
                mod_rdp_params, authentifier, report_message, ini);
        });
    }
}
