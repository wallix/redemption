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

#include <iostream>
#include <string>
#include <openssl/ssl.h>

#include "main/version.hpp"

#include "configs/config.hpp"
#include "front/client_front.hpp"

#include "core/client_info.hpp"
#include "transport/socket_transport.hpp"
#include "core/wait_obj.hpp"
#include "mod/mod_api.hpp"
#include "mod/rdp/rdp_params.hpp"
#include "mod/rdp/rdp.hpp"
#include "utils/redirection_info.hpp"
#include "utils/netutils.hpp"
#include "utils/genrandom.hpp"
#include "program_options/program_options.hpp"


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

    std::string username;
    std::string password;
    ClientInfo client_info;

    client_info.width = 800;
    client_info.height = 600;
    client_info.bpp = 32;

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
        {"verbose", &verbose, "verbose"},
    });

    auto options = po::parse_command_line(argc, argv, desc);

    if (options.count("help") > 0) {
        std::cout <<
            "\n"
            "ReDemPtion Stand alone RDP Client.\n"
            "Copyright (C) Wallix 2010-2015.\n"
            "\n"
            "Usage: rdpproxy [options]\n\n"
            << desc << std::endl
        ;
        return 0;
    }

    if (options.count("version") > 0) {
        std::cout << VERSION << std::endl;
        return 0;
    }

    openlog("rdpclient", LOG_CONS | LOG_PERROR, LOG_USER);

    Inifile ini;

    ini.set<cfg::mod_rdp::persistent_disk_bitmap_cache>(false);
    ini.set<cfg::mod_rdp::persist_bitmap_cache_on_disk>(false);
    ini.set<cfg::client::persist_bitmap_cache_on_disk>(false);
    ini.set<cfg::client::persistent_disk_bitmap_cache>(false);

    SSL_library_init();
    ClientFront front(client_info, verbose > 10);
    ModRDPParams mod_rdp_params( username.c_str()
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

    /* SocketTransport mod_trans */
    int client_sck = ip_connect(target_device.c_str(), target_port, nbretry, retry_delai_ms);
    SocketTransport mod_trans( "RDP Server", client_sck, target_device.c_str(), target_port, std::chrono::seconds(1), to_verbose_flags(verbose), nullptr);

    /* Random */
    UdevRandom gen;
    TimeSystem timeobj;

    NullAuthentifier authentifier;
    NullReportMessage report_message;

    /* mod_api */
    mod_rdp mod( mod_trans, front, client_info, redir_info, gen, timeobj, mod_rdp_params, authentifier, report_message, ini);

    using Ms = std::chrono::milliseconds;
    return run_test_client(
        "RDP", mod_trans.sck, mod, front,
        Ms(inactivity_time_ms), Ms(max_time_ms), screen_output);
}
