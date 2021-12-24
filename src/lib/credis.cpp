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

#include "lib/credis.hpp"
#include "main/version.hpp"
#include "capture/redis.hpp"
#include "utils/c_interface.hpp"
#include "utils/static_string.hpp"
#include "utils/sugar/int_to_chars.hpp"

#define CHECK_NOTHROW(expr, error_expr, return_err) \
    do {                                            \
        try { expr; }                               \
        catch (...) {                               \
            EXIT_ON_EXCEPTION();                    \
            error_expr;                             \
            return return_err;                      \
        }                                           \
    } while (0)


class RedemptionRedis
{
public:
    static constexpr std::string_view resp_ok = "+OK\r\n";

    RedisCmdSet cmd;
    RedisWriter writer;
    bytes_view builded_command {""_av};
    char const* err_msg = nullptr;
    char rep[resp_ok.size() + 1] {};
    std::size_t rep_len = 0;
};

namespace
{
    static RedemptionRedisCode to_redemption_redis_code(RedisIOCode code) noexcept
    {
        switch (code) {
            case RedisIOCode::Ok: return RedemptionRedisCode::Ok;
            case RedisIOCode::WantRead: return RedemptionRedisCode::WantRead;
            case RedisIOCode::WantWrite: return RedemptionRedisCode::WantWrite;
            case RedisIOCode::ConnectError: return RedemptionRedisCode::ConnectError;
            case RedisIOCode::CertificateError: return RedemptionRedisCode::CertificateError;
            case RedisIOCode::ReadError: return RedemptionRedisCode::ReadError;
            case RedisIOCode::WriteError: return RedemptionRedisCode::WriteError;
            case RedisIOCode::Timeout: return RedemptionRedisCode::Timeout;
            case RedisIOCode::UnknownResponse: return RedemptionRedisCode::UnknownResponse;
        }
        return RedemptionRedisCode::UnknownError;
    }
}

