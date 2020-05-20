#pragma once

#include <cstring>
#include <iterator>
#include <arpa/inet.h>
#include <string_view>

#include "utils/sugar/array_view.hpp"

namespace
{   
    constexpr std::string_view IPV4_MAPPED_IPV6_PREFIX = "::ffff:";
}

/* Need declarations here because of -Wmissing-declarations 
   compilation flag */
[[nodiscard]]
const char *get_ipv4_mapped_ipv6_from_ipv6(const char *) noexcept;

[[nodiscard]]
bool is_ipv6_address(const char *) noexcept;

[[nodiscard]]
bool convert_ipv4_mapped_ipv6_to_ipv6(const char *, array_view_char)
  noexcept;


const char *get_ipv4_mapped_ipv6_from_ipv6(const char *ipv6_address) noexcept
{
    return !std::strncmp(ipv6_address,
                         IPV4_MAPPED_IPV6_PREFIX.data(),
                         IPV4_MAPPED_IPV6_PREFIX.size()) ?
        ipv6_address + IPV4_MAPPED_IPV6_PREFIX.size() : nullptr;
}

bool is_ipv6_address(const char *ipv6_address) noexcept
{
    sockaddr_in6 s6;
    
    return ::inet_pton(AF_INET6, ipv6_address, &s6.sin6_addr) == 1;
}

bool convert_ipv4_mapped_ipv6_to_ipv6(const char *ipv4_address,
                                      array_view_char ipv6_address)
    noexcept
{
    if (sockaddr_in s; ::inet_pton(AF_INET, ipv4_address, &s.sin_addr) != 1)
    {
        return false;
    }
    std::strncat(ipv6_address.data(),
                 IPV4_MAPPED_IPV6_PREFIX.data(),
                 ipv6_address.size());
    std::strncat(ipv6_address.data(),
                 ipv4_address,
                 ipv6_address.size());
    return true;   
}
