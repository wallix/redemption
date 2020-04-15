#pragma once

#include <netinet/in.h>
#include <string.h>

struct IpAddress
{
    char ip_addr[46];

    static_assert(sizeof(IpAddress::ip_addr) >= INET6_ADDRSTRLEN);

    IpAddress(const char *ip_addr)
    {
        std::strncpy(this->ip_addr, ip_addr, sizeof(this->ip_addr));
    }
};
