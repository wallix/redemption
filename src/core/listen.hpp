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

   listener layer management

*/


#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h> // for sockaddr_in
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>

#include "utils/log.hpp"
#include "utils/invalid_socket.hpp"
#include "utils/select.hpp"
#include "utils/sugar/unique_fd.hpp"
#include "utils/sugar/scope_exit.hpp"
#include "utils/timeval_ops.hpp"
#include "cxx/diagnostic.hpp"

#include <chrono>

#if !defined(IP_TRANSPARENT)
#define IP_TRANSPARENT 19
#endif

enum class EnableTransparentMode : bool { No, Yes };

inline unique_fd create_local_server(char const* sck_name)
{
    unique_fd unique_sck {socket(PF_INET, SOCK_STREAM, 0)};
    int const sck = unique_sck.fd();

    /* reuse same port if a previous daemon was stopped */
    int allow_reuse = 1;
    setsockopt(sck, SOL_SOCKET, SO_REUSEADDR, &allow_reuse, sizeof(allow_reuse));

    /* set snd buffer to at least 32 Kbytes */
    int snd_buffer_size = 32768;
    unsigned int option_len = sizeof(snd_buffer_size);
    if (0 == getsockopt(sck, SOL_SOCKET, SO_SNDBUF, &snd_buffer_size, &option_len)) {
        if (snd_buffer_size < 32768) {
            snd_buffer_size = 32768;
            setsockopt(sck, SOL_SOCKET, SO_SNDBUF, &snd_buffer_size, sizeof(snd_buffer_size));
        }
    }

    /* set non blocking */
    fcntl(sck, F_SETFL, fcntl(sck, F_GETFL) | O_NONBLOCK);

    union
    {
        sockaddr_un s;
        sockaddr addr;
    } u;

    auto len = strnlen(sck_name, sizeof(u.s.sun_path)-1u);
    memcpy(u.s.sun_path, sck_name, len);
    u.s.sun_path[len] = 0;
    u.s.sun_family = AF_UNIX;

    LOG(LOG_INFO, "Listen: binding socket %d on %s", sck, sck_name);
    if (0 != ::bind(sck, &u.addr, sizeof(u))) {
        LOG(LOG_ERR, "Listen: error binding socket [errno=%d] %s", errno, strerror(errno));
        return invalid_fd();
    }

    LOG(LOG_INFO, "Listen: listening on socket %d", sck);
    if (0 != listen(sck, 2)) {
        LOG(LOG_ERR, "Listen: error listening on socket\n");
    }

    // OK, keep the temporary socket everything was fine
    return unique_sck;
}

inline unique_fd create_server_bind_sck(
    unique_fd fd_sck, sockaddr& addr, EnableTransparentMode enable_transparent_mode)
{
    const int sck = fd_sck.fd();

    /* reuse same port if a previous daemon was stopped */
    int allow_reuse = 1;
    setsockopt(sck, SOL_SOCKET, SO_REUSEADDR, &allow_reuse, sizeof(allow_reuse));

    /* set snd buffer to at least 32 Kbytes */
    int snd_buffer_size = 32768;
    unsigned int option_len = sizeof(snd_buffer_size);
    if (0 == getsockopt(sck, SOL_SOCKET, SO_SNDBUF, &snd_buffer_size, &option_len)) {
        if (snd_buffer_size < 32768) {
            snd_buffer_size = 32768;
            setsockopt(sck, SOL_SOCKET, SO_SNDBUF, &snd_buffer_size, sizeof(snd_buffer_size));
        }
    }

    /* set non blocking */
    fcntl(sck, F_SETFL, fcntl(sck, F_GETFL) | O_NONBLOCK);

    if (0 != ::bind(sck, &addr, sizeof(addr))) {
        LOG(LOG_ERR, "Listen: error binding socket [errno=%d] %s", errno, strerror(errno));
        return invalid_fd();
    }

    if (bool(enable_transparent_mode)) {
        LOG(LOG_INFO, "Enable transparent proxying on listened socket.\n");
        int optval = 1;

        if (setsockopt(sck, SOL_IP, IP_TRANSPARENT, &optval, sizeof(optval))) {
            LOG(LOG_ERR, "Failed to enable transparent proxying on listened socket.\n");
            return invalid_fd();
        }
    }

    LOG(LOG_INFO, "Listen: listening on socket %d", sck);
    if (0 != listen(sck, 2)) {
        LOG(LOG_ERR, "Listen: error listening on socket\n");
    }

    // OK, keep the temporary socket everything was fine
    return fd_sck;
}

