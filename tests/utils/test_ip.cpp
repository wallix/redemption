#include <array>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string_view>

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "utils/ip.hpp"

using namespace std::string_view_literals;

template <std::size_t N>
using char_array_t = std::array<char, N>;

// + 1 to be sure to have enough space for null terminated character
using ipv4_address_t = char_array_t<INET_ADDRSTRLEN + 1>;
using ipv6_address_t = char_array_t<INET6_ADDRSTRLEN + 1>;
using port_t = char_array_t<8 + 1>;

RED_AUTO_TEST_CASE(Test_is_ipv4_mapped_ipv6_with_null_ip_address)
{
    constexpr const char *ip_address = nullptr;

    RED_CHECK(!is_ipv4_mapped_ipv6(ip_address));
}

RED_AUTO_TEST_CASE(Test_is_ipv4_mapped_ipv6_with_empty_ip_address)
{
    constexpr const char *ip_address = "";

    RED_CHECK(!is_ipv4_mapped_ipv6(ip_address));
}

RED_AUTO_TEST_CASE(Test_is_ipv4_mapped_ipv6_with_bad_ip_address)
{
    constexpr const char *ip_address = "123456";

    RED_CHECK(!is_ipv4_mapped_ipv6(ip_address));
}

RED_AUTO_TEST_CASE(Test_is_ipv4_mapped_ipv6_with_ipv4_address)
{
    constexpr const char *ip_address = "192.0.2.128";

    RED_CHECK(!is_ipv4_mapped_ipv6(ip_address));
}

RED_AUTO_TEST_CASE(Test_is_ipv4_mapped_ipv6_with_ipv6_address)
{
    constexpr const char *ip_address = "2001:db8:85a3::8a2e:370:7334";

    RED_CHECK(!is_ipv4_mapped_ipv6(ip_address));
}

RED_AUTO_TEST_CASE(Test_is_ipv4_mapped_ipv6_with_ipv4_mapped_ipv6_address)
{
    constexpr const char *ip_address = "::ffff:192.0.2.128";
    
    RED_CHECK(is_ipv4_mapped_ipv6(ip_address));
}

RED_AUTO_TEST_CASE(Test_get_ivp4_address)
{
    constexpr const char *ipv6_address = "::ffff:192.0.2.128";
    ipv4_address_t dest_ipv4 { };

    get_ipv4_address(ipv6_address, dest_ipv4.data(), dest_ipv4.size());
    RED_CHECK_EQUAL(std::string_view(dest_ipv4.data()), "192.0.2.128"sv);
}

RED_AUTO_TEST_CASE(Test_get_underlying_ip_port_with_empty_sockaddr)
{
    sockaddr sa;
    ipv4_address_t dest_ip { };
    port_t dest_port { };

    memset(&sa, 0, sizeof(sa));
    RED_CHECK_NE(get_underlying_ip_port(sa,
                                        sizeof(sa),
                                        dest_ip.data(),
                                        dest_ip.size(),
                                        dest_port.data(),
                                        dest_port.size()),
                 nullptr);
}

RED_AUTO_TEST_CASE(Test_get_underlying_ip_port_with_ipv4_sockaddr)
{    
    union
    {
        sockaddr sa;
        sockaddr_in sin;
    } u;

    memset(&u, 0, sizeof(u));
    u.sin.sin_family = AF_INET;
    inet_aton("192.0.2.128", &u.sin.sin_addr);
    u.sin.sin_port = htons(4242);

    ipv4_address_t dest_ip { };
    port_t dest_port { };

    RED_CHECK_EQUAL(get_underlying_ip_port(u.sa,
                                           sizeof(u.sin),
                                           dest_ip.data(),
                                           dest_ip.size(),
                                           dest_port.data(),
                                           dest_port.size()),
                    nullptr);
    RED_CHECK_EQUAL(std::string_view(dest_ip.data()), "192.0.2.128"sv);
    RED_CHECK_EQUAL(std::string_view(dest_port.data()), "4242"sv);
}

RED_AUTO_TEST_CASE(Test_get_underlying_ip_port_with_ipv6_sockaddr)
{
    union
    {
        sockaddr sa;
        sockaddr_in6 sin6;
    } u;

    memset(&u, 0, sizeof(u));
    u.sin6.sin6_family = AF_INET6;
    inet_pton(AF_INET6, "2001:db8:85a3::8a2e:370:7334", &u.sin6.sin6_addr);
    u.sin6.sin6_port = htons(4242);

    ipv6_address_t dest_ip { };
    port_t dest_port { };

    RED_CHECK_EQUAL(get_underlying_ip_port(u.sa,
                                           sizeof(u.sin6),
                                           dest_ip.data(),
                                           dest_ip.size(),
                                           dest_port.data(),
                                           dest_port.size()),
                    nullptr);
    RED_CHECK_EQUAL(std::string_view(dest_ip.data()),
                    "2001:db8:85a3::8a2e:370:7334"sv);
    RED_CHECK_EQUAL(std::string_view(dest_port.data()),
                    "4242"sv);
}

RED_AUTO_TEST_CASE(Test_get_underlying_ip_port_with_ipv4_mapped_ipv6_sockaddr)
{    
    union
    {
        sockaddr sa;
        sockaddr_in6 sin6;
    } u;
    
    memset(&u, 0, sizeof(u));
    u.sin6.sin6_family = AF_INET6;
    inet_pton(AF_INET6, "::ffff:192.0.2.128", &u.sin6.sin6_addr);
    u.sin6.sin6_port = htons(4242);

    ipv6_address_t dest_ip { };
    port_t dest_port { };
    
    RED_CHECK_EQUAL(get_underlying_ip_port(u.sa,
                                           sizeof(u.sin6),
                                           dest_ip.data(),
                                           dest_ip.size(),
                                           dest_port.data(),
                                           dest_port.size()),
                    nullptr);
    RED_CHECK_EQUAL(std::string_view(dest_ip.data()), "192.0.2.128"sv);
    RED_CHECK_EQUAL(std::string_view(dest_port.data()), "4242"sv);
}
    
