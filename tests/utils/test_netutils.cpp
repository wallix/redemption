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

*/

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/working_directory.hpp"

#include "utils/netutils.hpp"

#include <cstdlib>
#include <unistd.h>
#include <string_view>

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

using namespace std::string_view_literals;

RED_AUTO_TEST_CASE(Test_compare_binary_ipv4)
{
    in_addr in_addr;

    RED_CHECK(inet_aton("127.1", &in_addr));
    RED_CHECK(compare_binary_ipv4(in_addr, "127.0.0.1"));

    RED_CHECK(inet_aton("192.0.2.128", &in_addr));
    RED_CHECK(compare_binary_ipv4(in_addr, "192.0.2.128"));

    RED_CHECK(inet_aton("255.6.42.125", &in_addr));
    RED_CHECK(compare_binary_ipv4(in_addr, "255.6.42.125"));

    RED_CHECK(inet_aton("3232235521", &in_addr));
    RED_CHECK(compare_binary_ipv4(in_addr, "192.168.0.1"));

    RED_CHECK(inet_aton("10.20.100.200", &in_addr));
    RED_CHECK(compare_binary_ipv4(in_addr, "10.20.100.200"));

    RED_CHECK(inet_aton("127.300", &in_addr));
    RED_CHECK(compare_binary_ipv4(in_addr, "127.0.1.44"));

    RED_CHECK(inet_aton("55.66.77.88", &in_addr));
    RED_CHECK(compare_binary_ipv4(in_addr, "55.66.77.88"));



    RED_CHECK(inet_aton("32.18.99.22", &in_addr));
    RED_CHECK(!compare_binary_ipv4(in_addr, "58.187.5.221"));

    RED_CHECK(inet_aton("0.0.0.0", &in_addr));
    RED_CHECK(!compare_binary_ipv4(in_addr, "0.0.0.1"));

    RED_CHECK(inet_aton("11.22.33.44", &in_addr));
    RED_CHECK(!compare_binary_ipv4(in_addr, "55.66.77.88"));

    RED_CHECK(inet_aton("192.0.2.128", &in_addr));
    RED_CHECK(!compare_binary_ipv4(in_addr, "255.6.42.125"));

    RED_CHECK(inet_aton("1234", &in_addr));
    RED_CHECK(!compare_binary_ipv4(in_addr, "efgh"));

    RED_CHECK(inet_aton("4242", &in_addr));
    RED_CHECK(!compare_binary_ipv4(in_addr, "1.2.3."));

    RED_CHECK(inet_aton("0", &in_addr));
    RED_CHECK(!compare_binary_ipv4(in_addr, ""));
}

RED_AUTO_TEST_CASE(Test_compare_binary_ipv6)
{
    in6_addr in6_addr;

    RED_CHECK(inet_pton(AF_INET6, "2001:abcd::1234", &in6_addr) == 1);
    RED_CHECK(compare_binary_ipv6(in6_addr,
                                  "2001:abcd::1234"));

    RED_CHECK(inet_pton(AF_INET6, "fe80:0000::", &in6_addr) == 1);
    RED_CHECK(compare_binary_ipv6(in6_addr,
                                  "fe80::"));

    RED_CHECK(inet_pton(AF_INET6, "::1", &in6_addr) == 1);
    RED_CHECK(compare_binary_ipv6(in6_addr,
                                  "0000:0000:0000:0000:0000:0000:0000:0001"));

    RED_CHECK(inet_pton(AF_INET6, "2a0d:5d40:888:4176:d999:e759:0962:019f", &in6_addr) == 1);
    RED_CHECK(compare_binary_ipv6(in6_addr,
                                  "2a0d:5d40:888:4176:d999:e759:962:19f"));

    RED_CHECK(inet_pton(AF_INET6, "2001:0db8:0000:0000:1:ff00:0042:8329", &in6_addr) == 1);
    RED_CHECK(compare_binary_ipv6(in6_addr,
                                  "2001:0db8::0000:1:ff00:0042:8329"));

    RED_CHECK(inet_pton(AF_INET6, "2001:db8:3c4d:15:0:d234:3eee::", &in6_addr) == 1);
    RED_CHECK(compare_binary_ipv6(in6_addr,
                                  "2001:db8:3c4d:15:0:d234:3eee:0000"));

    RED_CHECK(inet_pton(AF_INET6, "::ffff:127.0.0.1", &in6_addr) == 1);
    RED_CHECK(compare_binary_ipv6(in6_addr,
                                  "::ffff:127.0.0.1"));



    RED_CHECK(inet_pton(AF_INET6, "::ffff:255.255.255.255", &in6_addr) == 1);
    RED_CHECK(!compare_binary_ipv6(in6_addr,
                                   "::ffff:127.0.0.1"));

    RED_CHECK(inet_pton(AF_INET6, "2001:abcd::1234", &in6_addr) == 1);
    RED_CHECK(!compare_binary_ipv6(in6_addr,
                                   "fe80::"));

    RED_CHECK(inet_pton(AF_INET6, "2a0d:0356:0888:abcd:d999:957e:0333:012a", &in6_addr) == 1);
    RED_CHECK(!compare_binary_ipv6(in6_addr,
                                   "2001:0db8:0000:0000::ff00:0042:8329"));

    RED_CHECK(inet_pton(AF_INET6, "::1", &in6_addr) == 1);
    RED_CHECK(!compare_binary_ipv6(in6_addr,
                                   "::"));

    RED_CHECK(inet_pton(AF_INET6, "1:2:3:4:5:6:7:8", &in6_addr) == 1);
    RED_CHECK(!compare_binary_ipv6(in6_addr,
                                   "abcdefgh"));

    RED_CHECK(inet_pton(AF_INET6, "1:2:3:4::", &in6_addr) == 1);
    RED_CHECK(!compare_binary_ipv6(in6_addr,
                                   "5:6:7:8"));

    RED_CHECK(inet_pton(AF_INET6, "::", &in6_addr) == 1);
    RED_CHECK(!compare_binary_ipv6(in6_addr,
                                   ""));
}

