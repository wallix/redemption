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
   Copyright (C) Wallix 2017
   Author(s): Christophe Grosjean, Clément Moroldo

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

chars_view redis_buffer_get_current_data_view(RedemptionRedis* redis)
{
    uint64_t len;
    uint8_t const* data = ::redis_buffer_get_current_data(redis, &len);
    return bytes_view{data, len}.as_chars();
};

RedemptionRedisCode redis_buffer_push_data(RedemptionRedis* redis, bytes_view buffer)
{
    return redis_buffer_push_data(redis, buffer.data(), buffer.size());
};

chars_view redis_get_last_error_zmessage(RedemptionRedis* redis)
{
    auto msg = redis_get_last_error_message(redis);
    return chars_view(msg, strlen(msg));
}

} // anonymous namespace


RED_AUTO_TEST_CASE(TestCRedisBuffer)
{
    using Code = RedemptionRedisCode;

    auto* redis = redis_new("my_image_key", 2);

    RED_CHECK(redis_buffer_get_current_data_view(redis) == ""_av);
    RED_CHECK(redis_buffer_push_cmd_auth(redis, "sacrifice") == Code::Ok);
    RED_CHECK(redis_buffer_push_cmd_select_db(redis, 42) == Code::Ok);
    RED_CHECK(redis_buffer_get_current_data_view(redis) == ""_av);
    RED_CHECK(redis_buffer_build_commands(redis) == Code::Ok);
    RED_CHECK(redis_buffer_get_current_data_view(redis) ==
        "*2\r\n$4\r\nAUTH\r\n$9\r\nsacrifice\r\n"
        "*2\r\n$6\r\nSELECT\r\n$2\r\n42\r\n"
        ""_av_ascii);
    redis_buffer_clear(redis);

    RED_CHECK(redis_buffer_get_current_data_view(redis) == ""_av);
    RED_CHECK(redis_buffer_push_data(redis, "blabla"_av) == Code::Ok);
    RED_CHECK(redis_buffer_push_data(redis, "bloblo"_av) == Code::Ok);
    RED_CHECK(redis_buffer_get_current_data_view(redis) == ""_av);
    RED_CHECK(redis_buffer_build_commands(redis) == Code::Ok);
    RED_CHECK(redis_buffer_get_current_data_view(redis) == "blablabloblo"_av);
    RED_CHECK(redis_buffer_build_cmd_set(redis) == Code::Ok);
    RED_CHECK(redis_buffer_get_current_data_view(redis) ==
        "*5\r\n"
        "$3\r\nSET\r\n"
        "$12\r\nmy_image_key\r\n"
        "$12\r\nblablabloblo\r\n"
        "$2\r\nEX\r\n"
        "$1\r\n2\r\n"_av_ascii);
    redis_buffer_clear(redis);

    RED_CHECK(redis_buffer_get_current_data_view(redis) == ""_av);

    redis_delete(redis);
}


RED_AUTO_TEST_CASE(TestCRedisWriter)
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

    unique_fd client_fd = ip_connect(addr, port);
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

    using Code = RedemptionRedisCode;

    auto* redis = redis_new("my_image_key", 2);

    RED_CHECK(redis_set_fd(redis, client_fd.fd()) == Code::Ok);
    RED_CHECK(redis_buffer_push_cmd_auth(redis, "sacrifice") == Code::Ok);
    RED_CHECK(redis_buffer_push_cmd_select_db(redis, 42) == Code::Ok);
    RED_CHECK(redis_buffer_build_commands(redis) == Code::Ok);
    RED_CHECK(redis_write_builded_commands(redis) == Code::Ok);
    redis_buffer_clear(redis);

    RED_REQUIRE(recv() ==
        "*2\r\n$4\r\nAUTH\r\n$9\r\nsacrifice\r\n"
        "*2\r\n$6\r\nSELECT\r\n$2\r\n42\r\n"_av_ascii);
    RED_REQUIRE(server_send("+OK\r\n+OK\r\n"_av));

    RED_CHECK(redis_read_response_ok(redis) == Code::Ok);
    RED_CHECK(redis_read_response_ok(redis) == Code::Ok);

    RED_CHECK(redis_buffer_push_data(redis, "blabla"_av) == Code::Ok);
    RED_CHECK(redis_buffer_push_data(redis, "bloblo"_av) == Code::Ok);
    RED_CHECK(redis_buffer_build_cmd_set(redis) == Code::Ok);
    RED_CHECK(redis_write_builded_commands(redis) == Code::Ok);
    redis_buffer_clear(redis);

    RED_REQUIRE(recv() ==
        "*5\r\n"
        "$3\r\nSET\r\n"
        "$12\r\nmy_image_key\r\n"
        "$12\r\nblablabloblo\r\n"
        "$2\r\nEX\r\n"
        "$1\r\n2\r\n"_av_ascii);
    RED_REQUIRE(server_send("+OK\r\n"_av));

    RED_CHECK(redis_read_response_ok(redis) == Code::Ok);
    RED_CHECK(redis_read_response_ok(redis) == Code::WantRead);

    RED_REQUIRE(server_send("+ERR\r\n"_av));
    RED_CHECK(redis_read_response_ok(redis) == Code::UnknownResponse);
    RED_CHECK(redis_get_last_error_zmessage(redis) == "+ERR\r"_av_ascii);

    redis_delete(redis);
}
