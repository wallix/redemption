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
#include "utils/uninit_buffer.hpp"

#include <memory>


extern "C"
{
    REDEMPTION_LIB_EXPORT
    char const * credis_version()
    {
        return VERSION;
    }
}

#define CHECK_NOTHROW(expr, error_expr, return_err) \
    do {                                            \
        try { expr; }                               \
        catch (...) {                               \
            EXIT_ON_EXCEPTION();                    \
            error_expr;                             \
            return return_err;                      \
        }                                           \
    } while (0)

#define CHECK_NOTHROW_INT_R(expr) [&]{ \
        try {                          \
            expr;                      \
            return 0;                  \
        }                              \
        catch (...) {                  \
            EXIT_ON_EXCEPTION();       \
            return -1;                 \
        }                              \
    }()


struct CRedisBuffer
{
    struct Writer
    {
        uint8_t* p;

        void push(char c) noexcept
        {
            push(static_cast<uint8_t>(c));
        }

        void push(uint8_t c) noexcept
        {
            *p++ = c;
        }

        void push(bytes_view av) noexcept
        {
            std::memcpy(p, av.data(), av.size());
            p += av.size();
        }

        void move(bytes_view av) noexcept
        {
            std::memmove(p, av.data(), av.size());
            p += av.size();
        }
    };

    CRedisBuffer() = default;

    CRedisBuffer(std::size_t start_capacity, std::size_t reserved_prefix, std::size_t reserved_suffix)
    {
        reset(start_capacity, reserved_prefix, reserved_suffix);
    }

    void push_cmd_header(unsigned value)
    {
        auto value_s = int_to_decimal_chars(value);

        auto n = 3 + value_s.size();
        auto writer = use(n);
        writer.push('*');
        writer.push(value_s);
        writer.push("\r\n"_av);
    }

    void push_arg(int64_t value)
    {
        push_arg(int_to_decimal_chars(value));
    }

    void push_arg(uint64_t value)
    {
        push_arg(int_to_decimal_chars(value));
    }

    void push_arg(bytes_view value)
    {
        auto value_len_s = int_to_decimal_chars(value.size());

        auto n = 5 + value.size() + value_len_s.size();
        auto writer = use(n);
        writer.push('$');
        writer.push(value_len_s);
        writer.push("\r\n"_av);
        writer.push(value);
        writer.push("\r\n"_av);
    }

    void push_arg(std::nullptr_t)
    {
        auto cmd = "$-1\r\n"_av;
        auto writer = use(cmd.size());
        writer.push(cmd);
    }

    void push_arg_size(uint32_t value)
    {
        auto value_s = int_to_decimal_chars(value);

        auto n = 3 + value_s.size();
        auto writer = use(n);
        writer.push('$');
        writer.push(value_s);
        writer.push("\r\n"_av);
    }

    void push_arg_separator()
    {
        use(2).push("\r\n"_av);
    }

    void push_raw_data(bytes_view value)
    {
        use(value.size()).push(value);
    }

    bool is_valid_prefix(bytes_view prefix) const noexcept
    {
        return prefix.size() <= reserved_prefix;
    }

    bool is_valid_suffix(bytes_view suffix) const noexcept
    {
        return suffix.size() <= reserved_suffix;
    }

    writable_bytes_view build_with_prefix_and_suffix(bytes_view prefix, bytes_view suffix) noexcept
    {
        assert(is_valid_prefix(prefix));
        assert(is_valid_suffix(suffix));
        auto p = buf.data() + reserved_prefix - prefix.size();
        memcpy(p, prefix.data(), prefix.size());
        memcpy(buf.data() + reserved_prefix + used_len, suffix.data(), suffix.size());
        return {p, used_len + prefix.size() + suffix.size()};
    }

    bytes_view buffer() const noexcept
    {
        return {buf.data() + reserved_prefix, used_len};
    }

    std::size_t capacity() const noexcept
    {
        return buf.size() - (reserved_prefix + reserved_suffix);
    }

    writable_bytes_view buffer() noexcept
    {
        return {buf.data() + reserved_prefix, used_len};
    }

    Writer use(std::size_t n)
    {
        auto capacity = n + used_len + reserved_prefix + reserved_suffix;
        buf.grow(grow_size(capacity), used_len, reserved_prefix);

        auto* p = buf.data() + reserved_prefix + used_len;
        used_len += n;
        return Writer{p};
    }

    void force_buffer_size(std::size_t n) noexcept
    {
        assert(n <= capacity());
        used_len = n;
    }

