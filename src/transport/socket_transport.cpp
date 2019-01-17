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
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean, Raphael Zhou, Meng Tan

   Transport layer abstraction, socket implementation with TLS support
*/

#include "transport/socket_transport.hpp"
#include "utils/netutils.hpp"
#include "utils/hexdump.hpp"
#include "utils/select.hpp"
#include "utils/difftimeval.hpp"
#include "system/tls_context.hpp"

#include <vector>
#include <cstring>
#include <memory>

#include <sys/types.h>
#include <sys/socket.h>


namespace
{
    ssize_t tls_recv_all(TLSContext & tls, uint8_t * data, size_t const len);
    ssize_t socket_recv_all(int sck, char const * name, uint8_t * data, size_t const len,
        std::chrono::milliseconds recv_timeout);
    ssize_t socket_recv_partial(int sck, uint8_t * data, size_t const len);
    // ssize_t socket_send_all(int sck, const uint8_t * data, size_t len);
    ssize_t socket_send_partial(int sck, const uint8_t * data, size_t len);
}

SocketTransport::SocketTransport(
    const char * name, unique_fd sck, const char *ip_address, int port,
    std::chrono::milliseconds recv_timeout,
    Verbose verbose, std::string * error_message
)
    : sck(sck.release())
    , name(name)
    , port(port)
    , error_message(error_message)
    , tls(nullptr)
    , recv_timeout(recv_timeout)
    , verbose(verbose)
{
    if (bool(verbose)) {
        LOG(LOG_INFO, "SocketTransport: recv_timeout=%zu", size_t(recv_timeout.count()));
    }

    strncpy(this->ip_address, ip_address, sizeof(this->ip_address)-1);
    this->ip_address[127] = 0;
}

SocketTransport::~SocketTransport()
{
    if (this->sck > INVALID_SOCKET){
        this->disconnect(); /*NOLINT(clang-analyzer-optin.cplusplus.VirtualCall)*/
    }

    this->tls.reset();

    if (bool(verbose)) {
        LOG( LOG_INFO
           , "%s (%d): total_received=%" PRIu64 ", total_sent=%" PRIu64
           , this->name, this->sck, this->total_received, this->total_sent);
    }
}

bool SocketTransport::has_pending_data() const
{
    return this->tls && this->tls->pending_data();
}


bool SocketTransport::has_waiting_data() const
{
    return !this->async_buffers.empty();
}

array_view_const_u8 SocketTransport::get_public_key() const
{
    return this->tls ? this->tls->get_public_key() : nullptr;
}

void SocketTransport::enable_server_tls(const char * certificate_password,
                                        const char * ssl_cipher_list)
{
    if (this->tls != nullptr) {
        // TODO this should be an error, no need to commute two times to TLS
        return;
    }
    this->tls = std::make_unique<TLSContext>();

    LOG(LOG_INFO, "SocketTransport::enable_server_tls() start");

    this->tls->enable_server_tls(this->sck, certificate_password, ssl_cipher_list);

    LOG(LOG_INFO, "SocketTransport::enable_server_tls() done");
}

Transport::TlsResult SocketTransport::enable_client_tls(bool server_cert_store,
                                                        ServerCertCheck server_cert_check,
                                                        ServerNotifier & server_notifier,
                                                        const char * certif_path)
{
    Transport::TlsResult ret;

    switch (this->tls_state) {
        case TLSState::Uninit:
            LOG(LOG_INFO, "Client TLS start");
            this->tls = std::make_unique<TLSContext>();
            if (!this->tls->enable_client_tls_start(this->sck, this->error_message)) {
                return Transport::TlsResult::Fail;
            }
            this->tls_state = TLSState::Want;
            REDEMPTION_CXX_FALLTHROUGH;
        case TLSState::Want:
            ret = this->tls->enable_client_tls_loop(this->error_message);
            if (ret == Transport::TlsResult::Ok) {
                try {
                    bool ensure_server_certificate_match =
                        (server_cert_check == ServerCertCheck::fails_if_no_match_or_missing)
                      ||(server_cert_check == ServerCertCheck::fails_if_no_match_and_succeed_if_no_know);

                    bool ensure_server_certificate_exists =
                        (server_cert_check == ServerCertCheck::fails_if_no_match_or_missing)
                      ||(server_cert_check == ServerCertCheck::succeed_if_exists_and_fails_if_missing);

                    ret = this->tls->check_certificate(server_cert_store,
                                                       ensure_server_certificate_match,
                                                       ensure_server_certificate_exists,
                                                       server_notifier,
                                                       certif_path,
                                                       this->error_message,
                                                       this->ip_address,
                                                       this->port);
                    assert(ret != Transport::TlsResult::Want);
                }
                catch (...) {
                    this->tls_state = TLSState::Uninit;
                    // Disconnect tls if needed
                    this->tls.reset();
                    LOG(LOG_ERR, "SocketTransport::enable_client_tls() failed");
                    throw;
                }
                LOG(LOG_INFO, "SocketTransport::enable_client_tls() done");
                this->tls_state = TLSState::Ok;
            }
            return ret;
        case TLSState::Ok:
            // TODO this should be an error, no need to commute two times to TLS
            return Transport::TlsResult::Fail;
        default:
            LOG(LOG_ERR, "SocketTransport::%s() unhandled state for tls_state", __FUNCTION__);
            return Transport::TlsResult::Fail;
    }
}

