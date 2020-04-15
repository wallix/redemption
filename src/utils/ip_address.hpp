#pragma once

struct IpAddress
{
    char ip_addr[46];

    IpAddress();
    IpAddress(const char *ip_addr);
};
