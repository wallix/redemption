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
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean

   Network related utility functions

*/

#ifndef _REDEMPTION_UTILS_NETUTILS_HPP_
#define _REDEMPTION_UTILS_NETUTILS_HPP_

#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <netdb.h>

#include"log.hpp"

static inline bool try_again(int errnum){
    int res = false;
    TODO("Check wich signals are actually necessary depending on what we are doing "
         "looks like EINPROGRESS or EALREADY only occurs when calling connect()"
         "EAGAIN is when blocking IO would block (other name for EWOULDBLOCK)"
         "EINTR when an interruption stopped system call (and we could do it again)")
    switch (errnum){
        case EAGAIN:
        /* case EWOULDBLOCK: */ // same as EAGAIN on Linux
        case EINPROGRESS:
        case EALREADY:
        case EBUSY:
        case EINTR:
            res = true;
            break;
        default:
            ;
    }
    return res;
}


static inline int ip_connect(const char* ip, int port,
             int nbretry = 3, int retry_delai_ms = 1000,
             uint32_t verbose = 0)
{
    LOG(LOG_INFO, "connecting to %s:%d\n", ip, port);
    // we will try connection several time
    // the trial process include "ocket opening, hostname resolution, etc
    // because some problems can come from the local endpoint,
    // not necessarily from the remote endpoint.
    int sck = socket(PF_INET, SOCK_STREAM, 0);

    /* set snd buffer to at least 32 Kbytes */
    int snd_buffer_size = 32768;
    unsigned int option_len = sizeof(snd_buffer_size);
    if (0 == getsockopt(sck, SOL_SOCKET, SO_SNDBUF, &snd_buffer_size, &option_len)) {
        if (snd_buffer_size < 32768) {
            snd_buffer_size = 32768;
            if (-1 == setsockopt(sck,
                    SOL_SOCKET,
                    SO_SNDBUF,
                    &snd_buffer_size, sizeof(snd_buffer_size))){
                LOG(LOG_WARNING, "setsockopt failed with errno=%d", errno);
                return -1;
            }
        }
    }
    else {
        LOG(LOG_WARNING, "getsockopt failed with errno=%d", errno);
        return -1;
    }

    union
    {
      struct sockaddr s;
      struct sockaddr_storage ss;
      struct sockaddr_in s4;
      struct sockaddr_in6 s6;
    } u;

    memset(&u, 0, sizeof(u));
    u.s4.sin_family = AF_INET;
    u.s4.sin_port = htons(port);
    u.s4.sin_addr.s_addr = inet_addr(ip);

    if (u.s4.sin_addr.s_addr == INADDR_NONE) {
    TODO(" gethostbyname is obsolete use new function getnameinfo")
        LOG(LOG_INFO, "Asking ip to DNS for %s\n", ip);
        struct hostent *h = gethostbyname(ip);
        if (!h) {
            LOG(LOG_ERR, "DNS resolution failed for %s with errno =%d (%s)\n",
                ip, errno, strerror(errno));
            return -1;
        }
        u.s4.sin_addr.s_addr = *((int*)(*(h->h_addr_list)));
    }

    fcntl(sck, F_SETFL, fcntl(sck, F_GETFL) | O_NONBLOCK);

    int trial = 0;
    for (; trial < nbretry ; trial++){
        int res = ::connect(sck, &u.s, sizeof(u));
        if (-1 != res){
            // connection suceeded
            break;
        }
        if (trial > 0){
            LOG(LOG_INFO, "Connection to %s failed with errno = %d (%s)",
                ip, errno, strerror(errno));
        }
        if ((errno == EINPROGRESS) || (errno == EALREADY)){
            // try again
            fd_set fds;
            FD_ZERO(&fds);
            struct timeval timeout = {
                retry_delai_ms / 1000,
                1000 * (retry_delai_ms % 1000)
            };
            FD_SET(sck, &fds);
            // exit select on timeout or connect or error
            // connect will catch the actual error if any,
            // no need to care of select result
            select(sck+1, NULL, &fds, NULL, &timeout);
        }
        else {
            // real failure
           trial = nbretry;
        }
    }
    if (trial >= nbretry){
        LOG(LOG_INFO, "All trials done connecting to %s:%d\n", ip, port);
        return -1;
    }
    LOG(LOG_INFO, "connection to %s:%d succeeded : socket %d\n", ip, port, sck);

    return sck;
}

#endif
