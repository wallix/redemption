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

#include "capture/redis.hpp"

#include <chrono>

#include <sys/time.h>

struct RedisSyncSession
{
    using IOCode = RedisIOCode;

    struct TlsParams
    {
        bool enable_tls;
        const char * cert_file;
        const char * key_file;
        const char * ca_cert_file;
    };

    IOCode open(
        zstring_view address, unsigned port,
        bounded_chars_view<0, 256> password, unsigned db,
        std::chrono::milliseconds timeout, TlsParams tls_params);

    void close();

    IOCode send(bytes_view buffer);

    int get_last_errno() const noexcept;
    char const* get_last_error_message() const;

private:
    template<class F, class Buffer>
    IOCode loop_event(F&& f, Buffer buffer, IOCode code_for_waiting);
    IOCode send_impl(bytes_view buffer);
    IOCode recv_impl(writable_bytes_view buffer);

    enum class State : bool
    {
        WaitResponse,
        WaitPassword,
    };

    timeval tv;
    State state;
    fd_set rfds;
    fd_set wfds;
    RedisWriter writer;
    static constexpr std::size_t resp_buffer_len = 5;
    char resp_buffer[resp_buffer_len + 1];
    char const* error_msg = nullptr;
};
