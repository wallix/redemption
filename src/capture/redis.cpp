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

#include "capture/redis.hpp"
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
    // [EXPIRE][SET without size nor data][formatted header for SET][sep LEN/STR][...data...]
    //   init              init                     uninit               init      append()
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
    auto offset = reserved_buffer_size();
    cmd_buffer.resize(offset);

    // seperator
    cmd_buffer[offset - 2] = '\r';
    cmd_buffer[offset - 1] = '\n';
}

std::size_t RedisCmdSet::reserved_buffer_size() const noexcept
{
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

bytes_view RedisCmdSet::current_data() const noexcept
{
    return bytes_view(cmd_buffer).drop_front(reserved_buffer_size());
}

bytes_view RedisCmdSet::build_command()
{
    auto copy = [&](uint8_t* p, bytes_view av) {
        memcpy(p, av.data(), av.size());
        return p + av.size();
    };

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
    auto set_cmd_size = preformatted_header_size + data_len_as_str.size() + 2 /* sep */;
    auto header_for_set_cmd = cmd_buffer.data() + start_data_pos - set_cmd_size;

    // push SET command
    auto* p = header_for_set_cmd;
    p = copy(p, {preformatted_header_for_set_cmd, preformatted_header_size});
    p = copy(p, data_len_as_str);

    return {header_for_set_cmd, cmd_buffer.data() + cmd_buffer.size()};
}

RedisAuth::RedisAuth(bounded_chars_view<0, 256> password, unsigned db)
{
    // send data:
    // AUTH password
    // SELECT db
    auto db_s = int_to_decimal_chars(db);
    static_str_assign(
        buffer,
        "*2\r\n$4\r\nAUTH\r\n$"_sized_av,
        int_to_decimal_chars(password.size()),
        "\r\n"_sized_av, password, "\r\n"
        "*2\r\n$6\r\nSELECT\r\n$"_sized_av,
        int_to_decimal_chars(db_s.size()),
        "\r\n"_sized_av, db_s, "\r\n"_sized_av
    );

    if (password.empty()) {
        offset = 20;
        nb_command = 1;
    }
    else {
        offset = 0;
        nb_command = 2;
    }
}

bytes_view RedisAuth::packet() const noexcept
{
    return make_array_view(buffer).drop_front(offset);
}

struct RedisWriter::Access
{
    static SSL* as_ssl(Tls& self) noexcept
    {
        return static_cast<SSL*>(self.ssl);
    }

    static bool is_ssl(Tls const& self) noexcept
    {
        return self.ssl;
    }

    static SSL_CTX* as_ssl_ctx(Tls& self) noexcept
    {
        return static_cast<SSL_CTX*>(self.ssl_ctx);
    }
};


char const* RedisWriter::Tls::open(
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

void RedisWriter::Tls::close()
{
    if (ssl) SSL_free(Access::as_ssl(*this));
    if (ssl_ctx) SSL_CTX_free(Access::as_ssl_ctx(*this));
    ssl = nullptr;
    ssl_ctx = nullptr;
}

RedisWriter::Tls::~Tls()
{
    close();
}


void RedisWriter::set_fd(int fd)
{
    tls.close();
    this->fd = fd;
}

void RedisWriter::close()
{
    tls.close();
    this->fd = -1;
    ssl_errnum = 0;
}

char const* RedisWriter::enable_tls(
    const char* cacert_filename, const char* cert_filename, const char* private_key_filename)
{
    return tls.open(
        cacert_filename,
        cert_filename,
        private_key_filename,
        fd);
}

zstring_view redios_io_code_to_zstring(RedisIOCode code) noexcept
{
    switch (code) {
        case RedisIOCode::Ok: return "Ok"_zv;
        case RedisIOCode::WantRead: return "WantRead"_zv;
        case RedisIOCode::WantWrite: return "WantWrite"_zv;
        case RedisIOCode::ConnectError: return "ConnectError"_zv;
        case RedisIOCode::CertificateError: return "CertificateError"_zv;
        case RedisIOCode::ReadError: return "ReadError"_zv;
        case RedisIOCode::WriteError: return "WriteError"_zv;
        case RedisIOCode::Timeout: return "Timeout"_zv;
        case RedisIOCode::UnknownResponse: return "UnknownResponse"_zv;
    }
    REDEMPTION_UNREACHABLE();
}

RedisWriter::IOCode RedisWriter::ssl_connect()
{
    auto* ssl = Access::as_ssl(tls);
    int ret = SSL_connect(ssl);
    if (ret > 0) {
        return IOCode::Ok;
    }
    return ssl_result_to_io_code(ret, IOCode::ConnectError);
}

RedisWriter::IOResult RedisWriter::recv(writable_bytes_view buffer)
{
    auto* ssl = Access::as_ssl(tls);

    if (!ssl) {
        for (;;) {
            ssize_t res = ::recv(fd, buffer.data(), buffer.size(), 0);
            if (res >= 0) {
                return {IOCode::Ok, checked_int{res}};
            }
            if (errno != EAGAIN && errno != EINTR) {
                return {IOCode::ReadError, 0};
            }
        }
    }
    else {
        int res = SSL_read(ssl, buffer.data(), checked_int{buffer.size()});
        if (res > 0) {
            return {IOCode::Ok, checked_int{res}};
        }
        return {ssl_result_to_io_code(res, IOCode::ReadError), 0};
    }
}

RedisWriter::IOResult RedisWriter::send(bytes_view buffer)
{
    auto* ssl = Access::as_ssl(tls);

    if (!ssl) {
        for (;;) {
            ssize_t res = ::send(fd, buffer.data(), buffer.size(), 0);
            if (res >= 0) {
                return {IOCode::Ok, checked_int{res}};
            }
            if (errno != EAGAIN && errno != EINTR) {
                return {IOCode::WriteError, 0};
            }
        }
    }
    else {
        int res = SSL_write(ssl, buffer.data(), checked_int{buffer.size()});
        if (res > 0) {
            return {IOCode::Ok, checked_int{res}};
        }
        return {ssl_result_to_io_code(res, IOCode::WriteError), 0};
    }
}

const char * RedisWriter::get_last_error_message() const
{
    if (not Access::is_ssl(tls) || ssl_errnum == SSL_ERROR_SYSCALL) {
        return strerror(errno);
    }

    return ERR_reason_error_string(checked_int{ssl_errnum});
}

int RedisWriter::get_last_errno() const noexcept
{
    if (not Access::is_ssl(tls) || ssl_errnum == SSL_ERROR_SYSCALL) {
        return errno;
    }

    return 0;
}

RedisWriter::IOCode RedisWriter::ssl_result_to_io_code(int res, IOCode code)
{
    auto* ssl = Access::as_ssl(tls);
    int ssl_error = SSL_get_error(ssl, res);

    if (ssl_error == SSL_ERROR_WANT_WRITE) {
        return IOCode::WantWrite;
    }

    if (ssl_error == SSL_ERROR_WANT_READ) {
        return IOCode::WantRead;
    }

    ssl_errnum = ssl_error;
    return code;
}

RedisSyncSession::IOCode RedisSyncSession::open(
    zstring_view address, unsigned int port,
    bounded_chars_view<0, 256> password, unsigned int db,
    std::chrono::milliseconds timeout, TlsParams tls_params)
{
    using IOCode = IOCode;

    tv = to_timeval(timeout);

    // open socket
    close();
    int fd = ip_connect_blocking(address, checked_int(port)).release();
    if (fd == -1) {
        return IOCode::ConnectError;
    }

    writer.set_fd(fd);

    io_fd_zero(rfds);
    io_fd_zero(wfds);

    // enable tls
    if (tls_params.enable_tls) {
        error_msg = writer.enable_tls(tls_params.ca_cert_file,
                                      tls_params.cert_file,
                                      tls_params.key_file);
        if (error_msg) {
            return IOCode::CertificateError;
        }

        auto code = loop_event(
            [&](bytes_view){ return RedisWriter::IOResult{writer.ssl_connect(), 0}; },
            ""_av, IOCode::WantWrite
        );
        if (code != IOCode::Ok) {
            return code;
        }
    }

    RedisAuth auth(password, db);
    if (auth.count_command() == 2) {
        state = State::WaitPassword;
    }
    else {
        assert(auth.count_command() == 1);
        state = State::WaitResponse;
    }

    return send_impl(auth.packet());
}

void RedisSyncSession::close()
{
    error_msg = nullptr;
    int fd = writer.get_fd();
    writer.close();
    if (fd != -1) {
        ::close(fd);
    }
}

RedisSyncSession::IOCode RedisSyncSession::send(bytes_view buffer)
{
    assert(writer.get_fd() != -1);

    using namespace std::string_view_literals;
    constexpr auto expected_resp = "+OK\r\n"sv;
    static_assert(resp_buffer_len == expected_resp.size());

    for (;;) {
        auto remaining = make_writable_array_view(resp_buffer).first(expected_resp.size());
        auto code = recv_impl(remaining);

        if (code == IOCode::Ok) {
            auto resp = make_writable_array_view(resp_buffer);
            if (resp.first(expected_resp.size()).as<std::string_view>() != expected_resp) {
                error_msg = resp.data();
                resp.back() = '\0';
                return IOCode::UnknownResponse;
            }

            if (state == State::WaitResponse) {
                return send_impl(buffer);
            }
            state = State::WaitResponse;
        }
        else {
            return code;
        }
    }
}

int RedisSyncSession::get_last_errno() const noexcept
{
    return writer.get_last_errno();
}

char const* RedisSyncSession::get_last_error_message() const
{
    return error_msg ? error_msg : writer.get_last_error_message();
}

RedisSyncSession::IOCode RedisSyncSession::send_impl(bytes_view buffer)
{
    return loop_event(
        [&](bytes_view buffer){ return writer.send(buffer); },
        buffer, IOCode::WantWrite
    );
}

RedisSyncSession::IOCode RedisSyncSession::recv_impl(writable_bytes_view buffer)
{
    return loop_event(
        [&](writable_bytes_view buffer){ return writer.recv(buffer); },
        buffer, IOCode::WantRead
    );
}

template<class F, class Buffer>
RedisSyncSession::IOCode RedisSyncSession::loop_event(F&& f, Buffer buffer, IOCode code_for_waiting)
{
    using IOCode = IOCode;

    fd_set* rfds_ref = nullptr;
    fd_set* wfds_ref = nullptr;

    int fd = writer.get_fd();

    for (;;) {
        auto result = f(buffer);
        if (result.code == IOCode::Ok) {
            if (result.len == buffer.size()) {
                return result.code;
            }
            buffer = buffer.drop_front(result.len);
            result.code = code_for_waiting;
        }

        if (result.code == IOCode::WantRead) {
            rfds_ref = &rfds;
            io_fd_set(fd, rfds);
        }
        else if (result.code == IOCode::WantWrite) {
            wfds_ref = &wfds;
            io_fd_set(fd, wfds);
        }
        else {
            close();
            return result.code;
        }

        auto tv_remaining = tv;
        int nfds = select(fd+1, rfds_ref, wfds_ref, nullptr, &tv_remaining);

        if (nfds > 0) {
            // ok, continue
        }
        else if (nfds == 0 || (errno != EINTR && errno != EAGAIN)) {
            // possibly EINVAL -> negative timeout
            error_msg = strerror(errno);
            return (nfds == 0) ? IOCode::Timeout : IOCode::ConnectError;
        }
    }
}