    void clear() noexcept
    {
        used_len = 0;
    }

    void free()
    {
        buf.free();
        used_len = 0;
        reserved_prefix = 0;
        reserved_suffix = 0;
    }

    void reset(std::size_t capacity, std::size_t reserved_prefix, std::size_t reserved_suffix)
    {
        this->reserved_prefix = reserved_prefix;
        this->reserved_suffix = reserved_suffix;
        this->used_len = 0;

        auto reserved = reserved_prefix + reserved_suffix + capacity;
        if (reserved) {
            buf.grow(grow_size(reserved), 0);
        }
    }

private:
    static std::size_t grow_size(std::size_t n) noexcept
    {
        if (n <= 128) {
            return 128;
        }

        // next pow 2
        n--;
        for (std::size_t i = 1; i < sizeof(n); i *= 2) {
            n |= n >> i;
        }
        ++n;

        return n;
    }

    UninitDynamicBuffer buf;
    std::size_t used_len = 0;
    std::size_t reserved_prefix = 0;
    std::size_t reserved_suffix = 0;
};

namespace
{
    template<class... AV>
    void credis_buffer_push_avs(CRedisBuffer& buffer, AV... av)
    {
        std::size_t n = (... + av.size());
        auto writer = buffer.use(n);
        (writer.push(av), ...);
    }
} // anonymous namespace

