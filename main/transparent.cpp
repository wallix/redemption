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

#define LOGPRINT

#include "listen.hpp"
#include "rdp/rdp_transparent.hpp"
#include "session.hpp"
#include "sockettransport.hpp"

int main(int argc, char * argv[]) {
    // This server only support one incoming connection before closing listener
    class ServerOnce : public Server {
    public:
        int  sck;
        char ip_source[256];

        ServerOnce() : sck(0) {
           this->ip_source[0] = 0;
        }

        virtual Server_status start(int incoming_sck) {
            struct sockaddr_in sin;
            unsigned int sin_size = sizeof(struct sockaddr_in);
            ::memset(&sin, 0, sin_size);
            this->sck = ::accept(incoming_sck, (struct sockaddr *)&sin, &sin_size);
            ::strcpy(this->ip_source, ::inet_ntoa(sin.sin_addr));
            LOG(LOG_INFO, "Incoming socket to %d (ip=%s)\n", this->sck, this->ip_source);
            return START_WANT_STOP;
        }
    } one_shot_server;
    Listen listener(one_shot_server, 0, 3389, true, 5);  // 25 seconds to connect, or timeout
    listener.run();

    Inifile ini;
//    ini.debug.front = 511;
    ini.debug.front = 0;
//    ini.debug.mod_rdp = 511;
    ini.debug.mod_rdp = 0;
//    int verbose = 511;
    int verbose = 0;

    int nodelay = 1;
    if (-1 == setsockopt( one_shot_server.sck, IPPROTO_TCP, TCP_NODELAY, (char *)&nodelay
                        , sizeof(nodelay))) {
        LOG(LOG_INFO, "Failed to set socket TCP_NODELAY option on client socket");
    }
    wait_obj front_event(one_shot_server.sck);
    SocketTransport front_trans("RDP Client", one_shot_server.sck, "0.0.0.0", 0
                               , ini.debug.front, 0);

    LCGRandom gen(0);

    const bool fastpath_support = true;
    const bool mem3blt_support  = false;
    Front front(&front_trans, SHARE_PATH "/" DEFAULT_FONT_NAME, &gen, &ini,
        fastpath_support, mem3blt_support);
    null_mod no_mod(front);

    while (front.up_and_running == 0) {
        front.incoming(no_mod);
    }

    LOG(LOG_INFO, "hostname=%s", front.client_info.hostname);

//    const char * target_device = "10.10.46.78";
//    unsigned     target_port   = 3389;
//    const char * username      = "Administrateur@qa";
//    const char * password      = "S3cur3!1nux";
    const char * target_device = "10.10.47.205";
    unsigned     target_port   = 3389;
    const char * username      = "Administrateur";
    const char * password      = "SecureLinux";

    int client_sck = ip_connect(target_device, target_port, 3, 1000, ini.debug.mod_rdp);
    SocketTransport mod_trans( "RDP Server", client_sck, target_device, target_port
                             , /*ini.debug.mod_rdp*/0, &ini.context.auth_error_message);

//    UdevRandom gen;

    try {
        mod_rdp_transparent mod( mod_trans
                               , username
                               , password
                               , "0.0.0.0"
                               , front
                               , target_device
                               , false              // tls
                               , front.client_info
                               , gen
                               , front.keymap.key_flags
                               , ini.globals.auth_channel
                               , ini.globals.alternate_shell.get_cstr()
                               , ini.globals.shell_working_directory.get_cstr()
                               , false  // fast-path
                               , true   // mem3blt
                               , false  // bitmap update
                               , ini.debug.mod_rdp);
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

                front_event.add_to_fd_set(rfds, max);
                mod.event.add_to_fd_set(rfds, max);

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
                    } catch (...) {
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
