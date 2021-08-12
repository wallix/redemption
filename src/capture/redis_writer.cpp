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

#include "capture/redis_writer.hpp"
#include "utils/sugar/int_to_chars.hpp"
#include "utils/static_string.hpp"
#include "utils/to_timeval.hpp"
#include "utils/netutils.hpp"
#include "utils/select.hpp"
#include "utils/log.hpp"
#include "core/error.hpp"
#include "cxx/cxx.hpp"

#include <sys/socket.h>


// redis format:
// '*' {nb_args} '\r\n'
// ( '$' {arg_len} '\r\n' {arg_value} '\r\n' )*
// Command:
// SET key value
// SELECT db
// AUTH password
// AUTH username password

namespace
{
    std::size_t reserved_buffer_size(std::size_t reserved)
    {
        // reserved + data_len
        return reserved * 2 + detail::int_to_chars_buf_size + 2;
    }
}

RedisCmdSet::RedisCmdSet(chars_view key_name)
{
    cmd_buffer.reserve(32 * 1024);

    append("*3\r\n$3\r\nSET\r\n$"_av);
    append(int_to_decimal_chars(key_name.size()));
    append("\r\n"_av);
    append(key_name);
    append("\r\n$"_av);

    reserved = cmd_buffer.size();
    clear();
}

void RedisCmdSet::clear()
{
    cmd_buffer.resize(reserved_buffer_size(reserved));
}

void RedisCmdSet::append(bytes_view data)
{
    cmd_buffer.insert(cmd_buffer.end(), data.begin(), data.end());
}

bytes_view RedisCmdSet::build_command()
{
    const auto field_sep = "\r\n"_av;

    append(field_sep);
    auto data_len_s = int_to_decimal_chars(cmd_buffer.size() - (reserved_buffer_size(reserved) + field_sep.size()));

    auto* reserved_reverse_it = cmd_buffer.data() + reserved_buffer_size(reserved);

    auto stream_write = [&](bytes_view av) mutable {
        auto p = reserved_reverse_it - av.size();
        memcpy(p, av.data(), av.size());
        reserved_reverse_it = p;
    };
    stream_write(field_sep);
    stream_write(data_len_s);
    stream_write(bytes_view(cmd_buffer).first(reserved));

    return {reserved_reverse_it, cmd_buffer.data() + cmd_buffer.size()};
}


RedisWriter::RedisWriter(
    bounded_chars_view<0, 127> address, std::chrono::milliseconds timeout,
    bounded_chars_view<0, 127> password, unsigned db)
: tv(to_timeval(timeout))
, db(db)
, address(address)
, password(password)
{}

RedisWriter::~RedisWriter()
{
    close();
}

bool RedisWriter::open()
{
    close();
    fd = addr_connect_blocking(address.c_str(), true).release();
    if (fd == -1) {
        return false;
    }

    // AUTH password
    // SELECT db
    auto db_s = int_to_decimal_chars(db);
    auto data = static_str_concat(
        "*2\r\n$4\r\nAUTH\r\n$"_sized_av,
        int_to_decimal_chars(password.size()),
        "\r\n"_sized_av, password, "\r\n"
        "*2\r\n$6\r\nSELECT\r\n$"_sized_av,
        int_to_decimal_chars(db_s.size()),
        "\r\n"_sized_av, db_s, "\r\n"_sized_av
    );
    bytes_view av = data;

    if (password.empty()) {
        av = av.from_offset(20);
    }

    ssize_t res = ::send(fd, av.data(), av.size(), 0);
    if (res <= 0 || std::size_t(res) != av.size()) {
        ::close(fd);
        fd = -1;
        return false;
    }

    io_fd_zero(fds);

    state = State::FirstPacket;
    return true;
}

void RedisWriter::close()
{
    if (fd != -1) {
        ::close(fd);
        fd = -1;
    }
}

namespace
{
    using IOResult = RedisWriter::IOResult;

    IOResult receive_response(int fd, timeval tv, fd_set& fds)
    {
        for (;;) {
            auto tv_cp = tv;
            io_fd_set(fd, fds);
            int nfds = select(fd+1, &fds, nullptr, nullptr, &tv_cp);
            if (nfds > 0) {
                using namespace std::string_view_literals;
                constexpr auto expected_resp = "+OK\r\n"sv;
                char buffer[expected_resp.size()];
                ssize_t res = ::recv(fd, buffer, expected_resp.size(), 0);
                if (res > 0) {
                    if (std::string_view(buffer, std::size_t(res)) == expected_resp) {
                        return IOResult::Ok;
                    }
                    LOG(LOG_ERR, "Redis: unknown response: %.*s", int(res), buffer);
                    return IOResult::UnknownResponse;
                }
                else if (res == -1) {
                    if (errno != EAGAIN && errno != EINTR) {
                        return IOResult::ReadError;
                    }
                }
                else {
                    return IOResult::Timeout;
                }
            }
            else if (nfds == 0) {
                return IOResult::Timeout;
            }
            else if (errno != EINTR) {
                return IOResult::ReadEventError;
            }
        }
    }
}

RedisWriter::IOResult RedisWriter::send(bytes_view data)
{

#define CHECK(x, err) do { if (REDEMPTION_UNLIKELY(x == -1)) return err; } while (0)

    if (state == State::FirstPacket) {
        if (!password.empty()) {
            auto res = receive_response(fd, tv, fds);
            if (res != IOResult::Ok) {
                return res;
            }
        }
        state = State::WaitResponse;
    }

    if (auto res = receive_response(fd, tv, fds)
      ; res != IOResult::Ok
    ) {
        return res;
    }

    auto send_data = [this](uint8_t const* p, std::size_t len){
        ssize_t res = ::send(fd, p, len, 0);
        if (REDEMPTION_UNLIKELY(res == -1)) {
            if (errno == EAGAIN || errno == EINTR) {
                res = 0;
            }
        }
        return res;
    };

    auto* p = data.data();
    auto len = data.size();

    ssize_t res = send_data(p, len);
    CHECK(res, IOResult::WriteError);

    if (len != std::size_t(res)) {
        auto tv_remaining = tv;
        auto t = MonotonicTimePoint::clock::now();

        for (;;) {
            io_fd_set(fd, fds);
            int nfds = select(fd+1, nullptr, &fds, nullptr, &tv_remaining);
            if (nfds > 0) {
                p += res;
                len -= std::size_t(res);
                res = send_data(p, len);
                CHECK(res, IOResult::WriteError);
                if (std::size_t(res) == len) {
                    break;
                }
            }
            else if (nfds == 0) {
                return IOResult::Timeout;
            }
            else if (errno != EINTR) {
                // possibly EINVAL -> negative timeout
                return IOResult::WriteEventError;
            }

            tv_remaining = to_timeval(MonotonicTimePoint::clock::now() - t);
        }
    }

    return IOResult::Ok;
}
