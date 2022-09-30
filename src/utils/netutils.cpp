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

#include "utils/netutils.hpp"

#include "utils/log.hpp"
#include "utils/select.hpp"
#include "utils/static_string.hpp"
#include "utils/sugar/split.hpp"
#include "utils/sugar/int_to_chars.hpp"
#include "utils/sugar/chars_to_int.hpp"

#include <charconv>

#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstring>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static_assert(sizeof(IpAddress::ip_addr) >= INET6_ADDRSTRLEN);

void AddrInfoDeleter::operator()(addrinfo *addr_info) noexcept
{
    freeaddrinfo(addr_info);
}

bool try_again(int errnum)
{
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

namespace
{
    bool set_snd_buffer(int sck, int buffer_size)
    {
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
                    LOG(LOG_WARNING, "setsockopt(SOL_SOCKET, SO_SNDBUF) failed with errno = %d (%s)", errno, strerror(errno));
                    return false;
                }
            }
        }
        else {
            LOG(LOG_WARNING, "getsockopt(SOL_SOCKET, SO_SNDBUF) failed with errno = %d (%s)", errno, strerror(errno));
            return false;
        }

        return true;
    }

    void set_tcp_user_timeout(int sck, std::chrono::milliseconds timeout)
    {
        unsigned int tcp_user_timeout = timeout.count();
        if (-1 == setsockopt(sck,
                IPPROTO_TCP,
                TCP_USER_TIMEOUT,
                &tcp_user_timeout, sizeof(tcp_user_timeout))){
            LOG(LOG_WARNING, "setsockopt(IPPROTO_TCP, TCP_USER_TIMEOUT) failed with errno = %d (%s)", errno, strerror(errno));
        }
    }

    unique_fd connect_sck(unique_fd sck, std::chrono::milliseconds connection_establishment_timeout,
                          sockaddr & addr, socklen_t addr_len, const char * target, bool no_log,
                          char const** error_result = nullptr)
    {
        int raw_sck = sck.fd();

        fcntl(raw_sck, F_SETFL, fcntl(raw_sck, F_GETFL) | O_NONBLOCK);

        if (-1 == ::connect(raw_sck, &addr, addr_len)) {
            int const err = errno;
            if (err != EINPROGRESS && err != EALREADY) {
                goto connection_error;
            }

            fd_set fds;
            io_fd_zero(fds);
            io_fd_set(raw_sck, fds);
            auto connection_establishment_timeout_ms = connection_establishment_timeout.count();
            timeval timeout = {
                connection_establishment_timeout_ms / 1000,
                (connection_establishment_timeout_ms % 1000) * 1000
            };
            // exit select on timeout or connect or error
            // connect will catch the actual error if any,
            // no need to care of select result
            int res = select(raw_sck+1, nullptr, &fds, nullptr, &timeout);
            if (res != 1) {
                goto connection_error;
            }

            int sck_error = 0;
            socklen_t optlen = sizeof(sck_error);
            if (-1 == getsockopt(raw_sck, SOL_SOCKET, SO_ERROR, &sck_error, &optlen)) {
                goto connection_error;
            }

            if (sck_error != 0) {
                goto connection_error;
            }
        }

        LOG_IF(!no_log, LOG_INFO, "connection to %s succeeded : socket %d", target, raw_sck);
        return sck;

        connection_error:
        int const err = errno;
        char const* errmes = strerror(err);
        if (error_result) {
            *error_result = errmes;
        }
        LOG(LOG_INFO, "Connection to %s failed with errno = %d (%s)", target, err, errmes);
        return unique_fd{-1};
    }
} // namespace

