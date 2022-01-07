/*
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

Product name: redemption, a FLOSS RDP proxy
Copyright (C) Wallix 2021
Author(s): Proxies Team
*/


#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "lib/credis.hpp"
#include "utils/netutils.hpp"
#include "core/listen.hpp"

#include <chrono>
#include <thread>

using namespace std::chrono_literals;

namespace
{

chars_view credis_buffer_get_data_view(CRedisBuffer* buffer)
{
    uint64_t len;
    char const* data = credis_buffer_get_data(buffer, &len);
    return chars_view{data, len};
}

chars_view credis_transport_get_last_error_zmessage(CRedisTransport* redis)
{
    auto msg = credis_transport_get_last_error_message(redis);
    return chars_view(msg, strlen(msg));
}

} // anonymous namespace


RED_AUTO_TEST_CASE(TestCRedisBuffer)
{
    auto* buffer = credis_buffer_new(0, 0, 0);

    RED_CHECK(credis_buffer_get_data_view(buffer) == ""_av);
    RED_CHECK(credis_buffer_push_cmd_auth(buffer, "sacrifice") == 0);
    RED_CHECK(credis_buffer_get_data_view(buffer) ==
        "*2\r\n$4\r\nAUTH\r\n$9\r\nsacrifice\r\n"
        ""_av_ascii);
    RED_CHECK(credis_buffer_push_cmd_select_db(buffer, 42) == 0);
    RED_CHECK(credis_buffer_get_data_view(buffer) ==
        "*2\r\n$4\r\nAUTH\r\n$9\r\nsacrifice\r\n"
        "*2\r\n$6\r\nSELECT\r\n$2\r\n42\r\n"
        ""_av_ascii);
    credis_buffer_clear(buffer);

    RED_CHECK(credis_buffer_get_data_view(buffer) == ""_av);
    RED_CHECK(credis_buffer_push_cmd_header(buffer, 3) == 0);
    RED_CHECK(credis_buffer_get_data_view(buffer) == "*3\r\n"_av_ascii);
    RED_CHECK(credis_buffer_push_arg_size(buffer, 4) == 0);
    RED_CHECK(credis_buffer_get_data_view(buffer) ==
        "*3\r\n$4\r\n"_av_ascii);
    RED_CHECK(credis_buffer_push_raw_data(buffer, "abcd", 4) == 0);
    RED_CHECK(credis_buffer_get_data_view(buffer) ==
        "*3\r\n$4\r\nabcd"_av_ascii);
    RED_CHECK(credis_buffer_push_arg_separator(buffer) == 0);
    RED_CHECK(credis_buffer_get_data_view(buffer) ==
        "*3\r\n$4\r\nabcd\r\n"_av_ascii);
    RED_CHECK(credis_buffer_push_i64_arg(buffer, 20) == 0);
    RED_CHECK(credis_buffer_get_data_view(buffer) ==
        "*3\r\n$4\r\nabcd\r\n$2\r\n20\r\n"_av_ascii);
    RED_CHECK(credis_buffer_push_u64_arg(buffer, 31) == 0);
    RED_CHECK(credis_buffer_get_data_view(buffer) ==
        "*3\r\n$4\r\nabcd\r\n$2\r\n20\r\n$2\r\n31\r\n"_av_ascii);
    RED_CHECK(credis_buffer_push_string_arg(buffer, "blabla", 5) == 0);
    RED_CHECK(credis_buffer_get_data_view(buffer) ==
        "*3\r\n$4\r\nabcd\r\n$2\r\n20\r\n$2\r\n31\r\n"
        "$5\r\nblabl\r\n"_av_ascii);
    RED_CHECK(credis_buffer_push_null_arg(buffer) == 0);
    RED_CHECK(credis_buffer_get_data_view(buffer) ==
        "*3\r\n$4\r\nabcd\r\n$2\r\n20\r\n$2\r\n31\r\n"
        "$5\r\nblabl\r\n$-1\r\n"_av_ascii);
    RED_CHECK(credis_buffer_push_raw_data(buffer, "xyz", 3) == 0);
    RED_CHECK(credis_buffer_get_data_view(buffer) ==
        "*3\r\n$4\r\nabcd\r\n$2\r\n20\r\n$2\r\n31\r\n"
        "$5\r\nblabl\r\n$-1\r\nxyz"_av_ascii);

    uint64_t len;
    char* data;
    credis_buffer_reset(buffer, 0, 5, 5);
    data = credis_buffer_alloc_fragment(buffer, 3);
    RED_CHECK(credis_buffer_get_data_view(buffer).size() == 3);
    memcpy(data, "abc", 3);
    RED_CHECK(credis_buffer_get_data_view(buffer) == "abc"_av_ascii);
    credis_buffer_set_size(buffer, 200);
    RED_CHECK(credis_buffer_get_data_view(buffer).size() == 200);
    credis_buffer_set_size(buffer, 2);
    RED_CHECK(credis_buffer_get_data_view(buffer) == "ab"_av_ascii);
    data = credis_buffer_build_with_prefix_and_suffix(
        buffer, "pre", 3, "post", 4, &len);
    RED_CHECK(chars_view(data, len) == "preabpost"_av_ascii);
    data = credis_buffer_build_with_prefix_and_suffix(
        buffer, "preprepre", 9, "post", 4, &len);
    RED_CHECK(chars_view(data, len) == chars_view(nullptr));

    data = credis_buffer_alloc_fragment(buffer, 9);
    RED_CHECK(credis_buffer_get_data_view(buffer).size() == 11);
    *data++ = 'x';
    RED_CHECK(credis_buffer_pop(buffer, 8) == 3);
    RED_CHECK(credis_buffer_get_data_view(buffer) == "abx"_av_ascii);
    data = credis_buffer_alloc_fragment(buffer, 4);
    RED_CHECK(credis_buffer_get_data_view(buffer).size() == 7);
    *data++ = 'y';
    *data++ = 'X';
    *data++ = 'Y';
    *data++ = 'z';
    RED_CHECK(credis_buffer_pop(buffer, 0) == 7);
    RED_CHECK(credis_buffer_get_data_view(buffer) == "abxyXYz"_av_ascii);
    RED_CHECK(credis_buffer_pop(buffer, 111) == 0);
    RED_CHECK(credis_buffer_get_data_view(buffer) == ""_av_ascii);

    credis_buffer_delete(buffer);
}