extern "C"
{

REDEMPTION_LIB_EXPORT
char const * redis_version()
{
    return VERSION;
}

REDEMPTION_LIB_EXPORT
RedemptionRedis* redis_new(char const* key_name, unsigned expiration_delay)
{
    SCOPED_TRACE;

    chars_view key {key_name, strlen(key_name)};

    CHECK_NOTHROW(
        return (new(std::nothrow) RedemptionRedis{
            RedisCmdSet{key, std::chrono::seconds(expiration_delay)},
            RedisWriter{},
        }),
        void(),
        nullptr
    );
}

void redis_delete(RedemptionRedis* redis)
{
    delete redis;
}

REDEMPTION_LIB_EXPORT
char const* redis_get_last_error_message(RedemptionRedis* redis)
{
    SCOPED_TRACE;

    if (auto* msg = redis->err_msg) {
        redis->err_msg = nullptr;
        return msg;
    }

    if (auto* msg = redis->writer.get_last_error_message()) {
        return msg;
    }

    return "No error";
}

REDEMPTION_LIB_EXPORT
RedemptionRedisCode redis_buffer_push_cmd_auth(RedemptionRedis* redis, char const* password)
{
    SCOPED_TRACE;

    constexpr auto maxlen = 256;
    auto passlen = strnlen(password, maxlen);
    auto str = static_str_concat(
        "*2\r\n$4\r\nAUTH\r\n$"_sized_av,
        int_to_decimal_chars(passlen),
        "\r\n"_sized_av,
        bounded_chars_view<0, maxlen>::assumed(password, passlen),
        "\r\n"_sized_av
    );

    CHECK_NOTHROW(
        redis->cmd.append(str),
        redis->err_msg = "malloc error",
        RedemptionRedisCode::MallocError
    );
    return RedemptionRedisCode::Ok;
}

REDEMPTION_LIB_EXPORT
RedemptionRedisCode redis_buffer_push_cmd_select_db(RedemptionRedis* redis, unsigned db)
{
    SCOPED_TRACE;

    auto db_s = int_to_decimal_chars(db);
    auto str = static_str_concat(
        "*2\r\n$6\r\nSELECT\r\n$"_sized_av,
        int_to_decimal_chars(db_s.size()),
        "\r\n"_sized_av, db_s, "\r\n"_sized_av
    );

    CHECK_NOTHROW(
        redis->cmd.append(str),
        redis->err_msg = "malloc error",
        RedemptionRedisCode::MallocError
    );
    return RedemptionRedisCode::Ok;
}

REDEMPTION_LIB_EXPORT
RedemptionRedisCode redis_buffer_build_commands(RedemptionRedis* redis)
{
    redis->builded_command = redis->cmd.current_data();
    return RedemptionRedisCode::Ok;
}

REDEMPTION_LIB_EXPORT
RedemptionRedisCode redis_buffer_push_data(RedemptionRedis* redis, uint8_t const* buffer, uint64_t len)
{
    SCOPED_TRACE;

    CHECK_NOTHROW(
        redis->cmd.append({buffer, len}),
        redis->err_msg = "malloc error",
        RedemptionRedisCode::MallocError
    );
    return RedemptionRedisCode::Ok;
}

REDEMPTION_LIB_EXPORT
RedemptionRedisCode redis_buffer_build_cmd_set(RedemptionRedis* redis)
{
    SCOPED_TRACE;

    CHECK_NOTHROW(
        redis->builded_command = redis->cmd.build_command(),
        redis->err_msg = "malloc error",
        RedemptionRedisCode::MallocError
    );
    return RedemptionRedisCode::Ok;
}

REDEMPTION_LIB_EXPORT
uint8_t const* redis_buffer_get_current_data(RedemptionRedis* redis, uint64_t* output_len)
{
    SCOPED_TRACE;

    *output_len = redis->builded_command.size();
    return redis->builded_command.data();
}

REDEMPTION_LIB_EXPORT
void redis_buffer_clear(RedemptionRedis* redis)
{
    redis->builded_command = ""_av;
    redis->cmd.clear();
}

REDEMPTION_LIB_EXPORT
RedemptionRedisCode redis_set_fd(RedemptionRedis* redis, int fd)
{
    SCOPED_TRACE;

    redis->writer.set_fd(fd);
    if (fd >= 0) {
        return RedemptionRedisCode::Ok;
    }

    redis->err_msg = "invalid fd";
    return RedemptionRedisCode::InvalidFd;
}

REDEMPTION_LIB_EXPORT
RedemptionRedisCode redis_enable_tls(RedemptionRedis* redis, char const* ca_cert_file,
                     char const* cert_file, char const* key_file)
{
    SCOPED_TRACE;

    redis->err_msg = redis->writer.enable_tls(ca_cert_file, cert_file, key_file);
    if (!redis->err_msg) {
        return RedemptionRedisCode::Ok;
    }
    return RedemptionRedisCode::CertificateError;
}

REDEMPTION_LIB_EXPORT
RedemptionRedisCode redis_ssl_connect(RedemptionRedis* redis)
{
    SCOPED_TRACE;
    return to_redemption_redis_code(redis->writer.ssl_connect());
}

REDEMPTION_LIB_EXPORT
RedemptionRedisCode redis_read(RedemptionRedis* redis, uint8_t* buffer,
                uint64_t len, uint64_t* output_len)
{
    SCOPED_TRACE;

    auto result = redis->writer.recv(writable_bytes_view{buffer, len});
    *output_len = result.len;
    return to_redemption_redis_code(result.code);
}

REDEMPTION_LIB_EXPORT
RedemptionRedisCode redis_write(RedemptionRedis* redis, uint8_t const* buffer,
                uint64_t len, uint64_t* output_len)
{
    SCOPED_TRACE;

    auto result = redis->writer.send(bytes_view{buffer, len});
    *output_len = result.len;
    return to_redemption_redis_code(result.code);
}

REDEMPTION_LIB_EXPORT
RedemptionRedisCode redis_write_builded_commands(RedemptionRedis* redis)
{
    SCOPED_TRACE;

    auto result = redis->writer.send(redis->builded_command);
    redis->builded_command = redis->builded_command.drop_front(result.len);
    return to_redemption_redis_code(result.code);
}

REDEMPTION_LIB_EXPORT
RedemptionRedisCode redis_read_response_ok(RedemptionRedis* redis)
{
    SCOPED_TRACE;

    constexpr auto expected_resp = RedemptionRedis::resp_ok;

    writable_bytes_view buf{redis->rep, expected_resp.size()};

    auto result = redis->writer.recv(buf.drop_front(redis->rep_len));
    redis->rep_len += result.len;
    if (bool(result.code)) {
        return to_redemption_redis_code(result.code);
    }

    if (result.len > 0 && redis->rep_len == expected_resp.size()) {
        if (chars_view{redis->rep, expected_resp.size()}.as<std::string_view>() == expected_resp) {
            redis->rep_len = 0;
            return RedemptionRedisCode::Ok;
        }

        redis->rep[redis->rep_len] = 0;
        redis->rep_len = 0;
        redis->err_msg = redis->rep;
        return RedemptionRedisCode::UnknownResponse;
    }

    return RedemptionRedisCode::WantRead;
}

}
