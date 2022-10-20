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
char const* credis_version();


struct CRedisBuffer;
//@{
REDEMPTION_LIB_EXPORT
CRedisBuffer* credis_buffer_new(std::size_t start_capacity,
                                std::size_t reserved_prefix,
                                std::size_t reserved_suffix);

REDEMPTION_LIB_EXPORT
void credis_buffer_delete(CRedisBuffer* buffer);

REDEMPTION_LIB_EXPORT
int credis_buffer_reset(CRedisBuffer* buffer,
                        std::size_t start_capacity,
                        std::size_t reserved_prefix,
                        std::size_t reserved_suffix);

/// Allocate and use \p length bytes.
REDEMPTION_LIB_EXPORT
uint8_t* credis_buffer_alloc_fragment(CRedisBuffer* buffer, std::size_t length);

/// Allocate and use \p length_in_out bytes after \p p + \p used_size.
REDEMPTION_LIB_EXPORT
uint8_t* credis_buffer_alloc_max_fragment_at(CRedisBuffer* buffer,
                                             std::size_t* min_length_in_out,
                                             uint8_t const* p,
                                             std::size_t used_size);

REDEMPTION_LIB_EXPORT
int credis_buffer_set_size(CRedisBuffer* buffer, std::size_t n);

REDEMPTION_LIB_EXPORT
int credis_buffer_set_size_at(CRedisBuffer* buffer, uint8_t* p, std::size_t used_size);

/// \return new buffer length
REDEMPTION_LIB_EXPORT
std::size_t credis_buffer_pop(CRedisBuffer* buffer, std::size_t n);

REDEMPTION_LIB_EXPORT
int credis_buffer_push_cmd_header(CRedisBuffer* buffer, unsigned nargs);

REDEMPTION_LIB_EXPORT
int credis_buffer_push_i64_arg(CRedisBuffer* buffer, int64_t n);

REDEMPTION_LIB_EXPORT
int credis_buffer_push_u64_arg(CRedisBuffer* buffer, uint64_t n);

REDEMPTION_LIB_EXPORT
int credis_buffer_push_string_arg(CRedisBuffer* buffer,
                                  uint8_t const* value, uint32_t length);

REDEMPTION_LIB_EXPORT
int credis_buffer_push_null_arg(CRedisBuffer* buffer);

REDEMPTION_LIB_EXPORT
int credis_buffer_push_arg_size(CRedisBuffer* buffer, uint32_t length);

REDEMPTION_LIB_EXPORT
int credis_buffer_push_arg_separator(CRedisBuffer* buffer);

REDEMPTION_LIB_EXPORT
int credis_buffer_push_raw_data(CRedisBuffer* buffer,
                                uint8_t const* value, std::size_t length);

REDEMPTION_LIB_EXPORT
void credis_buffer_clear(CRedisBuffer* buffer);

/// Release memory and reset prefix/suffix to 0.
REDEMPTION_LIB_EXPORT
void credis_buffer_free(CRedisBuffer* buffer);

REDEMPTION_LIB_EXPORT
uint8_t* credis_buffer_get_data(CRedisBuffer* buffer, std::size_t* output_length);

REDEMPTION_LIB_EXPORT
int credis_buffer_push_cmd_auth(CRedisBuffer* buffer, char const* password);

REDEMPTION_LIB_EXPORT
int credis_buffer_push_cmd_select_db(CRedisBuffer* buffer, unsigned db);

REDEMPTION_LIB_EXPORT
uint8_t* credis_buffer_build_with_prefix_and_suffix(
    CRedisBuffer* buffer,
    uint8_t const* prefix, std::size_t prefix_length,
    uint8_t const* suffix, std::size_t suffix_length,
    std::size_t* output_length);
//@}


struct CRedisCmdSet;
//@{
REDEMPTION_LIB_EXPORT
CRedisCmdSet* credis_cmd_set_new(char const* key_name,
                                 unsigned expiration_delay_in_seconds,
                                 std::size_t start_capacity);

REDEMPTION_LIB_EXPORT
void credis_cmd_set_delete(CRedisCmdSet* cmd);

REDEMPTION_LIB_EXPORT
CRedisBuffer* credis_cmd_set_get_buffer(CRedisCmdSet* cmd);

REDEMPTION_LIB_EXPORT
int credis_cmd_set_free_buffer(CRedisCmdSet* cmd, std::size_t start_capacity);

REDEMPTION_LIB_EXPORT
uint8_t* credis_cmd_set_build_command(CRedisCmdSet* cmd, std::size_t* output_length);
//@}


enum class [[nodiscard]] CRedisTransportCode : int
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
    UnknownError,
};


struct CRedisTransport;
//@{
/// \return nullptr when error
REDEMPTION_LIB_EXPORT
CRedisTransport* credis_transport_new();

REDEMPTION_LIB_EXPORT
void credis_transport_delete(CRedisTransport* redis);

REDEMPTION_LIB_EXPORT
char const* credis_transport_get_last_error_message(CRedisTransport* redis);

REDEMPTION_LIB_EXPORT
CRedisTransportCode credis_transport_set_fd(CRedisTransport* redis, int fd);

REDEMPTION_LIB_EXPORT
CRedisTransportCode credis_transport_enable_tls(CRedisTransport* redis,
                                                char const* ca_cert_file,
                                                char const* cert_file,
                                                char const* key_file);

REDEMPTION_LIB_EXPORT
CRedisTransportCode credis_transport_ssl_connect(CRedisTransport* redis);

REDEMPTION_LIB_EXPORT
CRedisTransportCode credis_transport_read(CRedisTransport* redis,
                                          uint8_t* buffer,
                                          std::size_t length,
                                          std::size_t* output_length);

REDEMPTION_LIB_EXPORT
CRedisTransportCode credis_transport_write(CRedisTransport* redis,
                                           uint8_t const* buffer,
                                           std::size_t length,
                                           std::size_t* output_length);

/// \return CRedisTransportCode::Ok, CRedisTransportCode::WantRead or CRedisTransportCode::UnknownResponse
REDEMPTION_LIB_EXPORT
CRedisTransportCode credis_transport_read_response_ok(CRedisTransport* redis);
//@}

}
