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

#pragma once

#include <unistd.h>
#include <fcntl.h>
#include <cinttypes>
#include <memory>
#include <string>

#include "system/openssl.hpp"
#include "core/defines.hpp"
#include "core/wait_obj.hpp"
#include "transport/transport.hpp"
#include "utils/netutils.hpp"
#include "utils/fileutils.hpp"

#include "utils/invalid_socket.hpp"

#include "utils/verbose_flags.hpp"

// X509_NAME_print_ex() prints a human readable version of nm to BIO out.
// Each line (for multiline formats) is indented by indent spaces.
// The output format can be extensively customised by use of the flags parameter.

class SocketTransport
: public Transport
{
    size_t total_sent = 0;
    size_t total_received = 0;

public:
    int sck;
    int sck_closed; // TODO replaced by sck == -1 ?
    const char * name;

    char ip_address[128];
    int  port;

    std::string * error_message;
    TLSContext * tls;

    REDEMPTION_VERBOSE_FLAGS(private, verbose)
    {
        none,
        dump = 0x100,
    };

    SocketTransport( const char * name, int sck, const char *ip_address, int port
                   , Verbose verbose, std::string * error_message = nullptr)
    : sck(sck)
    , sck_closed(0)
    , name(name)
    , port(port)
    , error_message(error_message)
    , tls(nullptr)
    , verbose(verbose)
    {
        strncpy(this->ip_address, ip_address, sizeof(this->ip_address)-1);
        this->ip_address[127] = 0;
    }

    ~SocketTransport() override {
        if (!this->sck_closed){
            this->disconnect();
        }

        if (this->tls) {
            if (this->tls->allocated_ssl) {
                //SSL_shutdown(this->tls->allocated_ssl);
                SSL_free(this->tls->allocated_ssl);
            }

            if (this->tls->allocated_ctx) {
                SSL_CTX_free(this->tls->allocated_ctx);
            }

            delete this->tls;
        }

        if (bool(verbose)) {
            LOG( LOG_INFO
               , "%s (%d): total_received=%" PRIu64 ", total_sent=%" PRIu64
               , this->name, this->sck, this->total_received, this->total_sent);
        }
    }

    bool is_set(wait_obj & obj, fd_set & rfds) const
    {
        return this->has_pending_data() || obj.is_set(this->sck, rfds);
    }

    bool has_pending_data() const
    {
        if (this->recv_buf_size - this->recv_buf_index) {
            return true;
        }
        return this->tls && SSL_pending(this->tls->allocated_ssl);
    }

    int get_fd() const override { return this->sck; }

    const uint8_t * get_public_key() const override {
        return this->tls ? this->tls->public_key.get() : nullptr;
    }

    size_t get_public_key_length() const override {
        return this->tls ? this->tls->public_key_length : 0;
    }

    void enable_server_tls(const char * certificate_password,
            const char * ssl_cipher_list) override {
        if (this->tls != nullptr) {
            // TODO this should be an error, no need to commute two times to TLS
            return;
        }
        this->tls = new TLSContext();

        LOG(LOG_INFO, "SocketTransport::enable_server_tls() start");

        this->tls->enable_server_tls(this->sck, certificate_password, ssl_cipher_list);

        LOG(LOG_INFO, "SocketTransport::enable_server_tls() done");
    }

    void enable_client_tls(bool server_cert_store,
                           ServerCertCheck server_cert_check,
                           ServerNotifier & server_notifier,
                           const char * certif_path
            ) override {

        if (this->tls != nullptr) {
            // TODO this should be an error, no need to commute two times to TLS
            return;
        }

        this->tls = new TLSContext();

        LOG(LOG_INFO, "Client TLS start");
        bool ensure_server_certificate_match =
            (server_cert_check == ServerCertCheck::fails_if_no_match_or_missing)
           ||(server_cert_check == ServerCertCheck::fails_if_no_match_and_succeed_if_no_know);

        bool ensure_server_certificate_exists =
            (server_cert_check == ServerCertCheck::fails_if_no_match_or_missing)
           ||(server_cert_check == ServerCertCheck::succeed_if_exists_and_fails_if_missing);

        try {
            this->tls->enable_client_tls(this->sck,
               server_cert_store,
               ensure_server_certificate_match,
               ensure_server_certificate_exists,
               server_notifier,
               certif_path,
               this->error_message,
               &this->ip_address[0],
               this->port);
        }
        catch (...) {
            // Disconnect tls if needed
            if (this->tls) {
                if (this->tls->allocated_ssl) {
                    SSL_free(this->tls->allocated_ssl);
                    this->tls->allocated_ssl = nullptr;
                }
                if (this->tls->allocated_ctx) {
                    SSL_CTX_free(this->tls->allocated_ctx);
                    this->tls->allocated_ctx = nullptr;
                }
                delete this->tls;
                this->tls = nullptr;
            }

            LOG(LOG_ERR, "SocketTransport::enable_client_tls() failed");
            throw;
        }

        LOG(LOG_INFO, "SocketTransport::enable_client_tls() done");
    }

    bool disconnect()override {
        if (0 == strcmp("127.0.0.1", this->ip_address)){
            // silent trace in the case of watchdog
            LOG(LOG_INFO, "Socket %s (%d) : closing connection\n", this->name, this->sck);
        }
        // Disconnect tls if needed
        if (this->tls) {
            if (this->tls->allocated_ssl) {
                SSL_free(this->tls->allocated_ssl);
                this->tls->allocated_ssl = nullptr;
            }
            if (this->tls->allocated_ctx) {
                SSL_CTX_free(this->tls->allocated_ctx);
                this->tls->allocated_ctx = nullptr;
            }
            delete this->tls;
            this->tls = nullptr;
        }
        shutdown(this->sck, 2);
        close(this->sck);
        this->sck = 0;
        this->sck_closed = 1;
        return true;
    }

    bool connect() override {
        if (this->sck_closed == 1){
            this->sck = ip_connect(this->ip_address, this->port, 3, 1000);
            this->sck_closed = 0;
        }
        return true;
    }

    bool can_recv()
    {
        if (this->recv_buf_size - this->recv_buf_index) {
            return true;
        }

        int rv = 0;
        fd_set rfds;

        io_fd_zero(rfds);
        if (this->sck > 0) {
            io_fd_set(this->sck, rfds);
            timeval time { 0, 0 };
            rv = select(this->sck + 1, &rfds, nullptr, nullptr, &time); /* don't wait */
            if (rv > 0) {
                int opt;
                unsigned int opt_len = sizeof(opt);

                if (getsockopt(this->sck, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&opt), &opt_len) == 0) {
                    rv = (opt == 0);
                }
            }
        }
        return rv;
    }

    size_t do_partial_read(uint8_t * buffer, size_t len) override
    {
        if (bool(this->verbose & Verbose::dump)) {
            LOG(LOG_INFO, "Socket %s (%d) receiving %zu bytes", this->name, this->sck, len);
        }

        ssize_t const res = this->privpartial_recv(buffer, len);

        if (res <= 0){
            throw Error(ERR_TRANSPORT_NO_MORE_DATA, 0);
        }

        if (res >= 0) {
            this->total_received += res;

            if (bool(this->verbose & Verbose::dump)) {
                LOG(LOG_INFO, "Recv done on %s (%d) %zu bytes", this->name, this->sck, res);
                hexdump_c(buffer, res);
                LOG(LOG_INFO, "Dump done on %s (%d) %zu bytes", this->name, this->sck, res);
            }
        }

        return res;
    }

    Read do_atomic_read(uint8_t * buffer, size_t len) override {
        if (bool(this->verbose & Verbose::dump)) {
            LOG(LOG_INFO, "Socket %s (%d) receiving %zu bytes", this->name, this->sck, len);
        }

        ssize_t res = this->privrecv(buffer, len);
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


    void do_send(const uint8_t * const buffer, size_t len) override {
        if (len == 0) { return; }

        if (bool(this->verbose & Verbose::dump)) {
            LOG(LOG_INFO, "Sending on %s (%d) %zu bytes", this->name, this->sck, len);
            hexdump_c(buffer, len);
            LOG(LOG_INFO, "Sent dumped on %s (%d) %zu bytes", this->name, this->sck, len);
        }

        ssize_t res = (this->tls) ? this->tls->privsend_tls(buffer, len) : this->privsend(buffer, len);
        if (res < 0) {
            LOG(LOG_WARNING,
                "SocketTransport::Send failed on %s (%d) errno=%u [%s]",
                this->name, this->sck, errno, strerror(errno));
            throw Error(ERR_TRANSPORT_WRITE_FAILED);
        }
        if (res < static_cast<ssize_t>(len)) {
            throw Error(ERR_TRANSPORT_NO_MORE_DATA);
        }

        this->total_sent += len;
    }

private:
    uint8_t recv_buf[64 * 1024];
    size_t recv_buf_size = 0;
    size_t recv_buf_index = 0;

    ssize_t privrecv(uint8_t * data, size_t const len)
    {
        size_t buf_remaining = this->recv_buf_size - this->recv_buf_index;
        if (len <= buf_remaining) {
            memcpy(data, this->recv_buf + this->recv_buf_index, len);
            this->recv_buf_index += len;
            return len;
        }

        size_t remaining_len = len;

        if (buf_remaining) {
            memcpy(data, this->recv_buf + this->recv_buf_index, buf_remaining);
            remaining_len -= buf_remaining;
            data += buf_remaining;
            this->recv_buf_index += buf_remaining;
        }

        if (this->tls) {
            while (remaining_len > 0) {
                ssize_t const res = this->tls->privpartial_recv_tls(
                    this->recv_buf, sizeof(this->recv_buf));

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

                if (remaining_len <= size_t(res)) {
                    memcpy(data, this->recv_buf, remaining_len);
                    this->recv_buf_size = res;
                    this->recv_buf_index = remaining_len;
                    remaining_len = 0;
                }
                else {
                    memcpy(data, this->recv_buf, res);
                    remaining_len -= res;
                    data += res;
                }
            }

            return len;
        }

        while (remaining_len > 0) {
            ssize_t res = ::recv(this->sck, this->recv_buf, sizeof(this->recv_buf), 0);
            switch (res) {
                case -1: /* error, maybe EAGAIN */
                    if (try_again(errno)) {
                        fd_set fds;
                        struct timeval time = { 0, 100000 };
                        io_fd_zero(fds);
                        io_fd_set(this->sck, fds);
                        ::select(this->sck + 1, &fds, nullptr, nullptr, &time);
                        continue;
                    }
                    if (len != remaining_len){
                        return len - remaining_len;
                    }
                    // TODO replace this with actual error management, EOF is not even an option for sockets
                    return -1;
                case 0: /* no data received, socket closed */
                    // if we were not able to receive the amount of data required, this is an error
                    // not need to process the received data as it will end badly
                    return -1;
                default: /* some data received */
                    if (remaining_len <= size_t(res)) {
                        memcpy(data, this->recv_buf, remaining_len);
                        this->recv_buf_size = res;
                        this->recv_buf_index = remaining_len;
                        remaining_len = 0;
                    }
                    else {
                        memcpy(data, this->recv_buf, res);
                        remaining_len -= res;
                        data += res;

                        fd_set fds;
                        struct timeval time = { 1, 0 };
                        io_fd_zero(fds);
                        io_fd_set(this->sck, fds);
                        if ((::select(this->sck + 1, &fds, nullptr, nullptr, &time) < 1) ||
                            !io_fd_isset(this->sck, fds)) {
                            LOG(LOG_ERR, "Recv fails on %s (%d) %zu bytes", this->name, this->sck, remaining_len);
                            return -1;
                        }
                    }
                break;
            }
        }

        return len;
    }

    ssize_t privpartial_recv(uint8_t * data, size_t const len)
    {
        size_t buf_remaining = this->recv_buf_size - this->recv_buf_index;
        if (len <= buf_remaining) {
            memcpy(data, this->recv_buf + this->recv_buf_index, len);
            this->recv_buf_index += len;
            return len;
        }

        if (buf_remaining) {
            memcpy(data, this->recv_buf + this->recv_buf_index, buf_remaining);
            data += buf_remaining;
            this->recv_buf_index += buf_remaining;
            return buf_remaining;
        }

        if (this->tls) {
            ssize_t const res = this->tls->privpartial_recv_tls(
                this->recv_buf, sizeof(this->recv_buf));

            if (res <= 0) {
                return res;
            }

            if (len <= size_t(res)) {
                memcpy(data, this->recv_buf, len);
                this->recv_buf_size = res;
                this->recv_buf_index = len;
            }
            else {
                memcpy(data, this->recv_buf, res);
                data += res;
            }

            return res;
        }

        ssize_t res = 0;

        // TODO (temporary) test on EAGAIN
        while ((res = ::recv(this->sck, this->recv_buf, sizeof(this->recv_buf), 0)) == -1
            && errno == EAGAIN) {
        }
        switch (res) {
            case -1: /* error, maybe EAGAIN */ {
                int err = errno;
                if (try_again(err)) {
                    return 0;
                }
                return -1;
            }
            case 0: /* no data received, socket closed */
                // if we were not able to receive the amount of data required, this is an error
                // not need to process the received data as it will end badly
                return -1;
            default: /* some data received */
                if (len <= size_t(res)) {
                    memcpy(data, this->recv_buf, len);
                    this->recv_buf_size = res;
                    this->recv_buf_index = len;
                    return len;
                }
                else {
                    memcpy(data, this->recv_buf, res);
                    data += res;
                }
                return res;
        }
    }

    ssize_t privsend(const uint8_t * data, size_t len)
    {
        size_t total = 0;
        while (total < len) {
            ssize_t sent = ::send(this->sck, data + total, len - total, 0);
            switch (sent){
            case -1:
                if (try_again(errno)) {
                    fd_set wfds;
                    struct timeval time = { 0, 10000 };
                    io_fd_zero(wfds);
                    io_fd_set(this->sck, wfds);
                    select(this->sck + 1, nullptr, &wfds, nullptr, &time);
                    continue;
                }
                return -1;
            case 0:
                return -1;
            default:
                total = total + sent;
            }
        }
        return len;
    }

};
