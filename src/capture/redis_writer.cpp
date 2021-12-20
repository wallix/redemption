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
#include "cxx/cxx.hpp"

#include <openssl/ssl.h>
#include <openssl/err.h>

#include <sys/socket.h>


// redis format:
// '*' {nb_args} '\r\n'
// ( '$' {arg_len} '\r\n' {arg_value} '\r\n' )*
// Command:
// SET key value [EX second]
// SELECT db
// AUTH password
// AUTH username password

RedisCmdSet::RedisCmdSet(chars_view key_name, std::chrono::seconds expiration_delay)
{
    // buffer contents:
    // [EXPIRE value][SET cmd without size nor data][reserved formatted header for SET][data]
    cmd_buffer.reserve(32 * 1024);

    // EXPIRE value
    auto expiration_delay_as_str = int_to_decimal_chars(expiration_delay.count());
    append("\r\n$2\r\nEX\r\n$"_av);
    append(int_to_decimal_chars(expiration_delay_as_str.size()));
    append("\r\n"_av);
    append(expiration_delay_as_str);
    append("\r\n"_av);

    preformatted_header_set_pos = cmd_buffer.size();

    // preformatted header for SET command
    append("*5\r\n$3\r\nSET\r\n$"_av);
    append(int_to_decimal_chars(key_name.size()));
    append("\r\n"_av);
    append(key_name);
    append("\r\n$"_av);

    preformatted_size = cmd_buffer.size();
    clear();
}

std::size_t RedisCmdSet::reserved_buffer_size() noexcept
{
    // [EXPIRE] [header for SET] [reserved size of SET] [separator]
    return preformatted_header_set_pos
        + (preformatted_size - preformatted_header_set_pos) * 2
        + detail::int_to_chars_buf_size + 2;
}

void RedisCmdSet::clear()
{
    cmd_buffer.resize(reserved_buffer_size());
}

void RedisCmdSet::append(bytes_view data)
{
    cmd_buffer.insert(cmd_buffer.end(), data.begin(), data.end());
}

bytes_view RedisCmdSet::build_command()
{
    auto copy = [&](uint8_t* p, bytes_view av) {
        memcpy(p, av.data(), av.size());
        return p + av.size();
    };

    constexpr auto field_sep = "\r\n"_av;

    auto start_data_pos = reserved_buffer_size();
    auto data_len = cmd_buffer.size() - start_data_pos;

    // push EXPIRE command
    auto n = cmd_buffer.size();
    // note: cmd_buffer.insert(cmd_buffer.end(), {cmd_buffer.data(), preformatted_header_set_pos})
    // is invalid with -D_GLIBCXX_DEBUG (same after cmd_buffer.reserve()):
    // Error: attempt to insert with an iterator range [__first, __last) from this container.
    // maybe a bug of libstdc++ ?
    cmd_buffer.resize(n + preformatted_header_set_pos);
    copy(cmd_buffer.data() + n, {cmd_buffer.data(), preformatted_header_set_pos});

    auto data_len_as_str = int_to_decimal_chars(data_len);

    auto preformatted_header_size = preformatted_size - preformatted_header_set_pos;
    auto preformatted_header_for_set_cmd = cmd_buffer.data() + preformatted_header_set_pos;
    auto set_cmd_size = preformatted_header_size + data_len_as_str.size() + field_sep.size();
    auto header_for_set_cmd = cmd_buffer.data() + start_data_pos - set_cmd_size;

    // push SET command
    auto* p = header_for_set_cmd;
    p = copy(p, {preformatted_header_for_set_cmd, preformatted_header_size});
    p = copy(p, data_len_as_str);
    p = copy(p, field_sep);

    return {header_for_set_cmd, cmd_buffer.data() + cmd_buffer.size()};
}


RedisWriter::RedisWriter() = default;

RedisWriter::~RedisWriter()
{
    close();
}

RedisWriter::IOResult::IOResult(Code code, ErrorCtx error_ctx) noexcept
: code_(code)
, data_{.error_ctx = error_ctx}
{
    assert(code != Code::UnknownResponse);
}

RedisWriter::IOResult::IOResult(bounded_array_view<char, 0, 5> resp) noexcept
: code_(Code::UnknownResponse)
, data_{.resp = {}}
{
    memcpy(data_.resp, resp.data(), resp.size());
}