char const* resolve_ipv4_address(const char* ip, in_addr & s4_sin_addr)
{
    if (!inet_aton(ip, &s4_sin_addr)) {
        struct addrinfo *addr_info = nullptr;
        struct addrinfo hints { };

        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        int result = getaddrinfo(ip, nullptr, &hints, &addr_info);
        if (result) {
            char const* error = (result == EAI_SYSTEM) ? strerror(errno) : gai_strerror(result);
            LOG(LOG_ERR, "DNS resolution failed for %s with errno = %d (%s)",
                ip, (result == EAI_SYSTEM) ? errno : result, error);
            return error;
        }
        s4_sin_addr.s_addr = (reinterpret_cast<sockaddr_in *>(addr_info->ai_addr))->sin_addr.s_addr; /*NOLINT*/
        freeaddrinfo(addr_info);
    }
    return nullptr;
}

unique_fd ip_connect_ipv4(const char *ip,
                          int port,
                          std::chrono::milliseconds establishment_timeout,
                          std::chrono::milliseconds tcp_user_timeout,
                          char const **error_result)
{
    LOG(LOG_INFO, "connecting to %s:%d", ip, port);

    // we will try connection several time
    // the trial process include "socket opening, hostname resolution, etc
    // because some problems can come from the local endpoint,
    // not necessarily from the remote endpoint.
    unique_fd sck { socket(PF_INET, SOCK_STREAM, 0) };

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
    if (auto error = resolve_ipv4_address(ip, u.s4.sin_addr)){
        if (error_result) {
            *error_result = error;
        }
        LOG(LOG_ERR, "Connecting to %s:%d failed", ip, port);
        return unique_fd{-1};
    }

    /* set snd buffer to at least 32 Kbytes */
    if (!set_snd_buffer(sck.fd(), 32768)) {
        if (error_result) {
            *error_result = "Cannot set socket buffer size";
        }
        LOG(LOG_ERR, "Connecting to %s:%d failed : cannot set socket buffer size", ip, port);
        return unique_fd{-1};
    }

    if (tcp_user_timeout.count()) {
        set_tcp_user_timeout(sck.fd(), tcp_user_timeout);
    }

    char text_target[256];
    snprintf(text_target, sizeof(text_target), "%s:%d (%s)", ip, port, inet_ntoa(u.s4.sin_addr));

    bool const no_log = false;

    return connect_sck(std::move(sck),
                       establishment_timeout,
                       u.s,
                       sizeof(u),
                       text_target,
                       no_log,
                       error_result);
}

unique_fd ip_connect_blocking(const char* ip,
                              int port,
                              std::chrono::milliseconds establishment_timeout,
                              std::chrono::milliseconds tcp_user_timeout,
                              char const** error_result)
{
    auto fd = ip_connect(ip,
                         port,
                         establishment_timeout,
                         tcp_user_timeout,
                         error_result);
    if (fd) {
        const auto sck = fd.fd();
        fcntl(sck, F_SETFL, fcntl(sck, F_GETFL) & ~O_NONBLOCK);
    }
    return fd;
}

AddrInfoPtrWithDel_t
resolve_both_ipv4_and_ipv6_address(const char *ip,
                                   int port,
                                   const char **error_result) noexcept
{
    addrinfo *addr_info = nullptr;
    addrinfo hints { };

    hints.ai_flags |= AI_V4MAPPED;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    auto port_str = int_to_decimal_zchars(port);
    if (int res = ::getaddrinfo(ip, port_str.c_str(), &hints, &addr_info))
    {
        const char *error = (res == EAI_SYSTEM) ?
            strerror(errno) : gai_strerror(res);

        if (error_result)
        {
            *error_result = error;
        }
        LOG(LOG_ERR,
            "DNS resolution failed for %s with errno = %d (%s)",
            ip,
            (res == EAI_SYSTEM) ? errno : res, error);
    }
    return AddrInfoPtrWithDel_t(addr_info);
}

unique_fd ip_connect(const char *ip,
                     int port,
                     DefaultConnectTag,
                     const char **error_result) noexcept
{
    return ip_connect(ip,
                      port,
                      std::chrono::milliseconds(1000),
                      std::chrono::milliseconds::zero(),
                      error_result);
}

