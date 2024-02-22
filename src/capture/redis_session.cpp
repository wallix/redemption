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

#include "capture/redis_session.hpp"
#include "utils/sugar/int_to_chars.hpp"
#include "utils/static_string.hpp"
#include "utils/to_timeval.hpp"
#include "utils/netutils.hpp"
#include "utils/select.hpp"
#include "cxx/cxx.hpp"

#include <openssl/ssl.h>
#include <openssl/err.h>

#include <sys/socket.h>
#include <sys/select.h>


RedisSyncSession::IOCode RedisSyncSession::open(
    zstring_view address, unsigned int port,
    bounded_chars_view<0, 256> password, unsigned int db,
    std::chrono::milliseconds timeout, TlsParams tls_params)
{
    using IOCode = IOCode;

    tv = to_timeval(timeout);

    // open socket
    close();
    int fd = addr_connect(address, checked_int(port), false).release();
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
