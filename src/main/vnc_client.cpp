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

#include "main/version.hpp"

#include "utils/log.hpp"
#include "front/client_front.hpp"
#include "core/client_info.hpp"
#include "core/session.hpp"
#include "transport/socket_transport.hpp"
#include "mod/mod_api.hpp"
#include "mod/vnc/vnc.hpp"
#include "program_options/program_options.hpp"


int main(int argc, char** argv)
{
    uint64_t verbose = 16;
    std::string target_device;
    int target_port = 5900;
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
    client_info.keylayout = 0x04C;

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
            "Stand alone VNC Client.\n"
            "Copyright (C) Wallix 2010-2017.\n"
            "\n"
            "Usage: vnc_client [options]\n\n"
            << desc << std::endl
        ;
        return 0;
    }

    if (options.count("version") > 0) {
        std::cout << VERSION << std::endl;
        return 0;
    }

    openlog("vncclient", LOG_CONS | LOG_PERROR, LOG_USER);

    ClientInfo info;
    info.keylayout = 0x04C;
    info.console_session = 0;
    info.brush_cache_code = 0;
    info.bpp = 24;
    info.width = 800;
    info.height = 600;
    info.build = 420;

    int vnc_sck = ip_connect(target_device.c_str(), target_port, nbretry, retry_delai_ms);
    SocketTransport mod_trans( "VNC Target", vnc_sck, target_device.c_str(), target_port, std::chrono::seconds(1), to_verbose_flags(verbose), nullptr);
    // mod_trans.connect();

    SSL_library_init();
    ClientFront front(info, verbose);
    //VncFront front(mod_trans, gen, ini, cctx, authentifier, fastpath_support, mem3blt_support, now, input_filename.c_str(), nullptr);

    const bool is_socket_transport = true;
    const VncBogusClipboardInfiniteLoop bogus_clipboard_infinite_loop {};
    Font font;
    NullReportMessage report_message;

    /* mod_api */
    mod_vnc mod(
        mod_trans
      , username.c_str()
      , password.c_str()
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
      , nullptr
      , to_verbose_flags(verbose));
    mod.get_event().set_trigger_time(wait_obj::NOW);

    using Ms = std::chrono::milliseconds;
    return run_test_client(
        "VNC", mod_trans.sck, mod, front,
        Ms(inactivity_time_ms), Ms(max_time_ms), screen_output);
}