bool SocketTransport::disconnect()
{
    if (0 == strcmp("127.0.0.1", this->ip_address)){
        // silent trace in the case of watchdog
        LOG(LOG_INFO, "Socket %s (%d) : closing connection\n", this->name, this->sck);
    }
    this->tls_state = TLSState::Uninit;
    // Disconnect tls if needed
    this->tls.reset();
    shutdown(this->sck, 2); // 2 = SHUT_RDWR
    close(this->sck);
    this->sck = INVALID_SOCKET;
    return true;
}

bool SocketTransport::connect()
{
    if (this->sck <= INVALID_SOCKET){
        this->sck = ip_connect(this->ip_address, this->port, 3, 1000).release();
    }
    return this->sck > INVALID_SOCKET;
}

size_t SocketTransport::do_partial_read(uint8_t * buffer, size_t len)
{
    if (bool(this->verbose & Verbose::dump)) {
        LOG(LOG_INFO, "Socket %s (%d) receiving %zu bytes", this->name, this->sck, len);
    }

    ssize_t const res = this->tls ? this->tls->privpartial_recv_tls(buffer, len) : socket_recv_partial(this->sck, buffer, len);

    if (res <= 0){
        throw Error(ERR_TRANSPORT_NO_MORE_DATA, 0);
    }

    if (res >= 0) {
        this->total_received += res;

        if (bool(this->verbose & Verbose::dump)) {
            LOG(LOG_INFO, "Recv done on %s (%d) %zd bytes", this->name, this->sck, res);
            hexdump_c(buffer, res);
            LOG(LOG_INFO, "Dump done on %s (%d) %zd bytes", this->name, this->sck, res);
        }
    }

    return res;
}

SocketTransport::Read SocketTransport::do_atomic_read(uint8_t * buffer, size_t len)
{
    if (bool(this->verbose & Verbose::dump)) {
        LOG(LOG_INFO, "Socket %s (%d) receiving %zu bytes", this->name, this->sck, len);
    }

    ssize_t res = this->tls ? tls_recv_all(*this->tls, buffer, len) : socket_recv_all(this->sck, this->name, buffer, len, this->recv_timeout);
    //std::cout << "res=" << int(res) << " len=" << int(len) <<  std::endl;

    // we properly reached end of file on a block boundary
    if (res == 0){
        return Read::Eof;
    }

    if (res < 0){
        throw Error(ERR_TRANSPORT_NO_MORE_DATA, 0);
    }

    if (static_cast<size_t>(res) < len){
        throw Error(ERR_TRANSPORT_NO_MORE_DATA, 0);
    }

    if (bool(this->verbose & Verbose::dump)) {
        LOG(LOG_INFO, "Recv done on %s (%d) %zu bytes", this->name, this->sck, len);
        hexdump_c(buffer, len);
        LOG(LOG_INFO, "Dump done on %s (%d) %zu bytes", this->name, this->sck, len);
    }

    this->total_received += len;
    return Read::Ok;
}

SocketTransport::AsyncBuf::AsyncBuf(const uint8_t* data, std::size_t len)
: data([](const uint8_t* data, std::size_t len){
        uint8_t * tmp = new uint8_t[len];
        memcpy(tmp, data, len);
        return tmp;
    }(data, len))
, p(this->data.get())
, e(this->p + len)
{
}

void SocketTransport::do_send(const uint8_t * const buffer, size_t const len)
{
    if (len == 0) { return; }

    if (!this->async_buffers.empty()) {
        this->async_buffers.emplace_back(buffer, len);
        return;
    }

    if (bool(this->verbose & Verbose::dump)) {
        LOG(LOG_INFO, "Sending on %s (%d) %zu bytes", this->name, this->sck, len);
        hexdump_c(buffer, len);
        LOG(LOG_INFO, "Sent dumped on %s (%d) %zu bytes", this->name, this->sck, len);
    }

    ssize_t res = (this->tls)
        ? this->tls->privpartial_send_tls(buffer, len)
        : socket_send_partial(this->sck, buffer, len);

    if (res <= 0) {
        LOG(LOG_WARNING,
            "SocketTransport::Send failed on %s (%d) errno=%d [%s]",
            this->name, this->sck, errno, strerror(errno));
        throw Error(ERR_TRANSPORT_WRITE_FAILED);
    }

    if (res < static_cast<ssize_t>(len)) {
        this->async_buffers.emplace_back(buffer + res, len - res);
    }

    this->total_sent += res;
}