RED_AUTO_TEST_CASE(Test_find_probe_client)
{
    // ipv4 found
    RED_CHECK(find_probe_client(",,0.0.0.0,,"sv,
                                "0.0.0.0"_zv,
                                false));
    RED_CHECK(find_probe_client("10.20.30.40,1.2.3.4"sv,
                                "10.20.30.40"_zv,
                                false));
    RED_CHECK(find_probe_client("127.0.0.1,,25.50.75.100"sv,
                                "25.50.75.100"_zv,
                                false));
    RED_CHECK(find_probe_client("20.30.40.50,60.70.80.90,99.72.43.13"sv,
                                "99.72.43.13"_zv,
                                false));
    RED_CHECK(find_probe_client("255.255.255.255"sv,
                                "255.255.255.255"_zv,
                                false));

    // ipv4 not found
    RED_CHECK(!find_probe_client("5.6.7.8"sv,
                                 "1.2.3.4"_zv,
                                 false));
    RED_CHECK(!find_probe_client("abcdef,127.0.0.2,,1.2.3.4"sv,
                                 "127.0.0.1"_zv,
                                 false));
    RED_CHECK(!find_probe_client("40.50.60.70"sv,
                                 "70.60.50.40"_zv,
                                 false));
    RED_CHECK(!find_probe_client("00000000000000000000000000000000000000000000000000,,,,,"sv,
                                 "100.75.50.20"_zv,
                                 false));
    RED_CHECK(!find_probe_client(""sv,
                                 "0.10.100.0"_zv,
                                 false));
    // ipv6 found
    RED_CHECK(find_probe_client("fe80::,::1"sv,
                                "0000:0000:0000:0000:0000:0000:0000:0001"_zv,
                                true));
    RED_CHECK(find_probe_client("2001:abcd::1234"sv,
                                "2001:abcd:0000:0000::1234"_zv,
                                true));
    RED_CHECK(find_probe_client("2a0d:0356:0888:abcd:d999:957e:0333:012a"sv,
                                "2a0d:0356:0888:abcd:d999:957e:0333:012a"_zv,
                                true));
    RED_CHECK(find_probe_client(",::ffff:255.255.255.255,2001:abcd::"sv,
                                "::ffff:255.255.255.255"_zv,
                                true));
    RED_CHECK(find_probe_client("2a0d:5d40:0888:4176:d999:e759:0962:019f"sv,
                                "2a0d:5d40:888:4176:d999:e759:962:19f"_zv,
                                true));
    RED_CHECK(find_probe_client("fe80::1234:5678:9abc%eth0"sv,
                                "fe80::1234:5678:9abc%eth0"_zv,
                                true));

    // ipv6 not found
    RED_CHECK(!find_probe_client("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA,,,,,"sv,
                                 "::ffff:127.0.0.1"_zv,
                                 true));
    RED_CHECK(!find_probe_client("2001:0db8:3c4d:0015:0000:0000:1a2f:1a2b",
                                 "0000:0000:0000:0000:0000:0000:0000:0000"_zv,
                                 true));
    RED_CHECK(!find_probe_client("1234,2001:0db8"sv,
                                 "2001:0db8:0000:0000::ff00:0042:8329"_zv,
                                 true));
    RED_CHECK(!find_probe_client("fe80,2001:0db8:3c4d:0015::"sv,
                                 "2001:0db8:3c4d:0015:0000:0000:1a2f:1a2b"_zv,
                                 true));
    RED_CHECK(!find_probe_client(""sv,
                                 "fe80::1234"_zv,
                                 true));
    RED_CHECK(!find_probe_client("fe80::1234:5678:9abc%eth0"sv,
                                 "fe80::1234:5678:9abc%eth1"_zv,
                                 true));

    // ipv4 and ipv6 found
    RED_CHECK(find_probe_client("168.63.129.16,fe80::1234:5678:9abc"sv,
                                "168.63.129.16"_zv,
                                false));
    RED_CHECK(find_probe_client("168.63.129.16,fe80::1234:5678:9abc"sv,
                                "fe80::1234:5678:9abc"_zv,
                                true));
}

