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

#include "capture/redis_writer.hpp"
#include "core/listen.hpp"

#include <thread>


RED_AUTO_TEST_CASE(TestRedisSet)
{
    RedisCmdSet cmd{"my_image_key"_av};

    cmd.append("blabla"_av);
    cmd.append("tagadasouinsouin"_av);
    RED_CHECK(cmd.build_command() ==
        "*3\r\n"
        "$3\r\nSET\r\n"
        "$12\r\nmy_image_key\r\n"
        "$22\r\nblablatagadasouinsouin\r\n"_av_ascii);

    cmd.clear();
    cmd.append("tic tac toe"_av);
    RED_CHECK(cmd.build_command() ==
        "*3\r\n"
        "$3\r\nSET\r\n"
        "$12\r\nmy_image_key\r\n"
        "$11\r\ntic tac toe\r\n"_av_ascii);
}

RED_AUTO_TEST_CASE(TestRedisServer)
{
    auto addr = "127.0.0.1:4446"_sized_av;

    unique_fd sck_server = create_server(inet_addr("127.0.0.1"), 4446, EnableTransparentMode::No);
    RED_REQUIRE(sck_server.is_open());

    fcntl(sck_server.fd(), F_SETFL, fcntl(sck_server.fd(), F_GETFL) & ~O_NONBLOCK);

    std::vector<uint8_t> message;

    auto server_fn = [&]{
        sockaddr s {};
        socklen_t sin_size = sizeof(s);
        int sck = accept(sck_server.fd(), &s, &sin_size);

        auto recv = [&]{
            char buff[100];
            ssize_t r = ::recv(sck, buff, std::size_t(buff), 0);
            if (r > 0) {
                message.insert(message.end(), buff, buff + r);
            }
        };

        auto send = [&](chars_view resp){
            ::send(sck, resp.data(), resp.size(), 0);
        };

        // auth
        // select
        recv();
        send("+OK\r\n"_av);
        send("+OK\r\n"_av);
        // first message
        recv();
        send("+OK\r\n"_av);
        // second message
        recv();
        send("+OK\r\n"_av);
        // third message -> error
        recv();
        send("-ERR\r\n"_av);
    };

    using namespace std::chrono_literals;

    RedisWriter cmd(addr, 100ms, "admin"_sized_av, 0);

    // open -> close -> open -> close
    for (int i = 0; i < 2; ++i) {
        RED_TEST_CONTEXT("i = " << i) {
            std::thread t(server_fn);

            // delay for thread creation
            std::this_thread::sleep_for(50ms);

            RED_REQUIRE(cmd.open());

            RED_CHECK(cmd.send("bla bla"_av) == RedisWriter::IOResult::Ok);
            RED_CHECK(cmd.send("bla bla bla"_av) == RedisWriter::IOResult::Ok);
            RED_CHECK(cmd.send("bad"_av) == RedisWriter::IOResult::Ok);
            RED_CHECK(cmd.send("receive response"_av) == RedisWriter::IOResult::UnknownResponse);

            cmd.close();

            t.join();

            RED_CHECK(message ==
                "*2\r\n$4\r\nAUTH\r\n$5\r\nadmin\r\n"
                "*2\r\n$6\r\nSELECT\r\n$1\r\n0\r\n"
                "bla blabla bla blabad"_av_ascii);
            message.clear();
        }
    }
}