extern "C"
{

REDEMPTION_LIB_EXPORT
CRedisBuffer* credis_buffer_new(std::size_t start_capacity,
                                std::size_t reserved_prefix,
                                std::size_t reserved_suffix)
{
    SCOPED_TRACE;

    CHECK_NOTHROW(
        return new(std::nothrow) CRedisBuffer(start_capacity,
                                              reserved_prefix,
                                              reserved_suffix),
        void(),
        nullptr
    );
}

REDEMPTION_LIB_EXPORT
void credis_buffer_delete(CRedisBuffer* buffer)
{
    SCOPED_TRACE;

    delete buffer;
}

REDEMPTION_LIB_EXPORT
int credis_buffer_reset(CRedisBuffer* buffer,
                        std::size_t start_capacity,
                        std::size_t reserved_prefix,
                        std::size_t reserved_suffix)
{
    SCOPED_TRACE;

    return CHECK_NOTHROW_INT_R(
        buffer->reset(start_capacity, reserved_prefix, reserved_suffix)
    );
}

REDEMPTION_LIB_EXPORT
char* credis_buffer_alloc_fragment(CRedisBuffer* buffer, std::size_t length)
{
    SCOPED_TRACE;

    CHECK_NOTHROW(
        return char_ptr_cast(buffer->use(length).p),
        void(),
        nullptr
    );
}

REDEMPTION_LIB_EXPORT
int credis_buffer_set_size(CRedisBuffer* buffer, std::size_t n)
{
    SCOPED_TRACE;

    auto length = buffer->buffer().size();
    if (length < n) {
        return CHECK_NOTHROW_INT_R(buffer->use(n - length));
    }

    buffer->force_buffer_size(n);
    return 0;
}

REDEMPTION_LIB_EXPORT
std::size_t credis_buffer_pop(CRedisBuffer* buffer, std::size_t n)
{
    SCOPED_TRACE;

    auto length = buffer->buffer().size();
    length -= std::min(n, length);
    buffer->force_buffer_size(length);
    return length;
}

REDEMPTION_LIB_EXPORT
int credis_buffer_push_cmd_header(CRedisBuffer* buffer, unsigned nargs)
{
    SCOPED_TRACE;

    return CHECK_NOTHROW_INT_R(buffer->push_cmd_header(nargs));
}

REDEMPTION_LIB_EXPORT
int credis_buffer_push_i64_arg(CRedisBuffer* buffer, int64_t n)
{
    SCOPED_TRACE;

    return CHECK_NOTHROW_INT_R(buffer->push_arg(n));
}

REDEMPTION_LIB_EXPORT
int credis_buffer_push_u64_arg(CRedisBuffer* buffer, uint64_t n)
{
    SCOPED_TRACE;

    return CHECK_NOTHROW_INT_R(buffer->push_arg(n));
}

REDEMPTION_LIB_EXPORT
int credis_buffer_push_string_arg(CRedisBuffer* buffer,
                                  char const* value, uint32_t length)
{
    SCOPED_TRACE;

    return CHECK_NOTHROW_INT_R(buffer->push_arg({value, length}));
}

REDEMPTION_LIB_EXPORT
int credis_buffer_push_null_arg(CRedisBuffer* buffer)
{
    SCOPED_TRACE;

    return CHECK_NOTHROW_INT_R(buffer->push_arg(nullptr));
}

REDEMPTION_LIB_EXPORT
int credis_buffer_push_arg_size(CRedisBuffer* buffer, uint32_t length)
{
    SCOPED_TRACE;

    return CHECK_NOTHROW_INT_R(buffer->push_arg_size(length));
}

REDEMPTION_LIB_EXPORT
int credis_buffer_push_arg_separator(CRedisBuffer* buffer)
{
    SCOPED_TRACE;

    return CHECK_NOTHROW_INT_R(buffer->push_arg_separator());
}

REDEMPTION_LIB_EXPORT
int credis_buffer_push_raw_data(CRedisBuffer* buffer,
                                char const* value, std::size_t length)
{
    SCOPED_TRACE;

    return CHECK_NOTHROW_INT_R(buffer->push_raw_data({value, length}));
}

REDEMPTION_LIB_EXPORT
void credis_buffer_clear(CRedisBuffer* buffer)
{
    SCOPED_TRACE;

    buffer->clear();
}

REDEMPTION_LIB_EXPORT
void credis_buffer_free(CRedisBuffer* buffer)
{
    SCOPED_TRACE;

    buffer->free();
}

REDEMPTION_LIB_EXPORT
char* credis_buffer_get_data(CRedisBuffer* buffer, std::size_t* output_length)
{
    SCOPED_TRACE;

    auto av = buffer->buffer();
    *output_length = av.size();
    return av.as_chars().data();
}

REDEMPTION_LIB_EXPORT
int credis_buffer_push_cmd_auth(CRedisBuffer* buffer, char const* password)
{
    SCOPED_TRACE;

    auto passlen = strlen(password);
    return CHECK_NOTHROW_INT_R(
        credis_buffer_push_avs(
            *buffer,
            "*2\r\n$4\r\nAUTH\r\n$"_av,
            int_to_decimal_chars(passlen),
            "\r\n"_av,
            bytes_view{password, passlen},
            "\r\n"_av
        )
    );
}

REDEMPTION_LIB_EXPORT
int credis_buffer_push_cmd_select_db(CRedisBuffer* buffer, unsigned db)
{
    SCOPED_TRACE;

    auto db_s = int_to_decimal_chars(db);
    return CHECK_NOTHROW_INT_R(
        credis_buffer_push_avs(
            *buffer,
            "*2\r\n$6\r\nSELECT\r\n$"_av,
            int_to_decimal_chars(db_s.size()),
            "\r\n"_av, db_s, "\r\n"_av
        )
    );
}

REDEMPTION_LIB_EXPORT
char* credis_buffer_build_with_prefix_and_suffix(
    CRedisBuffer* buffer,
    char const* prefix, std::size_t prefix_length,
    char const* suffix, std::size_t suffix_length,
    std::size_t* output_length)
{
    SCOPED_TRACE;

    auto av_prefix = prefix ? bytes_view{prefix, prefix_length} : bytes_view{"", 0};
    auto av_suffix = suffix ? bytes_view{suffix, suffix_length} : bytes_view{"", 0};

    if (buffer->is_valid_prefix(av_prefix) && buffer->is_valid_suffix(av_suffix)) {
        auto av = buffer->build_with_prefix_and_suffix(av_prefix, av_suffix);
        *output_length = av.size();
        return av.as_chars().data();
    }

    *output_length = 0;
    return nullptr;
}

} // extern "C"


struct CRedisCmdSet : CRedisBuffer
{
    using CRedisBuffer::CRedisBuffer;

    bytes_view prefix() const noexcept
    {
        return {start_data, prefix_len};
    }

    bytes_view suffix() const noexcept
    {
        return {start_data + prefix_len, suffix_len};
    }

    uint8_t* start_data;
    std::size_t prefix_len;
    std::size_t suffix_len;
};

namespace
{
    uint8_t* push_data(uint8_t* p, bytes_view av) noexcept
    {
        std::memcpy(p, av.data(), av.size());
        return p + av.size();
    };
} // anonymous namespace