RED_AUTO_TEST_CASE(ParseIpConntrack_IPv4)
{
    chars_view conntrack1
      = "unknown  2 580 src=10.10.43.13 dst=224.0.0.251 packets=2 bytes=64 [UNREPLIED] src=224.0.0.251 dst=10.10.43.13 packets=0 bytes=0 mark=0 secmark=0 use=2\n"
        "udp      17 28 src=10.10.43.30 dst=255.255.255.255 sport=17500 dport=17500 packets=1102 bytes=154280 [UNREPLIED] src=255.255.255.255 dst=10.10.43.30 sport=17500 dport=17500 packets=0 bytes=0 mark=0 secmark=0 use=2\n"
        "unknown  2 205 src=10.10.47.124 dst=224.0.0.251 packets=2 bytes=64 [UNREPLIED] src=224.0.0.251 dst=10.10.47.124 packets=0 bytes=0 mark=0 secmark=0 use=2\n"
        "unknown  2 209 src=10.10.41.13 dst=224.0.0.251 packets=5 bytes=160 [UNREPLIED] src=224.0.0.251 dst=10.10.41.13 packets=0 bytes=0 mark=0 secmark=0 use=2\n"
        "udp      17 19 src=10.10.41.14 dst=10.10.47.255 sport=17500 dport=17500 packets=5937 bytes=831180 [UNREPLIED] src=10.10.47.255 dst=10.10.41.14 sport=17500 dport=17500 packets=0 bytes=0 mark=0 secmark=0 use=2\n"
        "udp      17 19 src=10.10.41.14 dst=255.255.255.255 sport=17500 dport=17500 packets=5937 bytes=831180 [UNREPLIED] src=255.255.255.255 dst=10.10.41.14 sport=17500 dport=17500 packets=0 bytes=0 mark=0 secmark=0 use=2\n"
        "udp      17 29 src=10.10.43.33 dst=10.10.47.255 sport=17500 dport=17500 packets=971 bytes=135940 [UNREPLIED] src=10.10.47.255 // dst=10.10.43.33 sport=17500 dport=17500 packets=0 bytes=0 mark=0 secmark=0 use=2\n"
        "udp      17 29 src=10.10.43.33 dst=255.255.255.255 sport=17500 dport=17500 packets=971 bytes=135940 [UNREPLIED] src=255.255.255.255 dst=10.10.43.33 sport=17500 dport=17500 packets=0 bytes=0 mark=0 secmark=0 use=2\n"
        "unknown  2 455 src=10.10.47.170 dst=224.0.0.251 packets=1 bytes=32 [UNREPLIED] src=224.0.0.251 dst=10.10.47.170 packets=0 bytes=0 mark=0 secmark=0 use=2\n"
        "unknown  2 580 src=0.0.0.0 dst=224.0.0.1 packets=1422 bytes=45504 [UNREPLIED] src=224.0.0.1 dst=0.0.0.0 packets=0 bytes=0 mark=0 secmark=0 use=2\n"
        "tcp      6 431995 ESTABLISHED src=127.0.0.1 dst=127.0.0.1 sport=51040 dport=3450 packets=5 bytes=588 src=127.0.0.1 dst=127.0.0.1 sport=3450 dport=51040 packets=4 bytes=573 [ASSURED] mark=0 secmark=0 use=2\n"
        "udp      17 28 src=10.10.43.30 dst=10.10.47.255 sport=17500 dport=17500 packets=1102 bytes=154280 [UNREPLIED] src=10.10.47.255 dst=10.10.43.30 sport=17500 dport=17500 packets=0 bytes=0 mark=0 secmark=0 use=2\n"
        "unknown  2 79 src=10.10.45.11 dst=224.0.0.251 packets=1 bytes=32 [UNREPLIED] src=224.0.0.251 dst=10.10.45.11 packets=0 bytes=0 mark=0 secmark=0 use=2\n"
        "tcp      6 431979 ESTABLISHED src=10.10.43.13 dst=10.10.46.78 sport=41971 dport=3389 packets=96 bytes=10739 src=10.10.47.93 dst=10.10.43.13 sport=3389 dport=41971 packets=96 bytes=39071 [ASSURED] mark=0 secmark=0 use=2\n"
        "udp      17 0 src=10.10.41.14 dst=10.10.47.255 sport=631 dport=631 packets=1 bytes=274 [UNREPLIED] src=10.10.47.255 dst=10.10.41.14 sport=631 dport=631 packets=0 bytes=0 mark=0 secmark=0 use=2\n"
        "tcp      6 299 ESTABLISHED src=10.10.43.13 dst=10.10.47.93 sport=36699 dport=22 packets=5256 bytes=437137 src=10.10.47.93 dst=10.10.43.13 sport=22 dport=36699 packets=3523 bytes=572101 [ASSURED] mark=0 secmark=0 use=2\n"
        "tcp      6 431997 ESTABLISHED src=10.10.43.13 dst=10.10.47.93 sport=46392 dport=3389 packets=90 bytes=10061 src=10.10.47.93 dst=10.10.43.13 sport=3389 dport=46392 packets=89 bytes=38707 [ASSURED] mark=0 secmark=0 use=2\n"
        "tcp      6 431997 ESTABLISHED src=255.255.255.255 dst=0.0.0.0 sport=26542 dport=4242 packets=90 bytes=10061 src=0.0.0.0 dst=255.255.255.255 sport=4242 dport=26542 packets=89 bytes=38707 [ASSURED] mark=0 secmark=0 use=2\n"
        "tcp      6 431997 ESTABLISHED src=16.20.32.20 dst=137.253.0.4 sport=12345 dport=2424 packets=90 bytes=10061 src=127.0.0.1 dst=16.20.32.20 sport=2424 dport=12345 packets=89 bytes=38707 [ASSURED] mark=0 secmark=0 use=2\n"
        "udp      17 0 src=10.10.43.31 dst=10.10.47.255 sport=57621 dport=57621 packets=1139 bytes=82008 [UNREPLIED] src=10.10.47.255 dst=10.10.43.31 sport=57621 dport=57621 packets=0 bytes=0 mark=0 secmark=0 use=2\n"
        "tcp      6 431997 ESTABLISHED src=5678 dst=1234 sport=56789 dport=1234 packets=90 bytes=10061 src=1234 dst=5678 sport=1234 dport=5678 packets=89 bytes=38707 [ASSURED] mark=0 secmark=0 use=2\n"
        "tcp      6 431997 ESTABLISHED src=4.5 dst=1.2.3. sport=56789 dport=1234 packets=90 bytes=10061 src=1.2.3. dst=4.5 sport=1234 dport=5678 packets=89 bytes=38707 [ASSURED] mark=0 secmark=0 use=2\n"
        "tcp      6 431997 ESTABLISHED src=. dst=127..0..1..1 sport=56789 dport=1234 packets=90 bytes=10061 src=127..0..0..1 dst=. sport=1234 dport=5678 packets=89 bytes=38707 [ASSURED] mark=0 secmark=0 use=2\n"_av;

    chars_view conntrack2
      = "unknown  2 580 src=10.10.43.13 dst=224.0.0.251 [UNREPLIED] src=224.0.0.251 dst=10.10.43.13 mark=0 secmark=0 use=2\n"
        "udp      17 28 src=10.10.43.30 dst=255.255.255.255 sport=17500 dport=17500 [UNREPLIED] src=255.255.255.255 dst=10.10.43.30 sport=17500 dport=17500 mark=0 secmark=0 use=2\n"
        "unknown  2 205 src=10.10.47.124 dst=224.0.0.251 [UNREPLIED] src=224.0.0.251 dst=10.10.47.124 mark=0 secmark=0 use=2\n"
        "unknown  2 209 src=10.10.41.13 dst=224.0.0.251 [UNREPLIED] src=224.0.0.251 dst=10.10.41.13 mark=0 secmark=0 use=2\n"
        "udp      17 19 src=10.10.41.14 dst=10.10.47.255 sport=17500 dport=17500 [UNREPLIED] src=10.10.47.255 dst=10.10.41.14 sport=17500 dport=17500 mark=0 secmark=0 use=2\n"
        "udp      17 19 src=10.10.41.14 dst=255.255.255.255 sport=17500 dport=17500 [UNREPLIED] src=255.255.255.255 dst=10.10.41.14 sport=17500 dport=17500 mark=0 secmark=0 use=2\n"
        "udp      17 29 src=10.10.43.33 dst=10.10.47.255 sport=17500 dport=17500 [UNREPLIED] src=10.10.47.255 // dst=10.10.43.33 sport=17500 dport=17500 mark=0 secmark=0 use=2\n"
        "udp      17 29 src=10.10.43.33 dst=255.255.255.255 sport=17500 dport=17500 [UNREPLIED] src=255.255.255.255 dst=10.10.43.33 sport=17500 dport=17500 mark=0 secmark=0 use=2\n"
        "unknown  2 455 src=10.10.47.170 dst=224.0.0.251 [UNREPLIED] src=224.0.0.251 dst=10.10.47.170 mark=0 secmark=0 use=2\n"
        "unknown  2 580 src=0.0.0.0 dst=224.0.0.1 [UNREPLIED] src=224.0.0.1 dst=0.0.0.0 mark=0 secmark=0 use=2\n"
        "tcp      6 431995 ESTABLISHED src=127.0.0.1 dst=127.0.0.1 sport=51040 dport=3450 src=127.0.0.1 dst=127.0.0.1 sport=3450 dport=51040 [ASSURED] mark=0 secmark=0 use=2\n"
        "udp      17 28 src=10.10.43.30 dst=10.10.47.255 sport=17500 dport=17500 [UNREPLIED] src=10.10.47.255 dst=10.10.43.30 sport=17500 dport=17500 mark=0 secmark=0 use=2\n"
        "unknown  2 79 src=10.10.45.11 dst=224.0.0.251 [UNREPLIED] src=224.0.0.251 dst=10.10.45.11 mark=0 secmark=0 use=2\n"
        "tcp      6 431979 ESTABLISHED src=10.10.43.13 dst=10.10.46.78 sport=41971 dport=3389 src=10.10.47.93 dst=10.10.43.13 sport=3389 dport=41971 [ASSURED] mark=0 secmark=0 use=2\n"
        "udp      17 0 src=10.10.41.14 dst=10.10.47.255 sport=631 dport=631 [UNREPLIED] src=10.10.47.255 dst=10.10.41.14 sport=631 dport=631 mark=0 secmark=0 use=2\n"
        "tcp      6 299 ESTABLISHED src=10.10.43.13 dst=10.10.47.93 sport=36699 dport=22 src=10.10.47.93 dst=10.10.43.13 sport=22 dport=36699 [ASSURED] mark=0 secmark=0 use=2\n"
        "tcp      6 431997 ESTABLISHED src=10.10.43.13 dst=10.10.47.93 sport=46392 dport=3389 src=10.10.47.93 dst=10.10.43.13 sport=3389 dport=46392 [ASSURED] mark=0 secmark=0 use=2\n"
        "tcp      6 431997 ESTABLISHED src=255.255.255.255 dst=0.0.0.0 sport=26542 dport=4242 src=0.0.0.0 dst=255.255.255.255 sport=4242 dport=26542 [ASSURED] mark=0 secmark=0 use=2\n"
        "tcp      6 431997 ESTABLISHED src=16.20.32.20 dst=137.253.0.4 sport=12345 dport=2424 src=127.0.0.1 dst=16.20.32.20 sport=2424 dport=12345 [ASSURED] mark=0 secmark=0 use=2\n"
        "udp      17 0 src=10.10.43.31 dst=10.10.47.255 sport=57621 dport=57621 [UNREPLIED] src=10.10.47.255 dst=10.10.43.31 sport=57621 dport=57621 mark=0 secmark=0 use=2\n"
        "tcp      6 431997 ESTABLISHED src=5678 dst=1234 sport=56789 dport=1234 src=1234 dst=5678 sport=1234 dport=5678 [ASSURED] mark=0 secmark=0 use=2\n"
        "tcp      6 431997 ESTABLISHED src=4.5 dst=1.2.3. sport=56789 dport=1234 src=1.2.3. dst=4.5 sport=1234 dport=5678 [ASSURED] mark=0 secmark=0 use=2\n"
        "tcp      6 431997 ESTABLISHED src=. dst=127..0..1..1 sport=56789 dport=1234 src=127..0..0..1 dst=. sport=1234 dport=5678 [ASSURED] mark=0 secmark=0 use=2\n"_av;

    std::string prefix =  "ipv4     2 ";
    std::string conntrack3 = prefix;
    for (char c: conntrack1) {
        conntrack3 += c;
        if (c == '\n') {
            conntrack3 += prefix;
        }
    }
    conntrack3.resize(conntrack3.size() - prefix.size());

    std::string conntrack4 = prefix;
    for (char c: conntrack2) {
        conntrack4 += c;
        if (c == '\n') {
            conntrack4 += prefix;
        }
    }
    conntrack4.resize(conntrack4.size() - prefix.size());

    struct D {
        char const* name;
        chars_view data;
    };
    RED_TEST_CONTEXT_DATA(D const& d, d.name, {
        // "tcp      6 431979 ESTABLISHED src=10.10.43.13 dst=10.10.46.78 sport=41971 dport=3389 packets=96 bytes=10739 src=10.10.47.93 dst=10.10.43.13 sport=3389 dport=41971 packets=96 bytes=39071 [ASSURED] mark=0 secmark=0 use=2\n"
        D{"test1", conntrack1},
        // "tcp      6 431979 ESTABLISHED src=10.10.43.13 dst=10.10.46.78 sport=41971 dport=3389 src=10.10.47.93 dst=10.10.43.13 sport=3389 dport=41971 [ASSURED] mark=0 secmark=0 use=2\n"
        D{"test2", conntrack2},
        D{"test3", conntrack3},
        D{"test4", conntrack4}
    })
    {
        WorkingFile wf(d.name);
        int fd = ::open(wf.c_str(), O_RDWR|O_CREAT, 0777);
        RED_CHECK(ssize_t(d.data.size()) == write(fd,
                                                  d.data.data(),
                                                  d.data.size()));
        RED_CHECK(0 == lseek(fd, 0, SEEK_SET));
        char transparent_target[256];

        RED_CHECK("10.10.46.78"sv == parse_ip_conntrack(
                        fd,
                        "10.10.47.93",
                        "10.10.43.13",
                        3389,
                        41971,
                        make_writable_array_view(transparent_target),
                        false,
                        0)
                  .to_sv());

        RED_CHECK(0 == lseek(fd, 0, SEEK_SET));
        transparent_target[0] = 0;
        RED_CHECK("0.0.0.0"sv == parse_ip_conntrack(
                        fd,
                        "0.0.0.0",
                        "255.255.255.255",
                        4242,
                        26542,
                        make_writable_array_view(transparent_target),
                        false,
                        0)
                  .to_sv());

        RED_CHECK(0 == lseek(fd, 0, SEEK_SET));
        transparent_target[0] = 0;
        RED_CHECK("137.253.0.4"sv == parse_ip_conntrack(
                        fd,
                        "127.0.0.1",
                        "16.20.32.20",
                        2424,
                        12345,
                        make_writable_array_view(transparent_target),
                        false,
                        0)
                  .to_sv());

        RED_CHECK(0 == lseek(fd, 0, SEEK_SET));
        transparent_target[0] = 0;
        RED_CHECK(""sv == parse_ip_conntrack(
                        fd,
                        "10.10.47.21",
                        "10.10.43.13",
                        3389,
                        46392,
                        make_writable_array_view(transparent_target),
                        false,
                        0)
                  .to_sv());

        RED_CHECK(0 == lseek(fd, 0, SEEK_SET));
        transparent_target[0] = 0;
        RED_CHECK(""sv == parse_ip_conntrack(
                        fd,
                        "1234",
                        "5678",
                        1234,
                        56789,
                        make_writable_array_view(transparent_target),
                        false,
                        0)
                  .to_sv());

        RED_CHECK(0 == lseek(fd, 0, SEEK_SET));
        transparent_target[0] = 0;
        RED_CHECK(""sv == parse_ip_conntrack(
                        fd,
                        "1.2.3.",
                        "4.5",
                        1234,
                        56789,
                        make_writable_array_view(transparent_target),
                        false,
                        0)
                  .to_sv());

        RED_CHECK(0 == lseek(fd, 0, SEEK_SET));
        transparent_target[0] = 0;
        RED_CHECK(""sv == parse_ip_conntrack(
                        fd,
                        "127..0..0..1",
                        ".",
                        1234,
                        56789,
                        make_writable_array_view(transparent_target),
                        false,
                        0)
                  .to_sv());

        close(fd);
    }
}