inline unique_fd create_unix_server(
    char const* sck_name,
    EnableTransparentMode enable_transparent_mode = EnableTransparentMode::No)
{
    unique_fd sck {socket(AF_UNIX, SOCK_STREAM, 0)};

    union
    {
      sockaddr_un s;
      sockaddr addr;
    } u;

    auto len = strnlen(sck_name, sizeof(u.s.sun_path)-1u);
    memcpy(u.s.sun_path, sck_name, len);
    u.s.sun_path[len] = 0;
    u.s.sun_family = AF_UNIX;

    LOG(LOG_INFO, "Listen: binding socket %d on %s", sck.fd(), sck_name);
    return create_server_bind_sck(std::move(sck), u.addr, enable_transparent_mode);
}

inline unique_fd create_server(
    uint32_t s_addr, int port,
    EnableTransparentMode enable_transparent_mode = EnableTransparentMode::No)
{
    unique_fd sck {socket(PF_INET, SOCK_STREAM, 0)};

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
    u.s4.sin_addr.s_addr = s_addr;

    LOG(LOG_INFO, "Listen: binding socket %d on %s:%d",
        sck.fd(), ::inet_ntoa(u.s4.sin_addr), port);
    return create_server_bind_sck(std::move(sck), u.s, enable_transparent_mode);
}

struct ServerLoopIgnoreTimeout
{
    bool operator()(int /*sck*/) const noexcept
    {
        return true;
    }
};

inline bool is_no_error_server_loop() noexcept
{
    REDEMPTION_DIAGNOSTIC_PUSH
    REDEMPTION_DIAGNOSTIC_GCC_ONLY_IGNORE("-Wlogical-op")
    if (errno == EAGAIN
     || errno == EWOULDBLOCK
     || errno == EINPROGRESS
     || errno == EINTR
    ) {
        return true; /* these are not really errors */
    }
    REDEMPTION_DIAGNOSTIC_POP
    return false;
}

template<class CbNewConn>
int unique_server_loop(unique_fd sck, CbNewConn&& cb_new_conn)
{
    SCOPE_EXIT(if (sck) shutdown(sck.fd(), SHUT_RDWR));

    fd_set rfds;
    io_fd_zero(rfds);
    const int max = sck.fd() + 1;

    for (;;) {
        io_fd_set(sck.fd(), rfds);
        timeval timeout{std::chrono::minutes(10).count(), 0};

        int const r = select(max, &rfds, nullptr, nullptr, &timeout);

        if (r < 0) {
            if (is_no_error_server_loop()) {
                continue;
            }
            LOG(LOG_ERR, "socket error detected in listen (%s)\n", strerror(errno));
            return -1;
        }

        if (r && !cb_new_conn(sck.fd())) {
            return 0;
        }
    }
}


template<class CbNewConn>
int two_server_loop(unique_fd sck1, unique_fd sck2, CbNewConn&& cb_new_conn)
{
    SCOPE_EXIT(if (sck1) shutdown(sck1.fd(), SHUT_RDWR));
    SCOPE_EXIT(if (sck2) shutdown(sck2.fd(), SHUT_RDWR));

    fd_set rfds;
    io_fd_zero(rfds);
    const int max = std::max(sck1.fd(), sck2.fd()) + 1;

    for (;;) {
        io_fd_set(sck1.fd(), rfds);
        io_fd_set(sck2.fd(), rfds);
        timeval timeout{std::chrono::minutes(10).count(), 0};

        int const r = select(max, &rfds, nullptr, nullptr, &timeout);

        if (r < 0) {
            if (is_no_error_server_loop()) {
                continue;
            }
            LOG(LOG_ERR, "socket error detected in listen (%s)\n", strerror(errno));
            return -1;
        }

        if (r) {
            bool stopped = false;
            if (io_fd_isset(sck1.fd(), rfds) && !cb_new_conn(sck1.fd())) {
                stopped = true;
            }
            if (io_fd_isset(sck2.fd(), rfds) && !cb_new_conn(sck2.fd())) {
                stopped = true;
            }
            if (stopped) {
                return 0;
            }
        }
    }
}