extern "C"
{

REDEMPTION_LIB_EXPORT
CRedisCmdSet* credis_cmd_set_new(char const* key_name,
                                 unsigned expiration_delay_in_seconds,
                                 std::size_t start_capacity)
{
    SCOPED_TRACE;

    chars_view key {key_name, strlen(key_name)};

    auto cmd_set0 = "*5\r\n$3\r\nSET\r\n$"_av;
    auto cmd_set1 = int_to_decimal_chars(key.size());
    auto cmd_set2 = "\r\n"_av;
    auto cmd_set3 = bytes_view(key);
    auto cmd_set4 = "\r\n$"_av;

    auto expiration_delay_as_str = int_to_decimal_chars(expiration_delay_in_seconds);
    auto ex_arg0 = "\r\n$2\r\nEX\r\n$"_av;
    auto ex_arg1 = int_to_decimal_chars(expiration_delay_as_str.size());
    auto ex_arg2 = "\r\n"_av;
    auto ex_arg3 = bytes_view(expiration_delay_as_str);
    auto ex_arg4 = "\r\n"_av;

    std::size_t prefix_len = cmd_set0.size()
                           + cmd_set1.size()
                           + cmd_set2.size()
                           + cmd_set3.size()
                           + cmd_set4.size();

    std::size_t suffix_len = ex_arg0.size()
                           + ex_arg1.size()
                           + ex_arg2.size()
                           + ex_arg3.size()
                           + ex_arg4.size();

    std::size_t reserved = prefix_len
                         + suffix_len
                         + alignof(CRedisCmdSet)
                         + sizeof(CRedisCmdSet);

    void* const raw_allocated = ::operator new(reserved, std::nothrow);
    if (!raw_allocated) {
        return nullptr;
    }

    uint8_t* p = static_cast<uint8_t*>(raw_allocated);

    p = push_data(p, cmd_set0);
    p = push_data(p, cmd_set1);
    p = push_data(p, cmd_set2);
    p = push_data(p, cmd_set3);
    p = push_data(p, cmd_set4);

    p = push_data(p, ex_arg0);
    p = push_data(p, ex_arg1);
    p = push_data(p, ex_arg2);
    p = push_data(p, ex_arg3);
    p = push_data(p, ex_arg4);

    auto remaining = reserved - checked_cast<std::size_t>(p - static_cast<uint8_t*>(raw_allocated));
    void* voidp = p;

    void* voidp2 = std::align(alignof(CRedisCmdSet), sizeof(CRedisCmdSet), voidp, remaining);
    if (!voidp2) {
        ::operator delete(raw_allocated);
        return nullptr;
    }

    CRedisCmdSet* ret;

    std::size_t cmd_set_end_len = detail::int_to_chars_buf_size + 2 /* number + \r\n */;
    CHECK_NOTHROW(
        ret = new(voidp2) CRedisCmdSet(start_capacity,
                                       prefix_len + cmd_set_end_len,
                                       suffix_len),
        ::operator delete(raw_allocated),
        nullptr
    );

    ret->start_data = static_cast<uint8_t*>(raw_allocated);
    ret->prefix_len = prefix_len;
    ret->suffix_len = suffix_len;

    return ret;
}

REDEMPTION_LIB_EXPORT
void credis_cmd_set_delete(CRedisCmdSet* cmd)
{
    SCOPED_TRACE;

    void* p = cmd->start_data;
    cmd->~CRedisCmdSet();
    ::operator delete(p);
}

REDEMPTION_LIB_EXPORT
CRedisBuffer* credis_cmd_set_get_buffer(CRedisCmdSet* cmd)
{
    SCOPED_TRACE;

    return cmd;
}

REDEMPTION_LIB_EXPORT
int credis_cmd_set_free_buffer(CRedisCmdSet* cmd, std::size_t start_capacity)
{
    SCOPED_TRACE;

    cmd->free();
    std::size_t cmd_set_end_len = detail::int_to_chars_buf_size + 2 /* number + \r\n */;

    return CHECK_NOTHROW_INT_R(
        cmd->reset(start_capacity,
                   cmd->prefix_len + cmd_set_end_len,
                   cmd->suffix_len)
    );
}

REDEMPTION_LIB_EXPORT
char* credis_cmd_set_build_command(CRedisCmdSet* cmd, std::size_t* output_length)
{
    SCOPED_TRACE;

    auto av = cmd->buffer();
    auto av_len_s = int_to_decimal_chars(av.size());

    auto prefix = cmd->prefix();
    uint8_t* data = av.data() - (prefix.size() + av_len_s.size() + 2);

    // push header
    uint8_t* p = data;
    p = push_data(p, prefix);
    p = push_data(p, av_len_s);
    p = push_data(p, "\r\n"_av);

    // push ex arg
    p = av.end();
    p = push_data(p, cmd->suffix());

    *output_length = checked_int(p - data);
    return char_ptr_cast(data);
}

} // extern "C"


