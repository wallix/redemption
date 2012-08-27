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

#if !defined(CORE_LISTEN_HPP)
#define CORE_LISTEN_HPP

#include "mainloop.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h> // for sockaddr_in
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>

#include "log.hpp"
#include "server.hpp"

using namespace std;

struct Listen {
    Server & server;
    int port;
    int sck;

    Listen(Server & server, int port, bool exit_on_timeout = false, int timeout_sec = 60) 
        : server(server)
        , port(port)
        , sck(0) 
    {
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

        struct sockaddr_in s;
        memset(&s, 0, sizeof(struct sockaddr_in));
        s.sin_family = AF_INET;
        s.sin_port = htons(this->port);
        s.sin_addr.s_addr = INADDR_ANY;

        LOG(LOG_INFO, "Listen: binding socket %d on port %d", this->sck, this->port);
        if (0 != bind(this->sck, (struct sockaddr*)&s, sizeof(struct sockaddr_in))) {
            LOG(LOG_ERR, "Listen: error binding socket [errno=%u] %s", errno, strerror(errno));
            ((this->sck) && (shutdown(this->sck, 2), close(this->sck)));
            goto end_of_listener;
        }

        LOG(LOG_INFO, "Listen: listening on socket %d", this->sck);
        if (0 != listen(this->sck, 2)) {
            LOG(LOG_ERR, "Listen: error listening on socket\n");
            goto end_of_listener;
        }

        while (1) {
            fd_set rfds;
            FD_ZERO(&rfds);
            FD_SET(this->sck, &rfds);
            struct timeval timeout;
            timeout.tv_sec = timeout_sec;
            timeout.tv_usec = 0;

            switch (select(this->sck + 1, &rfds, 0, 0, &timeout)){
            default:
                if ((errno == EAGAIN) || (errno == EWOULDBLOCK) || (errno == EINPROGRESS) || (errno == EINTR)) {
                    continue; /* these are not really errors */
                }
                LOG(LOG_WARNING, "socket error detected in listen (%s)\n", strerror(errno));
            goto end_of_listener;
            case 0:
                if (exit_on_timeout){
                    LOG(LOG_INFO, "Listener exiting on timeout");
                    goto end_of_listener;
                }
            break;
            case 1:
                if (Server::START_WANT_STOP == this->server.start(this->sck)){
                    close(this->sck);
                    goto end_of_listener;
                }
            break; 
            }
        }
        end_of_listener:;
        if (this->sck){
            shutdown(this->sck, 2);
            close(this->sck);
        }
    }
};

#endif
