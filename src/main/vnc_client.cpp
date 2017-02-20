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
#include "core/vnc_front.hpp"
#include "core/client_info.hpp"
#include "core/session.hpp"
#include "transport/socket_transport.hpp"
#include "utils/invalid_socket.hpp"
#include "core/RDP/RDPDrawable.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryBrushCache.hpp"
#include "core/wait_obj.hpp"
#include "mod/mod_api.hpp"
#include "utils/redirection_info.hpp"
#include "core/channel_list.hpp"
#include "utils/genrandom.hpp"
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


void run_mod(mod_api & mod, VncFront & front, SocketTransport * st_mod);

int main(int argc, char** argv)
{
    RedirectionInfo redir_info;
    int verbose = 16;
    std::string target_device = "10.10.46.70";
    int target_port = 5900;
    int nbretry = 3;
    int retry_delai_ms = 1000;

    std::string username = "user2003";
    std::string password = "SecureLinux$42";
    ClientInfo client_info;

    std::string input_filename;


    client_info.width = 800;
    client_info.height = 600;
    client_info.bpp = 32;
    client_info.keylayout = 0x04C;
    /* Program options */
    po::options_description desc({
        {'h', "help","produce help message"},
        {'i', "input-file",    &input_filename,               "input ini file name"},
        {'t', "target-device", &target_device, "target device"},
        {'u', "username", &username, "username"},
        {'p', "password", &password, "password"},
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

    Inifile ini;
    CryptoContext cctx;
    UdevRandom gen;

    NullAuthentifier authentifier;


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

    VncFront front(false, false, info, verbose);

 //   VncFront front(sock_trans, gen, ini, cctx, authentifier, fastpath_support, mem3blt_support, now, input_filename.c_str(), nullptr);



    const bool is_socket_transport = true;
    const VncBogusClipboardInfiniteLoop bogus_clipboard_infinite_loop {};
    Font font;

    /* Random */
    TimeSystem timeobj;

    /* mod_api */
 //   mod_rdp mod( mod_trans, front, client_info, redir_info, gen, timeobj, mod_rdp_params);

     mod_vnc mod(
        sock_trans
      , "10.10.46.70"
      , "SecureLinux$42"
      , front
      , client_info.width
      , client_info.height
      , font
      , Translator(Translation::EN)
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
      , authentifier       // acl
      , verbose);
    mod.get_event().set();

    run_mod(mod, front, &sock_trans);

    return 0;
}


void run_mod(mod_api &mod, VncFront &front, SocketTransport *st_mod) {
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

//            if (mod.get_event().is_set(st_mod?st_mod->sck:INVALID_SOCKET, rfds)) {
//                timeout = time_mark;
//            }

            int num = select(max + 1, &rfds, &wfds, nullptr, &timeout);


            if (num < 0) {
                if (errno == EINTR) {
                    continue;
                }

                LOG(LOG_INFO, "VNC CLIENT :: errno = %d\n", errno);
                break;
            }

            if (num > 0){ // incoming data from network socket
                LOG(LOG_INFO, "VNC CLIENT :: select num = %d\n", num);
            }

            if (mod.get_event().is_set(st_mod?st_mod->sck:INVALID_SOCKET, rfds)) {
                mod.get_event().reset();
                mod.rdp_input_up_and_running(); // really we only need this once
                LOG(LOG_INFO, "VNC CLIENT :: draw_event");
                mod.draw_event(time(nullptr), front);
            }



/*

            if (front_event.is_set(st_front?st_front->sck:INVALID_SOCKET, rfds)) {
                time_t now = time(nullptr);

                try {
                    front.incoming(mod, now);
                }
                catch (...) {
                    run_session = false;
                    continue;
                };
            }


            if (front.up_and_running) {
                if (mod.get_event().is_set(st_mod?st_mod->sck:INVALID_SOCKET, rfds)) {
                    mod.get_event().reset();
                    mod.draw_event(time(nullptr), front);
                    if (mod.get_event().signal != BACK_EVENT_NONE) {
                        mod_event_signal = mod.get_event().signal;
                    }

                    if (mod_event_signal == BACK_EVENT_NEXT) {
                        run_session = false;
                    }
                }
            }
            else {
            }

*/






        } catch (Error & e) {
            LOG(LOG_ERR, "VNC CLIENT :: Exception raised = %d!\n", e.id);
            run_session = false;
        };
    }   // while (run_session)
    return;
}
