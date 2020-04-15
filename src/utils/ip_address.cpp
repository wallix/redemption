#include <netinet/in.h>
#include <cstring>

#include "ip_address.hpp"

static_assert(sizeof(IpAddress::ip_addr) >= INET6_ADDRSTRLEN);

IpAddress::IpAddress() : ip_addr { }
{ }

IpAddress::IpAddress(const char *ip_addr)
{
    std::strncpy(this->ip_addr, ip_addr, sizeof(this->ip_addr));
}
