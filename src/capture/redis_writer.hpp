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
#include "utils/sugar/zstring_view.hpp"
#include "utils/sugar/bounded_array_view.hpp"

#include <chrono>
#include <vector>

#include <sys/select.h>
#include <sys/time.h>


// redis_command_set(key, value) with accumulator
class RedisCmdSet
{
public:
    explicit RedisCmdSet(chars_view key_name, std::chrono::seconds expiration_delay);

    RedisCmdSet(RedisCmdSet const&) = delete;
    RedisCmdSet& operator=(RedisCmdSet const&) = delete;

    void clear();
    void append(bytes_view data);
    bytes_view build_command();

private:
    std::size_t reserved_buffer_size() noexcept;

    std::vector<uint8_t> cmd_buffer;
    std::size_t preformatted_header_set_pos;
    std::size_t preformatted_size;
};


class RedisWriter
{
public:
    struct TlsParams
    {
        bool enable_tls;
        const char * cert_file;
        const char * key_file;
        const char * ca_cert_file;
    };

    explicit RedisWriter();

    RedisWriter(RedisWriter const&) = delete;
    RedisWriter& operator=(RedisWriter const&) = delete;

    ~RedisWriter();

    struct [[nodiscard]] IOResult
    {
        enum class Code : uint8_t
        {
            Ok,
            ConnectError,
            CertificateError,
            ReadError,
            WriteError,
            Timeout,
            UnknownResponse,
        };

        struct ErrorCtx
        {
            char const* msg = nullptr;
            int errnum = 0;
            int sslnum = 0;
        };

        static IOResult Ok() noexcept
        {
            return IOResult(IOResult::Code::Ok, ErrorCtx{});
        }

        static IOResult Unknown(bounded_array_view<char, 0, 5> resp) noexcept;

        explicit IOResult(Code code, ErrorCtx error_ctx) noexcept;

        bool ok() const noexcept
        {
            return code_ == Code::Ok;
        }

        Code code() const noexcept
        {
            return code_;
        }

        int errnum() const noexcept;
        char const* code_as_cstring() const noexcept;
        char const* error_message() const;

    private:
        explicit IOResult(bounded_array_view<char, 0, 5> resp) noexcept;

        union Data
        {
            ErrorCtx error_ctx;
            char resp[6];
        };

        Code code_;
        Data data_;
    };

    IOResult open(
        zstring_view address, zstring_view password, unsigned db,
        std::chrono::milliseconds timeout, TlsParams tls_params);
    void close();

    IOResult send(bytes_view data);

private:
    class Access;

    struct RedisTlsCtx
    {
        char const* open(
            char const* cacert_filename,
            char const* cert_filename,
            char const* private_key_filename,
            int fd);

        void close();

    private:
        friend Access;

        void* ssl_ctx;
        void* ssl;
    };

    enum class State : bool
    {
        WaitResponse,
        WaitPassword,
    };

    RedisTlsCtx tls {};
    timeval tv;
    int fd = -1;
    State state;
    fd_set rfds;
    fd_set wfds;
};
