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

#include "utils/sugar/unique_fd.hpp"
#include "utils/sugar/bytes_view.hpp"
#include "utils/sugar/zstring_view.hpp"

#include <cstdint>
#include <cstdio>
#include <chrono>
#include <memory>

class in_addr;
class in6_addr;
class addrinfo;
class sockaddr_storage;

struct IpAddress
{
    char ip_addr[46] {};
};

struct AddrInfoDeleter
{
    void operator()(addrinfo *addr_info) noexcept;
};

using AddrInfoPtrWithDel_t = std::unique_ptr<addrinfo, AddrInfoDeleter>;

struct DefaultConnectTag { };

bool try_again(int errnum);

/// std::expected
/// \return nullptr if ok, view string if error
char const* resolve_ipv4_address(const char* ip, in_addr & s4_sin_addr);

unique_fd ip_connect_ipv4(const char* ip,
                          int port,
                          std::chrono::milliseconds establishment_timeout,
                          std::chrono::milliseconds tcp_user_timeout,
                          char const** error_result = nullptr);

unique_fd ip_connect_blocking(const char* ip,
                              int port,
                              std::chrono::milliseconds establishment_timeout,
                              std::chrono::milliseconds tcp_user_timeout,
                              char const** error_result = nullptr);

[[nodiscard]]
AddrInfoPtrWithDel_t resolve_both_ipv4_and_ipv6_address(
    const char *ip, int port, const char **error_result = nullptr) noexcept;

unique_fd ip_connect(const char *ip,
                     int port,
                     DefaultConnectTag default_connect_tag,
                     const char **error_result = nullptr) noexcept;

unique_fd ip_connect(const char* ip,
                     int port,
                     std::chrono::milliseconds establishment_timeout,
                     std::chrono::milliseconds tcp_user_timeout,
                     const char **error_result = nullptr) noexcept;

unique_fd local_connect(const char* sck_name,
                        std::chrono::milliseconds establishment_timeout,
                        bool no_log);

unique_fd addr_connect(const char* addr,
                       std::chrono::milliseconds establishment_timeout,
                       bool no_log_for_unix_socket);

unique_fd addr_connect(const char* addr, int port, bool no_log_for_unix_socket);

unique_fd addr_connect_blocking(
    const char* addr,
    std::chrono::milliseconds establishment_timeout,
    bool no_log_for_unix_socket);

[[nodiscard]]
bool compare_binary_ipv4(const in_addr& in_addr, const char *ipv4);

[[nodiscard]]
bool compare_binary_ipv6(const in6_addr& in6_addr, const char *ipv6);

[[nodiscard]]
bool compare_binary_ip(const sockaddr_storage& ss, const char *ip, bool is_ipv6);

bool get_in_addr_from_ip(sockaddr_storage& ss, const char *ip, bool is_ipv6);

/// \return ip found or empty view whether not found or error
zstring_view parse_ip_conntrack(
    int fd, const char * source, const char * dest, int sport, int dport,
    writable_bytes_view transparent_dest, bool is_ipv6, uint32_t verbose);

FILE* popen_conntrack(const char* source_ip, int source_port, int target_port);

[[nodiscard]]
bool get_local_ip_address(IpAddress& client_address, int fd) noexcept;

[[nodiscard]]
bool find_probe_client(std::string_view probe_client_addresses,
                       zstring_view source_ip,
                       bool is_ipv6);