RED_AUTO_TEST_CASE(TestCRedisCmdSet)
{
    auto* cmd = credis_cmd_set_new("my_image_key", 2, 0);
    auto* buffer = credis_cmd_set_get_buffer(cmd);

    RED_CHECK(credis_buffer_push_raw_data(buffer, "abcde", 5) == 0);
    RED_CHECK(credis_buffer_push_raw_data(buffer, "fgh", 3) == 0);

    uint64_t len;
    char* data = credis_cmd_set_build_command(cmd, &len);
    RED_CHECK(chars_view(data, len) ==
        "*5\r\n"
        "$3\r\nSET\r\n"
        "$12\r\nmy_image_key\r\n"
        "$8\r\nabcdefgh\r\n"
        "$2\r\nEX\r\n"
        "$1\r\n2\r\n"_av_ascii);

    credis_buffer_clear(buffer);
    data = credis_cmd_set_build_command(cmd, &len);
    RED_CHECK(chars_view(data, len) ==
        "*5\r\n"
        "$3\r\nSET\r\n"
        "$12\r\nmy_image_key\r\n"
        "$0\r\n\r\n"
        "$2\r\nEX\r\n"
        "$1\r\n2\r\n"_av_ascii);

    credis_cmd_set_delete(cmd);
}


RED_AUTO_TEST_CASE(TestCRedisTransport)
{
    using namespace std::chrono_literals;

    auto addr = "127.0.0.1"_zv;
    int port = 4446;

    unique_fd sck_server = invalid_fd();
    for (int i = 0; i < 5; ++i) {
        sck_server = create_server(inet_addr(addr), port, EnableTransparentMode::No);
        if (sck_server.is_open()) {
            break;
        }
        // wait another test...
        std::this_thread::sleep_for(50ms);
    }
    RED_REQUIRE(sck_server.is_open());

    fcntl(sck_server.fd(), F_SETFL, fcntl(sck_server.fd(), F_GETFL) & ~O_NONBLOCK);

    unique_fd client_fd = ip_connect(addr, port, DefaultConnectTag { });
    RED_REQUIRE(client_fd.is_open());

    sockaddr s {};
    socklen_t sin_size = sizeof(s);
    int sck = accept(sck_server.fd(), &s, &sin_size);

    RED_REQUIRE(sck != -1);
    unique_fd usck{sck};

    std::vector<uint8_t> message;

    auto recv = [&]{
        message.clear();
        char buff[100];
        ssize_t r = ::recv(sck, buff, std::size(buff), 0);
        if (r > 0) {
            message.assign(buff, buff + r);
        }
        return bytes_view(message).as_chars();
    };

    auto server_send = [&](chars_view resp){
        return ::send(sck, resp.data(), resp.size(), 0) == ssize_t(resp.size());
    };

    using Code = CRedisTransportCode;

    auto* redis = credis_transport_new();

    auto msg1 =
        "*2\r\n$4\r\nAUTH\r\n$9\r\nsacrifice\r\n"
        "*2\r\n$6\r\nSELECT\r\n$2\r\n42\r\n"_av_ascii;

    uint64_t out_len;

    RED_CHECK(credis_transport_set_fd(redis, client_fd.fd()) == Code::Ok);
    RED_CHECK(credis_transport_write(redis, msg1.bytes.data(), msg1.bytes.size(), &out_len) == Code::Ok);
    RED_CHECK(out_len == msg1.bytes.size());
    RED_REQUIRE(recv() == msg1);

    RED_REQUIRE(server_send("+OK\r\n+OK\r\n"_av));
    RED_CHECK(credis_transport_read_response_ok(redis) == Code::Ok);
    RED_CHECK(credis_transport_read_response_ok(redis) == Code::Ok);

    auto msg2 =
        "*5\r\n"
        "$3\r\nSET\r\n"
        "$12\r\nmy_image_key\r\n"
        "$12\r\nblablabloblo\r\n"
        "$2\r\nEX\r\n"
        "$1\r\n2\r\n"_av_ascii;

    RED_CHECK(credis_transport_write(redis, msg2.bytes.data(), msg2.bytes.size(), &out_len) == Code::Ok);
    RED_CHECK(out_len == msg2.bytes.size());
    RED_REQUIRE(recv() == msg2);

    RED_REQUIRE(server_send("+OK\r\n"_av));
    RED_CHECK(credis_transport_read_response_ok(redis) == Code::Ok);
    RED_CHECK(credis_transport_read_response_ok(redis) == Code::WantRead);

    RED_REQUIRE(server_send("+ERR\r\n"_av));

    // Some case, buffer is not sent when we try to read
    std::this_thread::sleep_for(50ms);
    RED_CHECK(credis_transport_read_response_ok(redis) == Code::UnknownResponse);
    RED_CHECK(credis_transport_get_last_error_zmessage(redis) == "+ERR\r"_av_ascii);

    credis_transport_delete(redis);
}