unique_fd ip_connect(const char *ip,
                     int port,
                     std::chrono::milliseconds establishment_timeout,
                     std::chrono::milliseconds tcp_user_timeout,
                     const char **error_result) noexcept
{
    AddrInfoPtrWithDel_t addr_info_ptr =
        resolve_both_ipv4_and_ipv6_address(ip, port, error_result);

    if (!addr_info_ptr)
    {
        return unique_fd{-1};
    }


    LOG(LOG_INFO, "connecting to %s:%d", ip, port);

    // we will try connection several time
    // the trial process include "socket opening, hostname resolution, etc
    // because some problems can come from the local endpoint,
    // not necessarily from the remote endpoint.

    int raw_sck = socket(addr_info_ptr->ai_family,
                         addr_info_ptr->ai_socktype,
                         addr_info_ptr->ai_protocol);

    if (raw_sck == -1)
    {
        if (error_result)
        {
            *error_result = "Cannot create socket";
        }
        LOG(LOG_ERR, "socket failed : %s", ::strerror(errno));
        return unique_fd{-1};
    }

    unique_fd sck {raw_sck};

    /* set snd buffer to at least 32 Kbytes */
    if (!set_snd_buffer(raw_sck, 32768))
    {
        if (error_result)
        {
            *error_result = "Cannot set socket buffer size";
        }
        LOG(LOG_ERR,
            "Connecting to %s:%d failed : cannot set socket buffer size",
            ip,
            port);
        return unique_fd{-1};
    }

    if (tcp_user_timeout.count()) {
        set_tcp_user_timeout(raw_sck, tcp_user_timeout);
    }

    char resolved_ip_addr[NI_MAXHOST] { };

    if (int res = ::getnameinfo(addr_info_ptr->ai_addr,
                                addr_info_ptr->ai_addrlen,
                                resolved_ip_addr,
                                sizeof(resolved_ip_addr),
                                nullptr,
                                0,
                                NI_NUMERICHOST))
    {
        if (error_result)
        {
            *error_result = "Cannot get ip address";
        }
        LOG(LOG_ERR, "getnameinfo failed : %s",
            (res == EAI_SYSTEM) ? ::strerror(errno) : ::gai_strerror(res));
        return unique_fd{-1};
    }

    char text_target[2048] { };

    snprintf(text_target, sizeof(text_target),
             "%s:%d (%s)", ip, port, resolved_ip_addr);

    const bool no_log = false;

    return connect_sck(std::move(sck),
                       establishment_timeout,
                       *addr_info_ptr->ai_addr,
                       addr_info_ptr->ai_addrlen,
                       text_target,
                       no_log,
                       error_result);
}

