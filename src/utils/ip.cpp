#include <cstring>
#include <string_view>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>

#include "ip.hpp"

namespace
{
    using namespace std::string_view_literals;
    
    constexpr auto IPV4_MAPPED_IPV6_PREFIX = "::ffff:"sv;
}

bool is_ipv4_mapped_ipv6(const char *ipv6_address) noexcept
{
    unsigned char buf[sizeof(in6_addr)] { };
    
    if (!ipv6_address || inet_pton(AF_INET6, ipv6_address, buf) != 1)
    {
        return false;
    }    
    return std::strncmp(ipv6_address,
                        IPV4_MAPPED_IPV6_PREFIX.data(),
                        IPV4_MAPPED_IPV6_PREFIX.size()) == 0;
}

void get_ipv4_address(const char *ipv6_address,
                      char *dest_ip,
                      std::size_t dest_ip_size) noexcept
{
    const char *ipv4_address = ipv6_address + IPV4_MAPPED_IPV6_PREFIX.size();

    std::strncpy(dest_ip, ipv4_address, dest_ip_size); 
}

const char *get_underlying_ip_port(const sockaddr& sa,
                                   socklen_t socklen,
                                   char *dest_ip,
                                   std::size_t dest_ip_size,
                                   char *dest_port,
                                   std::size_t dest_port_size) noexcept
{
    char ip_address[INET6_ADDRSTRLEN] { };

    if (int res = getnameinfo(&sa,
                              socklen,
                              ip_address,
                              sizeof(ip_address),
                              dest_port,
                              dest_port_size,
                              NI_NUMERICHOST | NI_NUMERICSERV))
    {
        return (res == EAI_SYSTEM) ? strerror(errno) : gai_strerror(res);
    }
    if (is_ipv4_mapped_ipv6(ip_address))
    {
        get_ipv4_address(ip_address, dest_ip, dest_ip_size);
    }
    else
    {
        std::strncpy(dest_ip, ip_address, dest_ip_size);
    }
    return nullptr;
}