RedisWriter::IOResult RedisWriter::IOResult::Unknown(bounded_array_view<char, 0, 5> resp) noexcept
{
    return IOResult(resp);
}

int RedisWriter::IOResult::errnum() const noexcept
{
    if (code() != Code::UnknownResponse) {
        return data_.error_ctx.errnum;
    }
    return 0;
}

const char * RedisWriter::IOResult::code_as_cstring() const noexcept
{
    switch (code()) {
        case Code::Ok: return "Ok";
        case Code::ConnectError: return "Connect";
        case Code::CertificateError: return "Certificate";
        case Code::ReadError: return "Read";
        case Code::WriteError: return "Write";
        case Code::Timeout: return "Timeout";
        case Code::UnknownResponse: return "Unknown response";
    }
    REDEMPTION_UNREACHABLE();
}

const char * RedisWriter::IOResult::error_message() const
{
    if (code() != Code::UnknownResponse) {
        if (data_.error_ctx.msg) {
            return data_.error_ctx.msg;
        }
        if (data_.error_ctx.errnum) {
            return strerror(data_.error_ctx.errnum);
        }
        if (data_.error_ctx.sslnum) {
            return ERR_reason_error_string(checked_int{data_.error_ctx.sslnum});
        }
        return "unknown error";
    }

    return data_.resp;
}

