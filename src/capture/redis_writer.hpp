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

#pragma once

#include "utils/sugar/bytes_view.hpp"
#include "utils/static_string.hpp"

#include <chrono>
#include <vector>

#include <sys/select.h>
#include <sys/time.h>


// redis_command_set(key, value) with accumulator
class RedisCmdSet
{
public:
    explicit RedisCmdSet(chars_view key_name);

    RedisCmdSet(RedisCmdSet const&) = delete;
    RedisCmdSet& operator=(RedisCmdSet const&) = delete;

    void clear();
    void append(bytes_view data);
    bytes_view build_command();

private:
    std::vector<uint8_t> cmd_buffer;
    std::size_t reserved;
};


class RedisWriter
{
public:
    explicit RedisWriter(
        bounded_chars_view<0, 127> address, std::chrono::milliseconds timeout,
        bounded_chars_view<0, 127> password, unsigned db
    );

    RedisWriter(RedisWriter const&) = delete;
    RedisWriter& operator=(RedisWriter const&) = delete;

    ~RedisWriter();

    bool open();
    void close();

    enum class IOResult : uint8_t
    {
        Ok,
        ReadError,
        WriteError,
        ReadEventError,
        WriteEventError,
        Timeout,
        UnknownResponse,
    };

    IOResult send(bytes_view data);

private:
    enum class State : bool
    {
        FirstPacket,
        WaitResponse,
    };

    timeval tv;
    fd_set fds;
    int fd = -1;
    unsigned db;
    State state {};
    static_string<127> address;
    static_string<127> password;
};
