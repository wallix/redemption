#include "utils/ip.hpp"
#include "utils/strutils.hpp"

#include <charconv>

#include <cstring>
#include <cerrno>

#include <sys/socket.h>
#include <netdb.h>


namespace
{
    constexpr auto IPV4_MAPPED_IPV6_PREFIX = "::ffff:"_av;

    bool is_ipv4_mapped_ipv6(chars_view ip_address) noexcept
    {
        if (utils::starts_with(ip_address, IPV4_MAPPED_IPV6_PREFIX)) {
            // differentiated ::ffff:192.0.2.128 of ::ffff:c000:280
            // note: this case may not be possible
            for (char c : ip_address.drop_front(IPV4_MAPPED_IPV6_PREFIX.size())) {
                if (c == '.') {
                    return true;
                }
            }
        }

        return false;
    }
}

IpPort::ErrorMessage IpPort::extract_of(sockaddr const& sa, socklen_t socklen) noexcept
{
    _ip_address_len = 0;

    char dest_port[32];
    if (int res = getnameinfo(&sa, socklen,
                              _ip_address, sizeof(_ip_address),
                              dest_port, sizeof(dest_port),
                              NI_NUMERICHOST | NI_NUMERICSERV))
    {
        // force zero terminated
        _ip_address[0] = '\0';
        return ErrorMessage{(res == EAI_SYSTEM) ? strerror(errno) : gai_strerror(res)};
    }

    std::size_t ip_len = strlen(_ip_address);
    _ip_address_len = checked_int{ip_len};
    _ip_address_offset = 0;
    if (is_ipv4_mapped_ipv6({_ip_address, ip_len})) {
        _ip_address_len -= IPV4_MAPPED_IPV6_PREFIX.size();
        _ip_address_offset += IPV4_MAPPED_IPV6_PREFIX.size();
    }

    [[maybe_unused]]
    auto r = std::from_chars(dest_port, dest_port + strlen(dest_port), _port);
    // assume that dest_port is still valid
    assert(!bool(r.ec));

    return ErrorMessage{};
}
