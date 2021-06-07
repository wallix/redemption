#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "utils/ip.cpp" // for is_ipv4_mapped_ipv6

#include <cstring>

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>


namespace
{
    IpPort::ErrorMessage ip_port_with_ipv4(IpPort& ip_port, char const* ipv4_addr, uint16_t port)
    {
        union
        {
            sockaddr sa;
            sockaddr_in sin;
        } u;

        std::memset(&u, 0, sizeof(u));
        u.sin.sin_family = AF_INET;
        inet_aton(ipv4_addr, &u.sin.sin_addr);
        u.sin.sin_port = htons(port);

        return ip_port.extract_of(u.sa, sizeof(u.sin));
    }

    IpPort::ErrorMessage ip_port_with_ipv6(IpPort& ip_port, char const* ipv6_addr, uint16_t port)
    {
        union
        {
            sockaddr sa;
            sockaddr_in6 sin6;
        } u;

        std::memset(&u, 0, sizeof(u));
        u.sin6.sin6_family = AF_INET6;
        inet_pton(AF_INET6, ipv6_addr, &u.sin6.sin6_addr);
        u.sin6.sin6_port = htons(port);

        return ip_port.extract_of(u.sa, sizeof(u.sin6));
    }
}

RED_AUTO_TEST_CASE(Test_is_ipv4_mapped_ipv6)
{
    RED_CHECK(is_ipv4_mapped_ipv6("::ffff:192.0.2.128"_av));
    RED_CHECK(!is_ipv4_mapped_ipv6("::ffff:c000:280"_av));
    RED_CHECK(!is_ipv4_mapped_ipv6("2001:db8:85a3::8a2e:370:7334"_av));
}

RED_AUTO_TEST_CASE(Test_IpPort)
{
    IpPort ip_port;

    RED_CHECK_EQ(ip_port_with_ipv4(ip_port, "192.0.2.128", 1234).error, nullptr);
    RED_CHECK(ip_port.ip_address() == "192.0.2.128");
    RED_CHECK(ip_port.port() == 1234);

    RED_CHECK_EQ(ip_port_with_ipv6(ip_port, "::ffff:192.0.2.128", 4242).error, nullptr);
    RED_CHECK(ip_port.ip_address() == "192.0.2.128");
    RED_CHECK(ip_port.port() == 4242);

    RED_CHECK_EQ(ip_port_with_ipv6(ip_port, "2001:db8:85a3::8a2e:370:7334", 1234).error, nullptr);
    RED_CHECK(ip_port.ip_address() == "2001:db8:85a3::8a2e:370:7334");
    RED_CHECK(ip_port.port() == 1234);

    RED_CHECK_EQ(ip_port_with_ipv6(ip_port, "::ffff:c000:280", 4242).error, nullptr);
    RED_CHECK(ip_port.ip_address() == "192.0.2.128");
    RED_CHECK(ip_port.port() == 4242);

    RED_CHECK_EQ(ip_port_with_ipv6(ip_port, "0::ffff:192.0.2.128", 1234).error, nullptr);
    RED_CHECK(ip_port.ip_address() == "192.0.2.128");
    RED_CHECK(ip_port.port() == 1234);
}
