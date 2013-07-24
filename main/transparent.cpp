/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Raphael Zhou

    rdp transparent module main header file
*/

#include <boost/program_options/options_description.hpp>
#include <string>

#include "listen.hpp"
#include "rdp/rdp_transparent.hpp"
#include "session.hpp"
#include "sockettransport.hpp"

int main(int argc, char * argv[]) {
    openlog("transparent", LOG_CONS | LOG_PERROR, LOG_USER);

    const char * copyright_notice =
        "\n"
        "ReDemPtion Transparent Proxy " VERSION ".\n"
        "Copyright (C) Wallix 2010-2013.\n"
        "Christophe Grosjean, Raphael Zhou.\n"
        "\n"
        ;

    std::string input_filename;
    std::string output_filename;
    std::string target_device;
    uint32_t    target_port;
    std::string username;
    std::string password;

    target_port = 3389;

    boost::program_options::options_description desc("Options");
    desc.add_options()
    ("help,h",    "produce help message")
    ("version,v", "show software version")

    ("input-file,i",    boost::program_options::value(&input_filename),  "input ini file name")
    ("output-file,o",   boost::program_options::value(&output_filename), "output int file name")
    ("target-device,t", boost::program_options::value(&target_device),   "target device[:port]")
    ("username,u",      boost::program_options::value(&username),        "username")
    ("password,p",      boost::program_options::value(&password),        "password")
    ;

    boost::program_options::variables_map options;
    boost::program_options::store(
        boost::program_options::command_line_parser(argc, argv).options(desc).run(),
        options
    );
    boost::program_options::notify(options);

    if (options.count("help") > 0) {
        cout << copyright_notice;
        cout << "Usage: redver [options]\n\n";
        cout << desc << endl;
        exit(-1);
    }

    if (options.count("version") > 0) {
        cout << copyright_notice;
        exit(-1);
    }

    if (   (input_filename.c_str()[0] == 0)
        && (output_filename.c_str()[0] == 0)) {
        cout << "Missing input or output ini file name : use -i filename or -o filename\n\n";
        exit(-1);
    }

    if (   (input_filename.c_str()[0] != 0)
        && (output_filename.c_str()[0] != 0)) {
        cout << "use -i filename or -o filename\n\n";
        exit(-1);
    }

    if (target_device.c_str()[0] == 0) {
        cout << "Missing target device : use -t target_device[:port]\n\n";
        exit(-1);
    }

    size_t pos = target_device.find(':');
    if (pos != string::npos) {
        target_port = ::atoi(target_device.substr(pos + 1).c_str());
        target_device.resize(pos);
    }

    if (username.c_str()[0] == 0) {
        cout << "Missing username : use -u username\n\n";
        exit(-1);
    }

    if (password.c_str()[0] == 0) {
        cout << "Missing password : use -u password\n\n";
        exit(-1);
    }


    // This server only support one incoming connection before closing listener
    class ServerOnce : public Server {
    public:
        int  sck;
        char ip_source[256];

        ServerOnce() : sck(0) {
           this->ip_source[0] = 0;
        }

        virtual Server_status start(int incoming_sck) {
            union {
                struct sockaddr s;
                struct sockaddr_storage ss;
                struct sockaddr_in s4;
                struct sockaddr_in6 s6;
            } u;
            unsigned int sin_size = sizeof(u);
            memset(&u, 0, sin_size);
            this->sck = ::accept(incoming_sck, &u.s, &sin_size);
            ::strcpy(this->ip_source, ::inet_ntoa(u.s4.sin_addr));
            LOG(LOG_INFO, "Incoming socket to %d (ip=%s)\n", this->sck, this->ip_source);
            return START_WANT_STOP;
        }
    } one_shot_server;
    Listen listener(one_shot_server, 0, 3389, true, 5);  // 25 seconds to connect, or timeout
    listener.run();

    Inifile ini;
    ConfigurationLoader cfg_loader(ini, CFG_PATH "/" RDPPROXY_INI);
//    ini.debug.front = 511;
    ini.debug.front = 0;
//    ini.debug.mod_rdp = 511;
    ini.debug.mod_rdp = 0;

    int nodelay = 1;
    if (-1 == setsockopt( one_shot_server.sck, IPPROTO_TCP, TCP_NODELAY, (char *)&nodelay
                        , sizeof(nodelay))) {
        LOG(LOG_INFO, "Failed to set socket TCP_NODELAY option on client socket");
    }
    wait_obj front_event(one_shot_server.sck);
    SocketTransport front_trans( "RDP Client", one_shot_server.sck, "0.0.0.0", 0
                               , ini.debug.front, 0);

    LCGRandom gen(0);

    const bool fastpath_support = true;
    const bool mem3blt_support  = false;
    Front front(&front_trans, SHARE_PATH "/" DEFAULT_FONT_NAME, &gen, &ini,
        fastpath_support, mem3blt_support, true, input_filename.c_str());
    null_mod no_mod(front);

    while (front.up_and_running == 0) {
        front.incoming(no_mod);
    }

    LOG(LOG_INFO, "hostname=%s", front.client_info.hostname);

    int client_sck = ip_connect(target_device.c_str(), target_port, 3, 1000, ini.debug.mod_rdp);
    SocketTransport mod_trans( "RDP Server", client_sck, target_device.c_str(), target_port
                             , ini.debug.mod_rdp, &ini.context.auth_error_message);

    try {
        mod_rdp_transparent mod( mod_trans
                               , username.c_str()
                               , password.c_str()
                               , "0.0.0.0"
                               , front
                               , target_device.c_str()
                               , true               // tls
                               , front.client_info
                               , gen
                               , front.keymap.key_flags
                               , ini.globals.auth_channel
                               , ini.globals.alternate_shell.get_cstr()
                               , ini.globals.shell_working_directory.get_cstr()
                               , true   // fast-path
                               , true   // mem3blt
                               , false  // bitmap update
                               , output_filename.c_str()
                               , ini.debug.mod_rdp
                               , true);
        mod.event.obj = client_sck;

        struct      timeval time_mark = { 0, 50000 };
        bool        run_session       = true;
        BackEvent_t mod_event_signal  = BACK_EVENT_NONE;

        while (run_session) {
            try {
                unsigned max = 0;
                fd_set   rfds;
                fd_set   wfds;

                FD_ZERO(&rfds);
                FD_ZERO(&wfds);
                struct timeval timeout = time_mark;

                front_event.add_to_fd_set(rfds, max, timeout);
                mod.event.add_to_fd_set(rfds, max, timeout);

                if (mod.event.is_set(rfds)) {
                    timeout.tv_sec  = 0;
                    timeout.tv_usec = 0;
                }

                int num = select(max + 1, &rfds, &wfds, 0, &timeout);

                if (num < 0) {
                    if (errno == EINTR) {
                        continue;
                    }

                    // Socket error
                    break;
                }

                if (front_event.is_set(rfds)) {
                    try {
                        front.incoming(mod);
                    }
                    catch (...) {
                        run_session = false;
                        continue;
                    };
                }

                if (front.up_and_running) {
                    if (mod.event.is_set(rfds)) {
                        mod.draw_event();
                        if (mod.event.signal != BACK_EVENT_NONE) {
                            mod_event_signal = mod.event.signal;

                            mod.event.reset();
                        }
                        mod.event.reset();

                        if (mod_event_signal == BACK_EVENT_NEXT) {
                            run_session = false;
                        }
                    }
                }
            } catch (Error & e) {
                LOG(LOG_INFO, "Session::Session exception = %d!\n", e.id);
                run_session = false;
            };
        }   // while (run_session)
    }   // try
    catch (Error & e) {
        LOG(LOG_ERR, "errid = %d", e.id);
    }

    front.disconnect();

    shutdown(client_sck, 2);
    close(client_sck);

    shutdown(one_shot_server.sck, 2);
    close(one_shot_server.sck);

    LOG(LOG_INFO, "Listener closed\n");
    LOG(LOG_INFO, "Incoming socket %d (ip=%s)\n", one_shot_server.sck, one_shot_server.ip_source);

    return 0;
}
