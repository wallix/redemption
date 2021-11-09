#pragma once

#include <sys/types.h>
#include <netinet/in.h>
#include "utils/sugar/zstring_view.hpp"

struct IpPort
{
    IpPort() noexcept
    {
        _ip_address[0] = '\0';
    }

    int port() const noexcept
    {
        return _port;
    }

    zstring_view ip_address() const noexcept
    {
        return zstring_view::from_null_terminated(
            _ip_address + _ip_address_offset, _ip_address_len
        );
    }

    bool is_ipv6() const noexcept
    {
        return _is_ipv6;
    }

    struct [[nodiscard]] ErrorMessage
    {
        bool has_errror() const noexcept { return error; }
        const char * error = nullptr;
    };

    ErrorMessage extract_of(sockaddr const& sa, socklen_t socklen) noexcept;

private:
    char _ip_address[INET6_ADDRSTRLEN];
    bool _is_ipv6 = false;
    uint16_t _ip_address_offset = 0;
    uint16_t _ip_address_len = 0;
    uint16_t _port = 0;
};
