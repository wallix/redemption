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


#pragma once

#include "utils/log.hpp"

#include <cstddef>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/un.h>

#include "utils/sugar/array_view.hpp"
#include "utils/select.hpp"
#include "utils/log.hpp"

static inline bool try_again(int errnum){
    int res = false;
    // TODO Check wich signals are actually necessary depending on what we are doing
    // looks like EINPROGRESS or EALREADY only occurs when calling connect()
    // EAGAIN is when blocking IO would block (other name for EWOULDBLOCK)
    // EINTR when an interruption stopped system call (and we could do it again)
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

inline bool set_snd_buffer(int sck, int buffer_size = 32768) {
    /* set snd buffer to at least 32 Kbytes */
    int snd_buffer_size = buffer_size;
    socklen_t option_len = static_cast<socklen_t>(sizeof(snd_buffer_size));
    if (0 == getsockopt(sck, SOL_SOCKET, SO_SNDBUF, &snd_buffer_size, &option_len)) {
        if (snd_buffer_size < buffer_size) {
            snd_buffer_size = buffer_size;
            if (-1 == setsockopt(sck,
                    SOL_SOCKET,
                    SO_SNDBUF,
                    &snd_buffer_size, sizeof(snd_buffer_size))){
                LOG(LOG_WARNING, "setsockopt failed with errno=%d", errno);
                return false;
            }
        }
    }
    else {
        LOG(LOG_WARNING, "getsockopt failed with errno=%d", errno);
        return false;
    }

    return true;
}

inline int connect_sck(int sck, int nbretry, int retry_delai_ms, sockaddr & addr, socklen_t addr_len, const char * target)
{
    fcntl(sck, F_SETFL, fcntl(sck, F_GETFL) | O_NONBLOCK);

    int trial = 0;
    for (; trial < nbretry ; trial++){
        int res = ::connect(sck, &addr, addr_len);
        if (-1 != res){
            // connection suceeded
            break;
        }
        int const err =  errno;
        if (trial > 0){
            LOG(LOG_INFO, "Connection to %s failed with errno = %d (%s)", target, err, strerror(err));
        }
        if ((err == EINPROGRESS) || (err == EALREADY)){
            // try again
            fd_set fds;
            io_fd_zero(fds);
            io_fd_set(sck, fds);
            struct timeval timeout = {
                retry_delai_ms / 1000,
                1000 * (retry_delai_ms % 1000)
            };
            // exit select on timeout or connect or error
            // connect will catch the actual error if any,
            // no need to care of select result
            select(sck+1, nullptr, &fds, nullptr, &timeout);
        }
        else {
            // real failure
           trial = nbretry;
        }
    }

    if (trial >= nbretry){
        LOG(LOG_INFO, "All trials done connecting to %s", target);
        return -1;
    }

    LOG(LOG_INFO, "connection to %s succeeded : socket %d", target, sck);
    return sck;
}

static int resolve_ipv4_address(const char* ip, in_addr & s4_sin_addr)
{
    if (!inet_aton(ip, &s4_sin_addr)) {
        struct addrinfo * addr_info = nullptr;
        int               result    = getaddrinfo(ip, nullptr, nullptr, &addr_info);
        if (result) {
            LOG(LOG_ERR, "DNS resolution failed for %s with errno = %d (%s)\n",
                ip, (result == EAI_SYSTEM) ? errno : result
                  , (result == EAI_SYSTEM) ? strerror(errno) : gai_strerror(result));
            return -1;
        }
        s4_sin_addr.s_addr = (reinterpret_cast<sockaddr_in *>(addr_info->ai_addr))->sin_addr.s_addr;
        freeaddrinfo(addr_info);
    }
    return 0;
}

static inline int ip_connect(const char* ip, int port,
             int nbretry /* 3 */, int retry_delai_ms /*1000*/)
{
    LOG(LOG_INFO, "connecting to %s:%d\n", ip, port);


    // we will try connection several time
    // the trial process include "socket opening, hostname resolution, etc
    // because some problems can come from the local endpoint,
    // not necessarily from the remote endpoint.
    int sck = socket(PF_INET, SOCK_STREAM, 0);

    union
    {
      sockaddr s;
      sockaddr_storage ss;
      sockaddr_in s4;
      sockaddr_in6 s6;
    } u;

    memset(&u, 0, sizeof(u));
    u.s4.sin_family = AF_INET;
    REDEMPTION_DIAGNOSTIC_PUSH
    REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wold-style-cast") // only to release
    u.s4.sin_port = htons(port);
    REDEMPTION_DIAGNOSTIC_POP
    int status = resolve_ipv4_address(ip, u.s4.sin_addr);
    if (status){
        LOG(LOG_INFO, "Connecting to %s:%d failed\n", ip, port);
        close(sck);
        return status;
    }

    /* set snd buffer to at least 32 Kbytes */
    if (!set_snd_buffer(sck, 32768)) {
        LOG(LOG_INFO, "Connecting to %s:%d failed : cannot set socket buffer size\n", ip, port);
        close(sck);
        return -1;
    }


    char text_target[256];
    snprintf(text_target, sizeof(text_target), "%s:%d (%s)", ip, port, inet_ntoa(u.s4.sin_addr));

    return connect_sck(sck, nbretry, retry_delai_ms, u.s, sizeof(u), text_target);
}


// TODO int retry_delai_ms -> std::milliseconds
static inline int local_connect(const char* sck_name,
             int nbretry = 3, int retry_delai_ms = 1000)
{
    char target[1024] = {};
    snprintf(target, sizeof(target), "%s", sck_name);

    LOG(LOG_INFO, "connecting to %s", sck_name);
    // we will try connection several time
    // the trial process include "ocket opening, hostname resolution, etc
    // because some problems can come from the local endpoint,
    // not necessarily from the remote endpoint.
    int sck = socket(AF_UNIX, SOCK_STREAM, 0);

    /* set snd buffer to at least 32 Kbytes */
    if (!set_snd_buffer(sck, 32768)) {
        return -1;
    }

    union
    {
      sockaddr_un s;
      sockaddr addr;
    } u;

    auto len = strnlen(sck_name, sizeof(u.s.sun_path)-1u);
    memcpy(u.s.sun_path, sck_name, len);
    u.s.sun_path[len] = 0;
    u.s.sun_family = AF_UNIX;

    return connect_sck(sck, nbretry, retry_delai_ms, u.addr, static_cast<int>(offsetof(sockaddr_un, sun_path) + strlen(u.s.sun_path) + 1u), target);
}