struct CRedisTransport
{
    static constexpr std::string_view resp_ok = "+OK\r\n";

    RedisWriter writer;
    char const* err_msg = nullptr;
    char rep[resp_ok.size() + 1] {};
    std::size_t rep_len = 0;
};

namespace
{
    static CRedisTransportCode to_credis_code(RedisIOCode code) noexcept
    {
        switch (code) {
            case RedisIOCode::Ok: return CRedisTransportCode::Ok;
            case RedisIOCode::WantRead: return CRedisTransportCode::WantRead;
            case RedisIOCode::WantWrite: return CRedisTransportCode::WantWrite;
            case RedisIOCode::ConnectError: return CRedisTransportCode::ConnectError;
            case RedisIOCode::CertificateError: return CRedisTransportCode::CertificateError;
            case RedisIOCode::ReadError: return CRedisTransportCode::ReadError;
            case RedisIOCode::WriteError: return CRedisTransportCode::WriteError;
            case RedisIOCode::Timeout: return CRedisTransportCode::Timeout;
            case RedisIOCode::UnknownResponse: return CRedisTransportCode::UnknownResponse;
        }
        return CRedisTransportCode::UnknownError;
    }
} // anonymous namespace

extern "C"
{

REDEMPTION_LIB_EXPORT
CRedisTransport* credis_transport_new()
{
    SCOPED_TRACE;

    return new(std::nothrow) CRedisTransport{};
}

REDEMPTION_LIB_EXPORT
void credis_transport_delete(CRedisTransport* redis)
{
    delete redis;
}

REDEMPTION_LIB_EXPORT
char const* credis_transport_get_last_error_message(CRedisTransport* redis)
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
CRedisTransportCode credis_transport_set_fd(CRedisTransport* redis, int fd)
{
    SCOPED_TRACE;

    redis->writer.set_fd(fd);
    if (fd >= 0) {
        return CRedisTransportCode::Ok;
    }

    redis->err_msg = "invalid fd";
    return CRedisTransportCode::InvalidFd;
}

REDEMPTION_LIB_EXPORT
CRedisTransportCode credis_transport_enable_tls(CRedisTransport* redis,
                                                char const* ca_cert_file,
                                                char const* cert_file,
                                                char const* key_file)
{
    SCOPED_TRACE;

    redis->err_msg = redis->writer.enable_tls(ca_cert_file, cert_file, key_file);
    if (!redis->err_msg) {
        return CRedisTransportCode::Ok;
    }
    return CRedisTransportCode::CertificateError;
}

REDEMPTION_LIB_EXPORT
CRedisTransportCode credis_transport_ssl_connect(CRedisTransport* redis)
{
    SCOPED_TRACE;
    return to_credis_code(redis->writer.ssl_connect());
}

REDEMPTION_LIB_EXPORT
CRedisTransportCode credis_transport_read(CRedisTransport* redis,
                                          uint8_t* buffer,
                                          std::size_t length,
                                          std::size_t* output_length)
{
    SCOPED_TRACE;

    auto result = redis->writer.recv(writable_bytes_view{buffer, length});
    *output_length = result.len;
    return to_credis_code(result.code);
}

REDEMPTION_LIB_EXPORT
CRedisTransportCode credis_transport_write(CRedisTransport* redis,
                                           uint8_t const* buffer,
                                           std::size_t length,
                                           std::size_t* output_length)
{
    SCOPED_TRACE;

    auto result = redis->writer.send(bytes_view{buffer, length});
    *output_length = result.len;
    return to_credis_code(result.code);
}

REDEMPTION_LIB_EXPORT
CRedisTransportCode credis_transport_read_response_ok(CRedisTransport* redis)
{
    SCOPED_TRACE;

    constexpr auto expected_resp = CRedisTransport::resp_ok;

    writable_bytes_view buf{redis->rep, expected_resp.size()};

    auto result = redis->writer.recv(buf.drop_front(redis->rep_len));
    redis->rep_len += result.len;
    if (bool(result.code)) {
        return to_credis_code(result.code);
    }

    if (result.len > 0 && redis->rep_len == expected_resp.size()) {
        if (chars_view{redis->rep, expected_resp.size()}.as<std::string_view>() == expected_resp) {
            redis->rep_len = 0;
            return CRedisTransportCode::Ok;
        }

        redis->rep[redis->rep_len] = 0;
        redis->rep_len = 0;
        redis->err_msg = redis->rep;
        return CRedisTransportCode::UnknownResponse;
    }

    return CRedisTransportCode::WantRead;
}

}