namespace
{

using IOResult = RedisWriter::IOResult;
using ErrorCtx = IOResult::ErrorCtx;

template<class F>
int redis_tls_func(int fd, SSL* ssl, timeval tv,
                   fd_set& read_fds, fd_set& write_fds,
                   IOResult& result, IOResult::Code error_code, F&& f)
{
    for (;;) {
        int ret = f();
        if (ret > 0) {
            return ret;
        }

        int ssl_error = SSL_get_error(ssl, ret);

        fd_set* rfds = nullptr;
        fd_set* wfds = nullptr;

        if (ssl_error == SSL_ERROR_WANT_READ) {
            rfds = &read_fds;
            io_fd_set(fd, read_fds);
        }
        else if (ssl_error == SSL_ERROR_WANT_WRITE) {
            wfds = &write_fds;
            io_fd_set(fd, write_fds);
        }
        else {
            ErrorCtx error_ctx = (ssl_error == SSL_ERROR_SYSCALL)
                ? ErrorCtx{.errnum = errno}
                : ErrorCtx{.sslnum = ssl_error};
            result = IOResult(error_code, error_ctx);
            return -1;
        }

        auto tv_remaining = tv;
        int nfds = select(fd+1, rfds, wfds, nullptr, &tv_remaining);

        if (nfds > 0) {
        }
        else if (nfds == 0) {
            result = IOResult(IOResult::Code::Timeout, ErrorCtx{.errnum = errno});
            return -1;
        }
        else if (errno != EINTR && errno != EAGAIN) {
            // possibly EINVAL -> negative timeout
            result = IOResult(error_code, ErrorCtx{.errnum = errno});
            return -1;
        }
    }
}

IOResult redis_send_on_fd(int fd, bytes_view buffer, timeval tv, fd_set& write_fds)
{
    auto* p = buffer.data();
    auto len = buffer.size();

    for (;;) {
        io_fd_set(fd, write_fds);
        auto tv_remaining = tv;
        int nfds = select(fd+1, nullptr, &write_fds, nullptr, &tv_remaining);
        if (nfds > 0) {
            ssize_t res = ::send(fd, p, len, 0);
            if (REDEMPTION_UNLIKELY(res == -1)) {
                if (errno == EAGAIN || errno == EINTR) {
                    res = 0;
                }
                return IOResult(IOResult::Code::WriteError, ErrorCtx{.errnum = errno});
            }
            if (std::size_t(res) == len) {
                return IOResult::Ok();
            }
            p += res;
            len -= std::size_t(res);
        }
        else if (nfds == 0) {
            return IOResult(IOResult::Code::Timeout, ErrorCtx{.errnum = errno});
        }
        else if (errno != EINTR && errno != EAGAIN) {
            // possibly EINVAL -> negative timeout
            return IOResult(IOResult::Code::WriteError, ErrorCtx{.errnum = errno});
        }
    }
}

IOResult redis_send_on_ssl(int fd, SSL* ssl, bytes_view buffer, timeval tv,
                           fd_set& read_fds, fd_set& write_fds)
{
    auto error = IOResult::Ok();
    auto* p = buffer.data();
    int len = checked_int{buffer.size()};
    redis_tls_func(fd, ssl, tv, read_fds, write_fds, error, IOResult::Code::WriteError, [&]{
       return SSL_write(ssl, p, len);
    });
    return error;
}

IOResult redis_send(int fd, SSL* ssl, bytes_view buffer, timeval tv, fd_set& read_fds, fd_set& write_fds)
{
    if (!ssl) {
        return redis_send_on_fd(fd, buffer, tv, write_fds);
    }
    return redis_send_on_ssl(fd, ssl, buffer, tv, read_fds, write_fds);
}


ssize_t redis_recv_on_fd(int fd, writable_bytes_view buffer, timeval tv, fd_set& fds, IOResult& error)
{
    for (;;) {
        io_fd_set(fd, fds);
        auto tv_remaining = tv;
        int nfds = select(fd+1, &fds, nullptr, nullptr, &tv_remaining);
        if (nfds > 0) {
            using namespace std::string_view_literals;
            ssize_t res = ::recv(fd, buffer.data(), buffer.size(), 0);
            if (res > 0) {
                return res;
            }
            else if (res == -1) {
                if (errno != EAGAIN && errno != EINTR) {
                    error = IOResult(IOResult::Code::ReadError, ErrorCtx{.errnum = errno});
                    return -1;
                }
            }
            else {
                error = IOResult(IOResult::Code::Timeout, ErrorCtx{.errnum = errno});
                return -1;
            }
        }
        else if (nfds == 0) {
            error = IOResult(IOResult::Code::Timeout, ErrorCtx{.errnum = errno});
            return -1;
        }
        else if (errno != EINTR) {
            error = IOResult(IOResult::Code::ReadError, ErrorCtx{.errnum = errno});
            return -1;
        }
    }
}

int redis_recv_on_ssl(int fd, SSL* ssl, writable_bytes_view buffer, timeval tv,
                          fd_set& read_fds, fd_set& write_fds, IOResult& error)
{
    int res = -1;
    auto* p = buffer.data();
    int len = checked_int{buffer.size()};
    return redis_tls_func(fd, ssl, tv, read_fds, write_fds, error, IOResult::Code::ReadError, [&]{
       res = SSL_read(ssl, p, len);
       return res;
    });
}

IOResult receive_response(int fd, SSL* ssl, timeval tv, fd_set& read_fds, fd_set& write_fds)
{
    using namespace std::string_view_literals;
    constexpr auto expected_resp = "+OK\r\n"sv;
    char buffer[expected_resp.size()];

    writable_bytes_view data = make_writable_array_view(buffer);

    auto error = IOResult::Ok();

    ssize_t ret;
    for (;;) {
        if (!ssl) {
            ret = redis_recv_on_fd(fd, data, tv, write_fds, error);
        }
        else {
            ret = redis_recv_on_ssl(fd, ssl, data, tv, read_fds, write_fds, error);
        }

        if (ret < 0) {
            return error;
        }

        data = data.drop_front(checked_int{ret});
        if (data.empty()) {
            break;
        }
    }

    if (std::string_view(buffer, expected_resp.size()) == expected_resp) {
        return IOResult::Ok();
    }

    return IOResult::Unknown(make_sized_array_view(buffer));
}

} // anonymous namespace

struct RedisWriter::Access
{
    static SSL* as_ssl(RedisTlsCtx& self) noexcept
    {
        return static_cast<SSL*>(self.ssl);
    }

    static SSL_CTX* as_ssl_ctx(RedisTlsCtx& self) noexcept
    {
        return static_cast<SSL_CTX*>(self.ssl_ctx);
    }
};

