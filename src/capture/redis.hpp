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
#include "utils/static_string.hpp"

#include <chrono>
#include <vector>


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

    bytes_view current_data() const noexcept;

private:
    std::size_t reserved_buffer_size() const noexcept;

    std::vector<uint8_t> cmd_buffer;
    std::size_t preformatted_header_set_pos;
    std::size_t preformatted_size;
};

struct RedisAuth
{
    RedisAuth(bounded_chars_view<0, 256> password, unsigned db);

    bytes_view packet() const noexcept;

    unsigned count_command() const noexcept
    {
        return nb_command;
    }

private:
    unsigned nb_command;
    unsigned offset;
    static_string<384> buffer;
};

enum class [[nodiscard]] RedisIOCode : uint8_t
{
    Ok,
    WantRead,
    WantWrite,
    ConnectError,
    CertificateError,
    ReadError,
    WriteError,
    Timeout,
    UnknownResponse,
};

zstring_view redis_io_code_to_zstring(RedisIOCode code) noexcept;

struct RedisWriter
{
    using IOCode = RedisIOCode;

    struct [[nodiscard]] IOResult
    {
        IOCode code;
        std::size_t len;
    };

    void set_fd(int fd);

    int get_fd() const noexcept
    {
        return fd;
    }

    void close();

    [[nodiscard]]
    char const* enable_tls(
        char const* cacert_filename,
        char const* cert_filename,
        char const* private_key_filename);

    IOCode ssl_connect();

    IOResult recv(writable_bytes_view buffer);
    IOResult send(bytes_view buffer);

    int get_last_errno() const noexcept;
    char const* get_last_error_message() const;

private:
    IOCode ssl_result_to_io_code(int res, IOCode code);

    class Access;

    struct Tls
    {
        char const* open(
            char const* cacert_filename,
            char const* cert_filename,
            char const* private_key_filename,
            int fd);

        void close();

        ~Tls();

    private:
        friend Access;

        void* ssl_ctx;
        void* ssl;
    };

    Tls tls {};
    int fd = -1;
    int ssl_errnum = 0;
};
