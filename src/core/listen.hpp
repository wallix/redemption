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
#include <netdb.h>

#include "utils/log.hpp"
#include "utils/invalid_socket.hpp"
#include "utils/select.hpp"
#include "utils/sugar/unique_fd.hpp"
#include "utils/sugar/scope_exit.hpp"
#include "utils/sugar/zstring_view.hpp"
#include "cxx/diagnostic.hpp"

#include <chrono>

#if !defined(IP_TRANSPARENT)
#define IP_TRANSPARENT 19
#endif

enum class EnableTransparentMode : bool { No, Yes };

inline unique_fd
create_server_bind_sck(unique_fd fd_sck,
                       sockaddr_storage& addr,
                       socklen_t addrlen,
                       EnableTransparentMode enable_transparent_mode) noexcept
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

    if (0 != ::bind(sck, &reinterpret_cast<sockaddr&>(addr), addrlen)) {
        LOG(LOG_ERR, "Listen: error binding socket [errno=%d] %s", errno, strerror(errno));
        return invalid_fd();
    }

    if (bool(enable_transparent_mode)) {
        LOG(LOG_INFO, "Enable transparent proxying on listened socket.");
        int optval = 1;

        if (setsockopt(sck, SOL_IP, IP_TRANSPARENT, &optval, sizeof(optval))) {
            LOG(LOG_ERR, "Failed to enable transparent proxying on listened socket.");
            return invalid_fd();
        }
    }

    LOG(LOG_INFO, "Listen: listening on socket %d", sck);
    if (0 != listen(sck, 2)) {
        LOG(LOG_ERR, "Listen: error listening on socket");
    }

    // OK, keep the temporary socket everything was fine
    return fd_sck;
}

inline unique_fd
create_unix_server(zstring_view sck_name,
                   EnableTransparentMode enable_transparent_mode) noexcept
{
    unique_fd sck {socket(AF_UNIX, SOCK_STREAM, 0)};

    union
    {
      sockaddr_un s;
      sockaddr addr;
      sockaddr_storage ss;
    } u;

    auto len = std::min(sck_name.size(), sizeof(u.s.sun_path)-1u);
    memcpy(u.s.sun_path, sck_name.data(), len);
    u.s.sun_path[len] = 0;
    u.s.sun_family = AF_UNIX;

    LOG(LOG_INFO, "Listen: binding socket %d on %s", sck.fd(), sck_name);
    return create_server_bind_sck(std::move(sck),
                                  u.ss,
                                  sizeof(u.s),
                                  enable_transparent_mode);
}

inline unique_fd
create_server(uint32_t s_addr,
              int port,
              EnableTransparentMode enable_transparent_mode) noexcept
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
    REDEMPTION_DIAGNOSTIC_PUSH()
    REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wold-style-cast") // only to release
    u.s4.sin_port = htons(port);
    REDEMPTION_DIAGNOSTIC_POP()
    u.s4.sin_addr.s_addr = s_addr;

    LOG(LOG_INFO, "Listen: binding socket %d on %s:%d",
        sck.fd(), ::inet_ntoa(u.s4.sin_addr), port);
    return create_server_bind_sck(std::move(sck),
                                  u.ss,
                                  sizeof(u.s4),
                                  enable_transparent_mode);
}

inline unique_fd
create_ip_dual_stack_server(int port,
                            EnableTransparentMode enable_transparent_mode)
    noexcept
{
    unique_fd sck(socket(AF_INET6, SOCK_STREAM, 0));

    if (!sck)
    {
        LOG(LOG_ERR, "socket() failed : %s", strerror(errno));
        return invalid_fd();
    }

    union
    {
        sockaddr s;
        sockaddr_storage ss;
        sockaddr_in6 s6;
    } u;

    memset(&u, 0, sizeof(u));
    u.s6.sin6_family = AF_INET6;
    REDEMPTION_DIAGNOSTIC_PUSH()
    REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wold-style-cast") // only to release
    u.s6.sin6_port = htons(port);
    REDEMPTION_DIAGNOSTIC_POP()
    u.s6.sin6_addr = in6addr_any;

    char ip_address[INET6_ADDRSTRLEN] { };

    if (int res = getnameinfo(&u.s,
                              sizeof(u.s6),
                              ip_address,
                              sizeof(ip_address),
                              nullptr,
                              0,
                              NI_NUMERICHOST))
    {
        LOG(LOG_ERR,
            "getnameinfo() failed : %s",
            (res == EAI_SYSTEM) ? strerror(errno) : gai_strerror(res));
        return invalid_fd();
    }
    LOG(LOG_INFO,
        "Listen: binding socket %d on [%s]:%d",
        sck.fd(),
        ip_address,
        port);

    /* Turn off IPV6_V6ONLY option if
       operating system has net.ipv6.bindv6only equal to 1 */
    if (int allow_ipv6_only = 0; setsockopt(sck.fd(),
                                            IPPROTO_IPV6,
                                            IPV6_V6ONLY,
                                            &allow_ipv6_only,
                                            sizeof(allow_ipv6_only)) == -1)
    {
        LOG(LOG_ERR, "setsockopt() failed: %s", strerror(errno));
        return invalid_fd();
    }
    return create_server_bind_sck(std::move(sck),
                                  u.ss,
                                  sizeof(u.s6),
                                  enable_transparent_mode);
}

[[nodiscard]]
inline unique_fd
interface_create_server(bool enable_ipv6,
                        uint32_t s_addr,
                        int port,
                        EnableTransparentMode enable_transparent_mode) noexcept
{
    return enable_ipv6 ?
        create_ip_dual_stack_server(port, enable_transparent_mode) :
        create_server(s_addr, port, enable_transparent_mode);
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
    REDEMPTION_DIAGNOSTIC_PUSH()
    REDEMPTION_DIAGNOSTIC_GCC_ONLY_IGNORE("-Wlogical-op")
    // these are not really errors
    return errno == EAGAIN
        || errno == EWOULDBLOCK
        || errno == EINPROGRESS
        || errno == EINTR
    ;
    REDEMPTION_DIAGNOSTIC_POP()
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
        int const r = select(max, &rfds, nullptr, nullptr, nullptr);

        if (r < 0) {
            if (is_no_error_server_loop()) {
                continue;
            }
            LOG(LOG_ERR, "socket error detected in listen (%s)", strerror(errno));
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
        int const r = select(max, &rfds, nullptr, nullptr, nullptr);

        if (r < 0) {
            if (is_no_error_server_loop()) {
                continue;
            }
            LOG(LOG_ERR, "socket error detected in listen (%s)", strerror(errno));
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
