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
 * Copyright (C) Wallix 2017
 *
 * free VNC client main program
 *
 */

#include <iostream>
#include <string>

#define LOGPRINT
#include "utils/log.hpp"
//#include "core/front_api.hpp"
#include "front/client_front.hpp"
#include "core/client_info.hpp"
#include "core/session.hpp"
#include "transport/socket_transport.hpp"
#include "utils/invalid_socket.hpp"
#include "mod/mod_api.hpp"
#include "mod/vnc/vnc.hpp"
#include "program_options/program_options.hpp"



namespace po = program_options;
//using namespace std;

/*

    void save_to_png(const char * filename) {
        std::FILE * file = fopen(filename, "w+");
        dump_png24(file, this->gd.data(), this->gd.width(),
                   this->gd.height(), this->gd.rowsize(), true);
        fclose(file);
    }


*/


void run_mod(mod_api & mod, ClientFront & front, SocketTransport * st_mod);

int main(int argc, char** argv)
{
    int verbose = 16;
    std::string target_device = "10.10.46.70";
    int target_port = 5900;
    int nbretry = 3;
    int retry_delai_ms = 1000;

    //std::string username = "user2003";
    //std::string password = "SecureLinux$42";
    ClientInfo client_info;

    client_info.width = 800;
    client_info.height = 600;
    client_info.bpp = 32;
    client_info.keylayout = 0x04C;
    /* Program options */
    po::options_description desc({
        {'h', "help","produce help message"},
        {'t', "target-device", &target_device, "target device"},
        //{'u', "username", &username, "username"},
        //{'p', "password", &password, "password"},
        {"verbose", &verbose, "verbose"},
    });

    auto options = po::parse_command_line(argc, argv, desc);

    if (options.count("help") > 0) {
        std::cout <<
            "\n"
            "Stand alone VNC Client.\n"
            "Copyright (C) Wallix 2010-2017.\n"
            "\n"
            "Usage: vnc_client [options]\n\n"
            << desc << std::endl
        ;
        return 0;
    }

    ClientInfo info;
    info.keylayout = 0x04C;
    info.console_session = 0;
    info.brush_cache_code = 0;
    info.bpp = 24;
    info.width = 800;
    info.height = 600;
    info.build = 420;

    int vnc_sck = ip_connect(target_device.c_str(), target_port, nbretry, retry_delai_ms);
    SocketTransport sock_trans( "VNC Target", vnc_sck, target_device.c_str(), target_port, to_verbose_flags(verbose), nullptr);
    // sock_trans.connect();

    SSL_library_init();
    ClientFront front(info, verbose);
    //VncFront front(sock_trans, gen, ini, cctx, authentifier, fastpath_support, mem3blt_support, now, input_filename.c_str(), nullptr);

    const bool is_socket_transport = true;
    const VncBogusClipboardInfiniteLoop bogus_clipboard_infinite_loop {};
    Font font;
    NullReportMessage report_message;

    /* mod_api */
    mod_vnc mod(
        sock_trans
      , "10.10.46.70"
      , "SecureLinux$42"
      , front
      , client_info.width
      , client_info.height
      , font
      , "label message", "label pass"
      , Theme()
      , client_info.keylayout
      , 0             /* key_flags */
      , true          /* clipboard */
      , true          /* clipboard */
      , "0,1,-239"    /* encodings: Raw,CopyRect,Cursor pseudo-encoding */
      , false         /* allow authentification retries */
      , is_socket_transport
      , mod_vnc::ClipboardEncodingType::UTF8
      , bogus_clipboard_infinite_loop
      , report_message
      , false
      , verbose);
//    mod.get_event().set();
    mod.get_event().set_trigger_time(wait_obj::NOW);

    run_mod(mod, front, &sock_trans);

    return 0;
}



inline void run_mod(mod_api &mod, ClientFront &front, SocketTransport *st_mod) {
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

            mod.get_event().wait_on_fd(st_mod->sck, rfds, max, timeout);

            int num = select(max + 1, &rfds, &wfds, nullptr, &timeout);

            LOG(LOG_INFO, "VNC CLIENT :: select num = %d\n", num);

            if (num < 0) {
                if (errno == EINTR) {
                    continue;
                }

                LOG(LOG_INFO, "VNC CLIENT :: errno = %d\n", errno);
                break;
            }

            if (mod.get_event().is_set(st_mod->sck, rfds)) {
//                mod.get_event().reset();
                mod.get_event().reset_trigger_time();
                mod.draw_event(time(nullptr), front);

            }
        } catch (Error & e) {
            LOG(LOG_ERR, "VNC CLIENT :: Exception raised = %d!\n", e.id);
            run_session = false;
        };
    }   // while (run_session)
    LOG(LOG_INFO, "VNC CLIENT :: << run_mod");
    return;
}
