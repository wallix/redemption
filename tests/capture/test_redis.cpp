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

#include "capture/redis.hpp"
#include "core/listen.hpp"

#include <thread>

RED_AUTO_TEST_CASE(TestRedisSet)
{
    using namespace std::chrono_literals;
    RedisCmdSet cmd{"my_image_key"_av, 2s};

    ut::default_ascii_min_len = 0;

    cmd.append("blabla"_av);
    cmd.append("tagadasouinsouin"_av);
    RED_CHECK(cmd.current_data() == "blablatagadasouinsouin"_av);
    RED_CHECK(cmd.build_command() ==
        "*5\r\n"
        "$3\r\nSET\r\n"
        "$12\r\nmy_image_key\r\n"
        "$22\r\nblablatagadasouinsouin\r\n"
        "$2\r\nEX\r\n"
        "$1\r\n2\r\n"_av_ascii);

    cmd.clear();
    cmd.append("tic tac toe"_av);
    RED_CHECK(cmd.build_command() ==
        "*5\r\n"
        "$3\r\nSET\r\n"
        "$12\r\nmy_image_key\r\n"
        "$11\r\ntic tac toe\r\n"
        "$2\r\nEX\r\n"
        "$1\r\n2\r\n"_av_ascii);
}

RED_AUTO_TEST_CASE(TestRedisServer)
{
    using namespace std::chrono_literals;

    auto addr = "127.0.0.1"_zv;
    int port = 4446;
    auto password = "admin"_sized_av;

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

    std::vector<uint8_t> message;

    RedisSyncSession session;

    // open -> close -> open -> close
    for (int i = 0; i < 2; ++i) {
        RED_TEST_CONTEXT("i = " << i) {
            RED_REQUIRE(session.open(addr, unsigned(port), password, 0, 50ms, RedisSyncSession::TlsParams{})
                == RedisIOCode::Ok);

            sockaddr s {};
            socklen_t sin_size = sizeof(s);
            int sck = accept(sck_server.fd(), &s, &sin_size);

            RED_REQUIRE(sck != -1);
            unique_fd usck{sck};

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

            RED_REQUIRE(recv() ==
                "*2\r\n$4\r\nAUTH\r\n$5\r\nadmin\r\n"
                "*2\r\n$6\r\nSELECT\r\n$1\r\n0\r\n"_av);
            RED_REQUIRE(server_send("+OK\r\n+OK\r\n"_av));

            RED_CHECK(session.send("bla bla"_av) == RedisIOCode::Ok);

            RED_REQUIRE(recv() == "bla bla"_av);
            RED_REQUIRE(server_send("+OK\r\n"_av));
            RED_CHECK(session.send("bla bla bla"_av) == RedisIOCode::Ok);

            RED_REQUIRE(recv() == "bla bla bla"_av);
            RED_REQUIRE(server_send("+OK\r\n"_av));
            RED_CHECK(session.send("bad"_av) == RedisIOCode::Ok);

            RED_REQUIRE(recv() == "bad"_av);
            RED_REQUIRE(server_send("-ERR\r\n"_av));
            RED_CHECK(session.send("receive response"_av) == RedisIOCode::UnknownResponse);

            session.close();
        }
    }
}
