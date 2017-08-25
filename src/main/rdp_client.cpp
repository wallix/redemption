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


void run_mod(mod_api & mod, ClientFront & front, wait_obj & front_event, SocketTransport * st_mod, SocketTransport * st_front);

int main(int argc, char** argv)
{
    RedirectionInfo redir_info;
    int verbose = 0;
    std::string target_device = "10.10.47.205";
    int target_port = 3389;
    int nbretry = 3;
    int retry_delai_ms = 1000;

    std::string username = "administrateur";
    std::string password = "SecureLinux";
    ClientInfo client_info;

    client_info.width = 800;
    client_info.height = 600;
    client_info.bpp = 32;

    /* Program options */
    namespace po = program_options;
    po::options_description desc({
        {'h', "help","produce help message"},
        {'t', "target-device", &target_device, "target device"},
        {'u', "username", &username, "username"},
        {'p', "password", &password, "password"},
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


    Inifile ini;
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
                               , to_verbose_flags(verbose));

    if (verbose > 128) {
        mod_rdp_params.log();
    }

    /* SocketTransport mod_trans */
    int client_sck = ip_connect(target_device.c_str(), target_port, nbretry, retry_delai_ms);
    SocketTransport mod_trans( "RDP Server", client_sck, target_device.c_str(), target_port, to_verbose_flags(verbose), nullptr);


    wait_obj front_event;

    /* Random */
    UdevRandom gen;
    TimeSystem timeobj;

    NullAuthentifier authentifier;
    NullReportMessage report_message;

    /* mod_api */
    mod_rdp mod( mod_trans, front, client_info, redir_info, gen, timeobj, mod_rdp_params, authentifier, report_message, ini);

    run_mod(mod, front, front_event, &mod_trans, nullptr);

    return 0;
}



void run_mod(mod_api & mod, ClientFront & front, wait_obj &, SocketTransport * st_mod, SocketTransport *) {
    struct      timeval time_mark = { 0, 50000 };
    bool        run_session       = true;

    while (run_session) {
        try {
            unsigned max = 0;
            fd_set   rfds;
            fd_set   wfds;

            io_fd_zero(rfds);
            io_fd_zero(wfds);
            struct timeval timeout = time_mark;

            mod.get_event().wait_on_fd(st_mod?st_mod->sck:INVALID_SOCKET, rfds, max, timeout);

            if (mod.get_event().is_set(st_mod?st_mod->sck:INVALID_SOCKET, rfds)) {
                timeout.tv_sec  = 2;
                timeout.tv_usec = 0;
            }

            int num = select(max + 1, &rfds, &wfds, nullptr, &timeout);

            LOG(LOG_INFO, "RDP CLIENT :: select num = %d\n", num);

            if (num < 0) {
                if (errno == EINTR) {
                    continue;
                }

                LOG(LOG_INFO, "RDP CLIENT :: errno = %d\n", errno);
                break;
            }

            if (mod.get_event().is_set(st_mod?st_mod->sck:INVALID_SOCKET, rfds)) {
                LOG(LOG_INFO, "RDP CLIENT :: draw_event");
                mod.draw_event(time(nullptr), front);
            }

        } catch (Error & e) {
            LOG(LOG_ERR, "RDP CLIENT :: Exception raised = %u!\n", e.id);
            run_session = false;
        };
    }   // while (run_session)
    return;
}