char const* RedisWriter::RedisTlsCtx::open(
    const char *cacert_filename,
    const char *cert_filename,
    const char *private_key_filename,
    int fd)
{
#define CHECK(x, msg) do { if (REDEMPTION_UNLIKELY(x)) { return msg; } } while (0)

    // Create context

    auto* ssl_ctx = SSL_CTX_new(SSLv23_client_method());
    this->ssl_ctx = ssl_ctx;
    CHECK(!ssl_ctx, "ctx create");

    SSL_CTX_set_options(ssl_ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);
    SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_PEER, nullptr);

    CHECK(!SSL_CTX_load_verify_locations(ssl_ctx, cacert_filename, nullptr),
        "ca_cert load");

    CHECK(!SSL_CTX_use_certificate_chain_file(ssl_ctx, cert_filename),
        "client_cert load");

    CHECK(!SSL_CTX_use_PrivateKey_file(ssl_ctx, private_key_filename, SSL_FILETYPE_PEM),
        "private_key load");

    // Connect

    auto* ssl = SSL_new(ssl_ctx);
    this->ssl = ssl;
    CHECK(!ssl_ctx, "ssl create");

    // SSL_set_mode(ssl, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);
    SSL_clear_mode(ssl, SSL_MODE_AUTO_RETRY);
    SSL_set_fd(ssl, fd);
    SSL_set_connect_state(ssl);

    ERR_clear_error();

    return nullptr;

#undef CHECK
}

void RedisWriter::RedisTlsCtx::close()
{
    if (ssl) SSL_free(Access::as_ssl(*this));
    if (ssl_ctx) SSL_CTX_free(Access::as_ssl_ctx(*this));
    ssl = nullptr;
    ssl_ctx = nullptr;
}


RedisWriter::IOResult RedisWriter::open(
    zstring_view address, unsigned port,
    zstring_view password, unsigned db,
    std::chrono::milliseconds timeout, TlsParams tls_params)
{
    tv = to_timeval(timeout);

    // open socket
    close();
    fd = ip_connect_blocking(address, checked_int(port)).release();
    if (fd == -1) {
        return IOResult(IOResult::Code::ConnectError, ErrorCtx{.errnum = errno});
    }

    // enable tls
    if (tls_params.enable_tls) {
        auto* error_msg = tls.open(
            tls_params.ca_cert_file,
            tls_params.cert_file,
            tls_params.key_file,
            fd);
        if (error_msg) {
            return IOResult(IOResult::Code::CertificateError, ErrorCtx{.msg = error_msg});
        }

        IOResult result = IOResult::Ok();
        redis_tls_func(fd, Access::as_ssl(tls), tv, rfds, wfds, result, IOResult::Code::ConnectError, [&]{
            return SSL_connect(Access::as_ssl(tls));
        });

        if (not result.ok()) {
            return result;
        }
    }

    // send data:
    // AUTH password
    // SELECT db
    auto db_s = int_to_decimal_chars(db);
    auto truncated_password = bounded_chars_view<0, 256>(truncated_bounded_array_view(password));
    auto data = static_str_concat(
        "*2\r\n$4\r\nAUTH\r\n$"_sized_av,
        int_to_decimal_chars(truncated_password.size()),
        "\r\n"_sized_av, truncated_password, "\r\n"
        "*2\r\n$6\r\nSELECT\r\n$"_sized_av,
        int_to_decimal_chars(db_s.size()),
        "\r\n"_sized_av, db_s, "\r\n"_sized_av
    );
    bytes_view av = data;

    if (password.empty()) {
        av = av.from_offset(20);
        state = State::WaitResponse;
    }
    else {
        state = State::WaitPassword;
    }

    io_fd_zero(rfds);
    io_fd_zero(wfds);

    return redis_send(fd, Access::as_ssl(tls), av, tv, rfds, wfds);
}

void RedisWriter::close()
{
    if (fd != -1) {
        ::close(fd);
        fd = -1;
    }
    tls.close();
}

RedisWriter::IOResult RedisWriter::send(bytes_view data)
{
    auto* ssl = Access::as_ssl(tls);

    if (state == State::WaitPassword) {
        auto res = receive_response(fd, ssl, tv, rfds, wfds);
        if (not res.ok()) {
            return res;
        }
        state = State::WaitResponse;
    }

    auto res = receive_response(fd, ssl, tv, rfds, wfds);
    if (not res.ok()) {
        return res;
    }

    return redis_send(fd, ssl, data, tv, rfds, wfds);
}