void SocketTransport::send_waiting_data()
{
    auto first = begin(this->async_buffers);
    auto last = end(this->async_buffers);

    for (; first != last; ++first) {
        size_t const len = first->e - first->p;
        ssize_t res = (this->tls)
            ? this->tls->privpartial_send_tls(first->p, len)
            : socket_send_partial(this->sck, first->p, len);

        if (res <= 0) {
            LOG(LOG_WARNING,
                "SocketTransport::Send failed on %s (%d) errno=%d [%s]",
                this->name, this->sck, errno, strerror(errno));
            throw Error(ERR_TRANSPORT_WRITE_FAILED);
        }

        this->total_sent += res;

        if (res != static_cast<ssize_t>(len)) {
            first->p += res;
            break;
        }
    }

    this->async_buffers.erase(begin(this->async_buffers), first);
}

namespace
{
    ssize_t tls_recv_all(TLSContext & tls, uint8_t * data, size_t const len)
    {
        size_t remaining_len = len;
        while (remaining_len > 0) {
            ssize_t const res = tls.privpartial_recv_tls(data, remaining_len);

            if (res <= 0) {
                if (res == 0) {
                    if (len != remaining_len) {
                        LOG(LOG_WARNING, "TLS receive for %zu bytes, ZERO RETURN got %zu",
                            len, len - remaining_len);
                    }
                    return remaining_len - len;
                }
                return res;
            }

            remaining_len -= res;
            data += res;
        }

        return len;
    }

    ssize_t socket_recv_all(
        int sck, char const* name, uint8_t* data, size_t const len,
        std::chrono::milliseconds recv_timeout)
    {
        size_t remaining_len = len;

        while (remaining_len > 0) {
            ssize_t res = ::recv(sck, data, remaining_len, 0);
            switch (res) {
                case -1: /* error, maybe EAGAIN */
                    if (try_again(errno)) {
                        fd_set fds;
                        struct timeval time = { 0, 100000 };
                        io_fd_zero(fds);
                        io_fd_set(sck, fds);
                        ::select(sck + 1, &fds, nullptr, nullptr, &time);
                        continue;
                    }
                    if (len != remaining_len) {
                        return len - remaining_len;
                    }
                    // TODO replace this with actual error management, EOF is not even an option for sockets
                    return -1;
                case 0: /* no data received, socket closed */
                    // if we were not able to receive the amount of data required, this is an error
                    // not need to process the received data as it will end badly
                    return -1;
                default: /* some data received */
                    remaining_len -= res;
                    data += res;
                    if (remaining_len) {
                        fd_set fds;
                        struct timeval time = addusectimeval(recv_timeout, {0, 0});
                        io_fd_zero(fds);
                        io_fd_set(sck, fds);
                        int ret = ::select(sck + 1, &fds, nullptr, nullptr, &time);
                        if ((ret < 1) ||
                            !io_fd_isset(sck, fds)) {
                            LOG(LOG_ERR, "Recv fails on %s (%d) %zu bytes, ret=%d", name, sck, remaining_len, ret);
                            return -1;
                        }
                    }
                    break;
            }
        }

        return len;
    }

    ssize_t socket_recv_partial(int sck, uint8_t* data, size_t const len)
    {
        ssize_t const res = ::recv(sck, data, len, 0);
        switch (res) {
            case -1: /* error, maybe EAGAIN */
                return try_again(errno) ? 0 : -1;
            case 0: /* no data received, socket closed */
                return -1;
            default: /* some data received */
                return res;
        }
    }

    // ssize_t socket_send_all(int sck, const uint8_t* data, size_t len)
    // {
    //     size_t total = 0;
    //     while (total < len) {
    //         ssize_t sent = ::send(sck, data + total, len - total, 0);
    //         switch (sent) {
    //             case -1:
    //                 if (try_again(errno)) {
    //                     fd_set wfds;
    //                     struct timeval time = { 0, 10000 };
    //                     io_fd_zero(wfds);
    //                     io_fd_set(sck, wfds);
    //                     select(sck + 1, nullptr, &wfds, nullptr, &time);
    //                     continue;
    //                 }
    //                 return -1;
    //             case 0:
    //                 return -1;
    //             default:
    //                 total += sent;
    //         }
    //     }
    //     return len;
    // }

    ssize_t socket_send_partial(int sck, const uint8_t* data, size_t len)
    {
        ssize_t const sent = ::send(sck, data, len, 0);
        switch (sent) {
            case -1:
                return try_again(errno) ? sent : -1;
            case 0:
                return 0;
            default:
                return sent;
        }
    }
} // anonymous namespace
