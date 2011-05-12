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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   listener layer management

*/

#if !defined(LISTEN_HPP)
#define LISTEN_HPP

#include "mainloop.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h> // for sockaddr_in
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include "log.hpp"
#include <errno.h>
#include <pthread.h>
#include "session.hpp"
#include "wait_obj.hpp"
#include <iostream>
#include <vector>
using namespace std;

/* rdp listener */
struct Listen {
    int sck;

    Listen() {
        this->sck = 0;
    }

    ~Listen()
    {
        /* stop listening */
        if (this->sck){
            shutdown(this->sck, 2);
            close(this->sck);
        }
    }

    /*****************************************************************************/
    /* wait for incoming connections */
    int listen_main_loop()
    {
        int rv = 0;
        try{
            Inifile ini(CFG_PATH "/" RDPPROXY_INI);

            struct sockaddr_in s;
            this->sck = socket(PF_INET, SOCK_STREAM, 0);

            /* reuse same port if a previous daemon was stopped */
            unsigned int option_len;
            int allow_reuse = 1;
            setsockopt(this->sck, SOL_SOCKET, SO_REUSEADDR, (char*)&allow_reuse, sizeof(allow_reuse));

            /* set snd buffer to at least 32 Kbytes */
            int snd_buffer_size;
            option_len = sizeof(snd_buffer_size);
            if (0 == getsockopt(this->sck, SOL_SOCKET, SO_SNDBUF,
                (char*)&snd_buffer_size, &option_len)) {
                if (snd_buffer_size < 32768) {
                    snd_buffer_size = 32768;
                    setsockopt(this->sck, SOL_SOCKET, SO_SNDBUF,
                        (char*)&snd_buffer_size, sizeof(snd_buffer_size));
                }
            }

            /* set non blocking */
            fcntl(this->sck, F_SETFL, fcntl(this->sck, F_GETFL) | O_NONBLOCK);

            memset(&s, 0, sizeof(struct sockaddr_in));
            s.sin_family = AF_INET;
            s.sin_port = htons(ini.globals.port);
            s.sin_addr.s_addr = INADDR_ANY;
            rv = bind(this->sck, (struct sockaddr*)&s, sizeof(struct sockaddr_in));

            if (rv != 0) {
                ((this->sck) && (shutdown(this->sck, 2), close(this->sck)));
                rv = 1;
                LOG(LOG_ERR, "listen_main_loop error at binding return\n");
                throw 1;
            }

            rv = listen(this->sck, 2);
            if (0 != rv) {
                LOG(LOG_ERR, "listen error in listen_main_loop\n");
                throw 1;
            }
            struct wait_obj listen_event(this->sck);

            while (1) {
                int robjs_count = 0;
                unsigned robjs[8];
                robjs[robjs_count++] = listen_event.obj;

                fd_set rfds;
                fd_set wfds;

                unsigned max = 0;
                FD_ZERO(&rfds);
                FD_ZERO(&wfds);
                for (int i = 0; i < robjs_count; i++) {
                    FD_SET(robjs[i], &rfds);
                    if (robjs[i] > max) {
                        max = robjs[i];
                    }
                }

                int i = select(max + 1, &rfds, &wfds, 0, 0);
                if (i < 0
                && (errno == EAGAIN
                || errno == EWOULDBLOCK
                ||  errno == EINPROGRESS
                ||  errno == EINTR)) {
                        continue; /* these are not really errors */
                }
                if (i < 0) {
                    LOG(LOG_WARNING, "socket error detected in listen (%s)\n", strerror(errno));
                    break;
                }

                 /* incoming connection */
                if (listen_event.is_set())
                {
                    struct sockaddr_in sin;
                    unsigned int sin_size = sizeof(struct sockaddr_in);
                    memset(&sin, 0, sin_size);
                    int sck = accept(this->sck, (struct sockaddr*)&sin, &sin_size);
                    char ip_source[256];
                    strcpy(ip_source, inet_ntoa(sin.sin_addr));
                    if (i < 0
                        && errno == EAGAIN
                        && errno == EWOULDBLOCK
                        &&  errno == EINPROGRESS
                        &&  errno == EINTR) {
                            continue; /* these are not really errors */
                    }
                    if (i < 0) {
                        LOG(LOG_INFO, "socket Listener : accept error detected\n");
                        break;
                    }
                    /* start new process */
                    pid_t pid = fork();
                    switch (pid) {
                    case 0: /* child */
                    try {
                        close(this->sck);
                        LOG(LOG_INFO, "Setting new session socket to %d\n", sck);
                        Session session(sck, ip_source, &ini);
                        session.session_main_loop();
                    } catch (...) {
                    };
                    exit(0);
                    break;
                    default: /* father */
                        close(sck);
                        break;
                    case -1:
                        // error forking
                        LOG(LOG_INFO, "Error creating process for new session : %s\n", strerror(errno));
                    break;
                    }
                }
            }
        }
        catch(...){
           LOG(LOG_WARNING, "socket Listener end on exception raised\n");
           rv = 1;
        }
       return rv;
    }
};

#endif