RED_AUTO_TEST_CASE(ParseIpConntrack_IPv6)
{
    chars_view conntrack1
      = "unknown  2 580 src=2001:A304:6101:1::E0:F726:4E58 dst=::1 packets=2 bytes=64 [UNREPLIED] src=1924:D122:6101:13::EA:F206:4EE8 dst=fe80:: packets=0 bytes=0 mark=0 secmark=0 use=2\n"
        "udp      17 28 src=2001:0620:0000:0000:0211:24FF:FE80:C12C dst=fe80::20e:cff:fe3b:883c sport=17500 dport=17500 packets=1102 bytes=154280 [UNREPLIED] src=fe80::20e:cff:fe3b:883c dst=2001:0620:0000:0000:0211:24FF:FE80:C12C sport=17500 dport=17500 packets=0 bytes=0 mark=0 secmark=0 use=2\n"
        "unknown  2 205 src=2020:3D56:8882:abcd:d111:412C:666:12b dst=2001:0db8:85a3:0000:0000:8a2e:0370:7334 packets=2 bytes=64 [UNREPLIED] src=2001:0db8:85a3:0000:0000:8a2e:0370:7334 dst=2020:3D56:8882:abcd:d111:412C:666:12b packets=0 bytes=0 mark=0 secmark=0 use=2\n"
        "unknown  2 209 src=2001:db8:0:0:0:0:2:1 dst=2001:0620:0000:0000:0211:24FF:FE80:FFFF packets=5 bytes=160 [UNREPLIED] src=2001:db8:0:0:0:0:2:1 dst=2001:0620:0000:0000:0211:24FF:FE80:FFFF packets=0 bytes=0 mark=0 secmark=0 use=2\n"
        "udp      17 19 src=2a0d:356:888:abcd:d999:957e:333:12a  dst=2001:0db8:0000:0000:0000:ff00:0042:8329 sport=17500 dport=6042 packets=5937 bytes=831180 [UNREPLIED] src=2001:0db8:0000:0000:0000:ff00:0042:8329 dst=2a0d:356:888:abcd:d999:957e:333:12a sport=6042 dport=17500 packets=0 bytes=0 mark=0 secmark=0 use=2\n"
        "udp      17 19 src=2002::1234:abcd:ffff:c0a8:101 dst=2002::1234:abcd:ffff:c0a8:101 sport=17500 dport=17500 packets=5937 bytes=831180 [UNREPLIED] src=2002::1234:abcd:ffff:c0a8:101 dst=2002::1234:abcd:ffff:c0a8:101 sport=17500 dport=17500 packets=0 bytes=0 mark=0 secmark=0 use=2\n"
        "udp      17 29 src=0000:: dst=F800:: sport=17500 dport=17500 packets=971 bytes=135940 [UNREPLIED] src=F800:: // dst=0000:: sport=17500 dport=17500 packets=0 bytes=0 mark=0 secmark=0 use=2\n"
        "udp      17 29 src=2001:db8:1234::f350:2256:f3dd dst=2001:db8:1234::f350:2256:f3dd sport=17500 dport=17500 packets=971 bytes=135940 [UNREPLIED] src=0000:: dst=f800:: sport=17500 dport=17500 packets=0 bytes=0 mark=0 secmark=0 use=2\n"
        "unknown  2 455 src=2001:db8:1234::f350:2256:f3dd dst=2001:A304:6101:1::E0:F726:4E58 packets=1 bytes=32 [UNREPLIED] src=001:0620:0000:0000:0211:24FF:FE80:C12C dst=2001:db8:1234::f350:2256:f3dd packets=0 bytes=0 mark=0 secmark=0 use=2\n"
        "unknown  2 580 src=::1 dst=9542:413A:: packets=1422 bytes=45504 [UNREPLIED] src=9542:413A:: dst=::1 packets=0 bytes=0 mark=0 secmark=0 use=2\n"
        "tcp      6 431995 ESTABLISHED src=::1 dst=::1 sport=51040 dport=3450 packets=5 bytes=588 src=::1 dst=::1 sport=3450 dport=51040 packets=4 bytes=573 [ASSURED] mark=0 secmark=0 use=2\n"
        "udp      17 28 src=5ABC::3A09:DD dst=9999::AAAA:DD sport=17500 dport=17500 packets=1102 bytes=154280 [UNREPLIED] src=9999::AAAA:DD dst=5ABC::3A09:DD sport=17500 dport=17500 packets=0 bytes=0 mark=0 secmark=0 use=2\n"
        "unknown  2 79 src=ABCD:EF12:3456:: dst=ABCD:EF12:3456:: packets=1 bytes=32 [UNREPLIED] src=ABCD:EF12:3456:: dst=1234:: packets=0 bytes=0 mark=0 secmark=0 use=2\n"
        "tcp      6 431979 ESTABLISHED src=2001:0db8:0000:0000::ff00:0042:8329 dst=2a0d:5d40:888:4176:d999:e759:962:19f sport=41971 dport=3389 packets=96 bytes=10739 src=2a0d:356:888:abcd:d999:957e:333:12a dst=2001:0db8:0000:0000::ff00:0042:8329 sport=3389 dport=41971 packets=96 bytes=39071 [ASSURED] mark=0 secmark=0 use=2\n"
        "udp      17 0 src=1234:5678::9 dst=1234:5678::9 sport=631 dport=631 packets=1 bytes=274 [UNREPLIED] src=fe80:: dst=1234:5678::9 sport=631 dport=631 packets=0 bytes=0 mark=0 secmark=0 use=2\n"
        "tcp      6 299 ESTABLISHED src=::ffff:255.255.255.255 dst=::ffff:127.0.0.1 sport=36699 dport=22 packets=5256 bytes=437137 src=::ffff:127.0.0.1 dst=::ffff:255.255.255.255 sport=22 dport=36699 packets=3523 bytes=572101 [ASSURED] mark=0 secmark=0 use=2\n"
        "tcp      6 431997 ESTABLISHED src=fe80:: dst=2001:abcd::1234 sport=46392 dport=3389 packets=90 bytes=10061 src=2001:abcd::1234 dst=fe80:: sport=3389 dport=46392 packets=89 bytes=38707 [ASSURED] mark=0 secmark=0 use=2\n"
        "tcp      6 431997 ESTABLISHED src=:: dst=::1 sport=4242 dport=12000 packets=90 bytes=10061 src=::1 dst=:: sport=12000 dport=4242 packets=89 bytes=38707 [ASSURED] mark=0 secmark=0 use=2\n"
        "udp      17 0 src=4242:4242:4242:4242:4242:4242:4242:4242 dst=1111:2222:3333:4444:5555:6666:7777:8888 sport=57621 dport=57621 packets=1139 bytes=82008 [UNREPLIED] src=1111:2222:3333:4444:5555:6666:7777:8888 dst=4242:4242:4242:4242:4242:4242:4242:4242 sport=57621 dport=57621 packets=0 bytes=0 mark=0 secmark=0 use=2\n"
        "tcp      6 431997 ESTABLISHED src=5678 dst=1234 sport=56789 dport=1234 packets=90 bytes=10061 src=1234 dst=5678 sport=1234 dport=56789 packets=89 bytes=38707 [ASSURED] mark=0 secmark=0 use=2\n"
        "tcp      6 431997 ESTABLISHED src=abcd:: dst=fe80::: sport=56789 dport=1234 packets=90 bytes=10061 src=fe80::: dst=abcd:: sport=1234 dport=56789 packets=89 bytes=38707 [ASSURED] mark=0 secmark=0 use=2\n"
        "tcp      6 431997 ESTABLISHED src=4242:4242:4242:4242:4242 dst=2001:abcd:1234 sport=56789 dport=1234 packets=90 bytes=10061 src=2001:abcd:1234 dst=4242:4242:4242:4242:4242 sport=1234 dport=56789 packets=89 bytes=38707 [ASSURED] mark=0 secmark=0 use=2\n"
        "tcp      6 431997 ESTABLISHED src=::1 dst=2001:abcd:1234::4242:: sport=56789 dport=1234 packets=90 bytes=10061 src=2001:abcd:1234::4242:: dst=::1 sport=1234 dport=56789 packets=89 bytes=38707 [ASSURED] mark=0 secmark=0 use=2\n"_av;

    chars_view conntrack2
      = "unknown  2 580 src=2001:A304:6101:1::E0:F726:4E58 dst=::1 [UNREPLIED] src=1924:D122:6101:13::EA:F206:4EE8 dst=fe80:: mark=0 secmark=0 use=2\n"
        "udp      17 28 src=2001:0620:0000:0000:0211:24FF:FE80:C12C dst=fe80::20e:cff:fe3b:883c sport=17500 dport=17500 [UNREPLIED] src=fe80::20e:cff:fe3b:883c dst=2001:0620:0000:0000:0211:24FF:FE80:C12C sport=17500 dport=17500 mark=0 secmark=0 use=2\n"
        "unknown  2 205 src=2020:3D56:8882:abcd:d111:412C:666:12b dst=2001:0db8:85a3:0000:0000:8a2e:0370:7334 [UNREPLIED] src=2001:0db8:85a3:0000:0000:8a2e:0370:7334 dst=2020:3D56:8882:abcd:d111:412C:666:12b mark=0 secmark=0 use=2\n"
        "unknown  2 209 src=2001:db8:0:0:0:0:2:1 dst=2001:0620:0000:0000:0211:24FF:FE80:FFFF [UNREPLIED] src=2001:db8:0:0:0:0:2:1 dst=2001:0620:0000:0000:0211:24FF:FE80:FFFF mark=0 secmark=0 use=2\n"
        "udp      17 19 src=2a0d:356:888:abcd:d999:957e:333:12a  dst=2001:0db8:0000:0000:0000:ff00:0042:8329 sport=17500 dport=17500 [UNREPLIED] src=2001:0db8:0000:0000:0000:ff00:0042:8329 dst=2a0d:356:888:abcd:d999:957e:333:12a sport=17500 dport=17500 mark=0 secmark=0 use=2\n"
        "udp      17 19 src=2002::1234:abcd:ffff:c0a8:101 dst=2002::1234:abcd:ffff:c0a8:101 sport=17500 dport=17500 [UNREPLIED] src=2002::1234:abcd:ffff:c0a8:101 dst=2002::1234:abcd:ffff:c0a8:101 sport=17500 dport=17500 mark=0 secmark=0 use=2\n"
        "udp      17 29 src=0000:: dst=F800:: sport=17500 dport=17500 [UNREPLIED] src=F800:: // dst=0000:: sport=17500 dport=17500 mark=0 secmark=0 use=2\n"
        "udp      17 29 src=2001:db8:1234::f350:2256:f3dd dst=2001:db8:1234::f350:2256:f3dd sport=17500 dport=17500 [UNREPLIED] src=0000:: dst=f800:: sport=17500 dport=17500 mark=0 secmark=0 use=2\n"
        "unknown  2 455 src=2001:db8:1234::f350:2256:f3dd dst=2001:A304:6101:1::E0:F726:4E58 [UNREPLIED] src=001:0620:0000:0000:0211:24FF:FE80:C12C dst=2001:db8:1234::f350:2256:f3dd mark=0 secmark=0 use=2\n"
        "unknown  2 580 src=::1 dst=9542:413A:: [UNREPLIED] src=9542:413A:: dst=::1 mark=0 secmark=0 use=2\n"
        "tcp      6 431995 ESTABLISHED src=::1 dst=::1 sport=51040 dport=3450 src=::1 dst=::1 sport=3450 dport=51040 [ASSURED] mark=0 secmark=0 use=2\n"
        "udp      17 28 src=5ABC::3A09:DD dst=9999::AAAA:DD sport=17500 dport=17500 [UNREPLIED] src=9999::AAAA:DD dst=5ABC::3A09:DD sport=17500 dport=17500 mark=0 secmark=0 use=2\n"
        "unknown  2 79 src=ABCD:EF12:3456:: dst=ABCD:EF12:3456:: [UNREPLIED] src=ABCD:EF12:3456:: dst=1234:: mark=0 secmark=0 use=2\n"
        "tcp      6 431979 ESTABLISHED src=2001:0db8:0000:0000::ff00:0042:8329 dst=2a0d:5d40:888:4176:d999:e759:962:19f sport=41971 dport=3389 src=2a0d:356:888:abcd:d999:957e:333:12a dst=2001:0db8:0000:0000::ff00:0042:8329 sport=3389 dport=41971 [ASSURED] mark=0 secmark=0 use=2\n"
        "udp      17 0 src=1234:5678::9 dst=1234:5678::9 sport=631 dport=631 [UNREPLIED] src=fe80:: dst=1234:5678::9 sport=631 dport=631 mark=0 secmark=0 use=2\n"
        "tcp      6 299 ESTABLISHED src=::ffff:255.255.255.255 dst=::ffff:127.0.0.1 sport=36699 dport=22 src=::ffff:127.0.0.1 dst=::ffff:255.255.255.255 sport=22 dport=36699 [ASSURED] mark=0 secmark=0 use=2\n"
        "tcp      6 431997 ESTABLISHED src=fe80:: dst=2001:abcd::1234 sport=46392 dport=3389 src=2001:abcd::1234 dst=fe80:: sport=3389 dport=46392 [ASSURED] mark=0 secmark=0 use=2\n"
        "tcp      6 431997 ESTABLISHED src=:: dst=::1 sport=4242 dport=12000 src=::1 dst=:: sport=12000 dport=4242 [ASSURED] mark=0 secmark=0 use=2\n"
        "udp      17 0 src=4242:4242:4242:4242:4242:4242:4242:4242 dst=1111:2222:3333:4444:5555:6666:7777:8888 sport=57621 dport=57621 [UNREPLIED] src=1111:2222:3333:4444:5555:6666:7777:8888 dst=4242:4242:4242:4242:4242:4242:4242:4242 sport=57621 dport=57621 mark=0 secmark=0 use=2\n"
        "tcp      6 431997 ESTABLISHED src=5678 dst=1234 sport=56789 dport=1234 src=1234 dst=5678 sport=1234 dport=56789 [ASSURED] mark=0 secmark=0 use=2\n"
        "tcp      6 431997 ESTABLISHED src=abcd:: dst=fe80::: sport=56789 dport=1234 src=fe80::: dst=abcd:: sport=1234 dport=56789 [ASSURED] mark=0 secmark=0 use=2\n"
        "tcp      6 431997 ESTABLISHED src=4242:4242:4242:4242:4242 dst=2001:abcd:1234 sport=56789 dport=1234 src=2001:abcd:1234 dst=4242:4242:4242:4242:4242 sport=1234 dport=56789 [ASSURED] mark=0 secmark=0 use=2\n"
        "tcp      6 431997 ESTABLISHED src=::1 dst=2001:abcd:1234::4242:: sport=56789 dport=1234 src=2001:abcd:1234::4242:: dst=::1 sport=1234 dport=56789 [ASSURED] mark=0 secmark=0 use=2\n"_av;

    std::string prefix =  "ipv6     10 ";
    std::string conntrack3 = prefix;
    for (char c: conntrack1) {
        conntrack3 += c;
        if (c == '\n') {
            conntrack3 += prefix;
        }
    }
    conntrack3.resize(conntrack3.size() - prefix.size());

    std::string conntrack4 = prefix;
    for (char c: conntrack2) {
        conntrack4 += c;
        if (c == '\n') {
            conntrack4 += prefix;
        }
    }
    conntrack4.resize(conntrack4.size() - prefix.size());

    struct D {
        char const* name;
        chars_view data;
    };
    RED_TEST_CONTEXT_DATA(D const& d, d.name, {
        //  "tcp      6 431979 ESTABLISHED src=2001:0db8:0000:0000:0000:ff00:0042:8329 dst=2a0d:5d40:888:4176:d999:e759:962:19f sport=41971 dport=3389 packets=96 bytes=10739 src=2a0d:356:888:abcd:d999:957e:333:12a dst=2001:0db8:0000:0000:0000:ff00:0042:8329 sport=3389 dport=41971 packets=96 bytes=39071 [ASSURED] mark=0 secmark=0 use=2\n"
        D{"test1", conntrack1},
        //  "tcp      6 431979 ESTABLISHED src=2001:0db8:0000:0000::ff00:0042:8329 dst=2a0d:5d40:888:4176:d999:e759:962:19f sport=41971 dport=3389 src=2a0d:356:888:abcd:d999:957e:333:12a dst=2001:0db8:0000:0000::ff00:0042:8329 sport=3389 dport=41971 [ASSURED] mark=0 secmark=0 use=2\n"
        D{"test2", conntrack2},
        D{"test3", conntrack3},
        D{"test4", conntrack4}
    })
    {
        WorkingFile wf(d.name);
        int fd = ::open(wf.c_str(), O_RDWR|O_CREAT, 0777);
        RED_CHECK(ssize_t(d.data.size()) == write(fd,
                                                  d.data.data(),
                                                  d.data.size()));
        RED_CHECK(0 == lseek(fd, 0, SEEK_SET));
        char transparent_target[256];

        RED_CHECK(
            "::ffff:127.0.0.1"sv == parse_ip_conntrack(
                      fd,
                      "::ffff:127.0.0.1",
                      "::ffff:255.255.255.255",
                      22,
                      36699,
                      make_writable_array_view(transparent_target),
                      true,
                      0)
            .to_sv());

        RED_CHECK(0 == lseek(fd, 0, SEEK_SET));
        transparent_target[0] = 0;
        RED_CHECK(
            "2001:abcd::1234"sv == parse_ip_conntrack(
                      fd,
                      "2001:abcd::1234",
                      "fe80::",
                      3389,
                      46392,
                      make_writable_array_view(transparent_target),
                      true,
                      0)
            .to_sv());

        RED_CHECK(0 == lseek(fd, 0, SEEK_SET));
        transparent_target[0] = 0;
        RED_CHECK(
            "::1"sv == parse_ip_conntrack(
                      fd,
                      "::1",
                      "::",
                      12000,
                      4242,
                      make_writable_array_view(transparent_target),
                      true,
                      0)
            .to_sv());

        RED_CHECK(0 == lseek(fd, 0, SEEK_SET));
        transparent_target[0] = 0;
        RED_CHECK(
            "::1"sv == parse_ip_conntrack(
                      fd,
                      "0000:0000:0000:0000:0000:0000:0000:0001",
                      "0000:0000:0000:0000:0000:0000:0000:0000",
                      12000,
                      4242,
                      make_writable_array_view(transparent_target),
                      true,
                      0)
            .to_sv());

        RED_CHECK(0 == lseek(fd, 0, SEEK_SET));
        transparent_target[0] = 0;
        RED_CHECK(
            "2a0d:5d40:888:4176:d999:e759:962:19f"sv == parse_ip_conntrack(
                      fd,
                      "2a0d:0356:0888:abcd:d999:957e:0333:012a",
                      "2001:0db8:0000:0000::ff00:0042:8329",
                      3389,
                      41971,
                      make_writable_array_view(transparent_target),
                      true,
                      0)
            .to_sv());

        RED_CHECK(0 == lseek(fd, 0, SEEK_SET));
        transparent_target[0] = 0;
        RED_CHECK(""sv == parse_ip_conntrack(
                      fd,
                      "2001:0db8:3c4d:0015:0000:0000:1a2f:1a2b",
                      "2001:db8:3c4d:15:0:d234:3eee::",
                      3389,
                      46392,
                      make_writable_array_view(transparent_target),
                      true,
                      0)
                  .to_sv());

        RED_CHECK(0 == lseek(fd, 0, SEEK_SET));
        transparent_target[0] = 0;
        RED_CHECK(""sv == parse_ip_conntrack(
                      fd,
                      "1234",
                      "5678",
                      1234,
                      56789,
                      make_writable_array_view(transparent_target),
                      true,
                      0)
                  .to_sv());

        RED_CHECK(0 == lseek(fd, 0, SEEK_SET));
        transparent_target[0] = 0;
        RED_CHECK(""sv == parse_ip_conntrack(
                      fd,
                      "fe80:::",
                      "abcd::",
                      1234,
                      56789,
                      make_writable_array_view(transparent_target),
                      true,
                      0)
                  .to_sv());

        RED_CHECK(0 == lseek(fd, 0, SEEK_SET));
        transparent_target[0] = 0;
        RED_CHECK(""sv == parse_ip_conntrack(
                      fd,
                      "2001:abcd:1234",
                      "4242:4242:4242:4242:4242",
                      1234,
                      56789,
                      make_writable_array_view(transparent_target),
                      true,
                      0)
                  .to_sv());

        RED_CHECK(0 == lseek(fd, 0, SEEK_SET));
        transparent_target[0] = 0;
        RED_CHECK(""sv == parse_ip_conntrack(
                      fd,
                      "2001:abcd:1234::4242::",
                      "::1",
                      1234,
                      56789,
                      make_writable_array_view(transparent_target),
                      true,
                      0)
                  .to_sv());

        close(fd);
    }
}
