/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

*/

#ifndef _REDEMPTION_CORE_SESSION_SERVER_HPP_
#define _REDEMPTION_CORE_SESSION_SERVER_HPP_

#include"config.hpp"
#include"ssl_calls.hpp"
#include"server.hpp"
#include"session.hpp"

class SessionServer : public Server
{
    int * refreshconf ;

    public:
    SessionServer(int * refreshconf) : refreshconf(refreshconf) {
    }

    virtual Server_status start(int incoming_sck)
    {
        union
        {
          struct sockaddr s;
          struct sockaddr_storage ss;
          struct sockaddr_in s4;
          struct sockaddr_in6 s6;
        } u;
        unsigned int sin_size = sizeof(u);
        memset(&u, 0, sin_size);
        TODO("We should manage accept errors")
        int sck = accept(incoming_sck, &u.s, &sin_size);
        char text[256];
        char ip_source[256];
        strcpy(ip_source, inet_ntoa(u.s4.sin_addr));
        /* start new process */
        pid_t pid = fork();
        switch (pid) {
        case 0: /* child */
        {
            close(incoming_sck);

            Inifile ini(CFG_PATH "/" RDPPROXY_INI);
            if (ini.globals.debug.session){
                LOG(LOG_INFO, "Setting new session socket to %d\n", sck);
            }

            int nodelay = 1;
            if (0 == setsockopt(sck, IPPROTO_TCP, TCP_NODELAY, (char*)&nodelay, sizeof(nodelay))){
                wait_obj front_event(sck);
                SocketTransport front_trans("RDP Client", sck, ini.globals.debug.front);

                // Create session file
                int child_pid = getpid();
                char session_file[256];
                sprintf(session_file, "%s/redemption/session_%d.pid", PID_PATH, child_pid);
                int fd = open(session_file, O_WRONLY | O_CREAT, S_IRWXU);
                if (fd == -1) {
                    LOG(LOG_ERR, "Writing process id to SESSION ID FILE failed. Maybe no rights ?:%d:%d\n", errno, strerror(errno));
                    _exit(1);
                }
                size_t lg = snprintf(text, 255, "%d", child_pid);
                if (write(fd, text, lg) == -1) {
                    LOG(LOG_ERR, "Couldn't write pid to %s: %s", PID_PATH "/redemption/session_<pid>.pid", strerror(errno));
                    _exit(1);
                }
                close(fd);

                // Launch session
                LOG(LOG_INFO, "New session on %u (pid=%u) from %s", (unsigned)sck, (unsigned)child_pid, ip_source);
                Session session(front_event, front_trans, ip_source, this->refreshconf, &ini);

                // Suppress session file
                unlink(session_file);

                if (ini.globals.debug.session){
                    LOG(LOG_INFO, "Session::end of Session(%u)", sck);
                }

                shutdown(sck, 2);
                close(sck);
            }
            else {
                LOG(LOG_ERR, "Failed to set socket TCP_NODELAY option on client socket");
            }
            return START_WANT_STOP;
        }
        break;
        default: /* father */
        {
            close(sck);
        }
        break;
        case -1:
            // error forking
            LOG(LOG_ERR, "Error creating process for new session : %s\n", strerror(errno));
        break;
        }
        return START_FAILED;
    }
};

#endif