unique_fd local_connect(const char* sck_name,
                        std::chrono::milliseconds establishment_timeout,
                        bool no_log)
{
    LOG_IF(!no_log, LOG_INFO, "connecting to %s", sck_name);
    // we will try connection several time
    // the trial process include "ocket opening, hostname resolution, etc
    // because some problems can come from the local endpoint,
    // not necessarily from the remote endpoint.
    unique_fd sck { socket(AF_UNIX, SOCK_STREAM, 0) };

    /* set snd buffer to at least 32 Kbytes */
    if (!set_snd_buffer(sck.fd(), 32768)) {
        return unique_fd{-1};
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

    return connect_sck(std::move(sck),
                       establishment_timeout,
                       u.addr,
                       checked_int(offsetof(sockaddr_un, sun_path) + len + 1u),
                       sck_name,
                       no_log);
}

unique_fd addr_connect(const char* addr,
                       std::chrono::milliseconds establishment_timeout,
                       bool no_log_for_unix_socket)
{
    const char* pos = strchr(addr, ':');
    if (!pos) {
        return local_connect(addr,
                             establishment_timeout,
                             no_log_for_unix_socket);
    }

    auto port_result = decimal_chars_to_int<int>(pos + 1);
    if (port_result.ec == std::errc()) {
        std::string ip(addr, pos);
        return ip_connect(ip.c_str(), port_result.val, DefaultConnectTag {});
    }

    LOG(LOG_ERR, "Connecting to %s failed: invalid port", pos + 1);
    return unique_fd{-1};
}

unique_fd addr_connect_blocking(
    const char* addr,
    std::chrono::milliseconds establishment_timeout,
    bool no_log_for_unix_socket)
{
    auto fd = addr_connect(addr,
                           establishment_timeout,
                           no_log_for_unix_socket);
    if (fd) {
        const auto sck = fd.fd();
        fcntl(sck, F_SETFL, fcntl(sck, F_GETFL) & ~O_NONBLOCK);
    }
    return fd;
}


struct LineBuffer
{
    char buffer[20480];
    int end_buffer;

    int fd;
    int begin_line;
    int eol;

    explicit LineBuffer(int fd)
    : end_buffer(0)
    , fd(fd)
    , begin_line(0)
    , eol(0)
    {
    }

    int readline()
    {
        for (int i = this->begin_line; i < this->end_buffer; i++){
            if (this->buffer[i] == '\n'){
                this->eol = i+1;
                return 1;
            }
        }
        size_t trailing_room = sizeof(this->buffer) - this->end_buffer;
        // reframe buffer if no trailing room left
        if (trailing_room == 0){
            size_t used_len = this->end_buffer - this->begin_line;
            memmove(this->buffer, &(this->buffer[this->begin_line]), used_len);
            this->end_buffer = used_len;
            this->begin_line = 0;
        }

        ssize_t res = read(this->fd, &(this->buffer[this->end_buffer]), sizeof(this->buffer) - this->end_buffer);
        if (res < 0){
            return res;
        }
        this->end_buffer += res;
        if (this->begin_line == this->end_buffer) {
            return 0;
        }
        for (int i = this->begin_line; i < this->end_buffer; i++){
            if (this->buffer[i] == '\n'){
                this->eol = i+1;
                return 1;
            }
        }
        this->eol = this->end_buffer;
        return 1;
    }
};

bool compare_binary_ipv4(const in_addr& in_addr, const char *ipv4)
{
    assert(ipv4);

    struct in_addr inaddr2;

    return inet_aton(ipv4, &inaddr2)
        && in_addr.s_addr == inaddr2.s_addr;
}

bool compare_binary_ipv6(const in6_addr& in6_addr, const char *ipv6)
{
    assert(ipv6);

    struct in6_addr in6_addr2;

    return inet_pton(AF_INET6, ipv6, &in6_addr2) == 1
        && memcmp(in6_addr.s6_addr,
                  in6_addr2.s6_addr,
                  sizeof(in6_addr::s6_addr)) == 0;
}

bool compare_binary_ip(const sockaddr_storage& ss, const char *ip, bool is_ipv6)
{
    assert(ip);

    return (is_ipv6)
        ? compare_binary_ipv6(reinterpret_cast<const sockaddr_in6&>(ss).sin6_addr, ip)
        : compare_binary_ipv4(reinterpret_cast<const sockaddr_in&>(ss).sin_addr, ip);
}

bool get_in_addr_from_ip(sockaddr_storage& ss, const char *ip, bool is_ipv6)
{
    assert(ip);

    return (is_ipv6)
        ? inet_pton(AF_INET6, ip, &reinterpret_cast<sockaddr_in6 *>(&ss)->sin6_addr) == 1
        : inet_aton(ip, &reinterpret_cast<sockaddr_in *>(&ss)->sin_addr);
}

namespace minipeg
{
namespace
{
    struct Rng
    {
        char const* s;
        char const* e;

        std::size_t size() const noexcept { return std::size_t(e-s); }
    };

    auto to_parser(char c)
    {
        return [c](Rng& r){
            if (*r.s == c) {
                ++r.s;
                return true;
            }
            return false;
        };
    }

    auto to_parser(std::string_view s)
    {
        return [s](Rng& r){
            if (r.size() >= s.size()) {
                if (std::string_view(r.s, s.size()) == s) {
                    r.s += s.size();
                    return true;
                }
            }
            return false;
        };
    }

    auto to_parser(char const* s)
    {
        return to_parser(std::string_view(s));
    }

    template<class P>
    P to_parser(P p)
    {
        return p;
    }

    template<class... Ps>
    auto group(Ps... ps)
    {
        if constexpr (sizeof...(Ps) == 1) {
            return [](auto p) { return p; }(to_parser(ps)...);
        }
        else {
            return [](auto... p) {
                return [=](Rng& r){
                    auto s = r.s;

                    if ((... && p(r))) {
                        return true;
                    }
                    r.s = s;
                    return false;
                };
            }(to_parser(ps)...);
        }
    }

    template<class... Ps>
    auto alternative(Ps... ps)
    {
        if constexpr (sizeof...(Ps) == 1) {
            return [](auto p) { return p; }(to_parser(ps)...);
        }
        else {
            auto parser_decorator = [](auto p) {
                return [=](Rng& r){
                    auto s = r.s;

                    if (p(r)) {
                        return true;
                    }

                    r.s = s;
                    return false;
                };
            };

            return [](auto... decorated_parser) {
                return [=](Rng& r){
                    return (... || decorated_parser(r));
                };
            }(parser_decorator(to_parser(ps))...);
        }
    }

    template<class... Ps>
    auto zero_or_one(Ps... ps)
    {
        return [p = group(ps...)](Rng& r){
            p(r);
            return true;
        };
    }

    template<class... Ps>
    auto one_or_more(Ps... ps)
    {
        return [p = group(ps...)](Rng& r){
            if (p(r)) {
                while (p(r)) {
                }
                return true;
            }
            return false;
        };
    }

    template<std::size_t N, std::size_t M = N, class... Ps>
    auto repeat(Ps... ps)
    {
        return [p = group(ps...)](Rng& r){
            auto s = r.s;
            for (std::size_t i = 0; i < N; ++i) {
                if (!p(r)) {
                    r.s = s;
                    return false;
                }
            }

            for (std::size_t i = N; i < M && p(r); ++i) {
            }

            return true;
        };
    }

    template<class... Ps>
    auto after(Ps... ps)
    {
        return [p = group(ps...)](Rng& r){
            auto s = r.s;
            while (!p(r)) {
                ++r.s;
                if (r.s == r.e) {
                    r.s = s;
                    return false;
                }
            }
            return true;
        };
    }

    template<class F>
    auto is(F f)
    {
        return [f](Rng& r){
            if (f(*r.s)) {
                ++r.s;
                return true;
            }
            return false;
        };
    }

    struct Capture
    {
        char const* start;
        char const* end;

        std::size_t size() const noexcept { return end - start; }

        zstring_view copy(writable_chars_view out) const noexcept
        {
            memcpy(out.data(), this->start, this->size());
            out[this->size()] = '\0';

            return zstring_view::from_null_terminated(out.data(),
                                                      this->size());
        }
    };

    template<class... Ps>
    auto capture(Capture& cap, Ps... ps)
    {
        return [&cap, p = group(ps...)](Rng& r){
            auto s = r.s;
            if (p(r)) {
                cap.start = s;
                cap.end = r.s;
                return true;
            }
            return false;
        };
    }
} // anonymous namespace
} // namespace minipeg

zstring_view parse_ip_conntrack(
    int fd, const char * source, const char * dest, int sport, int dport,
    writable_bytes_view transparent_dest, bool is_ipv6, uint32_t verbose)
{
    assert(source && dest);

    LineBuffer line(fd);
    //"tcp      6 299 ESTABLISHED src=10.10.43.13 dst=10.10.47.93 sport=36699 dport=22 packets=5256 bytes=437137 src=10.10.47.93 dst=10.10.43.13 sport=22 dport=36699 packets=3523 bytes=572101 [ASSURED] mark=0 secmark=0 use=2\n"
    //"tcp      6 431979 ESTABLISHED src=2001:0db8:0000:0000::ff00:0042:8329 dst=2a0d:5d40:888:4176:d999:e759:962:19f sport=41971 dport=3389 packets=96 bytes=10739 src=2a0d:356:888:abcd:d999:957e:333:12a dst=2001:0db8:0000:0000::ff00:0042:8329 sport=3389 dport=41971 packets=96 bytes=39071 [ASSURED] mark=0 secmark=0 use=2\n"

    using namespace minipeg;

    Capture in_ip_dest_cap;

    Capture out_ip_src_cap;
    Capture out_ip_dest_cap;

    auto sport_s = int_to_decimal_chars(sport);
    auto dport_s = int_to_decimal_chars(dport);

    auto sv = [](chars_view av)
    {
        return std::string_view(av.data(), av.size());
    };
    auto is_digit = [](char c)
    {
        return c <= '9' && c >= '0';
    };
    auto is_hex = [](char c)
    {
        return (c <= '9' && c >= '0')
            || (c <= 'F' && c >= 'A')
            || (c <= 'f' && c >= 'a');
    };

    auto digit = is(is_digit);
    auto hex = is(is_hex);
    auto ws = one_or_more(' ');
    auto int_ = one_or_more(digit);

    // IPv4
    auto int3 = repeat<1, 3>(digit);
    auto ipv4 = group(int3, '.', int3, '.', int3, '.', int3);

    // IPv6
    auto hex4 = repeat<1, 4>(hex);

    // "::", "::1" formats
    auto ipv6_f1 = group("::", zero_or_one(hex4));

    // "::ffff:127.0.0.1", "::ffff:255.255.255.255" formats
    auto ipv6_f2 = group(ipv6_f1, ':', ipv4);

    // "fe80::", "2001:db8:3c4d:15:0:d234:3eee::" formats
    auto colon_hex6 = repeat<1, 6>(':', hex4);
    auto ipv6_f3 = group(hex4, zero_or_one(colon_hex6), "::");

    // "2a0d:356:888:abcd:d999:957e:333:12a", "2001:abcd::1234:e4d2" formats
    auto colon2 = repeat<1, 2>(':');
    auto colon_hex7 = repeat<1, 7>(colon2, hex4);
    auto ipv6_f4 = group(hex4, colon_hex7);

    auto ipv6_all = alternative(ipv6_f4, ipv6_f3, ipv6_f2, ipv6_f1);

    // Both IPv4 and IPv6
    auto ip = alternative(ipv4, ipv6_all);

    auto parser = group(
        zero_or_one("ipv", alternative('4', '6'), ws, alternative('2', "10"), ws),
        "tcp", ws, '6', ws, int_, ws, "ESTABLISHED", ws,
        "src=", ip, ws,
        "dst=", capture(in_ip_dest_cap, ip), ws,
        "sport=", int_, ws,
        "dport=", int_, ' ',
        after("src="), capture(out_ip_src_cap, ip), ws,
        "dst=", capture(out_ip_dest_cap, ip), ws,
        "sport=", sv(sport_s), ws,
        "dport=", sv(dport_s), ' ',
        after("[ASSURED]")
    );

    char out_ip_src[INET6_ADDRSTRLEN] = {};
    char out_ip_dest[INET6_ADDRSTRLEN] = {};
    sockaddr_storage source_ss;
    sockaddr_storage dest_ss;

    if (!get_in_addr_from_ip(source_ss, source, is_ipv6)
        || !get_in_addr_from_ip(dest_ss, dest, is_ipv6))
    {
        // source or dest aren't valid IPs
        return zstring_view{};
    }

    int status = line.readline();

    for (; status == 1 ; (line.begin_line = line.eol), (status = line.readline())) {
        if (verbose) {
            fprintf(stderr, "Line: %.*s", line.eol - line.begin_line, &line.buffer[line.begin_line]);
        }

        if (line.eol - line.begin_line < 100) {
            continue;
        }

        const char * s = line.buffer + line.begin_line;

        const bool contains_endl = line.buffer[line.eol-1] == '\n';
        if (contains_endl) {
            line.buffer[line.eol-1] = 0;
        }

        Rng rng{s, line.buffer + (line.eol - 1)};

        if (parser(rng) && in_ip_dest_cap.size() <= transparent_dest.size())
        {
            zstring_view out_ip_src_view = out_ip_src_cap.copy(
                make_writable_array_view(out_ip_src));
            zstring_view out_ip_dest_view = out_ip_dest_cap.copy(
                make_writable_array_view(out_ip_dest));

            if (compare_binary_ip(source_ss,
                                  out_ip_src_view.c_str(),
                                  is_ipv6)
                && compare_binary_ip(dest_ss,
                                     out_ip_dest_view.c_str(),
                                     is_ipv6))
            {
                zstring_view ip = in_ip_dest_cap.copy(
                    transparent_dest.as_chars());

                LOG_IF(verbose, LOG_INFO, "Match found: %s", ip);

                return ip;
            }
        }

        if (contains_endl) {
            line.buffer[line.eol-1] = '\n';
        }
    }
    // transparent ip route not found in ip_conntrack
    return zstring_view{};
}

FILE* popen_conntrack(const char* source_ip, int source_port, int target_port)
{
    char cmd[256];
    sprintf(cmd, "/usr/sbin/conntrack -L -p tcp --src %s --sport %d --dport %d",
            source_ip, source_port, target_port);
    return popen(cmd, "r");
}

bool get_local_ip_address(IpAddress& client_address, int fd) noexcept
{
    union
    {
        sockaddr s;
        sockaddr_in s4;
        sockaddr_in6 s6;
        sockaddr_storage ss;
    } u;
    socklen_t namelen = sizeof(u);

    std::memset(&u, 0, namelen);
    if (::getsockname(fd, &u.s, &namelen) == -1)
    {
        LOG(LOG_ERR, "getsockname failed with errno = %d (%s)",
            errno, strerror(errno));
        return false;
    }

    if (int res = ::getnameinfo(&u.s,
                                sizeof(u.ss),
                                client_address.ip_addr,
                                sizeof(client_address.ip_addr),
                                nullptr,
                                0,
                                NI_NUMERICHOST))
    {
        LOG(LOG_ERR, "getnameinfo failed : %s",
            (res == EAI_SYSTEM) ? ::strerror(errno) : ::gai_strerror(res));
        return false;
    }

    return true;
}

bool find_probe_client(std::string_view probe_client_addresses,
                       zstring_view source_ip,
                       bool is_ipv6)
{
    assert(!source_ip.empty());

    if (!probe_client_addresses.empty())
    {
        sockaddr_storage source_ss;
        static_string<INET6_ADDRSTRLEN> probe_ip_buf;

        auto is_found = [&](chars_view addr){
            return probe_ip_buf.try_assign(addr)
                && compare_binary_ip(source_ss, probe_ip_buf.c_str(), is_ipv6);
        };

        auto find_interface = [](chars_view ip){
            return static_cast<char const*>(memchr(ip.data(), '%', ip.size()));
        };

        if (get_in_addr_from_ip(source_ss, source_ip.c_str(), is_ipv6))
        {
            for (chars_view addr : split_with(probe_client_addresses, ','))
            {
                if (is_found(addr))
                {
                    return true;
                }
            }
        }
        // possibly ipv6 with interface (subnet): fe80::1234:5678:9abc%eth0
        else if (char const* const interface_p1 = find_interface(source_ip))
        {
            if (!probe_ip_buf.try_assign(chars_view(source_ip.c_str(), interface_p1))
             || !get_in_addr_from_ip(source_ss, probe_ip_buf.c_str(), is_ipv6))
            {
                return false;
            }

            auto n1 = static_cast<std::size_t>(source_ip.end() - interface_p1);

            for (chars_view addr : split_with(probe_client_addresses, ','))
            {
                if (char const* interface_p2 = find_interface(addr))
                {
                    auto n2 = static_cast<std::size_t>(addr.end() - interface_p2);
                    if (n1 == n2
                     && is_found({addr.data(), interface_p2})
                     && 0 == memcmp(interface_p1 + 1, interface_p2 + 1, n1 - 1))
                    {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}
