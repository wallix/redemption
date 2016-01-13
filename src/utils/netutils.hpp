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

#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <netdb.h>

#include "log.hpp"

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

namespace { namespace detail_ { namespace netutils {

bool set_snd_buffer(int sck, int buffer_size = 32768) {
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

int connect_sck(
    int sck, int nbretry, int retry_delai_ms,
    sockaddr & addr, socklen_t addr_len,
    const char * ip, int port
) {
    fcntl(sck, F_SETFL, fcntl(sck, F_GETFL) | O_NONBLOCK);

    char ip_addr[256];
    memset(ip_addr, 0, sizeof(ip_addr));
    if ((addr.sa_family == AF_INET) && !isdigit(*ip)) {
        union
        {
            struct sockaddr s;
            struct sockaddr_storage ss;
            struct sockaddr_in s4;
            struct sockaddr_in6 s6;
        } u;
        memset(&u, 0, sizeof(u));
        memcpy(&u.s, &addr, sizeof(u.s));
        snprintf(ip_addr, sizeof(ip_addr), " (%s)", inet_ntoa(u.s4.sin_addr));
    }

    int trial = 0;
    for (; trial < nbretry ; trial++){
        int res = ::connect(sck, &addr, addr_len);
        if (-1 != res){
            // connection suceeded
            break;
        }
        int const err =  errno;
        if (trial > 0){
            LOG(LOG_INFO, "Connection to %s%s failed with errno = %d (%s)", ip, ip_addr, err, strerror(err));
        }
        if ((err == EINPROGRESS) || (err == EALREADY)){
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
            select(sck+1, nullptr, &fds, nullptr, &timeout);
        }
        else {
            // real failure
           trial = nbretry;
        }
    }

    if (trial >= nbretry){
        if (port == -1) {
            LOG(LOG_INFO, "All trials done connecting to %s", ip);
        }
        else {
            LOG(LOG_INFO, "All trials done connecting to %s:%d", ip, port);
        }
        return -1;
    }

    if (port == -1) {
        LOG(LOG_INFO, "connection to %s succeeded : socket %d", ip, sck);
    }
    else {
        LOG(LOG_INFO, "connection to %s:%d succeeded : socket %d", ip, port, sck);
    }

    return sck;
}

} } }

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
    if (!detail_::netutils::set_snd_buffer(sck, 32768)) {
        return -1;
    }
    
    union
    {
      sockaddr s;
      sockaddr_storage ss;
      sockaddr_in s4;
      sockaddr_in6 s6;
    } u;

    memset(&u, 0, sizeof(u));
    u.s4.sin_family = AF_INET;
    u.s4.sin_port = htons(port);
    u.s4.sin_addr.s_addr = inet_addr(ip);

    if (u.s4.sin_addr.s_addr == INADDR_NONE) {
        struct addrinfo * addr_info = nullptr;
        int               result    = getaddrinfo(ip, nullptr, nullptr, &addr_info);

        if (result) {
            int          _error;
            const char * _strerror;

            if (result == EAI_SYSTEM) {
                _error    = errno;
                _strerror = strerror(errno);
            }
            else {
                _error    = result;
                _strerror = gai_strerror(result);
            }
            LOG(LOG_ERR, "DNS resolution failed for %s with errno =%d (%s)\n",
                ip, _error, _strerror);
            return -1;
        }
        u.s4.sin_addr.s_addr = (reinterpret_cast<sockaddr_in *>(addr_info->ai_addr))->sin_addr.s_addr;
        freeaddrinfo(addr_info);
    }

    return detail_::netutils::connect_sck(sck, nbretry, retry_delai_ms, u.s, sizeof(u), ip, port);
}


static inline int local_connect(const char* sck_name,
             int nbretry = 3, int retry_delai_ms = 1000,
             uint32_t verbose = 0)
{
    LOG(LOG_INFO, "connecting to %s", sck_name);
    // we will try connection several time
    // the trial process include "ocket opening, hostname resolution, etc
    // because some problems can come from the local endpoint,
    // not necessarily from the remote endpoint.
    int sck = socket(AF_UNIX, SOCK_STREAM, 0);

    /* set snd buffer to at least 32 Kbytes */
    if (!detail_::netutils::set_snd_buffer(sck, 32768)) {
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

    return detail_::netutils::connect_sck(
        sck, nbretry, retry_delai_ms,
        u.addr,
        static_cast<int>(offsetof(sockaddr_un, sun_path) + strlen(u.s.sun_path) + 1u),
        sck_name, -1
    );
}

#endif
