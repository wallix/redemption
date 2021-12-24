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

#include "cxx/cxx.hpp"

#include <cstdint>

extern "C"
{
    REDEMPTION_LIB_EXPORT
    char const* redis_version();

    struct RedemptionRedis;

    enum class [[nodiscard]] RedemptionRedisCode : uint8_t
    {
        // from RedisIOCode
        //@{
        Ok,
        WantRead,
        WantWrite,
        ConnectError,
        CertificateError,
        ReadError,
        WriteError,
        Timeout,
        UnknownResponse,
        //@}

        InvalidFd,
        MallocError,
        UnknownError,
    };


    // Resource
    //@{
    /// \return nullptr when error
    REDEMPTION_LIB_EXPORT
    RedemptionRedis* redis_new(char const* key_name,
                               unsigned expiration_delay);

    REDEMPTION_LIB_EXPORT
    void redis_delete(RedemptionRedis* redis);
    //@}


    // Error
    //@{
    REDEMPTION_LIB_EXPORT
    char const* redis_get_last_error_message(RedemptionRedis* redis);
    //@}


    // Buffer
    //@{
    REDEMPTION_LIB_EXPORT
    RedemptionRedisCode redis_buffer_push_cmd_auth(RedemptionRedis* redis,
                                                   char const* password);

    REDEMPTION_LIB_EXPORT
    RedemptionRedisCode redis_buffer_push_cmd_select_db(RedemptionRedis* redis,
                                                        unsigned db);

    REDEMPTION_LIB_EXPORT
    RedemptionRedisCode redis_buffer_build_commands(RedemptionRedis* redis);

    REDEMPTION_LIB_EXPORT
    RedemptionRedisCode redis_buffer_push_data(RedemptionRedis* redis,
                                               uint8_t const* buffer,
                                               uint64_t len);

    REDEMPTION_LIB_EXPORT
    RedemptionRedisCode redis_buffer_build_cmd_set(RedemptionRedis* redis);

    REDEMPTION_LIB_EXPORT
    uint8_t const* redis_buffer_get_current_data(RedemptionRedis* redis,
                                                 uint64_t* output_len);

    REDEMPTION_LIB_EXPORT
    void redis_buffer_clear(RedemptionRedis* redis);
    //@}


    // read / write
    //@{
    REDEMPTION_LIB_EXPORT
    RedemptionRedisCode redis_set_fd(RedemptionRedis* redis, int fd);

    REDEMPTION_LIB_EXPORT
    RedemptionRedisCode redis_enable_tls(RedemptionRedis* redis,
                                         char const* ca_cert_file,
                                         char const* cert_file,
                                         char const* key_file);

    REDEMPTION_LIB_EXPORT
    RedemptionRedisCode redis_ssl_connect(RedemptionRedis* redis);

    REDEMPTION_LIB_EXPORT
    RedemptionRedisCode redis_read(RedemptionRedis* redis,
                                   uint8_t* buffer,
                                   uint64_t len,
                                   uint64_t* output_len);

    REDEMPTION_LIB_EXPORT
    RedemptionRedisCode redis_write(RedemptionRedis* redis,
                                    uint8_t const* buffer,
                                    uint64_t len,
                                    uint64_t* output_len);

    REDEMPTION_LIB_EXPORT
    RedemptionRedisCode redis_write_builded_commands(RedemptionRedis* redis);

    REDEMPTION_LIB_EXPORT
    RedemptionRedisCode redis_read_response_ok(RedemptionRedis* redis);
    //@}
}
